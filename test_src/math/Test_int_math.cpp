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

#include "gpcc/src/math/int_math.hpp"
#include "gtest/gtest.h"
#include <limits>

namespace gpcc_tests {
namespace math {

using namespace testing;
using namespace gpcc::math;

TEST(gpcc_math_int_math_Tests, Sqrt32_MinMax)
{
  ASSERT_EQ(0, sqrt32(0));
  ASSERT_EQ(65535, sqrt32(std::numeric_limits<uint32_t>::max()));
}
TEST(gpcc_math_int_math_Tests, Sqrt32_SomeNumbers)
{
  ASSERT_EQ(1U, sqrt32(1U));
  ASSERT_EQ(1U, sqrt32(2U));
  ASSERT_EQ(1U, sqrt32(3U));
  ASSERT_EQ(2U, sqrt32(4U));
  ASSERT_EQ(2U, sqrt32(5U));
  ASSERT_EQ(2U, sqrt32(6U));
  ASSERT_EQ(2U, sqrt32(7U));
  ASSERT_EQ(2U, sqrt32(8U));
  ASSERT_EQ(3U, sqrt32(9U));
  ASSERT_EQ(65535UL, sqrt32(4294836225UL));
}

} // namespace math
} // namespace gpcc_tests
