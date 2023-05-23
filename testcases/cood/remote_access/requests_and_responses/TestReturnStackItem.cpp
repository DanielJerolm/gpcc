/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/stream/MemStreamReader.hpp>
#include <gpcc/stream/MemStreamWriter.hpp>
#include <gpcc/string/tools.hpp>
#include <gtest/gtest.h>

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
  gpcc::stream::MemStreamWriter msw(storage, sizeof(storage), gpcc::stream::IStreamWriter::Endian::Little);

  uut.ToBinary(msw);
  ASSERT_EQ(msw.RemainingCapacity(), sizeof(storage) - ReturnStackItem::binarySize) << "Unexpected number of bytes written";
  msw.Close();

  // check: uut should have not changed
  EXPECT_EQ(uut.GetID(), 1U);
  EXPECT_EQ(uut.GetInfo(), 2U);

  // deserialize
  gpcc::stream::MemStreamReader msr(storage, ReturnStackItem::binarySize, gpcc::stream::IStreamReader::Endian::Little);

  ReturnStackItem uut2(msr);

  EXPECT_EQ(uut2.GetID(), 1U);
  EXPECT_EQ(uut2.GetInfo(), 2U);
}

} // namespace gpcc_tests
} // namespace cood
