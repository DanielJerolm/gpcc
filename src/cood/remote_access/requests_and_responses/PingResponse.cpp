/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/PingResponse.hpp>
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include <stdexcept>

namespace gpcc {
namespace cood {

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
 */
PingResponse::PingResponse(void)
: ResponseBase(ResponseTypes::pingResponse)
{
}

/**
 * \brief Constructor. Creates a @ref PingResponse object from data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref PingResponse object.
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
PingResponse::PingResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, PingResponsePassKey)
: ResponseBase(ResponseTypes::pingResponse, sr, versionOnHand)
{
}

// <-- ResponseBase

/// \copydoc gpcc::cood::ResponseBase::GetBinarySize
size_t PingResponse::GetBinarySize(void) const
{
  return ResponseBase::GetBinarySize();
}

/// \copydoc gpcc::cood::ResponseBase::ToBinary
void PingResponse::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  ResponseBase::ToBinary(sw);
}

/// \copydoc gpcc::cood::ResponseBase::ToString
std::string PingResponse::ToString(void) const
{
  return "Ping response";
}

// --> ResponseBase

} // namespace cood
} // namespace gpcc
