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
