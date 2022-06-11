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

#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <limits>
#include <stdexcept>

namespace gpcc_tests {
namespace time {

using gpcc::time::TimeSpan;

using namespace testing;

#define NS_PER_US  1000
#define NS_PER_MS  1000000
#define NS_PER_SEC 1000000000
#define NS_PER_MIN (60LL * NS_PER_SEC) // (64bit!)
#define NS_PER_HR  (60   * NS_PER_MIN) // (64bit!)
#define NS_PER_DAY (24   * NS_PER_HR)  // (64bit!)

TEST(gpcc_time_Timespan_Tests, Create_ns)
{
  TimeSpan const ts_min = TimeSpan::ns(std::numeric_limits<int64_t>::min());
  ASSERT_EQ(std::numeric_limits<int64_t>::min(), ts_min.ns());

  TimeSpan const ts_max = TimeSpan::ns(std::numeric_limits<int64_t>::max());
  ASSERT_EQ(std::numeric_limits<int64_t>::max(), ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_us)
{
  TimeSpan const ts_min = TimeSpan::us(std::numeric_limits<int64_t>::min() / NS_PER_US);
  ASSERT_EQ((std::numeric_limits<int64_t>::min() / NS_PER_US) * NS_PER_US, ts_min.ns());

  TimeSpan const ts_max = TimeSpan::us(std::numeric_limits<int64_t>::max() / NS_PER_US);
  ASSERT_EQ((std::numeric_limits<int64_t>::max() / NS_PER_US) * NS_PER_US, ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_us_Ovfl)
{
  ASSERT_THROW({TimeSpan const ts_min = TimeSpan::us(std::numeric_limits<int64_t>::min() / NS_PER_US - 1); (void)ts_min;}, std::overflow_error);
  ASSERT_THROW({TimeSpan const ts_max = TimeSpan::us(std::numeric_limits<int64_t>::max() / NS_PER_US + 1); (void)ts_max;}, std::overflow_error);
}
TEST(gpcc_time_Timespan_Tests, Create_ms)
{
  TimeSpan const ts_min = TimeSpan::ms(std::numeric_limits<int64_t>::min() / NS_PER_MS);
  ASSERT_EQ((std::numeric_limits<int64_t>::min() / NS_PER_MS) * NS_PER_MS, ts_min.ns());

  TimeSpan const ts_max = TimeSpan::ms(std::numeric_limits<int64_t>::max() / NS_PER_MS);
  ASSERT_EQ((std::numeric_limits<int64_t>::max() / NS_PER_MS) * NS_PER_MS, ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_ms_Ovfl)
{
  ASSERT_THROW({TimeSpan const ts_min = TimeSpan::ms(std::numeric_limits<int64_t>::min() / NS_PER_MS - 1); (void)ts_min;}, std::overflow_error);
  ASSERT_THROW({TimeSpan const ts_max = TimeSpan::ms(std::numeric_limits<int64_t>::max() / NS_PER_MS + 1); (void)ts_max;}, std::overflow_error);
}
TEST(gpcc_time_Timespan_Tests, Create_sec)
{
  TimeSpan const ts_min = TimeSpan::sec(std::numeric_limits<int64_t>::min() / NS_PER_SEC);
  ASSERT_EQ((std::numeric_limits<int64_t>::min() / NS_PER_SEC) * NS_PER_SEC, ts_min.ns());

  TimeSpan const ts_max = TimeSpan::sec(std::numeric_limits<int64_t>::max() / NS_PER_SEC);
  ASSERT_EQ((std::numeric_limits<int64_t>::max() / NS_PER_SEC) * NS_PER_SEC, ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_sec_Ovfl)
{
  ASSERT_THROW({TimeSpan const ts_min = TimeSpan::sec(std::numeric_limits<int64_t>::min() / NS_PER_SEC - 1); (void)ts_min;}, std::overflow_error);
  ASSERT_THROW({TimeSpan const ts_max = TimeSpan::sec(std::numeric_limits<int64_t>::max() / NS_PER_SEC + 1); (void)ts_max;}, std::overflow_error);
}
TEST(gpcc_time_Timespan_Tests, Create_min)
{
  TimeSpan const ts_min = TimeSpan::min(std::numeric_limits<int64_t>::min() / NS_PER_MIN);
  ASSERT_EQ((std::numeric_limits<int64_t>::min() / NS_PER_MIN) * NS_PER_MIN, ts_min.ns());

  TimeSpan const ts_max = TimeSpan::min(std::numeric_limits<int64_t>::max() / NS_PER_MIN);
  ASSERT_EQ((std::numeric_limits<int64_t>::max() / NS_PER_MIN) * NS_PER_MIN, ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_min_Ovfl)
{
  ASSERT_THROW({TimeSpan const ts_min = TimeSpan::min(std::numeric_limits<int64_t>::min() / NS_PER_MIN - 1); (void)ts_min;}, std::overflow_error);
  ASSERT_THROW({TimeSpan const ts_max = TimeSpan::min(std::numeric_limits<int64_t>::max() / NS_PER_MIN + 1); (void)ts_max;}, std::overflow_error);
}
TEST(gpcc_time_Timespan_Tests, Create_hr)
{
  TimeSpan const ts_min = TimeSpan::hr(std::numeric_limits<int64_t>::min() / NS_PER_HR);
  ASSERT_EQ((std::numeric_limits<int64_t>::min() / NS_PER_HR) * NS_PER_HR, ts_min.ns());

  TimeSpan const ts_max = TimeSpan::hr(std::numeric_limits<int64_t>::max() / NS_PER_HR);
  ASSERT_EQ((std::numeric_limits<int64_t>::max() / NS_PER_HR) * NS_PER_HR, ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_hr_Ovfl)
{
  ASSERT_THROW({TimeSpan const ts_min = TimeSpan::hr(std::numeric_limits<int64_t>::min() / NS_PER_HR - 1); (void)ts_min;}, std::overflow_error);
  ASSERT_THROW({TimeSpan const ts_max = TimeSpan::hr(std::numeric_limits<int64_t>::max() / NS_PER_HR + 1); (void)ts_max;}, std::overflow_error);
}
TEST(gpcc_time_Timespan_Tests, Create_days)
{
  TimeSpan const ts_min = TimeSpan::days(std::numeric_limits<int64_t>::min() / NS_PER_DAY);
  ASSERT_EQ((std::numeric_limits<int64_t>::min() / NS_PER_DAY) * NS_PER_DAY, ts_min.ns());

  TimeSpan const ts_max = TimeSpan::days(std::numeric_limits<int64_t>::max() / NS_PER_DAY);
  ASSERT_EQ((std::numeric_limits<int64_t>::max() / NS_PER_DAY) * NS_PER_DAY, ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_days_Ovfl)
{
  ASSERT_THROW({TimeSpan const ts_min = TimeSpan::days(std::numeric_limits<int64_t>::min() / NS_PER_DAY - 1); (void)ts_min;}, std::overflow_error);
  ASSERT_THROW({TimeSpan const ts_max = TimeSpan::days(std::numeric_limits<int64_t>::max() / NS_PER_DAY + 1); (void)ts_max;}, std::overflow_error);
}
TEST(gpcc_time_Timespan_Tests, Create_NegativeMaximum)
{
  TimeSpan ts_min = TimeSpan::NegativeMaximum();
  ASSERT_EQ(std::numeric_limits<int64_t>::min(), ts_min.ns());
}
TEST(gpcc_time_Timespan_Tests, Create_PositiveMaximum)
{
  TimeSpan ts_max = TimeSpan::PositiveMaximum();
  ASSERT_EQ(std::numeric_limits<int64_t>::max(), ts_max.ns());
}
TEST(gpcc_time_Timespan_Tests, CopyConstruction)
{
  TimeSpan const ts1 = TimeSpan::ns(10);
  TimeSpan ts2(ts1);

  ASSERT_EQ(10, ts2.ns());
}
TEST(gpcc_time_Timespan_Tests, MoveConstruction)
{
  TimeSpan ts1 = TimeSpan::ns(10);
  TimeSpan ts2(std::move(ts1));

  ASSERT_EQ(10, ts2.ns());
}
TEST(gpcc_time_Timespan_Tests, CopyAssignment)
{
  TimeSpan const ts1 = TimeSpan::ns(10);
  TimeSpan ts2 = TimeSpan::ns(100);

  ts2 = ts1;

  ASSERT_EQ(10, ts2.ns());
}
TEST(gpcc_time_Timespan_Tests, CopyAssignmentSelf)
{
  TimeSpan ts1 = TimeSpan::ns(10);

  // do it this way to avoid complaints from eclipse indexer
  #define SELFASSIGNMENT ts1 = ts1;
  SELFASSIGNMENT;
  #undef SELFASSIGNMENT

  ASSERT_EQ(10, ts1.ns());
}
TEST(gpcc_time_Timespan_Tests, MoveAssignment)
{
  TimeSpan ts1 = TimeSpan::ns(10);
  TimeSpan ts2 = TimeSpan::ns(100);

  ts2 = std::move(ts1);

  ASSERT_EQ(10, ts2.ns());
}
TEST(gpcc_time_Timespan_Tests, MoveAssignmentSelf)
{
  TimeSpan ts1 = TimeSpan::ns(10);

  // do it this way to avoid complaints from eclipse indexer
  #define SELFASSIGNMENT ts1 = std::move(ts1);
  SELFASSIGNMENT;
  #undef SELFASSIGNMENT

  ASSERT_EQ(10, ts1.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorPlus)
{
  TimeSpan const ts1 = TimeSpan::ns(10);
  TimeSpan const ts2 = TimeSpan::ns(100);
  TimeSpan const ts3 = TimeSpan::ns(-20);
  TimeSpan sum;

  sum = ts1 + ts2;
  ASSERT_EQ(110, sum.ns());

  sum = ts1 + ts3;
  ASSERT_EQ(-10, sum.ns());

  sum = sum + ts2;
  ASSERT_EQ(90, sum.ns());

  sum = sum + sum;
  ASSERT_EQ(180, sum.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorPlus_BoundOvfl)
{
  TimeSpan ts;
  TimeSpan sum;

  ts  = TimeSpan::PositiveMaximum();
  sum = TimeSpan::ns(3);

  ASSERT_THROW(sum = ts + TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(3, sum.ns());
  ASSERT_THROW(sum = ts + TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(3, sum.ns());

  sum = ts + TimeSpan::NegativeMaximum();
  ASSERT_EQ(-1, sum.ns());

  ts  = TimeSpan::NegativeMaximum();
  sum = TimeSpan::ns(3);

  ASSERT_THROW(sum = ts + TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(3, sum.ns());
  ASSERT_THROW(sum = ts + TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(3, sum.ns());

  sum = ts + TimeSpan::PositiveMaximum();
  ASSERT_EQ(-1, sum.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorMinus)
{
  TimeSpan const ts1 = TimeSpan::ns(10);
  TimeSpan const ts2 = TimeSpan::ns(100);
  TimeSpan const ts3 = TimeSpan::ns(-20);
  TimeSpan diff;

  diff = ts1 - ts2;
  ASSERT_EQ(-90, diff.ns());

  diff = ts1 - ts3;
  ASSERT_EQ(30, diff.ns());

  diff = diff - ts2;
  ASSERT_EQ(-70, diff.ns());

  diff = diff - diff;
  ASSERT_EQ(0, diff.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorMinus_BoundsOvfl)
{
  TimeSpan ts;
  TimeSpan diff;

  ts  = TimeSpan::PositiveMaximum();
  diff = TimeSpan::ns(3);

  ASSERT_THROW(diff = ts - TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(3, diff.ns());
  ASSERT_THROW(diff = ts - TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(3, diff.ns());

  diff = ts - TimeSpan::PositiveMaximum();
  ASSERT_EQ(0, diff.ns());

  ts  = TimeSpan::NegativeMaximum();
  diff = TimeSpan::ns(3);

  ASSERT_THROW(diff = ts - TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(3, diff.ns());
  ASSERT_THROW(diff = ts - TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(3, diff.ns());

  diff = ts - TimeSpan::NegativeMaximum();
  ASSERT_EQ(0, diff.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorPlusAssign)
{
  TimeSpan const ts1 = TimeSpan::ns(10);
  TimeSpan const ts2 = TimeSpan::ns(100);
  TimeSpan const ts3 = TimeSpan::ns(-20);
  TimeSpan const ts4 = TimeSpan::ns(0);
  TimeSpan sum = TimeSpan::ns(0);

  sum += ts1;
  ASSERT_EQ(10, sum.ns());

  sum += ts3;
  ASSERT_EQ(-10, sum.ns());

  sum += ts2;
  ASSERT_EQ(90, sum.ns());

  sum += ts4;
  ASSERT_EQ(90, sum.ns());

  sum += sum;
  ASSERT_EQ(180, sum.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorPlusAssign_BoundsOvfl)
{
  TimeSpan sum;

  sum = TimeSpan::PositiveMaximum();

  ASSERT_THROW(sum += TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(TimeSpan::PositiveMaximum().ns(), sum.ns());
  ASSERT_THROW(sum += TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(TimeSpan::PositiveMaximum().ns(), sum.ns());

  sum += TimeSpan::NegativeMaximum();
  ASSERT_EQ(-1, sum.ns());


  sum = TimeSpan::NegativeMaximum();
  ASSERT_THROW(sum += TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(TimeSpan::NegativeMaximum().ns(), sum.ns());
  ASSERT_THROW(sum += TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(TimeSpan::NegativeMaximum().ns(), sum.ns());

  sum += TimeSpan::PositiveMaximum();
  ASSERT_EQ(-1, sum.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorMinusAssign)
{
  TimeSpan const ts1 = TimeSpan::ns(10);
  TimeSpan const ts2 = TimeSpan::ns(100);
  TimeSpan const ts3 = TimeSpan::ns(-20);
  TimeSpan const ts4 = TimeSpan::ns(0);
  TimeSpan diff = TimeSpan::ns(0);

  diff -= ts1;
  ASSERT_EQ(-10, diff.ns());

  diff -= ts3;
  ASSERT_EQ(10, diff.ns());

  diff -= ts2;
  ASSERT_EQ(-90, diff.ns());

  diff -= ts4;
  ASSERT_EQ(-90, diff.ns());

  diff -= diff;
  ASSERT_EQ(0, diff.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorMinusAssign_BoundsOvfl)
{
  TimeSpan sum;

  sum = TimeSpan::PositiveMaximum();

  ASSERT_THROW(sum -= TimeSpan::ns(-1), std::overflow_error);
  ASSERT_EQ(TimeSpan::PositiveMaximum().ns(), sum.ns());
  ASSERT_THROW(sum -= TimeSpan::NegativeMaximum(), std::overflow_error);
  ASSERT_EQ(TimeSpan::PositiveMaximum().ns(), sum.ns());

  sum -= TimeSpan::PositiveMaximum();
  ASSERT_EQ(0, sum.ns());


  sum = TimeSpan::NegativeMaximum();
  ASSERT_THROW(sum -= TimeSpan::ns(1), std::overflow_error);
  ASSERT_EQ(TimeSpan::NegativeMaximum().ns(), sum.ns());
  ASSERT_THROW(sum -= TimeSpan::PositiveMaximum(), std::overflow_error);
  ASSERT_EQ(TimeSpan::NegativeMaximum().ns(), sum.ns());

  sum -= TimeSpan::NegativeMaximum();
  ASSERT_EQ(0, sum.ns());
}
TEST(gpcc_time_Timespan_Tests, OperatorLessThan)
{
  TimeSpan const uut1 = TimeSpan::ns(10);
  TimeSpan const uut2 = TimeSpan::ns(11);

  ASSERT_TRUE(uut1 < uut2);
  ASSERT_FALSE(uut2 < uut1);
  ASSERT_FALSE(uut1 < uut1);
}
TEST(gpcc_time_Timespan_Tests, OperatorLessThanOrEqual)
{
  TimeSpan const uut1 = TimeSpan::ns(10);
  TimeSpan const uut2 = TimeSpan::ns(11);

  ASSERT_TRUE(uut1 <= uut2);
  ASSERT_FALSE(uut2 <= uut1);
  ASSERT_TRUE(uut1 <= uut1);
}
TEST(gpcc_time_Timespan_Tests, OperatorGreaterThan)
{
  TimeSpan const uut1 = TimeSpan::ns(10);
  TimeSpan const uut2 = TimeSpan::ns(11);

  ASSERT_FALSE(uut1 > uut2);
  ASSERT_TRUE(uut2 > uut1);
  ASSERT_FALSE(uut1 > uut1);
}
TEST(gpcc_time_Timespan_Tests, OperatorGreaterThanOrEqual)
{
  TimeSpan const uut1 = TimeSpan::ns(10);
  TimeSpan const uut2 = TimeSpan::ns(11);

  ASSERT_FALSE(uut1 >= uut2);
  ASSERT_TRUE(uut2 >= uut1);
  ASSERT_TRUE(uut1 >= uut1);
}
TEST(gpcc_time_Timespan_Tests, OperatorEqual)
{
  TimeSpan const uut1 = TimeSpan::ns(10);
  TimeSpan const uut2 = TimeSpan::ns(11);
  TimeSpan const uut3 = TimeSpan::ns(11);

  ASSERT_FALSE(uut1 == uut2);
  ASSERT_TRUE(uut2 == uut3);
}
TEST(gpcc_time_Timespan_Tests, OperatorNotEqual)
{
  TimeSpan const uut1 = TimeSpan::ns(10);
  TimeSpan const uut2 = TimeSpan::ns(11);
  TimeSpan const uut3 = TimeSpan::ns(11);

  ASSERT_TRUE(uut1 != uut2);
  ASSERT_FALSE(uut2 != uut3);
}
TEST(gpcc_time_Timespan_Tests, Get_us)
{
  TimeSpan ts = TimeSpan::ns(NS_PER_US-1);

  ASSERT_EQ(0, ts.us());
  ts += TimeSpan::ns(1);
  ASSERT_EQ(1, ts.us());

  ts = TimeSpan::ns(-NS_PER_US+1);
  ASSERT_EQ(0, ts.us());
  ts -= TimeSpan::ns(1);
  ASSERT_EQ(-1, ts.us());
}
TEST(gpcc_time_Timespan_Tests, Get_ms)
{
  TimeSpan ts = TimeSpan::ns(NS_PER_MS-1);

  ASSERT_EQ(0, ts.ms());
  ts += TimeSpan::ns(1);
  ASSERT_EQ(1, ts.ms());

  ts = TimeSpan::ns(-NS_PER_MS+1);
  ASSERT_EQ(0, ts.ms());
  ts -= TimeSpan::ns(1);
  ASSERT_EQ(-1, ts.ms());
}
TEST(gpcc_time_Timespan_Tests, Get_sec)
{
  TimeSpan ts = TimeSpan::ns(NS_PER_SEC-1);

  ASSERT_EQ(0, ts.sec());
  ts += TimeSpan::ns(1);
  ASSERT_EQ(1, ts.sec());

  ts = TimeSpan::ns(-NS_PER_SEC+1);
  ASSERT_EQ(0, ts.sec());
  ts -= TimeSpan::ns(1);
  ASSERT_EQ(-1, ts.sec());
}
TEST(gpcc_time_Timespan_Tests, Get_min)
{
  TimeSpan ts = TimeSpan::ns(NS_PER_MIN-1);

  ASSERT_EQ(0, ts.min());
  ts += TimeSpan::ns(1);
  ASSERT_EQ(1, ts.min());

  ts = TimeSpan::ns(-NS_PER_MIN+1);
  ASSERT_EQ(0, ts.min());
  ts -= TimeSpan::ns(1);
  ASSERT_EQ(-1, ts.min());
}
TEST(gpcc_time_Timespan_Tests, Get_hr)
{
  TimeSpan ts = TimeSpan::ns(NS_PER_HR-1);

  ASSERT_EQ(0, ts.hr());
  ts += TimeSpan::ns(1);
  ASSERT_EQ(1, ts.hr());

  ts = TimeSpan::ns(-NS_PER_HR+1);
  ASSERT_EQ(0, ts.hr());
  ts -= TimeSpan::ns(1);
  ASSERT_EQ(-1, ts.hr());
}
TEST(gpcc_time_Timespan_Tests, Get_days)
{
  TimeSpan ts = TimeSpan::ns(NS_PER_DAY-1);

  ASSERT_EQ(0, ts.days());
  ts += TimeSpan::ns(1);
  ASSERT_EQ(1, ts.days());

  ts = TimeSpan::ns(-NS_PER_DAY+1);
  ASSERT_EQ(0, ts.days());
  ts -= TimeSpan::ns(1);
  ASSERT_EQ(-1, ts.days());
}
TEST(gpcc_time_Timespan_Tests, ToString_posValues)
{
  TimeSpan ts;
  std::string s;

  ts = TimeSpan::days(130) + TimeSpan::min(3) + TimeSpan::sec(55) + TimeSpan::ms(12) + TimeSpan::ns(133);
  s = ts.ToString();
  ASSERT_EQ("130d 00:03:55.012000133ns", s);

  ts = TimeSpan::hr(23) + TimeSpan::min(3) + TimeSpan::sec(55) + TimeSpan::ms(12);
  s = ts.ToString();
  ASSERT_EQ("23:03:55.012ms", s);

  ts = TimeSpan::min(3) + TimeSpan::sec(55) + TimeSpan::ms(12) + TimeSpan::us(1);
  s = ts.ToString();
  ASSERT_EQ("3:55.012001us", s);

  ts = TimeSpan::min(3) + TimeSpan::sec(55) + TimeSpan::ms(12) + TimeSpan::ns(133);
  s = ts.ToString();
  ASSERT_EQ("3:55.012000133ns", s);

  ts = TimeSpan::min(3) + TimeSpan::sec(55) + TimeSpan::ms(12);
  s = ts.ToString();
  ASSERT_EQ("3:55.012ms", s);

  ts = TimeSpan::min(3) + TimeSpan::sec(55);
  s = ts.ToString();
  ASSERT_EQ("3:55min", s);

  ts = TimeSpan::min(3);
  s = ts.ToString();
  ASSERT_EQ("3min", s);

  ts = TimeSpan::min(59);
  s = ts.ToString();
  ASSERT_EQ("59min", s);

  ts = TimeSpan::sec(55);
  s = ts.ToString();
  ASSERT_EQ("55sec", s);

  ts = TimeSpan::sec(6);
  s = ts.ToString();
  ASSERT_EQ("6sec", s);

  ts = TimeSpan::sec(55) + TimeSpan::ms(3);
  s = ts.ToString();
  ASSERT_EQ("0:55.003ms", s);

  ts = TimeSpan::sec(55) + TimeSpan::us(3);
  s = ts.ToString();
  ASSERT_EQ("0:55.000003us", s);

  ts = TimeSpan::sec(6) + TimeSpan::ms(3);
  s = ts.ToString();
  ASSERT_EQ("0:06.003ms", s);

  ts = TimeSpan::us(3)+ TimeSpan::ns(1);
  s = ts.ToString();
  ASSERT_EQ("3001ns", s);

  ts = TimeSpan::us(3);
  s = ts.ToString();
  ASSERT_EQ("3us", s);

  ts = TimeSpan::ns(3);
  s = ts.ToString();
  ASSERT_EQ("3ns", s);

  ts = TimeSpan::ns(0);
  s = ts.ToString();
  ASSERT_EQ("0ns", s);
}
TEST(gpcc_time_Timespan_Tests, ToString_negValues)
{
  TimeSpan ts;
  std::string s;

  ts = TimeSpan::days(-130) + TimeSpan::min(-3) + TimeSpan::sec(-55) + TimeSpan::ms(-12) + TimeSpan::ns(-133);
  s = ts.ToString();
  ASSERT_EQ("-130d 00:03:55.012000133ns", s);

  ts = TimeSpan::hr(-23) + TimeSpan::min(-3) + TimeSpan::sec(-55) + TimeSpan::ms(-12);
  s = ts.ToString();
  ASSERT_EQ("-23:03:55.012ms", s);

  ts = TimeSpan::min(-3) + TimeSpan::sec(-55) + TimeSpan::ms(-12) + TimeSpan::us(-1);
  s = ts.ToString();
  ASSERT_EQ("-3:55.012001us", s);

  ts = TimeSpan::min(-3) + TimeSpan::sec(-55) + TimeSpan::ms(-12) + TimeSpan::ns(-133);
  s = ts.ToString();
  ASSERT_EQ("-3:55.012000133ns", s);

  ts = TimeSpan::min(-3) + TimeSpan::sec(-55) + TimeSpan::ms(-12);
  s = ts.ToString();
  ASSERT_EQ("-3:55.012ms", s);

  ts = TimeSpan::min(-3) + TimeSpan::sec(-55);
  s = ts.ToString();
  ASSERT_EQ("-3:55min", s);

  ts = TimeSpan::min(-3);
  s = ts.ToString();
  ASSERT_EQ("-3min", s);

  ts = TimeSpan::min(-59);
  s = ts.ToString();
  ASSERT_EQ("-59min", s);

  ts = TimeSpan::sec(-55);
  s = ts.ToString();
  ASSERT_EQ("-55sec", s);

  ts = TimeSpan::sec(-6);
  s = ts.ToString();
  ASSERT_EQ("-6sec", s);

  ts = TimeSpan::sec(-55) + TimeSpan::ms(-3);
  s = ts.ToString();
  ASSERT_EQ("-0:55.003ms", s);

  ts = TimeSpan::sec(-55) + TimeSpan::us(-3);
  s = ts.ToString();
  ASSERT_EQ("-0:55.000003us", s);

  ts = TimeSpan::sec(-6) + TimeSpan::ms(-3);
  s = ts.ToString();
  ASSERT_EQ("-0:06.003ms", s);

  ts = TimeSpan::us(-3)+ TimeSpan::ns(-1);
  s = ts.ToString();
  ASSERT_EQ("-3001ns", s);

  ts = TimeSpan::us(-3);
  s = ts.ToString();
  ASSERT_EQ("-3us", s);

  ts = TimeSpan::ns(-3);
  s = ts.ToString();
  ASSERT_EQ("-3ns", s);

  ts = TimeSpan::ns(0);
  s = ts.ToString();
  ASSERT_EQ("0ns", s);
}
TEST(gpcc_time_Timespan_Tests, ToString_prec_Structure)
{
  TimeSpan ts;
  std::string s;

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:00.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("00:00:00.000000000ns", s);

  ts = TimeSpan::days(1);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("1.00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("1.00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("1.00:00:00.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("1.00:00:00.000000000ns", s);
}
TEST(gpcc_time_Timespan_Tests, ToString_prec_posValues)
{
  TimeSpan ts;
  std::string s;

  ts = TimeSpan::days(130) + TimeSpan::min(3) + TimeSpan::sec(55) + TimeSpan::ms(12) + TimeSpan::ns(133);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("130.00:03:55", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("130.00:03:55.012ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("130.00:03:55.012000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("130.00:03:55.012000133ns", s);

  ts = TimeSpan::ns(133);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:00.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("00:00:00.000000133ns", s);

  ts = TimeSpan::ns(999);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:00.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("00:00:00.000000999ns", s);

  ts = TimeSpan::ns(1000);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:00.000001us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("00:00:00.000001000ns", s);

  ts = TimeSpan::ms(1000);
  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:01", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:01.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:01.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("00:00:01.000000000ns", s);
}
TEST(gpcc_time_Timespan_Tests, ToString_prec_negValues)
{
  TimeSpan ts;
  std::string s;

  ts = TimeSpan::days(-130) + TimeSpan::min(-3) + TimeSpan::sec(-55) + TimeSpan::ms(-12) + TimeSpan::ns(-133);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("-130.00:03:55", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("-130.00:03:55.012ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("-130.00:03:55.012000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("-130.00:03:55.012000133ns", s);

  ts = TimeSpan::ns(-133);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:00.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("-00:00:00.000000133ns", s);

  ts = TimeSpan::ns(-999);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("00:00:00.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("-00:00:00.000000999ns", s);

  ts = TimeSpan::ns(-1000);

  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("00:00:00", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("00:00:00.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("-00:00:00.000001us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("-00:00:00.000001000ns", s);

  ts = TimeSpan::ms(-1000);
  s = ts.ToString(TimeSpan::Precison::sec);
  ASSERT_EQ("-00:00:01", s);

  s = ts.ToString(TimeSpan::Precison::ms);
  ASSERT_EQ("-00:00:01.000ms", s);

  s = ts.ToString(TimeSpan::Precison::us);
  ASSERT_EQ("-00:00:01.000000us", s);

  s = ts.ToString(TimeSpan::Precison::ns);
  ASSERT_EQ("-00:00:01.000000000ns", s);
}

} // time
} // gpcc_tests
