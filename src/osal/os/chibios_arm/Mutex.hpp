/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019 Daniel Jerolm

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

#ifdef OS_CHIBIOS_ARM

#ifndef MUTEX_HPP_201701282154
#define MUTEX_HPP_201701282154

#include <ch.h>

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A mutex.
 *
 * # Features:
 * - Non-recursive mutex
 * - Basic methods: @ref Lock(), @ref TryLock(), @ref Unlock()
 * - Priority inheritance protocol supported
 *
 * # Constraints/Restrictions
 * - _All threads using instances of class Mutex must live in the same process._
 * - _Mutexes must be unlocked in lock-reverse order._
 *
 * # Usage
 * It is recommended to use class @ref Mutex in conjunction with an automatic mutex locker/unlocker class like
 * @ref MutexLocker or @ref AdvancedMutexLocker. Using these classes will simplify writing exception- and thread-
 * cancellation-safe code.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Mutex final
{
  friend class ConditionVariable;

  public:
    Mutex(void);
    Mutex(Mutex const &) = delete;
    Mutex(Mutex &&) = delete;
    ~Mutex(void);

    Mutex& operator=(Mutex const &) = delete;
    Mutex& operator=(Mutex &&) = delete;

    void Lock(void);
    bool TryLock(void);
    void Unlock(void) noexcept;

  private:
    /// The encapsulated ChibiOS-mutex.
    mutex_t mutex;
};

/**
 * \brief Locks the mutex.
 *
 * If the mutex is already locked by another thread, then this method will block until the other thread unlocks the
 * mutex and this thread acquires the mutex.
 *
 * \pre   The mutex must not yet be acquired by the calling thread.
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
inline void Mutex::Lock(void)
{
  chMtxLock(&mutex);
}

/**
 * \brief Tries to lock the mutex.
 *
 * Same as @ref Lock(), but returns immediately if the mutex is already locked by the calling thread or by
 * another thread.
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
 * \retval true
 *    The mutex has been locked by the calling thread.
 * \retval false
 *    The mutex is already locked by the calling thread __or__ by another thread.
 */
inline bool Mutex::TryLock(void)
{
  return chMtxTryLock(&mutex);
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef MUTEX_HPP_201701282154
#endif // #ifdef OS_CHIBIOS_ARM
