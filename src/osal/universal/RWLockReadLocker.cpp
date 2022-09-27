/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#include <gpcc/osal/RWLockReadLocker.hpp>
#include <gpcc/osal/exceptions.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>

namespace gpcc {
namespace osal {

/**
 * \brief Constructor. Creates a @ref RWLockReadLocker instance and read-locks the managed @ref RWLock. \n
 *        The time waiting for acquisition of the read-lock is limited by a timeout (absolute value).
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws TimeoutError   Timeout while waiting for acquisition of the read-lock ([details](@ref gpcc::osal::TimeoutError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rwLock
 * Reference to the @ref RWLock instance that shall be managed by this @ref RWLockReadLocker. \n
 * The @ref RWLock will be read-locked when the @ref RWLockReadLocker is instantiated and the read-lock will
 * be released when the @ref RWLockReadLocker is destroyed.
 * \param absTimeout
 * Timeout specified as an absolute point in time when waiting to acquire the read-lock.\n
 * _This must be specified using Clocks::monotonic._
 */
RWLockReadLocker::RWLockReadLocker(RWLock& rwLock, gpcc::time::TimePoint const & absTimeout)
: pRWLock(&rwLock)
{
  if (!pRWLock->ReadLock(absTimeout))
    throw TimeoutError("RWLockReadLocker::RWLockReadLocker: Timeout acquiring read-lock");
}

/**
 * \brief Constructor. Creates a @ref RWLockReadLocker instance and read-locks the managed @ref RWLock. \n
 *        The time waiting for acquisition of the read-lock is limited by a timeout (relative value).
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws TimeoutError   Timeout while waiting for acquisition of the read-lock ([details](@ref gpcc::osal::TimeoutError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rwLock
 * Reference to the @ref RWLock instance that shall be managed by this @ref RWLockReadLocker. \n
 * The @ref RWLock will be read-locked when the @ref RWLockReadLocker is instantiated and the read-lock will
 * be released when the @ref RWLockReadLocker is destroyed.
 * \param timeout
 * Timespan specifying the timeout when waiting to acquire the read-lock.
 */
RWLockReadLocker::RWLockReadLocker(RWLock& rwLock, gpcc::time::TimeSpan const & timeout)
: pRWLock(&rwLock)
{
  auto const absTimeout = gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::monotonic) + timeout;

  if (!pRWLock->ReadLock(absTimeout))
    throw TimeoutError("RWLockReadLocker::RWLockReadLocker: Timeout acquiring read-lock");
}

} // namespace osal
} // namespace gpcc

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
