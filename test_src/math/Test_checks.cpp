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
