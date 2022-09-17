/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#ifndef MUTEX_HPP_201904071051
#define MUTEX_HPP_201904071051

#include <pthread.h>
#include <memory>
#include <cstddef>

namespace gpcc {
namespace osal {

namespace internal {
class TFCCore;
class ThreadBlocker;
class TimeLimitedThreadBlocker;
class UnmanagedConditionVariable;
}

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A mutex.
 *
 * __Note:__\n
 * __This mutex is managed by GPCC's TFC feature.__\n
 * __The features listed below are supported by GPCC's normal mutex implementation.__\n
 * __The TFC variant does not support the priority inheritance protocol. However, this is not a problem, because TFC__
 * __pretends that the software is executed on a machine with infinite speed and an infinite number of CPU cores.__\n
 *
 * # Features:
 * - Non-recursive mutex
 * - Basic methods: @ref Lock(), @ref TryLock(), @ref Unlock()
 * - Priority inheritance protocol supported
 *
 * __Additional TFC-specific features:__
 * - __Runtime check for recursive lock.__
 * - __Runtime check if the thread which wants to unlock really has the mutex acquired.__
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
    friend class internal::ThreadBlocker;
    friend class internal::TimeLimitedThreadBlocker;

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
    /// Pointer to the @ref internal::TFCCore instance.
    /** This is setup by the constructor and not changed afterwards. */
    internal::TFCCore* const pTFCCore;

    /// Lock-state of the mutex.
    /** TFCCore's big lock is required. */
    bool locked;

    /// ID of the thread which has the mutex locked.
    /** TFCCore's big lock is required.\n
        This is only valid, if @ref locked is true. */
    pthread_t thread_id;

    /// Number of blocked threads.
    /** TFCCore's big lock is required. */
    size_t nbOfblockedThreads;

    /// Flag indicating if a blocked thread is going to wake up and acquire the mutex.
    /** TFCCore's big lock is required. */
    bool blockedThreadIsGoingToWakeUp;

    /// Condition variable used to signal when @ref locked is cleared.
    /** This must be used in conjunction with TFCCore's big lock. */
    std::unique_ptr<internal::UnmanagedConditionVariable> spUnlockedCV;

    void InternalLock(void);
    void InternalUnlock(void);
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef MUTEX_HPP_201904071051
#endif // #ifdef OS_LINUX_ARM_TFC
