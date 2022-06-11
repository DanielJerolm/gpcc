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

#ifndef IREMOTEOBJECTDICTIONARYACCESS_HPP_202007021858
#define IREMOTEOBJECTDICTIONARYACCESS_HPP_202007021858

#include <memory>

namespace gpcc {
namespace cood {

class IRemoteObjectDictionaryAccessNotifiable;
class RequestBase;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_ITF
 * \brief Interface offering remote access to an @ref ObjectDictionary.
 *
 * This is used in conjunction with the @ref IRemoteObjectDictionaryAccessNotifiable interface.\n
 * The pair of the two interfaces is abbreviated using the term RODA/RODAN.\n
 * For details please refer to @ref GPCC_COOD_REMOTEACCESS_ITF.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IRemoteObjectDictionaryAccess
{
  public:
    virtual void Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable) = 0;
    virtual void Unregister(void) noexcept = 0;
    virtual void Send(std::unique_ptr<RequestBase> & spReq) = 0;

    virtual void RequestExecutionContext(void) = 0;

  protected:
    IRemoteObjectDictionaryAccess(void) = default;
    IRemoteObjectDictionaryAccess(IRemoteObjectDictionaryAccess const &) = default;
    IRemoteObjectDictionaryAccess(IRemoteObjectDictionaryAccess &&) noexcept = default;

    virtual ~IRemoteObjectDictionaryAccess(void) = default;

    IRemoteObjectDictionaryAccess& operator=(IRemoteObjectDictionaryAccess const &) = default;
    IRemoteObjectDictionaryAccess& operator=(IRemoteObjectDictionaryAccess &&) noexcept = default;
};

/**
 * \fn IRemoteObjectDictionaryAccess::Register
 * \brief Registers an [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 *        interface.
 *
 * In addition to registration of the interface, some implementations of this method may also use the calling context to
 * perform synchronous I/O operations (e.g. operations on device drivers or network sockets).
 *
 * \pre   There is no [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 *        interface registered yet.
 *
 * \post  [IRemoteObjectDictionaryAccessNotifiable::OnReady](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 *        will be invoked when this interface is ready to accept remote access requests.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param pNotifiable
 * [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable) interface of the
 * client who wants to use this interface.\n
 * nullptr is not allowed.\n
 * __Note:__ Methods of the referenced notifiable-interface may be invoked even before this method returns.
 */

/**
 * \fn IRemoteObjectDictionaryAccess::Unregister
 * \brief Unregisters the [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 *        interface previously registered via [Register()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Register).
 *
 * This method guarantees, that the unregistered interface will not be invoked any more after this method has returned.
 * It blocks until a potential call to the interface that shall be unregistered has completed. If there is no interface
 * registered, then this method will have no effect.
 *
 * Additionally some implementations of this method may also block until I/O operations (e.g. operations on device
 * drivers or network sockets) have completed.
 *
 * __Outstanding requests and responses:__\n
 * Any pending responses, which have not yet been delivered to the notifiable interface will be dropped. Pending
 * responses will also not be delivered if a
 * [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable) interface is
 * registered immediately after this method has returned.
 *
 * Any pending requests for object dictionary access may or may not be executed. Pending request may even be executed
 * after unregistration of the client has completed. However, any responses associated with these requests will be
 * dropped in any case.
 *
 * Any pending requests for calling [IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext()]
 * (@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext) will also be dropped.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * No more than one thread is allowed to execute this at any time.
 *
 * __Exception safety:__\n
 * No-throw guarantee required.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */

/**
 * \fn IRemoteObjectDictionaryAccess::Send
 * \brief Sends a remote access request to the remote access server.
 *
 * The request will be processed asynchronously. The response will be delivered to the registered
 * [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable) interface.
 *
 * Please note the chapter "RODA/RODAN interface policies" in @ref GPCC_COOD_REMOTEACCESS_ITF.
 *
 * \pre   A [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 *        interface has been registered via [Register()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Register).
 *
 * \pre   The remote access server and the connection to it is ready. The ready-state can be oberserved via
 *        [IRemoteObjectDictionaryAccessNotifiable::OnReady()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady)
 *        and [IRemoteObjectDictionaryAccessNotifiable::OnDisconnected()]
 *        (@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnDisconnected).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc                    Out of memory.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * \throws RequestTooLargeError              The size of the serialized request object exceeds the maximum size for
 *                                           requests permitted by this interface ([details](@ref gpcc::cood::RequestTooLargeError)).
 *
 * \throws ResponseTooLargeError             The `maxResponseSize` attribute of the request object exceeds the maximum
 *                                           size for responses permitted by this interface ([details](@ref gpcc::cood::ResponseTooLargeError)).
 *
 * \throws MinimumResponseSizeNotMetError    The `maxResponseSize` attribute of the request object minus the size of
 *                                           its stack of [ReturnStackItem](@ref gpcc::cood::ReturnStackItem) objects
 *                                           is less than the minimum useful reponse size
 *                                           ([details](@ref gpcc::cood::MinimumResponseSizeNotMetError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param spReq
 * Reference to a unique pointer to the remote access request object. nullptr is not allowed.\n
 * In case of success (no exception thrown and no deferred thread cancellation), ownership will move to the
 * @ref IRemoteObjectDictionaryAccess interface. In case of failure or deferred thread cancellation, ownership remains
 * at the referenced unique pointer.
 */

/**
 * \fn IRemoteObjectDictionaryAccess::RequestExecutionContext
 * \brief Requests invocation of [IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext).
 *
 * If a request is already pending, then this method has no effect.
 *
 * If invoked from within [IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext),
 * then another invocation of [IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext)
 * will occur.
 *
 * \pre   A [IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)
 *        interface has been registered via [Register()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Register).
 *
 * \pre   The remote access server and the connection to it is ready. The ready-state can be oberserved via
 *        [IRemoteObjectDictionaryAccessNotifiable::OnReady()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady)
 *        and [IRemoteObjectDictionaryAccessNotifiable::OnDisconnected()]
 *        (@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnDisconnected).
 *
 * \post  [IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext)
 *        will be invoked. The exact point in time when the method will be invoked is random and completely asynchronous
 *        to this call.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws RemoteAccessServerNotReadyError   The RODA interface is not in ready-state
 *                                           ([details](@ref gpcc::cood::RemoteAccessServerNotReadyError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */

} // namespace cood
} // namespace gpcc

#endif // IREMOTEOBJECTDICTIONARYACCESS_HPP_202007021858