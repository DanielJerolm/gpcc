/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_CHIBIOS_ARM

#include "ConditionVariable.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "Panic.hpp"

#define NS_PER_SEC      1000000000UL
#define NS_PER_SYSTICK  (NS_PER_SEC / CH_CFG_ST_FREQUENCY)

static_assert(NS_PER_SEC % NS_PER_SYSTICK == 0, "Some calculations in this class will not work with configured system tick period");

namespace gpcc {
namespace osal {

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
 */
ConditionVariable::ConditionVariable(void)
{
  chCondObjectInit(&condVar);
}

/**
 * \brief Destructor.
 *
 * \pre   No thread must be blocked on the condition variable.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ConditionVariable::~ConditionVariable(void)
{
  if (queue_notempty(&condVar.queue))
    Panic("ConditionVariable::~ConditionVariable: At least one thread is blocked on CV!");
}

/**
 * \brief Unlocks a mutex and blocks on the condition variable atomically.
 *
 * The current thread is blocked until any of the methods @ref Signal() or @ref Broadcast() are invoked.
 *
 * Note:
 * - On some systems, this method is a cancellation point.\n
 *   (The mutex is locked on cancellation. It is recommended to use @ref gpcc::osal::MutexLocker or similar to simplify
 *   writing exception-safe and thread-cancellation-safe code).
 * - A call to @ref Signal() wakes up _only one_ waiter.
 * - A call to @ref Broadcast() wakes up _all_ waiters.
 * - Beware of spurious wake-ups:\n
 *   Do not assume, that the condition is always true on wake up!\n
 *   Always double check the condition (before calling @ref Wait() and after @ref Wait() returns).\n
 *   Always wait for a condition variable in a tight loop.
 *
 * ~~~{.cpp}
 * gpcc::osal::MutexLocker locker(myMutex);
 *
 * while (condition == false)
 *   condVar.Wait(myMutex);
 *
 * // condition has become true!
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * Even in case of an exception, `mutex` will be locked.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.\n
 * On some systems, this method contains a cancellation point.\n
 * In case of thread-cancellation, `mutex` will be locked.
 *
 * - - -
 *
 * \param mutex
 * Reference to the mutex associated with the predicate the calling thread is waiting for.\n
 * The mutex object will be unlocked while the thread is blocked on the condition variable.\n
 * The mutex unlock and the thread blocking are performed as an atomic operation.\n
 * When the method returns, then the mutex will be locked again regardless of the reason for returning (signal,
 * exception, thread cancellation).\n
 * The referenced mutex must be the latest mutex locked by the calling thread.
 */
void ConditionVariable::Wait(Mutex & mutex)
{
  if (chThdGetSelfX()->mtxlist != &mutex.mutex)
    gpcc::osal::Panic("ConditionVariable::Wait: Mutex unlock order violated!");

  (void)chCondWait(&condVar);
}

/**
 * \brief Unlocks a mutex and blocks on the condition variable atomically (with timeout).
 *
 * The current thread is blocked until any of the methods @ref Signal() or @ref Broadcast() are invoked, or until a
 * specific absolute point in time is reached.
 *
 * Note:
 * - On some systems, this method is a cancellation point.\n
 *   (The mutex is locked on cancellation. It is recommended to use @ref gpcc::osal::MutexLocker or similar to simplify
 *   writing exception-safe and thread-cancellation-safe code).
 * - A call to @ref Signal() wakes up _only one_ waiter.
 * - A call to @ref Broadcast() wakes up _all_ waiters.
 * - Beware of spurious wake-ups:\n
 *   Do not assume, that the condition is always true on wake up!\n
 *   Always double check the condition (before calling @ref Wait() and after @ref Wait() returns).\n
 *   Always wait for a condition variable in a tight loop.
 *
 * ~~~{.cpp}
 * gpcc::osal::MutexLocker locker(myMutex);
 *
 * // calculate timeout (here: 1 second from now)
 * gpcc::time::TimePoint tp = gpcc::time::TimePoint::FromSystemClock(Clocks::monotonic);
 * tp += gpcc::time::TimeSpan::sec(1);
 *
 * bool timeout = false;
 * while ((condition == false) && (timeout == false))
 *   timeout = condVar.TimeLimitedWait(myMutex, tp);
 *
 * // condition has become true or we have a timeout condition
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * Even in case of an exception, `mutex` will be locked.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.\n
 * On some systems, this method contains a cancellation point.\n
 * In case of thread-cancellation, `mutex` will be locked.
 *
 * - - -
 *
 * \param mutex
 * Reference to the mutex associated with the predicate the calling thread is waiting for.\n
 * The mutex object will be unlocked while the thread is blocked on the condition variable.\n
 * The mutex unlock and the thread blocking are performed as an atomic operation.\n
 * When the method returns, then the mutex will be locked again regardless of the reason for returning (signal,
 * timeout, exception, thread cancellation).\n
 * The referenced mutex must be the latest mutex locked by the calling thread.
 *
 * \param absoluteTimeout
 * Absolute point in time when the timeout for waiting for the condition being signalled or broadcasted expires.\n
 * This method will return if the specified absolute point in time passes (= the system time equals or exceeds
 * `absoluteTimeout`) regardless whether the condition has been signaled or not. This method will also return
 * immediately, if the specified point in time has already been passed when the call to this method is made.\n
 * The mutex will be locked again when the method returns due to a timeout condition.\n
 * The time is specified using @ref gpcc::time::Clocks::monotonic
 *
 * \retval true
 *   Woke up due to timeout.
 * \retval false
 *   Woke up due to signal.
 */
bool ConditionVariable::TimeLimitedWait(Mutex & mutex, time::TimePoint const & absoluteTimeout)
{
  using namespace gpcc::time;

  if (chThdGetSelfX()->mtxlist != &mutex.mutex)
    gpcc::osal::Panic("ConditionVariable::TimeLimitedWait: Mutex unlock order violated!");

  while (true)
  {
    // get current time
    TimePoint const curr_time = TimePoint::FromSystemClock(Clocks::monotonic);

    // timeout?
    if (curr_time >= absoluteTimeout)
      return true;

    // calculate remaining time (cannot be zero)
    TimeSpan const remaining_time = absoluteTimeout - curr_time;

    if (remaining_time.ns() <= 0)
      PANIC(); // Arithmetic error

    // Calculate number of system timer ticks until timeout.
    // We round up to next tick and add 1 tick extra for uncertainty due to granularity of system tick interrupt)
    uint64_t const ticks_till_timeout = ((static_cast<uint64_t>(remaining_time.ns()) + (NS_PER_SYSTICK - 1U)) / NS_PER_SYSTICK) + 1U;

    // wait in chunks of TIME_MAX_INTERVAL if ticks_till_timeout is too large to handle
    if (ticks_till_timeout > TIME_MAX_INTERVAL)
    {
      if (chCondWaitTimeout(&condVar, TIME_MAX_INTERVAL) != MSG_TIMEOUT)
        return false;
      else
        mutex.Lock();
    }
    else
    {
      if (chCondWaitTimeout(&condVar, static_cast<sysinterval_t>(ticks_till_timeout)) != MSG_TIMEOUT)
        return false;
      else
      {
        mutex.Lock();
        return true;
      }
    }
  } // while (true)

  // never reached, but makes compiler happy
  return false;
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_CHIBIOS_ARM
