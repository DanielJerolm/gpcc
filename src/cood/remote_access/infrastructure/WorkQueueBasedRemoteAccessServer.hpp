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

#ifndef WORKQUEUEBASEDREMOTEACCESSSERVER_HPP_202105170848
#define WORKQUEUEBASEDREMOTEACCESSSERVER_HPP_202105170848

#include "RemoteAccessServer.hpp"
#include "gpcc/src/execution/async/DeferredWorkPackage.hpp"
#include "gpcc/src/execution/async/IDeferredWorkQueue.hpp"
#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/osal/Mutex.hpp"

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
