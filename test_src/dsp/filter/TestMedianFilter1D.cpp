/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019 Daniel Jerolm

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


#include "gpcc/src/dsp/filter/MedianFilter1D.hpp"
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
