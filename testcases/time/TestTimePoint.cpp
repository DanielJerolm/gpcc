/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include <gpcc_test/compiler/warnings.hpp>
#include <gtest/gtest.h>
#include <limits>
#include <stdexcept>
#include <cassert>

#define MS10_in_NS 10000000L

#define NS_PER_SEC 1000000000L

#define TIME_MULTIPLIER 1

namespace gpcc_tests {
namespace time {

using gpcc::time::TimePoint;
using gpcc::time::Clocks;
using gpcc::time::TimeSpan;

using namespace testing;

TEST(gpcc_time_TimePoint_Tests, DefaultConstructor)
{
  TimePoint uut;

  ASSERT_EQ(0, uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, CopyConstructor)
{
  TimePoint tp(94,23);
  TimePoint uut(tp);

  ASSERT_EQ(94, uut.Get_sec());
  ASSERT_EQ(23, uut.Get_nsec());

  ASSERT_EQ(94, tp.Get_sec());
  ASSERT_EQ(23, tp.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, MoveConstructor)
{
  TimePoint tp(94,23);
  TimePoint uut(std::move(tp));

  ASSERT_EQ(94, uut.Get_sec());
  ASSERT_EQ(23, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromTimespec)
{
  struct timespec ts = {55, 12};

  TimePoint uut(ts);

  ts.tv_sec  = 0;
  ts.tv_nsec = 0;

  ASSERT_EQ(55, uut.Get_sec());
  ASSERT_EQ(12, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromTimespecNotNormalized1)
{
  struct timespec ts = {55, -12};

  TimePoint uut(ts);

  ts.tv_sec  = 0;
  ts.tv_nsec = 0;

  ASSERT_EQ(54, uut.Get_sec());
  ASSERT_EQ(999999988, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromTimespecNotNormalized2)
{
  struct timespec ts = {55, 1000000005};

  TimePoint uut(ts);

  ts.tv_sec  = 0;
  ts.tv_nsec = 0;

  ASSERT_EQ(56, uut.Get_sec());
  ASSERT_EQ(5, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromTimeT)
{
  time_t t = 55;

  TimePoint uut(t);

  t = 0;

  ASSERT_EQ(55, uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromTimeT_Negative)
{
  time_t t = -55;

  TimePoint uut(t);

  t = 0;

  ASSERT_EQ(-55, uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromSecAndNs)
{
  TimePoint uut(45,33);

  ASSERT_EQ(45, uut.Get_sec());
  ASSERT_EQ(33, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, ConstructFromSecNormalization)
{
  TimePoint uut(45,1000000012);

  ASSERT_EQ(46, uut.Get_sec());
  ASSERT_EQ(12, uut.Get_nsec());
}
#ifndef SKIP_LOAD_DEPENDENT_TESTS
#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_TimePoint_Tests, FromSystemClock_Clock_RealtimeCoarse)
{
  // Test-case skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  TimePoint uut = TimePoint::FromSystemClock(Clocks::realtimeCoarse);

  struct timespec ref;
  int const ret = clock_gettime(CLOCK_REALTIME_COARSE, &ref);
  ASSERT_EQ(0, ret);

  TimePoint const TP_Reference(ref);
  TimeSpan const difference = TP_Reference - uut;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::realtimeCoarse) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
TEST(gpcc_time_TimePoint_Tests, FromSystemClock_Clock_RealtimePrecise)
{
  // Test-case skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  TimePoint uut = TimePoint::FromSystemClock(Clocks::realtimePrecise);

  struct timespec ref;
  int const ret = clock_gettime(CLOCK_REALTIME, &ref);
  ASSERT_EQ(0, ret);

  TimePoint const TP_Reference(ref);
  TimeSpan const difference = TP_Reference - uut;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::realtimeCoarse) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
#endif
#endif

#ifndef SKIP_LOAD_DEPENDENT_TESTS
#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
TEST(gpcc_time_TimePoint_Tests, FromSystemClock_Clock_MonotonicCoarse)
{
  // Test-case skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  TimePoint uut = TimePoint::FromSystemClock(Clocks::monotonicCoarse);

  struct timespec ref;
  int const ret = clock_gettime(CLOCK_MONOTONIC_COARSE, &ref);
  ASSERT_EQ(0, ret);

  TimePoint const TP_Reference(ref);
  TimeSpan const difference = TP_Reference - uut;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::realtimeCoarse) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
TEST(gpcc_time_TimePoint_Tests, FromSystemClock_Clock_MonotonicPrecise)
{
  // Test-case skipped if TFC is present.
  // Rationale: No relationship between emulated clock and system clock

  TimePoint uut = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  struct timespec ref;
  int const ret = clock_gettime(CLOCK_MONOTONIC, &ref);
  ASSERT_EQ(0, ret);

  TimePoint const TP_Reference(ref);
  TimeSpan const difference = TP_Reference - uut;

  int64_t const difference_ns = difference.ns();
  std::cout << "Delta (Clocks::realtimeCoarse) (ns): " << difference_ns << std::endl;
  EXPECT_GE(difference_ns, 0);
  EXPECT_LT(difference_ns, MS10_in_NS);
}
#endif
#endif
TEST(gpcc_time_TimePoint_Tests, AssignTimespec)
{
  TimePoint uut;
  struct timespec ts = {12, 88};

  uut = ts;

  ts.tv_sec  = 0;
  ts.tv_nsec = 0;

  ASSERT_EQ(12, uut.Get_sec());
  ASSERT_EQ(88, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, AssignTimespecNormalization)
{
  TimePoint uut;
  struct timespec ts = {12, -88};

  uut = ts;

  ts.tv_sec  = 0;
  ts.tv_nsec = 0;

  ASSERT_EQ(11, uut.Get_sec());
  ASSERT_EQ(999999912, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, AssignTimet)
{
  TimePoint uut(1,1);
  time_t t = 87;

  uut = t;

  t = 0;

  ASSERT_EQ(87, uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());

  t = -87;

  uut = t;

  t = 0;

  ASSERT_EQ(-87, uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, MoveAssign)
{
  TimePoint uut(10,10);
  TimePoint tp(88,12);

  ASSERT_EQ(10, uut.Get_sec());
  ASSERT_EQ(10, uut.Get_nsec());

  uut = std::move(tp);

  ASSERT_EQ(88, uut.Get_sec());
  ASSERT_EQ(12, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, MoveAssignSelf)
{
  TimePoint uut(10,10);

  ASSERT_EQ(10, uut.Get_sec());
  ASSERT_EQ(10, uut.Get_nsec());

  GPCC_DISABLE_WARN_SELFMOVE();
  uut = std::move(uut);
  GPCC_RESTORE_WARN_SELFMOVE();

  ASSERT_EQ(10, uut.Get_sec());
  ASSERT_EQ(10, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, Assign)
{
  TimePoint uut1(11,12);
  TimePoint const uut2(13,14);

  ASSERT_EQ(11, uut1.Get_sec());
  ASSERT_EQ(12, uut1.Get_nsec());
  ASSERT_EQ(13, uut2.Get_sec());
  ASSERT_EQ(14, uut2.Get_nsec());

  uut1 = uut2;

  ASSERT_EQ(13, uut1.Get_sec());
  ASSERT_EQ(14, uut1.Get_nsec());
  ASSERT_EQ(13, uut2.Get_sec());
  ASSERT_EQ(14, uut2.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, AssignSelf)
{
  TimePoint uut(10,10);

  ASSERT_EQ(10, uut.Get_sec());
  ASSERT_EQ(10, uut.Get_nsec());

  // this construct avoids complaints from the eclipse indexer
  #define SELFASSIGN uut = uut
  SELFASSIGN;
  #undef SELFASSIGN

  ASSERT_EQ(10, uut.Get_sec());
  ASSERT_EQ(10, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, OperatorPlusTimespan)
{
  TimePoint const uut(25, 55);
  TimePoint result;

  // add positive timespan
  result = uut + TimeSpan::ms(1250);
  ASSERT_EQ(26, result.Get_sec());
  ASSERT_EQ(250000055, result.Get_nsec());

  // add negative timespan
  result = uut + TimeSpan::ms(-2001);
  ASSERT_EQ(22, result.Get_sec());
  ASSERT_EQ(999000055, result.Get_nsec());

  // negative result
  result = uut + TimeSpan::ms(-31001);
  ASSERT_EQ(-7, result.Get_sec());
  ASSERT_EQ(999000055, result.Get_nsec());

  // normalization present?
  result = uut + TimeSpan::ns(999999999);
  ASSERT_EQ(26, result.Get_sec());
  ASSERT_EQ(54, result.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, OperatorPlusTimespan_BoundOvfl)
{
  TimePoint uut;
  TimePoint result;

  result.Set(88,88);
  uut.Set(std::numeric_limits<time_t>::max(), 999999999);
  ASSERT_THROW(result = uut + TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());
  ASSERT_THROW(result = uut + TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());

  if (sizeof(time_t) == 8U)
  {
    result = uut + TimeSpan::NegativeMaximum();
    ASSERT_EQ(0x7FFFFFFDDA3E82FBLL, result.Get_sec());
    ASSERT_EQ(145224191, result.Get_nsec());
  }
  else
  {
    ASSERT_THROW(result = uut + TimeSpan::NegativeMaximum(), std::overflow_error);
    ASSERT_EQ(88, result.Get_sec());
    ASSERT_EQ(88, result.Get_nsec());
  }

  result.Set(88,88);
  uut.Set(std::numeric_limits<time_t>::min(), 0);
  ASSERT_THROW(result = uut + TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());
  ASSERT_THROW(result = uut + TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    result = uut + TimeSpan::PositiveMaximum();
    ASSERT_EQ(static_cast<time_t>(0x8000000225C17D04LL), result.Get_sec());
    ASSERT_EQ(854775807, result.Get_nsec());
  }
  else
  {
    ASSERT_THROW(result = uut + TimeSpan::PositiveMaximum(), std::overflow_error);
    ASSERT_EQ(88, result.Get_sec());
    ASSERT_EQ(88, result.Get_nsec());
  }
}
TEST(gpcc_time_TimePoint_Tests, OperatorMinusTimespan)
{
  TimePoint const uut(25, 55);
  TimePoint result;

  // sub positive timespan
  result = uut - TimeSpan::ms(1250);
  ASSERT_EQ(23, result.Get_sec());
  ASSERT_EQ(750000055, result.Get_nsec());

  // sub negative timespan
  result = uut - TimeSpan::ms(-2001);
  ASSERT_EQ(27, result.Get_sec());
  ASSERT_EQ(1000055, result.Get_nsec());

  // negative result
  result = uut - TimeSpan::ms(31001);
  ASSERT_EQ(-7, result.Get_sec());
  ASSERT_EQ(999000055, result.Get_nsec());

  // normalization present?
  result = uut - TimeSpan::ns(56);
  ASSERT_EQ(24, result.Get_sec());
  ASSERT_EQ(999999999, result.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, OperatorMinusTimespan_BoundOvfl)
{
  TimePoint uut;
  TimePoint result;

  result.Set(88,88);
  uut.Set(std::numeric_limits<time_t>::max(), 999999999);
  ASSERT_THROW(result = uut - TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());
  ASSERT_THROW(result = uut - TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    result = uut - TimeSpan::PositiveMaximum();
    ASSERT_EQ(0x7FFFFFFDDA3E82FBLL, result.Get_sec());
    ASSERT_EQ(145224192, result.Get_nsec());
  }
  else
  {
    ASSERT_THROW(result = uut - TimeSpan::PositiveMaximum(), std::overflow_error);
    ASSERT_EQ(88, result.Get_sec());
    ASSERT_EQ(88, result.Get_nsec());
  }

  result.Set(88,88);
  uut.Set(std::numeric_limits<time_t>::min(), 0);
  ASSERT_THROW(result = uut - TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());
  ASSERT_THROW(result = uut - TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(88, result.Get_sec());
  ASSERT_EQ(88, result.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    result = uut - TimeSpan::NegativeMaximum();
    ASSERT_EQ(static_cast<time_t>(0x8000000225C17D04LL), result.Get_sec());
    ASSERT_EQ(854775808, result.Get_nsec());
  }
  else
  {
    ASSERT_THROW(result = uut - TimeSpan::NegativeMaximum(), std::overflow_error);
    ASSERT_EQ(88, result.Get_sec());
    ASSERT_EQ(88, result.Get_nsec());
  }
}
TEST(gpcc_time_TimePoint_Tests, OperatorMinusTimepoint)
{
  TimePoint const uut1(55,12);
  TimePoint const uut2(155,55);
  TimeSpan diff;

  diff = uut2 - uut1;
  ASSERT_EQ(100000000043L, diff.ns());

  diff = uut1 - uut2;
  ASSERT_EQ(-100000000043L, diff.ns());
}
TEST(gpcc_time_TimePoint_Tests, OperatorMinusTimepoint_BoundsAndOvfl)
{
  time_t maxSec = (std::numeric_limits<int64_t>::max() / NS_PER_SEC) - 1L;
  if (maxSec > std::numeric_limits<time_t>::max())
    maxSec = std::numeric_limits<time_t>::max();

  time_t minSec = (std::numeric_limits<int64_t>::min() / NS_PER_SEC) + 1L;
  if (minSec < std::numeric_limits<time_t>::min())
    minSec = std::numeric_limits<time_t>::min();

  TimePoint const uut1(maxSec, 999999999L);
  TimePoint const uut2(0, 0);

  ASSERT_EQ(static_cast<int64_t>(maxSec) * NS_PER_SEC + 999999999L, (uut1 - uut2).ns());
  ASSERT_EQ(-static_cast<int64_t>(maxSec) * NS_PER_SEC - 999999999L, (uut2 - uut1).ns());

  TimePoint const uut3(minSec);

  ASSERT_EQ(static_cast<int64_t>(minSec) * NS_PER_SEC, (uut3 - uut2).ns());
  ASSERT_EQ(-static_cast<int64_t>(minSec) * NS_PER_SEC, (uut2 - uut3).ns());

  ASSERT_THROW(uut1 - uut3, std::overflow_error);
  ASSERT_THROW(uut3 - uut1, std::overflow_error);
}
TEST(gpcc_time_TimePoint_Tests, OperatorPlusAssignTimespan)
{
  TimePoint uut(25, 55);

  // add positive timespan
  uut += TimeSpan::ms(1250);
  ASSERT_EQ(26, uut.Get_sec());
  ASSERT_EQ(250000055, uut.Get_nsec());

  // add negative timespan
  uut += TimeSpan::ms(-2001);
  ASSERT_EQ(24, uut.Get_sec());
  ASSERT_EQ(249000055, uut.Get_nsec());

  // negative result
  uut += TimeSpan::ms(-31001);
  ASSERT_EQ(-7, uut.Get_sec());
  ASSERT_EQ(248000055, uut.Get_nsec());

  // normalization present?
  uut += TimeSpan::ns(-248000056);
  ASSERT_EQ(-8, uut.Get_sec());
  ASSERT_EQ(999999999, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, OperatorPlusAssignTimespan_BoundOvfl)
{
  TimePoint uut;

  uut.Set(std::numeric_limits<time_t>::max(), 999999999);
  ASSERT_THROW(uut += TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::max(), uut.Get_sec());
  ASSERT_EQ(999999999, uut.Get_nsec());
  ASSERT_THROW(uut += TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::max(), uut.Get_sec());
  ASSERT_EQ(999999999, uut.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    uut += TimeSpan::NegativeMaximum();
    ASSERT_EQ(0x7FFFFFFDDA3E82FBLL, uut.Get_sec());
    ASSERT_EQ(145224191, uut.Get_nsec());
  }
  else
  {
    ASSERT_THROW(uut += TimeSpan::NegativeMaximum(), std::overflow_error);
    ASSERT_EQ(std::numeric_limits<time_t>::max(), uut.Get_sec());
    ASSERT_EQ(999999999, uut.Get_nsec());
  }

  uut.Set(std::numeric_limits<time_t>::min(), 0);
  ASSERT_THROW(uut += TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::min(), uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());
  ASSERT_THROW(uut += TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::min(), uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    uut += TimeSpan::PositiveMaximum();
    ASSERT_EQ(static_cast<time_t>(0x8000000225C17D04LL), uut.Get_sec());
    ASSERT_EQ(854775807, uut.Get_nsec());
  }
  else
  {
    ASSERT_THROW(uut += TimeSpan::PositiveMaximum(), std::overflow_error);
    ASSERT_EQ(std::numeric_limits<time_t>::min(), uut.Get_sec());
    ASSERT_EQ(0, uut.Get_nsec());
  }
}
TEST(gpcc_time_TimePoint_Tests, OperatorMinusAssignTimespan)
{
  TimePoint uut(25, 55);

  // sub positive timespan
  uut -= TimeSpan::ms(1250);
  ASSERT_EQ(23, uut.Get_sec());
  ASSERT_EQ(750000055, uut.Get_nsec());

  // sub negative timespan
  uut -= TimeSpan::ms(-2001);
  ASSERT_EQ(25, uut.Get_sec());
  ASSERT_EQ(751000055, uut.Get_nsec());

  // negative result
  uut -= TimeSpan::ms(31001);
  ASSERT_EQ(-6, uut.Get_sec());
  ASSERT_EQ(750000055, uut.Get_nsec());

  // normalization present?
  uut -= TimeSpan::ns(750000056);
  ASSERT_EQ(-7, uut.Get_sec());
  ASSERT_EQ(999999999, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, OperatorMinusAssignTimespan_BoundOvfl)
{
  TimePoint uut;

  uut.Set(std::numeric_limits<time_t>::max(), 999999999);
  ASSERT_THROW(uut -= TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::max(), uut.Get_sec());
  ASSERT_EQ(999999999, uut.Get_nsec());
  ASSERT_THROW(uut -= TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::max(), uut.Get_sec());
  ASSERT_EQ(999999999, uut.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    uut -= TimeSpan::PositiveMaximum();
    ASSERT_EQ(0x7FFFFFFDDA3E82FBLL, uut.Get_sec());
    ASSERT_EQ(145224192, uut.Get_nsec());
  }
  else
  {
    ASSERT_THROW(uut -= TimeSpan::PositiveMaximum(), std::overflow_error);
    ASSERT_EQ(std::numeric_limits<time_t>::max(), uut.Get_sec());
    ASSERT_EQ(999999999, uut.Get_nsec());
  }

  uut.Set(std::numeric_limits<time_t>::min(), 0);
  ASSERT_THROW(uut -= TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::min(), uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());
  ASSERT_THROW(uut -= TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(std::numeric_limits<time_t>::min(), uut.Get_sec());
  ASSERT_EQ(0, uut.Get_nsec());

  if (sizeof(time_t) == 8)
  {
    uut -= TimeSpan::NegativeMaximum();
    ASSERT_EQ(static_cast<time_t>(0x8000000225C17D04LL), uut.Get_sec());
    ASSERT_EQ(854775808, uut.Get_nsec());
  }
  else
  {
    ASSERT_THROW(uut -= TimeSpan::NegativeMaximum(), std::overflow_error);
    ASSERT_EQ(std::numeric_limits<time_t>::min(), uut.Get_sec());
    ASSERT_EQ(0, uut.Get_nsec());
  }
}
TEST(gpcc_time_TimePoint_Tests, OperatorLessThan)
{
  TimePoint uut1(10, 10);
  TimePoint uut2(10, 10);

  ASSERT_FALSE(uut1 < uut2);

  uut2.Set(10, 11);
  ASSERT_TRUE(uut1 < uut2);

  uut2.Set(11, 10);
  ASSERT_TRUE(uut1 < uut2);

  uut1.Set(-5, 10);
  uut2.Set(-5, 10);
  ASSERT_FALSE(uut1 < uut2);

  uut2.Set(-5, 11);
  ASSERT_TRUE(uut1 < uut2);

  uut2.Set(-4, 10);
  ASSERT_TRUE(uut1 < uut2);

  uut2.Set(-6, 999999999);
  ASSERT_FALSE(uut1 < uut2);
}
TEST(gpcc_time_TimePoint_Tests, OperatorLessThanOrEqual)
{
  TimePoint uut1(10, 10);
  TimePoint uut2(10, 10);

  ASSERT_TRUE(uut1 <= uut2);

  uut2.Set(10, 9);
  ASSERT_FALSE(uut1 <= uut2);

  uut2.Set(10, 11);
  ASSERT_TRUE(uut1 <= uut2);

  uut2.Set(11, 10);
  ASSERT_TRUE(uut1 <= uut2);

  uut1.Set(-5, 10);
  uut2.Set(-5, 10);
  ASSERT_TRUE(uut1 <= uut2);

  uut2.Set(-5, 9);
  ASSERT_FALSE(uut1 <= uut2);

  uut2.Set(-5, 11);
  ASSERT_TRUE(uut1 <= uut2);

  uut2.Set(-4, 10);
  ASSERT_TRUE(uut1 <= uut2);

  uut2.Set(-6, 999999999);
  ASSERT_FALSE(uut1 <= uut2);
}
TEST(gpcc_time_TimePoint_Tests, OperatorGreaterThan)
{
  TimePoint uut1(10, 10);
  TimePoint uut2(10, 10);

  ASSERT_FALSE(uut2 > uut1);

  uut2.Set(10, 11);
  ASSERT_TRUE(uut2 > uut1);

  uut2.Set(11, 10);
  ASSERT_TRUE(uut2 > uut1);

  uut1.Set(-5, 10);
  uut2.Set(-5, 10);
  ASSERT_FALSE(uut2 > uut1);

  uut2.Set(-5, 11);
  ASSERT_TRUE(uut2 > uut1);

  uut2.Set(-4, 10);
  ASSERT_TRUE(uut2 > uut1);

  uut2.Set(-6, 999999999);
  ASSERT_FALSE(uut2 > uut1);
}
TEST(gpcc_time_TimePoint_Tests, OperatorGreaterThanOrEqual)
{
  TimePoint uut1(10, 10);
  TimePoint uut2(10, 10);

  ASSERT_TRUE(uut2 >= uut1);

  uut2.Set(10, 9);
  ASSERT_FALSE(uut2 >= uut1);

  uut2.Set(10, 11);
  ASSERT_TRUE(uut2 >= uut1);

  uut2.Set(11, 10);
  ASSERT_TRUE(uut2 >= uut1);

  uut1.Set(-5, 10);
  uut2.Set(-5, 10);
  ASSERT_TRUE(uut2 >= uut1);

  uut2.Set(-5, 9);
  ASSERT_FALSE(uut2 >= uut1);

  uut2.Set(-5, 11);
  ASSERT_TRUE(uut2 >= uut1);

  uut2.Set(-4, 10);
  ASSERT_TRUE(uut2 >= uut1);

  uut2.Set(-6, 999999999);
  ASSERT_FALSE(uut2 >= uut1);
}
TEST(gpcc_time_TimePoint_Tests, OperatorEqual)
{
  TimePoint uut1(10,10);
  TimePoint uut2(10,10);

  ASSERT_TRUE(uut1 == uut2);

  uut2.Set(9, 10);
  ASSERT_FALSE(uut1 == uut2);

  uut2.Set(10, 9);
  ASSERT_FALSE(uut1 == uut2);
}
TEST(gpcc_time_TimePoint_Tests, OperatorNotEqual)
{
  TimePoint uut1(10,10);
  TimePoint uut2(10,10);

  ASSERT_FALSE(uut1 != uut2);

  uut2.Set(9, 10);
  ASSERT_TRUE(uut1 != uut2);

  uut2.Set(10, 9);
  ASSERT_TRUE(uut1 != uut2);
}
#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST(gpcc_time_TimePoint_Tests, LatchSystemClock_Clock_Realtime)
{
  TimePoint uut1 = TimePoint::FromSystemClock(Clocks::realtimeCoarse);
  TimePoint uut2;
  uut2.LatchSystemClock(Clocks::realtimeCoarse);

  TimeSpan const delta = uut2 - uut1;

  ASSERT_TRUE(delta.ns() >= 0);
  ASSERT_TRUE(delta.ns() <= 100000000LL * TIME_MULTIPLIER);
}
#endif
#ifndef SKIP_LOAD_DEPENDENT_TESTS
TEST(gpcc_time_TimePoint_Tests, LatchSystemClock_Clock_Monotonic)
{
  TimePoint uut1 = TimePoint::FromSystemClock(Clocks::monotonicCoarse);
  TimePoint uut2;
  uut2.LatchSystemClock(Clocks::monotonicCoarse);

  TimeSpan const delta = uut2 - uut1;

  ASSERT_TRUE(delta.ns() >= 0);
  ASSERT_TRUE(delta.ns() <= 100000000LL * TIME_MULTIPLIER);
}
TEST(gpcc_time_TimePoint_Tests, LatchSystemClock_Clock_MonotonicPrecise)
{
  TimePoint uut1 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);
  TimePoint uut2;
  uut2.LatchSystemClock(Clocks::monotonicPrecise);

  TimeSpan const delta = uut2 - uut1;

  ASSERT_TRUE(delta.ns() >= 0);
  ASSERT_TRUE(delta.ns() <= 100000000LL * TIME_MULTIPLIER);
}
#endif
TEST(gpcc_time_TimePoint_Tests, Set)
{
  TimePoint uut;

  uut.Set(94, 23);
  ASSERT_EQ(94, uut.Get_sec());
  ASSERT_EQ(23, uut.Get_nsec());

  uut.Set(-12, 55);
  ASSERT_EQ(-12, uut.Get_sec());
  ASSERT_EQ(55, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, SetNormalization)
{
  TimePoint uut;

  uut.Set(100, -5);
  ASSERT_EQ(99, uut.Get_sec());
  ASSERT_EQ(999999995, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, Normalization)
{
  TimePoint uut;

  uut.Set(1, 1000000567);
  ASSERT_EQ(2, uut.Get_sec());
  ASSERT_EQ(567, uut.Get_nsec());

  uut.Set(2, 2000000123);
  ASSERT_EQ(4, uut.Get_sec());
  ASSERT_EQ(123, uut.Get_nsec());

  uut.Set(3, -15);
  ASSERT_EQ(2, uut.Get_sec());
  ASSERT_EQ(999999985, uut.Get_nsec());

  uut.Set(4, -1000000898);
  ASSERT_EQ(2, uut.Get_sec());
  ASSERT_EQ(999999102, uut.Get_nsec());

  uut.Set(5, -2000000111);
  ASSERT_EQ(2, uut.Get_sec());
  ASSERT_EQ(999999889, uut.Get_nsec());

  uut.Set(-1, -12);
  ASSERT_EQ(-2, uut.Get_sec());
  ASSERT_EQ(999999988, uut.Get_nsec());

  uut.Set(-2, -1000000654);
  ASSERT_EQ(-4, uut.Get_sec());
  ASSERT_EQ(999999346, uut.Get_nsec());

  uut.Set(-3, -2000000536);
  ASSERT_EQ(-6, uut.Get_sec());
  ASSERT_EQ(999999464, uut.Get_nsec());
}
TEST(gpcc_time_TimePoint_Tests, GetTimeSpecRef)
{
  TimePoint uut(1,2);

  struct timespec const & ts = uut.Get_timespec_ref();

  ASSERT_EQ(1, ts.tv_sec);
  ASSERT_EQ(2, ts.tv_nsec);

  char const * const pcUUT = reinterpret_cast<char const*>(&uut);
  char const * const pcTS  = reinterpret_cast<char const*>(&ts);

  // verify that the referenced timespec is inside the uut object
  ASSERT_TRUE((pcTS >= pcUUT) && (pcTS <= pcUUT + sizeof(TimePoint) - sizeof(ts.tv_nsec)));
}
TEST(gpcc_time_TimePoint_Tests, GetTimeSpecPtr)
{
  TimePoint uut(1,2);

  struct timespec const * const pTS = uut.Get_timespec_ptr();

  ASSERT_EQ(1, pTS->tv_sec);
  ASSERT_EQ(2, pTS->tv_nsec);

  char const * const pcUUT = reinterpret_cast<char const*>(&uut);
  char const * const pcTS  = reinterpret_cast<char const*>(pTS);

  // verify that the referenced timespec is inside the uut object
  ASSERT_TRUE((pcTS >= pcUUT) && (pcTS <= pcUUT + sizeof(TimePoint) - sizeof(pTS->tv_nsec)));
}
TEST(gpcc_time_TimePoint_Tests, ToStringEpoch)
{
  TimePoint tp(0,0);
  std::string s(tp.ToString());
  ASSERT_TRUE(s == "1970-01-01 00:00:00.000ms");
  ASSERT_TRUE(s.length() == TimePoint::stringLength);
}
TEST(gpcc_time_TimePoint_Tests, ToString)
{
  // dummy timepoint (31.10.2016, 16:31:55)
  struct tm t1;
  t1.tm_year  = 2016  - 1900;
  t1.tm_mon   = 10    - 1;
  t1.tm_mday  = 31;
  t1.tm_hour  = 16;
  t1.tm_min   = 31;
  t1.tm_sec   = 55;

  // create time_t (UTC!) from dummy timepoint
  time_t const t2 = timegm(&t1);
  assert(t2 != -1);

  // create TimePoint from t2 + 559ms
  TimePoint tp(t2);
  tp += TimeSpan::us(559000);

  std::string s(tp.ToString());
  ASSERT_TRUE(s == "2016-10-31 16:31:55.559ms");
  ASSERT_TRUE(s.length() == TimePoint::stringLength);

  tp -= TimeSpan::ms(550);
  s = tp.ToString();
  ASSERT_TRUE(s == "2016-10-31 16:31:55.009ms");
  ASSERT_TRUE(s.length() == TimePoint::stringLength);

  tp -= TimeSpan::ms(9);
  s = tp.ToString();
  ASSERT_TRUE(s == "2016-10-31 16:31:55.000ms");
  ASSERT_TRUE(s.length() == TimePoint::stringLength);
}

} // namespace time
} // namespace gpcc_tests
