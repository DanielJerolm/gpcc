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

#include "gpcc/src/osal/os/linux_arm_tfc/internal/UnmanagedMutex.hpp"
#include "gpcc/src/osal/os/linux_arm_tfc/internal/UnmanagedMutexLocker.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"

#include <unistd.h>

// Some of the tests here use two threads. The following sleeptime in ms shall allow
// one of the two threads to sleep until the other of the two threads has ran into a lock
// or something.
#define SLEEPTIME_MS          10

namespace gpcc_tests {
namespace osal {
namespace internal {

using gpcc::osal::internal::UnmanagedMutex;
using gpcc::osal::internal::UnmanagedMutexLocker;
using gpcc::osal::Thread;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;
using gpcc::time::Clocks;

using namespace testing;

namespace {

// Point in time when the second thread acquired the lock.
TimePoint otherThreadLocked;

#ifndef SKIP_LOAD_DEPENDENT_TESTS
// Locks the given mutex, latches the system time into "otherThreadLocked" and terminates.
void* threadEntryA(UnmanagedMutex* pUUT)
{
  UnmanagedMutexLocker locker(pUUT);
  otherThreadLocked.LatchSystemClock(Clocks::monotonic);
  return nullptr;
}

// Polls for a lock of the given mutex via TryLock(), latches the system time into "otherThreadLocked" and terminates.
void* threadEntryB(UnmanagedMutex* pUUT)
{
  while (!pUUT->TryLock())
    Thread::Sleep_ms(SLEEPTIME_MS);
  ON_SCOPE_EXIT(Unlock) { pUUT->Unlock(); };

  otherThreadLocked.LatchSystemClock(Clocks::monotonic);
  return nullptr;
}
#endif

} // anonymus namespace

TEST(gpcc_osal_internal_UnmanagedMutex_Tests, LockUnlock)
{
  UnmanagedMutex uut;

  uut.Lock();
  uut.Unlock();
}

TEST(gpcc_osal_internal_UnmanagedMutex_Tests, TryLock)
{
  UnmanagedMutex uut;

  uut.Lock();
  ASSERT_FALSE(uut.TryLock());
  uut.Unlock();

  ASSERT_TRUE(uut.TryLock());
  ASSERT_FALSE(uut.TryLock());
  uut.Unlock();
}

#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST(gpcc_osal_internal_UnmanagedMutex_Tests, BlockOtherThread_ViaLock)
{
  Thread t("UnmanagedMutex_Tests");
  UnmanagedMutex uut;

  uut.Lock();
  ON_SCOPE_EXIT(unlockUUT1) { uut.Unlock(); };

  // start thread
  t.Start(std::bind(&threadEntryA, &uut), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };

  // replace unlockUUT1 (intention: unlock uut before joining thread "t")
  ON_SCOPE_EXIT_DISMISS(unlockUUT1);
  ON_SCOPE_EXIT(unlockUUT2) { uut.Unlock(); };

  // allow the newly created thread to run into uut.Lock()
  usleep(SLEEPTIME_MS * 1000UL);

  // measure start time and unlock uut
  TimePoint const mainThreadUnlocks = TimePoint::FromSystemClock(Clocks::monotonic);
  ON_SCOPE_EXIT_DISMISS(unlockUUT2);
  uut.Unlock();

  // join with thread "t"
  ON_SCOPE_EXIT_DISMISS(JoinThread);
  t.Join();

  // examine result
  TimeSpan const duration = otherThreadLocked - mainThreadUnlocks;

  ASSERT_TRUE(duration.ns() == 0);
}
#endif

#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST(gpcc_osal_internal_UnmanagedMutex_Tests, BlockOtherThread_ViaTryLock)
{
  Thread t("Mutex_Tests");
  UnmanagedMutex uut;

  uut.Lock();
  ON_SCOPE_EXIT(unlockUUT) { uut.Unlock(); };

  // start thread
  t.Start(std::bind(&threadEntryB, &uut), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(2 * SLEEPTIME_MS);

  // measure start time and unlock uut
  TimePoint const mainThreadUnlocks = TimePoint::FromSystemClock(Clocks::monotonic);
  ON_SCOPE_EXIT_DISMISS(unlockUUT);
  uut.Unlock();

  // join with thread "t"
  ON_SCOPE_EXIT_DISMISS(CancelThread);
  ON_SCOPE_EXIT_DISMISS(JoinThread);
  t.Join();

  // examine result
  TimeSpan const duration = otherThreadLocked - mainThreadUnlocks;

  ASSERT_TRUE(duration.ns() >= 0);
  ASSERT_TRUE(duration.ms() <= SLEEPTIME_MS);
}
#endif

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_ARM_TFC
