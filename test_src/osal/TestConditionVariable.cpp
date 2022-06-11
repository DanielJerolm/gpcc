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

#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <functional>

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
#include <unistd.h>
#endif

// Sleeptime in ms for the main thread to allow the helper thread to run into the condition variable's
// Wait()- or TimeLimitedWait()-method or to leave the Wait()- or TimeLimitedWait()-method and terminate.
#define SLEEPTIME_MS          10

// Timeout in ms when waiting for predicate with timeout.
#define TIMEOUT_MS            100

// Timeout in ms when waiting for predicate without any signal.
#define NO_SIGNAL_TIMEOUT_MS  10

namespace gpcc_tests {
namespace osal {

using namespace gpcc::time;
using namespace gpcc::osal;

using namespace testing;

/// Test fixture for gpcc::osal::ConditionVariable related tests.
class gpcc_osal_ConditionVariable_TestsF: public Test
{
  protected:
    gpcc_osal_ConditionVariable_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

  protected:
    Mutex m;
    uint_fast8_t blockedThreads; // Mutex "m" required
    bool predicate;              // Mutex "m" required
    ConditionVariable uut;

    void* ThreadEntry_ReturnWhenPredicateTrue(void);
    void* ThreadEntry_ReturnWhenPredicateTrueWithTimeout(void);
};

gpcc_osal_ConditionVariable_TestsF::gpcc_osal_ConditionVariable_TestsF(void)
: Test()
, m()
, blockedThreads(0U)
, predicate(false)
, uut()
{
}

void gpcc_osal_ConditionVariable_TestsF::SetUp(void)
{
}
void gpcc_osal_ConditionVariable_TestsF::TearDown(void)
{
}

void* gpcc_osal_ConditionVariable_TestsF::ThreadEntry_ReturnWhenPredicateTrue(void)
{
  MutexLocker m_locker(m);

  ++blockedThreads;
  ON_SCOPE_EXIT(decBlockedThreads) { --blockedThreads; };

  while (!predicate)
    uut.Wait(m);

  // check that the mutex is locked
  if (m.TryLock())
    throw std::runtime_error("Mutex was not locked upon return from Wait()");

  return nullptr;
}

void* gpcc_osal_ConditionVariable_TestsF::ThreadEntry_ReturnWhenPredicateTrueWithTimeout(void)
{
  TimePoint const timeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(TIMEOUT_MS);

  MutexLocker m_locker(m);

  ++blockedThreads;
  ON_SCOPE_EXIT(decBlockedThreads) { --blockedThreads; };

  while (!predicate)
  {
    if (uut.TimeLimitedWait(m, timeout))
      throw std::runtime_error("Unexpected timeout in unit test");
  }

  // check that the mutex is locked
  if (m.TryLock())
    throw std::runtime_error("Mutex was not locked upon return from Wait()");

  return nullptr;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST(gpcc_osal_ConditionVariable_Tests, Instantiation)
{
  ConditionVariable uut;
}

TEST(gpcc_osal_ConditionVariable_Tests, SignalNoWaiter)
{
  Mutex m;
  MutexLocker m_locker(m);

  ConditionVariable uut;
  uut.Signal();

  // Check that signal is lost. TimeLimitedWait() must return with timeout.
  TimePoint const timeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(NO_SIGNAL_TIMEOUT_MS);
  ASSERT_TRUE(uut.TimeLimitedWait(m, timeout));

  // check that mutex is locked
  ASSERT_FALSE(m.TryLock());
}

TEST(gpcc_osal_ConditionVariable_Tests, BroadcastNoWaiter)
{
  Mutex m;
  MutexLocker m_locker(m);

  ConditionVariable uut;
  uut.Broadcast();

  // Check that broadcast is lost. TimeLimitedWait() must return with timeout.
  TimePoint const timeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(NO_SIGNAL_TIMEOUT_MS);
  ASSERT_TRUE(uut.TimeLimitedWait(m, timeout));

  // check that mutex is locked
  ASSERT_FALSE(m.TryLock());
}

TEST_F(gpcc_osal_ConditionVariable_TestsF, WaitAndSignal)
{
  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, WaitAndSignal)::ThreadEntry_ReturnWhenPredicateTrue, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // wait until the newly created thread has run into uut.Wait()
  while (true)
  {
    Thread::Sleep_ms(SLEEPTIME_MS);

    MutexLocker m_locker(m);
    if (blockedThreads == 1U)
      break;
  }

  {
    MutexLocker m_locker(m);
    predicate = true;
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(thread_cancel);
}

TEST_F(gpcc_osal_ConditionVariable_TestsF, WaitAndBroadcast)
{
  Thread thread1("GPCC unit test helper thread");
  Thread thread2("GPCC unit test helper thread");

  thread1.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, WaitAndBroadcast)::ThreadEntry_ReturnWhenPredicateTrue, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread1_join) { thread1.Join(); };
  ON_SCOPE_EXIT(thread1_cancel) { thread1.Cancel(); };

