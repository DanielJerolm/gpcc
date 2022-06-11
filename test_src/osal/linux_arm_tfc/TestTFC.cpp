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

#ifdef OS_LINUX_ARM_TFC

#include "gpcc/src/osal/Thread.hpp"
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

#endif // #ifdef OS_LINUX_ARM_TFC
