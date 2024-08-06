/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#include <gpcc/osal/Thread.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>


// Sleep time when polling for something in ms
#define POLL_SLEEP_MS 2

// This delay (in ms) shall allow a new created thread to execute some code,
// before the main thread continues.
// However, tests are designed that they will work properly even if this delay
// would be zero.
#define DELAY_FOR_OTHER_THREAD_MS 10


namespace gpcc_tests {
namespace osal {

using namespace testing;
using namespace gpcc;
using namespace gpcc::osal;

class TestException : public std::exception
{
  public:
    const char* what() const throw()
    {
      return "TestException";
    }
};

// Test fixture for gpcc::osal::Thread related tests.
class gpcc_osal_Thread_TestsF: public Test
{
  public:
    gpcc_osal_Thread_TestsF(void);

  protected:
    void SetUp(void) override;
    void TearDown(void) override;

  protected:
    Mutex mutex;
    std::atomic<bool> flag;
    uint32_t otherThreads_PID;

    void* ThreadEntry_DeterminePID(void);
    void* ThreadEntry_IsItMe(Thread* const pThread);
    void* ThreadEntry_LockUnlockMutexAndReturn(void);
    void* ThreadEntry_AttemptToCancelSelf(Thread* const pThread);
    void* ThreadEntry_CheckArgAndReturn(uint32_t const v);
    void* ThreadEntry_RunTillCancel(Thread* const pThread);
    void* ThreadEntry_TerminateNow(Thread* const pThread);
    void* ThreadEntry_Throw(void);
    void* ThreadEntry_AttemptToJoinSelf(Thread* const pThread);
    void* ThreadEntry_JoinOtherThread(Thread* const pThread);
    void* ThreadEntry_SetCancelabilityEnabled(Thread* const pThread);
    void* ThreadEntry_DisableCancelability(Thread* const pThread);
    void* ThreadEntry_DisableAndEnableCancelability(Thread* const pThread);
    void* ThreadEntry_CancelOnTestForCancellation(Thread* const pThread);
    void* ThreadEntry_Demo_DeferredTermination(Thread* const pThread);
    void* ThreadEntry_Demo_TerminateNow(Thread* const pThread);

