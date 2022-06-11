/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2022 Daniel Jerolm

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

#ifdef OS_LINUX_ARM

#ifndef CONDITIONVARIABLE_HPP_201702042220
#define CONDITIONVARIABLE_HPP_201702042220

#include <pthread.h>

namespace gpcc {

namespace time {
class TimePoint;
}

namespace osal {
class Mutex;

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A condition variable.
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
    /// The encapsulated POSIX condition-variable.
    pthread_cond_t condVar;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef CONDITIONVARIABLE_HPP_201702042220
#endif // #ifdef OS_LINUX_ARM
