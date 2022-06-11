/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifdef OS_LINUX_X64_TFC

#ifndef UNMANAGEDMUTEXLOCKER_HPP_201702252258
#define UNMANAGEDMUTEXLOCKER_HPP_201702252258

#include "UnmanagedMutex.hpp"

namespace gpcc {
namespace osal {
namespace internal {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Automatic locker/unlocker for unmanaged mutexes (class @ref UnmanagedMutex).
 *
 * This class provides the same functionality like class @ref MutexLocker, but this one is intended
 * to be used with class @ref UnmanagedMutex.
 */
class UnmanagedMutexLocker final
{
    friend class AdvancedUnmanagedMutexLocker;

  public:
    UnmanagedMutexLocker(void) = delete;
    explicit UnmanagedMutexLocker(UnmanagedMutex* const _pMutex);
    explicit UnmanagedMutexLocker(UnmanagedMutex& mutex);
    UnmanagedMutexLocker(UnmanagedMutexLocker const &) = delete;
    UnmanagedMutexLocker(UnmanagedMutexLocker&& other) noexcept;
    ~UnmanagedMutexLocker(void);

    UnmanagedMutexLocker& operator=(UnmanagedMutexLocker const &) = delete;
    UnmanagedMutexLocker& operator=(UnmanagedMutexLocker&&) = delete;

  private:
    /// Pointer to the @ref UnmanagedMutex managed by this class. nullptr = none (e.g. stolen by move constructor).
    UnmanagedMutex* pMutex;
};

/**
 * \brief Constructor. Creates an @ref UnmanagedMutexLocker instance and locks the given @ref UnmanagedMutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _pMutex
 * Pointer to the @ref UnmanagedMutex that shall be managed by this @ref UnmanagedMutexLocker. \n
 * The @ref UnmanagedMutex is locked when the @ref UnmanagedMutexLocker is instantiated and the @ref UnmanagedMutex
 * is released when the @ref UnmanagedMutexLocker is destroyed.\n
 * If this is nullptr, then the @ref UnmanagedMutexLocker will be passive.
 */
inline UnmanagedMutexLocker::UnmanagedMutexLocker(UnmanagedMutex* const _pMutex)
: pMutex(_pMutex)
{
  if (pMutex != nullptr)
    pMutex->Lock();
}

/**
 * \brief Constructor. Creates an @ref UnmanagedMutexLocker instance and locks the given @ref UnmanagedMutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param mutex
 * Reference to the @ref UnmanagedMutex instance that shall be managed by this @ref UnmanagedMutexLocker. \n
 * The @ref UnmanagedMutex is locked when the @ref UnmanagedMutexLocker is instantiated and the @ref UnmanagedMutex
 * is released when the @ref UnmanagedMutexLocker is destroyed.
 */
inline UnmanagedMutexLocker::UnmanagedMutexLocker(UnmanagedMutex& mutex)
: pMutex(&mutex)
{
  pMutex->Lock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref UnmanagedMutex is moved from another @ref UnmanagedMutexLocker to the newly
 * constructed @ref UnmanagedMutexLocker.
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * The responsibility to release the @ref UnmanagedMutex is moved from the referenced @ref UnmanagedMutexLocker
 * `other` to the new constructed @ref UnmanagedMutexLocker. After moving, the @ref UnmanagedMutexLocker referenced
 * by `other` will behave passive.
 */
inline UnmanagedMutexLocker::UnmanagedMutexLocker(UnmanagedMutexLocker&& other) noexcept
: pMutex(other.pMutex)
{
  other.pMutex = nullptr;
}

inline UnmanagedMutexLocker::~UnmanagedMutexLocker(void)
/**
 * \brief Destructor. Unlocks the @ref UnmanagedMutex managed by the @ref UnmanagedMutexLocker instance.
 *
 * If the @ref UnmanagedMutexLocker instance is passive, then this does nothing.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref UnmanagedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (pMutex != nullptr)
    pMutex->Unlock();
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef UNMANAGEDMUTEXLOCKER_HPP_201702252258
#endif // #ifdef OS_LINUX_X64_TFC
