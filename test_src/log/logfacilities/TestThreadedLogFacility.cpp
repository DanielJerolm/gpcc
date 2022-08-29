/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TestILogFacility.hpp"
#include "TestILogFacilityCtrl.hpp"
#include <gpcc/log/logfacilities/ThreadedLogFacility.hpp>
#include "gtest/gtest.h"
#include <stdexcept>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_log_ThreadedLogFacility_, ILogFacility_Tests1F, ThreadedLogFacility);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_log_ThreadedLogFacility_, ILogFacility_Tests2F, ThreadedLogFacility);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_log_ThreadedLogFacility_, ILogFacilityCtrl_TestsF, ThreadedLogFacility);

TEST(gpcc_log_ThreadedLogFacility_Tests, Instantiation)
{
  std::unique_ptr<ThreadedLogFacility> spUUT(new ThreadedLogFacility("LFThread", 8));
  spUUT.reset();
}
TEST(gpcc_log_ThreadedLogFacility_Tests, Instantiation_BadCapacity)
{
  std::unique_ptr<ThreadedLogFacility> spUUT;

  ASSERT_THROW(spUUT.reset(new ThreadedLogFacility("LFThread", 7)), std::invalid_argument);
}
TEST(gpcc_log_ThreadedLogFacility_Tests, StartStop)
{
  std::unique_ptr<ThreadedLogFacility> spUUT(new ThreadedLogFacility("LFThread", 8));
  spUUT->Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  spUUT->Stop();
}

TEST(gpcc_log_ThreadedLogFacility_DeathTests, DestroyButLoggerNotUnregistered)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<ThreadedLogFacility> spUUT(new ThreadedLogFacility("LFThread", 8));
  std::unique_ptr<Logger> spLogger(new Logger("TL1"));

  spUUT->Register(*spLogger.get());
  ON_SCOPE_EXIT() { spUUT->Unregister(*spLogger.get()); };

  EXPECT_DEATH(spUUT.reset(), ".*gpcc/src/log/logfacilities/ThreadedLogFacility.cpp.*");
}
TEST(gpcc_log_ThreadedLogFacility_DeathTests, DestroyButBackendNotUnregistered)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<ThreadedLogFacility> spUUT(new ThreadedLogFacility("LFThread", 8));
  std::unique_ptr<FakeBackend> spBE(new FakeBackend());

  spUUT->Register(*spBE.get());
  ON_SCOPE_EXIT() { spUUT->Unregister(*spBE.get()); };

  EXPECT_DEATH(spUUT.reset(), ".*gpcc/src/log/logfacilities/ThreadedLogFacility.cpp.*");
}

} // namespace log
} // namespace gpcc_tests
