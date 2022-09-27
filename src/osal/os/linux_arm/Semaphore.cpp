/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM

#include <gpcc/osal/Semaphore.hpp>
#include <gpcc/osal/Panic.hpp>
#include <stdexcept>
#include <system_error>

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
  if (initialValue > MAX)
    throw std::invalid_argument("Semaphore::Semaphore: initialValue too large");

  int const status = sem_init(&semaphore, 0, static_cast<unsigned int>(initialValue));
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "sem_init(...) failed");
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
  if (sem_destroy(&semaphore) != 0)
    PANIC();
}

/**
 * \brief Increments (posts) the semaphore.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Semaphore::Post(void)
{
  int const status = sem_post(&semaphore);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "sem_post(...) failed");
}

/**
 * \brief Decrements the semaphore's counter if it is greater than zero or waits if the counter is zero.
 *
 * The semaphore's counter cannot become negative. If the counter is already zero, then it can't be decremented any
 * more and the thread will be blocked until another thread invokes @ref Post().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.\n
 * On some systems, this method contains a cancellation point.
 */
void Semaphore::Wait(void)
{
  int const status = sem_wait(&semaphore);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "sem_wait(...) failed");
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM
