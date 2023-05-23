/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/math/int_math.hpp>
#include <gtest/gtest.h>
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