  thread2.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, WaitAndBroadcast)::ThreadEntry_ReturnWhenPredicateTrue, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread2_join) { thread2.Join(); };
  ON_SCOPE_EXIT(thread2_cancel) { thread2.Cancel(); };

  // wait until the newly created threads have run into uut.Wait()
  while (true)
  {
    Thread::Sleep_ms(SLEEPTIME_MS);

    MutexLocker m_locker(m);
    if (blockedThreads == 2U)
      break;
  }

  {
    MutexLocker m_locker(m);
    predicate = true;
    uut.Broadcast();
  }

  ON_SCOPE_EXIT_DISMISS(thread2_cancel);
  ON_SCOPE_EXIT_DISMISS(thread1_cancel);
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_ConditionVariable_TestsF, WaitWithTimeoutAndSignal)
{
  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, WaitWithTimeoutAndSignal)::ThreadEntry_ReturnWhenPredicateTrueWithTimeout, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // wait until the newly created thread has run into uut.Wait()
  while (true)
  {
    Thread::Sleep_ms(SLEEPTIME_MS);

    MutexLocker m_locker(m);
    if (blockedThreads == 1U)
      break;
  }

  {
    MutexLocker m_locker(m);
    predicate = true;
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(thread_cancel);
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_ConditionVariable_TestsF, WaitWithTimeoutAndBroadcast)
{
  Thread thread1("GPCC unit test helper thread");
  Thread thread2("GPCC unit test helper thread");

  thread1.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, WaitWithTimeoutAndBroadcast)::ThreadEntry_ReturnWhenPredicateTrueWithTimeout, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread1_join) { thread1.Join(); };
  ON_SCOPE_EXIT(thread1_cancel) { thread1.Cancel(); };

  thread2.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, WaitWithTimeoutAndBroadcast)::ThreadEntry_ReturnWhenPredicateTrueWithTimeout, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread2_join) { thread2.Join(); };
  ON_SCOPE_EXIT(thread2_cancel) { thread2.Cancel(); };

  // wait until the newly created threads have run into uut.Wait()
  while (true)
  {
    Thread::Sleep_ms(SLEEPTIME_MS);

    MutexLocker m_locker(m);
    if (blockedThreads == 2U)
      break;
  }

  {
    MutexLocker m_locker(m);
    predicate = true;
    uut.Broadcast();
  }

  ON_SCOPE_EXIT_DISMISS(thread2_cancel);
  ON_SCOPE_EXIT_DISMISS(thread1_cancel);
}
#endif

TEST_F(gpcc_osal_ConditionVariable_TestsF, WaitWithTimeoutNoSignal)
{
  MutexLocker m_locker(m);

  TimePoint const timeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(NO_SIGNAL_TIMEOUT_MS);
  ASSERT_TRUE(uut.TimeLimitedWait(m, timeout));

  // check that mutex is locked
  ASSERT_FALSE(m.TryLock());
}

#if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
TEST_F(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_DuringWait)
{
  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_DuringWait)::ThreadEntry_ReturnWhenPredicateTrue, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // wait until the newly created thread has run into uut.Wait()
  while (true)
  {
    Thread::Sleep_ms(SLEEPTIME_MS);

    MutexLocker m_locker(m);
    if (blockedThreads == 1U)
      break;
  }
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
#if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
TEST_F(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_BeforeWait)
{
  Thread thread("GPCC unit test helper thread");

  // lock mutex to prevent new thread to run into Wait()
  gpcc::osal::AdvancedMutexLocker mutexLocker(m);

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_BeforeWait)::ThreadEntry_ReturnWhenPredicateTrue, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(unlock_mutex) { mutexLocker.Unlock(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // Allow the newly created thread to run into m.Lock().
  // Running into m.Lock() is not required for passing the test, but we want a well-defined pre-condition, so this
  // test case requires TFC or a light-loaded machine.
  Thread::Sleep_ms(SLEEPTIME_MS);
}
#endif
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
#if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
TEST_F(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_DuringTimeLimitedWait)
{
  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_DuringTimeLimitedWait)::ThreadEntry_ReturnWhenPredicateTrueWithTimeout, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // wait until the newly created thread has run into uut.Wait()
  while (true)
  {
    Thread::Sleep_ms(SLEEPTIME_MS);

    MutexLocker m_locker(m);
    if (blockedThreads == 1U)
      break;
  }
}
#endif
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
#if (defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
TEST_F(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_BeforeTimeLimitedWait)
{
  Thread thread("GPCC unit test helper thread");

  // lock mutex to prevent new thread to run into TimeLimitedWait()
  gpcc::osal::AdvancedMutexLocker mutexLocker(m);

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_ConditionVariable_TestsF, DeferredCancellation_BeforeTimeLimitedWait)::ThreadEntry_ReturnWhenPredicateTrueWithTimeout, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join)
  {
    thread.Join();
  };
  ON_SCOPE_EXIT(unlock_mutex)
  {
    mutexLocker.Unlock();

    // In a TFC-environment, deferred cancellation on ConditionVariable::TimeLimitedWait() may increment the emulated
    // system time to the next event. Without the following sleep, the next event would be the timeout condition and
    // TimeLimitedWait() could report a timeout condition.
    Thread::Sleep_ms(SLEEPTIME_MS);
  };
  ON_SCOPE_EXIT(thread_cancel)
  {
    thread.Cancel();
  };

  // Allow the newly created thread to run into m.Lock().
  // Running into m.Lock() is not required for passing the test, but we want a well-defined pre-condition, so this
  // test case requires TFC or a light-loaded machine.
  Thread::Sleep_ms(SLEEPTIME_MS);
}
#endif
#endif

} // namespace osal
} // namespace gpcc_tests
