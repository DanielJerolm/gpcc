/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef WRITEREQUESTRESPONSE_HPP_202006292122
#define WRITEREQUESTRESPONSE_HPP_202006292122

#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/sdo_abort_codes.hpp>

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

    WriteRequestResponse(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, WriteRequestResponsePassKey);

    WriteRequestResponse(WriteRequestResponse const &) = default;
    WriteRequestResponse(WriteRequestResponse &&) = default;

    ~WriteRequestResponse(void) = default;

    WriteRequestResponse& operator=(WriteRequestResponse const &) = delete;
    WriteRequestResponse& operator=(WriteRequestResponse &&) = delete;

    // <-- ResponseBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::stream::IStreamWriter & sw) const override;

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
