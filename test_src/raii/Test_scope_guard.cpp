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

#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <iomanip>

namespace gpcc_tests {
namespace raii {

using namespace testing;

class TestException1 {};
class TestException2 {};

static void FunctionThrowingTestException1(void)
{
  throw TestException1();
}

static void FunctionThrowingAndCatchingTestException1(void)
{
  try
  {
    FunctionThrowingTestException1();
  }
  catch (TestException1 const & e)
  {
    (void)e;
  }
}

TEST(gpcc_raii_scope_guard_Tests, NoDissmiss)
{
  bool cleanupDone = false;

  {
    ON_SCOPE_EXIT() { cleanupDone = true; };
  }

  ASSERT_TRUE(cleanupDone);
}
TEST(gpcc_raii_scope_guard_Tests, Dismiss)
{
  bool cleanupDone = false;

  {
    ON_SCOPE_EXIT() { cleanupDone = true; };

    ON_SCOPE_EXIT_DISMISS();
  }

  ASSERT_FALSE(cleanupDone);
}
TEST(gpcc_raii_scope_guard_Tests, ScopeLeftByBreakBeforeDismiss)
{
  bool cleanupDone = false;

  do
  {
    ON_SCOPE_EXIT() { cleanupDone = true; };

    // leave scope before dismiss
    break;

    ON_SCOPE_EXIT_DISMISS();
  }
  while (false);

  ASSERT_TRUE(cleanupDone);
}
TEST(gpcc_raii_scope_guard_Tests, ScopeLeftByException)
{
  bool cleanupDone = false;
  bool TestException2Caught = false;

  try
  {
    ON_SCOPE_EXIT() { cleanupDone = true; };

    // leave scope by throwing TestException2 before dismiss
    throw TestException2();

    ON_SCOPE_EXIT_DISMISS();
  }
  catch (TestException2 const & e)
  {
    (void)e;
    TestException2Caught = true;
  }

  ASSERT_TRUE(cleanupDone);
  ASSERT_TRUE(TestException2Caught);
}
TEST(gpcc_raii_scope_guard_Tests, ScopeLeftByExceptionWithCaughtExceptionInCleanupHandler)
{
  bool cleanupDone1 = false;
  bool cleanupDone2 = false;
  bool TestException1Caught = false;
  bool TestException2Caught = false;

  try
  {
    ON_SCOPE_EXIT() { cleanupDone1 = true; FunctionThrowingAndCatchingTestException1(); cleanupDone2 = true; };

    // leave scope by throwing TestException2 before dismiss
    throw TestException2();

    ON_SCOPE_EXIT_DISMISS();
  }
  catch (TestException1 const & e)
  {
    (void)e;
    // this should not be reached, checked below
    TestException1Caught = true;
  }
  catch (TestException2 const & e)
  {
    (void)e;
    TestException2Caught = true;
  }

  ASSERT_TRUE(cleanupDone1);
  ASSERT_TRUE(cleanupDone2);
  ASSERT_FALSE(TestException1Caught);
  ASSERT_TRUE(TestException2Caught);
}

static void ScopeLeftByExceptionWith_UNCAUGHT_ExceptionInCleanupHandler(void)
{
  try
  {
    try
    {
      ON_SCOPE_EXIT() { FunctionThrowingTestException1(); };

      // leave scope by throwing TestException2 before dismiss
      throw TestException2();

      ON_SCOPE_EXIT_DISMISS();
    }
    catch (TestException2 const & e)
    {
      (void)e;
      // This is never reached. Otherwise EXPECT_DEATH in calling TEST will fail.
    }
  }
  catch (TestException1 const & e)
  {
    (void)e;
    // This is never reached. Otherwise EXPECT_DEATH in calling TEST will fail.
  }
}

TEST(gpcc_raii_scope_guard_DeathTests, ScopeLeftByExceptionWith_UNCAUGHT_ExceptionInCleanupHandler)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(ScopeLeftByExceptionWith_UNCAUGHT_ExceptionInCleanupHandler(), ".*gpcc::ScopeGuard: Rollback threw.*");
}

static void NoDissmissWith_UNCAUGHT_ExceptionInCleanupHandler(void)
{
  try
  {
    ON_SCOPE_EXIT() { FunctionThrowingTestException1(); };
  }
  catch (TestException1 const & e)
  {
    (void)e;
    // This is never reached. Otherwise EXPECT_DEATH in calling TEST will fail.
  }
}

TEST(gpcc_raii_scope_guard_DeathTests, NoDissmissWith_UNCAUGHT_ExceptionInCleanupHandler)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(NoDissmissWith_UNCAUGHT_ExceptionInCleanupHandler(), ".*gpcc::ScopeGuard: Rollback threw.*");
}
TEST(gpcc_raii_scope_guard_Tests, NestedScopeGuards)
{
  std::vector<int> rollbacks;
  rollbacks.reserve(6);

  {
    ON_SCOPE_EXIT(sg1) { rollbacks.push_back(1); };
    ON_SCOPE_EXIT(sg2) { rollbacks.push_back(2); };
    ON_SCOPE_EXIT(sg3) { rollbacks.push_back(3); };

    {
      ON_SCOPE_EXIT(sg4) { rollbacks.push_back(4); };
      ON_SCOPE_EXIT(sg5) { rollbacks.push_back(5); };

      rollbacks.push_back(0);

      ON_SCOPE_EXIT_DISMISS(sg4);
    }

    ON_SCOPE_EXIT_DISMISS(sg2);
  }

  ASSERT_EQ(static_cast<size_t>(4), rollbacks.size());

  ASSERT_EQ(0, rollbacks[0]);
  ASSERT_EQ(5, rollbacks[1]);
  ASSERT_EQ(3, rollbacks[2]);
  ASSERT_EQ(1, rollbacks[3]);
}
TEST(gpcc_raii_scope_guard_Tests, make_ScopeGuard)
{
  using namespace gpcc::raii;

  bool cleanupDone = false;

  {
    auto guard = make_ScopeGuard([&]() { cleanupDone = true; });
  }

  ASSERT_TRUE(cleanupDone);
}
TEST(gpcc_raii_scope_guard_Tests, Performance_WithoutDismiss)
{
  int const loops = 10000000;

  using namespace gpcc::time;

  int i;
  volatile int a, b;
  volatile bool cancel = false;
  TimePoint start, stop;

  // reference loop without scope guard
  a = 0;
  b = 0;
  start = TimePoint::FromSystemClock(Clocks::monotonic);
  for (i = 0; i < loops; i++)
  {
    a++;
    b++;
  }
  stop = TimePoint::FromSystemClock(Clocks::monotonic);
  TimeSpan const duration_no_scope_gaurd = stop - start;

  ASSERT_EQ(loops, a);
  ASSERT_EQ(loops, b);

  // loop with scope guard
  a = 0;
  b = 0;
  start = TimePoint::FromSystemClock(Clocks::monotonic);
  for (i = 0; i < loops; i++)
  {
    ON_SCOPE_EXIT() { a++; };
    b++;
    if (cancel)
    {
      // never reached, but compiler doesn't know (cancel is volatile)
      throw std::runtime_error("cancel is true");
    }
  }
  stop = TimePoint::FromSystemClock(Clocks::monotonic);
  TimeSpan const duration_with_scope_gaurd = stop - start;

  ASSERT_EQ(loops, a);
  ASSERT_EQ(loops, b);

  // print results
  std::cout << "Number of loops: " << loops << std::endl;
  std::cout << "Loop without scope-guard: " << duration_no_scope_gaurd.us() << "us" << std::endl;
  std::cout << "Loop with scope-guard...: " << duration_with_scope_gaurd.us() << "us" << std::endl;
}

