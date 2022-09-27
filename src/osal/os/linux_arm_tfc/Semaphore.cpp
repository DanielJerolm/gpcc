/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include <gpcc/osal/Semaphore.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "internal/TFCCore.hpp"
#include "internal/UnmanagedConditionVariable.hpp"
#include "internal/UnmanagedMutexLocker.hpp"
#include <stdexcept>

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
: pTFCCore(internal::TFCCore::Get())
, v(0)
, blockedThreads(0)
, threadsToBeReleased(0)
, spFreeCV(std::make_unique<internal::UnmanagedConditionVariable>())
{
  if (initialValue > MAX)
    throw std::invalid_argument("Semaphore::Semaphore: initialValue too large");

  v = static_cast<int32_t>(initialValue);
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
  internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());
  if ((v < 0) || (blockedThreads != 0) || (threadsToBeReleased != 0))
    Panic("Semaphore::~Semaphore: At least one thread blocked on semaphore");
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
  internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());

  if (v == MAX)
    throw std::runtime_error("Semaphore::Post: maximum reached, cannot post any more");

  // at least one thread blocked?
  if (v < 0)
  {
    if (threadsToBeReleased == 0)
      spFreeCV->Signal();
    v++;
    threadsToBeReleased++;
    pTFCCore->ReportThreadAboutToWakeUp();
  }
  else
    v++;
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
  // The doxygen comment above is a standard comment, which is valid for all semaphore implementations provided by
  // GPCC's OSAL. Of course under the hood "v" can become negative, because it has two meanings in this semaphore
  // implementation:
  // If "v" is zero or positive, then it has the meaning of the semaphore's value.
  // If "v" is negative, then it has the meaning of a counter which counts the number of blocked threads. In detail,
  // it counts the number of blocked threads NOT about to wake up. From the outside's point of view, the semaphore's
  // value must be considered zero, though it is negative.

  internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());

  v--;

  // thread blocked?
  if (v < 0)
  {
    blockedThreads++;
    ON_SCOPE_EXIT(BlockedThreads) { blockedThreads--; };

    pTFCCore->ReportThreadPermanentlyBlockedBegin();
    ON_SCOPE_EXIT(BlockedEnd) { pTFCCore->ReportThreadPermanentlyBlockedEnd(); };

    // wait until at least one thread can be released
    try
    {
      while (threadsToBeReleased == 0)
        spFreeCV->Wait(pTFCCore->GetBigLock());
    }
    catch (...)
    {
      // No threads to be released?
      // Then we experienced an error or deferred thread cancellation.
      if (threadsToBeReleased == 0)
      {
        // recover semaphore's value
        v++;

        // prepare for call to ReportThreadPermanentlyBlockedEnd() in ON_SCOPE_EXIT above
        pTFCCore->ReportThreadAboutToWakeUp();
      }
      else
      {
        // At least one thread shall be released.
        // This means that we experienced an error or deferred thread cancellation AND
        // someone has posted the semaphore in parallel.

        if (blockedThreads == threadsToBeReleased)
        {
          // All blocked threads shall be woken up.
          // This case is simple. We just consume the "post" and wake-up another thread.
          threadsToBeReleased--;
          if (threadsToBeReleased != 0)
            Signal_freeCV();
        }
        else if (blockedThreads > threadsToBeReleased)
        {
          // There are more blocked threads than threads that shall be woken up.
          // In this case we recover "v" and we DO NOT consume the "post". Instead we grant it to
          // another blocked thread. This is OK, because this thread has been woken up due to
          // an error or deferred thread cancellation.
          v++;
          Signal_freeCV();

          // prepare for call to ReportThreadPermanentlyBlockedEnd() in ON_SCOPE_EXIT above
          pTFCCore->ReportThreadAboutToWakeUp();
        }
        else
        {
          PANIC(); // blockedThreads < threadsToBeReleased
        }
      } // if (threadsToBeReleased == 0)... else...

      throw;
    } // catch (...)

    // Thread has been woken up. Wake-up the next thread if at least one more can be released.
    threadsToBeReleased--;
    if (threadsToBeReleased != 0)
      Signal_freeCV();
  } // if (v < 0)
}

/**
 * \brief Signals the condition variable @ref spFreeCV
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 */
void Semaphore::Signal_freeCV(void) noexcept
{
  try
  {
    spFreeCV->Signal();
  }
  catch (...)
  {
    PANIC();
  }
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
