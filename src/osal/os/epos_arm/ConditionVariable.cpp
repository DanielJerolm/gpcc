/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_EPOS_ARM

#include <gpcc/osal/ConditionVariable.hpp>
#include <epos/time/arithmetic.h>
#include <epos/time/time.h>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <stdexcept>

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
  epos_convar_Init(&condVar);
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
  if (epos_convar_IsAnyThreadBlocked(&condVar))
    Panic("ConditionVariable::~ConditionVariable: Blocked threads");
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
  epos_convar_Wait(&condVar, &mutex.mutex);
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
 * gpcc::time::TimePoint tp = gpcc::time::TimePoint::FromSystemClock(ConditionVariable::clockID);
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
 * The time must be specified using the clock @ref ConditionVariable::clockID.
 *
 * \retval true
 *   Woke up due to timeout.
 * \retval false
 *   Woke up due to signal.
 */
bool ConditionVariable::TimeLimitedWait(Mutex & mutex, time::TimePoint const & absoluteTimeout)
{
  uint64_t absTimeout_ns;
  if (!epos_time_TimespecToU64_ns(&absTimeout_ns, absoluteTimeout.Get_timespec_ptr()))
    throw std::overflow_error("Timeout too large");

  // compensate the clock granularity to ensure that the desired timespan is not underrun
  absTimeout_ns = epos_time_EnsureMinTimeSpanMonotonic_u64(absTimeout_ns);

  return epos_convar_TimeLimitedWait(&condVar, &mutex.mutex, absTimeout_ns);
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_EPOS_ARM
