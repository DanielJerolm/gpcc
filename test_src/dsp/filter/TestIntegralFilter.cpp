/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/dsp/filter/IntegralFilter.hpp>
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
