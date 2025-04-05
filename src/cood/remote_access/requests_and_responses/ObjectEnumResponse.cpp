/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021, 2024, 2025 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <gpcc/string/tools.hpp>
#include <exception>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const ObjectEnumResponse::objectEnumResponseBinarySize_;
size_t const ObjectEnumResponse::maxNbOfIndices_;

/**
 * \brief Constructor. Creates a response object with encapsulated result initialized with an error status.
 *        This is the start point for any response, even those indicating success.
 *
 * Use @ref SetError() to change the result to a different error code at a later point in time.\n
 * Use @ref SetData() to attach the enumerated indices to the response object and to clear the error status.
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
 * \param result
 * Result value that shall be encapsulated in the object.\n
 * This must be a negative status code. @ref SDOAbortCode::OK is not allowed.
 */
ObjectEnumResponse::ObjectEnumResponse(SDOAbortCode const result)
: ResponseBase(ResponseTypes::objectEnumResponse)
, result_(result)
, complete_(false)
, indices_()
{
  if (result_ == SDOAbortCode::OK)
    throw std::invalid_argument("ObjectEnumResponse::ObjectEnumResponse: Negative result expected");
}

/**
 * \brief Constructor. Creates a @ref ObjectEnumResponse object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref ObjectEnumResponse object.
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
ObjectEnumResponse::ObjectEnumResponse(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectEnumResponsePassKey)
: ResponseBase(ResponseTypes::objectEnumResponse, sr, versionOnHand)
, result_(SDOAbortCode::GeneralError)
, complete_(false)
, indices_()
{
  auto const result_u32 = sr.Read_uint32();
  try
  {
    result_ = U32ToSDOAbortCode(result_u32);
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(std::runtime_error("ObjectEnumResponse::ObjectEnumResponse: Data read from 'sr' is invalid"));
  }

  if (result_ == SDOAbortCode::OK)
  {
    complete_ = sr.Read_bool();

    // MSB...
    uint_fast32_t nbOfIndices = (sr.Read_bool()) ? (1UL << 16U) : 0U;
    sr.Skip(6U);

    // ...and the lower 16 bits
    nbOfIndices |= sr.Read_uint16();
    if (nbOfIndices != 0U)
    {
      if (nbOfIndices > maxNbOfIndices_)
        throw std::runtime_error("ObjectEnumResponse::ObjectEnumResponse: Data read from 'sr' is invalid");

      // if maximum number of indices is contained, then enumeration MUST be complete
      if ((nbOfIndices == maxNbOfIndices_) && (!complete_))
        throw std::runtime_error("ObjectEnumResponse::ObjectEnumResponse: Data read from 'sr' is invalid");

      indices_.reserve(nbOfIndices);

      // read first index
      uint16_t val = sr.Read_uint16();
      indices_.push_back(val);
      --nbOfIndices;

      // read the others and check for ascending order
      while (nbOfIndices != 0U)
      {
        val = sr.Read_uint16();

        if (val <= indices_.back())
          throw std::runtime_error("ObjectEnumResponse::ObjectEnumResponse: Data read from 'sr' is invalid");

        indices_.push_back(val);
        --nbOfIndices;
      }

      // if the last index is included, then enumeration MUST be complete
      if ((!complete_) && (indices_.back() == 0xFFFFU))
        throw std::runtime_error("ObjectEnumResponse::ObjectEnumResponse: Data read from 'sr' is invalid");
    }
    else
    {
      // if no index is included, then enumeration MUST be complete
      if (!complete_)
        throw std::runtime_error("ObjectEnumResponse::ObjectEnumResponse: Data read from 'sr' is invalid");
    }
  }
}

/**
 * \brief Calculates the maximum number of indices that can be attached to a @ref ObjectEnumResponse object which still
 *        meets the maximum response size.
 *
 * This is intended to be used by class @ref RemoteAccessServer to determine the maximum number of indices that can
 * be attached to a response object.
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
 * Maximum number of object indices that can be attached to the response object.\n
 * Zero, if the given `maxResponseSize` is too small.
 */
