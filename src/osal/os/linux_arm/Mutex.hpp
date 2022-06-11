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

#ifndef MUTEX_HPP_201702042221
#define MUTEX_HPP_201702042221

#include <pthread.h>
#include <bits/posix_opt.h>

#ifndef _POSIX_THREAD_PRIO_INHERIT
#error "Need support for priority inheritance!"
#endif

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
    /// The encapsulated pthread-mutex.
    pthread_mutex_t mutex;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef MUTEX_HPP_201702042221
#endif // #ifdef OS_LINUX_ARM
