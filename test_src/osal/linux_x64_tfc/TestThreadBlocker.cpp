/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#ifdef OS_LINUX_X64_TFC

#include "gpcc/src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include "gpcc/src/osal/os/linux_x64_tfc/internal/ThreadBlocker.hpp"
#include "gpcc/src/osal/os/linux_x64_tfc/internal/UnmanagedMutex.hpp"
#include "gpcc/src/osal/os/linux_x64_tfc/internal/UnmanagedMutexLocker.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gtest/gtest.h"
#include <unistd.h>


// Sleeptime in ms for the main thread to allow the helper thread to run into the UUT's blocking method.
#define SLEEPTIME_MS          10


namespace gpcc_tests {
namespace osal {
namespace internal {


using gpcc::osal::Mutex;
using gpcc::osal::MutexLocker;
using gpcc::osal::Thread;
using gpcc::osal::internal::TFCCore;
using gpcc::osal::internal::ThreadBlocker;
using gpcc::osal::internal::UnmanagedMutex;
using gpcc::osal::internal::UnmanagedMutexLocker;

using namespace testing;


namespace {

// Helper function:
// 1. Locks a dummy mutex
// 2. Locks the TFC big lock
// 3. Invokes Block(...) on the given ThreadBlocker
void* threadEntry(ThreadBlocker* const pTB)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  // create an TFC-MANAGED (!) mutex and lock it
  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  try
  {
    UnmanagedMutexLocker locker(bigLock);

    try
    {
      pTB->Block(dummyMutex);
    }
    catch (...)
    {
      // Check that the big-lock is really re-acquired by Block().
      // Re-acquisition works in case of an exception and in case of deferred thread cancellation.
      if (bigLock.TryLock())
        gpcc::osal::Panic("gpcc_osal_internal_ThreadBlocker_Tests: Big-Lock was not re-acquired upon exception or thread cancellation");

      throw;
    }
  }
  catch (...)
  {
    // Check that the TFC-MANAGED dummy mutex is really re-acquired by Block().
    // Re-acquisition works in case of an exception and in case of deferred thread cancellation.
    if (dummyMutex.TryLock())
      gpcc::osal::Panic("gpcc_osal_internal_ThreadBlocker_Tests: Mutex was not re-acquired upon exception or thread cancellation");

    throw;
  }

  return nullptr;
}

} // anonymus namespace


TEST(gpcc_osal_internal_ThreadBlocker_Tests, Instantiation)
{
  ThreadBlocker uut;
}

TEST(gpcc_osal_internal_ThreadBlocker_Tests, Signal_NoBlockedThread)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  ThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.Signal();
}

TEST(gpcc_osal_internal_ThreadBlocker_Tests, Signal_Twice)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  ThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.Signal();
  ASSERT_THROW(uut.Signal(), std::exception);
}

TEST(gpcc_osal_internal_ThreadBlocker_Tests, Block_AlreadySignaled)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  ThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    uut.Signal();
    uut.Block(dummyMutex);
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_ThreadBlocker_Tests, Block_AlreadySignaled2)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  ThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    uut.Signal();
    uut.Block(dummyMutex);
    uut.Block(dummyMutex);
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_ThreadBlocker_Tests, Block)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  Thread t("ThreadBlocker_Tests");
  ThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntry, &uut), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  // TFC managed sleep. Does not continue before all other threads have been blocked somewhere.
  Thread::Sleep_ms(SLEEPTIME_MS);

  {
    UnmanagedMutexLocker bigLockLocker(bigLock);
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(CancelThread);
}

TEST(gpcc_osal_internal_ThreadBlocker_Tests, Block_DeferredCancellation)
{
  Thread t("ThreadBlocker_Tests");
  ThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntry, &uut), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  // TFC managed sleep. Does not continue before all other threads have been blocked somewhere.
  Thread::Sleep_ms(SLEEPTIME_MS);
}

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_X64_TFC
