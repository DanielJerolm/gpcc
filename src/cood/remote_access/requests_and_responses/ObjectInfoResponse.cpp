/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "ObjectInfoResponse.hpp"
#include "gpcc/src/cood/exceptions.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include <limits>
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \brief Constructor. Creates a @ref ObjectInfoResponse instance containing a negative result indicating that
 *        the object meta data query has failed for some reason.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _result
 * Result of the object meta data query operation.\n
 * @ref SDOAbortCode::OK is not allowed.
 */
ObjectInfoResponse::ObjectInfoResponse(SDOAbortCode const _result)
: ResponseBase(ResponseTypes::objectInfoResponse)
, result(_result)
, inclusiveNames(false)
, inclusiveAppSpecificMetaData(false)
, objectCode(Object::ObjectCode::Null)
, objType(DataType::null)
, objName()
, maxNbOfSubindices(0U)
, firstSubindex(0U)
, subindexDescr()
{
  if (result == SDOAbortCode::OK)
    throw std::invalid_argument("ObjectInfoResponse::ObjectInfoResponse: Result 'OK' not allowed for this CTOR");
}

/**
 * \brief Constructor. Creates a @ref ObjectInfoResponse instance filled with information about the meta data of a given
 *        CANopen object and a range of subindices.
 *
 * The range of subindices whose meta data shall be queried can be specified via `_firstSubindex` and `lastSubindex`.
 * The created object will always contain the meta data of at least one subindex. If necessary, then `_firstSubindex`
 * will be reduced. If the response size is too small for the meta data of at least one subindex, then this will throw.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param obj
 * Unmodifiable reference to the object whose meta data shall be queried.
 *
 * \param _firstSubindex
 * Number of the first subindex that shall be contained in the response.\n
 * If the value exceeds the object's maximum number of subindices minus one, then this value will be reduced so that the
 * response contains the meta data of at least one subindex.
 *
 * \param lastSubindex
 * Number of the last subindex that shall be contained in the response.\n
 * _This value will be reduced_ if the maximum response size would be exceeded. See documentation of class
 * @ref ObjectInfoResponse for details about fragmentation.\n
 * The value returned by @ref GetLastQueriedSubindex() may be larger than this if the queried object is an ARRAY and if
 * application specific meta data is not included in the query.
 *
 * \param _inclusiveNames
 * Controls if the names of the object and the subindices shall be included in the response (true) or not (false).\n
 * If names are included, then the size of the response may increase significantly.
 *
 * \param _inclusiveAppSpecificMetaData
 * Controls if application specific meta data of the subindices shall be included in the response (true) or not (false).\n
 * If application specific meta data is included, then the size of the response may increase significantly.
 *
 * \param maxResponseSize
 * Maximum permitted response size in byte.\n
 * The value refers to a serialized response object incl. potential @ref ReturnStackItem objects and response payload
 * data.
 *
 * \param returnStackSize
 * Size (in byte) of the stack of @ref ReturnStackItem objects that will be moved from the request object to the
 * response object.
 */
ObjectInfoResponse::ObjectInfoResponse(Object const & obj,
                                       uint8_t const _firstSubindex,
                                       uint8_t lastSubindex,
                                       bool const _inclusiveNames,
                                       bool const _inclusiveAppSpecificMetaData,
                                       size_t const maxResponseSize,
                                       size_t const returnStackSize)
