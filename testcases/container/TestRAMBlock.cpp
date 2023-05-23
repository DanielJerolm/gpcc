/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/container/RAMBlock.hpp>
#include <gpcc/stream/MemStreamReader.hpp>
#include <gpcc/stream/MemStreamWriter.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>

namespace gpcc_tests {
namespace container  {

using namespace gpcc::container;
using namespace testing;

TEST(gpcc_container_RAMBlock_Tests, Construction_Zero)
{
  std::unique_ptr<RAMBlock> spUUT;

  auto Check = [&](size_t s)
    {
      EXPECT_FALSE(spUUT->IsDirty());
      EXPECT_EQ(s, spUUT->GetSize());
      EXPECT_EQ(0U, spUUT->GetPageSize());

      if (s != 0)
      {
        std::vector<uint8_t> readBackData(s);
        spUUT->Read(0, s, readBackData.data());

        for (auto const v: readBackData)
        {
          if (v != 0)
          {
            ADD_FAILURE() << "Failed at size " << s;
            break;
          }
        }
      }
    };

  spUUT.reset(new RAMBlock(0));
  Check(0);

  spUUT.reset(new RAMBlock(1));
  Check(1);

  spUUT.reset(new RAMBlock(10));
  Check(10);

  spUUT.reset(new RAMBlock(128));
  Check(128);
}

TEST(gpcc_container_RAMBlock_Tests, Construction_NonZero)
{
  std::unique_ptr<RAMBlock> spUUT;

  auto Check = [&](size_t s, uint8_t value)
    {
      EXPECT_FALSE(spUUT->IsDirty());
      EXPECT_EQ(s, spUUT->GetSize());
      EXPECT_EQ(0U, spUUT->GetPageSize());

      if (s != 0)
      {
        std::vector<uint8_t> readBackData(s);
        spUUT->Read(0, s, readBackData.data());

        for (auto const v: readBackData)
        {
          if (v != value)
          {
            ADD_FAILURE() << "Failed at size " << s;
            break;
          }
        }
      }
    };

  spUUT.reset(new RAMBlock(0, 3));
  Check(0, 3);

  spUUT.reset(new RAMBlock(1, 4));
  Check(1, 4);

  spUUT.reset(new RAMBlock(10, 0));
  Check(10, 0);

  spUUT.reset(new RAMBlock(128, 5));
  Check(128, 5);
}

TEST(gpcc_container_RAMBlock_Tests, Construction_FromIStreamReader)
{
  std::unique_ptr<RAMBlock> spUUT;

  std::vector<uint8_t> data;
  data.reserve(64);
  for (uint_fast8_t i = 0; i < 64; i++)
    data.push_back(i);

  auto Check = [&](size_t s)
    {
      EXPECT_FALSE(spUUT->IsDirty());
      EXPECT_EQ(s, spUUT->GetSize());
      EXPECT_EQ(0U, spUUT->GetPageSize());

      if (s != 0)
      {
        std::vector<uint8_t> readBackData(s);
        spUUT->Read(0, s, readBackData.data());

        for (size_t i = 0; i < s; i++)
        {
          if (readBackData[i] != data[i])
          {
            ADD_FAILURE() << "Failed at size " << s;
          }
        }
      }
    };

  gpcc::stream::MemStreamReader msr(data.data(), data.size(), gpcc::stream::MemStreamReader::Endian::Little);

  spUUT.reset(new RAMBlock(0, msr));
  Check(0);
  EXPECT_EQ(64U, msr.RemainingBytes());

  msr = gpcc::stream::MemStreamReader(data.data(), data.size(), gpcc::stream::MemStreamReader::Endian::Little);
  spUUT.reset(new RAMBlock(1, msr));
  Check(1);
  EXPECT_EQ(64U - 1U, msr.RemainingBytes());

  msr = gpcc::stream::MemStreamReader(data.data(), data.size(), gpcc::stream::MemStreamReader::Endian::Little);
  spUUT.reset(new RAMBlock(10, msr));
  Check(10);
  EXPECT_EQ(64U - 10U, msr.RemainingBytes());

  msr = gpcc::stream::MemStreamReader(data.data(), data.size(), gpcc::stream::MemStreamReader::Endian::Little);
  spUUT.reset(new RAMBlock(64, msr));
  Check(64);
  EXPECT_EQ(64U - 64U, msr.RemainingBytes());
}

TEST(gpcc_container_RAMBlock_Tests, Construction_FromIStreamReader_Fail)
{
  std::unique_ptr<RAMBlock> spUUT;

  std::vector<uint8_t> data;
  data.reserve(64);
  for (uint_fast8_t i = 0; i < 64; i++)
    data.push_back(i);

  gpcc::stream::MemStreamReader msr(data.data(), data.size(), gpcc::stream::MemStreamReader::Endian::Little);

  ASSERT_THROW(spUUT.reset(new RAMBlock(65, msr)), std::exception);
  EXPECT_EQ(gpcc::stream::MemStreamReader::States::error, msr.GetState());
}

TEST(gpcc_container_RAMBlock_Tests, Construction_CopyFromVector_ZeroSize)
{
  std::vector<uint8_t> data;

  RAMBlock uut(data);

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(0U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, Construction_CopyFromVector_OK)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uint8_t buffer[3];
  uut.Read(0, 3, buffer);

  EXPECT_EQ(buffer[0], data[0]);
  EXPECT_EQ(buffer[1], data[1]);
  EXPECT_EQ(buffer[2], data[2]);
}

TEST(gpcc_container_RAMBlock_Tests, Construction_MoveFromVector_ZeroSize)
{
  std::vector<uint8_t> data;

  RAMBlock uut(std::move(data));

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(0U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, Construction_MoveFromVector_OK)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(std::move(data));

  EXPECT_EQ(0U, data.size());

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uint8_t buffer[3];
  uut.Read(0, 3, buffer);

  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, CopyConstructor_ZeroSize)
{
  RAMBlock uut(0);
  RAMBlock uut2(uut);

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(0U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(0U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, CopyConstructor_NonZeroSize_NotDirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  RAMBlock uut2(uut);

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uint8_t buffer[3];

  uut.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, CopyConstructor_NonZeroSize_Dirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  uut.SetDirtyFlag();

  RAMBlock uut2(uut);

  EXPECT_TRUE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  EXPECT_TRUE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uint8_t buffer[3];

  uut.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, MoveConstructor_ZeroSize)
{
  RAMBlock uut(0);
  RAMBlock uut2(std::move(uut));

  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(0U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, MoveConstructor_NonZeroSize_NotDirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  RAMBlock uut2(std::move(uut));

  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uint8_t buffer[3];

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, MoveConstructor_NonZeroSize_Dirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  uut.SetDirtyFlag();

  RAMBlock uut2(std::move(uut));

  EXPECT_TRUE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uint8_t buffer[3];

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, CopyAssignment_ZeroSize)
{
  RAMBlock uut(0);
  RAMBlock uut2(5);

  // before...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(5U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  // copy...
  uut2 = uut;

  // after...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(0U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(0U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, CopyAssignment_NonZeroSize_NotDirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  RAMBlock uut2(2);

  uint8_t buffer[3];

  // before...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(2U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 2, buffer);
  EXPECT_EQ(buffer[0], 0);
  EXPECT_EQ(buffer[1], 0);

  // copy...
  uut2 = uut;

  // after...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uut.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);

  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, CopyAssignment_NonZeroSize_Dirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  uut.SetDirtyFlag();

  RAMBlock uut2(2);

  uint8_t buffer[3];

  // before...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(2U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 2, buffer);
  EXPECT_EQ(buffer[0], 0);
  EXPECT_EQ(buffer[1], 0);

  // copy...
  uut2 = uut;

  // after...
  EXPECT_TRUE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uut.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);

  EXPECT_TRUE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, MoveAssignment_ZeroSize)
{
  RAMBlock uut(0);
  RAMBlock uut2(5);

  // before...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(5U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  // move...
  uut2 = std::move(uut);

  // after...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(0U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, MoveAssignment_NonZeroSize_NotDirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  RAMBlock uut2(2);

  uint8_t buffer[3];

  // before...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(2U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 2, buffer);
  EXPECT_EQ(buffer[0], 0);
  EXPECT_EQ(buffer[1], 0);

  // move...
  uut2 = std::move(uut);

  // after...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, MoveAssignment_NonZeroSize_Dirty)
{
  std::vector<uint8_t> data = {1, 5, 8};

  RAMBlock uut(data);
  uut.SetDirtyFlag();

  RAMBlock uut2(2);

  uint8_t buffer[3];

  // before...
  EXPECT_FALSE(uut2.IsDirty());
  EXPECT_EQ(2U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 2, buffer);
  EXPECT_EQ(buffer[0], 0);
  EXPECT_EQ(buffer[1], 0);

  // move...
  uut2 = std::move(uut);

  // after...
  EXPECT_TRUE(uut2.IsDirty());
  EXPECT_EQ(3U, uut2.GetSize());
  EXPECT_EQ(0U, uut2.GetPageSize());

  uut2.Read(0, 3, buffer);
  EXPECT_EQ(buffer[0], 1);
  EXPECT_EQ(buffer[1], 5);
  EXPECT_EQ(buffer[2], 8);
}

TEST(gpcc_container_RAMBlock_Tests, CopyAssignment_Vector_ZeroSize)
{
  RAMBlock uut(5);

  // before...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(5U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  std::vector<uint8_t> data;

  // copy-assign
  uut = data;

  // after...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(0U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, CopyAssignment_Vector_NonZeroSize_NotDirty)
{
  RAMBlock uut(5);

  // before...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(5U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  std::vector<uint8_t> data = {1U, 77U};

  // copy-assign...
  uut = data;

  // after...
  ASSERT_EQ(2U, data.size());
  EXPECT_EQ(1U, data[0]);
  EXPECT_EQ(77U, data[1]);

  EXPECT_FALSE(uut.IsDirty());
  ASSERT_EQ(2U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uint8_t buffer[2];
  uut.Read(0, 2, buffer);

  EXPECT_EQ(1U, buffer[0]);
  EXPECT_EQ(77U, buffer[1]);
}

TEST(gpcc_container_RAMBlock_Tests, CopyAssignment_Vector_NonZeroSize_Dirty)
{
  RAMBlock uut(5);
  uut.SetDirtyFlag();

  // before...
  EXPECT_TRUE(uut.IsDirty());
  EXPECT_EQ(5U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  std::vector<uint8_t> data = {1U, 77U};

  // copy-assign...
  uut = data;

  // after...
  ASSERT_EQ(2U, data.size());
  EXPECT_EQ(1U, data[0]);
  EXPECT_EQ(77U, data[1]);

  EXPECT_FALSE(uut.IsDirty());
  ASSERT_EQ(2U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uint8_t buffer[2];
  uut.Read(0, 2, buffer);

  EXPECT_EQ(1U, buffer[0]);
  EXPECT_EQ(77U, buffer[1]);
}

TEST(gpcc_container_RAMBlock_Tests, MoveAssignment_Vector_ZeroSize)
{
  RAMBlock uut(5);

  // before...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(5U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  std::vector<uint8_t> data;

  // move-assign
  uut = std::move(data);

  // after...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(0U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());
}

TEST(gpcc_container_RAMBlock_Tests, MoveAssignment_Vector_NonZeroSize_NotDirty)
{
  RAMBlock uut(5);

  // before...
  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(5U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  std::vector<uint8_t> data = {1U, 77U};

  // move-assign...
  uut = std::move(data);

  // after...
  EXPECT_FALSE(uut.IsDirty());
  ASSERT_EQ(2U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uint8_t buffer[2];
  uut.Read(0, 2, buffer);

  EXPECT_EQ(1U, buffer[0]);
  EXPECT_EQ(77U, buffer[1]);
}

TEST(gpcc_container_RAMBlock_Tests, MoveAssignment_Vector_NonZeroSize_Dirty)
{
  RAMBlock uut(5);
  uut.SetDirtyFlag();

  // before...
  EXPECT_TRUE(uut.IsDirty());
  EXPECT_EQ(5U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  std::vector<uint8_t> data = {1U, 77U};

  // move-assign...
  uut = std::move(data);

  // after...
  EXPECT_FALSE(uut.IsDirty());
  ASSERT_EQ(2U, uut.GetSize());
  EXPECT_EQ(0U, uut.GetPageSize());

  uint8_t buffer[2];
  uut.Read(0, 2, buffer);

  EXPECT_EQ(1U, buffer[0]);
  EXPECT_EQ(77U, buffer[1]);
}

TEST(gpcc_container_RAMBlock_Tests, Is_Set_Clear_DirtyFlag)
{
  RAMBlock uut(5);

  EXPECT_FALSE(uut.IsDirty());
  uut.ClearDirtyFlag();
  EXPECT_FALSE(uut.IsDirty());

  uut.SetDirtyFlag();
  EXPECT_TRUE(uut.IsDirty());
  uut.SetDirtyFlag();
  EXPECT_TRUE(uut.IsDirty());

  uut.ClearDirtyFlag();
  EXPECT_FALSE(uut.IsDirty());
}

TEST(gpcc_container_RAMBlock_Tests, GetDataAndClearDirtyFlag_ZeroSize)
{
  RAMBlock uut(0);

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(0U, v.size());
  EXPECT_FALSE(uut.IsDirty());

  uut.SetDirtyFlag();

  v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(0U, v.size());
  EXPECT_FALSE(uut.IsDirty());
}

TEST(gpcc_container_RAMBlock_Tests, GetDataAndClearDirtyFlag_NonZeroSize)
{
  std::vector<uint8_t> data = {1U, 7U, 3U};
  RAMBlock uut(data);

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_TRUE(v == data);

  EXPECT_FALSE(uut.IsDirty());
  EXPECT_EQ(3U, uut.GetSize());

  uut.SetDirtyFlag();

  v = uut.GetDataAndClearDirtyFlag();

  EXPECT_TRUE(v == data);
  EXPECT_FALSE(uut.IsDirty());
}

TEST(gpcc_container_RAMBlock_Tests, WriteToStreamAndClearDirtyFlag_ZeroSize)
{
  uint8_t buffer[32];
  gpcc::stream::MemStreamWriter msw(buffer, sizeof(buffer), gpcc::stream::IStreamWriter::Endian::Little);

  RAMBlock uut(0);

  uut.WriteToStreamAndClearDirtyFlag(msw);

  EXPECT_EQ(32U, msw.RemainingCapacity());
  EXPECT_FALSE(uut.IsDirty());

  uut.SetDirtyFlag();

  uut.WriteToStreamAndClearDirtyFlag(msw);

  EXPECT_EQ(32U, msw.RemainingCapacity());
  EXPECT_FALSE(uut.IsDirty());
}

TEST(gpcc_container_RAMBlock_Tests, WriteToStreamAndClearDirtyFlag_NonZeroSize)
{
  uint8_t buffer[32];
  gpcc::stream::MemStreamWriter msw(buffer, sizeof(buffer), gpcc::stream::IStreamWriter::Endian::Little);

  std::vector<uint8_t> data = {1U, 55U, 9U};
  RAMBlock uut(data);

  uut.WriteToStreamAndClearDirtyFlag(msw);

  EXPECT_EQ(29U, msw.RemainingCapacity());
  EXPECT_FALSE(uut.IsDirty());

  uut.SetDirtyFlag();

  uut.WriteToStreamAndClearDirtyFlag(msw);

  EXPECT_EQ(26U, msw.RemainingCapacity());
  EXPECT_FALSE(uut.IsDirty());

  EXPECT_EQ(1U, buffer[0]);
  EXPECT_EQ(55U, buffer[1]);
  EXPECT_EQ(9U, buffer[2]);
  EXPECT_EQ(1U, buffer[3]);
  EXPECT_EQ(55U, buffer[4]);
  EXPECT_EQ(9U, buffer[5]);
}

TEST(gpcc_container_RAMBlock_Tests, Read_OK)
{
  std::vector<uint8_t> data = {23U, 1U, 22U, 78U, 9U, 45U};
  uint8_t buffer[32];

  auto checkFF = [&buffer](size_t start)
    {
      for (size_t i = start; i < sizeof(buffer); ++i)
      {
        if (buffer[i] != 0xFFU)
        {
          ADD_FAILURE();
          break;
        }
      }
    };

  RAMBlock uut(data);

  // read zero bytes
  memset(buffer, 0xFFU, sizeof(buffer));
  uut.Read(0, 0, buffer);
  checkFF(0);

  // read 1 byte at address 0
  memset(buffer, 0xFFU, sizeof(buffer));
  uut.Read(0, 1, buffer);
  EXPECT_EQ(23U, buffer[0]);
  checkFF(1);

  // read 1 byte at end
  memset(buffer, 0xFFU, sizeof(buffer));
  uut.Read(5, 1, buffer);
  EXPECT_EQ(45U, buffer[0]);
  checkFF(1);

  // read 2 bytes at address 0
  memset(buffer, 0xFFU, sizeof(buffer));
  uut.Read(0, 2, buffer);
  EXPECT_EQ(23U, buffer[0]);
  EXPECT_EQ(1U, buffer[1]);
  checkFF(2);

  // read 2 bytes at end-1
  memset(buffer, 0xFFU, sizeof(buffer));
  uut.Read(4, 2, buffer);
  EXPECT_EQ(9U, buffer[0]);
  EXPECT_EQ(45U, buffer[1]);
  checkFF(2);

  // read everything
  memset(buffer, 0xFFU, sizeof(buffer));
  uut.Read(0, 6, buffer);
  EXPECT_EQ(23U, buffer[0]);
  EXPECT_EQ(1U, buffer[1]);
  EXPECT_EQ(22U, buffer[2]);
  EXPECT_EQ(78U, buffer[3]);
  EXPECT_EQ(9U, buffer[4]);
  EXPECT_EQ(45U, buffer[5]);
  checkFF(6);
}

TEST(gpcc_container_RAMBlock_Tests, Read_Bad)
{
  std::vector<uint8_t> data = {23U, 1U, 22U, 78U, 9U, 45U};
  uint8_t buffer[32];

  auto checkFF = [&buffer](size_t start)
    {
      for (size_t i = start; i < sizeof(buffer); ++i)
      {
        if (buffer[i] != 0xFFU)
        {
          ADD_FAILURE();
          break;
        }
      }
    };

  RAMBlock uut(data);

  memset(buffer, 0xFFU, sizeof(buffer));
  EXPECT_THROW(uut.Read(0, 7, buffer), std::exception);
  checkFF(0);

  memset(buffer, 0xFFU, sizeof(buffer));
  EXPECT_THROW(uut.Read(1, 6, buffer), std::exception);
  checkFF(0);

  memset(buffer, 0xFFU, sizeof(buffer));
  EXPECT_THROW(uut.Read(6, 0, buffer), std::exception);
  checkFF(0);

  memset(buffer, 0xFFU, sizeof(buffer));
  EXPECT_THROW(uut.Read(6, 1, buffer), std::exception);
  checkFF(0);
}

TEST(gpcc_container_RAMBlock_Tests, Write_Zero)
{
  uint8_t buffer[32];
  RAMBlock uut(6);

  // write zero bytes
  uut.Write(0, 0, buffer);
  EXPECT_FALSE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  for (auto const e: v)
  {
    if (e != 0)
    {
      ADD_FAILURE();
      break;
    }
  }
}

TEST(gpcc_container_RAMBlock_Tests, Write_OneByte)
{
  uint8_t buffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  uut.Write(0, 1, buffer);
  EXPECT_TRUE(uut.IsDirty());
  uut.Write(5, 1, buffer + 1U);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(0U,   v[1]);
  EXPECT_EQ(0U,   v[2]);
  EXPECT_EQ(0U,   v[3]);
  EXPECT_EQ(0U,   v[4]);
  EXPECT_EQ(67U,  v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, Write_TwoBytes)
{
  uint8_t buffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  buffer[2] = 8U;
  buffer[3] = 9U;
  uut.Write(0, 2, buffer);
  EXPECT_TRUE(uut.IsDirty());
  uut.Write(4, 2, buffer + 2U);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(67U,  v[1]);
  EXPECT_EQ(0U,   v[2]);
  EXPECT_EQ(0U,   v[3]);
  EXPECT_EQ(8U,   v[4]);
  EXPECT_EQ(9U,   v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, Write_All)
{
  uint8_t buffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  buffer[2] = 8U;
  buffer[3] = 9U;
  buffer[4] = 45U;
  buffer[5] = 12U;

  uut.Write(0, 6, buffer);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(67U,  v[1]);
  EXPECT_EQ(8U,   v[2]);
  EXPECT_EQ(9U,   v[3]);
  EXPECT_EQ(45U,  v[4]);
  EXPECT_EQ(12U,  v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, Write_Bad)
{
  std::vector<uint8_t> const data = { 4U, 7U, 9U, 23U, 44U, 28U };

  uint8_t buffer[32];
  memset(buffer, 0 ,sizeof(buffer));

  RAMBlock uut(data);

  auto check = [&]()
    {
      EXPECT_FALSE(uut.IsDirty());
      auto v = uut.GetDataAndClearDirtyFlag();
      EXPECT_TRUE(v == data);
    };

  EXPECT_THROW(uut.Write(0, 7, buffer), std::exception);
  check();

  EXPECT_THROW(uut.Write(1, 6, buffer), std::exception);
  check();

  EXPECT_THROW(uut.Write(6, 0, buffer), std::exception);
  check();

  EXPECT_THROW(uut.Write(6, 1, buffer), std::exception);
  check();
}

TEST(gpcc_container_RAMBlock_Tests, WriteAndCheck_Zero)
{
  uint8_t buffer[32];
  uint8_t auxBuffer[32];
  RAMBlock uut(6);

  // write zero bytes
  uut.WriteAndCheck(0, 0, buffer, auxBuffer);
  EXPECT_FALSE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  for (auto const e: v)
  {
    if (e != 0)
    {
      ADD_FAILURE();
      break;
    }
  }
}

TEST(gpcc_container_RAMBlock_Tests, WriteAndCheck_OneByte)
{
  uint8_t buffer[32];
  uint8_t auxBuffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  uut.WriteAndCheck(0, 1, buffer, auxBuffer);
  EXPECT_TRUE(uut.IsDirty());
  uut.WriteAndCheck(5, 1, buffer + 1U, auxBuffer);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(0U,   v[1]);
  EXPECT_EQ(0U,   v[2]);
  EXPECT_EQ(0U,   v[3]);
  EXPECT_EQ(0U,   v[4]);
  EXPECT_EQ(67U,  v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, WriteAndCheck_OneByte_NoAuxBuffer)
{
  uint8_t buffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  uut.WriteAndCheck(0, 1, buffer, nullptr);
  EXPECT_TRUE(uut.IsDirty());
  uut.WriteAndCheck(5, 1, buffer + 1U, nullptr);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(0U,   v[1]);
  EXPECT_EQ(0U,   v[2]);
  EXPECT_EQ(0U,   v[3]);
  EXPECT_EQ(0U,   v[4]);
  EXPECT_EQ(67U,  v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, WriteAndCheck_TwoBytes)
{
  uint8_t buffer[32];
  uint8_t auxBuffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  buffer[2] = 8U;
  buffer[3] = 9U;
  uut.WriteAndCheck(0, 2, buffer, auxBuffer);
  EXPECT_TRUE(uut.IsDirty());
  uut.WriteAndCheck(4, 2, buffer + 2U, auxBuffer);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(67U,  v[1]);
  EXPECT_EQ(0U,   v[2]);
  EXPECT_EQ(0U,   v[3]);
  EXPECT_EQ(8U,   v[4]);
  EXPECT_EQ(9U,   v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, WriteAndCheck_All)
{
  uint8_t buffer[32];
  uint8_t auxBuffer[32];
  RAMBlock uut(6);

  // write one byte at first and last address
  buffer[0] = 5U;
  buffer[1] = 67U;
  buffer[2] = 8U;
  buffer[3] = 9U;
  buffer[4] = 45U;
  buffer[5] = 12U;

  uut.WriteAndCheck(0, 6, buffer, auxBuffer);
  EXPECT_TRUE(uut.IsDirty());

  auto v = uut.GetDataAndClearDirtyFlag();

  EXPECT_EQ(5U,   v[0]);
  EXPECT_EQ(67U,  v[1]);
  EXPECT_EQ(8U,   v[2]);
  EXPECT_EQ(9U,   v[3]);
  EXPECT_EQ(45U,  v[4]);
  EXPECT_EQ(12U,  v[5]);
}

TEST(gpcc_container_RAMBlock_Tests, WriteAndCheck_Bad)
{
  std::vector<uint8_t> const data = { 4U, 7U, 9U, 23U, 44U, 28U };

  uint8_t buffer[32];
  uint8_t auxBuffer[32];
  memset(buffer, 0 ,sizeof(buffer));

  RAMBlock uut(data);

  auto check = [&]()
    {
      EXPECT_FALSE(uut.IsDirty());
      auto v = uut.GetDataAndClearDirtyFlag();
      EXPECT_TRUE(v == data);
    };

  EXPECT_THROW(uut.WriteAndCheck(0, 7, buffer, auxBuffer), std::exception);
  check();

  EXPECT_THROW(uut.WriteAndCheck(1, 6, buffer, auxBuffer), std::exception);
  check();

  EXPECT_THROW(uut.WriteAndCheck(6, 0, buffer, auxBuffer), std::exception);
  check();

  EXPECT_THROW(uut.WriteAndCheck(6, 1, buffer, auxBuffer), std::exception);
  check();
}

} // namespace container
} // namespace gpcc_tests
