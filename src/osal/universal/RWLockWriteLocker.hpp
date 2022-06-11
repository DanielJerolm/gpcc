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

#ifndef RWLOCKWRITELOCKER_HPP_201806301438
#define RWLOCKWRITELOCKER_HPP_201806301438

#include "gpcc/src/osal/RWLock.hpp"

namespace gpcc {

namespace time {
  class TimePoint;
  class TimeSpan;
}

namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING GPCC_RAII
 * \copydoc RWLockReadLocker
 */
class RWLockWriteLocker final
{
  public:
    RWLockWriteLocker(void) = delete;
    explicit RWLockWriteLocker(RWLock* const _pRWLock);
    explicit RWLockWriteLocker(RWLock& rwLock);
    RWLockWriteLocker(RWLock& rwLock, gpcc::time::TimePoint const & absTimeout);
    RWLockWriteLocker(RWLock& rwLock, gpcc::time::TimeSpan const & timeout);
    RWLockWriteLocker(RWLockWriteLocker const &) = delete;
    RWLockWriteLocker(RWLockWriteLocker&& other) noexcept;
    ~RWLockWriteLocker(void);

    RWLockWriteLocker& operator=(RWLockWriteLocker const &) = delete;
    RWLockWriteLocker& operator=(RWLockWriteLocker&&) = delete;

  private:
    /// Pointer to the managed @ref RWLock. nullptr = none (e.g. stolen by move constructor).
    RWLock* pRWLock;
};

/**
 * \brief Constructor. Creates a @ref RWLockWriteLocker instance and write-locks the managed @ref RWLock.
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
 * Pointer to the @ref RWLock that shall be managed by this @ref RWLockWriteLocker. \n
 * The @ref RWLock will be write-locked when the @ref RWLockWriteLocker is instantiated and the write-lock will
 * be released when the @ref RWLockWriteLocker is destroyed.\n
 * If this is nullptr, then the @ref RWLockWriteLocker will be passive.
 */
inline RWLockWriteLocker::RWLockWriteLocker(RWLock* const _pRWLock)
: pRWLock(_pRWLock)
{
  if (pRWLock != nullptr)
    pRWLock->WriteLock();
}

/**
 * \brief Constructor. Creates a @ref RWLockWriteLocker instance and write-locks the managed @ref RWLock.
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
 * Reference to the @ref RWLock instance that shall be managed by this @ref RWLockWriteLocker. \n
 * The @ref RWLock will be write-locked when the @ref RWLockWriteLocker is instantiated and the write-lock will
 * be released when the @ref RWLockWriteLocker is destroyed.
 */
inline RWLockWriteLocker::RWLockWriteLocker(RWLock& rwLock)
: pRWLock(&rwLock)
{
  pRWLock->WriteLock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref RWLock is moved from another @ref RWLockWriteLocker to the newly
 * constructed @ref RWLockWriteLocker.
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
 * The responsibility to release the @ref RWLock is moved from the referenced @ref RWLockWriteLocker `other` to the
 * new constructed @ref RWLockWriteLocker. After moving, the @ref RWLockWriteLocker referenced by `other` will behave passive.
 */
inline RWLockWriteLocker::RWLockWriteLocker(RWLockWriteLocker&& other) noexcept
: pRWLock(other.pRWLock)
{
  other.pRWLock = nullptr;
}

/**
 * \brief Destructor. Unlocks the managed @ref RWLock.
 *
 * If the @ref RWLockWriteLocker is passive, then this does nothing.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref RWLockWriteLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
inline RWLockWriteLocker::~RWLockWriteLocker(void)
{
  if (pRWLock != nullptr)
    pRWLock->ReleaseWriteLock();
}

} // namespace osal
} // namespace gpcc

#endif // RWLOCKWRITELOCKER_HPP_201806301438

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