    void TestUncaughtException(void);
};

gpcc_osal_Thread_TestsF::gpcc_osal_Thread_TestsF()
: Test()
, mutex()
, flag(false)
, otherThreads_PID(0)
{
}

void gpcc_osal_Thread_TestsF::SetUp(void)
{
}
void gpcc_osal_Thread_TestsF::TearDown(void)
{
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_DeterminePID(void)
{
  otherThreads_PID = Thread::GetPID();
  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_IsItMe(Thread* const pThread)
{
  bool* const pRetVal = new bool;
  *pRetVal = pThread->IsItMe();

  while (!flag)
    Thread::Sleep_ms(POLL_SLEEP_MS);

  return pRetVal;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_LockUnlockMutexAndReturn(void)
{
  mutex.Lock();
  mutex.Unlock();
  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_AttemptToCancelSelf(Thread* const pThread)
{
  std::unique_ptr<bool> spRetVal(new bool);

  *spRetVal = false;

  try
  {
    pThread->Cancel();
  }
  catch (std::logic_error const & e)
  {
    *spRetVal = true;
  }

  return spRetVal.release();
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_CheckArgAndReturn(uint32_t const v)
{
  if (v != 0xDEADBEEFU)
    return nullptr;

  uint32_t* const pRetVal = new uint32_t;
  *pRetVal = 0x12345678U;
  return pRetVal;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_RunTillCancel(Thread* const pThread)
{
  while (true)
  {
    Thread::Sleep_ms(POLL_SLEEP_MS);
    pThread->TestForCancellation();
  }

  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_TerminateNow(Thread* const pThread)
{
  std::unique_ptr<uint32_t> spRetVal(new uint32_t);
  *spRetVal = 0xDEADBEEF;

  pThread->TerminateNow(spRetVal.release());

  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_Throw(void)
{
  throw TestException();

  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_AttemptToJoinSelf(Thread* const pThread)
{
  std::unique_ptr<bool> spRetVal(new bool);

  *spRetVal = false;

  try
  {
    pThread->Join(nullptr);
  }
  catch (std::logic_error const & e)
  {
    *spRetVal = true;
  }

  flag = true;

  return spRetVal.release();
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_JoinOtherThread(Thread* const pThread)
{
  pThread->Join();
  flag = true;
  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_SetCancelabilityEnabled(Thread* const pThread)
{
  std::unique_ptr<bool> spRetVal(new bool);

  *spRetVal = true;

  // must be initially true and set must be ignored
  if (!pThread->SetCancelabilityEnabled(true))
    *spRetVal = false;

  // set to false
  if (!pThread->SetCancelabilityEnabled(false))
    *spRetVal = false;

  // 2nd set to false must be ignored
  if (pThread->SetCancelabilityEnabled(false))
    *spRetVal = false;

  // set back to true
  if (pThread->SetCancelabilityEnabled(true))
    *spRetVal = false;

  // 2nd set to true must be ignored
  if (!pThread->SetCancelabilityEnabled(true))
    *spRetVal = false;

  return spRetVal.release();
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_DisableCancelability(Thread* const pThread)
{
  (void)pThread->SetCancelabilityEnabled(false);

  do
  {
    Thread::Sleep_ms(POLL_SLEEP_MS);
  }
  while (!pThread->IsCancellationPending());

  // this must have no effect
  pThread->TestForCancellation();

  // return something that is not nullptr
  return this;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_DisableAndEnableCancelability(Thread* const pThread)
{
  (void)pThread->SetCancelabilityEnabled(false);

  do
  {
    Thread::Sleep_ms(POLL_SLEEP_MS);
  }
  while (!pThread->IsCancellationPending());

  // this must have no effect
  pThread->TestForCancellation();

  // this must have no effect
  (void)pThread->SetCancelabilityEnabled(true);

  // return something that is not nullptr
  return this;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_CancelOnTestForCancellation(Thread* const pThread)
{
  (void)pThread->SetCancelabilityEnabled(false);

  do
  {
    Thread::Sleep_ms(POLL_SLEEP_MS);
  }
  while (!pThread->IsCancellationPending());

  // this must have no effect
  pThread->TestForCancellation();

  (void)pThread->SetCancelabilityEnabled(true);

  flag = true;

  pThread->TestForCancellation();

  // return something that is not nullptr
  return this;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_Demo_DeferredTermination(Thread* const pThread)
{
  // You may place some breakpoints here and in TEST_F(gpcc_osal_Thread_TestsF, Demo_DeferredTermination)
  // to check out that deferred cancellation works and destructors of objects instantiated on the stack
  // are invoked.

  ON_SCOPE_EXIT()
  {
    // PLACE BREAKPOINT HERE
    // (this should be invoked after the "throw" in the "catch (...)" clause has been executed)
    volatile int i = 0;
    (void)i;
  };

  MutexLocker mutexLocker(mutex);

  try
  {
    volatile int i = 0;
    while (true)
    {
      Thread::Sleep_ms(POLL_SLEEP_MS);
      pThread->TestForCancellation();
      i = (i + 1) % 100;
    }
  }
  catch (...)
  {
    // PLACE BREAKPOINT HERE
    // (This should be invoked after the thread has been cancelled. Remember: Thread cancellation
    // is implemented based on a special exception)
    throw;
  }

  // PLACE BREAKPOINT HERE
  // (this should never be reached)
  assert(false);
  return nullptr;
}

void* gpcc_osal_Thread_TestsF::ThreadEntry_Demo_TerminateNow(Thread* const pThread)
{
  // You may place some breakpoints here and in TEST_F(gpcc_osal_Thread_TestsF, Demo_TerminateNow)
  // to check out that thread termination works and destructors of objects instantiated on the stack
  // are invoked.

  ON_SCOPE_EXIT()
  {
    // PLACE BREAKPOINT HERE
    // (this should be invoked after the "throw" in the "catch (...)" clause has been executed)
    volatile int i = 0;
    (void)i;
  };

  MutexLocker mutexLocker(mutex);

  try
  {
    pThread->TerminateNow(nullptr);
  }
  catch (...)
  {
    // PLACE BREAKPOINT HERE
    // (This should be invoked after TerminateNow(). Remember: Thread cancellation
    // is implemented based on a special exception)
    throw;
  }

  // PLACE BREAKPOINT HERE
  // (this should never be reached)
  assert(false);
  return nullptr;
}

void gpcc_osal_Thread_TestsF::TestUncaughtException(void)
{
  Thread uut("Test");

  uut.Start(std::bind(&gpcc_osal_Thread_TestsF::ThreadEntry_Throw, this),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  uut.Join();
}

typedef gpcc_osal_Thread_TestsF gpcc_osal_Thread_DeathTestsF;

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_osal_Thread_TestsF, Instantiation)
{
  Thread uut("Test");
}

TEST_F(gpcc_osal_Thread_TestsF, GetThreadRegistry)
{
  IThreadRegistry& tr = Thread::GetThreadRegistry();

  auto locker = tr.Lock();
}

TEST_F(gpcc_osal_Thread_TestsF, GetPID)
{
  uint32_t const local_PID = Thread::GetPID();

  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, GetPID)::ThreadEntry_DeterminePID, this),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  uut.Join();

  ASSERT_EQ(local_PID, otherThreads_PID);
}

TEST_F(gpcc_osal_Thread_TestsF, Sleep_ms)
{
  using namespace gpcc::time;

  // test small timespan
  auto tp1 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  Thread::Sleep_ms(25);
  auto tp2 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  TimeSpan duration = tp2 - tp1;
  ASSERT_TRUE(duration.ms() >= 25);

  // test large timespan
  tp1 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  Thread::Sleep_ms(2508);
  tp2 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  duration = tp2 - tp1;
  ASSERT_TRUE(duration.ms() >= 2508);
}

TEST_F(gpcc_osal_Thread_TestsF, Sleep_ns)
{
  using namespace gpcc::time;

  // test small timespan
  auto tp1 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  Thread::Sleep_ns(25000000UL);
  auto tp2 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  TimeSpan duration = tp2 - tp1;
  ASSERT_TRUE(duration.ns() >= 25000000L);

  // test large timespan
  tp1 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  Thread::Sleep_ns(2508000000UL);
  tp2 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  duration = tp2 - tp1;
  ASSERT_TRUE(duration.ns() >= 2508000000L);
}

TEST_F(gpcc_osal_Thread_TestsF, GetName)
{
  Thread uut("TestABCD");

  std::string const name = uut.GetName();

  ASSERT_TRUE(name == "TestABCD");
}

TEST_F(gpcc_osal_Thread_TestsF, GetInfo)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, GetInfo)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(join) { uut.Join(); };
  ON_SCOPE_EXIT(cancel) { uut.Cancel(); };

  std::string info;

  info = uut.GetInfo(12);
  #if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
  std::cout << "Name         State DS  Scope Policy   prio   Guard   Stack  StackU" << std::endl;
  #endif
  std::cout << info << std::endl;

  ASSERT_TRUE(gpcc::string::StartsWith(info, "Test "));

  ON_SCOPE_EXIT_DISMISS(cancel);
  uut.Cancel();
  ON_SCOPE_EXIT_DISMISS(join);
  uut.Join();

  info = uut.GetInfo(12);
  #if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
  std::cout << "Name         State DS  Scope Policy   prio   Guard   Stack  StackU" << std::endl;
  #endif
  std::cout << info << std::endl;
}

TEST_F(gpcc_osal_Thread_TestsF, GetInfo_CutLongName)
{
  Thread uut("VeryLongThreadName"); // 18 chars

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, GetInfo_CutLongName)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(join) { uut.Join(); };
  ON_SCOPE_EXIT(cancel) { uut.Cancel(); };

  std::string info;

  info = uut.GetInfo(12);
  #if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
  std::cout << "Name         State DS  Scope Policy   prio   Guard   Stack  StackU" << std::endl;
  #endif
  std::cout << info << std::endl;

  ASSERT_TRUE(gpcc::string::StartsWith(info, "VeryLongT... "));

  ON_SCOPE_EXIT_DISMISS(cancel);
  uut.Cancel();
  ON_SCOPE_EXIT_DISMISS(join);
  uut.Join();

  info = uut.GetInfo(12);
  #if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
  std::cout << "Name         State DS  Scope Policy   prio   Guard   Stack  StackU" << std::endl;
  #endif
  std::cout << info << std::endl;
}

TEST_F(gpcc_osal_Thread_TestsF, GetInfo_MinNameFieldWidth)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, GetInfo_MinNameFieldWidth)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(join) { uut.Join(); };
  ON_SCOPE_EXIT(cancel) { uut.Cancel(); };

  std::string info;

  EXPECT_THROW(info = uut.GetInfo(3), std::invalid_argument);
  ASSERT_NO_THROW(info = uut.GetInfo(4));
  #if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
  std::cout << "Name State DS  Scope Policy   prio   Guard   Stack  StackU" << std::endl;
  #endif
  std::cout << info << std::endl;

  ASSERT_TRUE(gpcc::string::StartsWith(info, "Test "));

  ON_SCOPE_EXIT_DISMISS(cancel);
  uut.Cancel();
  ON_SCOPE_EXIT_DISMISS(join);
  uut.Join();

  EXPECT_THROW(info = uut.GetInfo(3), std::invalid_argument);
  ASSERT_NO_THROW(info = uut.GetInfo(4));

  #if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
  std::cout << "Name State DS  Scope Policy   prio   Guard   Stack  StackU" << std::endl;
  #endif
  std::cout << info << std::endl;
}

TEST_F(gpcc_osal_Thread_TestsF, IsItMe)
{
  Thread uut("Test");

  ASSERT_FALSE(uut.IsItMe());

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, IsItMe)::ThreadEntry_IsItMe, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT()
  {
    flag = true;
    uut.Join();
  };

  ASSERT_FALSE(uut.IsItMe());

  ON_SCOPE_EXIT_DISMISS();

  flag = true;
  std::unique_ptr<bool> spRetVal(static_cast<bool*>(uut.Join()));
  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_FALSE(uut.IsItMe());
  ASSERT_TRUE(*spRetVal);
}

TEST_F(gpcc_osal_Thread_TestsF, Start_BadParams)
{
  Thread uut("Test");

  ON_SCOPE_EXIT(cancelAndJoinUnwantedThread)
  {
    uut.Cancel();
    uut.Join();
  };

  // no thread entry function referenced
  ASSERT_THROW(uut.Start(Thread::tEntryFunction(), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize()), std::invalid_argument);

  // invalid priority level
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Inherit, Thread::maxPriority + 1, Thread::GetDefaultStackSize()), std::invalid_argument);
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, Thread::maxPriority + 1, Thread::GetDefaultStackSize()), std::invalid_argument);
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Idle, Thread::maxPriority + 1, Thread::GetDefaultStackSize()), std::invalid_argument);
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Batch, Thread::maxPriority + 1, Thread::GetDefaultStackSize()), std::invalid_argument);
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Fifo, Thread::maxPriority + 1, Thread::GetDefaultStackSize()), std::invalid_argument);
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::RR, Thread::maxPriority + 1, Thread::GetDefaultStackSize()), std::invalid_argument);

  // priority level not zero for scheduling policy "Inherit", "Other", "Idle", and "Batch"
  for (Thread::priority_t prio = Thread::minPriority+1; prio <= Thread::maxPriority; prio++)
  {
    if (prio != 0)
    {
      ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Inherit, prio, Thread::GetDefaultStackSize()), std::invalid_argument);
      ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, prio, Thread::GetDefaultStackSize()), std::invalid_argument);
      ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Idle, prio, Thread::GetDefaultStackSize()), std::invalid_argument);
      ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Batch, prio, Thread::GetDefaultStackSize()), std::invalid_argument);
    }
  }

  // invalid stack alignment
  for (size_t s = 1; s < Thread::GetStackAlign(); s++)
  {
    ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, 0, Thread::GetMinStackSize() + s), std::invalid_argument);
  }

  // stack too small
  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, 0, Thread::GetMinStackSize() - 1U), std::invalid_argument);

  ON_SCOPE_EXIT_DISMISS(cancelAndJoinUnwantedThread);

  // minimum stack size
  ASSERT_NO_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_BadParams)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, 0, Thread::GetMinStackSize()));

  uut.Cancel();
  uut.Join();
}

