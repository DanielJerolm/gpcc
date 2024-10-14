/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_EPOS_ARM

#include <gpcc/osal/Semaphore.hpp>
#include <gpcc/osal/Panic.hpp>

namespace gpcc {
namespace osal {

size_t const Semaphore::MAX;

/**
 * \brief Constructor. Creates a semaphore with a configurable initial value.
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
 * \param initialValue
 * Initial value for the semaphore's counter.
 */
Semaphore::Semaphore(size_t const initialValue)
{
  epos_semaphore_Init(&sem, initialValue);
}

/**
 * \brief Destructor.
 *
 * \pre   No thread must be blocked on the semaphore.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Semaphore::~Semaphore(void)
{
  if (epos_semaphore_IsAnyThreadBlocked(&sem))
    Panic("Semaphore::~Semaphore: Blocked threads");
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_EPOS_ARM