: ResponseBase(ResponseTypes::objectInfoResponse)
, result(SDOAbortCode::OK)
, inclusiveNames(_inclusiveNames)
, inclusiveAppSpecificMetaData(_inclusiveAppSpecificMetaData)
, objectCode(obj.GetObjectCode())
, objType(obj.GetObjectDataType())
, objName()
, maxNbOfSubindices(obj.GetMaxNbOfSubindices())
, firstSubindex(_firstSubindex)
, subindexDescr()
{
  if (firstSubindex > lastSubindex)
    throw std::invalid_argument("ObjectInfoResponse::ObjectInfoResponse: Subindex range is invalid");

  if ((maxNbOfSubindices == 0U) || (maxNbOfSubindices > 256U))
    throw std::logic_error("ObjectInfoResponse::ObjectInfoResponse: obj.GetMaxNbOfSubindices() returns invalid value");

  if (inclusiveNames)
    objName = obj.GetObjectName();

  if (firstSubindex >= maxNbOfSubindices)
    firstSubindex = maxNbOfSubindices - 1U;

  // ARRAY is special:
  // If application specific meta data is NOT included in the response, then all SIs have same properties. In that case
  // it is sufficient to collect information about SI0 and SI1 only. The information collected about SI1 can be used for
  // all other SIs too.
  if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
  {
    if (firstSubindex > 1U)
    {
      firstSubindex = 1U;
      lastSubindex = 1U;
    }
    else if (lastSubindex > 1U)
    {
      lastSubindex = 1U;
    }
  }

  if (lastSubindex >= maxNbOfSubindices)
    lastSubindex = maxNbOfSubindices - 1U;


  // Now [firstSubindex; lastSubindex] is the range of subindices we have to query and fill into "subindexDescr".
  // We have to fill at least the information about "firstSubIndex" into "subindexDescr".
  // The remaining number of queried subindices depends on the maximum payload capacity of the remote access response.

  size_t remainingCapacity = CalcRemainingPayload(maxResponseSize, returnStackSize);

  subindexDescr.reserve(((lastSubindex - firstSubindex) + 1U));

  for (uint_fast16_t i = firstSubindex; i <= lastSubindex; i++)
  {
    // Query meta data from SI and determine binary size of the container created for the subindex' meta data.
    subindexDescr.emplace_back(obj, i, inclusiveNames, inclusiveAppSpecificMetaData);
    auto const s = subindexDescr.back().GetBinarySize();

    // capacity of response exceeded?
    if (s > remainingCapacity)
    {
      // Is it the first subindex description? -> :-( we have to deliver at least one description
      if (i == firstSubindex)
      {
        throw std::runtime_error("ObjectInfoResponse::ObjectInfoResponse: No space for at least one SI description");
      }
      else
      {
        // remove the last item and finish
        subindexDescr.pop_back();
        break;
      }
    }

    remainingCapacity -= s;
    if (remainingCapacity == 0U)
      break;
  }
}

/**
 * \brief Constructor. Creates a @ref ObjectInfoResponse object from data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref ObjectInfoResponse object.
 *
 * This is intended to be invoked by @ref ResponseBase::FromBinary() only. In conjunction with
 * @ref ResponseBase::FromBinary(), this is the counterpart to @ref ResponseBase::ToBinary().
 *
 * \post   Any data associated with the object has been consumed from the stream.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from `sr` and `sr` is not recovered.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from `sr` and `sr` is not recovered.
 *
 * - - -
 *
 * \param sr
 * Stream from which the data shall be read.
 *
 * \param versionOnHand
 * Version of serialized object read from `sr`.
 */
