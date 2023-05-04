/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "DWQwithThread.hpp"
#include <gpcc/execution/async/WorkPackage.hpp>
#include <gpcc/osal/Thread.hpp>
#include "gtest/gtest.h"
#include <atomic>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace gpcc_tests {
namespace execution  {
namespace async      {

using gpcc::execution::async::WorkPackage;
using gpcc::osal::Thread;

TEST(gpcc_tests_execution_async_DWQwithThread_Tests, CreateAndDestroy)
{
  std::unique_ptr<DWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<DWQwithThread>("UUT"));
  spUUT.reset();
}

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_tests_execution_async_DWQwithThread_Tests, ExecuteWP)
{
  std::atomic<bool> called(false);
  auto func = [&]() { called = true; };

  auto spUUT = std::make_unique<DWQwithThread>("UUT");

  spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));
  Thread::Sleep_ms(10U);

  EXPECT_TRUE(called);

  spUUT->GetDWQ().FlushNonDeferredWorkPackages();
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_tests_execution_async_DWQwithThread_Tests, WorkPackagesLeftUponDestruction)
{
  std::atomic<uint8_t> nbOfCalls(0U);
  auto func = [&]()
  {
    ++nbOfCalls;
    Thread::Sleep_ms(10U);
  };

  WorkPackage staticWP(this, 0U, func);

  auto spUUT = std::make_unique<DWQwithThread>("UUT");

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
  spUUT.reset();
  EXPECT_TRUE(nbOfCalls == 1U);
}
#endif

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_tests_execution_async_DWQwithThread_DeathTests, WorkpackageThrows)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto func = [&]() { throw std::runtime_error("Intentionally thrown exception."); };

  auto spUUT = std::make_unique<DWQwithThread>("UUT");

  auto lethalCode = [&]()
  {
    spUUT->GetDWQ().Add(WorkPackage::CreateDynamic(this, 0U, func));
    Thread::Sleep_ms(10U);
  };

  EXPECT_DEATH(lethalCode(), ".*DWQwithThread::ThreadEntry: A work package threw.*");

  spUUT->GetDWQ().FlushNonDeferredWorkPackages();
}
#endif

} // namespace execution
} // namespace async
} // namespace gpcc_tests
