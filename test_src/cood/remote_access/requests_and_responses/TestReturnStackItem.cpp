/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::ReturnStackItem;

TEST(gpcc_cood_ReturnStackItem_Tests, CTOR)
{
  ReturnStackItem uut(1U, 2U);

  EXPECT_EQ(uut.GetID(), 1U);
  EXPECT_EQ(uut.GetInfo(), 2U);
}

TEST(gpcc_cood_ReturnStackItem_Tests, Copy_CTOR)
{
  ReturnStackItem uut1(1U, 2U);
  ReturnStackItem uut2(uut1);

  EXPECT_EQ(uut1.GetID(), 1U);
  EXPECT_EQ(uut1.GetInfo(), 2U);

  EXPECT_EQ(uut2.GetID(), 1U);
  EXPECT_EQ(uut2.GetInfo(), 2U);
}

TEST(gpcc_cood_ReturnStackItem_Tests, Move_CTOR)
{
  ReturnStackItem uut1(1U, 2U);
  ReturnStackItem uut2(std::move(uut1));

  EXPECT_EQ(uut2.GetID(), 1U);
  EXPECT_EQ(uut2.GetInfo(), 2U);
}

TEST(gpcc_cood_ReturnStackItem_Tests, Copy_Assignment)
{
  ReturnStackItem uut1(1U, 2U);
  ReturnStackItem uut2(3U, 4U);

  uut2 = uut1;

  EXPECT_EQ(uut1.GetID(), 1U);
  EXPECT_EQ(uut1.GetInfo(), 2U);

  EXPECT_EQ(uut2.GetID(), 1U);
  EXPECT_EQ(uut2.GetInfo(), 2U);
}

TEST(gpcc_cood_ReturnStackItem_Tests, Move_Assignment)
{
  ReturnStackItem uut1(1U, 2U);
  ReturnStackItem uut2(3U, 4U);

  uut2 = std::move(uut1);

  EXPECT_EQ(uut2.GetID(), 1U);
  EXPECT_EQ(uut2.GetInfo(), 2U);
}

TEST(gpcc_cood_ReturnStackItem_Tests, CompareEqual)
{
  ReturnStackItem uut1(1U, 2U);
  ReturnStackItem uut2(1U, 2U);
  ReturnStackItem uut3(2U, 2U);
  ReturnStackItem uut4(1U, 1U);
  ReturnStackItem uut5(3U, 4U);

  EXPECT_TRUE(uut1 == uut2);
  EXPECT_FALSE(uut1 == uut3);
  EXPECT_FALSE(uut1 == uut4);
  EXPECT_FALSE(uut1 == uut5);

  EXPECT_TRUE(uut1 == uut1);
}

TEST(gpcc_cood_ReturnStackItem_Tests, CompareNotEqual)
{
  ReturnStackItem uut1(1U, 2U);
  ReturnStackItem uut2(1U, 2U);
  ReturnStackItem uut3(2U, 2U);
  ReturnStackItem uut4(1U, 1U);
  ReturnStackItem uut5(3U, 4U);

  EXPECT_FALSE(uut1 != uut2);
  EXPECT_TRUE(uut1 != uut3);
  EXPECT_TRUE(uut1 != uut4);
  EXPECT_TRUE(uut1 != uut5);

  EXPECT_FALSE(uut1 != uut1);
}

TEST(gpcc_cood_ReturnStackItem_Tests, SerializeAndDeserialize)
{
  ReturnStackItem uut(1U, 2U);

  // serialize
  uint8_t storage[64U];
  gpcc::Stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::Stream::IStreamWriter::Endian::Little);

  uut.ToBinary(msw);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - ReturnStackItem::binarySize) << "Unexpected number of bytes written";
  msw.Close();

  // check: uut should have not changed
  EXPECT_EQ(uut.GetID(), 1U);
  EXPECT_EQ(uut.GetInfo(), 2U);

  // deserialize
  gpcc::Stream::MemStreamReader msr(storage, ReturnStackItem::binarySize, gpcc::Stream::IStreamReader::Endian::Little);

  ReturnStackItem uut2(msr);

  EXPECT_EQ(uut2.GetID(), 1U);
  EXPECT_EQ(uut2.GetInfo(), 2U);
}

} // namespace gpcc_tests
} // namespace cood
