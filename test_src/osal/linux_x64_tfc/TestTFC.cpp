/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#include <gpcc/osal/Thread.hpp>
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace osal {

using namespace testing;
using namespace gpcc::time;
using namespace gpcc::osal;

TEST(gpcc_osal_tfc, PreciseSleep)
{
  TimePoint tp_start = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(100);
  TimePoint tp_100 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(200);
  TimePoint tp_300 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(500);
  TimePoint tp_800 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  Thread::Sleep_ms(1000);
  TimePoint tp_1800 = TimePoint::FromSystemClock(Clocks::monotonicPrecise);

  EXPECT_EQ( 100000000LL, ( tp_100 - tp_start).ns());
  EXPECT_EQ( 300000000LL, ( tp_300 - tp_start).ns());
  EXPECT_EQ( 800000000LL, ( tp_800 - tp_start).ns());
  EXPECT_EQ(1800000000LL, (tp_1800 - tp_start).ns());
}

} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_X64_TFC
