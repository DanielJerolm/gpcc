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

#ifndef READREQUESTRESPONSE_HPP_202010101331
#define READREQUESTRESPONSE_HPP_202010101331

#include "ResponseBase.hpp"
#include "gpcc/src/cood/sdo_abort_codes.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref ReadRequestResponse (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ReadRequestResponsePassKey final
{
  private:
    friend class ResponseBase;

    ReadRequestResponsePassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request response:\n
 *        Single subindex or complete access read request response.
 *
 * # Usage
 * ## For server
 * 1. Create a @ref ReadRequestResponse instance indicating a negative status.
 * 2. Perform the read operation.
 * 3. Attach result to response object:\n
 *    _In case of success:_\n
 *    Use @ref SetData() to turn the initial negative status into @ref SDOAbortCode::OK and to attach the read
 *    data to the response object.\n
 *    _In case of error:_\n
 *    Use @ref SetError() to replace the initial negative status with a different error status if necessary.
 *
 * ## For clients
 * 1. Query result via @ref GetResult().
 * 2. In case of success, query the read data's size via @ref GetDataSize() and query the read data via @ref GetData().
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ReadRequestResponse final : public ResponseBase
{
  public:
    ReadRequestResponse(void) = delete;

    explicit ReadRequestResponse(SDOAbortCode const _result);

    ReadRequestResponse(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ReadRequestResponsePassKey);

    ReadRequestResponse(ReadRequestResponse const &) = default;
    ReadRequestResponse(ReadRequestResponse && other) noexcept;

    ~ReadRequestResponse(void) = default;

    static size_t CalcMaxDataPayload(size_t const maxResponseSize, size_t const returnStackSize) noexcept;

    ReadRequestResponse& operator=(ReadRequestResponse const &) = delete;
    ReadRequestResponse& operator=(ReadRequestResponse &&) = delete;

    // <-- ResponseBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::Stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> ResponseBase

    // for server
    void SetError(SDOAbortCode const _result);
    void SetData(std::vector<uint8_t> && _data, size_t const _sizeInBit);

    // for originator of read request
    SDOAbortCode GetResult(void) const noexcept;
    size_t GetDataSize(void) const;
    std::vector<uint8_t> const & GetData(void) const;

  private:
    /// Binary size of a serialized @ref ReadRequestResponse object with __positive__ @ref result
    /// (excl. base class @ref ResponseBase and any data).
    static size_t const readRequestResponseBinarySize = 7U;


    /// Result of the read request.
    SDOAbortCode result;

    /// Data that has been read.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    std::vector<uint8_t> data;

    /// Size of @ref data in bit.
    /** This is only valid, if @ref result is @ref SDOAbortCode::OK. */
    size_t sizeInBit;
};

} // namespace cood
} // namespace gpcc

#endif // READREQUESTRESPONSE_HPP_202010101331
