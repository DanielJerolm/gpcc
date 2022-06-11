/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "DWQwithThread.hpp"
#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <memory>
#include <stdexcept>
#include <cstdint>

namespace gpcc_tests {
namespace execution  {
namespace async      {

TEST(gpcc_tests_execution_async_DWQwithThread_Tests, CreateAndDestroy)
{
  std::unique_ptr<DWQwithThread> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<DWQwithThread>("UUT"));
  EXPECT_NO_THROW(spUUT.reset());
}

#if !defined(SKIP_TFC_BASED_TESTS)
TEST(gpcc_tests_execution_async_DWQwithThread_Tests, ExecuteWP)
{
  std::atomic<bool> called(false);
  auto func = [&]() { called = true; };

  auto spUUT = std::make_unique<DWQwithThread>("UUT");

  spUUT->GetDWQ().Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0U, func));
  gpcc::osal::Thread::Sleep_ms(10U);

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
    gpcc::osal::Thread::Sleep_ms(10U);
  };

  gpcc::execution::async::WorkPackage staticWP(this, 0U, func);

  auto spUUT = std::make_unique<DWQwithThread>("UUT");

  // Add first work package. Execution will take 10ms.
  spUUT->GetDWQ().Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0U, func));

  // Add two more work packages. They are not intended to be executed because the UUT is destroyed before
  // they execute.
  spUUT->GetDWQ().Add(staticWP);
  spUUT->GetDWQ().Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0U, func));

  // wait until the first dynamic work package is executing...
  gpcc::osal::Thread::Sleep_ms(5U);
  EXPECT_TRUE(nbOfCalls == 1U);

  // ...and then destroy the UUT
  ASSERT_NO_THROW(spUUT.reset());

  // wait until all work packages have executed if they had not been removed from the queue
  gpcc::osal::Thread::Sleep_ms(30U);
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
    spUUT->GetDWQ().Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0U, func));
    gpcc::osal::Thread::Sleep_ms(10U);
  };

  EXPECT_DEATH(lethalCode(), ".*DWQwithThread::ThreadEntry: A work package threw.*");

  spUUT->GetDWQ().FlushNonDeferredWorkPackages();
}
#endif

} // namespace execution
} // namespace async
} // namespace gpcc_tests
