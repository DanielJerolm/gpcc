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

#ifndef WRITEREQUESTRESPONSE_HPP_202006292122
#define WRITEREQUESTRESPONSE_HPP_202006292122

#include "ResponseBase.hpp"
#include "gpcc/src/cood/sdo_abort_codes.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref WriteRequestResponse (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class WriteRequestResponsePassKey final
{
  private:
    friend class ResponseBase;

    WriteRequestResponsePassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request response:\n
 *        Single subindex or complete access write request response.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class WriteRequestResponse final : public ResponseBase
{
  public:
    WriteRequestResponse(void) = delete;

    explicit WriteRequestResponse(SDOAbortCode const _result);

    WriteRequestResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, WriteRequestResponsePassKey);

    WriteRequestResponse(WriteRequestResponse const &) = default;
    WriteRequestResponse(WriteRequestResponse &&) = default;

    ~WriteRequestResponse(void) = default;

    WriteRequestResponse& operator=(WriteRequestResponse const &) = delete;
    WriteRequestResponse& operator=(WriteRequestResponse &&) = delete;

    // <-- ResponseBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::Stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> ResponseBase

    // for server
    void SetResult(SDOAbortCode const _result) noexcept;

    // for originator of request
    SDOAbortCode GetResult(void) const noexcept;

  private:
    /// Binary size of a serialized @ref WriteRequestResponse object (excl. base class @ref ResponseBase).
    static size_t const writeRequestResponseBinarySize = 4U;

    /// Result of the write request.
    SDOAbortCode result;
};

/**
 * \brief Sets the encapsulated result value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _result
 * Desired value for the encapsulated result value.
 */
inline void WriteRequestResponse::SetResult(SDOAbortCode const _result) noexcept
{
  result = _result;
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
inline SDOAbortCode WriteRequestResponse::GetResult(void) const noexcept
{
  return result;
}

} // namespace cood
} // namespace gpcc

#endif // WRITEREQUESTRESPONSE_HPP_202006292122
