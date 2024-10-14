/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_EPOS_ARM) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC))

#ifndef RWLOCK_HPP_201701280949
#define RWLOCK_HPP_201701280949

#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <cstdint>

namespace gpcc {

namespace time {
class TimePoint;
}

namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief Lock providing reader- and writer-aware mutual exclusion.
 *
 * # Summary
 * The @ref RWLock provides a lock mechanism, which distinguishes between readers and writers.
 *
 * Readers can acquire a _read-lock_ and writers can acquire a _write-lock_. The @ref RWLock allows multiple readers
 * to read-lock a resource at the same time, while writers can only acquire a write-lock if the resource is not locked
 * by any reader or writer. Therefore no more than one writer can write-lock the resource at any time. A reader may
 * acquire multiple read-locks.
 *
 * An @ref RWLock is more efficient than a @ref Mutex if data is frequently read by multiple readers and only rarely
 * written by one (or very few) writers.
 *
 * # Rules
 * These rules apply to GPCC's RWLock in general. There may be specific implementations for different operating
 * systems, which may be less strict in some points.
 *
 * - The thread which has acquired a read- or write-lock must also unlock the @ref RWLock. A thread cannot release the
 *   lock acquired by a different thread.
 * - A thread may acquire _n_ read-locks. It finally has to unlock the read-lock _n_ times.
 * - A thread may acquire one write-lock. It finally has to unlock the write-lock one time.
 * - A thread holding a read-lock must not acquire a write-lock:
 *   + With TFC, a dead-lock will be detected if all other threads in the process are also blocked.
 *   + Without TFC, behaviour is undefined.
 * - A thread holding a write-lock must not acquire a read-lock or another write-lock:
 *   + With TFC, a dead-lock will be detected if all other threads in the process are also blocked.
 *   + Without TFC, behaviour is undefined.
 *
 * # Protocol
 * Writers are blocked until all readers who have the @ref RWLock already acquired have finished.
 *
 * Depending on the implementation, _new_ readers who want to acquire a read-lock while one or more writers are
 * _blocked_ may have to wait until _all_ the writers have been served, though a read-lock could be acquired.
 *
 * # Priority Inversion
 * Be aware of priority inversion. The @ref RWLock does not implement priority inheritance or any other strategy to
 * address priority inversion.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class RWLock final
{
  public:
    RWLock(void);
    RWLock(RWLock const &) = delete;
    RWLock(RWLock &&) = delete;
    ~RWLock(void);

    RWLock& operator=(RWLock const &) = delete;
    RWLock& operator=(RWLock &&) = delete;

    bool TryWriteLock(void);
    void WriteLock(void);
    bool WriteLock(time::TimePoint const & absoluteTimeout);
    void ReleaseWriteLock(void);

    bool TryReadLock(void);
    void ReadLock(void);
    bool ReadLock(time::TimePoint const & absoluteTimeout);
    void ReleaseReadLock(void);

  private:
    /// Mutex protecting access to internals.
    Mutex mutex;

    /// Number of acquired locks.
    /** @ref mutex is required.\n
        > 0  : Number of readers that have locked\n
        = 0  : Unlocked\n
        = -1 : Locked by __one__ writer */
    int32_t nbOfLocks;

    /// Number of blocked writers.
    /** @ref mutex is required.\n
        = 0 : No writer blocked\n
        > 0 : Number of blocked writers */
    int32_t nbOfBlockedWriters;

    /// Condition variable signaling to writers that @ref nbOfLocks has reached zero.
    /** This must be used in conjunction with @ref mutex. */
    ConditionVariable condVarForWriters;

    /// Condition variable signaling to readers that @ref nbOfLocks is >= 0 and @ref nbOfBlockedWriters is zero.
    /** This must be used in conjunction with @ref mutex. */
    ConditionVariable condVarForReaders;

    void SignalZero(void) noexcept;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef RWLOCK_HPP_201701280949
#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_EPOS_ARM) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC))
