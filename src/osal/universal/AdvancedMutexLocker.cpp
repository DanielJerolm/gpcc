/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2022 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>

namespace gpcc {
namespace osal {

/**
 * \brief Unlocks the @ref Mutex managed by this @ref AdvancedMutexLocker.
 *
 * If the @ref AdvancedMutexLocker does not manage a @ref Mutex (passed nullptr to constructor or
 * moved responsibility to another @ref AdvancedMutexLocker via move constructor) then
 * this method does nothing.
 *
 * _The managed mutex must be locked when calling this method._
 *
 * Note that the @ref Mutex must not necessarily be locked again when the @ref AdvancedMutexLocker is destroyed.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedMutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
void AdvancedMutexLocker::Unlock(void) noexcept
{
  if (pMutex != nullptr)
  {
    if (!locked)
      Panic("AdvancedMutexLocker::Unlock(): Mutex already unlocked");

    locked = false;
    pMutex->Unlock();
  }
}

/**
 * \brief Relocks the @ref Mutex managed by this @ref AdvancedMutexLocker.
 *
 * If the @ref AdvancedMutexLocker does not manage a @ref Mutex (passed nullptr to constructor or
 * moved responsibility to another @ref AdvancedMutexLocker via move constructor) then
 * this method does nothing.
 *
 * _The managed mutex must be unlocked when calling this method._
 *
 * Note that the @ref Mutex must not necessarily be locked again when the @ref AdvancedMutexLocker is destroyed.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref AdvancedMutexLocker instance.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
void AdvancedMutexLocker::Relock(void)
{
  if (pMutex != nullptr)
  {
    if (locked)
      Panic("AdvancedMutexLocker::Relock(): Mutex already locked");

    pMutex->Lock();
    locked = true;
  }
}

} // namespace osal
} // namespace gpcc

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
