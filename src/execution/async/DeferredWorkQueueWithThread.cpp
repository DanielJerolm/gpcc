/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#include <gpcc/execution/async/DeferredWorkQueueWithThread.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/string/tools.hpp>
#include <functional>

namespace gpcc      {
namespace execution {
namespace async     {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param threadName
 * Name for the thread that will run the work queue.
 */
DeferredWorkQueueWithThread::DeferredWorkQueueWithThread(std::string const & threadName)
: dwq()
, thread(threadName)
{
}

/**
 * \brief Starts the thread.
 *
 * \pre   The thread is not running.
 *
 * \post  The thread is running and processes work packages from the work queue.
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
 *
 * - - -
 *
 * \param schedPolicy
 * [Scheduling policy](@ref gpcc::osal::Thread::SchedPolicy) that shall be applied to the thread.
 *
 * \param priority
 * Priority level for the thread: 0 (low) .. 31 (high)\n
 * This is only relevant for the scheduling policies [SchedPolicy::Fifo](@ref gpcc::osal::Thread::SchedPolicy::Fifo)
 * and [SchedPolicy::RR](@ref gpcc::osal::Thread::SchedPolicy::RR).\n
 * _For the other scheduling policies this parameter is not relevant and must be zero._
 *
 * \param stackSize
 * Size of the stack of the thread in byte.\n
 * _This must be a multiple of_ @ref gpcc::osal::Thread::GetStackAlign(). \n
 * _This must be equal to or larger than_ @ref gpcc::osal::Thread::GetMinStackSize(). \n
 * On some platforms the final stack size might be larger than this, e.g. due to interrupt handling requirements.
 */
void DeferredWorkQueueWithThread::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                                        gpcc::osal::Thread::priority_t const priority,
                                        size_t const stackSize)
{
  thread.Start(std::bind(&DeferredWorkQueueWithThread::ThreadEntry, this), schedPolicy, priority, stackSize);
}

/**
 * \brief Stops the thread.
 *
 * If a work package is in progress, then the thread will be stopped after the work package has completed. If the
 * work queue is empty, then the thread will stop immediately.
 *
 * Enqueued work packages, that have not been processed yet remain in the work queue and are not removed by the
 * stop operation.
 *
 * \pre   The thread is running.
 *
 * \post  The thread is not running.
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
 */
void DeferredWorkQueueWithThread::Stop(void) noexcept
{
  try
  {
    dwq.RequestTermination();
    thread.Join();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("DeferredWorkQueueWithThread::Stop: Failed: ", e);
  }
}

/**
 * \brief Entry function for the thread running the work queue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures that there is only one thread per @ref DeferredWorkQueueWithThread instance executing this.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation will be disabled by this.
 *
 * - - -
 *
 * \return
 * Value returned by Thread::Join(). Here: Always nullptr.
 */
void* DeferredWorkQueueWithThread::ThreadEntry(void)
{
  thread.SetCancelabilityEnabled(false);

  try
  {
    dwq.Work();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("DeferredWorkQueueWithThread::ThreadEntry: A work package threw: ", e);
  }
  catch (...)
  {
    gpcc::osal::Panic("DeferredWorkQueueWithThread::ThreadEntry: Caught an unknown exception thrown by a work package.");
  }

  return nullptr;
}

} // namespace async
} // namespace execution
} // namespace gpcc
