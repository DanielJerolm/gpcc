/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <stdexcept>

namespace gpcc_tests {
namespace osal       {
namespace internal   {

using namespace testing;

class gpcc_osal_internal_TFCCore_TestsF: public Test
{
  protected:
    gpcc_osal_internal_TFCCore_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

  protected:
    gpcc::osal::internal::TFCCore* pUUT;

    void* ThreadEntry_Sleep100ms(void);
};

gpcc_osal_internal_TFCCore_TestsF::gpcc_osal_internal_TFCCore_TestsF(void)
: Test()
{
}

void gpcc_osal_internal_TFCCore_TestsF::SetUp(void)
{
  pUUT = gpcc::osal::internal::TFCCore::Get();
}

void gpcc_osal_internal_TFCCore_TestsF::TearDown(void)
{
}

void* gpcc_osal_internal_TFCCore_TestsF::ThreadEntry_Sleep100ms(void)
{
  gpcc::osal::Thread::Sleep_ms(100);
  return nullptr;
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, PreciseSleep)
{
  using namespace gpcc::time;
  using gpcc::osal::Thread;

  TimePoint tp_start = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(100);
  TimePoint tp_100 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(200);
  TimePoint tp_300 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(500);
  TimePoint tp_800 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(1000);
  TimePoint tp_1800 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  EXPECT_EQ( 100000000LL, ( tp_100 - tp_start).ns());
  EXPECT_EQ( 300000000LL, ( tp_300 - tp_start).ns());
  EXPECT_EQ( 800000000LL, ( tp_800 - tp_start).ns());
  EXPECT_EQ(1800000000LL, (tp_1800 - tp_start).ns());
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, EnableAndDisableWatchForAttemptToBlockWithExpiredTimeout)
{
  ASSERT_NO_THROW(pUUT->EnableWatchForAlreadyExpiredTimeout());
  EXPECT_THROW(pUUT->EnableWatchForAlreadyExpiredTimeout(), std::logic_error);

  bool retVal = false;
  ASSERT_NO_THROW(retVal = pUUT->DisableWatchForAlreadyExpiredTimeout());
  EXPECT_FALSE(retVal);

  EXPECT_THROW((void)pUUT->DisableWatchForAlreadyExpiredTimeout(), std::logic_error);
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, DetectAttemptToBlockWithExpiredTimeout)
{
  using namespace gpcc::time;
  using gpcc::osal::ConditionVariable;

  // Sleep a millisecond to ensure that we can subtract 1ns from the system time below.
  gpcc::osal::Thread::Sleep_ms(1);

  pUUT->EnableWatchForAlreadyExpiredTimeout();

  ConditionVariable cv;
  TimePoint const timeout = TimePoint::FromSystemClock(ConditionVariable::clockID) - TimeSpan::ns(1);

  {
    gpcc::osal::Mutex m;
    gpcc::osal::MutexLocker ml(m);
    ASSERT_TRUE(cv.TimeLimitedWait(m, timeout));
  }

  EXPECT_TRUE(pUUT->DisableWatchForAlreadyExpiredTimeout())
    << "Attempt to block with expired timeout has not been detected!";
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, EnableAndDisableWatchForBlockWithSameTimeout)
{
  ASSERT_NO_THROW(pUUT->EnableWatchForBlockWithSameTimeout());
  EXPECT_THROW(pUUT->EnableWatchForBlockWithSameTimeout(), std::logic_error);

  bool retVal = false;
  ASSERT_NO_THROW(retVal = pUUT->DisableWatchForBlockWithSameTimeout());
  EXPECT_FALSE(retVal);

  EXPECT_THROW((void)pUUT->DisableWatchForBlockWithSameTimeout(), std::logic_error);
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, DetectAttemptToBlockWithSameTimeout)
{
  using gpcc::osal::Thread;
  Thread thread1("Thread1");
  Thread thread2("Thread2");

  pUUT->EnableWatchForBlockWithSameTimeout();

  thread1.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_TFCCore_TestsF, DetectAttemptToBlockWithSameTimeout)::ThreadEntry_Sleep100ms, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinThread1) { (void)thread1.Join(nullptr); };

  thread2.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_TFCCore_TestsF, DetectAttemptToBlockWithSameTimeout)::ThreadEntry_Sleep100ms, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinThread2) { (void)thread2.Join(nullptr); };

  Thread::Sleep_ms(50U);

  EXPECT_TRUE(pUUT->DisableWatchForBlockWithSameTimeout())
    << "Attempt to block with same timeout has not been detected!";
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, EnableAndDisableWatchForSimultaneousResumeOfMultipleThreads)
{
  ASSERT_NO_THROW(pUUT->EnableWatchForSimultaneousResumeOfMultipleThreads());
  EXPECT_THROW(pUUT->EnableWatchForSimultaneousResumeOfMultipleThreads(), std::logic_error);

  bool retVal = false;
  ASSERT_NO_THROW(retVal = pUUT->DisableWatchForSimultaneousResumeOfMultipleThreads());
  EXPECT_FALSE(retVal);

  EXPECT_THROW((void)pUUT->DisableWatchForSimultaneousResumeOfMultipleThreads(), std::logic_error);
}

TEST_F(gpcc_osal_internal_TFCCore_TestsF, DetectSimultaneousResumeOfMultipleThreads)
{
  using gpcc::osal::Thread;
  Thread thread1("Thread1");
  Thread thread2("Thread2");

  thread1.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_TFCCore_TestsF, DetectSimultaneousResumeOfMultipleThreads)::ThreadEntry_Sleep100ms, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinThread1) { (void)thread1.Join(nullptr); };

  thread2.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_TFCCore_TestsF, DetectSimultaneousResumeOfMultipleThreads)::ThreadEntry_Sleep100ms, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinThread2) { (void)thread2.Join(nullptr); };

  Thread::Sleep_ms(50U);

  pUUT->EnableWatchForSimultaneousResumeOfMultipleThreads();

  Thread::Sleep_ms(100U);

  EXPECT_TRUE(pUUT->DisableWatchForSimultaneousResumeOfMultipleThreads())
    << "Unblocking of multiple threads after increment of system time has not been detected!";
}

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_X64_TFC
