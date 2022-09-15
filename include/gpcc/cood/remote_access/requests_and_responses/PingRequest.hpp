/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef PINGREQUEST_HPP_202108051135
#define PINGREQUEST_HPP_202108051135

#include "RequestBase.hpp"

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Key for public constructor of class @ref PingRequest (passkey-pattern).
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class PingRequestPassKey final
{
  private:
    friend class RequestBase;

    PingRequestPassKey(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Object dictionary remote access request:\n
 *        Ping.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class PingRequest final : public RequestBase
{
  public:
    PingRequest(void) = delete;

    PingRequest(size_t const _maxResponseSize);

    PingRequest(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, PingRequestPassKey);

    PingRequest(PingRequest const &) = default;
    PingRequest(PingRequest &&) noexcept = default;

    ~PingRequest(void) = default;

    PingRequest& operator=(PingRequest const &) = delete;
    PingRequest& operator=(PingRequest &&) = delete;

    // <-- RequestBase
    size_t GetBinarySize(void) const override;
    void ToBinary(gpcc::stream::IStreamWriter & sw) const override;

    std::string ToString(void) const override;
    // --> RequestBase
};

} // namespace cood
} // namespace gpcc

#endif // PINGREQUEST_HPP_202108051135