TEST_F(gpcc_osal_Thread_TestsF, Start_ButThreadAlreadyRunning)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_ButThreadAlreadyRunning)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, 0, Thread::GetMinStackSize());

  ON_SCOPE_EXIT()
  {
    uut.Cancel();
    uut.Join();
  };

  ASSERT_THROW(uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_ButThreadAlreadyRunning)::ThreadEntry_RunTillCancel, this, &uut), Thread::SchedPolicy::Other, 0, Thread::GetMinStackSize()), std::logic_error);
}

TEST_F(gpcc_osal_Thread_TestsF, Start_Policy_SP_Inherit)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_Policy_SP_Inherit)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Inherit, 0, Thread::GetDefaultStackSize());

  std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join()));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);
}

TEST_F(gpcc_osal_Thread_TestsF, Start_Policy_SP_Other)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_Policy_SP_Other)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join()));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);
}

#ifndef SKIP_SPECIAL_RIGHTS_BASED_TESTS
TEST_F(gpcc_osal_Thread_TestsF, Start_Policy_SP_Idle)
{
  // Note: This test requires special rights assigned to the user running the test on some systems

  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_Policy_SP_Idle)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Idle, 0, Thread::GetDefaultStackSize());

  std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join()));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);
}
#endif

#ifndef SKIP_SPECIAL_RIGHTS_BASED_TESTS
TEST_F(gpcc_osal_Thread_TestsF, Start_Policy_SP_Batch)
{
  // Note: This test requires special rights assigned to the user running the test on some systems

  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_Policy_SP_Batch)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Batch, 0, Thread::GetDefaultStackSize());

  std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join()));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);
}
#endif

