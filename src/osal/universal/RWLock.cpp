/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#include "RWLock.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/time/TimePoint.hpp"
#include <limits>
#include <stdexcept>

namespace gpcc {
namespace osal {

/**
 * \brief Constructor. The new @ref RWLock is unlocked.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
RWLock::RWLock(void)
: mutex()
, nbOfLocks(0)
, nbOfBlockedWriters(0)
, condVarForWriters()
, condVarForReaders()
{
}

/**
 * \brief Destructor.
 *
 * \pre   The @ref RWLock must not be locked by any reader or writer.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
RWLock::~RWLock(void)
{
  MutexLocker locker(mutex);
  if (nbOfLocks != 0)
    Panic("RWLock::~RWLock(): RWLock is locked");
}

/**
 * \brief Tries to acquire a write-lock (does not block).
 *
 * This returns immediately if the write-lock cannot be acquired.
 *
 * The calling thread is allowed hold a read- or write-lock on this @ref RWLock. In this case, this method will
 * return false.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \retval true   Write lock has been acquired.
 * \retval false  Write-lock has _not_ been acquired. The calling thread or another thread already hold a read- or
 *                write-lock on this @ref RWLock instance.
 */
bool RWLock::TryWriteLock(void)
{
  MutexLocker locker(mutex);

  // locked by someone?
  if (nbOfLocks != 0)
    return false;

  // acquire write-lock
  nbOfLocks = -1;
  return true;
}

/**
 * \brief Acquires a write-lock (blocking).
 *
 * This blocks until the write-lock is acquired.
 *
 * \pre   The calling thread must not hold any read- or write-lock on this @ref RWLock instance. Otherwise:
 *        - With TFC: A dead-lock will be detected if all other threads in the process are also blocked.
 *        - Without TFC: Behaviour is undefined.
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
 * Strong guarantee.
 */
void RWLock::WriteLock(void)
{
  MutexLocker locker(mutex);

  // locked by someone?
  if (nbOfLocks != 0)
  {
    if (nbOfBlockedWriters == std::numeric_limits<int32_t>::max())
      throw std::runtime_error("RWLock::WriteLock: No more writers can be blocked");

    nbOfBlockedWriters++;
    ON_SCOPE_EXIT()
    {
      nbOfBlockedWriters--;
      if (nbOfLocks == 0)
        SignalZero();
    };

    do
    {
      condVarForWriters.Wait(mutex);
    }
    while (nbOfLocks != 0);
  }

  // acquire write-lock
  nbOfLocks = -1;
}

/**
 * \brief Acquires a write-lock (blocking with timeout).
 *
 * This blocks until the write-lock is acquired or a timeout occurs.
 *
 * \pre   The calling thread must not hold any read- or write-lock on this @ref RWLock instance. Otherwise:
 *        - With TFC: The call to this method will return false after the timeout has expired
 *        - Without TFC: Behaviour is undefined
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param absoluteTimeout
 * Absolute point in time when the timeout for waiting for acquisition of the write-lock shall expire.\n
 * _This must be specified using Clocks::monotonic._
 *
 * \retval true   Write-lock acquired.
 * \retval false  Timeout. Write-lock _not_ acquired. Another writer or one or more readers already hold the lock.
 */
bool RWLock::WriteLock(time::TimePoint const & absoluteTimeout)
{
  MutexLocker locker(mutex);

  // locked by someone?
  if (nbOfLocks != 0)
  {
    if (nbOfBlockedWriters == std::numeric_limits<int32_t>::max())
      throw std::runtime_error("RWLock::WriteLock: No more writers can be blocked");

    nbOfBlockedWriters++;
    ON_SCOPE_EXIT()
    {
      nbOfBlockedWriters--;
      if (nbOfLocks == 0)
        SignalZero();
    };

    do
    {
      if ((condVarForWriters.TimeLimitedWait(mutex, absoluteTimeout)) && (nbOfLocks != 0))
      {
        // timeout
        return false;
      }
    }
    while (nbOfLocks != 0);
  }

  // acquire write lock
  nbOfLocks = -1;
  return true;
}

/**
 * \brief Releases a write-lock.
 *
 * \pre   The calling thread holds a write-lock on this @ref RWLock instance.
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
 * Strong guarantee.
 */
void RWLock::ReleaseWriteLock(void)
{
  MutexLocker locker(mutex);

  if (nbOfLocks != -1)
    throw std::logic_error("RWLock::ReleaseWriteLock(): Not locked");

  // release lock
  nbOfLocks = 0;

  // signal that the lock is free now
  SignalZero();
}

/**
 * \brief Tries to acquire a read-lock (does not block).
 *
 * This returns immediately if the read-lock cannot be acquired.
 *
 * The calling thread is allowed to hold one or more read-locks on this @ref RWLock instance. In this case, the
 * calling thread will acquire one more read-lock and this method will return true. Note that _all_ read-locks acquired
 * by the calling thread must be finally released by the appropriate number of calls to @ref ReleaseReadLock().
 *
 * The calling thread is allowed hold a write-lock on this @ref RWLock. In this case, this method will return false.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \retval true   Read-lock acquired.
 * \retval false  Read-lock _not_ acquired. A writer already holds the lock.
 */
bool RWLock::TryReadLock(void)
{
  MutexLocker locker(mutex);

  // locked by writer or any writer blocked?
  if ((nbOfLocks < 0) || (nbOfBlockedWriters != 0))
    return false;

  // acquire lock
  if (nbOfLocks == std::numeric_limits<int32_t>::max())
    throw std::runtime_error("RWLock::TryReadLock: Maximum number of read-locks reached");

  nbOfLocks++;
  return true;
}

/**
 * \brief Acquires a read-lock (blocking).
 *
 * This blocks until the read-lock is acquired.
 *
 * The calling thread is allowed to hold one or more read-locks on this @ref RWLock instance. In this case, the calling
 * thread will acquire one more read-lock and this method will return immediately. Note that _all_ read-locks acquired
 * by the calling thread must be finally released by the appropriate number of calls to @ref ReleaseReadLock().
 *
 * \pre   The calling thread must not hold a write-lock on this @ref RWLock instance. Otherwise:
 *        - With TFC: A dead-lock will be detected if all other threads in the process are also blocked.
 *        - Without TFC: Behaviour is undefined
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
 * Strong guarantee.
 */
void RWLock::ReadLock(void)
{
  MutexLocker locker(mutex);

  // locked by writer or writers waiting to acquire a lock?
  while ((nbOfLocks < 0) || (nbOfBlockedWriters != 0))
    condVarForReaders.Wait(mutex);

  // acquire lock
  if (nbOfLocks == std::numeric_limits<int32_t>::max())
    throw std::runtime_error("RWLock::ReadLock: Maximum number of read-locks reached");

  nbOfLocks++;
}

/**
 * \brief Acquires a read-lock (blocking with timeout).
 *
 * This blocks until the read-lock is acquired or a timeout occurs.
 *
 * The calling thread is allowed to hold one or more read-locks on this @ref RWLock instance. In this case, the
 * calling thread will acquire one more read-lock and this method will return immediately. Note that _all_ read-locks
 * acquired by the calling thread must be finally released by the appropriate number of calls to
 * @ref ReleaseReadLock().
 *
 * \pre   The calling thread must not hold a write-lock on this @ref RWLock instance. Otherwise:
 *        - With TFC: The call to this method will return false after the timeout has expired
 *        - Without TFC: Behaviour is undefined
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param absoluteTimeout
 * Absolute point in time when the timeout for waiting for acquisition of the read-lock shall expire.\n
 * _This must be specified using Clocks::monotonic._
 *
 * \retval true   Read-lock acquired.
 * \retval false  Timeout, read-lock _not_ acquired. A writer already holds the lock.
 */
bool RWLock::ReadLock(time::TimePoint const & absoluteTimeout)
{
  MutexLocker locker(mutex);

  // locked by writer or writers waiting to acquire a lock?
  while ((nbOfLocks < 0) || (nbOfBlockedWriters != 0))
  {
    if (   (condVarForReaders.TimeLimitedWait(mutex, absoluteTimeout))
        && ((nbOfLocks < 0) || (nbOfBlockedWriters != 0)))
    {
      // timeout
      return false;
    }
  }

  // acquire lock
  if (nbOfLocks == std::numeric_limits<int32_t>::max())
    throw std::runtime_error("RWLock::ReadLock: Maximum number of read-locks reached");

  nbOfLocks++;
  return true;
}

/**
 * \brief Releases a read-lock.
 *
 * \pre   The calling thread holds a read-lock on this @ref RWLock instance.
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
 * Strong guarantee.
 */
void RWLock::ReleaseReadLock(void)
{
  MutexLocker locker(mutex);

  if (nbOfLocks <= 0)
    throw std::logic_error("RWLock::ReleaseReadLock(): Not locked");

  // release lock
  nbOfLocks--;

  // signal that the lock is free now if this was the last read lock
  if (nbOfLocks == 0)
    SignalZero();
}

/**
 * \brief This must be called when the lock has become free (@ref nbOfLocks has reached zero).
 *
 * This checks if any writer or reader is waiting for the lock and signals that the lock is free now.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void RWLock::SignalZero(void) noexcept
{
  if (nbOfBlockedWriters != 0)
    condVarForWriters.Signal();
  else
    condVarForReaders.Broadcast();
}

} // namespace osal
} // namespace gpcc

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