ObjectInfoResponse::ObjectInfoResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectInfoResponsePassKey)
: ResponseBase(ResponseTypes::objectInfoResponse, sr, versionOnHand)
, result(SDOAbortCode::GeneralError)
, inclusiveNames(false)
, inclusiveAppSpecificMetaData(false)
, objectCode(Object::ObjectCode::Null)
, objType(DataType::null)
, objName()
, maxNbOfSubindices(0U)
, firstSubindex(0U)
, subindexDescr()
{
  try
  {
    result = U32ToSDOAbortCode(sr.Read_uint32());
    if (result != SDOAbortCode::OK)
      return;

    inclusiveNames = sr.Read_bool();
    inclusiveAppSpecificMetaData = sr.Read_bool();

    objectCode = Object::ToObjectCode(sr.Read_uint8());
    objType    = ToDataType(sr.Read_uint16());

    if (inclusiveNames)
      objName = sr.Read_string();

    maxNbOfSubindices = sr.Read_uint16();
    if ((maxNbOfSubindices == 0U) || (maxNbOfSubindices > 256U))
      throw std::runtime_error("Data read from 'sr' is invalid");

    firstSubindex = sr.Read_uint8();
    if (firstSubindex >= maxNbOfSubindices)
      throw std::runtime_error("Data read from 'sr' is invalid");

    uint_fast16_t s = sr.Read_uint16();
    if ((s == 0U) || (s > 256U))
      throw std::runtime_error("Data read from 'sr' is invalid");

    if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
    {
      if (firstSubindex > 1U)
        throw std::runtime_error("Data read from 'sr' is invalid");

      if ((firstSubindex + s) > 2U)
        throw std::runtime_error("Data read from 'sr' is invalid");
    }
    else
    {
      if ((firstSubindex + s) > maxNbOfSubindices)
        throw std::runtime_error("Data read from 'sr' is invalid");
    }

    subindexDescr.reserve(s);

    do
    {
      subindexDescr.emplace_back(sr);

      if (   (subindexDescr.back().inclName != inclusiveNames)
          || (subindexDescr.back().inclASM  != inclusiveAppSpecificMetaData))
      {
        throw std::runtime_error("Data read from 'sr' is invalid");
      }
    }
    while (--s != 0U);
  }
  catch (std::bad_alloc const &) { throw; }
  catch (std::exception const &)
  {
    std::throw_with_nested(std::runtime_error("ObjectInfoResponse::ObjectInfoResponse: Deserialization failed."));
  }
}

// <-- ResponseBase

/// \copydoc gpcc::cood::ResponseBase::GetBinarySize
size_t ObjectInfoResponse::GetBinarySize(void) const
{
  size_t s = ResponseBase::GetBinarySize();

  // result              4
  // ---------------------
  //                    =4
  s += 4U;
  if (result != SDOAbortCode::OK)
    return s;

  // inclusiveNames               0.1
  // inclusiveAppSpecificMetaData 0.1
  //                              0.6
  // objectCode                   +1
  // objType                      +2
  // objName                      variable
  // maxNbOfSubindices            +2
  // firstSubindex                +1
  // subindexDescr.size           +2
  // --------------------------------
  //                              =9 + objName
  s += 9U;

  if (inclusiveNames)
    s += objName.length() + 1U; // (incl. null-terminator)

  // subindexDescr
  for (auto const & e: subindexDescr)
    s += e.GetBinarySize();

  return s;
}

/// \copydoc gpcc::cood::ResponseBase::ToBinary
void ObjectInfoResponse::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  ResponseBase::ToBinary(sw);

  sw.Write_uint32(static_cast<uint32_t>(result));

  if (result != SDOAbortCode::OK)
    return;

  ValidateObjNotEmpty();

  sw.Write_bool(inclusiveNames);
  sw.Write_bool(inclusiveAppSpecificMetaData);

  sw.Write_uint8(Object::ToUint8(objectCode));
  sw.Write_uint16(ToUint16(objType));

  if (inclusiveNames)
    sw.Write_string(objName);

  sw.Write_uint16(maxNbOfSubindices);
  sw.Write_uint8(firstSubindex);
  sw.Write_uint16(subindexDescr.size());

  for (auto const & e: subindexDescr)
    e.ToBinary(sw);
}

/// \copydoc gpcc::cood::ResponseBase::ToString
std::string ObjectInfoResponse::ToString(void) const
{
  std::ostringstream s;
  s << "Object info response (" << SDOAbortCodeToDescrString(result) << ")";
  if (result == SDOAbortCode::OK)
  {
    ValidateObjNotEmpty();
    s << ", incl. " << subindexDescr.size() << " subindex descriptions";
  }

  return s.str();
}

// --> ResponseBase

/**
 * \brief Appends another @ref ObjectInfoResponse that is part of a fragmented transfer to this instance.
 *
 * Class @ref ObjectInfoResponse supports fragmentation. See description of class @ref ObjectInfoResponse for
 * details and for an example.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param fragment
 * @ref ObjectInfoResponse containing the next fragment of the transfer that shall be defragmented into this object.\n
 * In case of success (no exception thrown), the content of the referenced object will be moved into this object.\n
 * The referenced object is left in a valid, but undefined state.
 */
