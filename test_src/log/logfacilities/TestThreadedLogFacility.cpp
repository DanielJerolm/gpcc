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

#include "TestILogFacility.hpp"
#include "TestILogFacilityCtrl.hpp"
#include "gpcc/src/log/logfacilities/ThreadedLogFacility.hpp"
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
