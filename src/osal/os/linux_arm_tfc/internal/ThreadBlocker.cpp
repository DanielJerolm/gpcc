/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifdef OS_LINUX_ARM_TFC

#include "ThreadBlocker.hpp"
#include "TFCCore.hpp"
#include "UnmanagedMutexLocker.hpp"
#include "../Mutex.hpp"
#include "../Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
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
