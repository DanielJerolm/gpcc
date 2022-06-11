/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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

#include "NonBlockingSemaphore.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <stdexcept>

namespace gpcc               {
namespace ResourceManagement {
namespace semaphores         {

size_t const NonBlockingSemaphore::MAX;

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param initialValue
 * Initial value for the semaphore's counter.
 */
NonBlockingSemaphore::NonBlockingSemaphore(size_t const initialValue)
: mutex()
, cnt(initialValue)
, usersBlockedOnWait()
{
}

/**
 * \brief Destructor.
 *
 * \pre   There is no thread waiting for a callback after invocation of @ref Wait().
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
NonBlockingSemaphore::~NonBlockingSemaphore(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  if (!usersBlockedOnWait.empty())
    gpcc::osal::Panic("NonBlockingSemaphore::~NonBlockingSemaphore: At least one waiting thread.");
}

/**
 * \brief Increments (posts) the semaphore.
 *
 * \pre   The semaphore's counter is not at its maximum value (@ref MAX).
 *
 * \post  The semaphore's counter is either incremented or a thread waiting for decrement/acquisition of the semaphore
 *        receives its callback and acquires the semaphore.
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
void NonBlockingSemaphore::Post(void)
{
  std::unique_ptr<tSemAcquiredCallback> spCallback;

  {
    gpcc::osal::MutexLocker mutexLocker(mutex);
    if (usersBlockedOnWait.empty())
    {
      if (cnt == MAX)
        throw std::logic_error("NonBlockingSemaphore::Post: Cannot increment counter any more.");

      ++cnt;
    }
    else
    {
      spCallback = std::move(usersBlockedOnWait.front());
      usersBlockedOnWait.pop_front();
    }
  }

  if (spCallback)
  {
    try
    {
      (*spCallback)();
    }
    catch (...)
    {
      PANIC();
    }
  }
}

/**
 * \brief Decrements the semaphore's counter if it is greater than zero. Otherwise the provided callback will be stored
 *        and invoked when the semaphore is incremented (@ref Post()) at a later point in time.
 *
 * The semaphore's counter cannot become negative. If the counter is already zero, then it can't be decremented any
 * more. If this would be a synchronous implementation, then the calling thread would be blocked until another thread
 * invokes @ref Post(). However, in this implementation, the thread will not be blocked and the provided callback will
 * be stored. It will be invoked when someone increments the semaphore by calling @ref Post().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param cb
 * If the semaphore cannot be decremented/acquired now, then the function referenced by this will be invoked later after
 * someone has invoked @ref Post() and the caller of this has acquired the semaphore.
 *
 * \retval true   The semaphore has been decremented/acquired. There will be no callback.
 *
 * \retval false  The semaphore has not been decremented/acquired. The callback will be invoked at a later point in
 *                time when the semaphore is acquired/decremented.
 */
bool NonBlockingSemaphore::Wait(tSemAcquiredCallback const & cb)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (!cb)
    throw std::invalid_argument("NonBlockingSemaphore::Wait: !cb");

  if (cnt > 0U)
  {
    --cnt;
    return true;
  }
  else
  {
    usersBlockedOnWait.emplace_back(std::make_unique<tSemAcquiredCallback>(cb));
    return false;
  }
}

} // namespace semaphores
} // namespace ResourceManagement
} // namespace gpcc
