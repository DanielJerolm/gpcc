/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/osal/Semaphore.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <functional>
#include <memory>
#include <cstdint>

// Sleeptime in ms for the main thread to allow the helper thread to run into the semaphore's
// wait()-method or to leave the wait()-method and terminate.
#define SLEEPTIME_MS 10

namespace gpcc_tests {
namespace osal {

using namespace gpcc::osal;

using namespace testing;

/// Test fixture for gpcc::osal::Semaphore related tests.
class gpcc_osal_Semaphore_TestsF: public Test
{
  protected:
    gpcc_osal_Semaphore_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

  protected:
    std::unique_ptr<Semaphore> spUUT;
    std::atomic<bool> done;

    void* ThreadEntry(void);
};

gpcc_osal_Semaphore_TestsF::gpcc_osal_Semaphore_TestsF(void)
: Test()
, spUUT()
, done(false)
{
}

void gpcc_osal_Semaphore_TestsF::SetUp(void)
{
}
void gpcc_osal_Semaphore_TestsF::TearDown(void)
{
  spUUT.reset();
}

void* gpcc_osal_Semaphore_TestsF::ThreadEntry(void)
{
  spUUT->Wait();
  done = true;
  return nullptr;
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_Semaphore_TestsF, InstantiationZero)
{
  spUUT.reset(new Semaphore(0));

  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, InstantiationZero)::ThreadEntry, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(join) { thread.Join(); };
  ON_SCOPE_EXIT(cancel) { thread.Cancel(); };

  // allow thread to run into wait
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_FALSE(done);

  spUUT->Post();

  // allow thread to wake up
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(done);
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_Semaphore_TestsF, InstantiationFive)
{
  spUUT.reset(new Semaphore(5));

  Thread thread("GPCC unit test helper thread");

  for (uint_fast8_t i = 0U; i < 6U; i++)
  {
    thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, InstantiationFive)::ThreadEntry, this),
                 Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
    ON_SCOPE_EXIT(join) { thread.Join(); };
    ON_SCOPE_EXIT(cancel) { thread.Cancel(); };

    Thread::Sleep_ms(SLEEPTIME_MS);

    if (i == 5U)
    {
      ON_SCOPE_EXIT_DISMISS(cancel);
      ON_SCOPE_EXIT_DISMISS(join);
      break;
    }

    ASSERT_TRUE(done);
    ON_SCOPE_EXIT_DISMISS(cancel);
    ON_SCOPE_EXIT_DISMISS(join);
    thread.Join();
    done = false;
  }
  ON_SCOPE_EXIT(join2) { thread.Join(); };
  ON_SCOPE_EXIT(cancel2) { thread.Cancel(); };

  ASSERT_FALSE(done);

  spUUT->Post();

  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(done);
}
#endif

TEST_F(gpcc_osal_Semaphore_TestsF, InstantiationZero_NotConsumed)
{
  spUUT.reset(new Semaphore(0));
  spUUT->Post();
  spUUT.reset();
}

TEST_F(gpcc_osal_Semaphore_TestsF, InstantiationFive_NotConsumed)
{
  spUUT.reset(new Semaphore(5));
  spUUT.reset();
}

TEST_F(gpcc_osal_Semaphore_TestsF, InstantiationMAX_NotConsumed)
{
  spUUT.reset(new Semaphore(Semaphore::MAX));
  spUUT.reset();
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_Semaphore_TestsF, PostIncrements)
{
  spUUT.reset(new Semaphore(0));

  Thread thread("GPCC unit test helper thread");

  spUUT->Post();
  spUUT->Post();

  for (uint_fast8_t i = 0; i < 3; i++)
  {
    thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, PostIncrements)::ThreadEntry, this),
                 Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
    ON_SCOPE_EXIT(join) { thread.Join(); };
    ON_SCOPE_EXIT(cancel) { thread.Cancel(); };

    Thread::Sleep_ms(SLEEPTIME_MS);

    if (i == 2)
    {
      ON_SCOPE_EXIT_DISMISS(cancel);
      ON_SCOPE_EXIT_DISMISS(join);
      break;
    }

    ASSERT_TRUE(done);
    ON_SCOPE_EXIT_DISMISS(cancel);
    ON_SCOPE_EXIT_DISMISS(join);
    thread.Join();
    done = false;
  }
  ON_SCOPE_EXIT(join2) { thread.Join(); };
  ON_SCOPE_EXIT(cancel2) { thread.Cancel(); };

  ASSERT_FALSE(done);

  spUUT->Post();

  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(done);
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_Semaphore_TestsF, DeferredCancellation_OneThread)
{
  spUUT.reset(new Semaphore(0));

  Thread thread("GPCC unit test helper thread");

  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, DeferredCancellation_OneThread)::ThreadEntry, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(join1) { thread.Join(); };
  ON_SCOPE_EXIT(cancel1) { thread.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);

  ON_SCOPE_EXIT_DISMISS(cancel1);
  thread.Cancel();
  ON_SCOPE_EXIT_DISMISS(join1);
  thread.Join();

  // ensure that semaphore's count is still zero
  thread.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, DeferredCancellation_OneThread)::ThreadEntry, this),
               Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(join2) { thread.Join(); };
  ON_SCOPE_EXIT(cancel2) { thread.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_FALSE(done);

  spUUT->Post();

  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(done);
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_Semaphore_TestsF, DeferredCancellation_TwoThread)
{
  spUUT.reset(new Semaphore(0));

  Thread thread1("GPCC unit test helper thread 1");
  Thread thread2("GPCC unit test helper thread 2");

  thread1.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, DeferredCancellation_TwoThread)::ThreadEntry, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(join1) { thread1.Join(); };
  ON_SCOPE_EXIT(cancel1) { thread1.Cancel(); };

  thread2.Start(std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_osal_Semaphore_TestsF, DeferredCancellation_TwoThread)::ThreadEntry, this),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(join2) { thread2.Join(); };
  ON_SCOPE_EXIT(cancel2) { thread2.Cancel(); };

  Thread::Sleep_ms(SLEEPTIME_MS);

  ON_SCOPE_EXIT_DISMISS(cancel1);
  thread1.Cancel();
  ON_SCOPE_EXIT_DISMISS(join1);
  thread1.Join();

  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_FALSE(done);

  spUUT->Post();

  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(done);
}
#endif

} // namespace osal
} // namespace gpcc_tests