void ObjectInfoResponse::AddFragment(ObjectInfoResponse && fragment)
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::AddFragment: Query failed");

  ValidateObjNotEmpty();

  if (objectCode == Object::ObjectCode::Variable)
    throw std::logic_error("ObjectInfoResponse::AddFragment: Call not expected for VARIABLE object.");

  uint8_t nextSI;
  if (IsComplete(&nextSI))
    throw std::logic_error("ObjectInfoResponse::AddFragment: Object is already complete.");


  if (fragment.result != SDOAbortCode::OK)
    throw std::invalid_argument("ObjectInfoResponse::AddFragment: Fragment has bad result code.");

  fragment.ValidateObjNotEmpty();


  if (   (fragment.inclusiveNames != inclusiveNames)
      || (fragment.inclusiveAppSpecificMetaData != inclusiveAppSpecificMetaData)
      || (fragment.objectCode != objectCode)
      || (fragment.objType != objType)
      || (fragment.maxNbOfSubindices != maxNbOfSubindices))
  {
    throw std::invalid_argument("ObjectInfoResponse::AddFragment: Queried objects do not match");
  }

  if (fragment.firstSubindex != nextSI)
    throw std::invalid_argument("ObjectInfoResponse::AddFragment: Discontinuity");

  if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
  {
    if ((fragment.firstSubindex != 1U) || (fragment.subindexDescr.size() != 1U))
      throw std::invalid_argument("ObjectInfoResponse::AddFragment: Fragment invalid");
  }
  else
  {
    if (firstSubindex + subindexDescr.size() + fragment.subindexDescr.size() > maxNbOfSubindices)
      throw std::logic_error("ObjectInfoResponse::AddFragment: Merge would result in invalid object");
  }

  if (subindexDescr.capacity() < (subindexDescr.size() + fragment.subindexDescr.size()))
    throw std::logic_error("ObjectInfoResponse::AddFragment: Capacity invalid");

  for (auto & e: fragment.subindexDescr)
    subindexDescr.emplace_back(std::move(e));

  fragment.subindexDescr.clear();
}

/**
 * \brief Retrieves the result of the operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * @ref SDOAbortCode indicating the result of the meta data query.
 */
SDOAbortCode ObjectInfoResponse::GetResult(void) const noexcept
{
  return result;
}

/**
 * \brief Queries if the read meta data contains the names of the object and the subindices.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   The name of the object and the names of subindices are included.
 * \retval false  No names included.
 */
bool ObjectInfoResponse::IsInclusiveNames(void) const noexcept
{
  return inclusiveNames;
}

/**
 * \brief Queries if the read meta data contains application specific meta data of the subindices.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   Application specific meta data of the subindices is included.
 * \retval false  No application specific meta data is included.
 */
bool ObjectInfoResponse::IsInclusiveAppSpecificMetaData(void) const noexcept
{
  return inclusiveAppSpecificMetaData;
}

/**
 * \brief Retrieves the number of the first subindex whose meta data is contained in this response object.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Number of the first subindex whose meta data is contained in this response object.
 */
uint8_t ObjectInfoResponse::GetFirstQueriedSubindex(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::GetFirstQueriedSubindex: Query failed");

  return firstSubindex;
}

/**
 * \brief Retrieves the number of the last subindex whose meta data is contained in this response object.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Number of the last subindex whose meta data is contained in this response object.
 */
uint8_t ObjectInfoResponse::GetLastQueriedSubindex(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::GetLastQueriedSubindex: Query failed");

  ValidateObjNotEmpty();

  if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
  {
    // If application specific meta data is not included, then for ARRAY objects SI1..SIn have the same meta data,
    // so SI1's meta data can be used for all subindices.

    if ((firstSubindex + (subindexDescr.size() - 1U)) >= 1U)
      return maxNbOfSubindices - 1U;
    else
      return 0U;
  }
  else
  {
    return (firstSubindex + (subindexDescr.size() - 1U));
  }
}