TEST(gpcc_raii_scope_guard_Tests, Performance_WithDismiss)
{
  int const loops = 10000000;

  using namespace gpcc::time;

  int i;
  volatile int a, b;
  volatile bool cancel = false;
  TimePoint start, stop;

  // reference loop without scope guard
  a = 0;
  b = 0;
  start = TimePoint::FromSystemClock(Clocks::monotonic);
  for (i = 0; i < loops; i++)
  {
    if ((i % 8) == 0)
     a++;

    b++;
  }
  stop = TimePoint::FromSystemClock(Clocks::monotonic);
  TimeSpan const duration_no_scope_gaurd = stop - start;

  EXPECT_EQ(loops / 8, a);
  EXPECT_EQ(loops    , b);

  // loop with scope guard
  a = 0;
  b = 0;
  start = TimePoint::FromSystemClock(Clocks::monotonic);
  for (i = 0; i < loops; i++)
  {
    ON_SCOPE_EXIT() { a++; };
    b++;
    if (cancel)
    {
      // never reached, but compiler doesn't know (cancel is volatile)
      throw std::runtime_error("cancel is true");
    }
    if ((i % 8) != 0)
      ON_SCOPE_EXIT_DISMISS();
  }
  stop = TimePoint::FromSystemClock(Clocks::monotonic);
  TimeSpan const duration_with_scope_gaurd = stop - start;

  EXPECT_EQ(loops / 8, a);
  EXPECT_EQ(loops    , b);

  // print results
  std::cout << "Number of loops: " << loops << std::endl;
  std::cout << "Loop without scope-guard: " << duration_no_scope_gaurd.us() << "us" << std::endl;
  std::cout << "Loop with scope-guard...: " << duration_with_scope_gaurd.us() << "us" << std::endl;
}

} // namespace raii
} // namespace gpcc_tests
