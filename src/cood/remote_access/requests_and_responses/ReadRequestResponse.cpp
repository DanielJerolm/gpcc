/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "ReadRequestResponse.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include <exception>
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const ReadRequestResponse::readRequestResponseBinarySize;

/**
 * \brief Constructor. Creates a response object with encapsulated result initialized with an error value.
 *
 * Use @ref SetError() to change the result to a different error status code at a later point in time.\n
 * Use @ref SetData() to set the result to @ref SDOAbortCode::OK and to attach the read data to the response object.
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
 * Result value that shall be encapsulated in the object.\n
 * This must be a negative status code. @ref SDOAbortCode::OK is not allowed.
 */
ReadRequestResponse::ReadRequestResponse(SDOAbortCode const _result)
: ResponseBase(ResponseTypes::readRequestResponse)
, result(_result)
, data()
, sizeInBit(0U)
{
  if (result == SDOAbortCode::OK)
    throw std::invalid_argument("ReadRequestResponse::ReadRequestResponse: Negative result expected");
}

/**
 * \brief Constructor. Creates a @ref ReadRequestResponse object from data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref ReadRequestResponse object.
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
ReadRequestResponse::ReadRequestResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ReadRequestResponsePassKey)
: ResponseBase(ResponseTypes::readRequestResponse, sr, versionOnHand)
, data()
, sizeInBit(0U)
{
  auto const result_u32 = sr.Read_uint32();
  try
  {
    result = U32ToSDOAbortCode(result_u32);
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(std::runtime_error("ReadRequestResponse::ReadRequestResponse: Data read from 'sr' is invalid"));
  }

  if (result != SDOAbortCode::OK)
    return;

  // s = number of bytes
  // b = bits used in LAST byte
  uint_fast16_t s = sr.Read_uint16();
  uint_fast8_t  b = sr.Read_uint8();

  if (s != 0U)
  {
    if ((b == 0U) || (b > 8U))
      throw std::runtime_error("ReadRequestResponse::ReadRequestResponse: Data read from 'sr' is invalid");
    sizeInBit = ((s - 1UL) * 8UL) + b;

    data.reserve(s);
    while (s != 0U)
    {
      data.push_back(sr.Read_uint8());
      --s;
    }
  }
  else
  {
    if (b != 0U)
      throw std::runtime_error("ReadRequestResponse::ReadRequestResponse: Data read from 'sr' is invalid");
  }
}

/**
 * \brief Move constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * The new @ref ReadRequestResponse will be move-constructed from the one referenced by `other`.\n
 * The other instance is left with empty return item stack and no data.
 */
ReadRequestResponse::ReadRequestResponse(ReadRequestResponse && other) noexcept
: ResponseBase(std::move(other))
, result(other.result)
, data(std::move(other.data))
, sizeInBit(other.sizeInBit)
{
  other.data.clear();
  other.sizeInBit = 0U;
}

/**
 * \brief Calculates the maximum size of data (in byte) that can be attached to a @ref ReadRequestResponse object which
 *        still meets the maximum response size.
 *
 * This is intended to be used by class @ref RemoteAccessServer to determine the maximum permitted payload size.
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
 * Maximum size of the data payload (in byte) that can be attached to the response object.\n
 * Zero, if the given `maxResponseSize` is too small.
 */
size_t ReadRequestResponse::CalcMaxDataPayload(size_t const maxResponseSize, size_t const returnStackSize) noexcept
{
  size_t maxDataPayload = maxResponseSize;

  // subtract overhead of ResponseBase and ReadRequestResponse
  size_t const binarySize = baseBinarySize + readRequestResponseBinarySize;
  static_assert(binarySize <= ResponseBase::minimumUsefulResponseSize,
                "Base size of serialized ReadRequestResponse exceeds ResponseBase::minimumUsefulResponseSize");

  if (maxDataPayload <= binarySize)
    return 0U;
  else
    maxDataPayload -= binarySize;

  // subtract overhead of ReturnStackItem
  if (maxDataPayload <= returnStackSize)
    return 0U;
  else
    maxDataPayload -= returnStackSize;

  // limit to u16
  if (maxDataPayload > std::numeric_limits<uint16_t>::max())
    maxDataPayload = std::numeric_limits<uint16_t>::max();

  // that's it
  return maxDataPayload;
}

