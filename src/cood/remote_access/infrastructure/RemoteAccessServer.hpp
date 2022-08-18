/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef REMOTEACCESSSERVER_HPP_202006271720
#define REMOTEACCESSSERVER_HPP_202006271720

#include "gpcc/src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/container/IntrusiveDList.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <vector>

namespace gpcc {
namespace log  {
  class Logger;
}
}

namespace gpcc {
namespace cood {

class IObjectAccess;
class ObjectEnumRequest;
class ObjectInfoRequest;
class PingRequest;
class ReadRequest;
class RequestBase;
class ResponseBase;
class WriteRequest;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Base class for servers executing remote access requests to an @ref ObjectDictionary.
 *
 * This class offers a RODA/RODAN interface pair (see @ref GPCC_COOD_REMOTEACCESS_ITF) and executes all incoming
 * requests on the object dictionary referenced by the given @ref IObjectAccess interface.
 *
 * This class cannot be instantiated directly. Instead instantiate one of the derived classes which offer an
 * execution context for the server. The different derived classes offer different kinds of execution context.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class RemoteAccessServer : public IRemoteObjectDictionaryAccess
{
  public:
    RemoteAccessServer(void) = delete;
    RemoteAccessServer(RemoteAccessServer const &) = delete;
    RemoteAccessServer(RemoteAccessServer &&) = delete;

    RemoteAccessServer& operator=(RemoteAccessServer const &) = delete;
    RemoteAccessServer& operator=(RemoteAccessServer &&) = delete;

    // <-- IRemoteObjectDictionaryAccess
    void Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable) override;
    void Unregister(void) noexcept override;
    void Send(std::unique_ptr<RequestBase> & spReq) override;

    void RequestExecutionContext(void) override;
    // --> IRemoteObjectDictionaryAccess

  protected:
    /// Logger that shall be used by the remote access server and derived classes to log messages.
    /** This is nullptr if logging is not required. */
    gpcc::log::Logger * const pLogger;


    RemoteAccessServer(IObjectAccess & _od,
                       gpcc::log::Logger * const _pLogger,
                       size_t const _maxRequestSize,
                       size_t const _maxResponseSize);
    virtual ~RemoteAccessServer(void);

    virtual void RequestWorkInvocationHook(void) = 0;

    void OnStart(void);
    void Work(void);
    void OnStop(void);

  private:
    /// States of the server.
    enum class States
    {
      UnregisteredAndOff,   ///<No client registered and server off (stopped).
      UnregisteredAndIdle,  ///<No client registered, but server on (started/running).
      off,                  ///<Client registered, but server off (stopped).
      justRegistered,       ///<Client just registered (OnReady() not yet delivered), server on (started/running).
      idle,                 ///<Client registered, server on (started/running), queue empty.
      processing            ///<Client registered, server on (started/running), queue not empty.
    };


    /// Maximum size (in byte) of a serialized request object (incl. any @ref ReturnStackItem objects) that can be
    /// processed by the server.
    /** The value is configured via the constructor. It will be passed to the OnReady(...)-callback of any client
        registering at the provided @ref IRemoteObjectDictionaryAccess interface. */
    size_t const maxRequestSize;

    /// Maximum size (in byte) of a serialized response object (incl. any @ref ReturnStackItem objects) that can be
    /// created by the server and send back to the client.
    /** The value is configured via the constructor. It will be passed to the OnReady(...)-callback of any client
        registering at the provided @ref IRemoteObjectDictionaryAccess interface. */
    size_t const maxResponseSize;

    /// Interface for accessing the object dictionary.
    IObjectAccess & od;

    /// Mutex used to make registration and unregistration of the client thread-safe.
    /** Locking order: @ref clientMutex -> @ref apiMutex. */
    gpcc::osal::Mutex clientMutex;

    /// Mutex used to make the API thread-safe.
    /** Locking order: @ref clientMutex -> @ref apiMutex. */
    gpcc::osal::Mutex apiMutex;

    /// Flag used to prevent @ref Work() from processing requests if a thread has entered @ref Unregister().
    /** If the server's thread has higher priority than the thread blocked in @ref Unregister(), then this flag
        ensures that the thread that has invoked @ref Unregister() can proceed. */
    std::atomic<bool> unregisterPending;

    /// Current state of the server.
    /** @ref apiMutex required. */
    States state;

    /// Notifiable interface of registered client. nullptr = no client registered.
    /** @ref clientMutex required. */
    IRemoteObjectDictionaryAccessNotifiable * pClient;

    /// Queue for incoming requests.
    /** @ref apiMutex required. */
    gpcc::container::IntrusiveDList<RequestBase> queue;

    /// Flag indicating if a request for invocation of
    /// @ref IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext() is pending.
    /** @ref apiMutex required. */
    bool loanExecContextRequested;

    /// Temporary storage for return stack used by @ref MoveReturnStack().
    /** No mutex required, this is exclusively used by @ref MoveReturnStack(). */
    std::vector<ReturnStackItem> temporaryReturnStack;


    void SanityCheck(RequestBase const & request) const;

    void ServeRequest(RequestBase & request);
    void ServeObjectEnumRequest(ObjectEnumRequest & request);
    void ServeObjectInfoRequest(ObjectInfoRequest & request);
    void ServePingRequest(PingRequest & request);
    void ServeReadRequest(ReadRequest & request);
    void ServeWriteRequest(WriteRequest & request);

    void LogErrorWhileServingRequest(RequestBase const & request) noexcept;
    void MoveReturnStack(RequestBase & src, ResponseBase & dest) noexcept;
};


/**
 * \fn void RemoteAccessServer::RequestWorkInvocationHook
 * \brief Requests invocation of [Work()](@ref gpcc::cood::RemoteAccessServer::Work) by the derived class.
 *
 * This may be invoked in the context of any thread, especially from [Work()](@ref gpcc::cood::RemoteAccessServer::Work)
 * and [OnStart()](@ref gpcc::cood::RemoteAccessServer::OnStart).
 *
 * If invoked in the context of [Work()](@ref gpcc::cood::RemoteAccessServer::Work), then
 * [Work()](@ref gpcc::cood::RemoteAccessServer::Work) shall be invoked again after the call to
 * [Work()](@ref gpcc::cood::RemoteAccessServer::Work) has returned.
 *
 * This will not be invoked from [OnStop()](@ref gpcc::cood::RemoteAccessServer::OnStop).
 *
 * If a request is already pending, then this shall have no effect.
 *
 * If the derived class is _not in running state_, then the derived class is allowed to ignore the request. However,
 * serving the request when the derived class eventually returns to the _running state_ is not harmful since
 * [Work()](@ref gpcc::cood::RemoteAccessServer::Work) is aware of spurious calls.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */

} // namespace cood
} // namespace gpcc

#endif // REMOTEACCESSSERVER_HPP_202006271720
