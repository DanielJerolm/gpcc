/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019 Daniel Jerolm

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

#include "gpcc/src/cood/cli/string_conversion.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;
using namespace testing;

TEST(gpcc_cood_cli_string_conversion, StringToObjIndex)
{
  // valid input
  EXPECT_EQ(0x0U,    StringToObjIndex("0x0"));
  EXPECT_EQ(0x0U,    StringToObjIndex("0x0000"));
  EXPECT_EQ(0x1U,    StringToObjIndex("0x1"));
  EXPECT_EQ(0x1200U, StringToObjIndex("0x1200"));
  EXPECT_EQ(0xFFFFU, StringToObjIndex("0xFFFF"));
  EXPECT_EQ(0xFFFFU, StringToObjIndex("0xffff"));

  // invalid input
  EXPECT_THROW((void)StringToObjIndex(""), std::exception);
  EXPECT_THROW((void)StringToObjIndex(".0"), std::exception);
  EXPECT_THROW((void)StringToObjIndex("0."), std::exception);
  EXPECT_THROW((void)StringToObjIndex("0"), std::exception);
  EXPECT_THROW((void)StringToObjIndex("0x0x0"), std::exception);
  EXPECT_THROW((void)StringToObjIndex("0xG212"), std::exception);
}

TEST(gpcc_cood_cli_string_conversion, StringToObjIndexAndSubindex)
{
  uint16_t idx;
  uint8_t subIdx;

  // valid input
  StringToObjIndexAndSubindex("0x0:0", idx, subIdx);
  EXPECT_EQ(idx,    0U);
  EXPECT_EQ(subIdx, 0U);

  StringToObjIndexAndSubindex("0x0:00", idx, subIdx);
  EXPECT_EQ(idx,    0U);
  EXPECT_EQ(subIdx, 0U);

  StringToObjIndexAndSubindex("0x0:000", idx, subIdx);
  EXPECT_EQ(idx,    0U);
  EXPECT_EQ(subIdx, 0U);

  StringToObjIndexAndSubindex("0x0000:000", idx, subIdx);
  EXPECT_EQ(idx,    0U);
  EXPECT_EQ(subIdx, 0U);

  StringToObjIndexAndSubindex("0x10:13", idx, subIdx);
  EXPECT_EQ(idx,    0x10U);
  EXPECT_EQ(subIdx, 13U);

  StringToObjIndexAndSubindex("0x1000:13", idx, subIdx);
  EXPECT_EQ(idx,    0x1000U);
  EXPECT_EQ(subIdx, 13U);

  StringToObjIndexAndSubindex("0xFFFF:255", idx, subIdx);
  EXPECT_EQ(idx,    0xFFFFU);
  EXPECT_EQ(subIdx, 255U);

  StringToObjIndexAndSubindex("0xffff:255", idx, subIdx);
  EXPECT_EQ(idx,    0xFFFFU);
  EXPECT_EQ(subIdx, 255U);

  // invalid input
  EXPECT_THROW(StringToObjIndexAndSubindex("", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex(":", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x0000", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x0000:", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x0000 0", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x000G:0", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x0000:f2", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x000G:0x0", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex("0x000:0:0", idx, subIdx), std::exception);
  EXPECT_THROW(StringToObjIndexAndSubindex(":0", idx, subIdx), std::exception);
}

} // namespace cood
} // namespace gpcc_tests
