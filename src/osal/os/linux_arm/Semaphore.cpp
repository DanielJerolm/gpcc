/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2022 Daniel Jerolm

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

#ifdef OS_LINUX_ARM

#include "Semaphore.hpp"
#include "Panic.hpp"
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
