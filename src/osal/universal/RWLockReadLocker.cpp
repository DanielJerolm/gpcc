/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018, 2022 Daniel Jerolm

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

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#include "RWLockReadLocker.hpp"
#include "gpcc/src/osal/exceptions.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"

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