size_t ObjectEnumResponse::CalcMaxNbOfIndices(size_t const maxResponseSize, size_t const returnStackSize) noexcept
{
  // start with payload in byte
  size_t maxDataPayload = maxResponseSize;

  // subtract overhead of ResponseBase and ObjectEnumResponse
  size_t const binarySize = baseBinarySize_ + objectEnumResponseBinarySize_;
  static_assert((binarySize + sizeof(uint16_t)) <= ResponseBase::minimumUsefulResponseSize,
                "Base size of serialized ObjectEnumResponse plus one payload item exceeds ResponseBase::minimumUsefulResponseSize");

  if (maxDataPayload <= binarySize)
    return 0U;
  else
    maxDataPayload -= binarySize;

  // subtract overhead of ReturnStackItem
  if (maxDataPayload <= returnStackSize)
    return 0U;
  else
    maxDataPayload -= returnStackSize;

  // translate to number of indices
  size_t maxPossibleNbOfIndices = maxDataPayload / 2U;

  // limit to maxNbOfIndices_
  if (maxPossibleNbOfIndices > maxNbOfIndices_)
    maxPossibleNbOfIndices = maxNbOfIndices_;

  // that's it
  return maxPossibleNbOfIndices;
}

// <-- ResponseBase

/// \copydoc gpcc::cood::ResponseBase::GetBinarySize
size_t ObjectEnumResponse::GetBinarySize(void) const
{
  size_t s = ResponseBase::GetBinarySize();

  // result 4
  // --------
  //       =4
  s += 4U;

  if (result_ == SDOAbortCode::OK)
  {
    // complete             0.1
    // MSB of indices.size  0.1
    //                     +0.6
    // indices.size        +2
    // ------------------------
    //                     =3
    s += 3U;

    s += indices_.size() * sizeof(uint16_t);
  }

  return s;
}

/// \copydoc gpcc::cood::ResponseBase::ToBinary
void ObjectEnumResponse::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  ResponseBase::ToBinary(sw);

  sw.Write_uint32(static_cast<uint32_t>(result_));

  if (result_ == SDOAbortCode::OK)
  {
    sw.Write_bool(complete_);
    sw.Write_bool((indices_.size() & 0x10000UL) != 0U);
    sw.AlignToByteBoundary(false);

    sw.Write_uint16(static_cast<uint16_t>(indices_.size() & 0xFFFFUL));
    sw.Write_uint16(indices_.data(), indices_.size());
  }
}

/// \copydoc gpcc::cood::ResponseBase::ToString
std::string ObjectEnumResponse::ToString(void) const
{
  gpcc::string::StringComposer s;
  s << "Object enum response: " << SDOAbortCodeToDescrString(result_);

  if (result_ == SDOAbortCode::OK)
  {
    s << ", ";

    if (!complete_)
      s << "not ";

    s << "complete, " << indices_.size() << " indices";
  }

  return s.Get();
}

// --> ResponseBase

/**
 * \brief Sets the encapsulated result value to an error status and clears any data contained in the response object.
 *
 * If success shall be indicated, then use @ref SetData() instead of this.
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
 * \param result
 * Desired value for the encapsulated result.\n
 * An error status is expected. @ref SDOAbortCode::OK is not allowed.
 */
void ObjectEnumResponse::SetError(SDOAbortCode const result)
{
  if (result == SDOAbortCode::OK)
    throw std::invalid_argument("ObjectEnumResponse::SetError: Negative result expected");

  result_ = result;
  indices_.clear();
}

/**
 * \brief Sets the result to @ref SDOAbortCode::OK and sets the data contained in the response object.
 *
 * Any data already set will be discarded and replaced by the new data.
 *
 * If an error status shall be set, then use @ref SetError() instead of this.
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
 * \param indices
 * The content of the referenced vector will be moved into the response object.\n
 * The indices must be sorted in ascending order.
 * The referenced vector will be empty afterwards.
 *
 * \param complete
 * Indicates if the enumeration request has been completely served (true), or if there are some objects left to
 * enumerate (false) whose indices did not fit into the response.
 */
