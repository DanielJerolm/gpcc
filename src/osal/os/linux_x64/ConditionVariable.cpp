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

#ifdef OS_LINUX_X64

#include "ConditionVariable.hpp"
#include "Mutex.hpp"
#include "Panic.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include <system_error>
#include <ctime>

namespace gpcc {
namespace osal {

// Helper class for class ConditionVariable. Provides an initialized pthread_condattr_t structure
// to the constructor of class ConditionVariable.
class CondAttr final
{
    friend class ConditionVariable;

  public:
    CondAttr(void)
    {
      int status;

      status = pthread_condattr_init(&condAttr);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_condattr_init(...) failed");

      status = pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_condattr_setclock(...) failed");
    }
    CondAttr(CondAttr const &) = delete;
    CondAttr(CondAttr &&) = delete;

    ~CondAttr(void)
    {
      if (pthread_condattr_destroy(&condAttr) != 0)
        PANIC();
    }

    CondAttr& operator=(CondAttr const &) = delete;
    CondAttr& operator=(CondAttr &&) = delete;

  private:
    pthread_condattr_t condAttr;
};

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
  static CondAttr condAttr;

  int const status = pthread_cond_init(&condVar, &condAttr.condAttr);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_init(...) failed");
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
  if (pthread_cond_destroy(&condVar) != 0)
    PANIC();
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
  int const status = pthread_cond_wait(&condVar, &mutex.mutex);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_wait(...) failed");
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
  int const status = pthread_cond_timedwait(&condVar, &mutex.mutex, absoluteTimeout.Get_timespec_ptr());
  if (status == ETIMEDOUT)
    return true;
  else if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_timedwait(...) failed");
  else
    return false;
}

/**
 * \brief Unblocks at least one of the threads that are currently blocked on the condition variable.
 *
 * If multiple threads are blocked on the condition variable, then one thread is woken up.\n
 * Which one is woken up depends on the underlying operating system.\n
 * If no thread is blocked on the condition variable, then the signal is lost and this method has no effect.
 *
 * Note:
 * - A call to @ref Signal() wakes up _only one_ waiter.
 * - A call to @ref Broadcast() wakes up _all_ waiters.
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
 */
void ConditionVariable::Signal(void)
{
  int const status = pthread_cond_signal(&condVar);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_signal(...) failed");
}

/**
 * \brief Unblocks all threads currently blocked on the condition variable.
 *
 * Note:
 * - A call to @ref Signal() wakes up _only one_ waiter.
 * - A call to @ref Broadcast() wakes up _all_ waiters.
 * - Usually signaling is more efficient than broadcasting.
 * - If no thread is blocked on the condition variable, then the signal/broadcast is lost and this method has
 *   no effect.
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
 */
void ConditionVariable::Broadcast(void)
{
  int const status = pthread_cond_broadcast(&condVar);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_broadcast(...) failed");
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_X64