// <-- ResponseBase

/// \copydoc gpcc::cood::ResponseBase::GetBinarySize
size_t ReadRequestResponse::GetBinarySize(void) const
{
  size_t s = ResponseBase::GetBinarySize();

  s += 4U;
  if (result != SDOAbortCode::OK)
    return s;

  s += 3U;
  s += data.size();
  return s;
}

/// \copydoc gpcc::cood::ResponseBase::ToBinary
void ReadRequestResponse::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  ResponseBase::ToBinary(sw);

  sw.Write_uint32(static_cast<uint32_t>(result));

  if (result != SDOAbortCode::OK)
    return;

  // number of bytes
  sw.Write_uint16(static_cast<uint16_t>(data.size()));

  // number of bits in LAST byte
  if (data.empty())
  {
    sw.Write_uint8(0U);
  }
  else
  {
    uint8_t b = sizeInBit % 8U;
    if (b == 0U)
      b = 8U;
    sw.Write_uint8(b);
  }

  // data
  sw.Write_uint8(data.data(), data.size());
}

/// \copydoc gpcc::cood::ResponseBase::ToString
std::string ReadRequestResponse::ToString(void) const
{
  std::ostringstream s;
  s << "Read request response: " << SDOAbortCodeToDescrString(result);

  if (result == SDOAbortCode::OK)
  {
    s << ", " << (sizeInBit / 8U) << '.' << (sizeInBit % 8U) << " byte(s) of data";

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
  }

  return s.str();
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
 * \param _result
 * Desired value for the encapsulated result.\n
 * An error status is expected. @ref SDOAbortCode::OK is not allowed.
 */
void ReadRequestResponse::SetError(SDOAbortCode const _result)
{
  if (_result == SDOAbortCode::OK)
    throw std::invalid_argument("ReadRequestResponse::SetError: Negative result expected");

  result = _result;
  data.clear();
  sizeInBit = 0U;
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
 * \param _data
 * The content of the referenced vector will be moved into the response object.\n
 * The referenced vector will be empty afterwards.
 *
 * \param _sizeInBit
 * Size of the data in bit.
 */
void ReadRequestResponse::SetData(std::vector<uint8_t> && _data, size_t const _sizeInBit)
{
  if (_data.size() > std::numeric_limits<uint16_t>::max())
    throw std::invalid_argument("ReadRequestResponse::SetData: _data too large.");

  if (_data.size() != ((_sizeInBit + 7U) / 8U))
    throw std::invalid_argument("ReadRequestResponse::SetData: _data and _sizeInBit do not match.");

  data = std::move(_data);
  _data.clear();

  sizeInBit = _sizeInBit;

  result = SDOAbortCode::OK;
}

/**
 * \brief Retrieves the encapsulated result value.
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
 * The encapsulated result value.
 */
SDOAbortCode ReadRequestResponse::GetResult(void) const noexcept
{
  return result;
}

/**
 * \brief Retrieves the size of the encapsulated data in bit.
 *
 * \pre   The encapsulated result is @ref SDOAbortCode::OK.
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
 * Size of the encapsulated data in bit.
 */
size_t ReadRequestResponse::GetDataSize(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ReadRequestResponse::GetDataSize: Read failed");

  return sizeInBit;
}

/**
 * \brief Retrieves an unmodifiable reference to the data contained in the response object.
 *
 * \pre   The encapsulated result is @ref SDOAbortCode::OK.
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
 * Unmodifiable reference to an `std::vector<uint8_t>` containing the data encapsulated in the response object.\n
 * The life-time of the referenced object is limited to the life time of the @ref ReadRequestResponse instance.\n
 * The exact size of the data in bit can be queried via @ref GetDataSize(). Upper bits of the last byte may be unused.
 */
std::vector<uint8_t> const & ReadRequestResponse::GetData(void) const
{
  if (result != SDOAbortCode::OK)
    throw std::logic_error("ReadRequestResponse::GetData: Read failed");

  return data;
}

} // namespace cood
} // namespace gpcc
