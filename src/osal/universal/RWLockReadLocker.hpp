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

#ifndef RWLOCKREADLOCKER_HPP_201806301418
#define RWLOCKREADLOCKER_HPP_201806301418

#include "gpcc/src/osal/RWLock.hpp"

namespace gpcc {

namespace time {
  class TimePoint;
  class TimeSpan;
}

namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING GPCC_RAII
 * \brief Automatic locker/unlocker for RWLocks (see class @ref RWLock).
 *
 * The classes @ref RWLockReadLocker and @ref RWLockWriteLocker are both convenient classes, which allow to
 * automatically lock and unlock RWLocks (see class @ref RWLock) based on the RAII pattern.\n
 * Both classes lock a given @ref RWLock when they are instantiated.\n
 * Both classes unlock the @ref RWLock again when they are released.
 *
 * The classes are intended to be instantiated on the stack. They allow methods to return at any time
 * without forgetting to unlock the @ref RWLock and they ensure that the managed @ref RWLock is unlocked in case of
 * exceptions and deferred thread cancellation.
 *
 * The classes are intended to be used in a multi-threaded environment, but each instance of the
 * classes shall be used within a single thread only. This means that the thread which instantiated an
 * @ref RWLockReadLocker or @ref RWLockWriteLocker also releases it.
 *
 * Using class @ref RWLockReadLocker and @ref RWLockWriteLocker is theoretically for free. It can be completely
 * optimized away by the compiler.
 *
 * @ref RWLockReadLocker and @ref RWLockWriteLocker can be used as function return values.
 */
class RWLockReadLocker final
{
  public:
    RWLockReadLocker(void) = delete;
    explicit RWLockReadLocker(RWLock* const _pRWLock);
    explicit RWLockReadLocker(RWLock& rwLock);
    RWLockReadLocker(RWLock& rwLock, gpcc::time::TimePoint const & absTimeout);
    RWLockReadLocker(RWLock& rwLock, gpcc::time::TimeSpan const & timeout);
    RWLockReadLocker(RWLockReadLocker const &) = delete;
    RWLockReadLocker(RWLockReadLocker&& other) noexcept;
    ~RWLockReadLocker(void);

    RWLockReadLocker& operator=(RWLockReadLocker const &) = delete;
    RWLockReadLocker& operator=(RWLockReadLocker&&) = delete;

  private:
    /// Pointer to the managed @ref RWLock. nullptr = none (e.g. stolen by move constructor).
    RWLock* pRWLock;
};

/**
 * \brief Constructor. Creates a @ref RWLockReadLocker instance and read-locks the managed @ref RWLock.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _pRWLock
 * Pointer to the @ref RWLock that shall be managed by this @ref RWLockReadLocker. \n
 * The @ref RWLock will be read-locked when the @ref RWLockReadLocker is instantiated and the read-lock will
 * be released when the @ref RWLockReadLocker is destroyed.\n
 * If this is nullptr, then the @ref RWLockReadLocker will be passive.
 */
inline RWLockReadLocker::RWLockReadLocker(RWLock* const _pRWLock)
: pRWLock(_pRWLock)
{
  if (pRWLock != nullptr)
    pRWLock->ReadLock();
}

/**
 * \brief Constructor. Creates a @ref RWLockReadLocker instance and read-locks the managed @ref RWLock.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
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
 */
inline RWLockReadLocker::RWLockReadLocker(RWLock& rwLock)
: pRWLock(&rwLock)
{
  pRWLock->ReadLock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref RWLock is moved from another @ref RWLockReadLocker to the newly
 * constructed @ref RWLockReadLocker.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * The responsibility to release the @ref RWLock is moved from the referenced @ref RWLockReadLocker `other` to the
 * new constructed @ref RWLockReadLocker. After moving, the @ref RWLockReadLocker referenced by `other` will behave passive.
 */
inline RWLockReadLocker::RWLockReadLocker(RWLockReadLocker&& other) noexcept
: pRWLock(other.pRWLock)
{
  other.pRWLock = nullptr;
}

/**
 * \brief Destructor. Unlocks the managed @ref RWLock.
 *
 * If the @ref RWLockReadLocker is passive, then this does nothing.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref RWLockReadLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
inline RWLockReadLocker::~RWLockReadLocker(void)
{
  if (pRWLock != nullptr)
    pRWLock->ReleaseReadLock();
}

} // namespace osal
} // namespace gpcc

#endif // RWLOCKREADLOCKER_HPP_201806301418

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
