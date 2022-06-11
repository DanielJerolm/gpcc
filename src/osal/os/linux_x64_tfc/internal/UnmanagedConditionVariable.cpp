/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifdef OS_LINUX_X64_TFC

#include "UnmanagedConditionVariable.hpp"
#include "UnmanagedMutex.hpp"
#include "../Panic.hpp"
#include <system_error>

namespace gpcc {
namespace osal {
namespace internal {

// Helper class for class UnmanagedConditionVariable. Provides an initialized pthread_condattr_t structure
// to the constructor of class UnmanagedConditionVariable.
class CondAttr final
{
    friend class UnmanagedConditionVariable;

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
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 */
UnmanagedConditionVariable::UnmanagedConditionVariable(void)
{
  static CondAttr condAttr;

  int const status = pthread_cond_init(&condVar, &condAttr.condAttr);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_init(...) failed");
}

/**
 * \brief Destructor.
 *
 * _No thread must be blocked on the condition variable._
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
UnmanagedConditionVariable::~UnmanagedConditionVariable(void)
{
  if (pthread_cond_destroy(&condVar) != 0)
    PANIC();
}

/**
 * \brief Unlocks an unmanaged mutex and blocks on the condition variable atomically.
 *
 * The current thread is blocked until any of the methods @ref Signal() or @ref Broadcast() are invoked.
 *
 * Note:
 * - On some systems, this method is a cancellation point\n
 *   (The unmanaged mutex is locked on cancellation)
 * - A call to @ref Signal() wakes up _only one_ waiter
 * - A call to @ref Broadcast() wakes up _all_ waiters
 * - Beware of spurious wake-ups:
 *   Do not assume, that the condition is always true on wake up!
 * - always double check the condition (before calling @ref Wait() and after @ref Wait() returns)
 * - always wait for a condition variable in a tight loop
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * Even in case of an exception, `mutex` will be locked.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.\n
 * This method contains a cancellation point.\n
 * In case of thread-cancellation, `mutex` will be locked.
 *
 * ---
 *
 * \param mutex
 * Reference to the unmanaged mutex associated with the predicate the calling thread is waiting for.\n
 * The mutex object will be unlocked while the thread is blocked on the condition variable.\n
 * The mutex unlock and the thread blocking are performed as an atomic operation.\n
 * When the method returns, then the mutex will be locked again regardless of the reason for
 * returning (signal, exception, thread cancellation).
 */
void UnmanagedConditionVariable::Wait(UnmanagedMutex & mutex)
{
  int const status = pthread_cond_wait(&condVar, &mutex.mutex);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_wait(...) failed");

  // ------------------------------------------------
  // Explicit check for pending cancellation requests
  // ------------------------------------------------
  // On some systems it was observed that the thread blocked on a condition variable is woken up by a signal even though
  // the thread has a deferred cancellation request pending when the signaling happens. The cancellation request and
  // signaling of the condition variable happened almost back-to-back.
  // The observed behaviour is considered compliant to POSIX and it should be not harmful for a productive application
  // using GPCC's OSAL because deferred cancellation will simply happen when the thread hits the next cancellation
  // point. Maybe that is also why it is called "deferred cancellation".
  // However, there are some unit tests for GPCC's OSAL that rely on deferred cancellation having priority above a
  // condition variable signaling, if the deferred cancellation is requested __BEFORE__ the condition variable is
  // signaled. To guarantee that behavior, the following explicit cancellation point has been added.
  pthread_testcancel();
}

/**
 * \brief Unblocks at least one of the threads that are currently blocked on the condition variable.
 *
 * If multiple threads are blocked on the condition variable, then one thread is woken up.\n
 * Which one is woken up depends on the underlying operating system.\n
 * If no thread is blocked on the condition variable, then the signal is lost and this method has
 * no effect.
 *
 * Note:
 * - A call to @ref Signal() wakes up _only one_ waiter
 * - A call to @ref Broadcast() wakes up _all_ waiters
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
 * Safe, no cancellation point included.
 */
void UnmanagedConditionVariable::Signal(void)
{
  int const status = pthread_cond_signal(&condVar);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_signal(...) failed");
}

/**
 * \brief Unblocks all threads currently blocked on the condition variable.
 *
 * Note:
 * - A call to @ref Signal() wakes up _only one_ waiter
 * - A call to @ref Broadcast() wakes up _all_ waiters
 * - Usually signaling is more efficient than broadcasting
 * - If no thread is blocked on the condition variable, then the signal/broadcast is lost and
 *   this method has no effect.
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
 * Safe, no cancellation point included.
 */
void UnmanagedConditionVariable::Broadcast(void)
{
  int const status = pthread_cond_broadcast(&condVar);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_cond_broadcast(...) failed");
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_X64_TFC
