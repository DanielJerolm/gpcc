/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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
