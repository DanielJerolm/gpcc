/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2020 Daniel Jerolm

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

#include "gtest/gtest.h"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include <memory>
#include <iostream>
#include <cassert>
#include <cstring>

namespace gpcc_tests
{
namespace Stream
{

using namespace gpcc::Stream;

using namespace testing;

/// Test fixture for gpcc::Stream::MemStreamWriter related tests.
class GPCC_Stream_MemStreamWriter_Tests: public Test
{
  public:
    GPCC_Stream_MemStreamWriter_Tests(void);

  protected:
    void SetUp(void) override;
    void TearDown(void) override;

    bool compare_memory(uint8_t const * pExpected, size_t const s);

    uint8_t memory[128];
};
GPCC_Stream_MemStreamWriter_Tests::GPCC_Stream_MemStreamWriter_Tests(void)
: Test()
, memory()
{
}

void GPCC_Stream_MemStreamWriter_Tests::SetUp(void)
{
  // Watermark our memory. This gives us the chance to detect unexpected writes beyond the end of
  // the buffer.
  memset(memory, 0xFF, sizeof(memory));
}
void GPCC_Stream_MemStreamWriter_Tests::TearDown(void)
{
}

bool GPCC_Stream_MemStreamWriter_Tests::compare_memory(uint8_t const * pExpected, size_t const s)
{
  for (size_t i = 0; i < s; i++)
  {
    if (memory[i] != pExpected[i])
    {
      std::cout << "Mismatch at:" << i;
      return false;
    }
  }
  return true;
}

TEST_F(GPCC_Stream_MemStreamWriter_Tests, pMemIsnullptrButSizeIsNotZero)
{
  EXPECT_THROW(MemStreamWriter uut(nullptr, 1, IStreamWriter::Endian::Little), std::invalid_argument);
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, ZeroSize1)
{
  MemStreamWriter uut(memory, 0, IStreamWriter::Endian::Little);
  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());
  uut.Close();
  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, ZeroSize2)
{
  MemStreamWriter uut(nullptr, 0, IStreamWriter::Endian::Little);
  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());
  uut.Close();
  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction)
{
  assert(sizeof(memory) >= 5);
  MemStreamWriter uut1(memory, 5, IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  MemStreamWriter uut2(uut1);

  uut1.Write_uint8(0x05);
  uut2.Write_uint8(0x12);

  ASSERT_EQ(IStreamWriter::States::full, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut1.Close();
  uut2.Close();

  uint8_t expected[] = { 0x01, 0x02, 0x03, 0x04, 0x12};

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction_EndianLittle)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  MemStreamWriter uut2(uut1);
  ASSERT_EQ(IStreamWriter::Endian::Little, uut2.GetEndian());

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction_EndianBig)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Big);

  MemStreamWriter uut2(uut1);
  ASSERT_EQ(IStreamWriter::Endian::Big, uut2.GetEndian());

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction_BitPos)
{
  assert(sizeof(memory) >= 5);
  MemStreamWriter uut1(memory, 5, IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_Bit(true);
  uut1.Write_Bit(false);
  uut1.Write_Bit(true);

  MemStreamWriter uut2(uut1);

  uut1.Close();

  uut2.Write_Bit(false);
  uut2.Write_Bit(true);
  uut2.Write_Bit(true);

  uut2.Write_uint8(0x12);
  uut2.Write_uint8(0x13);

  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut2.Close();

  uint8_t expected[] = { 0x01, 0x02, 0x35, 0x12, 0x13};
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction_StateClosed)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);

  uut1.Close();

  MemStreamWriter uut2(uut1);
  ASSERT_EQ(IStreamWriter::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction_StateFull)
{
  assert(sizeof(memory) >= 2);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  ASSERT_EQ(IStreamWriter::States::full, uut1.GetState());

  MemStreamWriter uut2(uut1);
  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyConstruction_StateError)
{
  assert(sizeof(memory) >= 2);
  MemStreamWriter uut1(memory, 0, IStreamWriter::Endian::Little);

  EXPECT_THROW(uut1.Write_uint8(0x01), std::exception);
  ASSERT_EQ(IStreamWriter::States::error, uut1.GetState());

  MemStreamWriter uut2(uut1);
  ASSERT_EQ(IStreamWriter::States::error, uut2.GetState());

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction)
{
  assert(sizeof(memory) >= 5);
  MemStreamWriter uut1(memory, 5, IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());

  uut2.Write_uint8(0x12);
  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut2.Close();

  uint8_t expected[] = { 0x01, 0x02, 0x03, 0x04, 0x12};

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction_EndianLittle)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::Endian::Little, uut2.GetEndian());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction_EndianBig)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Big);

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::Endian::Big, uut2.GetEndian());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction_BitPos)
{
  assert(sizeof(memory) >= 5);
  MemStreamWriter uut1(memory, 5, IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_Bit(true);
  uut1.Write_Bit(false);
  uut1.Write_Bit(true);

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());

  uut2.Write_Bit(false);
  uut2.Write_Bit(true);
  uut2.Write_Bit(true);

  uut2.Write_uint8(0x12);
  uut2.Write_uint8(0x13);

  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut2.Close();

  uint8_t expected[] = { 0x01, 0x02, 0x35, 0x12, 0x13};
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction_StateClosed)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);

  uut1.Close();

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction_StateFull)
{
  assert(sizeof(memory) >= 2);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  ASSERT_EQ(IStreamWriter::States::full, uut1.GetState());

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveConstruction_StateError)
{
  assert(sizeof(memory) >= 2);
  MemStreamWriter uut1(memory, 0, IStreamWriter::Endian::Little);

  EXPECT_THROW(uut1.Write_uint8(0x01), std::exception);
  ASSERT_EQ(IStreamWriter::States::error, uut1.GetState());

  MemStreamWriter uut2(std::move(uut1));
  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::error, uut2.GetState());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_CloseBeforeMove)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut2.Write_uint8(0x55);
  uut2.Write_uint8(0x66);
  uut2.Write_uint8(0x77);
  uut2.Write_Bit(true);
  uut2.Write_Bit(false);
  uut2.Write_Bit(true);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  uut2 = uut1;

  uut1.Close();

  uut2.Write_uint8(0xAB);

  uut2.Close();

  EXPECT_EQ(0x55U, mem2[0]);
  EXPECT_EQ(0x66U, mem2[1]);
  EXPECT_EQ(0x77U, mem2[2]);
  EXPECT_EQ(0x05U, mem2[3]);

  uint8_t expected[] = { 0x01, 0x02, 0x03, 0x04, 0xAB};

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_BitPos)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);
  uut1.Write_Bit(true);
  uut1.Write_Bit(false);
  uut1.Write_Bit(true);

  uut2 = uut1;

  uut1.Close();

  uut2.Write_uint8(0xAB);

  uut2.Close();

  uint8_t expected[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0xAB};
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_Endian)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Big);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut1.GetEndian());
  ASSERT_EQ(IStreamWriter::Endian::Big, uut2.GetEndian());

  uut2 = uut1;

  ASSERT_EQ(IStreamWriter::Endian::Little, uut1.GetEndian());
  ASSERT_EQ(IStreamWriter::Endian::Little, uut2.GetEndian());

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_Closed)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut2.Write_uint8(0xAB);

  uut1.Close();
  uut2 = uut1;

  ASSERT_EQ(IStreamWriter::States::closed, uut2.GetState());

  uint8_t expected[] = { 0xFF };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_Full)
{
  assert(sizeof(memory) > 5);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut1.Write_uint8(0x33);

  uut2.Write_uint8(0xAB);

  uut2 = uut1;
  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut1.Close();
  uut2.Close();

  ASSERT_EQ(0xABU, mem2[0]);

  uint8_t expected[] = { 0xFF, 0x33 };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_Error)
{
  assert(sizeof(memory) > 5);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut1.Write_uint8(0x33);
  ASSERT_THROW(uut1.Write_uint8(0x12), std::exception);

  uut2.Write_uint8(0xAB);

  uut2 = uut1;
  ASSERT_EQ(IStreamWriter::States::error, uut2.GetState());

  uut1.Close();
  uut2.Close();

  ASSERT_EQ(0xABU, mem2[0]);

  uint8_t expected[] = { 0xFF, 0x33 };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_LeaveError)
{
  assert(sizeof(memory) > 5);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut1.Write_uint8(0x33);
  ASSERT_THROW(uut1.Write_uint8(0x12), std::exception);
  ASSERT_EQ(IStreamWriter::States::error, uut1.GetState());

  uut2.Write_uint8(0xAB);

  uut1 = uut2;
  ASSERT_EQ(IStreamWriter::States::open, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::open, uut2.GetState());

  uut1.Close();
  uut2.Close();

  ASSERT_EQ(0xABU, mem2[0]);

  uint8_t expected[] = { 0xFF, 0x33 };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment_Self)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x55);
  uut1.Write_uint8(0x66);
  uut1.Write_uint8(0x77);
  uut1.Write_Bit(true);
  uut1.Write_Bit(false);
  uut1.Write_Bit(true);

  MemStreamWriter & uut2 = uut1;
  uut1 = uut2;

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  uut1.Close();

  uint8_t expected[] = { 0x55, 0x66, 0x77, 0x05, 0x01, 0x02, 0x03, 0x04};

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_CloseBeforeMove)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut2.Write_uint8(0x55);
  uut2.Write_uint8(0x66);
  uut2.Write_uint8(0x77);
  uut2.Write_Bit(true);
  uut2.Write_Bit(false);
  uut2.Write_Bit(true);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());

  uut2.Write_uint8(0xAB);

  uut2.Close();

  EXPECT_EQ(0x55U, mem2[0]);
  EXPECT_EQ(0x66U, mem2[1]);
  EXPECT_EQ(0x77U, mem2[2]);
  EXPECT_EQ(0x05U, mem2[3]);

  uint8_t expected[] = { 0x01, 0x02, 0x03, 0x04, 0xAB};

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_BitPos)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut2.Write_uint8(0x55);
  uut2.Write_uint8(0x66);
  uut2.Write_uint8(0x77);
  uut2.Write_Bit(true);
  uut2.Write_Bit(false);
  uut2.Write_Bit(true);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  uut1 = std::move(uut2);

  ASSERT_EQ(IStreamWriter::States::closed, uut2.GetState());

  uut1.Write_Bit(false);
  uut1.Write_Bit(false);
  uut1.Write_Bit(true);

  uut1.Write_uint8(0xCD);

  uut1.Close();

  EXPECT_EQ(0x55U, mem2[0]);
  EXPECT_EQ(0x66U, mem2[1]);
  EXPECT_EQ(0x77U, mem2[2]);
  EXPECT_EQ(0x25U, mem2[3]);
  EXPECT_EQ(0xCDU, mem2[4]);

  uint8_t expected[] = { 0x01, 0x02, 0x03, 0x04 };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_Endian)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Big);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut1.GetEndian());
  ASSERT_EQ(IStreamWriter::Endian::Big, uut2.GetEndian());

  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut2.GetEndian());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_Closed)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut2.Write_uint8(0xAB);

  uut1.Close();
  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::closed, uut2.GetState());

  EXPECT_EQ(0xABU, mem2[0]);

  uint8_t expected[] = { 0xFF };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_Full)
{
  assert(sizeof(memory) > 5);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut1.Write_uint8(0x33);

  uut2.Write_uint8(0xAB);

  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::full, uut2.GetState());

  uut2.Close();

  ASSERT_EQ(0xABU, mem2[0]);

  uint8_t expected[] = { 0xFF, 0x33 };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_Error)
{
  assert(sizeof(memory) > 5);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut1.Write_uint8(0x33);
  ASSERT_THROW(uut1.Write_uint8(0x12), std::exception);

  uut2.Write_uint8(0xAB);

  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamWriter::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::error, uut2.GetState());

  uut2.Close();

  ASSERT_EQ(0xABU, mem2[0]);

  uint8_t expected[] = { 0xFF, 0x33 };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_LeaveError)
{
  assert(sizeof(memory) > 5);
  MemStreamWriter uut1(memory, 2, IStreamWriter::Endian::Little);

  uint8_t mem2[32];
  MemStreamWriter uut2(mem2, sizeof(mem2), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0xFF);
  uut1.Write_uint8(0x33);
  ASSERT_THROW(uut1.Write_uint8(0x12), std::exception);
  ASSERT_EQ(IStreamWriter::States::error, uut1.GetState());

  uut2.Write_uint8(0xAB);

  uut1 = std::move(uut2);

  ASSERT_EQ(IStreamWriter::States::open, uut1.GetState());
  ASSERT_EQ(IStreamWriter::States::closed, uut2.GetState());

  uut1.Write_uint8(0x58);
  uut1.Close();

  ASSERT_EQ(0xABU, mem2[0]);
  ASSERT_EQ(0x58U, mem2[1]);

  uint8_t expected[] = { 0xFF, 0x33 };
  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, MoveAssignment_Self)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut1.Write_uint8(0x55);
  uut1.Write_uint8(0x66);
  uut1.Write_uint8(0x77);
  uut1.Write_Bit(true);
  uut1.Write_Bit(false);
  uut1.Write_Bit(true);

  MemStreamWriter & uut2 = uut1;
  uut1 = std::move(uut2);

  uut1.Write_uint8(0x01);
  uut1.Write_uint8(0x02);
  uut1.Write_uint8(0x03);
  uut1.Write_uint8(0x04);

  uut1.Close();

  uint8_t expected[] = { 0x55, 0x66, 0x77, 0x05, 0x01, 0x02, 0x03, 0x04};

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteToZeroSizedStream)
{
  MemStreamWriter uut(memory, 0, IStreamWriter::Endian::Little);
  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  ASSERT_THROW(uut.Write_uint8(0x12), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteNothing)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(sizeof(memory), uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  ASSERT_NO_THROW(uut.Close());

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, DoubleClose)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  // 1st close
  ASSERT_NO_THROW(uut.Close());
  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  // 2nd close
  ASSERT_NO_THROW(uut.Close());
  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, NoClose)
{
  std::unique_ptr<MemStreamWriter> spUUT(new MemStreamWriter(memory, sizeof(memory), IStreamWriter::Endian::Little));

  // close is performed by destructor
  spUUT.reset();
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteLittleStreamOp)
{
  float f1 = 32.3;
  double d1 = 83.1;

  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut.GetEndian());

  uut << static_cast<uint8_t>(0x32);
  uut << static_cast<uint16_t>(0x9576);
  uut << static_cast<uint32_t>(0xABCD1234);
  uut << static_cast<uint64_t>(0x58624827AFEDCCAA);

  uut << static_cast<uint8_t>(0x00); // write offset afterwards: 16

  uut << static_cast<int8_t>(0x85);
  uut << static_cast<int16_t>(0x891A);
  uut << static_cast<int32_t>(0x9AFF5673);
  uut << static_cast<int64_t>(0xA2BCDEF77625392C);

  uut << static_cast<uint8_t>(0x00); // write offset afterwards: 32

  uut << f1;
  uut << d1;

  uut << true;
  EXPECT_EQ(1U, uut.GetNbOfCachedBits());
  uut << true;
  EXPECT_EQ(2U, uut.GetNbOfCachedBits());
  uut << false;
  EXPECT_EQ(3U, uut.GetNbOfCachedBits());
  uut << true;
  EXPECT_EQ(4U, uut.GetNbOfCachedBits());

  // 45

  uut << static_cast<char>('c');
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());
  uut << static_cast<char>('h');
  uut << static_cast<char>('a');
  uut << static_cast<char>('r');

  uut << std::string("Text");

  ASSERT_EQ(sizeof(memory) - 54, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = {  0x32, 0x76, 0x95, 0x34, 0x12, 0xCD, 0xAB, 0xAA, 0xCC, 0xED, 0xAF, 0x27, 0x48, 0x62, 0x58, 0x00,
                          0x85, 0x1A, 0x89, 0x73, 0x56, 0xFF, 0x9A, 0x2C, 0x39, 0x25, 0x76, 0xF7, 0xDE, 0xBC, 0xA2, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 'c',  'h',  'a',
                          'r',  'T',  'e',  'x',  't',  0x00, 0xFF};

  memcpy(&expected[32], &f1, sizeof(f1));
  memcpy(&expected[32 + sizeof(f1)], &d1, sizeof(d1));

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteLittleFuncCalls)
{
  float f1 = 32.3;
  double d1 = 83.1;

  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut.GetEndian());

  uut.Write_uint8(0x32);
  uut.Write_uint16(0x9576);
  uut.Write_uint32(0xABCD1234);
  uut.Write_uint64(0x58624827AFEDCCAA);

  uut.Write_uint8(0x00); // write offset afterwards: 16

  uut.Write_int8(0x85);
  uut.Write_int16(0x891A);
  uut.Write_int32(0x9AFF5673);
  uut.Write_int64(0xA2BCDEF77625392C);

  uut.Write_uint8(0x00); // write offset afterwards: 32

  uut.Write_float(f1);
  uut.Write_double(d1);

  uut.Write_bool(true);
  EXPECT_EQ(1U, uut.GetNbOfCachedBits());
  uut.Write_bool(true);
  EXPECT_EQ(2U, uut.GetNbOfCachedBits());
  uut.Write_bool(false);
  EXPECT_EQ(3U, uut.GetNbOfCachedBits());
  uut.Write_bool(true);
  EXPECT_EQ(4U, uut.GetNbOfCachedBits());

  uut.Write_Bit(false);
  EXPECT_EQ(5U, uut.GetNbOfCachedBits());
  uut.Write_Bit(false);
  EXPECT_EQ(6U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(7U, uut.GetNbOfCachedBits());
  uut.Write_Bit(false);
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());

  uut.Write_Bits(0x16, 5);
  EXPECT_EQ(5U, uut.GetNbOfCachedBits());

  // 46

  uut.Write_char('c');
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());
  uut.Write_char('h');
  uut.Write_char('a');
  uut.Write_char('r');

  uut.Write_string("Text");
  uut.Write_line("Line");

  ASSERT_EQ(sizeof(memory) - 60, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = {  0x32, 0x76, 0x95, 0x34, 0x12, 0xCD, 0xAB, 0xAA, 0xCC, 0xED, 0xAF, 0x27, 0x48, 0x62, 0x58, 0x00,
                          0x85, 0x1A, 0x89, 0x73, 0x56, 0xFF, 0x9A, 0x2C, 0x39, 0x25, 0x76, 0xF7, 0xDE, 0xBC, 0xA2, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B, 0x16, 'c',  'h',
                          'a',  'r',  'T',  'e',  'x',  't',  0x00, 'L',  'i',  'n',  'e',  '\n', 0xFF };

  memcpy(&expected[32], &f1, sizeof(f1));
  memcpy(&expected[32 + sizeof(f1)], &d1, sizeof(d1));

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBigStreamOp)
{
  float f1 = 32.3;
  double d1 = 83.1;

  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Big);

  ASSERT_EQ(IStreamWriter::Endian::Big, uut.GetEndian());

  uut << static_cast<uint8_t>(0x32);
  uut << static_cast<uint16_t>(0x9576);
  uut << static_cast<uint32_t>(0xABCD1234);
  uut << static_cast<uint64_t>(0x58624827AFEDCCAA);

  uut << static_cast<uint8_t>(0x00); // write offset afterwards: 16

  uut << static_cast<int8_t>(0x85);
  uut << static_cast<int16_t>(0x891A);
  uut << static_cast<int32_t>(0x9AFF5673);
  uut << static_cast<int64_t>(0xA2BCDEF77625392C);

  uut << static_cast<uint8_t>(0x00); // write offset afterwards: 32

  uut << f1;
  uut << d1;

  uut << true;
  EXPECT_EQ(1U, uut.GetNbOfCachedBits());
  uut << true;
  EXPECT_EQ(2U, uut.GetNbOfCachedBits());
  uut << false;
  EXPECT_EQ(3U, uut.GetNbOfCachedBits());
  uut << true;
  EXPECT_EQ(4U, uut.GetNbOfCachedBits());

  // 45

  uut << static_cast<char>('c');
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());
  uut << static_cast<char>('h');
  uut << static_cast<char>('a');
  uut << static_cast<char>('r');

  uut << std::string("Text");

  ASSERT_EQ(sizeof(memory) - 54, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = {  0x32, 0x95, 0x76, 0xAB, 0xCD, 0x12, 0x34, 0x58, 0x62, 0x48, 0x27, 0xAF, 0xED, 0xCC, 0xAA, 0x00,
                          0x85, 0x89, 0x1A, 0x9A, 0xFF, 0x56, 0x73, 0xA2, 0xBC, 0xDE, 0xF7, 0x76, 0x25, 0x39, 0x2C, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 'c',  'h',  'a',
                          'r',  'T',  'e',  'x',  't',  0x00, 0xFF };

  expected[32] = reinterpret_cast<uint8_t*>(&f1)[3];
  expected[33] = reinterpret_cast<uint8_t*>(&f1)[2];
  expected[34] = reinterpret_cast<uint8_t*>(&f1)[1];
  expected[35] = reinterpret_cast<uint8_t*>(&f1)[0];

  expected[36] = reinterpret_cast<uint8_t*>(&d1)[7];
  expected[37] = reinterpret_cast<uint8_t*>(&d1)[6];
  expected[38] = reinterpret_cast<uint8_t*>(&d1)[5];
  expected[39] = reinterpret_cast<uint8_t*>(&d1)[4];
  expected[40] = reinterpret_cast<uint8_t*>(&d1)[3];
  expected[41] = reinterpret_cast<uint8_t*>(&d1)[2];
  expected[42] = reinterpret_cast<uint8_t*>(&d1)[1];
  expected[43] = reinterpret_cast<uint8_t*>(&d1)[0];

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBigFuncCalls)
{
  float f1 = 32.3;
  double d1 = 83.1;

  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Big);

  ASSERT_EQ(IStreamWriter::Endian::Big, uut.GetEndian());

  uut.Write_uint8(0x32);
  uut.Write_uint16(0x9576);
  uut.Write_uint32(0xABCD1234);
  uut.Write_uint64(0x58624827AFEDCCAA);

  uut.Write_uint8(0x00); // write offset afterwards: 16

  uut.Write_int8(0x85);
  uut.Write_int16(0x891A);
  uut.Write_int32(0x9AFF5673);
  uut.Write_int64(0xA2BCDEF77625392C);

  uut.Write_uint8(0x00); // write offset afterwards: 32

  uut.Write_float(f1);
  uut.Write_double(d1);

  uut.Write_bool(true);
  EXPECT_EQ(1U, uut.GetNbOfCachedBits());
  uut.Write_bool(true);
  EXPECT_EQ(2U, uut.GetNbOfCachedBits());
  uut.Write_bool(false);
  EXPECT_EQ(3U, uut.GetNbOfCachedBits());
  uut.Write_bool(true);
  EXPECT_EQ(4U, uut.GetNbOfCachedBits());

  uut.Write_Bit(false);
  EXPECT_EQ(5U, uut.GetNbOfCachedBits());
  uut.Write_Bit(false);
  EXPECT_EQ(6U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(7U, uut.GetNbOfCachedBits());
  uut.Write_Bit(false);
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());

  uut.Write_Bits(0x16, 5);
  EXPECT_EQ(5U, uut.GetNbOfCachedBits());

  // 46

  uut.Write_char('c');
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());
  uut.Write_char('h');
  uut.Write_char('a');
  uut.Write_char('r');

  uut.Write_string("Text");
  uut.Write_line("Line");

  ASSERT_EQ(sizeof(memory) - 60, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = {  0x32, 0x95, 0x76, 0xAB, 0xCD, 0x12, 0x34, 0x58, 0x62, 0x48, 0x27, 0xAF, 0xED, 0xCC, 0xAA, 0x00,
                          0x85, 0x89, 0x1A, 0x9A, 0xFF, 0x56, 0x73, 0xA2, 0xBC, 0xDE, 0xF7, 0x76, 0x25, 0x39, 0x2C, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B, 0x16, 'c',  'h',
                          'a',  'r',  'T',  'e',  'x',  't',  0x00, 'L',  'i',  'n',  'e',  '\n', 0xFF };

  expected[32] = reinterpret_cast<uint8_t*>(&f1)[3];
  expected[33] = reinterpret_cast<uint8_t*>(&f1)[2];
  expected[34] = reinterpret_cast<uint8_t*>(&f1)[1];
  expected[35] = reinterpret_cast<uint8_t*>(&f1)[0];

  expected[36] = reinterpret_cast<uint8_t*>(&d1)[7];
  expected[37] = reinterpret_cast<uint8_t*>(&d1)[6];
  expected[38] = reinterpret_cast<uint8_t*>(&d1)[5];
  expected[39] = reinterpret_cast<uint8_t*>(&d1)[4];
  expected[40] = reinterpret_cast<uint8_t*>(&d1)[3];
  expected[41] = reinterpret_cast<uint8_t*>(&d1)[2];
  expected[42] = reinterpret_cast<uint8_t*>(&d1)[1];
  expected[43] = reinterpret_cast<uint8_t*>(&d1)[0];

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteMultipleElements)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut.GetEndian());

  uint8_t const data_u8[] = { 0x23, 0x87 };
  uut.Write_uint8(data_u8, 2);
  uint16_t const data_u16[] = { 0x9576, 0xACDC };
  uut.Write_uint16(data_u16, 2);
  uint32_t const data_u32[] = { 0xAB232DDC, 0x18457263 };
  uut.Write_uint32(data_u32, 2);
  uint64_t const data_u64[] = { 0x736492BB2C98AE72, 0x7482BB6C401BA7EF };
  uut.Write_uint64(data_u64, 2);

  // 30

  int8_t const data_i8[] = { static_cast<int8_t>(0xD5), static_cast<int8_t>(0xA2) };
  uut.Write_int8(data_i8, 2);
  int16_t const data_i16[] = { static_cast<int16_t>(0x0102), static_cast<int16_t>(0xA33F) };
  uut.Write_int16(data_i16, 2);
  int32_t const data_i32[] = { static_cast<int32_t>(0xCE33458E), static_cast<int32_t>(0x24CF2148) };
  uut.Write_int32(data_i32, 2);
  int64_t const data_i64[] = { static_cast<int64_t>(0x673647A638BC8DE2), static_cast<int64_t>(0xFF88F928EA3C5720) };
  uut.Write_int64(data_i64, 2);

  // 60

  float const data_float[] = { 33.3, -23E8 };
  uut.Write_float(data_float, 2);
  double const data_double[] = { 13.3, -23E-8 };
  uut.Write_double(data_double, 2);

  // 84

  bool const data_bool[] = { true, true, false, true };
  uut.Write_bool(data_bool, 4);

  uint8_t const data_bits[] = { 0x7E, 0x16 };

  uut.Write_Bits(data_bits, 13);

  // 87

  char const data_char[] = { 'c', 'h', 'a', 'r' };
  uut.Write_char(data_char, 4);

  // 91

  uut.Write_string("Text");

  // 96

  ASSERT_EQ(sizeof(memory) - 96, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = {  0x23, 0x87, 0x76, 0x95, 0xDC, 0xAC, 0xDC, 0x2D, 0x23, 0xAB, 0x63, 0x72, 0x45, 0x18, 0x72, 0xAE,
                          0x98, 0x2C, 0xBB, 0x92, 0x64, 0x73, 0xEF, 0xA7, 0x1B, 0x40, 0x6C, 0xBB, 0x82, 0x74, 0xD5, 0xA2,
                          0x02, 0x01, 0x3F, 0xA3, 0x8E, 0x45, 0x33, 0xCE, 0x48, 0x21, 0xCF, 0x24, 0xE2, 0x8D, 0xBC, 0x38,
                          0xA6, 0x47, 0x36, 0x67, 0x20, 0x57, 0x3C, 0xEA, 0x28, 0xF9, 0x88, 0xFF, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0xEB, 0x67, 0x01, 'c',  'h',  'a',  'r',  'T',  'e',  'x',  't',  0x00,
                          0xFF };

  memcpy(&expected[60], data_float, sizeof(data_float));
  memcpy(&expected[60 + sizeof(data_float)], data_double, sizeof(data_double));

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, AlignToByteBoundary_OK)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut.Write_Bit(true);
  uut.Write_Bit(false);
  EXPECT_EQ(6U, uut.AlignToByteBoundary(false));

  uut.FillBits(12, false);
  EXPECT_EQ(4U, uut.AlignToByteBoundary(true));

  uut.Write_uint8(0xDEU);
  EXPECT_EQ(0U, uut.AlignToByteBoundary(false));

  EXPECT_EQ(uut.RemainingCapacity(), sizeof(memory) - 4U);

  uut.Close();

  uint8_t expected[] = { 0x01U, 0x00U, 0xF0U, 0xDEU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, AlignToByteBoundary_InStateErrorAndClose)
{
  assert(sizeof(memory) > 16U);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xABU);
  uut.Write_uint8(0xCDU);
  uut.Write_uint8(0xEFU);
  uut.Write_uint8(0x35U);

  ASSERT_THROW(uut.Write_uint8(0x11U), FullError);
  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  EXPECT_THROW((void)uut.AlignToByteBoundary(false), ErrorStateError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  EXPECT_THROW((void)uut.AlignToByteBoundary(false), ClosedError);

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xABU, 0xCDU, 0xEFU, 0x35U, 0xFFU, 0xFFU, 0xFFU, 0xFFU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FillBitsAndBytes_OK)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut.FillBits(1, true);
  uut.FillBits(1, false);
  uut.Write_Bits(static_cast<uint8_t>(0x0FU), 4);
  uut.FillBytes(1, 0xFFU);
  uut.FillBytes(2, 0x55U);
  uut.FillBits(16, false);

  uut.FillBits(0, false);
  uut.FillBytes(0, 0);

  EXPECT_EQ(uut.RemainingCapacity(), sizeof(memory) - 6U);

  uut.Close();

  uint8_t expected[] = { 0x3DU, 0xFFU, 0x55U, 0x55U, 0x00U, 0x00U };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FullByFillBits)
{
  assert(sizeof(memory) > 16U);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xABU);
  uut.Write_uint8(0xCDU);
  uut.Write_uint8(0xEFU);

  uut.Write_Bits(static_cast<uint8_t>(0x00U), 7);

  uut.FillBits(1, true);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(0U, uut.RemainingCapacity());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xABU, 0xCDU, 0xEFU, 0x80U, 0xFFU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FullByFillBytes)
{
  assert(sizeof(memory) > 16U);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xABU);
  uut.Write_uint8(0xCDU);
  uut.Write_uint8(0xEFU);

  uut.FillBytes(1, 0x80U);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(0U, uut.RemainingCapacity());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xABU, 0xCDU, 0xEFU, 0x80U, 0xFFU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FillBitsOnFullStream)
{
  assert(sizeof(memory) > 16U);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xABU);
  uut.Write_uint8(0xCDU);
  uut.Write_uint8(0xEFU);
  uut.Write_uint8(0x35U);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(0U, uut.RemainingCapacity());

  EXPECT_THROW(uut.FillBits(1, false), FullError);

  EXPECT_EQ(IStreamWriter::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xABU, 0xCDU, 0xEFU, 0x35U, 0xFFU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FillBytesOnFullStream)
{
  assert(sizeof(memory) > 16U);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xABU);
  uut.Write_uint8(0xCDU);
  uut.Write_uint8(0xEFU);
  uut.Write_uint8(0x35U);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(0U, uut.RemainingCapacity());

  EXPECT_THROW(uut.FillBytes(1, 0x55U), FullError);

  EXPECT_EQ(IStreamWriter::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xABU, 0xCDU, 0xEFU, 0x35U, 0xFFU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FillBitsAndBytes_InStateErrorAndClose)
{
  assert(sizeof(memory) > 16U);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xABU);
  uut.Write_uint8(0xCDU);
  uut.Write_uint8(0xEFU);
  uut.Write_uint8(0x35U);

  ASSERT_THROW(uut.Write_uint8(0x11U), FullError);
  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  EXPECT_THROW(uut.FillBits(1, true), ErrorStateError);
  EXPECT_THROW(uut.FillBytes(1, 0x55U), ErrorStateError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  EXPECT_THROW(uut.FillBits(1, true), ClosedError);
  EXPECT_THROW(uut.FillBytes(1, 0x55U), ClosedError);

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xABU, 0xCDU, 0xEFU, 0x35U, 0xFFU, 0xFFU, 0xFFU, 0xFFU };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}

TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBits_UpperDoNotCare1)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  // bits 5, 6, and 7 must be ignored
  uut.Write_Bits(0xFF, 5);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0x1F, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBits_UpperDoNotCare2)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  // bits 10..15 must be ignored
  uint8_t bits[] = { 0xFF, 0xFF };
  uut.Write_Bits(bits, 10);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xFF, 0x03, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBits_NextByteAlignsProperly)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  uut.Write_Bits(0xFF, 5);
  uut.Write_uint8(0xAB); // <-- 3 padding bits must be added before 0xAB
  uut.Write_Bits(0xFF, 8);
  uut.Write_uint8(0xCD);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0x1F, 0xAB, 0xFF, 0xCD, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FullByByteWrite)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);
  uut.Write_uint8(0xEF);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(1), uut.RemainingCapacity());

  uut.Write_uint8(0x12);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xAB, 0xCD, 0xEF, 0x12, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FullByBitWrite)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);
  uut.Write_uint8(0xEF);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(1), uut.RemainingCapacity());

  uut.Write_Bits(0x12, 8);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xAB, 0xCD, 0xEF, 0x12, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, FullByBitAndByteWrite)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(2), uut.RemainingCapacity());

  uut.Write_Bits(0x12, 2);

  // note: bits do not count until a byte is full and they are written to the stream
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(2), uut.RemainingCapacity());

  uut.Write_uint8(0xEF);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xAB, 0xCD, 0x02, 0xEF, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CloseWritesRemainingBitsToStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(2), uut.RemainingCapacity());

  uut.Write_Bits(0x12, 2);

  // note: bits do not count until a byte is full and they are written to the stream
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(2), uut.RemainingCapacity());

  // close must write one more byte containing the two bits to the stream
  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  uint8_t expected[] = { 0xAB, 0xCD, 0x02, 0xFF };

  ASSERT_TRUE(compare_memory(expected, sizeof(expected)));
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteByteToFullStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);
  uut.Write_uint8(0xEF);
  uut.Write_uint8(0x12);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  ASSERT_THROW(uut.Write_uint8(0x55), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, Write8BitsToFullStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);
  uut.Write_uint8(0xEF);
  uut.Write_uint8(0x12);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  ASSERT_THROW(uut.Write_Bits(0x55, 8), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, Write1BitToFullStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);
  uut.Write_uint8(0xEF);
  uut.Write_uint8(0x12);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  ASSERT_THROW(uut.Write_Bit(true), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteTooManyBitsToStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 4, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);
  uut.Write_uint8(0xEF);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(1), uut.RemainingCapacity());

  uut.Write_Bits(0x55, 6);

  ASSERT_THROW(uut.Write_Bits(0x55, 3), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteByteInErrorState)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  ASSERT_THROW(uut.Write_uint8(0xEF), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  ASSERT_THROW(uut.Write_uint8(0xEF), ErrorStateError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBitsInErrorState)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  ASSERT_THROW(uut.Write_uint8(0xEF), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  ASSERT_THROW(uut.Write_Bit(true), ErrorStateError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteByteToClosedStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  uut.Close();

  ASSERT_THROW(uut.Write_uint8(0xEF), ClosedError);

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteBitToClosedStream)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  uut.Close();

  ASSERT_THROW(uut.Write_Bit(true), ClosedError);

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CloseStreamInErrorState)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  ASSERT_THROW(uut.Write_uint8(0xEF), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, RemainingCapacitySupported)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);
  EXPECT_TRUE(uut.IsRemainingCapacitySupported());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, RemainingCapacityInDifferentStates)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(2), uut.RemainingCapacity());

  uut.Write_uint8(0xAB);
  uut.Write_uint8(0xCD);

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());

  ASSERT_THROW(uut.Write_uint8(0xEF), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
  ASSERT_THROW(uut.RemainingCapacity(), ErrorStateError);

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
  ASSERT_THROW(uut.RemainingCapacity(), ClosedError);
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, GetNbOfCachedBitsInDifferentStates)
{
  assert(sizeof(memory) > 16);

  MemStreamWriter uut(memory, 2, IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Write_uint8(0xAB);
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(1U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(2U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(3U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(4U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(5U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(6U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(7U, uut.GetNbOfCachedBits());
  uut.Write_Bit(true);
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());

  ASSERT_EQ(IStreamWriter::States::full, uut.GetState());
  EXPECT_EQ(static_cast<size_t>(0), uut.RemainingCapacity());
  EXPECT_EQ(0U, uut.GetNbOfCachedBits());

  ASSERT_THROW(uut.Write_uint8(0xEF), FullError);

  ASSERT_EQ(IStreamWriter::States::error, uut.GetState());
  EXPECT_THROW(uut.RemainingCapacity(), ErrorStateError);
  EXPECT_THROW(uut.GetNbOfCachedBits(), ErrorStateError);

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
  EXPECT_THROW(uut.RemainingCapacity(), ClosedError);
  EXPECT_THROW(uut.GetNbOfCachedBits(), ClosedError);
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteZeroElements)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut.GetEndian());

  uint8_t const data_u8[] = { 0x23, 0x87 };
  ASSERT_NO_THROW(uut.Write_uint8(data_u8, 0));
  uint16_t const data_u16[] = { 0x9576, 0xACDC };
  ASSERT_NO_THROW(uut.Write_uint16(data_u16, 0));
  uint32_t const data_u32[] = { 0xAB232DDC, 0x18457263 };
  ASSERT_NO_THROW(uut.Write_uint32(data_u32, 0));
  uint64_t const data_u64[] = { 0x736492BB2C98AE72, 0x7482BB6C401BA7EF };
  ASSERT_NO_THROW(uut.Write_uint64(data_u64, 0));

  int8_t const data_i8[] = { static_cast<int8_t>(0xD5), static_cast<int8_t>(0xA2) };
  ASSERT_NO_THROW(uut.Write_int8(data_i8, 0));
  int16_t const data_i16[] = { static_cast<int16_t>(0x0102), static_cast<int16_t>(0xA33F) };
  ASSERT_NO_THROW(uut.Write_int16(data_i16, 0));
  int32_t const data_i32[] = { static_cast<int32_t>(0xCE33458E), static_cast<int32_t>(0x24CF2148) };
  ASSERT_NO_THROW(uut.Write_int32(data_i32, 0));
  int64_t const data_i64[] = { static_cast<int64_t>(0x673647A638BC8DE2), static_cast<int64_t>(0xFF88F928EA3C5720) };
  ASSERT_NO_THROW(uut.Write_int64(data_i64, 0));

  float const data_float[] = { 33.3, -23E8 };
  ASSERT_NO_THROW(uut.Write_float(data_float, 0));
  double const data_double[] = { 13.3, -23E-8 };
  ASSERT_NO_THROW(uut.Write_double(data_double, 0));

  bool const data_bool[] = { true, true, false, true };
  ASSERT_NO_THROW(uut.Write_bool(data_bool, 0));

  ASSERT_NO_THROW(uut.Write_Bits(static_cast<uint8_t>(0), 0));

  uint8_t const data_bits[] = { 0x7E, 0x16 };
  ASSERT_NO_THROW(uut.Write_Bits(data_bits, 0));

  char const data_char[] = { 'c', 'h', 'a', 'r' };
  uut.Write_char(data_char, 0);

  ASSERT_EQ(sizeof(memory), uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteEmptyString)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut.GetEndian());

  ASSERT_NO_THROW(uut.Write_string(std::string()));

  // check: only null-terminator must have been written
  ASSERT_EQ(sizeof(memory) - 1, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  ASSERT_EQ(0x00, memory[0]);
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, WriteEmptyLine)
{
  MemStreamWriter uut(memory, sizeof(memory), IStreamWriter::Endian::Little);

  ASSERT_EQ(IStreamWriter::Endian::Little, uut.GetEndian());

  ASSERT_NO_THROW(uut.Write_line(std::string()));

  // check: only '\n' must have been written
  ASSERT_EQ(sizeof(memory) - 1, uut.RemainingCapacity());
  ASSERT_EQ(IStreamWriter::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamWriter::States::closed, uut.GetState());

  ASSERT_EQ('\n', memory[0]);
}
TEST_F(GPCC_Stream_MemStreamWriter_Tests, CopyAssignment)
{
  MemStreamWriter uut1(memory, sizeof(memory), IStreamWriter::Endian::Little);
  MemStreamWriter uut2(memory, sizeof(memory) / 2U, IStreamWriter::Endian::Big);

  EXPECT_EQ(IStreamWriter::Endian::Little, uut1.GetEndian());
  EXPECT_EQ(sizeof(memory), uut1.RemainingCapacity());

  EXPECT_EQ(IStreamWriter::Endian::Big, uut2.GetEndian());
  EXPECT_EQ(sizeof(memory) / 2U, uut2.RemainingCapacity());

  uut2 = uut1;

  EXPECT_EQ(IStreamWriter::Endian::Little, uut2.GetEndian());
  EXPECT_EQ(sizeof(memory), uut2.RemainingCapacity());
}

} // namespace Stream
} // namespace gpcc_tests
