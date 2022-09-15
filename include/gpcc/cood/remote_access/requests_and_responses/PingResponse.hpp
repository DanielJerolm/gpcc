/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef PINGRESPONSE_HPP_202108052133
#define PINGRESPONSE_HPP_202108052133

#include "ResponseBase.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref PingResponse (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class PingResponsePassKey final
{
  private:
    friend class ResponseBase;

    PingResponsePassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request response:\n
 *        Ping response.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class PingResponse final : public ResponseBase
{
  public:
    PingResponse(void);
    PingResponse(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, PingResponsePassKey);

    PingResponse(PingResponse const &) = default;
    PingResponse(PingResponse &&) = default;

    ~PingResponse(void) = default;

    PingResponse& operator=(PingResponse const &) = delete;
    PingResponse& operator=(PingResponse &&) = delete;

    // <-- ResponseBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> ResponseBase
};

} // namespace cood
} // namespace gpcc

#endif // PINGRESPONSE_HPP_202108052133
