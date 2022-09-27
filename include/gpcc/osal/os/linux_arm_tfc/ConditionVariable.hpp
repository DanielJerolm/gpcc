/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#ifndef CONDITIONVARIABLE_HPP_201904071045
#define CONDITIONVARIABLE_HPP_201904071045

#include <vector>
#include <cstddef>

namespace gpcc {

namespace time {
class TimePoint;
}

namespace osal {

class Mutex;

namespace internal {
class TFCCore;
class ThreadBlockerBase;
}

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A condition variable.
 *
 * __Note:__\n
 * __This condition variable is managed by GPCC's TFC feature.__
 *
 * # Features
 * - Signaling of events to one or multiple threads waiting for a specific condition to come true.
 * - Threads can wait with and without timeout.
 * - Unlock of the latest locked mutex (class @ref Mutex) upon sleep and relock of that mutex upon wakeup.\n
 *   Entering sleep and unlocking of the mutex are performed as an atomic operation.
 *
 * # Constraints/Restrictions
 * - _All threads using instances of class_ @ref ConditionVariable _must live in the same process._
 * - _All threads that want to block on the condition variable must use the same mutex._
 * - _Only the latest locked mutex can be unlocked upon waiting for the condition variable._\n
 *   _Mutexes must be unlocked in lock-reverse order._
 *
 * # Usage
 * Condition variables are used to signal changes of variables, states or similar objects to threads waiting for that
 * variable or state to reach a certain value. In other words, condition variables are used to signal that a specific
 * condition has come true or _could_ be true now.
 *
 * The variable, state, or object involved in the condition is accessed by multiple threads and therefore always
 * protected by a mutex (class @ref Mutex). The condition variable is always closely coupled to that mutex and almost
 * always used in conjunction with the same mutex during the condition variable's life-time.
 *
 * _If you ever intent to use the same condition variable instance with different mutexes, then you should properly_
 * _review your software design._
 *
 * However, it is allowed to use multiple condition variables in conjunction with the same mutex and even in
 * conjunction with the same variable or state in order to signal different conditions.
 *
 * Condition variables require that a boolean expression can be created which indicates if the condition a thread is
 * waiting for is true or not. This is necessary, because the condition must _always_ be tested _before_ waiting for
 * a condition variable and _after_ the condition variable has been signaled. The reason is that condition variables
 * just give a hint that a condition _may_ be true now. On some operating systems spurious wake-ups from condition
 * variables may occur.
 *
 * The following example shows the basic usage of a condition variable. The example contains a variable `state`
 * representing some kind of state that is shared among two threads. Access to the variable is protected by mutex
 * `myMutex` and a condition variable `stateReadyConVar` is used to wake up the second thread if `state` is set to
 * `stateReady`.
 *
 * Code executed by thread 1:
 * ~~~{.cpp}
 * // First lock the mutex. In productive code you will likely use gpcc::osal::MutexLocker or similar.
 * myMutex.Lock();
 *
 * // update state to the new value
 * state = someNewState;
 *
 * // If the new value is stateReady, then we signal the condition variable which is used
 * // to indicate that state has been set to stateReady.
 * if (state == stateReady)
 *   stateReadyConVar.Signal();
 *
 * // Finally unlock the mutex. This allows a waiting thread to wake up and examine state.
 * myMutex.Unlock();
 * ~~~
 *
 * Code executed by thread 2 waiting for state to be set to `stateReady`:
 * ~~~{.cpp}
 * // First lock the mutex. In productive code you will likely use gpcc::osal::MutexLocker or similar.
 * myMutex.Lock();
 *
 * // Check condition first (it might already be true), then wait. Wait includes
 * // unlock of myMutex. After wake-up, myMutex will be relocked and the while-loop ensures that
 * // the condition is checked again.
 * while (state != stateReady)
 *   stateReadyConVar.Wait(myMutex);
 *
 * // State is stateReady now. myMutex is locked, so nobody else can
 * // change state while we hold the mutex.
 * // ...
 *
 * // finally unlock the mutex
 * myMutex.Unlock();
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ConditionVariable final
{
  public:
    ConditionVariable(void);
    ConditionVariable(ConditionVariable const &) = delete;
    ConditionVariable(ConditionVariable &&) = delete;
    ~ConditionVariable(void);

    ConditionVariable& operator=(ConditionVariable const &) = delete;
    ConditionVariable& operator=(ConditionVariable &&) = delete;

    void Wait(Mutex & mutex);
    bool TimeLimitedWait(Mutex & mutex, time::TimePoint const & absoluteTimeout);

    void Signal(void);
    void Broadcast(void);

  private:
    /// Pointer to the @ref internal::TFCCore instance.
    /** This is setup by the constructor and not changed afterwards. */
    internal::TFCCore* const pTFCCore;

    /// List with @ref internal::ThreadBlocker instances, each for one thread blocked on this condition variable.
    /** TFCCore's big lock is required. */
    std::vector<internal::ThreadBlockerBase*> blockedThreads;

    /// Number of threads currently blocked on this condition variable.
    /** TFCCore's big lock is required.\n
        This information seems to be redundant to `blockedThreads.size()`, but it is not.\n
        If a thread is woken up, then @ref Signal() and @ref Broadcast() will remove the associated
        entries from @ref blockedThreads, but the thread is not yet woken up. This is decremented
        by any blocked thread after it has really woken up again. */
    size_t nbOfBlockedThreads;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef CONDITIONVARIABLE_HPP_201904071045
#endif // #ifdef OS_LINUX_ARM_TFC
