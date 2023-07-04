/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#include "src/osal/os/linux_x64_tfc/internal/TimeLimitedThreadBlocker.hpp"
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include "src/osal/os/linux_x64_tfc/internal/UnmanagedMutex.hpp"
#include "src/osal/os/linux_x64_tfc/internal/UnmanagedMutexLocker.hpp"
#include <gtest/gtest.h>


// Sleeptime in ms for the main thread to allow the helper thread to run into the UUTs blocking method.
#define SLEEPTIME_MS          10

// Timeout value for timeouts that will never be exceeded. Note: TFC makes time precise on any machine.
#define DUMMY_TIMEOUT_MS      10

namespace gpcc_tests {
namespace osal {
namespace internal {


using gpcc::osal::ConditionVariable;
using gpcc::osal::Mutex;
using gpcc::osal::MutexLocker;
using gpcc::osal::Thread;
using gpcc::osal::internal::TFCCore;
using gpcc::osal::internal::TimeLimitedThreadBlocker;
using gpcc::osal::internal::UnmanagedMutex;
using gpcc::osal::internal::UnmanagedMutexLocker;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;
using gpcc::time::Clocks;

using namespace testing;


namespace {

static bool retVal = false;

// Helper function:
// 1. Locks a dummy mutex
// 2. Locks the TFC big lock
// 3. Invokes Block(...) on the given TimeLimitedThreadBlocker
void* threadEntryA(TimeLimitedThreadBlocker* const pTLTB, gpcc::time::TimePoint const absTimeout)
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
      retVal = pTLTB->Block(dummyMutex, absTimeout);
    }
    catch (...)
    {
      // Check that the big-lock is really re-acquired by Block().
      // Re-acquisition works in case of an exception and in case of deferred thread cancellation.
      if (bigLock.TryLock())
        gpcc::osal::Panic("gpcc_osal_internal_TimeLimitedThreadBlocker_Tests: Big-Lock was not re-acquired upon exception or thread cancellation");

      throw;
    }
  }
  catch (...)
  {
    // Check that the TFC-MANAGED dummy mutex is really re-acquired by Block().
    // Re-acquisition works in case of an exception and in case of deferred thread cancellation.
    if (dummyMutex.TryLock())
      gpcc::osal::Panic("gpcc_osal_internal_TimeLimitedThreadBlocker_Tests: Mutex was not re-acquired upon exception or thread cancellation");

    throw;
  }

  return &retVal;
}

// Helper function:
// 1. Locks the TFC big lock
// 2. Invokes Block(...) on the given TimeLimitedThreadBlocker
void* threadEntryB(TimeLimitedThreadBlocker* const pTLTB, gpcc::time::TimePoint const absTimeout)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  UnmanagedMutexLocker locker(bigLock);

  try
  {
    retVal = pTLTB->Block(absTimeout);
  }
  catch (...)
  {
    // Check that the big-lock is really re-acquired by Block().
    // Re-acquisition works in case of an exception and in case of deferred thread cancellation.
    if (bigLock.TryLock())
      gpcc::osal::Panic("gpcc_osal_internal_TimeLimitedThreadBlocker_Tests: Big-Lock was not re-acquired upon exception or thread cancellation");

    throw;
  }

  return &retVal;
}

} // anonymus namespace


TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Instantiation)
{
  TimeLimitedThreadBlocker uut;
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Signal_NoBlockedThread)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.Signal();
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Signal_Twice)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.Signal();
  ASSERT_THROW(uut.Signal(), std::exception);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, SignalTimeout_NoBlockedThread)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.SignalTimeout();
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, SignalTimeout_Twice)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.SignalTimeout();
  ASSERT_THROW(uut.SignalTimeout(), std::exception);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Signal_SignalTimeout)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.Signal();
  uut.SignalTimeout();
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, SignalTimeout_Signal)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  UnmanagedMutexLocker locker(bigLock);
  uut.SignalTimeout();
  uut.Signal();
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_AlreadySignaled)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    uut.Signal();
    ASSERT_FALSE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(DUMMY_TIMEOUT_MS)));
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_AlreadySignaled2)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    uut.Signal();
    ASSERT_FALSE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(DUMMY_TIMEOUT_MS)));
    ASSERT_FALSE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(DUMMY_TIMEOUT_MS)));
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_AlreadySignaledTimeout)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    ASSERT_TRUE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID)));
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_AlreadySignaledTimeout2)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    ASSERT_TRUE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID)));
    ASSERT_TRUE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(DUMMY_TIMEOUT_MS)));
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_SignaledPlusTimeout)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    uut.Signal();
    ASSERT_FALSE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID)));
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_SignaledPlusTimeout2)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  TimeLimitedThreadBlocker uut;

  Mutex dummyMutex;
  MutexLocker dummyMutexLocker(dummyMutex);

  {
    UnmanagedMutexLocker locker(bigLock);
    uut.Signal();
    ASSERT_FALSE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID)));
    ASSERT_FALSE(uut.Block(dummyMutex, TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(DUMMY_TIMEOUT_MS)));
  }

  EXPECT_FALSE(dummyMutex.TryLock());
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_NoTimeout_WithMutexUnlock)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  Thread t("ThreadBlocker_Tests");
  TimeLimitedThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntryA, &uut,
                    TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(SLEEPTIME_MS * 2)),
          Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);

  {
    UnmanagedMutexLocker bigLockLocker(bigLock);
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(CancelThread);
  ON_SCOPE_EXIT_DISMISS(JoinThread);

  ASSERT_TRUE(t.Join() != nullptr);
  ASSERT_FALSE(retVal);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_NoTimeout_WithoutMutexUnlock)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  Thread t("ThreadBlocker_Tests");
  TimeLimitedThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntryB, &uut,
                    TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(SLEEPTIME_MS * 2)),
          Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);

  {
    UnmanagedMutexLocker bigLockLocker(bigLock);
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(CancelThread);
  ON_SCOPE_EXIT_DISMISS(JoinThread);

  ASSERT_TRUE(t.Join() != nullptr);
  ASSERT_FALSE(retVal);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_Timeout_WithMutexUnlock)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  Thread t("ThreadBlocker_Tests");
  TimeLimitedThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntryA, &uut,
                    TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(SLEEPTIME_MS)),
          Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(2 * SLEEPTIME_MS);

  {
    UnmanagedMutexLocker bigLockLocker(bigLock);
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(CancelThread);
  ON_SCOPE_EXIT_DISMISS(JoinThread);

  ASSERT_TRUE(t.Join() != nullptr);
  ASSERT_TRUE(retVal);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_Timeout_WithoutMutexUnlock)
{
  UnmanagedMutex& bigLock = TFCCore::Get()->GetBigLock();

  Thread t("ThreadBlocker_Tests");
  TimeLimitedThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntryB, &uut,
                    TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(SLEEPTIME_MS)),
          Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(2 * SLEEPTIME_MS);

  {
    UnmanagedMutexLocker bigLockLocker(bigLock);
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(CancelThread);
  ON_SCOPE_EXIT_DISMISS(JoinThread);

  ASSERT_TRUE(t.Join() != nullptr);
  ASSERT_TRUE(retVal);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_DeferredCancellation_WithMutexUnlock)
{
  Thread t("ThreadBlocker_Tests");
  TimeLimitedThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntryA, &uut,
                    TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(SLEEPTIME_MS * 2)),
          Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);

  ON_SCOPE_EXIT_DISMISS(CancelThread);
  t.Cancel();

  ON_SCOPE_EXIT_DISMISS(JoinThread);
  ASSERT_TRUE(t.Join() == nullptr);
}

TEST(gpcc_osal_internal_TimeLimitedThreadBlocker_Tests, Block_DeferredCancellation_WithoutMutexUnlock)
{
  Thread t("ThreadBlocker_Tests");
  TimeLimitedThreadBlocker uut;

  // start thread
  t.Start(std::bind(&threadEntryB, &uut,
                    TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(SLEEPTIME_MS * 2)),
          Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(JoinThread) { t.Join(); };
  ON_SCOPE_EXIT(CancelThread) { t.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);

  ON_SCOPE_EXIT_DISMISS(CancelThread);
  t.Cancel();

  ON_SCOPE_EXIT_DISMISS(JoinThread);
  ASSERT_TRUE(t.Join() == nullptr);
}

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_X64_TFC
