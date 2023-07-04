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
#include <gtest/gtest.h>
#include <iostream>
#include <cstdint>
#include <ctime>

namespace gpcc_tests {
namespace time       {

using gpcc::time::Clocks;
using gpcc::time::GetPrecision_ns;
using gpcc::time::GetTime;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;

using namespace testing;

#define MS10_in_NS 10000000L

TEST(gpcc_time_clock_Tests, GetPrecision_ns)
{
  uint32_t p = GetPrecision_ns(Clocks::realtimeCoarse);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p, 1U);
#else
  EXPECT_GT(p, 0U);
  EXPECT_LE(p, MS10_in_NS);
#endif
  std::cout << "Precision Clocks::realtimeCoarse (ns): " << p << std::endl;

  uint32_t p_prec = GetPrecision_ns(Clocks::realtimePrecise);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p_prec, 1U);
#else
  EXPECT_GT(p_prec, 0U);
  EXPECT_LE(p_prec, MS10_in_NS);
#endif
  EXPECT_LE(p_prec, p);
  std::cout << "Precision Clocks::realtimePrecise (ns): " << p_prec << std::endl;

  p = GetPrecision_ns(Clocks::monotonicCoarse);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p, 1U);
#else
  EXPECT_GT(p, 0U);
  EXPECT_LE(p, MS10_in_NS);
#endif
  std::cout << "Precision Clocks::monotonicCoarse (ns): " << p << std::endl;

  p_prec = GetPrecision_ns(Clocks::monotonicPrecise);
#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_EQ(p_prec, 1U);
#else
  EXPECT_GT(p_prec, 0U);
  EXPECT_LE(p_prec, MS10_in_NS);
#endif
  EXPECT_LE(p_prec, p);
  std::cout << "Precision Clocks::monotonicPrecise (ns): " << p_prec << std::endl;
}

TEST(gpcc_time_clock_Tests, GetTime_justCall)
{
  struct timespec ts;
  GetTime(Clocks::realtimeCoarse, ts);
  std::cout << "Clock (Clocks::realtimeCoarse): " << TimePoint(ts).ToString() << std::endl;
  GetTime(Clocks::realtimePrecise, ts);
  std::cout << "Clock (Clocks::realtimePrecise): " << TimePoint(ts).ToString() << std::endl;

  GetTime(Clocks::monotonicCoarse, ts);
  std::cout << "Clock (Clocks::monotonicCoarse): " << TimePoint(ts).ToString() << std::endl;
  GetTime(Clocks::monotonicPrecise, ts);
  std::cout << "Clock (Clocks::monotonicPrecise): " << TimePoint(ts).ToString() << std::endl;
}

#ifndef SKIP_LOAD_DEPENDENT_TESTS
#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_clock_Tests, GetTime_Validate_Realtime)
{
  // Test-case is skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::realtimeCoarse, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_REALTIME_COARSE, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::realtimeCoarse) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
TEST(gpcc_time_clock_Tests, GetTime_Validate_RealtimePrecise)
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
  std::cout << "Delta (Clocks::realtimePrecise) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
TEST(gpcc_time_clock_Tests, GetTime_Validate_Monotonic)
{
  // Test-case is skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  struct timespec ts_fromUUT;
  struct timespec ts_reference;

  ASSERT_NO_THROW(GetTime(Clocks::monotonicCoarse, ts_fromUUT));
  ASSERT_EQ(0, clock_gettime(CLOCK_MONOTONIC_COARSE, &ts_reference));

  TimePoint const TP_from_UUT(ts_fromUUT);
  TimePoint const TP_Reference(ts_reference);
  TimeSpan const difference = TP_Reference - TP_from_UUT;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::monotonicCoarse) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
TEST(gpcc_time_clock_Tests, GetTime_Validate_MonotonicPrecise)
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
  std::cout << "Delta (Clocks::monotonicPrecise) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
#endif
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST(gpcc_time_clock_Tests, GetTime_DifferenceRealtimeClocks)
{
  struct timespec ts_realtime;
  struct timespec ts_realtimePrecise;

  GetTime(Clocks::realtimeCoarse, ts_realtime);
  GetTime(Clocks::realtimePrecise, ts_realtimePrecise);

  TimePoint const TP_realtime(ts_realtime);
  TimePoint const TP_realtimePrecise(ts_realtimePrecise);
  TimeSpan const difference = TP_realtimePrecise - TP_realtime;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::realtimePrecise vs Clocks::realtimeCoarse) (ns): " << difference_ns << std::endl;

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_TRUE(difference_ns == 0);
#else
  EXPECT_GE(difference_ns, 0);
  EXPECT_LE(difference_ns, 2 * MS10_in_NS);
#endif
}

