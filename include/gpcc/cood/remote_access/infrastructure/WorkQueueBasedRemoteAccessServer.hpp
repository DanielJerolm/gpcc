/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef WORKQUEUEBASEDREMOTEACCESSSERVER_HPP_202105170848
#define WORKQUEUEBASEDREMOTEACCESSSERVER_HPP_202105170848

#include "RemoteAccessServer.hpp"
#include <gpcc/execution/async/DeferredWorkPackage.hpp>
#include <gpcc/execution/async/IDeferredWorkQueue.hpp>
#include <gpcc/execution/async/WorkPackage.hpp>
#include <gpcc/osal/Mutex.hpp>

namespace gpcc {
namespace cood {

class IObjectAccess;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Server executing remote access requests to an @ref ObjectDictionary. This version uses an externally provided
 *        [deferred work queue](@ref gpcc::execution::async::DeferredWorkQueue) as execution context.
 *
 * For details about the functionality, please refer to base class @ref RemoteAccessServer.
 *
 * The thread executing the externally provided work queue must have deferred thread cancellation disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class WorkQueueBasedRemoteAccessServer final : public RemoteAccessServer
{
  public:
    WorkQueueBasedRemoteAccessServer(void) = delete;
    WorkQueueBasedRemoteAccessServer(gpcc::execution::async::IDeferredWorkQueue & _dwq,
                                     uint8_t const _oomRetryDelay_ms,
                                     IObjectAccess & _od,
                                     gpcc::log::Logger * const _pLogger,
                                     size_t const _maxRequestSize,
                                     size_t const _maxResponseSize);

    WorkQueueBasedRemoteAccessServer(WorkQueueBasedRemoteAccessServer const &) = delete;
    WorkQueueBasedRemoteAccessServer(WorkQueueBasedRemoteAccessServer &&) = delete;

    ~WorkQueueBasedRemoteAccessServer(void);

    WorkQueueBasedRemoteAccessServer& operator=(WorkQueueBasedRemoteAccessServer const &) = delete;
    WorkQueueBasedRemoteAccessServer& operator=(WorkQueueBasedRemoteAccessServer &&) = delete;

    void Start(void);
    void Stop(void) noexcept;

  private:
    /// States of the @ref WorkQueueBasedRemoteAccessServer.
    enum class States
    {
      Off,                  ///<Off.
      Starting,             ///<Starting. @ref wp is scheduled, RemoteAccessServer::OnStart() will be executed.
      On,                   ///<On, but no request for invocation of RemoteAccessServer::Work() is pending.
      InvocationRequested,  ///<On. @ref wp is scheduled, RemoteAccessServer::Work() will be invoked.
      RetryInvocation,      ///<On. @ref dwp is scheduled, invocation of RemoteAccessServer::Work() will be retried.
      Stopping              ///<Stopping. @ref wp is scheduled, RemoteAccessServer::OnStop() will be executed.
    };


    /// Externally provided deferred work queue used as execution context for this class.
    gpcc::execution::async::IDeferredWorkQueue & dwq;

    /// Delay in ms before retry after an out-of-memory related error.
    uint8_t const oomRetryDelay_ms;

    /// Mutex used to make @ref Start() and @ref Stop() thread-safe.
    /** Locking order: @ref startStopMutex -> @ref internalMutex */
    gpcc::osal::Mutex startStopMutex;

    /// Mutex used to make internals thread-safe.
    /** Locking order: @ref startStopMutex -> @ref internalMutex */
    gpcc::osal::Mutex internalMutex;

    /// State of the @ref WorkQueueBasedRemoteAccessServer.
    /** @ref internalMutex is required. */
    States state;

    /// Work package used for regular invocation of @ref WQentry().
    gpcc::execution::async::WorkPackage wp;

    /// Work package used for delayed invocation of @ref WQentry().
    gpcc::execution::async::DeferredWorkPackage dwp;


    void WQentry(void) noexcept;
    void ServeRequest(void);

    // <-- RemoteAccessServer
    void RequestWorkInvocationHook(void) override;
    // --> RemoteAccessServer
};

} // namespace cood
} // namespace gpcc

#endif // WORKQUEUEBASEDREMOTEACCESSSERVER_HPP_202105170848
