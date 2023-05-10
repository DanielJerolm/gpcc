/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#ifndef DEFERREDWORKQUEUEWITHTTHREAD_HPP_202305041946
#define DEFERREDWORKQUEUEWITHTTHREAD_HPP_202305041946

#include <gpcc/execution/async/DeferredWorkQueue.hpp>
#include <gpcc/osal/Thread.hpp>
#include <string>
#include <cstdint>

namespace gpcc      {
namespace execution {
namespace async     {

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Provides a @ref DeferredWorkQueue and one [Thread](@ref gpcc::osal::Thread) driving the work queue.
 *
 * This is a convenient class for clients. Each work queue usually also requires a thread. Using this class, clients do
 * not need to setup a thread themselves.
 *
 * This class does not expect work packages to throw. If a work package throws, then this class will
 * [panic](@ref GPCC_OSAL_PANIC).
 *
 * The work packages are executed with deferred thread cancellation disabled. The @ref Stop() method will stop the
 * thread after a work package currently in progress has completed. Until then the @ref Stop() method blocks.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class DeferredWorkQueueWithThread final
{
  public:
    DeferredWorkQueueWithThread(void) = delete;
    DeferredWorkQueueWithThread(std::string const & threadName);
    DeferredWorkQueueWithThread(DeferredWorkQueueWithThread const &) = delete;
    DeferredWorkQueueWithThread(DeferredWorkQueueWithThread&&) = delete;
    ~DeferredWorkQueueWithThread(void) = default;

    DeferredWorkQueueWithThread& operator=(DeferredWorkQueueWithThread const &) = delete;
    DeferredWorkQueueWithThread& operator=(DeferredWorkQueueWithThread&&) = delete;

    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

    DeferredWorkQueue& GetDWQ(void) noexcept;

  private:
    /// Deferred work queue instance.
    DeferredWorkQueue dwq;

    /// Thread used to drive @ref dwq.
    gpcc::osal::Thread thread;


    void* ThreadEntry(void);
};

/**
 * \fn gpcc::execution::async::DeferredWorkQueueWithThread::~DeferredWorkQueueWithThread(void)
 * \brief Destructor.
 *
 * Any dynamic work packages that are still enqueued in the work queue will be released.\n
 * Any static work packages that are still enqueued in the work queue will be removed from the work queue.
 *
 * \pre   The thread must not be running.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */

/**
 * \brief Retrieves a reference to the encapsulated @ref DeferredWorkQueue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * Reference to the encapsulated  @ref DeferredWorkQueue instance.\n
 * The object's life-span is coupled to the life-span of the @ref DeferredWorkQueueWithThread instance.
 */
inline DeferredWorkQueue& DeferredWorkQueueWithThread::GetDWQ(void) noexcept
{
  return dwq;
}

} // namespace async
} // namespace execution
} // namespace gpcc

#endif // DEFERREDWORKQUEUEWITHTTHREAD_HPP_202305041946
