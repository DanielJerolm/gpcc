/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#include "WriteRequestResponse.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
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
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref WriteRequestResponse object.
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
WriteRequestResponse::WriteRequestResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, WriteRequestResponsePassKey)
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
void WriteRequestResponse::ToBinary(gpcc::Stream::IStreamWriter & sw) const
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
