/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include "ThreadBlocker.hpp"
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "TFCCore.hpp"
#include "UnmanagedMutexLocker.hpp"
#include <stdexcept>

namespace gpcc {
namespace osal {
namespace internal {

/**
 * \brief Constructor.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
ThreadBlocker::ThreadBlocker(void)
: ThreadBlockerBase()
, pTFCCore(TFCCore::Get())
, signaled(false)
, blocked(false)
, signaledCV()
{
}

/**
 * \brief Destructor.
 *
 * No thread must be blocked on this @ref ThreadBlocker.
 *
 * ---
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
ThreadBlocker::~ThreadBlocker(void)
{
  if (blocked)
    PANIC();
}

/// \copydoc ThreadBlockerBase::Signal
void ThreadBlocker::Signal(void)
{
  if (signaled)
    throw std::runtime_error("ThreadBlocker::Signal: Double signal");

  signaledCV.Signal();
  signaled = true;

  if (blocked)
    pTFCCore->ReportThreadAboutToWakeUp();
}

/**
 * \brief Blocks the calling thread and unlocks a given locked _unmanaged_ mutex while the thread is blocked.
 *
 * The given _unmanaged_ mutex is always relocked before the method returns, even in case of an exception or
 * deferred thread cancellation.
 *
 * There must be no more than one thread blocked at any time.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * Basic guarantee:\n
 * - Emulated system time may be incremented
 *
 * Except from this, this method provides the strong guarantee:
 * - Even in case of an exception, `mutexToBeUnlocked` will be re-locked.
 * - No leaks or inconsistencies in case of an exception
 *
 * __Thread-cancellation-safety:__\n
 * Deferred cancellation is safe, but:
 * - Emulated system time may be incremented
 *
 * ---
 *
 * \param mutexToBeUnlocked
 * The referenced _unmanaged_ mutex will be unlocked if this method blocks.\n
 * The referenced _unmanaged_ mutex is guaranteed to be re-locked before this method returns (either
 * normally, due to an exception, or due to deferred thread cancellation).
 */
void ThreadBlocker::Block(Mutex& mutexToBeUnlocked)
{
  if (!mutexToBeUnlocked.locked)
    throw std::logic_error("ThreadBlocker::Block: mutexToBeUnlocked not locked");

  if (blocked)
    throw std::logic_error("ThreadBlocker::Block: There is already a thread blocked");

  if (!signaled)
  {
    blocked = true;
    ON_SCOPE_EXIT(clearBlocked) { blocked = false; };

    mutexToBeUnlocked.InternalUnlock();
    ON_SCOPE_EXIT(RelockMutex) { mutexToBeUnlocked.InternalLock(); };

    pTFCCore->ReportThreadPermanentlyBlockedBegin(); // <-- note: may increment emulated system time!
    ON_SCOPE_EXIT(BlockEnd) { pTFCCore->ReportThreadPermanentlyBlockedEnd(); };

    try
    {
      while (!signaled)
        signaledCV.Wait(pTFCCore->GetBigLock());
    }
    catch (...)
    {
      if (!signaled)
        pTFCCore->ReportThreadAboutToWakeUp();
      throw;
    }
  } // if (!signaled)
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
