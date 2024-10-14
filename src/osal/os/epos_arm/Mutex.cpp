/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_EPOS_ARM

#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Panic.hpp>

namespace gpcc {
namespace osal {

/**
 * \brief Constructor. Creates a new (unlocked) @ref Mutex object.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
Mutex::Mutex(void)
{
  epos_mutex_Init(&mutex);
}

/**
 * \brief Destructor.
 *
 * \pre   The mutex must not be locked by any thread.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Mutex::~Mutex(void)
{
  if (epos_mutex_IsLocked(&mutex))
    Panic("Mutex::~Mutex: Mutex is locked");
}

/**
 * \brief Unlocks the mutex.
 *
 * \pre   The mutex must be the latest (most recent) mutex locked by the calling thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This must only be invoked by the thread who has locked the mutex.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Mutex::Unlock(void) noexcept
{
  epos_mutex_Unlock(&mutex);
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_EPOS_ARM