void ObjectEnumResponse::SetData(std::vector<uint16_t> && indices, bool const complete)
{
  if (indices.size() > maxNbOfIndices_)
    throw std::invalid_argument("ObjectEnumResponse::SetData: indices is too large.");

  if (!complete)
  {
    // indices can't be empty if the enum operation is not complete
    if (indices.empty())
      throw std::invalid_argument("ObjectEnumResponse::SetData: Incomplete but no item");

    // if the last enumerated index is 0xFFFF, then the enum operation can't be incomplete
    if (indices.back() == 0xFFFFU)
      throw std::invalid_argument("ObjectEnumResponse::SetData: Incomplete but 0xFFFF included");

    // if all possible indices are enumerated, then the enum operation can't be incomplete
    if (indices.size() == maxNbOfIndices_)
      throw std::invalid_argument("ObjectEnumResponse::SetData: Incomplete but all included");
  }

  // check for ascending order of enumerated indices
  if (indices.size() > 1U)
  {
    int32_t prev = -1;
    for (int32_t const e: indices)
    {
      if (e <= prev)
        throw std::invalid_argument("ObjectEnumResponse::SetData: Not properly sorted");
      prev = e;
    }
  }

  indices_ = std::move(indices);
  indices.clear();

  result_ = SDOAbortCode::OK;
  complete_ = complete;
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
 * @ref SDOAbortCode indicating the result of the enumeration.
 */
SDOAbortCode ObjectEnumResponse::GetResult(void) const noexcept
{
  return result_;
}

/**
 * \brief Retrieves if the enumeration is complete.
 *
 * Class @ref ObjectEnumResponse supports fragmentation. See description of class @ref ObjectEnumResponse for
 * details and for an example.
 *
 * This method may be invoked either
 * - on a fragment _before_ it is added to the _very first_ response of a fragmented transfer
 * - or on the _very first_ response of a fragmented transfer at any time
 *
 * \pre   The result of the enumeration is @ref SDOAbortCode::OK.
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
 * \param pNextIndex
 * If the enumeration is not complete, then the index where a subsequent enumeration request shall continue is written
 * into the referenced variable.\n
 * This may be nullptr, if the caller is not interested in this information.
 *
 * \retval true  Enumeration is complete.
 * \retval false Enumeration is not complete. The caller shall emit another @ref ObjectEnumRequest which continues
 *               enumeration at the index written to `pNextIndex`.
 */
bool ObjectEnumResponse::IsComplete(uint16_t * const pNextIndex) const
{
  if (result_ != SDOAbortCode::OK)
    throw std::logic_error("ObjectEnumResponse::IsComplete: Enumeration failed");

  if (!complete_)
  {
    // "indices" can't be empty if the enum operation is not complete
    if (indices_.empty())
      throw std::logic_error("ObjectEnumResponse::IsComplete: Invalid. Source of move?");

    if (pNextIndex != nullptr)
    {
      uint32_t const nextIndex = indices_.back() + 1UL;
      if (nextIndex > 0xFFFFU)
        throw std::logic_error("ObjectEnumResponse::IsComplete: Invalid");

      *pNextIndex = nextIndex;
    }
  }

  return complete_;
}

/**
 * \brief Appends the content of a @ref ObjectEnumResponse that is part of a fragmented transfer to the content of
 *        this instance.
 *
 * Class @ref ObjectEnumResponse supports fragmentation. See description of class @ref ObjectEnumResponse for
 * details and for an example.
 *
 * \pre   The result of the enumeration is @ref SDOAbortCode::OK.
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
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param fragment
 * @ref ObjectEnumResponse containing the next fragment of the transfer that shall be defragmented into this object.
 */
void ObjectEnumResponse::AddFragment(ObjectEnumResponse const & fragment)
{
  if (result_ != SDOAbortCode::OK)
    throw std::logic_error("ObjectEnumResponse::AddFragment: Enumeration failed.");

  if (complete_)
    throw std::logic_error("ObjectEnumResponse::AddFragment: Already complete");

  if (fragment.result_ != SDOAbortCode::OK)
    throw std::invalid_argument("ObjectEnumResponse::AddFragment: Fragment contains bad result");

  if (   (!indices_.empty())
      && (!fragment.indices_.empty())
      && (indices_.back() >= fragment.indices_.front()))
  {
    throw std::invalid_argument("ObjectEnumResponse::AddFragment: Discontinuity");
  }

  // determine new size and reserve storage
  size_t const newSize = indices_.size() + fragment.indices_.size();
  if (newSize > maxNbOfIndices_)
    throw std::logic_error("ObjectEnumResponse::AddFragment");

  indices_.reserve(newSize);

  // prepare for rollback
  size_t const prevSize = indices_.size();
  ON_SCOPE_EXIT(undo) { indices_.resize(prevSize); };

  indices_.insert(indices_.end(), fragment.indices_.begin(), fragment.indices_.end());

  ON_SCOPE_EXIT_DISMISS(undo);

  complete_ = fragment.complete_;
}

/**
 * \brief Retrieves an unmodifiable reference to the enumerated indices contained in the response object.
 *
 * \pre   The result of the enumeration is @ref SDOAbortCode::OK.
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
 * Unmodifiable reference to an `std::vector<uint16_t>` containing the enumerated indices encapsulated in the response
 * object.\n
 * The life-time of the referenced object is limited to the life time of the @ref ObjectEnumResponse instance.
 */
std::vector<uint16_t> const & ObjectEnumResponse::GetIndices(void) const
{
  if (result_ != SDOAbortCode::OK)
    throw std::logic_error("ObjectEnumResponse::GetIndices: Enumeration failed");

  return indices_;
}

} // namespace cood
} // namespace gpcc
