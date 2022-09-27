/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef EXCEPTIONS_HPP_202007140835
#define EXCEPTIONS_HPP_202007140835

#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_EXCEPTIONS
 * \brief Exception thrown if the @ref IRemoteObjectDictionaryAccess interface is not ready to process a request.
 *
 * Potential reasons:
 * - The connection to the @ref RemoteAccessServer is broken (e.g. serial link or network disconnected or broken IPC)
 * - The @ref RemoteAccessServer is not running.
 * - Client did not wait for @ref IRemoteObjectDictionaryAccessNotifiable::OnReady().
 * - The client is about to receive a call to @ref IRemoteObjectDictionaryAccessNotifiable::OnDisconnected().
 *
 * This is not a permanent error. Connection to the server may be re-established and the
 * @ref IRemoteObjectDictionaryAccess interface may return to the ready-state. Clients should watch for
 * @ref IRemoteObjectDictionaryAccessNotifiable::OnReady() and
 * @ref IRemoteObjectDictionaryAccessNotifiable::OnDisconnected().
 *
 */
class RemoteAccessServerNotReadyError : public std::runtime_error
{
  public:
    inline RemoteAccessServerNotReadyError(void) : std::runtime_error("Remote access server not ready or disconnected.") {};
    virtual ~RemoteAccessServerNotReadyError(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_EXCEPTIONS
 * \brief Exception thrown if a request passed to @ref IRemoteObjectDictionaryAccess::Send() exceeds the maximum size
 *        for requests permitted by provider of the RODA interface.
 *
 * Potential reasons:
 * - Client did not respect the maximum permitted size for requests reported via
 *   @ref IRemoteObjectDictionaryAccessNotifiable::OnReady().
 *
 */
class RequestTooLargeError : public std::logic_error
{
  public:
    inline RequestTooLargeError(void) : std::logic_error("Size of request exceeds limit of RODA interface.") {};
    virtual ~RequestTooLargeError(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_EXCEPTIONS
 * \brief Exception thrown if the `maxResponseSize` attribute of a request passed to
 *        @ref IRemoteObjectDictionaryAccess::Send() exceeds the maximum size for responses permitted by the
 *        provider of the RODA interface.
 *
 * Potential reasons:
 * - Client did not respect the maximum permitted size for responses reported via
 *   @ref IRemoteObjectDictionaryAccessNotifiable::OnReady().
 *
 */
class ResponseTooLargeError : public std::logic_error
{
  public:
    inline ResponseTooLargeError(void) : std::logic_error("maxResponseSize-attribute of request exceeds limit of RODA interface.") {};
    virtual ~ResponseTooLargeError(void) = default;
};

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_EXCEPTIONS
 * \brief Exception thrown if the `maxResponseSize` attribute of a request passed to
 *        @ref IRemoteObjectDictionaryAccess::Send() minus the size of its stack of @ref ReturnStackItem objects
 *        (= size of the bare response) is less than the minimum useful size for response messages
 *        (@ref ResponseBase::minimumUsefulResponseSize).
 *
 * Potential reasons:
 * - Client initialized the request object with a too small value.
 *
 */
class MinimumResponseSizeNotMetError : public std::logic_error
{
  public:
    inline MinimumResponseSizeNotMetError(void) : std::logic_error("Maximum permitted response size (without RSI stack) does not meet minimum useful response size.") {};
    virtual ~MinimumResponseSizeNotMetError(void) = default;
};

} // namespace cood
} // namespace gpcc

#endif // EXCEPTIONS_HPP_202007140835
