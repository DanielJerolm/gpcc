/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
