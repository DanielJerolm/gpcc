/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
*/

#include <gpcc/execution/cyclic/TriggeredThreadedCyclicExec.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/stdif/notify/IIRQ2ThreadWakeup.hpp>
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
 * \param pThreadName
 * Name for the object's thread. This must point to a null-terminated string.
 * `nullptr` is not allowed.
 *
 * \param trigger
 * Reference to an [IIRQ2ThreadWakeup](@ref gpcc::stdif::IIRQ2ThreadWakeup) interface that shall be used to deliver the
 * cyclic trigger.
 *
 * \param timeout
 * Timeout for monitoring the cyclic trigger.\n
 * This should be approximately the expected period plus a reasonable safety-margin.
 *
 * \param isPllLockedFunc
 * Functor to a function/method that shall be used to query if the PLL driving @p trigger is in the locked state or
 * not.\n
 * If no PLL is used to drive @p trigger, or if the lock state shall not be monitored, then no function/method
 * shall be referenced. For details please refer to the documentation of @ref tIsPllLocked. \n
 * _A copy is generated._
 */
TriggeredThreadedCyclicExec::TriggeredThreadedCyclicExec(char const * const pThreadName,
                                                         stdif::IIRQ2ThreadWakeup & trigger,
                                                         time::TimeSpan const & timeout,
                                                         tIsPllLocked const & isPllLockedFunc)
: trigger_(trigger)
, timeout_(timeout)
, isPllLockedFunc_(isPllLockedFunc)
, thread_(pThreadName)
, mutex_()
, asyncReqFlags_(static_cast<uint8_t>(AsyncReqFlags::none))
, state_(States::stopped)
, startDelayCnt_(0U)
{
}

/**
 * \brief Destructor.
 *
 * \pre   The object must be in state @ref States::stopped.
 *
 * \pre   The object's thread must be terminated (call to @ref StopThread()).
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TriggeredThreadedCyclicExec::~TriggeredThreadedCyclicExec(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex_);
  if (state_ != States::stopped)
    PANIC();
}

/**
 * \brief Retrieves a null-terminated c-string with the name of an @ref States value.
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
 * \param state
 * @ref States value for which a null-terminated c-string with the value's name shall be retrieved.
 *
 * \return
 * Pointer to a null-terminated c-string with the name of the @ref States value specified by @p state.
 */
char const * TriggeredThreadedCyclicExec::State2String(States const state) noexcept
{
  switch (state)
  {
    case States::stopped:  return "stopped";
    case States::starting: return "start";
    case States::waitLock: return "waitLock";
    case States::running:  return "run";
  } // switch (state)

  PANIC();
}

/**
 * \brief Retrieves a null-terminated c-string with the name of an @ref StopReasons value.
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
 * \param code
 * @ref StopReasons value for which a null-terminated c-string with the value's name shall be retrieved.
 *
 * \return
 * Pointer to a null-terminated c-string with a the name of the @ref StopReasons value specified by @p code.
 */
char const * TriggeredThreadedCyclicExec::StopReasons2String(StopReasons const code) noexcept
{
  switch (code)
  {
    case StopReasons::none:            return "none";
    case StopReasons::reqStopSampling: return "reqStopSampling";
    case StopReasons::triggerTimeout:  return "triggerTimeout";
    case StopReasons::pllLossOfLock:   return "pllLossOfLock";
    case StopReasons::sampleRetFalse:  return "sampleRetFalse";
  }

  PANIC();
}

/**
 * \brief Retrieves a textual description for an @ref StopReasons value.
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
 * \param code
 * @ref StopReasons value for which a textual description shall be retrieved.
 *
 * \return
 * Pointer to a null-terminated c-string with a textual description of the @ref StopReasons value specified by @p code.
 */
char const * TriggeredThreadedCyclicExec::StopReasons2Description(StopReasons const code) noexcept
{
  switch (code)
  {
    case StopReasons::none:            return "State is not States::Stopped";
    case StopReasons::reqStopSampling: return "RequestStopSampling() was called";
    case StopReasons::triggerTimeout:  return "Trigger timeout";
    case StopReasons::pllLossOfLock:   return "PLL loss of lock";
    case StopReasons::sampleRetFalse:  return "Sample() returned false";
  }

  PANIC();
}

/**
 * \brief Starts the object's thread. This does not yet start sampling.
 *
 * \pre   The thread must not yet be running.
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
 * Scheduling policy that shall be used for the new thread. See @ref gpcc::osal::Thread::SchedPolicy for details.
 *
 * \param priority
 * Priority level: 0 (low) .. 31 (high)\n
 * This is only relevant for the scheduling policies @ref gpcc::osal::Thread::SchedPolicy::Fifo and
 * @ref gpcc::osal::Thread::SchedPolicy::RR. \n
 * _For the other scheduling policies this must be zero._
 *
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
  thread_.Start(std::bind(&TriggeredThreadedCyclicExec::InternalThreadEntry, this), schedPolicy, priority, stackSize);
}

/**
 * \brief Stops the object's thread and waits until the thread has terminated and joined.
 *
 * \pre   The thread is running.
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
 */
void TriggeredThreadedCyclicExec::StopThread(void) noexcept
{
  thread_.Cancel();
  (void)thread_.Join();
}

