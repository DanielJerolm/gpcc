/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include "src/osal/os/linux_arm_tfc/internal/UnmanagedConditionVariable.hpp"
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "src/osal/os/linux_arm_tfc/internal/AdvancedUnmanagedMutexLocker.hpp"
#include "src/osal/os/linux_arm_tfc/internal/UnmanagedMutexLocker.hpp"
#include "gtest/gtest.h"
#include <unistd.h>

// Sleeptime in ms for the main thread to allow the helper thread to run into the condition variable's
// Wait()- or TimeLimitedWait()-method or to leave the Wait()- or TimeLimitedWait()-method and terminate.
#define SLEEPTIME_MS          10

namespace gpcc_tests {
namespace osal {
namespace internal {

using namespace gpcc::osal::internal;

using namespace testing;

/// Test fixture for gpcc::osal::internal::UnmanagedConditionVariable related tests.
class gpcc_osal_internal_UnmanagedConditionVariable_TestsF: public Test
{
  protected:
    gpcc_osal_internal_UnmanagedConditionVariable_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

  protected:
    UnmanagedMutex m;
    uint_fast8_t blockedThreads; // Mutex "m" required
    bool predicate;              // Mutex "m" required
    UnmanagedConditionVariable uut;

    void* ThreadEntry_ReturnWhenPredicateTrue(void);
};

gpcc_osal_internal_UnmanagedConditionVariable_TestsF::gpcc_osal_internal_UnmanagedConditionVariable_TestsF(void)
: Test()
, m()
, blockedThreads(0U)
, predicate(false)
, uut()
{
}

void gpcc_osal_internal_UnmanagedConditionVariable_TestsF::SetUp(void)
{
}
void gpcc_osal_internal_UnmanagedConditionVariable_TestsF::TearDown(void)
{
}

void* gpcc_osal_internal_UnmanagedConditionVariable_TestsF::ThreadEntry_ReturnWhenPredicateTrue(void)
{
  UnmanagedMutexLocker m_locker(m);

  ++blockedThreads;
  ON_SCOPE_EXIT(decBlockedThreads) { --blockedThreads; };

  while (!predicate)
    uut.Wait(m);

  // check that the mutex is locked
  if (m.TryLock())
    throw std::runtime_error("Mutex was not locked upon return from Wait()");

  return nullptr;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST(gpcc_osal_internal_UnmanagedConditionVariable_Tests, Instantiation)
{
  UnmanagedConditionVariable uut;
}

TEST(gpcc_osal_internal_UnmanagedConditionVariable_Tests, SignalNoWaiter)
{
  UnmanagedConditionVariable uut;
  uut.Signal();
}

TEST(gpcc_osal_internal_UnmanagedConditionVariable_Tests, BroadcastNoWaiter)
{
  UnmanagedConditionVariable uut;
  uut.Broadcast();
}

TEST_F(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, WaitAndSignal)
{
  using gpcc::osal::Thread;

  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, WaitAndSignal)::ThreadEntry_ReturnWhenPredicateTrue, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // wait until the newly created thread has run into uut.Wait()
  while (true)
  {
    usleep(SLEEPTIME_MS * 1000UL);

    UnmanagedMutexLocker m_locker(m);
    if (blockedThreads == 1U)
      break;
  }

  {
    UnmanagedMutexLocker m_locker(m);
    predicate = true;
    uut.Signal();
  }

  ON_SCOPE_EXIT_DISMISS(thread_cancel);
}

TEST_F(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, WaitAndBroadcast)
{
  using gpcc::osal::Thread;

  Thread thread1("GPCC unit test helper thread");
  Thread thread2("GPCC unit test helper thread");

  thread1.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, WaitAndBroadcast)::ThreadEntry_ReturnWhenPredicateTrue, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread1_join) { thread1.Join(); };
  ON_SCOPE_EXIT(thread1_cancel) { thread1.Cancel(); };

  thread2.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, WaitAndBroadcast)::ThreadEntry_ReturnWhenPredicateTrue, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread2_join) { thread2.Join(); };
  ON_SCOPE_EXIT(thread2_cancel) { thread2.Cancel(); };

  // wait until the newly created threads have run into uut.Wait()
  while (true)
  {
    usleep(SLEEPTIME_MS * 1000UL);

    UnmanagedMutexLocker m_locker(m);
    if (blockedThreads == 2U)
      break;
  }

  {
    UnmanagedMutexLocker m_locker(m);
    predicate = true;
    uut.Broadcast();
  }

  ON_SCOPE_EXIT_DISMISS(thread2_cancel);
  ON_SCOPE_EXIT_DISMISS(thread1_cancel);
}

TEST_F(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, DeferredCancellation_DuringWait)
{
  using gpcc::osal::Thread;

  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, DeferredCancellation_DuringWait)::ThreadEntry_ReturnWhenPredicateTrue, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // wait until the newly created thread has run into uut.Wait()
  while (true)
  {
    usleep(SLEEPTIME_MS * 1000UL);

    UnmanagedMutexLocker m_locker(m);
    if (blockedThreads == 1U)
      break;
  }
}

#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST_F(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, DeferredCancellation_BeforeWait)
{
  using gpcc::osal::Thread;

  Thread thread("GPCC unit test helper thread");

  // lock mutex to prevent new thread to run into uut.Wait()
  AdvancedUnmanagedMutexLocker mutexLocker(m);

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_internal_UnmanagedConditionVariable_TestsF, DeferredCancellation_BeforeWait)::ThreadEntry_ReturnWhenPredicateTrue, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(thread_join) { thread.Join(); };
  ON_SCOPE_EXIT(unlock_mutex) { mutexLocker.Unlock(); };
  ON_SCOPE_EXIT(thread_cancel) { thread.Cancel(); };

  // Allow the newly created thread to run into m.Lock().
  // Running into m.Lock() is not required for passing the test, but we want a well-defined pre-condition, so this
  // test case requires a light-loaded machine.
  usleep(SLEEPTIME_MS * 1000UL);
}
#endif

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_ARM_TFC
