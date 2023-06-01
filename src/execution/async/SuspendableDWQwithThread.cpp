/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#include <gpcc/execution/async/SuspendableDWQwithThread.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
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
SuspendableDWQwithThread::SuspendableDWQwithThread(std::string const & threadName)
: dwq()
, thread(threadName)
, apiMutex()
, mutex()
, cvCtrlStatEvent()
, ctrlStat(CtrlStat::noThread)
{
}

/**
 * \brief Destructor.
 *
 * Any dynamic work packages that are still enqueued in the work queue will be released.\n
 * Any static work packages that are still enqueued in the work queue will be removed from the work queue.
 *
 * \pre   The thread must not be running. If required, use @ref Stop() to stop the thread.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
SuspendableDWQwithThread::~SuspendableDWQwithThread(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (ctrlStat != CtrlStat::noThread)
    gpcc::osal::Panic("~SuspendableDWQwithThread: Not stopped");
}

/**
 * \brief Starts the thread.
 *
 * \pre   The thread is not running.
 *
 * \post  The thread is running, but work package execution is suspended.\n
 *        Use @ref Resume() to start work package execution.
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
 * Deferred cancellation is not allowed.
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
void SuspendableDWQwithThread::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                                     gpcc::osal::Thread::priority_t const priority,
                                     size_t const stackSize)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (ctrlStat != CtrlStat::noThread)
    throw std::logic_error("SuspendableDWQwithThread::Start: Already started.");

  thread.Start(std::bind(&SuspendableDWQwithThread::ThreadEntry, this), schedPolicy, priority, stackSize);

  try
  {
    ctrlStat = CtrlStat::suspendReq;
    while (ctrlStat != CtrlStat::suspended)
    {
      cvCtrlStatEvent.Wait(mutex);
    }
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Stops the thread.
 *
 * If a work package is in progress, then the thread will be stopped after the work package has completed. If the
 * work queue is empty, or if work package execution is suspended, then the thread will stop immediately.
 *
 * Enqueued work packages, that have not been processed yet remain in the work queue and are not removed by the
 * stop operation.
 *
 * \pre   The thread is running, work package execution is either suspended or not.
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
 * Deferred cancellation is not allowed.
 *
 */
void SuspendableDWQwithThread::Stop(void) noexcept
{
  try
  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    {
      gpcc::osal::MutexLocker mutexLocker(mutex);

      if ((ctrlStat != CtrlStat::running) && (ctrlStat != CtrlStat::suspended))
        throw std::logic_error("Invalid state");

      if (ctrlStat == CtrlStat::running)
        dwq.RequestTermination();

      ctrlStat = CtrlStat::terminateReq;
      cvCtrlStatEvent.Signal();
    }

    thread.Join();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("SuspendableDWQwithThread::Stop: Failed: ", e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Suspends execution of work packages.
 *
 * If a work package is in progress, then this will block until the work package has completed.
 *
 * \pre   The thread is running and work package execution is not suspened.
 *
 * \post  Work package execution is suspended.
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
 * Deferred cancellation is not allowed.
 */
void SuspendableDWQwithThread::Suspend(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (ctrlStat != CtrlStat::running)
    throw std::logic_error("SuspendableDWQwithThread::Suspend: Invalid state");

  cvCtrlStatEvent.Signal();
  dwq.RequestTermination();
  ctrlStat = CtrlStat::suspendReq;

  try
  {
    while (ctrlStat != CtrlStat::suspended)
    {
      cvCtrlStatEvent.Wait(mutex);
    }
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Resumes execution of work packages.
 *
 * \pre   The thread is running and work package execution is suspened.
 *
 * \post  Work packages are executed.
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
 * Deferred cancellation is not allowed.
 */
void SuspendableDWQwithThread::Resume(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (ctrlStat != CtrlStat::suspended)
    throw std::logic_error("SuspendableDWQwithThread::Resume: Invalid state");

  cvCtrlStatEvent.Signal();
  ctrlStat = CtrlStat::startReq;

  try
  {
    while (ctrlStat != CtrlStat::running)
    {
      cvCtrlStatEvent.Wait(mutex);
    }
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Entry function for the thread running the work queue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures that there is only one thread per @ref SuspendableDWQwithThread instance executing this.
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
void* SuspendableDWQwithThread::ThreadEntry(void) noexcept
{
  thread.SetCancelabilityEnabled(false);

  while (true)
  {
    {
      // We will stay in this scope and respond to requests until ctrlStat is 'running'.
      // While ctrlStat is 'running' we will invoke dwq.Work().

      gpcc::osal::MutexLocker mutexLocker(mutex);
      bool runWQ = (ctrlStat == CtrlStat::running);

      while (!runWQ)
      {
        switch (ctrlStat)
        {
          case CtrlStat::startReq:
            ctrlStat = CtrlStat::running;
            runWQ = true;
            cvCtrlStatEvent.Signal();
            break;

          case CtrlStat::suspendReq:
            ctrlStat = CtrlStat::suspended;
            cvCtrlStatEvent.Signal();
            break;

          case CtrlStat::terminateReq:
            ctrlStat = CtrlStat::noThread;
            cvCtrlStatEvent.Signal();
            return nullptr;

          default:
            cvCtrlStatEvent.Wait(mutex);
        }
      }
    }

    try
    {
      dwq.Work();
    }
    catch (std::exception const & e)
    {
      gpcc::osal::Panic("SuspendableDWQwithThread: A work package threw: ", e);
    }
    catch (...)
    {
      gpcc::osal::Panic("SuspendableDWQwithThread: Caught an unknown exception thrown by a work package.");
    }
  }

  return nullptr;
}

} // namespace async
} // namespace execution
} // namespace gpcc