/**
 * \brief Requests start of sampling.
 *
 * It is not mandatory, that the object's thread is running. The thread may be started after calling this.
 *
 * \pre   The current state is @ref States::stopped.
 *
 * \pre   There is no pending start or stop request.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This may be called in the context of the object's own thread.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param startDelay
 * Number of extra cycles the @ref TriggeredThreadedCyclicExec instance shall remain in state @ref States::starting
 * before switching to @ref States::waitLock. If this is zero, then one cycle is spend in state @ref States::starting.
 */
void TriggeredThreadedCyclicExec::RequestStartSampling(uint8_t const startDelay)
{
  osal::MutexLocker mutexLocker(mutex_);

  if (state_ != States::stopped)
    throw std::logic_error("TriggeredThreadedCyclicExec::RequestStartSampling: Current state must be \"Stopped\"");

  if ((asyncReqFlags_ & (static_cast<uint8_t>(AsyncReqFlags::start) | static_cast<uint8_t>(AsyncReqFlags::stop))) != 0U)
    throw std::logic_error("TriggeredThreadedCyclicExec::RequestStartSampling: Start/Stop request already pending");

  asyncReqFlags_ |= static_cast<uint8_t>(AsyncReqFlags::start);
  startDelayCnt_ = startDelay;
}

/**
 * \brief Requests stop of sampling and removes a potential pending start request.
 *
 * \pre   There must be no pending stop request.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This may be called in the context of the object's own thread.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void TriggeredThreadedCyclicExec::RequestStopSampling(void)
{
  osal::MutexLocker mutexLocker(mutex_);

  if ((asyncReqFlags_ & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0U)
    throw std::logic_error("TriggeredThreadedCyclicExec::RequestStopSampling: Stop request already pending");

  // set stop request flag and clear a potential start request flag
  asyncReqFlags_ =   (asyncReqFlags_ | static_cast<uint8_t>(AsyncReqFlags::stop))
                   & static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::start));
}

/**
 * \brief Retrieves the current state of the @ref TriggeredThreadedCyclicExec instance.
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
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Current state of the @ref TriggeredThreadedCyclicExec instance.
 */
TriggeredThreadedCyclicExec::States TriggeredThreadedCyclicExec::GetCurrentState(void) const
{
  osal::MutexLocker mutexLocker(mutex_);
  return state_;
}

/**
 * \brief Entry function for the object's thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not applicable. Program logic ensures that there can only be one thread at any time.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Always nullptr.
 */
void* TriggeredThreadedCyclicExec::InternalThreadEntry(void)
{
  try
  {
    // always start in state stopped
    osal::AdvancedMutexLocker mutexLocker(mutex_);
    state_ = States::stopped;
    mutexLocker.Unlock();

    // loop until thread cancellation is requested
    while (!thread_.IsCancellationPending())
    {
      // wait for trigger
      stdif::IIRQ2ThreadWakeup::Result const result = trigger_.WaitWithTimeout(timeout_);
      bool const result_overrun = (result == stdif::IIRQ2ThreadWakeup::Result::AlreadySignalled);
      bool const result_timeout = (result == stdif::IIRQ2ThreadWakeup::Result::Timeout);

      mutexLocker.Relock();

      switch (state_)
      {
        case States::stopped:
        {
          if ((asyncReqFlags_ & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags_ &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else if ((asyncReqFlags_ & static_cast<uint8_t>(AsyncReqFlags::start)) != 0)
          {
            asyncReqFlags_ &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::start));
            state_ = States::starting;
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
          if ((asyncReqFlags_ & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags_ &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            state_ = States::stopped;
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else
          {
            if (startDelayCnt_ == 0)
            {
              state_ = States::waitLock;
              mutexLocker.Unlock();
              OnStateChange(States::waitLock, StopReasons::none);
            }
            else
            {
              startDelayCnt_--;
              mutexLocker.Unlock();
            }
          }
          break;
        }

        case States::waitLock:
        {
          if (result_timeout)
          {
            state_ = States::stopped;
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::triggerTimeout);
          }
          else if ((asyncReqFlags_ & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags_ &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            state_ = States::stopped;
            mutexLocker.Unlock();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else if ((!isPllLockedFunc_) || (isPllLockedFunc_()))
          {
            state_ = States::running;
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
          if (result_timeout)
          {
            state_ = States::stopped;
            mutexLocker.Unlock();
            OnStop();
            OnStateChange(States::stopped, StopReasons::triggerTimeout);
          }
          else if ((isPllLockedFunc_) && (!isPllLockedFunc_()))
          {
            state_ = States::stopped;
            mutexLocker.Unlock();
            OnStop();
            OnStateChange(States::stopped, StopReasons::pllLossOfLock);
          }
          else if ((asyncReqFlags_ & static_cast<uint8_t>(AsyncReqFlags::stop)) != 0)
          {
            asyncReqFlags_ &= static_cast<uint8_t>(~static_cast<uint8_t>(AsyncReqFlags::stop));
            state_ = States::stopped;
            mutexLocker.Unlock();
            OnStop();
            OnStateChange(States::stopped, StopReasons::reqStopSampling);
          }
          else
          {
            mutexLocker.Unlock();

            if (!Sample(result_overrun))
            {
              mutexLocker.Relock();
              state_ = States::stopped;
              mutexLocker.Unlock();
              OnStop();
              OnStateChange(States::stopped, StopReasons::sampleRetFalse);
            }
          }
          break;
        }
      } // switch (state_)

      Cyclic();
    } // while (!thread_.IsCancellationPending())
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
