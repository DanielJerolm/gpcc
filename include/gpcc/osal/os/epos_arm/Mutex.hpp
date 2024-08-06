/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_EPOS_ARM

#ifndef MUTEX_HPP_202404231958
#define MUTEX_HPP_202404231958

#include <epos/scheduler/mutex.h>

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
    /// The encapsulated EPOS-mutex.
    epos_mutex_t mutex;
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
  epos_mutex_Lock(&mutex);
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
  return epos_mutex_TryLock(&mutex);
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef MUTEX_HPP_202404231958
#endif // #ifdef OS_EPOS_ARM
