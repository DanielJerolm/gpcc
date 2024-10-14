/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#include <gpcc/execution/async/SuspendableDWQwithThread.hpp>
#include <gpcc/execution/async/WorkPackage.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace gpcc_tests {
namespace execution  {
namespace async      {

using gpcc::execution::async::SuspendableDWQwithThread;
using gpcc::execution::async::WorkPackage;
using gpcc::osal::Thread;

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, CreateAndDestroy)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, CreateStartStopAndDestroy)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));

  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  spUUT->Stop();

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, StartTwice)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));

  EXPECT_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()), std::logic_error);

  spUUT->Stop();
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_DeathTests, StopTwice)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  spUUT->Stop();

  EXPECT_DEATH(spUUT->Stop(), ".*SuspendableDWQwithThread::Stop: Failed.*");

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_DeathTests, StopButNeverStarted)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));

  ASSERT_DEATH(spUUT->Stop(), ".*SuspendableDWQwithThread::Stop: Failed.*");

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_DeathTests, DestroyButSuspended)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));

  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));

  EXPECT_DEATH(spUUT.reset(), ".*Not stopped.*");

  if (spUUT)
    spUUT->Stop();

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_DeathTests, DestroyButRunning)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));

  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  EXPECT_NO_THROW(spUUT->Resume());

  EXPECT_DEATH(spUUT.reset(), ".*Not stopped.*");

  if (spUUT)
    spUUT->Stop();

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, CreateStartResumeStopAndDestroy)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  EXPECT_NO_THROW(spUUT->Resume());
  spUUT->Stop();
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, CreateStartResumeSuspendStopAndDestroy)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  EXPECT_NO_THROW(spUUT->Resume());
  EXPECT_NO_THROW(spUUT->Suspend());
  spUUT->Stop();
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, ResumeTwice)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  EXPECT_NO_THROW(spUUT->Resume());

  EXPECT_THROW(spUUT->Resume(), std::logic_error);

  spUUT->Stop();
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, ResumeButNotRunning)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));

  EXPECT_THROW(spUUT->Resume(), std::logic_error);

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, SuspendButNotRunning)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));

  EXPECT_THROW(spUUT->Suspend(), std::logic_error);

  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, SuspendTwice)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
  EXPECT_NO_THROW(spUUT->Resume());
  EXPECT_NO_THROW(spUUT->Suspend());

  EXPECT_THROW(spUUT->Suspend(), std::logic_error);

  spUUT->Stop();
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, SuspendButNeverResumed)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));

  EXPECT_THROW(spUUT->Suspend(), std::logic_error);

  spUUT->Stop();
  spUUT.reset();
}

TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, MultipleResumeSuspendCycles)
{
  std::unique_ptr<SuspendableDWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<SuspendableDWQwithThread>("UUT"));
  ASSERT_NO_THROW(spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));

  for (uint_fast8_t i = 0; i < 8U; ++i)
  {
    ASSERT_NO_THROW(spUUT->Resume());
    ASSERT_NO_THROW(spUUT->Suspend());
  }

  spUUT->Stop();
  spUUT.reset();
}

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, ExecuteWP)
{
  std::atomic<bool> called(false);
  auto func = [&]() { called = true; };

  auto spUUT = std::make_unique<SuspendableDWQwithThread>("UUT");
  spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize());
  spUUT->Resume();

  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));
  Thread::Sleep_ms(10U);

  EXPECT_TRUE(called);

  spUUT->GetDWQ().FlushNonDeferredWorkPackages();
  spUUT->Stop();
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, NoWPExecutionBeforeFirstResume)
{
  std::atomic<bool> called(false);
  auto func = [&]() { called = true; };

  auto spUUT = std::make_unique<SuspendableDWQwithThread>("UUT");
  spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize());

  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));
  Thread::Sleep_ms(10U);
  EXPECT_FALSE(called);

  spUUT->Resume();
  Thread::Sleep_ms(10U);
  EXPECT_TRUE(called);

  spUUT->GetDWQ().FlushNonDeferredWorkPackages();
  spUUT->Stop();
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, WorkPackagesLeftUponSuspend)
{
  std::atomic<uint8_t> nbOfCalls(0U);
  auto func = [&]()
  {
    ++nbOfCalls;
    Thread::Sleep_ms(10U);
  };

  WorkPackage staticWP(this, 0U, func);

  auto spUUT = std::make_unique<SuspendableDWQwithThread>("UUT");
  spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize());
  spUUT->Resume();

  // Add first work package. Execution will take 10ms.
  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));

  // Add two more work packages. They are not intended to be executed because the UUT is destroyed before
  // they execute.
  spUUT->GetDWQ().Add(staticWP);
  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));

  // wait until the first dynamic work package is executing...
  Thread::Sleep_ms(5U);
  EXPECT_TRUE(nbOfCalls == 1U);

  // ...and then suspend work package execution
  spUUT->Suspend();
  EXPECT_TRUE(nbOfCalls == 1U);

  // wait some time and check that no WP is executed
  Thread::Sleep_ms(20U);
  EXPECT_TRUE(nbOfCalls == 1U);

  // ...and then destroy the UUT
  spUUT->Stop();
  spUUT.reset();
  EXPECT_TRUE(nbOfCalls == 1U);
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, WorkPackagesLeftUponDestruction)
{
  std::atomic<uint8_t> nbOfCalls(0U);
  auto func = [&]()
  {
    ++nbOfCalls;
    Thread::Sleep_ms(10U);
  };

  WorkPackage staticWP(this, 0U, func);

  auto spUUT = std::make_unique<SuspendableDWQwithThread>("UUT");
  spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize());
  spUUT->Resume();

  // Add first work package. Execution will take 10ms.
  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));

  // Add two more work packages. They are not intended to be executed because the UUT is destroyed before
  // they execute.
  spUUT->GetDWQ().Add(staticWP);
  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));

  // wait until the first dynamic work package is executing...
  Thread::Sleep_ms(5U);
  EXPECT_TRUE(nbOfCalls == 1U);

  // ...and then destroy the UUT
  spUUT->Stop();
  spUUT.reset();
  EXPECT_TRUE(nbOfCalls == 1U);
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_execution_async_SuspendableDWQwithThread_Tests, SuspendAndResumeWorkpackageExecution)
{
  std::atomic<uint8_t> nbOfCalls(0U);
  auto func = [&]()
  {
    ++nbOfCalls;
    Thread::Sleep_ms(10U);
  };

  WorkPackage staticWP(this, 0U, func);

  auto spUUT = std::make_unique<SuspendableDWQwithThread>("UUT");
  spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize());
  spUUT->Resume();

  // Add first work package. Execution will take 10ms.
  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));

  // Add two more work packages. Execution will take 10ms each.
  spUUT->GetDWQ().Add(staticWP);
  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));

  // wait until the first dynamic work package is executing...
  Thread::Sleep_ms(5U);
  EXPECT_TRUE(nbOfCalls == 1U);

  // ...and then suspend work package execution
  spUUT->Suspend();
  EXPECT_TRUE(nbOfCalls == 1U);

  // wait some time and check that no WP is executed
  Thread::Sleep_ms(20U);
  EXPECT_TRUE(nbOfCalls == 1U);

  spUUT->Resume();

  // wait until the 2nd work package is executing...
  Thread::Sleep_ms(5U);
  EXPECT_TRUE(nbOfCalls == 2U);

  // wait until the 3nd work package is executing...
  Thread::Sleep_ms(10U);
  EXPECT_TRUE(nbOfCalls == 3U);

  // ...and then destroy the UUT
  spUUT->Stop();
  spUUT.reset();
  EXPECT_TRUE(nbOfCalls == 3U);
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_execution_async_SuspendableDWQwithThread_DeathTests, WorkpackageThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto func = [&]() { throw std::runtime_error("Intentionally thrown exception."); };

  auto spUUT = std::make_unique<SuspendableDWQwithThread>("UUT");
  spUUT->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize());
  spUUT->Resume();

  auto lethalCode = [&]()
  {
    spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));
    Thread::Sleep_ms(10U);
  };

  EXPECT_DEATH(lethalCode(), ".*SuspendableDWQwithThread: A work package threw.*");

  spUUT->GetDWQ().FlushNonDeferredWorkPackages();
  spUUT->Stop();
  spUUT.reset();
}
#endif

} // namespace execution
} // namespace async
} // namespace gpcc_tests
