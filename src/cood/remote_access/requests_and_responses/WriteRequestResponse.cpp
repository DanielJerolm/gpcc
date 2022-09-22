/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <exception>
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const WriteRequestResponse::writeRequestResponseBinarySize;

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
 * \param _result
 * Result value that shall be encapsulated in the object.
 */
WriteRequestResponse::WriteRequestResponse(SDOAbortCode const _result)
: ResponseBase(ResponseTypes::writeRequestResponse)
, result(_result)
{
}

/**
 * \brief Constructor. Creates a @ref WriteRequestResponse object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref WriteRequestResponse object.
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
WriteRequestResponse::WriteRequestResponse(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, WriteRequestResponsePassKey)
: ResponseBase(ResponseTypes::writeRequestResponse, sr, versionOnHand)
{
  auto const result_u32 = sr.Read_uint32();
  try
  {
    result = U32ToSDOAbortCode(result_u32);
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(std::runtime_error("WriteRequestResponse::WriteRequestResponse: Data read from 'sr' is invalid"));
  }
}

// <-- ResponseBase

/// \copydoc gpcc::cood::ResponseBase::GetBinarySize
size_t WriteRequestResponse::GetBinarySize(void) const
{
  return ResponseBase::GetBinarySize() + writeRequestResponseBinarySize;
}

/// \copydoc gpcc::cood::ResponseBase::ToBinary
void WriteRequestResponse::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  ResponseBase::ToBinary(sw);

  sw.Write_uint32(static_cast<uint32_t>(result));
}

/// \copydoc gpcc::cood::ResponseBase::ToString
std::string WriteRequestResponse::ToString(void) const
{
  std::ostringstream s;
  s << "Write request response: " << SDOAbortCodeToDescrString(result);

  return s.str();
}

// --> ResponseBase

} // namespace cood
} // namespace gpcc