/**
 * \brief Retrieves if there are more subindices behind the one referenced by @ref GetLastQueriedSubindex(), whose meta
 *        data has not yet been queried.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pNextSubindex
 * If there are more subindices behind the one referenced by @ref GetLastQueriedSubindex(), whose meta data has not yet
 * been read, then the number of the subindex where reading meta data should continue is written into the referenced
 * variable.\n
 * nullptr is allowed if the caller is not interested in this information.
 *
 * \retval true   There are no more subindices left to be read.
 * \retval false  There is at least one subindex left. Its number will be written into the variable referenced by
 *                `pNextSubindex`.
 */
bool ObjectInfoResponse::IsComplete(uint8_t * const pNextSubindex) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::IsComplete: Query failed");

  ValidateObjNotEmpty();

  uint_fast16_t const nextSI = firstSubindex + subindexDescr.size();

  if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
  {
    // Note:
    // If application specific meta data is not included, then for ARRAY objects, firstSubindex is 0..1,
    // subindexDescr.size() is 1..2 and the sum of both is always equal to or less than 2.
    // This means: The first condition may only come true if maxNbOfSubindices is 1.
    if ((nextSI == maxNbOfSubindices) || (nextSI == 2U))
    {
      return true;
    }
    else
    {
      if (pNextSubindex != nullptr)
        *pNextSubindex = nextSI;

      return false;
    }
  }
  else
  {
    if (nextSI == maxNbOfSubindices)
    {
      return true;
    }
    else
    {
      if (pNextSubindex != nullptr)
        *pNextSubindex = nextSI;

      return false;
    }
  }
}

/**
 * \brief Retrieves the [object code](@ref gpcc::cood::Object::ObjectCode) of the queried object.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * [Object code](@ref gpcc::cood::Object::ObjectCode) of the queried object.
 */
Object::ObjectCode ObjectInfoResponse::GetObjectCode(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::GetObjectCode: Query failed");

  return objectCode;
}

/**
 * \brief Retrieves the [data type](@ref gpcc::cood::DataType) of the queried object.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * [Data type](@ref gpcc::cood::DataType) of the queried object.
 */
DataType ObjectInfoResponse::GetObjectDataType(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::GetObjectDataType: Query failed");

  return objType;
}

/**
 * \brief Retrieves the name of the queried object.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   Object and subindex names are included in the query.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Name of the queried object.
 */
std::string const & ObjectInfoResponse::GetObjectName(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::GetObjectName: Query failed");

  if (!inclusiveNames)
    throw std::logic_error("ObjectInfoResponse::GetObjectName: No names");

  return objName;
}

/**
 * \brief Retrieves the maximum number of subindices (incl. subindex 0) of the queried object.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Maximum number of subindices (incl. subindex 0) of the queried object.
 */
uint16_t ObjectInfoResponse::GetMaxNbOfSubindices(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::GetMaxNbOfSubindices: Query failed");

  return maxNbOfSubindices;
}

/**
 * \brief Retrieves if a specific subindex of the queried object is empty.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \retval true   Subindex is empty.
 * \retval false  Subindex is not empty.
 */
bool ObjectInfoResponse::IsSubIndexEmpty(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);
  return subindexDescr[i].empty;
}

/**
 * \brief Retrieves the [data type](@ref gpcc::cood::DataType) of a specific subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \return
 * [Data type](@ref gpcc::cood::DataType) of the object's subindex referenced by `subindex`.
 */
DataType ObjectInfoResponse::GetSubIdxDataType(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);
  return subindexDescr[i].dataType;
}

/**
 * \brief Retrieves the attributes of a specific subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \return
 * Attributes of the object's subindex referenced by `subindex`.
 */
Object::attr_t ObjectInfoResponse::GetSubIdxAttributes(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);
  return subindexDescr[i].attributes;
}

/**
 * \brief Retrieves the maximum size (in bit) of a specific subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \return
 * Maximum size (in bit) of the object's subindex referenced by `subindex`.
 */
size_t ObjectInfoResponse::GetSubIdxMaxSize(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);
  return subindexDescr[i].maxSize;
}

/**
 * \brief Retrieves the name of a specific subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   Object and subindex names are included in the query.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \return
 * Name of the object's subindex referenced by `subindex`.
 */
