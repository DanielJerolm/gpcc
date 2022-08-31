/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"

// Some of the tests here use two threads. The following sleeptime in ms shall allow
// one of the two threads to sleep until the other of the two threads has ran into a lock
// or something.
#define SLEEPTIME_MS          10


namespace gpcc_tests {
namespace osal {

using gpcc::osal::Mutex;
using gpcc::osal::MutexLocker;
using gpcc::osal::Thread;
using namespace gpcc::time;

using namespace testing;

namespace {

// Point in time when the second thread acquired the lock.
TimePoint otherThreadLocked;

// Locks the given mutex and waits for a thread cancellation request.
void* threadEntryA(Mutex* pUUT, Thread* pThread)
{
  MutexLocker locker(pUUT);

  while (!pThread->IsCancellationPending())
    Thread::Sleep_ms(SLEEPTIME_MS);

  return nullptr;
}

// Locks the given mutex, latches the system time into "otherThreadLocked" and terminates.
void* threadEntryB(Mutex* pUUT)
{
  MutexLocker locker(pUUT);
  otherThreadLocked.LatchSystemClock(Clocks::monotonic);
  return nullptr;
}

// Polls for a lock of the given mutex via TryLock(), latches the system time into "otherThreadLocked" and terminates.
void* threadEntryC(Mutex* pUUT)
{
  while (!pUUT->TryLock())
    Thread::Sleep_ms(SLEEPTIME_MS);
  ON_SCOPE_EXIT(Unlock) { pUUT->Unlock(); };

  otherThreadLocked.LatchSystemClock(Clocks::monotonic);
  return nullptr;
}

} // anonymus namespace

TEST(gpcc_osal_Mutex_Tests, Instantiation)
{
  Mutex uut;
}

TEST(gpcc_osal_Mutex_Tests, LockUnlock)
{
  Mutex uut;

  uut.Lock();
  uut.Unlock();
}

TEST(gpcc_osal_Mutex_Tests, TryLock)
{
  Mutex uut;

  uut.Lock();
  ASSERT_FALSE(uut.TryLock());
  uut.Unlock();

  ASSERT_TRUE(uut.TryLock());
  ASSERT_FALSE(uut.TryLock());
  uut.Unlock();
}

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
TEST(gpcc_osal_Mutex_DeathTests, TFC_RecursiveLockErrorDetection)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  Mutex uut;
  uut.Lock();
  EXPECT_DEATH(uut.Lock(), ".*The calling thread has the mutex already locked.*");
  uut.Unlock();
}
#endif

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
TEST(gpcc_osal_Mutex_DeathTests, TFC_UnlockButNotLockedErrorDetection)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  Mutex uut;
  EXPECT_DEATH(uut.Unlock(), ".*Not locked.*");
}
#endif

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
TEST(gpcc_osal_Mutex_DeathTests, TFC_UnlockButLockedBySomeoneElseErrorDetection)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  Thread t("Mutex_Tests");
  Mutex uut;

  // start thread
  t.Start(std::bind(&threadEntryA, &uut, &t), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  // wait until the second thread has locked the mutex
  while (uut.TryLock())
  {
    uut.Unlock();

    Thread::Sleep_ms(SLEEPTIME_MS);
  }

  EXPECT_DEATH(uut.Unlock(), ".*The calling thread is not the one which has locked the mutex.*");
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST(gpcc_osal_Mutex_Tests, BlockOtherThread_ViaLock)
{
  Thread t("Mutex_Tests");
  Mutex uut;

  uut.Lock();
  ON_SCOPE_EXIT(unlockUUT1) { uut.Unlock(); };

  // start thread
  t.Start(std::bind(&threadEntryB, &uut), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };

  // replace unlockUUT1 wit unlockUUT2 (intention: unlock uut before joining thread "t")
  ON_SCOPE_EXIT_DISMISS(unlockUUT1);
  ON_SCOPE_EXIT(unlockUUT2) { uut.Unlock(); };

  // allow the newly created thread to run into uut.Lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  // measure start time and unlock uut
  TimePoint const mainThreadUnlocks = TimePoint::FromSystemClock(Clocks::monotonic);
  ON_SCOPE_EXIT_DISMISS(unlockUUT2);
  uut.Unlock();

  // join with thread "t"
  ON_SCOPE_EXIT_DISMISS(JoinThread);
  t.Join();

  // examine result
  TimeSpan const duration = otherThreadLocked - mainThreadUnlocks;

  ASSERT_TRUE(duration.ns() >= 0);
  ASSERT_TRUE(duration.ms() < SLEEPTIME_MS);
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST(gpcc_osal_Mutex_Tests, BlockOtherThread_ViaTryLock)
{
  Thread t("Mutex_Tests");
  Mutex uut;

  uut.Lock();
  ON_SCOPE_EXIT(unlockUUT) { uut.Unlock(); };

  // start thread
  t.Start(std::bind(&threadEntryC, &uut), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  // allow the newly created thread to run into uut.TryLock
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
  ASSERT_TRUE(duration.ms() < 2 * SLEEPTIME_MS);
}
#endif

} // namespace osal
} // namespace gpcc_tests
