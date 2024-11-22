/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#include <gpcc/math/checks.hpp>
#include <gtest/gtest.h>

namespace gpcc_tests {
namespace math {

using namespace testing;
using namespace gpcc::math;

TEST(gpcc_math_checks_Tests, IsPowerOf2)
{
  EXPECT_FALSE(IsPowerOf2(static_cast<uint32_t>(0)));

  for (int i = 0; i < 32; i++)
  {
    EXPECT_TRUE(IsPowerOf2(static_cast<uint32_t>(1U << i)));
  }

  EXPECT_FALSE(IsPowerOf2(static_cast<uint32_t>(12U)));
  EXPECT_FALSE(IsPowerOf2(static_cast<uint32_t>(3825U)));
  EXPECT_FALSE(IsPowerOf2(static_cast<uint32_t>(255U)));
  EXPECT_FALSE(IsPowerOf2(static_cast<uint32_t>(257U)));
  EXPECT_FALSE(IsPowerOf2(static_cast<uint32_t>(928371983UL)));
}

} // namespace math
} // namespace gpcc_tests