std::string ObjectInfoResponse::GetSubIdxName(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);

  if (!subindexDescr[i].inclName)
    throw std::logic_error("ObjectInfoResponse::GetSubIdxName: No names");

  if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
  {
    if (subIdx == 0U)
      return subindexDescr[0].name;
    else
      return "Subindex " + std::to_string(static_cast<unsigned int>(subIdx));
  }
  else
  {
    return subindexDescr[i].name;
  }
}

/**
 * \brief Retrieves the size of the application specific meta data of a specific subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   Application specific meta data is included in the query.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \return
 * Size of the application-specific meta data of the object's subindex referenced by `subindex` in byte.\n
 * Zero if there is no application-specific meta data attached to the given subindex, or if the object does not support
 * application-specific meta data at all.
 */
size_t ObjectInfoResponse::GetAppSpecificMetaDataSize(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);

  if (!subindexDescr[i].inclASM)
    throw std::logic_error("ObjectInfoResponse::GetAppSpecificMetaDataSize: No ASM");

  return subindexDescr[i].appSpecMetaData.size();
}

/**
 * \brief Retrieves the application specific meta data of a specific subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   Application specific meta data is included in the query.
 *
 * \pre   The meta data of the subindex is contained in the meta data encapsulated in this object.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex.
 *
 * \return
 * Copy of the application-specific meta data of the given subindex.
 */
std::vector<uint8_t> ObjectInfoResponse::GetAppSpecificMetaData(uint8_t const subIdx) const
{
  // precondition "result == OK" is checked by MapSubindexToSubIndexDescr()
  // precondition "not source of move-operation" is checked by MapSubindexToSubIndexDescr()

  uint8_t const i = MapSubindexToSubIndexDescr(subIdx);

  if (!subindexDescr[i].inclASM)
    throw std::logic_error("ObjectInfoResponse::GetAppSpecificMetaData: No ASM");

  return subindexDescr[i].appSpecMetaData;
}

/**
 * \brief Validates, that the object was not the source of a move-operation.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK. \n
 *        This is intended to be checked by the caller and is not tested here.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void ObjectInfoResponse::ValidateObjNotEmpty(void) const
{
  if (subindexDescr.empty())
    throw std::logic_error("ObjectInfoResponse::ValidateObjNotEmpty: Object was source of move-operation.");
}

/**
 * \brief Calculates the remaining number of bytes that could be added to this @ref ObjectInfoResponse object, so that
 *        it still meets the maximum response size.
 *
 * This is intended to be used by the CTOR of class @ref ObjectInfoResponse to determine the number of subindex
 * descriptions that could be embedded in the object.
 *
 * The calculation takes into account:
 * - Current content of the object.
 * - Size of the return stack that will finally be moved to the object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::logic_error   Current size of object exceeds maximum response size.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param maxResponseSize
 * Maximum permitted response size in byte.\n
 * The value refers to a serialized response object incl. potential @ref ReturnStackItem objects and response payload
 * data.
 *
 * \param returnStackSize
 * Size (in byte) of the stack of @ref ReturnStackItem objects that will be moved from the request object to the
 * response object.
 *
 * \return
 * Maximum size (in byte) of the data payload that could be added to this object.
 */
size_t ObjectInfoResponse::CalcRemainingPayload(size_t const maxResponseSize, size_t const returnStackSize) const
{
  // start
  size_t remainingDataPayloadCapacity = maxResponseSize;

  // subtract current size of the object
  auto const binarySize = GetBinarySize();

  if (remainingDataPayloadCapacity < binarySize)
    throw std::logic_error("ObjectInfoResponse::CalcRemainingPayload: Object size exceeds maxResponseSize");

  remainingDataPayloadCapacity -= binarySize;

  // subtract overhead of return stack
  if (remainingDataPayloadCapacity < returnStackSize)
    throw std::logic_error("ObjectInfoResponse::CalcRemainingPayload: No space for return stack");

  remainingDataPayloadCapacity -= returnStackSize;

  return remainingDataPayloadCapacity;
}

