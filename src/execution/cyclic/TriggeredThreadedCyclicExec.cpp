/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TriggeredThreadedCyclicExec.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/StdIf/IIRQ2ThreadWakeup.hpp"
#include <stdexcept>

namespace gpcc {
namespace execution {
namespace cyclic {

/**
 * \brief Constructor.
 *
 * After creation of the object, @ref StartThread() must be called to start the object's thread. After that,
 * cyclic sampling can be started and stopped via @ref RequestStartSampling() and @ref RequestStopSampling().
 *
 * ---
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pThreadName
 * Pointer to a null-terminated c-string with the name for the object's thread.\n
 * _No copy is generated._\n
 * _The referenced string must not change during lifetime of the TriggeredThreadedCyclicExec object._\n
 * _nullptr is not allowed._
 * \param _trigger
 * Reference to an [IIRQ2ThreadWakeup](@ref gpcc::StdIf::IIRQ2ThreadWakeup) subclass instance that shall be
 * used to deliver the cyclic trigger.
 * \param _timeout
 * Reference to a [TimeSpan](@ref gpcc::time::TimeSpan) instance providing the timeout for monitoring the cyclic
 * trigger. This should be approximately the expected period plus a reasonable safety-margin.\n
 * _A copy is generated._
 * \param _isPllLockedFunc
 * Functor to a function/method that shall be used to retrieve if the PLL driving `_trigger` is in the locked
 * state or not.\n
 * If no PLL is used to drive `_trigger`, or if the lock state shall not be monitored, then no function/method
 * must be referenced. For details please refer to the documentation of @ref tIsPllLocked. \n
 * _A copy is generated._
 */
TriggeredThreadedCyclicExec::TriggeredThreadedCyclicExec(char const * const pThreadName,
                                                         StdIf::IIRQ2ThreadWakeup & _trigger,
                                                         time::TimeSpan const & _timeout,
                                                         tIsPllLocked const & _isPllLockedFunc)
: trigger(_trigger)
, timeout(_timeout)
, isPllLockedFunc(_isPllLockedFunc)
, thread(pThreadName)
, mutex()
, asyncReqFlags(static_cast<uint8_t>(AsyncReqFlags::none))
, state(States::stopped)
, startDelayCnt(0)
{
}

/**
 * \brief Destructor.
 *
 * The following must be ensured:
 * - The object must be in state @ref States::stopped
 * - the object's thread must be terminated (call to @ref StopThread())
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
TriggeredThreadedCyclicExec::~TriggeredThreadedCyclicExec(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  if (state != States::stopped)
    osal::Panic("TriggeredThreadedCyclicExec::~TriggeredThreadedCyclicExec: Still running");
}

/**
 * \brief Retrieves a null-terminated c-string with the name of an @ref States value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param state
 * @ref States value for which a null-terminated c-string with the value's name shall be retrieved.
 * \return
 * Pointer to a null-terminated c-string with the name of the @ref States value specified by parameter 'state'.
 */
char const * TriggeredThreadedCyclicExec::State2String(States const state)
{
  switch (state)
  {
    case States::stopped:   return "stopped";
    case States::starting:     return "start";
    case States::waitLock:  return "waitLock";
    case States::running:       return "run";
    default:
      throw std::invalid_argument("TriggeredThreadedCyclicExec::State2String: unknown state");
  } // switch (state)
}

/**
 * \brief Retrieves a null-terminated c-string with the name of an @ref StopReasons value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param code
 * @ref StopReasons value for which a null-terminated c-string with the value's name shall be retrieved.
 * \return
 * Pointer to a null-terminated c-string with a the name of the @ref StopReasons value specified by parameter 'code'.
 */
char const * TriggeredThreadedCyclicExec::StopReasons2String(StopReasons const code)
{
  switch (code)
  {
    case StopReasons::none:            return "none";
    case StopReasons::reqStopSampling: return "reqStopSampling";
    case StopReasons::triggerTimeout:  return "triggerTimeout";
    case StopReasons::pllLossOfLock:   return "pllLossOfLock";
    case StopReasons::sampleRetFalse:  return "sampleRetFalse";
    default:
      throw std::invalid_argument("TriggeredThreadedCyclicExec::StopReasons2String: unknown code");
  }
}

/**
 * \brief Retrieves a textual description for an @ref StopReasons value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param code
 * @ref StopReasons value for which a textual description shall be retrieved.
 * \return
 * Pointer to a null-terminated c-string with a textual description of the @ref StopReasons value
 * specified by parameter 'code'.
 */
char const * TriggeredThreadedCyclicExec::StopReasons2Description(StopReasons const code)
{
  switch (code)
  {
    case StopReasons::none:            return "State is not States::Stopped";
    case StopReasons::reqStopSampling: return "RequestStopSampling() was called";
    case StopReasons::triggerTimeout:  return "Trigger timeout";
    case StopReasons::pllLossOfLock:   return "PLL loss of lock";
    case StopReasons::sampleRetFalse:  return "Sample() returned false";
    default:
      throw std::invalid_argument("TriggeredThreadedCyclicExec::StopReasons2String: unknown code");
  }
}

/**
 * \brief Starts the object's thread. This does not yet start sampling.
 *
 * The thread must not yet be running.\n
 * After calling this, sampling can be enabled by calling @ref RequestStartSampling() if not yet done.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param schedPolicy
 * Scheduling policy that shall be used for the new thread. See @ref gpcc::osal::Thread::SchedPolicy for details.
 * \param priority
 * Priority level: 0 (low) .. 31 (high)\n
 * This is only relevant for the scheduling policies @ref gpcc::osal::Thread::SchedPolicy::Fifo and
 * @ref gpcc::osal::Thread::SchedPolicy::RR. \n
 * _For the other scheduling policies this must be zero._
 * \param stackSize
 * Size of the stack of the new thread in byte.\n
 * _This must be a multiple of `gpcc::osal::Thread::GetStackAlign()`._\n
 * _This must be equal to or larger than `gpcc::osal::Thread::GetMinStackSize()`._\n
 * Internally this may be round up to some quantity, e.g. the system's page size.
 */
void TriggeredThreadedCyclicExec::StartThread(osal::Thread::SchedPolicy const schedPolicy,
                                              osal::Thread::priority_t const priority,
                                              size_t const stackSize)
{
  thread.Start(std::bind(&TriggeredThreadedCyclicExec::InternalThreadEntry, this), schedPolicy, priority, stackSize);
}

/**
 * \brief Cancels the object's thread and waits until the thread has terminated and joined.
 *
 * Note:\n
 * Sampling is not stopped gracefully. Instead the object's thread is cancelled using deferred cancellation.
 *
 * The thread must not yet be stopped.
 *
 * This will block until the object's thread has terminated and has been cleaned-up (joined).\n
 * After this has returned, it is safe to restart the object's thread via @ref StartThread() or to destroy the object.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void TriggeredThreadedCyclicExec::StopThread(void) noexcept
{
  try
  {
    thread.Cancel();
    (void)thread.Join();
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Requests start of sampling.
 *
 * The current state must be @ref States::stopped and there must be no pending start or stop request.\n
 * It is not mandatory, that the object's thread is running.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This may even be called in the context of the object's own thread.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param startDelay
 * Number of extra cycles the @ref TriggeredThreadedCyclicExec instance shall remain in state @ref States::starting
 * before moving to @ref States::waitLock. If this is zero, then one cycle is spend in state @ref States::starting.
 */
void TriggeredThreadedCyclicExec::RequestStartSampling(uint8_t const startDelay)
{
  osal::MutexLocker mutexLocker(mutex);

  if (state != States::stopped)
    throw std::logic_error("TriggeredThreadedCyclicExec::RequestStartSampling: Current state must be \"Stopped\"");

  if ((asyncReqFlags & (static_cast<uint8_t>(AsyncReqFlags::start) | static_cast<uint8_t>(AsyncReqFlags::stop))) != 0)
    throw std::logic_error("TriggeredThreadedCyclicExec::RequestStartSampling: Start/Stop request already pending");

  asyncReqFlags |= static_cast<uint8_t>(AsyncReqFlags::start);
  startDelayCnt = startDelay;
}

/**
 * \brief Requests stop of sampling and removes a potential pending start request.
 *
 * There must be no pending stop request. Any pending start request is canceled.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This may even be called in the context of the object's own thread.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 */
void TriggeredThreadedCyclicExec::RequestStopSampling(void)
{
  osal::MutexLocker mutexLocker(mutex);

  if ((asyncReqFlags & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
    throw std::logic_error("TriggeredThreadedCyclicExec::RequestStopSampling: Stop request already pending");

  // set stop request flag and clear a potential start request flag
  asyncReqFlags = (asyncReqFlags | static_cast<uint8_t>(AsyncReqFlags::stop)) & static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::start));
}

/**
 * \brief Retrieves the current state of the @ref TriggeredThreadedCyclicExec instance.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Current state of the @ref TriggeredThreadedCyclicExec instance.
 */
TriggeredThreadedCyclicExec::States TriggeredThreadedCyclicExec::GetCurrentState(void) const
{
  osal::MutexLocker mutexLocker(mutex);
  return state;
}

/**
 * \brief Entry function for the object's thread.
 *
 * __Thread safety:__\n
 * Not applicable. Program logic ensures that there can only be one thread at any time.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Always nullptr.
 */
void* TriggeredThreadedCyclicExec::InternalThreadEntry(void)
{
  try
  {
    // always start in state stopped
    osal::AdvancedMutexLocker mutexLocker(mutex);
    state = States::stopped;
    mutexLocker.Unlock();

    // loop until thread cancellation is requested
    while (!thread.IsCancellationPending())
    {
      // wait for trigger
      StdIf::IIRQ2ThreadWakeup::Result const result = trigger.WaitWithTimeout(timeout);
      bool const overrun = (result == StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled);
      bool const timeout = (result == StdIf::IIRQ2ThreadWakeup::Result::Timeout);

      mutexLocker.Relock();

      switch (state)
      {
        case States::stopped:
        {
          if ((asyncReqFlags & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else if ((asyncReqFlags & static_cast<uint8_t>(AsyncReqFlags::start)) != 0)
          {
            asyncReqFlags &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::start));
            state = States::starting;
            mutexLocker.Unlock();
            OnStateChange(States::starting, StopReasons::none);
          }
          else
          {
            mutexLocker.Unlock();
          }
          break;
        }

        case States::starting:
        {
          if ((asyncReqFlags & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            state = States::stopped;
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else
          {
            if (startDelayCnt == 0)
            {
              state = States::waitLock;
              mutexLocker.Unlock();
              OnStateChange(States::waitLock, StopReasons::none);
            }
            else
            {
              startDelayCnt--;
              mutexLocker.Unlock();
            }
          }
          break;
        }

        case States::waitLock:
        {
          if (timeout)
          {
            state = States::stopped;
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::triggerTimeout);
          }
          else if ((asyncReqFlags & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            state = States::stopped;
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else if ((!isPllLockedFunc) || (isPllLockedFunc()))
          {
            state = States::running;
            mutexLocker.Unlock();
            OnStateChange(States::running, StopReasons::none);
            OnStart();
          }
          else
          {
            mutexLocker.Unlock();
          }
          break;
        }

        case States::running:
        {
          if (timeout)
          {
            state = States::stopped;
            mutexLocker.Unlock();
            OnStop();
            OnStateChange(States::stopped, StopReasons::triggerTimeout);
          }
          else if ((isPllLockedFunc) && (!isPllLockedFunc()))
          {
            state = States::stopped;
            mutexLocker.Unlock();
            OnStop();
            OnStateChange(States::stopped, StopReasons::pllLossOfLock);
          }
          else if ((asyncReqFlags & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            state = States::stopped;
            mutexLocker.Unlock();
            OnStop();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else
          {
            mutexLocker.Unlock();

            if (!Sample(overrun))
            {
              mutexLocker.Relock();
              state = States::stopped;
              mutexLocker.Unlock();
              OnStop();
              OnStateChange(States::stopped, StopReasons::sampleRetFalse);
            }
          }
          break;
        }
      } // switch (state)

      Cyclic();
    } // while (!thread.IsCancellationPending())
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }

  return nullptr;
}

} // namespace cyclic
} // namespace execution
} // namespace gpcc
