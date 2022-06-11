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

#ifndef ADVANCEDUNMANAGEDMUTEXLOCKER_HPP_201702261542
#define ADVANCEDUNMANAGEDMUTEXLOCKER_HPP_201702261542

#include "UnmanagedMutex.hpp"
#include "UnmanagedMutexLocker.hpp"
#include "../Panic.hpp"

namespace gpcc     {
namespace osal     {
namespace internal {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Automatic locker/unlocker for unmanaged mutexes (class @ref UnmanagedMutex).
 *
 * This class provides the same functionality like class @ref AdvancedMutexLocker, but this one is intended
 * to be used with class @ref UnmanagedMutex.
 */
class AdvancedUnmanagedMutexLocker final
{
  public:
    AdvancedUnmanagedMutexLocker(void) = delete;
    explicit AdvancedUnmanagedMutexLocker(UnmanagedMutex* const _pMutex);
    explicit AdvancedUnmanagedMutexLocker(UnmanagedMutex& mutex);
    AdvancedUnmanagedMutexLocker(AdvancedUnmanagedMutexLocker const &) = delete;
    AdvancedUnmanagedMutexLocker(AdvancedUnmanagedMutexLocker&& other) noexcept;
    AdvancedUnmanagedMutexLocker(UnmanagedMutexLocker&& other) noexcept;
    ~AdvancedUnmanagedMutexLocker(void);

    AdvancedUnmanagedMutexLocker& operator=(AdvancedUnmanagedMutexLocker const &) = delete;
    AdvancedUnmanagedMutexLocker& operator=(AdvancedUnmanagedMutexLocker&&) = delete;

    void Unlock(void) noexcept;
    void Relock(void);
    bool IsLocked(void) const noexcept;

  private:
    /// Pointer to the managed @ref UnmanagedMutex. nullptr = no mutex.
    UnmanagedMutex* pMutex;

    /// Lock-Flag.
    /** true =  UnmanagedMutex locked.\n
        false = UnmanagedMutex NOT locked. */
    bool locked;
};

/**
 * \brief Constructor. Creates an @ref AdvancedUnmanagedMutexLocker instance and locks the given @ref UnmanagedMutex.
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
 * Pointer to the @ref UnmanagedMutex that shall be managed by this @ref AdvancedUnmanagedMutexLocker. \n
 * The @ref UnmanagedMutex is locked when the @ref AdvancedUnmanagedMutexLocker is instantiated and the
 * @ref UnmanagedMutex is released when the @ref AdvancedUnmanagedMutexLocker is destroyed.\n
 * If this is nullptr, then the @ref AdvancedUnmanagedMutexLocker will be passive and
 * any call to @ref Relock() and @ref Unlock() will be ignored.
 */
inline AdvancedUnmanagedMutexLocker::AdvancedUnmanagedMutexLocker(UnmanagedMutex* const _pMutex)
: pMutex(_pMutex)
, locked(_pMutex != nullptr)
{
  if (locked)
    pMutex->Lock();
}

/**
 * \brief Constructor. Creates an @ref AdvancedUnmanagedMutexLocker instance and locks the given @ref UnmanagedMutex.
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
 * Reference to the @ref UnmanagedMutex that shall be managed by this @ref AdvancedUnmanagedMutexLocker. \n
 * The @ref UnmanagedMutex is locked when the @ref AdvancedUnmanagedMutexLocker is instantiated and the
 * @ref UnmanagedMutex is released when the @ref AdvancedUnmanagedMutexLocker is destroyed.
 */
inline AdvancedUnmanagedMutexLocker::AdvancedUnmanagedMutexLocker(UnmanagedMutex& mutex)
: pMutex(&mutex)
, locked(true)
{
  pMutex->Lock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref UnmanagedMutex is moved from another @ref AdvancedUnmanagedMutexLocker
 * to the newly constructed @ref AdvancedUnmanagedMutexLocker.
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
 * The responsibility to release the @ref UnmanagedMutex is moved from the referenced @ref AdvancedUnmanagedMutexLocker
 * `other` to the new constructed @ref AdvancedUnmanagedMutexLocker. After moving, the
 * @ref AdvancedUnmanagedMutexLocker referenced by `other` will behave passive.
 */
inline AdvancedUnmanagedMutexLocker::AdvancedUnmanagedMutexLocker(AdvancedUnmanagedMutexLocker&& other) noexcept
: pMutex(other.pMutex)
, locked(other.locked)
{
  other.pMutex = nullptr;
  other.locked = false;
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref UnmanagedMutex is moved from an @ref UnmanagedMutexLocker to the newly
 * constructed @ref AdvancedUnmanagedMutexLocker.
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
 * `other` to the new constructed @ref AdvancedUnmanagedMutexLocker. After moving, the @ref UnmanagedMutexLocker
 * referenced by `other` will behave passive.
 */
inline AdvancedUnmanagedMutexLocker::AdvancedUnmanagedMutexLocker(UnmanagedMutexLocker&& other) noexcept
: pMutex(other.pMutex)
, locked(other.pMutex != nullptr)
{
  other.pMutex = nullptr;
}

/**
 * \brief Destructor. Unlocks the managed @ref UnmanagedMutex if it is locked and if the
 * @ref AdvancedUnmanagedMutexLocker is not passive.
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedUnmanagedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
inline AdvancedUnmanagedMutexLocker::~AdvancedUnmanagedMutexLocker(void)
{
  if (locked)
    pMutex->Unlock();
}

inline bool AdvancedUnmanagedMutexLocker::IsLocked(void) const noexcept
/**
 * \brief Retrieves if this @ref AdvancedUnmanagedMutexLocker instance has its managed @ref UnmanagedMutex
 * currently locked.
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedUnmanagedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * true  = UnmanagedMutex is locked.\n
 * false = UnmanagedMutex is not locked.
 */
{
  return locked;
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef ADVANCEDUNMANAGEDMUTEXLOCKER_HPP_201702261542
#endif // #ifdef OS_LINUX_X64_TFC
