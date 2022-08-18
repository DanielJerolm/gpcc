/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include "Mutex.hpp"
#include "Panic.hpp"
#include "internal/TFCCore.hpp"
#include "internal/UnmanagedMutexLocker.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <pthread.h>

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
: pTFCCore(internal::TFCCore::Get())
, locked(false)
, thread_id()
, nbOfblockedThreads(0)
, blockedThreadIsGoingToWakeUp(false)
, unlockedCV()
{
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
  internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());
  if (locked)
    Panic("Mutex::~Mutex: Mutex still locked");
}

/**
 * \brief Locks the mutex.
 *
 * If the mutex is already locked by another thread, then this method will block until the other thread unlocks the
 * mutex and this thread acquires the mutex.
 *
 * \pre   The mutex must not yet be acquired by the calling thread.
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
void Mutex::Lock(void)
{
  internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());
  InternalLock();
}

/**
 * \brief Tries to lock the mutex.
 *
 * Same as @ref Lock(), but returns immediately if the mutex is already locked by the calling thread or by
 * another thread.
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
 *
 * - - -
 *
 * \retval true
 *    The mutex has been locked by the calling thread.
 * \retval false
 *    The mutex is already locked by the calling thread __or__ by another thread.
 */
bool Mutex::TryLock(void)
{
  internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());

  if (locked)
    return false;

  if (blockedThreadIsGoingToWakeUp)
  {
    // we will steal the lock from the thread which currently about to acquire the lock
    blockedThreadIsGoingToWakeUp = false;
    pTFCCore->UndoReportThreadAboutToWakeUp();
  }

  locked = true;
  thread_id = pthread_self();
  return true;
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
  try
  {
    internal::UnmanagedMutexLocker mutexLocker(pTFCCore->GetBigLock());

    InternalUnlock();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Locks the mutex (with TFC big-lock already acquired).
 *
 * If the mutex is already locked by another thread, then this method blocks until the other thread unlocks the mutex
 * and this thread acquires the mutex.
 *
 * \pre   The mutex must not yet be acquired by the calling thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * TFC's big lock must be locked by the caller.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Mutex::InternalLock(void)
{
  if (locked)
  {
    if (pthread_equal(thread_id, pthread_self()) != 0)
      Panic("Mutex::InternalLock: The calling thread has the mutex already locked");

    pTFCCore->ReportThreadPermanentlyBlockedBegin();
    nbOfblockedThreads++;
    ON_SCOPE_EXIT(ReportBlockedEnd)
    {
      nbOfblockedThreads--;
      pTFCCore->ReportThreadPermanentlyBlockedEnd();
    };

    // disable thread cancellation temporarily
    int oldState;
    if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState) != 0)
      PANIC();
    ON_SCOPE_EXIT(RecoverCancelState)
    {
      if (pthread_setcancelstate(oldState, nullptr) != 0)
        PANIC();
    };

    while (locked)
      unlockedCV.Wait(pTFCCore->GetBigLock());

    blockedThreadIsGoingToWakeUp = false;
  }
  else
  {
    if (blockedThreadIsGoingToWakeUp)
    {
      // we will steal the lock from the thread which currently about to acquire the lock
      blockedThreadIsGoingToWakeUp = false;
      pTFCCore->UndoReportThreadAboutToWakeUp();
    }
  }

  locked = true;
  thread_id = pthread_self();
}

/**
 * \brief Unlocks the mutex (with TFC big-lock already acquired).
 *
 * \pre   The mutex must be locked by the calling thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * TFC's big lock must be locked by the caller.\n
 * This must only be invoked by the thread who has locked the mutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Mutex::InternalUnlock(void)
{
  if (!locked)
    throw std::logic_error("Mutex::InternalUnlock: Not locked");

  if (blockedThreadIsGoingToWakeUp)
    PANIC();

  if (pthread_equal(thread_id, pthread_self()) == 0)
    Panic("Mutex::InternalUnlock: The calling thread is not the one which has locked the mutex");

  unlockedCV.Signal();
  locked = false;
  if (nbOfblockedThreads != 0)
  {
    blockedThreadIsGoingToWakeUp = true;
    pTFCCore->ReportThreadAboutToWakeUp();
  }
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
