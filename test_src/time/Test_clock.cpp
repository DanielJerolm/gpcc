/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "gpcc/src/time/clock.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <cstdint>
#include <ctime>

namespace gpcc_tests {
namespace time {

using gpcc::time::Clocks;
using gpcc::time::GetTime;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;

using namespace testing;

#ifndef SKIP_LOAD_DEPENDENT_TESTS
#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_clock_Tests, GetTime_Realtime)
{
  // Test-case skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::realtime, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  ASSERT_TRUE(difference_ns >= 0);
  ASSERT_TRUE(difference_ns < 1000000000);
}
#endif
#endif

#ifndef SKIP_LOAD_DEPENDENT_TESTS
#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_clock_Tests, GetTime_Monotonic)
{
  // Test-case skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::monotonic, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_MONOTONIC, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  ASSERT_TRUE(difference_ns >= 0);
  ASSERT_TRUE(difference_ns < 1000000000);
}
#endif
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST(gpcc_time_clock_Tests, GetTime_DifferenceClocks)
{
  struct timespec ts_monotonicPrecise;
  struct timespec ts_monotonic;

  GetTime(Clocks::monotonicPrecise, ts_monotonicPrecise);
  GetTime(Clocks::monotonic, ts_monotonic);

  TimePoint const TP_monotonicPrecise(ts_monotonicPrecise);
  TimePoint const TP_monotonic(ts_monotonic);
  TimeSpan const difference = TP_monotonicPrecise - TP_monotonic;

  int64_t const difference_ns = difference.ns();

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  ASSERT_TRUE(difference_ns == 0);
#else
  ASSERT_TRUE(difference_ns >= -10000000);
  ASSERT_TRUE(difference_ns < 10000000);
#endif
}
#endif

} // namespace time
} // namespace gpcc_tests
