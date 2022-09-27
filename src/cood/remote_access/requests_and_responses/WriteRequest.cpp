/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/WriteRequest.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/string/tools.hpp>
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const WriteRequest::writeRequestBinarySize;

/**
 * \brief Constructor.
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
 * \param _accessType
 * Access type.
 *
 * \param _index
 * Index of the object that shall be written.
 *
 * \param _subindex
 * Subindex that shall be written.
 *
 * \param _permissions
 * Permissions for the write-operation provided by the originator of the write request.\n
 * This shall be composed by logical-or-combination of one or multiple Object::attr_ACCESS_xxx values.\n
 * The composed value shall have at least one write permission (@ref Object::attr_ACCESS_WR) bit set.\n
 * There shall be no other bits set except for write permission bits.
 *
 * \param _data
 * Universal reference to an `std::vector<uint8_t>` containing the data that shall be written.\n
 * The data shall be encoded in CANopen format.\n
 * The vector shall contain at least one or two byte of data (depending on _accessType and _subindex) and no more than
 * 65535 bytes of data.\n
 * \n
 * The content of the referenced vector _will be moved into the WriteRequest-object_.\n
 * Afterwards the referenced vector will be in a valid but undefined state.\n
 * \n
 *
 * \param _maxResponseSize
 * Maximum size (in byte) of the serialized response object that can be processed by the creator of this request.
 * The value should be the minimum of the capability of the creator and the maximum possible response size announced
 * by @ref IRemoteObjectDictionaryAccessNotifiable::OnReady(), parameter `maxResponseSize`.\n
 * Minimum value: @ref ResponseBase::minimumUsefulResponseSize \n
 * Maximum value: @ref ResponseBase::maxResponseSize \n
 * \n
 * The value usually does not contain any @ref ReturnStackItem objects. However, if the creator of the request is going
 * to push a @ref ReturnStackItem object onto the stack of the request before passing it to
 * @ref IRemoteObjectDictionaryAccess::Send(), then `_maxResponseSize` shall be decreased by the size of
 * a serialized @ref ReturnStackItem object to compensate for @ref Push(), which will add the size of a
 * @ref ReturnStackItem object.\n
 * See figure below:
 * \htmlonly <style>div.image img[src="cood/RODA_ReqCTOR_MaxResponseSize.png"]{width:80%;}</style> \endhtmlonly
 * \image html "cood/RODA_ReqCTOR_MaxResponseSize.png" "Maximum response size with one ReturnStackItem"
 */
WriteRequest::WriteRequest(AccessType           const _accessType,
                           uint16_t             const _index,
                           uint8_t              const _subindex,
                           Object::attr_t       const _permissions,
                           std::vector<uint8_t> &&    _data,
                           size_t               const _maxResponseSize)
: RequestBase(RequestTypes::writeRequest, _maxResponseSize)
, accessType(_accessType)
, index(_index)
, subindex(_subindex)
, permissions(_permissions)
, data()
{
  if ((accessType != AccessType::singleSubindex) && (subindex > 1U))
    throw std::invalid_argument("WriteRequest::WriteRequest: Complete access requires '_subindex' in range 0..1");

  if (   ((permissions & Object::attr_ACCESS_WR) == 0U)
      || ((permissions & Object::attr_ACCESS_WR) != permissions))
  {
    throw std::invalid_argument("WriteRequest::WriteRequest: '_permissions' invalid.");
  }

  size_t const minReqData = ((accessType == AccessType::completeAccess_SI0_16bit) && (subindex == 0U)) ? 2U : 1U;

  if ((_data.size() < minReqData) || (_data.size() > std::numeric_limits<uint16_t>::max()))
    throw std::invalid_argument("WriteRequest::WriteRequest: '_data' is too small or too large");

  data = std::move(_data);
}

/**
 * \brief Constructor. Creates a @ref WriteRequest object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref WriteRequest object.
 *
 * This is intended to be invoked by @ref RequestBase::FromBinary() only. In conjunction with
 * @ref RequestBase::FromBinary(), this is the counterpart to @ref RequestBase::ToBinary().
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
WriteRequest::WriteRequest(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, WriteRequestPassKey)
: RequestBase(RequestTypes::writeRequest, sr, versionOnHand)
, accessType(U8_to_AccessType(sr.Read_uint8()))
, index(sr.Read_uint16())
, subindex(sr.Read_uint8())
, permissions(static_cast<Object::attr_t>(sr.Read_uint16()))
, data()
{
  if (versionOnHand != version)
    throw std::runtime_error("WriteRequest::WriteRequest: Version not supported");

  if ((accessType != AccessType::singleSubindex) && (subindex > 1U))
    throw std::runtime_error("WriteRequest::WriteRequest: Data read from 'sr' is invalid");

  if (   ((permissions & Object::attr_ACCESS_WR) == 0U)
      || ((permissions & Object::attr_ACCESS_WR) != permissions))
  {
    throw std::runtime_error("WriteRequest::WriteRequest: Data read from 'sr' is invalid");
  }

  size_t const minReqData = ((accessType == AccessType::completeAccess_SI0_16bit) && (subindex == 0U)) ? 2U : 1U;

  uint_fast16_t s = sr.Read_uint16();
  if (s < minReqData)
    throw std::runtime_error("WriteRequest::WriteRequest: Data read from 'sr' is invalid");

  data.reserve(s);
  while (s != 0U)
  {
    data.push_back(sr.Read_uint8());
    --s;
  }
}

/**
 * \brief Calculates the maximum data payload that can be added to a @ref WriteRequest object which still meets
 *        the maximum request size.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param maxRequestSize
 * Maximum permitted request size in byte.\n
 * The value refers to a serialized request object without any @ref ReturnStackItem objects included in the request.\n
 * Usually the caller will pass the minimum of its own capability and the maximum possible response size announced
 * by @ref IRemoteObjectDictionaryAccessNotifiable::OnReady(), parameter `maxResponseSize`.
 *
 * \param withRSI
 * Controls, if the function shall reserve space for one @ref ReturnStackItem pushed onto the stack of the request
 * object by the creator of the request.
 *
 * \return
 * Maximum size of the data payload that can be attached to the write request.\n
 * Zero, if the given `maxRequestSize` is too small.
 */