#ifndef SKIP_SPECIAL_RIGHTS_BASED_TESTS
TEST_F(gpcc_osal_Thread_TestsF, Start_Policy_SP_Fifo)
{
  // Note: This test requires special rights assigned to the user running the test on some systems

  Thread uut("Test");

  for (Thread::priority_t p = Thread::minPriority; p <= Thread::maxPriority; p++)
  {
    uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_Policy_SP_Fifo)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
              Thread::SchedPolicy::Fifo, p, Thread::GetDefaultStackSize());

    std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join()));

    ASSERT_TRUE(spRetVal != nullptr);
    ASSERT_EQ(0x12345678U, *spRetVal);
  }
}
#endif

#ifndef SKIP_SPECIAL_RIGHTS_BASED_TESTS
TEST_F(gpcc_osal_Thread_TestsF, Start_Policy_SP_RR)
{
  // Note: This test requires special rights assigned to the user running the test on some systems

  Thread uut("Test");

  for (Thread::priority_t p = Thread::minPriority; p <= Thread::maxPriority; p++)
  {
    uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Start_Policy_SP_RR)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
              Thread::SchedPolicy::RR, p, Thread::GetDefaultStackSize());

    std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join()));

    ASSERT_TRUE(spRetVal != nullptr);
    ASSERT_EQ(0x12345678U, *spRetVal);
  }
}
#endif