TEST(gpcc_time_clock_Tests, GetTime_DifferenceMonotonicClocks)
{
  struct timespec ts_monotonic;
  struct timespec ts_monotonicPrecise;

  GetTime(Clocks::monotonicCoarse, ts_monotonic);
  GetTime(Clocks::monotonicPrecise, ts_monotonicPrecise);

  TimePoint const TP_monotonic(ts_monotonic);
  TimePoint const TP_monotonicPrecise(ts_monotonicPrecise);
  TimeSpan const difference = TP_monotonicPrecise - TP_monotonic;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::monotonicPrecise vs Clocks::monotonicCoarse) (ns): " << difference_ns << std::endl;

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
  EXPECT_TRUE(difference_ns == 0);
#else
  EXPECT_GE(difference_ns, 0);
  EXPECT_LE(difference_ns, 2 * MS10_in_NS);
#endif
}
#endif

#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_clock_Tests, GetTime_Realtime_PreciseAlwaysLargerThanCoarse)
{
  // Test-case is skipped if TFC is present.
  // Rationale: It is not applicable to emulated clocks that do not increase without a sleep.

  struct timespec ts_realtime;
  struct timespec ts_realtimePrecise;
  TimePoint TP_realtime;
  TimePoint inital;
  TimeSpan min;
  TimeSpan max;
  size_t innerCycles = 0U;
  for (uint_fast8_t i = 0U; i < 100U; ++i)
  {
    bool first = true;

    do
    {
      ++innerCycles;
      GetTime(Clocks::realtimeCoarse, ts_realtime);
      GetTime(Clocks::realtimePrecise, ts_realtimePrecise);

      TP_realtime = ts_realtime;
      TimePoint const TP_realtimePrecise(ts_realtimePrecise);

      TimeSpan const difference = TP_realtimePrecise - TP_realtime;

      if (first)
      {
        first = false;
        inital = TP_realtime;
        min = difference;
        max = difference;
      }
      else
      {
        if (difference < min)
          min = difference;
        if (difference > max)
          max = difference;
      }
    } while (inital == TP_realtime);
  }

  TimeSpan const delta = max - min;
  std::cout << "Inner cycles: " << innerCycles << ", Min ns: " << min.ToString() << ", Max ns: " << max.ToString()
            << ", Max-Min ns: " << delta.ToString() << std::endl;

  EXPECT_GE(min.ns(), 0);
  EXPECT_LT(max.ns(), 2 * MS10_in_NS);
}

TEST(gpcc_time_clock_Tests, GetTime_Monotonic_PreciseAlwaysLargerThanCoarse)
{
  // Test-case is skipped if TFC is present.
  // Rationale: It is not applicable to emulated clocks that do not increase without a sleep.

  struct timespec ts_realtime;
  struct timespec ts_realtimePrecise;
  TimePoint TP_realtime;
  TimePoint inital;
  TimeSpan min;
  TimeSpan max;
  size_t innerCycles = 0U;
  for (uint_fast8_t i = 0U; i < 100U; ++i)
  {
    bool first = true;

    do
    {
      ++innerCycles;
      GetTime(Clocks::monotonicCoarse, ts_realtime);
      GetTime(Clocks::monotonicPrecise, ts_realtimePrecise);

      TP_realtime = ts_realtime;
      TimePoint const TP_realtimePrecise(ts_realtimePrecise);

      TimeSpan const difference = TP_realtimePrecise - TP_realtime;

      if (first)
      {
        first = false;
        inital = TP_realtime;
        min = difference;
        max = difference;
      }
      else
      {
        if (difference < min)
          min = difference;
        if (difference > max)
          max = difference;
      }
    } while (inital == TP_realtime);
  }

  TimeSpan const delta = max - min;
  std::cout << "Inner cycles: " << innerCycles << ", Min ns: " << min.ToString() << ", Max ns: " << max.ToString()
            << ", Max-Min ns: " << delta.ToString() << std::endl;
  EXPECT_GE(min.ns(), 0);
  EXPECT_LT(max.ns(), 2 * MS10_in_NS);
}
#endif

} // namespace time
} // namespace gpcc_tests