size_t WriteRequest::CalcMaxDataPayload(size_t const maxRequestSize, bool const withRSI) noexcept
{
  size_t maxDataPayload = maxRequestSize;

  // subtract overhead of base class RequestBase and class WriteRequest
  size_t const binarySize = baseBinarySize + writeRequestBinarySize;
  if (maxDataPayload <= binarySize)
    return 0U;
  else
    maxDataPayload -= binarySize;

  // optional: subtract overhead of one ReturnStackItem
  if (withRSI)
  {
    if (maxDataPayload <= ReturnStackItem::binarySize)
      return 0U;
    else
      maxDataPayload -= ReturnStackItem::binarySize;
  }

  // limit to u16
  if (maxDataPayload > std::numeric_limits<uint16_t>::max())
    maxDataPayload = std::numeric_limits<uint16_t>::max();

  // that's it
  return maxDataPayload;
}

// <-- RequestBase

/// \copydoc gpcc::cood::RequestBase::GetBinarySize
size_t WriteRequest::GetBinarySize(void) const
{
  return RequestBase::GetBinarySize() + writeRequestBinarySize + data.size();
}

/// \copydoc gpcc::cood::RequestBase::ToBinary
void WriteRequest::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  if (data.empty())
    throw std::logic_error("WriteRequest::ToBinary: Object empty. Was it involved in a move-operation?");

  RequestBase::ToBinary(sw);

  sw.Write_uint8(static_cast<uint8_t>(accessType));
  sw.Write_uint16(index);
  sw.Write_uint8(subindex);
  sw.Write_uint16(permissions);
  sw.Write_uint16(data.size());
  sw.Write_uint8(data.data(), data.size());
}

/// \copydoc gpcc::cood::RequestBase::ToString
std::string WriteRequest::ToString(void) const
{
  std::ostringstream s;

  s << "Write request (";

  switch (accessType)
  {
    case AccessType::singleSubindex:
      s << "single subindex";
      break;

    case AccessType::completeAccess_SI0_8bit:
      s << "CA, SI0 8bit";
      break;

    case AccessType::completeAccess_SI0_16bit:
      s << "CA, SI0 16bit";
      break;
  }

  s << ") for " << gpcc::string::ToHex(index, 4U) << ':' << static_cast<unsigned int>(subindex) << ", "
       "Permission = " << gpcc::string::ToHex(static_cast<uint16_t>(permissions), 4U) << ", "
    << data.size() << " byte(s) of data";

  if (data.size() <= 16U)
  {
    s << ':' << std::endl;

    for (uint_fast8_t i = 0U; i < data.size(); i++)
    {
      if (i != 0U)
        s << ' ';

      s << gpcc::string::ToHex(data[i], 2U);
    }
  }

  return s.str();
}

// --> RequestBase

/**
 * \brief Retrieves an unmodifiable reference to the data that shall be written.
 *
 * \pre   This @ref WriteRequest instance was not the source of a move-construction.
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
 * Unmodifiable reference to an `std::vector<uint8_t>` containing the data that shall be written.\n
 * The life-time of the referenced object is limited to the life time of the @ref WriteRequest instance.
 */
std::vector<uint8_t> const & WriteRequest::GetData(void) const
{
  if (data.empty())
    throw std::logic_error("WriteRequest::GetData: Object empty. Was it involved in a move-operation?");

  return data;
}

/**
 * \brief Safely converts an uint8_t value into a @ref AccessType enum value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   `u8` is not a valid value from the @ref AccessType enumeration.\n
 *                              This is not treated as a logic error, because `u8` is considered to be untested
 *                              data from outside world.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param u8
 * uint8_t value that shall be converted into a @ref AccessType enum value.\n
 * This method handles invalid values gracefully.
 *
 * \return
 * Corresponding value from the @ref AccessType enumeration.
 */
WriteRequest::AccessType WriteRequest::U8_to_AccessType(uint8_t const u8)
{
  switch (u8)
  {
    case static_cast<uint8_t>(AccessType::singleSubindex):
      return AccessType::singleSubindex;

    case static_cast<uint8_t>(AccessType::completeAccess_SI0_8bit):
      return AccessType::completeAccess_SI0_8bit;

    case static_cast<uint8_t>(AccessType::completeAccess_SI0_16bit):
      return AccessType::completeAccess_SI0_16bit;

    default:
      throw std::runtime_error("WriteRequest::U8_to_AccessType: No valid access type value");
  }
}

} // namespace cood
} // namespace gpcc