TEST_F(gpcc_osal_Thread_TestsF, StartAfterJoin)
{
  Thread uut("Test");

  std::unique_ptr<uint32_t> spRetVal;

  // 1st start
  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, StartAfterJoin)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  spRetVal.reset(static_cast<uint32_t*>(uut.Join()));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);

  // 2nd start
  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, StartAfterJoin)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  spRetVal.reset(static_cast<uint32_t*>(uut.Join()));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);
}

TEST_F(gpcc_osal_Thread_TestsF, Cancel)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Cancel)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT()
  {
    uut.Cancel();
    uut.Join();
  };

  Thread::Sleep_ms(DELAY_FOR_OTHER_THREAD_MS);
}

TEST_F(gpcc_osal_Thread_TestsF, CancelButNoThreadRunning)
{
  Thread uut("Test");
  ASSERT_THROW(uut.Cancel(), std::logic_error);
}

TEST_F(gpcc_osal_Thread_TestsF, CancelDouble)
{
  Thread uut("Test");

  mutex.Lock();
  ON_SCOPE_EXIT(unlockMutex1) { mutex.Unlock(); };

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, CancelDouble)::ThreadEntry_LockUnlockMutexAndReturn, this),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT() { uut.Join(); };

  ON_SCOPE_EXIT_DISMISS(unlockMutex1);
  ON_SCOPE_EXIT(unlockMutex2) { mutex.Unlock(); };

  uut.Cancel();

  ASSERT_THROW(uut.Cancel(), std::logic_error);
}