/**
 * \brief Determines the index in @ref subindexDescr that contains the meta data of a given subindex.
 *
 * \pre   The result of the query is @ref SDOAbortCode::OK.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * \throws std::logic_error           The subindex is existing, but this object does not contain information about the
 *                                    given subindex because the given subindex has not been queried.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subindex
 * Subindex.
 *
 * \return
 * Index in @ref subindexDescr that contains the meta data of the object's subindex referenced by `subindex`.
 */
uint8_t ObjectInfoResponse::MapSubindexToSubIndexDescr(uint8_t const subindex) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ObjectInfoResponse::MapSubindexToSubIndexDescr: Query failed");

  ValidateObjNotEmpty();

  if (subindex >= maxNbOfSubindices)
    throw SubindexNotExistingError();

  if ((objectCode == Object::ObjectCode::Array) && (!inclusiveAppSpecificMetaData))
  {
    // note: firstSubindex is 0 or 1

    if (subindex < firstSubindex)
      throw std::logic_error("ObjectInfoResponse::MapSubindexToSubIndexDescr: No information about 'subIdx'");

    if (subindex == firstSubindex)
      return 0U;

    if ((firstSubindex == 0U) && (subindexDescr.size() == 1U))
      throw std::logic_error("ObjectInfoResponse::MapSubindexToSubIndexDescr: No information about 'subIdx'");

    return (firstSubindex == 0U) ? 1U : 0U;
  }
  else
  {
    if ((subindex < firstSubindex) || (subindex > (firstSubindex + (subindexDescr.size() - 1U))))
      throw std::logic_error("ObjectInfoResponse::MapSubindexToSubIndexDescr: No information about 'subIdx'");

    return subindex - firstSubindex;
  }
}

/**
 * \brief Constructor. Creates a @ref SubindexDescr instance from meta data queried from a CANopen object.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param obj
 * Object whose meta data shall be queried.\n
 * The object's mutex does not need to be locked.
 *
 * \param subIndex
 * Subindex whose meta data shall be queried.
 *
 * \param inclusiveName
 * Controls if the name of the subindex shall be queried (true) or not (false).
 *
 * \param inclusiveASM
 * Controls if the application specific meta data of the subindex shall be queried (true) or not (false).
 */
ObjectInfoResponse::SubindexDescr::SubindexDescr(Object const & obj,
                                                uint8_t const subIndex,
                                                bool const inclusiveName,
                                                bool const inclusiveASM)
: empty(obj.IsSubIndexEmpty(subIndex))
, inclName(inclusiveName)
, inclASM(inclusiveASM)
, maxSizeU8(false)
, appSpecMetaDataSizeU8(false)
, dataType(DataType::null)
, attributes(0U)
, maxSize(0U)
, name()
, appSpecMetaData()
{
  if (!empty)
  {
    dataType   = obj.GetSubIdxDataType(subIndex);
    attributes = obj.GetSubIdxAttributes(subIndex);
    maxSize    = obj.GetSubIdxMaxSize(subIndex);

    if (maxSize <= 255U)
      maxSizeU8 = true;

    if ((sizeof(size_t) > 4U) && (maxSize > std::numeric_limits<uint32_t>::max()))
      throw std::logic_error("ObjectInfoResponse::SubindexDescr::SubindexDescr: Object::GetSubIdxMaxSize() returns invalid value.");

    if (inclName)
      name = obj.GetSubIdxName(subIndex);

    if (inclASM)
    {
      size_t const s = obj.GetAppSpecificMetaDataSize(subIndex);
      if (s != 0U)
      {
        if ((sizeof(size_t) > 4U) && (s > std::numeric_limits<uint32_t>::max()))
          throw std::logic_error("ObjectInfoResponse::SubindexDescr::SubindexDescr: Object::GetAppSpecificMetaDataSize() returns invalid value.");

        appSpecMetaData = obj.GetAppSpecificMetaData(subIndex);
      }

      if (s <= 255U)
        appSpecMetaDataSizeU8 = true;
    }
  }
}

/**
 * \brief Constructor. Creates a @ref SubindexDescr instance by deserialization from binary data.
 *
 * \post   Any data associated with the object has been consumed from the stream.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from `sr` and `sr` is not recovered.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from `sr` and `sr` is not recovered.
 *
 * - - -
 *
 * \param sr
 * Interface for reading binary data.
 */
