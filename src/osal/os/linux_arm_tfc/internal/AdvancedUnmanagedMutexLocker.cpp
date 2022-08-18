/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2022 Daniel Jerolm
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