TEST_F(gpcc_osal_Thread_TestsF, CancelSelf)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, CancelSelf)::ThreadEntry_AttemptToCancelSelf, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  std::unique_ptr<bool> spRetVal(static_cast<bool*>(uut.Join()));
  ASSERT_TRUE(*spRetVal);
}

TEST_F(gpcc_osal_Thread_TestsF, JoinCancelled)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinCancelled)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  uut.Cancel();

  bool cancelled;
  auto retVal = uut.Join(&cancelled);

  ASSERT_EQ(nullptr, retVal);
  ASSERT_TRUE(cancelled);
}

TEST_F(gpcc_osal_Thread_TestsF, JoinTerminatedByReturn)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinTerminatedByReturn)::ThreadEntry_CheckArgAndReturn, this, 0xDEADBEEFU),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  bool cancelled;
  std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join(&cancelled)));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0x12345678U, *spRetVal);

  ASSERT_FALSE(cancelled);
}

TEST_F(gpcc_osal_Thread_TestsF, JoinTerminatedByTerminateNow)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinTerminatedByTerminateNow)::ThreadEntry_TerminateNow, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  bool cancelled;
  std::unique_ptr<uint32_t> spRetVal(static_cast<uint32_t*>(uut.Join(&cancelled)));

  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_EQ(0xDEADBEEFU, *spRetVal);

  ASSERT_FALSE(cancelled);
}

TEST_F(gpcc_osal_Thread_TestsF, JoinSelf)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinSelf)::ThreadEntry_AttemptToJoinSelf, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  while (!flag)
    Thread::Sleep_ms(POLL_SLEEP_MS);

  std::unique_ptr<bool> spRetVal(static_cast<bool*>(uut.Join()));
  ASSERT_TRUE(spRetVal != nullptr);
  ASSERT_TRUE(*spRetVal);
}

TEST_F(gpcc_osal_Thread_TestsF, JoinChained)
{
  Thread uut("Test");
  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinChained)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinUUT) { uut.Join(); };
  ON_SCOPE_EXIT(cancelUUT) { uut.Cancel(); };

  Thread::Sleep_ms(DELAY_FOR_OTHER_THREAD_MS);

  Thread joiningThread("Test2");
  joiningThread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinChained)::ThreadEntry_JoinOtherThread, this, &uut),
                      Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinJoiningThread) { joiningThread.Join(); };
  ON_SCOPE_EXIT(cancelJoiningThread) { joiningThread.Cancel(); };

  Thread::Sleep_ms(DELAY_FOR_OTHER_THREAD_MS);

  ON_SCOPE_EXIT_DISMISS(cancelUUT);
  uut.Cancel();

  ON_SCOPE_EXIT_DISMISS(cancelJoiningThread);
  ON_SCOPE_EXIT_DISMISS(joinJoiningThread);
  ON_SCOPE_EXIT_DISMISS(joinUUT);
  joiningThread.Join();

  ASSERT_TRUE(flag);
}

TEST_F(gpcc_osal_Thread_TestsF, JoinDeferredCancellation)
{
  // this test checks proper behavior if the thread blocked in Thread::Join() is cancelled

  Thread uut("Test");
  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinDeferredCancellation)::ThreadEntry_RunTillCancel, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(cleanupUUT)
  {
    uut.Cancel();
    uut.Join();
  };

  Thread::Sleep_ms(DELAY_FOR_OTHER_THREAD_MS);

  Thread joiningThread("Test2");
  joiningThread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, JoinDeferredCancellation)::ThreadEntry_JoinOtherThread, this, &uut),
                      Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinJoiningThread) { joiningThread.Join(); };
  ON_SCOPE_EXIT(cancelJoiningThread) { joiningThread.Cancel(); };

  Thread::Sleep_ms(DELAY_FOR_OTHER_THREAD_MS);

  ON_SCOPE_EXIT_DISMISS(cancelJoiningThread);
  joiningThread.Cancel();
  ON_SCOPE_EXIT_DISMISS(joinJoiningThread);
  joiningThread.Join();

  ASSERT_FALSE(flag);
}

