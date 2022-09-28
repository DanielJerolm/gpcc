/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/time/clock.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "gtest/gtest.h"
#include <iostream>
#include <cstdint>
#include <ctime>

namespace gpcc_tests {
namespace time {

using gpcc::time::Clocks;
using gpcc::time::GetPrecision_ns;
using gpcc::time::GetTime;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;

using namespace testing;

TEST(gpcc_time_clock_Tests, GetPrecision_ns)
{
  uint32_t p;

  p = GetPrecision_ns(Clocks::realtime);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p, 1U);
#else
  EXPECT_GT(p, 0U);
  EXPECT_LT(p, 1000000000UL);
#endif
  std::cout << "Precision Clocks::realtime (ns): " << p << std::endl;

  p = GetPrecision_ns(Clocks::realtimePrecise);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p, 1U);
#else
  EXPECT_GT(p, 0U);
  EXPECT_LT(p, 1000000000UL);
#endif
  std::cout << "Precision Clocks::realtimePrecise (ns): " << p << std::endl;

  p = GetPrecision_ns(Clocks::monotonic);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p, 1U);
#else
  EXPECT_GT(p, 0U);
  EXPECT_LT(p, 1000000000UL);
#endif
  std::cout << "Precision Clocks::monotonic (ns): " << p << std::endl;

  p = GetPrecision_ns(Clocks::monotonicPrecise);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p, 1U);
#else
  EXPECT_GT(p, 0U);
  EXPECT_LT(p, 1000000000UL);
#endif
  std::cout << "Precision Clocks::monotonicPrecise (ns): " << p << std::endl;
}

TEST(gpcc_time_clock_Tests, GetTime)
{
  struct timespec ts;
  GetTime(Clocks::realtime, ts);
  std::cout << "Clock (Clocks::realtime): " << TimePoint(ts).ToString() << std::endl;
  GetTime(Clocks::realtimePrecise, ts);
  std::cout << "Clock (Clocks::realtimePrecise): " << TimePoint(ts).ToString() << std::endl;

  GetTime(Clocks::monotonic, ts);
  std::cout << "Clock (Clocks::monotonic): " << TimePoint(ts).ToString() << std::endl;
  GetTime(Clocks::monotonicPrecise, ts);
  std::cout << "Clock (Clocks::monotonicPrecise): " << TimePoint(ts).ToString() << std::endl;
}

#ifndef SKIP_LOAD_DEPENDENT_TESTS
#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_clock_Tests, GetTime_Realtime)
{
  // Test-case is skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::realtime, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME_COARSE, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  ASSERT_TRUE(difference_ns >= 0);
  ASSERT_TRUE(difference_ns < 1000000000L);
}
TEST(gpcc_time_clock_Tests, GetTime_RealtimePrecise)
{
  // Test-case is skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::realtimePrecise, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  ASSERT_TRUE(difference_ns >= 0);
  ASSERT_TRUE(difference_ns < 1000000000L);
}

TEST(gpcc_time_clock_Tests, GetTime_Monotonic)
{
  // Test-case is skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::monotonic, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_MONOTONIC_COARSE, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  ASSERT_TRUE(difference_ns >= 0);
  ASSERT_TRUE(difference_ns < 1000000000L);
}
TEST(gpcc_time_clock_Tests, GetTime_MonotonicPrecise)
{
  // Test-case is skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::monotonicPrecise, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_MONOTONIC, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  ASSERT_TRUE(difference_ns >= 0);
  ASSERT_TRUE(difference_ns < 1000000000L);
}
#endif
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST(gpcc_time_clock_Tests, GetTime_DifferenceRealtimeClocks)
{
  struct timespec ts_realtimePrecise;
  struct timespec ts_realtime;

  GetTime(Clocks::realtimePrecise, ts_realtimePrecise);
  GetTime(Clocks::realtime, ts_realtime);

  TimePoint const TP_realtimePrecise(ts_realtimePrecise);
  TimePoint const TP_realtime(ts_realtime);
  TimeSpan const difference = TP_realtimePrecise - TP_realtime;

  int64_t const difference_ns = difference.ns();

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  ASSERT_TRUE(difference_ns == 0);
#else
  ASSERT_TRUE(difference_ns >= -10000000L);
  ASSERT_TRUE(difference_ns < 10000000L);
#endif
}

TEST(gpcc_time_clock_Tests, GetTime_DifferenceMonotonicClocks)
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
  ASSERT_TRUE(difference_ns >= -10000000L);
  ASSERT_TRUE(difference_ns < 10000000L);
#endif
}
#endif

} // namespace time
} // namespace gpcc_tests
