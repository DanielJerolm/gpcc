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


#include "gpcc/src/dsp/filter/IntegralFilter.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace dsp {
namespace filter {

using namespace gpcc::dsp::filter;
using namespace testing;

TEST(gpcc_dsp_filter_IntegralFilter_Tests, BasicOperation)
{
  // <class T, T MAX, T LOWERTHR, T UPPERTHR>
  IntegralFilter<uint8_t, 100, 20, 80> uut;

  for (uint_fast8_t n = 0; n < 2; n++)
  {
    for (uint_fast8_t i = 1; i <= 80; i++)
    {
      ASSERT_FALSE(uut.Sample(true));
    }

    for (uint_fast8_t i = 1; i <= 40; i++)
    {
      ASSERT_TRUE(uut.Sample(true));
    }

    for (uint_fast8_t i = 1; i <= 80; i++)
    {
      ASSERT_TRUE(uut.Sample(false));
    }

    for (uint_fast8_t i = 1; i <= 40; i++)
    {
      ASSERT_FALSE(uut.Sample(false));
    }
  }

  for (uint_fast8_t i = 1; i <= 80; i++)
  {
    ASSERT_FALSE(uut.Sample(true));
  }

  for (uint_fast8_t n = 0; n < 2; n++)
  {
    ASSERT_TRUE(uut.Sample(true));

    for (uint_fast8_t i = 1; i <= 61; i++)
    {
      ASSERT_TRUE(uut.Sample(false));
    }

    ASSERT_FALSE(uut.Sample(false));

    for (uint_fast8_t i = 1; i <= 61; i++)
    {
      ASSERT_FALSE(uut.Sample(true));
    }
  }
}
TEST(gpcc_dsp_filter_IntegralFilter_Tests, Clear)
{
  // <class T, T MAX, T LOWERTHR, T UPPERTHR>
  IntegralFilter<uint8_t, 100, 20, 80> uut;

  for (uint_fast8_t i = 1; i <= 80; i++)
  {
    ASSERT_FALSE(uut.Sample(true));
  }

  for (uint_fast8_t n = 0; n < 2; n++)
  {
    uut.Clear();

    for (uint_fast8_t i = 1; i <= 80; i++)
    {
      ASSERT_FALSE(uut.Sample(true));
    }

    for (uint_fast8_t i = 1; i <= 40; i++)
    {
      ASSERT_TRUE(uut.Sample(true));
    }
  }
}
TEST(gpcc_dsp_filter_IntegralFilter_Tests, Minimal_MAX)
{
  // <class T, T MAX, T LOWERTHR, T UPPERTHR>
  IntegralFilter<uint8_t, 1, 1, 0> uut;

  ASSERT_TRUE(uut.Sample(true));
  ASSERT_TRUE(uut.Sample(true));
  ASSERT_FALSE(uut.Sample(false));
  ASSERT_FALSE(uut.Sample(false));
  ASSERT_TRUE(uut.Sample(true));
}

} // namespace filter
} // namespace dsp
} // namespace gpcc_tests
