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

#ifndef THREADBASEDREMOTEACCESSSERVER_HPP_202007041301
#define THREADBASEDREMOTEACCESSSERVER_HPP_202007041301

#include "RemoteAccessServer.hpp"
#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <string>

namespace gpcc {
namespace cood {

class IObjectAccess;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Server executing remote access requests to an @ref ObjectDictionary. This version uses an own thread as
 *        execution context.
 *
 * For details about the functionality, please refer to base class @ref RemoteAccessServer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ThreadBasedRemoteAccessServer final : public RemoteAccessServer
{
  public:
    ThreadBasedRemoteAccessServer(void) = delete;
    ThreadBasedRemoteAccessServer(std::string const & threadName,
                                  uint8_t const _oomRetryDelay_ms,
                                  IObjectAccess & _od,
                                  gpcc::log::Logger * const _pLogger,
                                  size_t const _maxRequestSize,
                                  size_t const _maxResponseSize);

    ThreadBasedRemoteAccessServer(ThreadBasedRemoteAccessServer const &) = delete;
    ThreadBasedRemoteAccessServer(ThreadBasedRemoteAccessServer &&) = delete;

    ~ThreadBasedRemoteAccessServer(void);

    ThreadBasedRemoteAccessServer& operator=(ThreadBasedRemoteAccessServer const &) = delete;
    ThreadBasedRemoteAccessServer& operator=(ThreadBasedRemoteAccessServer &&) = delete;

    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t  const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

  private:
    /// Thread used as execution context for this class.
    gpcc::osal::Thread thread;

    /// Delay in ms before retry after an out-of-memory related error.
    uint8_t const oomRetryDelay_ms;

    /// Mutex used to make @ref Start() and @ref Stop() thread-safe.
    /** Locking order: @ref startStopMutex -> @ref internalMutex */
    gpcc::osal::Mutex startStopMutex;

    /// Mutex used to make internals thread-safe.
    /** Locking order: @ref startStopMutex -> @ref internalMutex */
    gpcc::osal::Mutex internalMutex;

    /// Flag indicating if the remote access server is running.
    /** @ref startStopMutex is required. */
    bool running;

    /// Flag indicating if invocation of @ref RemoteAccessServer::Work() has been requested.
    /** @ref internalMutex is required. */
    bool invokeWorkRequestPending;

    /// Condition-variable used to indicate assertion of @ref invokeWorkRequestPending or a thread cancellation request.
    /** This shall be used in conjunction with @ref internalMutex.*/
    gpcc::osal::ConditionVariable cvInvokeWorkRequestPending;


    void* ThreadEntry(void) noexcept;
    void ServeRequests(void);

    // <-- RemoteAccessServer
    void RequestWorkInvocationHook(void) override;
    // --> RemoteAccessServer
};

} // namespace cood
} // namespace gpcc

#endif // THREADBASEDREMOTEACCESSSERVER_HPP_202007041301
