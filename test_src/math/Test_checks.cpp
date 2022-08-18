/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/math/checks.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace math {

using namespace testing;
using namespace gpcc::math;

TEST(gpcc_math_checks_Tests, IsPowerOf2)
{
  ASSERT_TRUE(IsPowerOf2(static_cast<uint32_t>(0)));
  for (int i = 0; i < 32; i++)
  {
    ASSERT_TRUE(IsPowerOf2(static_cast<uint32_t>(1U << i)));
  }

  ASSERT_FALSE(IsPowerOf2(static_cast<uint32_t>(12U)));
  ASSERT_FALSE(IsPowerOf2(static_cast<uint32_t>(3825U)));
  ASSERT_FALSE(IsPowerOf2(static_cast<uint32_t>(255U)));
  ASSERT_FALSE(IsPowerOf2(static_cast<uint32_t>(257U)));
  ASSERT_FALSE(IsPowerOf2(static_cast<uint32_t>(928371983UL)));
}

} // namespace math
} // namespace gpcc_tests