TEST_F(gpcc_osal_Thread_DeathTestsF, UncaughtException)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(TestUncaughtException(), ".*Caught exception: TestException.*");
}

TEST_F(gpcc_osal_Thread_TestsF, SetCancelabilityEnabled_WrongThread)
{
  Thread uut("Test");
  ASSERT_THROW((void)uut.SetCancelabilityEnabled(true), std::logic_error);
}

TEST_F(gpcc_osal_Thread_TestsF, SetCancelabilityEnabled)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, SetCancelabilityEnabled)::ThreadEntry_SetCancelabilityEnabled, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  std::unique_ptr<bool> spRetVal(static_cast<bool*>(uut.Join()));

  ASSERT_TRUE(*spRetVal);
}

TEST_F(gpcc_osal_Thread_TestsF, NoCancellationWhenCancellationDisabled)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, NoCancellationWhenCancellationDisabled)::ThreadEntry_DisableCancelability, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  uut.Cancel();

  bool cancelled;
  void* pRetVal = uut.Join(&cancelled);

  ASSERT_FALSE(cancelled);
  ASSERT_TRUE(pRetVal != nullptr);
}

TEST_F(gpcc_osal_Thread_TestsF, NoCancellationWhenCancellationDisabledAndEnabled)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, NoCancellationWhenCancellationDisabledAndEnabled)::ThreadEntry_DisableAndEnableCancelability, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  uut.Cancel();

  bool cancelled;
  void* pRetVal = uut.Join(&cancelled);

  ASSERT_FALSE(cancelled);
  ASSERT_TRUE(pRetVal != nullptr);
}

TEST_F(gpcc_osal_Thread_TestsF, TestForCancellation_WrongThread)
{
  Thread uut("Test");
  ASSERT_THROW(uut.TestForCancellation(), std::logic_error);
}

TEST_F(gpcc_osal_Thread_TestsF, TestForCancellation)
{
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, TestForCancellation)::ThreadEntry_CancelOnTestForCancellation, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  uut.Cancel();

  bool cancelled;
  void* pRetVal = uut.Join(&cancelled);

  ASSERT_TRUE(flag);
  ASSERT_TRUE(cancelled);
  ASSERT_TRUE(pRetVal == nullptr);
}

TEST_F(gpcc_osal_Thread_TestsF, TerminateNow_WrongThread)
{
  Thread uut("Test");
  ASSERT_THROW(uut.TerminateNow(nullptr), std::logic_error);
}

TEST_F(gpcc_osal_Thread_TestsF, Demo_DeferredTermination)
{
  // You may place some breakpoints here and in ThreadEntry_Demo_DeferredTermination()
  // to check out that deferred cancellation works and destructors of destroyed objects
  // are invoked.
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Demo_DeferredTermination)::ThreadEntry_Demo_DeferredTermination, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  // this should allow the loop in ThreadEntry_Demo_DeferredTermination() to take some turns...
  Thread::Sleep_ms(10 * POLL_SLEEP_MS);

  uut.Cancel();
  uut.Join();

  // mutex must be unlocked, because a MutexLocker was used in ThreadEntry_Demo_DeferredTermination().
  if (!mutex.TryLock())
    abort();
  mutex.Unlock();
}

TEST_F(gpcc_osal_Thread_TestsF, Demo_TerminateNow)
{
  // You may place some breakpoints here and in ThreadEntry_Demo_TerminateNow()
  // to check out that deferred cancellation works and destructors of destroyed objects
  // are invoked.
  Thread uut("Test");

  uut.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Thread_TestsF, Demo_TerminateNow)::ThreadEntry_Demo_TerminateNow, this, &uut),
            Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  uut.Cancel();
  uut.Join();

  if (!mutex.TryLock())
    abort();
  mutex.Unlock();
}

} // namespace osal
} // namespace gpcc_tests
