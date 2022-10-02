/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#include <gpcc/osal/RWLockWriteLocker.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/exceptions.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>

namespace gpcc {
namespace osal {

/**
 * \brief Constructor. Creates a @ref RWLockWriteLocker instance and write-locks the managed @ref RWLock. \n
 *        The time waiting for acquisition of the write-lock is limited by a timeout (absolute value).
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws TimeoutError   Timeout while waiting for acquisition of the write-lock ([details](@ref gpcc::osal::TimeoutError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rwLock
 * Reference to the @ref RWLock instance that shall be managed by this @ref RWLockWriteLocker. \n
 * The @ref RWLock will be write-locked when the @ref RWLockWriteLocker is instantiated and the write-lock will
 * be released when the @ref RWLockWriteLocker is destroyed.
 * \param absTimeout
 * Timeout specified as an absolute point in time when waiting to acquire the write-lock.\n
 * The time must be specified using the clock @ref gpcc::osal::ConditionVariable::clockID.
 */
RWLockWriteLocker::RWLockWriteLocker(RWLock& rwLock, gpcc::time::TimePoint const & absTimeout)
: pRWLock(&rwLock)
{
  if (!pRWLock->WriteLock(absTimeout))
    throw TimeoutError("RWLockWriteLocker::RWLockWriteLocker: Timeout acquiring write-lock");
}

/**
 * \brief Constructor. Creates a @ref RWLockWriteLocker instance and write-locks the managed @ref RWLock. \n
 *        The time waiting for acquisition of the write-lock is limited by a timeout (relative value).
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws TimeoutError   Timeout while waiting for acquisition of the write-lock ([details](@ref gpcc::osal::TimeoutError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rwLock
 * Reference to the @ref RWLock instance that shall be managed by this @ref RWLockWriteLocker. \n
 * The @ref RWLock will be write-locked when the @ref RWLockWriteLocker is instantiated and the write-lock will
 * be released when the @ref RWLockWriteLocker is destroyed.
 * \param timeout
 * Timespan specifying the timeout when waiting to acquire the write-lock.
 */
RWLockWriteLocker::RWLockWriteLocker(RWLock& rwLock, gpcc::time::TimeSpan const & timeout)
: pRWLock(&rwLock)
{
  auto const absTimeout = gpcc::time::TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID) + timeout;

  if (!pRWLock->WriteLock(absTimeout))
    throw TimeoutError("RWLockWriteLocker::RWLockWriteLocker: Timeout acquiring write-lock");
}

} // namespace osal
} // namespace gpcc

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
