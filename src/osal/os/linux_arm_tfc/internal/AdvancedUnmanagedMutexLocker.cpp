/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2022 Daniel Jerolm

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

#ifdef OS_LINUX_ARM_TFC

#ifndef ADVANCEDUNMANAGEDMUTEXLOCKER_HPP_202203221710
#define ADVANCEDUNMANAGEDMUTEXLOCKER_HPP_202203221710

#include "AdvancedUnmanagedMutexLocker.hpp"

namespace gpcc     {
namespace osal     {
namespace internal {

/**
 * \brief Unlocks the @ref UnmanagedMutex managed by this @ref AdvancedUnmanagedMutexLocker.
 *
 * If the @ref AdvancedUnmanagedMutexLocker does not manage a @ref UnmanagedMutex (passed nullptr to constructor or
 * moved responsibility to another @ref AdvancedUnmanagedMutexLocker via move constructor) then
 * this method does nothing.
 *
 * _The managed mutex must be locked when calling this method._
 *
 * Note that the @ref UnmanagedMutex must not necessarily be locked again when the @ref AdvancedUnmanagedMutexLocker
 * is destroyed.
 *
 * ---
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
void AdvancedUnmanagedMutexLocker::Unlock(void) noexcept
{
  if (pMutex != nullptr)
  {
    if (!locked)
      Panic("AdvancedUnmanagedMutexLocker::Unlock(): UnmanagedMutex already unlocked");

    locked = false;
    pMutex->Unlock();
  }
}

/**
 * \brief Relocks the @ref UnmanagedMutex managed by this @ref AdvancedUnmanagedMutexLocker.
 *
 * If the @ref AdvancedUnmanagedMutexLocker does not manage a @ref UnmanagedMutex (passed nullptr to constructor or
 * moved responsibility to another @ref AdvancedUnmanagedMutexLocker via move constructor) then
 * this method does nothing.
 *
 * _The managed mutex must be unlocked when calling this method._
 *
 * Note that the @ref UnmanagedMutex must not necessarily be locked again when the @ref AdvancedUnmanagedMutexLocker
 * is destroyed.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedUnmanagedMutexLocker instance.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
void AdvancedUnmanagedMutexLocker::Relock(void)
{
  if (pMutex != nullptr)
  {
    if (locked)
      Panic("AdvancedUnmanagedMutexLocker::Relock(): UnmanagedMutex already locked");

    pMutex->Lock();
    locked = true;
  }
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // ADVANCEDUNMANAGEDMUTEXLOCKER_HPP_202203221710
#endif // OS_LINUX_ARM_TFC