ObjectInfoResponse::SubindexDescr::SubindexDescr(gpcc::Stream::IStreamReader & sr)
: empty(false)
, inclName(false)
, inclASM(false)
, maxSizeU8(false)
, appSpecMetaDataSizeU8(false)
, dataType(DataType::null)
, attributes(0U)
, maxSize(0U)
, name()
, appSpecMetaData()
{
  empty = sr.Read_bool();

  if (empty)
  {
    sr.Skip(7U);
    return;
  }

  inclName = sr.Read_bool();
  inclASM = sr.Read_bool();
  maxSizeU8 = sr.Read_bool();
  appSpecMetaDataSizeU8 = sr.Read_bool();

  dataType = ToDataType(sr.Read_uint16());
  attributes = sr.Read_uint16();

  maxSize = maxSizeU8 ? sr.Read_uint8() : sr.Read_uint32();

  if (inclName)
    name = sr.Read_line();

  if (inclASM)
  {
    uint32_t s = appSpecMetaDataSizeU8 ? sr.Read_uint8() : sr.Read_uint32();
    appSpecMetaData.reserve(s);
    while (s != 0U)
    {
      appSpecMetaData.push_back(sr.Read_uint8());
      s--;
    }
  }
}

/**
 * \brief Returns the size of the output of @ref ToBinary().
 *
 * This method is intended to be used to determine the exact amount of memory required for invocation of
 * @ref ToBinary() in advance.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Size of the serialized object in byte.
 */
size_t ObjectInfoResponse::SubindexDescr::GetBinarySize(void) const
{
  // empty                  0.1
  // inclName              +0.1
  // inclASM               +0.1
  // maxSizeU8             +0.1
  // appSpecMetaDataSizeU8 +0.1
  //                       +0.3
  // --------------------------
  //                       =1
  size_t s = 1U;

  if (empty)
    return s;

  // dataType    2
  // attributes +2
  // -------------
  s +=          4U;

  // maxSize    1 / 4 (maxSizeU8 true/false)
  s += maxSizeU8 ? 1U : 4U;

  // name       variable/dynamic
  if (inclName)
    s += name.length() + 1U;

  // appSpecMetaData variable/dynamic
  if (inclASM)
  {
    s += appSpecMetaDataSizeU8 ? 1U : 4U;
    s += appSpecMetaData.size();
  }

  return s;
}

/**
 * \brief Writes a binary representation of the object into a stream.
 *
 * The counterpart of this is @ref SubindexDescr(gpcc::Stream::IStreamReader & sr).
 *
 * @ref GetBinarySize() may be used to determine the size of the written binary data in advance.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of undefined data may have been written to `sw`. `sw` is not recovered.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of undefined data may have been written to `sw`. `sw` is not recovered.
 *
 * - - -
 *
 * \param sw
 * The binary data is written into the referenced stream.
 */
void ObjectInfoResponse::SubindexDescr::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  sw.Write_bool(empty);

  if (empty)
  {
    sw.AlignToByteBoundary(false);
    return;
  }

  sw.Write_bool(inclName);
  sw.Write_bool(inclASM);
  sw.Write_bool(maxSizeU8);
  sw.Write_bool(appSpecMetaDataSizeU8);

  sw.Write_uint16(ToUint16(dataType));
  sw.Write_uint16(attributes);

  if (maxSizeU8)
    sw.Write_uint8(static_cast<uint8_t>(maxSize));
  else
    sw.Write_uint32(static_cast<uint32_t>(maxSize));

  if (inclName)
    sw.Write_string(name);

  if (inclASM)
  {
    if (appSpecMetaDataSizeU8)
      sw.Write_uint8(static_cast<uint8_t>(appSpecMetaData.size()));
    else
      sw.Write_uint32(static_cast<uint32_t>(appSpecMetaData.size()));
    sw.Write_uint8(appSpecMetaData.data(), appSpecMetaData.size());
  }
}

} // namespace cood
} // namespace gpcc
