/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#include <gpcc_test/execution/UnittestDurationLimiter.hpp>
#include <gtest/gtest.h>
#include <unistd.h>

namespace gpcc_tests {
namespace execution  {

#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST(gpcc_tests_execution_UnittestDurationLimiter_DeathTests, Trigger)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto lethalCode = [&]()
  {
    UnittestDurationLimiter uut(1U);
    sleep(2U);
  };

  EXPECT_DEATH(lethalCode(), ".*Maximum execution time exceeded.*");
}
#endif

#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST(gpcc_tests_execution_UnittestDurationLimiter_Tests, NoTrigger)
{
  UnittestDurationLimiter uut(2U);
  sleep(1U);
}
#endif

} // namespace execution
} // namespace gpcc_tests
