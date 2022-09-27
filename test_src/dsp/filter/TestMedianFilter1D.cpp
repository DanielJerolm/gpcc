/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/dsp/filter/MedianFilter1D.hpp>
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace dsp        {
namespace filter     {

using namespace gpcc::dsp::filter;
using namespace testing;

TEST(gpcc_dsp_filter_MedianFilter1D_Tests, BasicOperation)
{
  // <class T, uint_fast8_t WINDOWSIZE>
  MedianFilter1D<uint8_t, 3> uut;

  uint8_t const input[]          = { 128U, 130U, 129U, 130U, 200U, 110U, 0U,   200U, 120U, 115U };
  uint8_t const expectedOutput[] = { 128U, 128U, 129U, 130U, 130U, 130U, 110U, 110U, 120U, 120U };

  for (size_t i = 0; i < sizeof(input); i++)
  {
    uint8_t const output = uut.Sample(input[i]);
    ASSERT_EQ(expectedOutput[i], output);
  }
}

TEST(gpcc_dsp_filter_MedianFilter1D_Tests, Clear)
{
  // <class T, uint_fast8_t WINDOWSIZE>
  MedianFilter1D<uint8_t, 3> uut;

  uint8_t const input[]          = { 128U, 130U, 129U, 130U, 200U, 110U, 0U,   200U, 120U, 115U };
  uint8_t const expectedOutput[] = { 128U, 128U, 129U, 130U, 200U, 200U, 110U, 110U, 120U, 120U };
  //                                                        ^ Clear()

  for (size_t i = 0; i < sizeof(input); i++)
  {
    if (i == 4U)
      uut.Clear();

    uint8_t const output = uut.Sample(input[i]);
    ASSERT_EQ(expectedOutput[i], output);
  }
}

} // namespace filter
} // namespace dsp
} // namespace gpcc_tests
