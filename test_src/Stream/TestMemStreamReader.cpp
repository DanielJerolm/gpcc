/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2020, 2022 Daniel Jerolm

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
#include "gpcc/src/Stream/MemStreamReader.hpp"
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

/// Test fixture for gpcc::Stream::MemStreamReader related tests.
class GPCC_Stream_MemStreamReader_Tests: public Test
{
  public:
    GPCC_Stream_MemStreamReader_Tests(void);

  protected:
    float const f1;
    float const f2;
    double const d1;
    double const d2;

    uint8_t memory[128];
    size_t n;

    void SetUp(void) override;
    void TearDown(void) override;

    void PrepareLittleEndianTestData1(void);
    void PrepeareLitteEndianTestData2(void);
    void PrepareBigEndianTestData1(void);
    bool AnyNotFF(uint8_t const * p, size_t s);
};
GPCC_Stream_MemStreamReader_Tests::GPCC_Stream_MemStreamReader_Tests(void)
: Test()
, f1(32.3)
, f2(-12.3E-6)
, d1(83.1)
, d2(67.342E16)
, memory()
, n(0)
{
}

void GPCC_Stream_MemStreamReader_Tests::SetUp(void)
{
  memset(memory, 0x00, sizeof(memory));
}
void GPCC_Stream_MemStreamReader_Tests::TearDown(void)
{
}
void GPCC_Stream_MemStreamReader_Tests::PrepareLittleEndianTestData1(void)
{
  static uint8_t const data[] =
  //             0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  /* 0x00  */ {  0x32, 0x76, 0x95, 0x34, 0x12, 0xCD, 0xAB, 0xAA, 0xCC, 0xED, 0xAF, 0x27, 0x48, 0x62, 0x58, 0x00,
  /* 0x10  */    0x85, 0x1A, 0x89, 0x73, 0x56, 0xFF, 0x9A, 0x2C, 0x39, 0x25, 0x76, 0xF7, 0xDE, 0xBC, 0xA2, 0x00,
  /* 0x20  */    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 'c',  'h',  'a',
  /* 0x30  */    'r',  'T',  'e',  'x',  't',  0x00, 'L',  'i',  'n',  'e',  '1',  '\n', 'L',  'i',  'n',  'e',
  /* 0x40  */    '2',  '\r', 'L',  'i',  'n',  'e',  '3',  '\r', '\n', 'L',  'i',  'n',  'e',  '4',  0x00 };

  n = sizeof(data);
  assert(n <= sizeof(memory));
  memcpy(memory, data, n);

  MemStreamWriter msw(memory + 32U, sizeof(float) + sizeof(double), IStreamWriter::Endian::Little);
  msw << f1;
  msw << d1;
  msw.Close();
}
void GPCC_Stream_MemStreamReader_Tests::PrepeareLitteEndianTestData2(void)
{
  static uint8_t const data[] =
  //             0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  /* 0x00 */  {  0x23, 0x87, 0x76, 0x95, 0xDC, 0xAC, 0xDC, 0x2D, 0x23, 0xAB, 0x63, 0x72, 0x45, 0x18, 0x72, 0xAE,
  /* 0x10 */     0x98, 0x2C, 0xBB, 0x92, 0x64, 0x73, 0xEF, 0xA7, 0x1B, 0x40, 0x6C, 0xBB, 0x82, 0x74, 0xD5, 0xA2,
  /* 0x20 */     0x02, 0x01, 0x3F, 0xA3, 0x8E, 0x45, 0x33, 0xCE, 0x48, 0x21, 0xCF, 0x24, 0xE2, 0x8D, 0xBC, 0x38,
  /* 0x30 */     0xA6, 0x47, 0x36, 0x67, 0x20, 0x57, 0x3C, 0xEA, 0x28, 0xF9, 0x88, 0xFF, 0x00, 0x00, 0x00, 0x00,
  /* 0x40 */     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  /* 0x50 */     0x00, 0x00, 0x00, 0x00, 0xEB, 0x67, 0x01, 'c',  'h',  'a',  'r',  'T',  'e',  'x',  't',  0x00 };

  n = sizeof(data);
  assert(n <= sizeof(memory));
  memcpy(memory, data, n);

  MemStreamWriter msw(memory + 0x3C, 2*sizeof(float) + 2*sizeof(double), IStreamWriter::Endian::Little);
  msw << f1 << f2;
  msw << d1 << d2;
  msw.Close();
}
void GPCC_Stream_MemStreamReader_Tests::PrepareBigEndianTestData1(void)
{
  static uint8_t const data[] =
  //             0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
  /* 0x00 */  {  0x32, 0x95, 0x76, 0xAB, 0xCD, 0x12, 0x34, 0x58, 0x62, 0x48, 0x27, 0xAF, 0xED, 0xCC, 0xAA, 0x00,
  /* 0x10 */     0x85, 0x89, 0x1A, 0x9A, 0xFF, 0x56, 0x73, 0xA2, 0xBC, 0xDE, 0xF7, 0x76, 0x25, 0x39, 0x2C, 0x00,
  /* 0x20 */     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 'c',  'h',  'a',
  /* 0x30  */    'r',  'T',  'e',  'x',  't',  0x00, 'L',  'i',  'n',  'e',  '1',  '\n', 'L',  'i',  'n',  'e',
  /* 0x40  */    '2',  '\r', 'L',  'i',  'n',  'e',  '3',  '\r', '\n', 'L',  'i',  'n',  'e',  '4',  0x00 };

  n = sizeof(data);
  assert(n <= sizeof(memory));
  memcpy(memory, data, n);

  MemStreamWriter msw(memory + 32, sizeof(float) + sizeof(double), IStreamWriter::Endian::Big);
  msw << f1;
  msw << d1;
  msw.Close();
}
bool GPCC_Stream_MemStreamReader_Tests::AnyNotFF(uint8_t const * p, size_t s)
{
  while (s-- != 0)
  {
    if (*p++ != 0xFF)
      return true;
  }
  return false;
}

TEST_F(GPCC_Stream_MemStreamReader_Tests, pMemIsnullptrButSizeIsNotZero)
{
  EXPECT_THROW(MemStreamReader uut(nullptr, 1, IStreamReader::Endian::Little), std::invalid_argument);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ZeroSize1)
{
  MemStreamReader uut(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ZeroSize2)
{
  MemStreamReader uut(nullptr, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  uut2 >> data;
  EXPECT_EQ(0x32, data);

  uut2 >> data;
  EXPECT_EQ(0x76, data);

  uut1.Close();

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction_Endian_Little)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut1.GetEndian());

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut2.GetEndian());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction_Endian_Big)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Big);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut1.GetEndian());

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut2.GetEndian());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction_BitPos)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  uint8_t data;

  // read some bits
  data = uut1.Read_bits(3);
  EXPECT_EQ(0x02, data);

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  data = uut1.Read_bits(3);
  EXPECT_EQ(0x06, data);

  data = uut2.Read_bits(3);
  EXPECT_EQ(0x06, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  uut2 >> data;
  EXPECT_EQ(0x76, data);

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction_StateClosed)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  uut1.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction_StateEmpty)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uut1.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uut2.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyConstruction_StateError)
{
  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_THROW((void)uut1.Read_uint8(), EmptyError);
  ASSERT_EQ(IStreamReader::States::error, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());

  uut1.Close();

  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());
  uut2.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // move-create a new instance
  MemStreamReader uut2(std::move(uut1));
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction_Endian_Little)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut1.GetEndian());

  // move-create a copy
  MemStreamReader uut2(std::move(uut1));
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut2.GetEndian());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction_Endian_Big)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Big);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut1.GetEndian());

  // move-create a copy
  MemStreamReader uut2(std::move(uut1));
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut2.GetEndian());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction_BitPos)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  uint8_t data;

  // read some bits
  data = uut1.Read_bits(3);
  EXPECT_EQ(0x02, data);

  // move-create a new instance
  MemStreamReader uut2(std::move(uut1));
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  data = uut2.Read_bits(3);
  EXPECT_EQ(0x06, data);

  uut2 >> data;
  EXPECT_EQ(0x76, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction_StateClosed)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  uut1.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  // move-create a new instance
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction_StateEmpty)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut1.GetState());

  // move-create a new instance
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uut1.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uut2.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveConstruction_StateError)
{
  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_THROW((void)uut1.Read_uint8(), EmptyError);
  ASSERT_EQ(IStreamReader::States::error, uut1.GetState());

  // move-create a new instance
  MemStreamReader uut2(uut1);
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());

  uut1.Close();

  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());
  uut2.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // copy assign uut1 -> uut2
  uut2 = uut1;

  uut1 >> data;
  EXPECT_EQ(0x95, data);

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut1.Close();

  uut2 >> data;
  EXPECT_EQ(0x34, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_Endian_Little)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Big);
  ASSERT_EQ(IStreamReader::Endian::Big, uut2.GetEndian());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // copy assign uut1 -> uut2
  uut2 = uut1;

  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut1.GetEndian());
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut2.GetEndian());

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2 >> data;
  EXPECT_EQ(0x34, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_Endian_Big)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Big);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::Endian::Little, uut2.GetEndian());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // copy assign uut1 -> uut2
  uut2 = uut1;

  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut1.GetEndian());
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut2.GetEndian());

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2 >> data;
  EXPECT_EQ(0x34, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_BitPos)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  data = uut1.Read_bits(4);
  EXPECT_EQ(0x6, data);

  // copy assign uut1 -> uut2
  uut2 = uut1;

  uut1 >> data;
  EXPECT_EQ(0x95, data);

  data = uut2.Read_bits(4);
  EXPECT_EQ(0x7, data);

  uut1.Close();

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_StateClosed)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  // create a copy
  MemStreamReader uut2(uut1);

  uut1.Close();

  uint8_t data;

  uut2 >> data;
  EXPECT_EQ(0x32, data);

  // copy assign uut1 -> uut2
  uut2 = uut1;

  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_StateEmpty)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut1.GetState());

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Little);

  uint8_t data;

  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut2 = uut1;
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uut1.Close();
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_StateError)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut1.GetState());
  ASSERT_THROW((void)uut1.Read_char(), std::exception);

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Little);

  uint8_t data;

  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut2 = uut1;
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());

  uut1.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_RecoverFromErrorState)
{
  assert(sizeof(memory) >= 5U);
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 5, IStreamReader::Endian::Little);

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  ASSERT_THROW((void)uut2.Read_char(), std::exception);
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());

  uut2 = uut1;
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  uint8_t data;
  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_RecoverFromClosedState)
{
  assert(sizeof(memory) >= 5U);
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 5, IStreamReader::Endian::Little);

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  uut2.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());

  uut2 = uut1;
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  uint8_t data;
  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut1.Close();
  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CopyAssignment_Self)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  MemStreamReader& uut1_b = uut1;
  uut1 = uut1_b;

  uut1 >> data;
  EXPECT_EQ(0x95, data);

  uut1.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // move assign uut1 -> uut2
  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_Endian_Little)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Big);
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // move assign uut1 -> uut2
  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Little, uut2.GetEndian());

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_Endian_Big)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Big);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  // move assign uut1 -> uut2
  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::Endian::Big, uut2.GetEndian());

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_BitPos)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  data = uut1.Read_bits(4);
  EXPECT_EQ(0x6, data);

  // move assign uut1 -> uut2
  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  data = uut2.Read_bits(4);
  EXPECT_EQ(0x7, data);

  uut2 >> data;
  EXPECT_EQ(0x95, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_StateClosed)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());

  uut1.Close();

  uint8_t data;

  uut2 >> data;
  EXPECT_EQ(0x32, data);

  // move-copy assign uut1 -> uut2
  uut2 = std::move(uut1);

  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_StateEmpty)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut1.GetState());

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Little);

  uint8_t data;

  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::empty, uut2.GetState());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_StateError)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut1.GetState());
  ASSERT_THROW((void)uut1.Read_char(), std::exception);

  MemStreamReader uut2(memory, sizeof(memory), IStreamReader::Endian::Little);

  uint8_t data;

  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_RecoverFromErrorState)
{
  assert(sizeof(memory) >= 5U);
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 5, IStreamReader::Endian::Little);

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  ASSERT_THROW((void)uut2.Read_char(), std::exception);
  ASSERT_EQ(IStreamReader::States::error, uut2.GetState());

  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  uint8_t data;
  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_RecoverFromClosedState)
{
  assert(sizeof(memory) >= 5U);
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, 5, IStreamReader::Endian::Little);

  MemStreamReader uut2(memory, 0, IStreamReader::Endian::Little);
  uut2.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut2.GetState());

  uut2 = std::move(uut1);
  ASSERT_EQ(IStreamReader::States::open, uut2.GetState());
  ASSERT_EQ(IStreamReader::States::closed, uut1.GetState());

  uint8_t data;
  uut2 >> data;
  ASSERT_EQ(0x32, data);

  uut2.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, MoveAssignment_Self)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut1(memory, sizeof(memory), IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::open, uut1.GetState());
  uint8_t data;

  uut1 >> data;
  EXPECT_EQ(0x32, data);

  uut1 >> data;
  EXPECT_EQ(0x76, data);

  MemStreamReader& uut1_b = uut1;
  uut1 = std::move(uut1_b);

  uut1 >> data;
  EXPECT_EQ(0x95, data);

  uut1.Close();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadFromZeroSizedStream)
{
  MemStreamReader uut(memory, 0, IStreamReader::Endian::Little);
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  ASSERT_THROW((void)uut.Read_uint8(), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadNothing)
{
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  ASSERT_EQ(sizeof(memory), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  ASSERT_NO_THROW(uut.Close());

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, DoubleClose)
{
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  // 1st close
  ASSERT_NO_THROW(uut.Close());
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());

  // 2nd close
  ASSERT_NO_THROW(uut.Close());
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, NoClose)
{
  std::unique_ptr<MemStreamReader> spUUT(new MemStreamReader(memory, sizeof(memory), IStreamReader::Endian::Little));

  // close is performed by destructor
  spUUT.reset();
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLittleStreamOp)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::Endian::Little, uut.GetEndian());

  uint8_t u8;
  uut >> u8;
  ASSERT_EQ(0x32, u8);

  uint16_t u16;
  uut >> u16;
  ASSERT_EQ(0x9576, u16);

  uint32_t u32;
  uut >> u32;
  ASSERT_EQ(0xABCD1234, u32);

  uint64_t u64;
  uut >> u64;
  ASSERT_EQ(static_cast<uint64_t>(0x58624827AFEDCCAA), u64);

  uut >> u8;
  ASSERT_EQ(0, u8);

  int8_t i8;
  uut >> i8;
  ASSERT_EQ(static_cast<int8_t>(0x85), i8);

  int16_t i16;
  uut >> i16;
  ASSERT_EQ(static_cast<int16_t>(0x891A), i16);

  int32_t i32;
  uut >> i32;
  ASSERT_EQ(static_cast<int32_t>(0x9AFF5673), i32);

  int64_t i64;
  uut >> i64;
  ASSERT_EQ(static_cast<int64_t>(0xA2BCDEF77625392C), i64);

  uut >> u8;
  ASSERT_EQ(0, u8);

  float f;
  uut >> f;
  ASSERT_TRUE((f > f1 - 0.1) && (f < f1 + 0.1));

  double d;
  uut >> d;
  ASSERT_TRUE((d > d1 - 0.1) && (d < d1 + 0.1));

  bool b;
  uut >> b;
  ASSERT_TRUE(b);
  uut >> b;
  ASSERT_TRUE(b);
  uut >> b;
  ASSERT_FALSE(b);
  uut >> b;
  ASSERT_TRUE(b);

  // 45

  char c;
  uut >> c;
  ASSERT_EQ('c', c);
  uut >> c;
  ASSERT_EQ('h', c);
  uut >> c;
  ASSERT_EQ('a', c);
  uut >> c;
  ASSERT_EQ('r', c);

  std::string s;
  uut >> s;
  ASSERT_TRUE(s == "Text");
  uut >> s;
  ASSERT_TRUE(s == "Line1\nLine2\rLine3\r\nLine4");

  ASSERT_EQ(sizeof(memory) - n, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLittleFuncCalls)
{
  PrepareLittleEndianTestData1();

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::Endian::Little, uut.GetEndian());

  uint8_t u8 = uut.Read_uint8();
  ASSERT_EQ(0x32, u8);

  uint16_t u16 = uut.Read_uint16();
  ASSERT_EQ(0x9576, u16);

  uint32_t u32 = uut.Read_uint32();
  ASSERT_EQ(0xABCD1234, u32);

  uint64_t u64 = uut.Read_uint64();
  ASSERT_EQ(static_cast<uint64_t>(0x58624827AFEDCCAA), u64);

  u8 = uut.Read_uint8();
  ASSERT_EQ(0, u8);

  int8_t i8 = uut.Read_int8();
  ASSERT_EQ(static_cast<int8_t>(0x85), i8);

  int16_t i16 = uut.Read_int16();
  ASSERT_EQ(static_cast<int16_t>(0x891A), i16);

  int32_t i32 = uut.Read_int32();
  ASSERT_EQ(static_cast<int32_t>(0x9AFF5673), i32);

  int64_t i64 = uut.Read_int64();
  ASSERT_EQ(static_cast<int64_t>(0xA2BCDEF77625392C), i64);

  u8 = uut.Read_uint8();
  ASSERT_EQ(0, u8);

  float f = uut.Read_float();
  ASSERT_TRUE((f > f1 - 0.1) && (f < f1 + 0.1));

  double d = uut.Read_double();
  ASSERT_TRUE((d > d1 - 0.1) && (d < d1 + 0.1));

  bool b;
  b = uut.Read_bool();
  ASSERT_TRUE(b);
  b = uut.Read_bit();
  ASSERT_TRUE(b);
  u8 = uut.Read_bits(2);
  ASSERT_EQ(2, u8);

  // 45

  char c;
  c = uut.Read_char();
  ASSERT_EQ('c', c);
  c = uut.Read_char();
  ASSERT_EQ('h', c);
  c = uut.Read_char();
  ASSERT_EQ('a', c);
  c = uut.Read_char();
  ASSERT_EQ('r', c);

  std::string s = uut.Read_string();
  ASSERT_TRUE(s == "Text");

  s = uut.Read_line();
  ASSERT_TRUE(s == "Line1");
  s = uut.Read_line();
  ASSERT_TRUE(s == "Line2");
  s = uut.Read_line();
  ASSERT_TRUE(s == "Line3");
  s = uut.Read_line();
  ASSERT_TRUE(s == "Line4");
  // see test cases "ReadLine_NoEnd1" and "ReadLine_NoEnd2" to test Read_line() with no line ending at end of stream

  ASSERT_EQ(sizeof(memory) - n, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBigStreamOp)
{
  PrepareBigEndianTestData1();

  float f1 = 32.3;
  double d1 = 83.1;

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Big);

  ASSERT_EQ(IStreamReader::Endian::Big, uut.GetEndian());

  uint8_t u8;
  uut >> u8;
  ASSERT_EQ(static_cast<uint8_t>(0x32), u8);

  uint16_t u16;
  uut >> u16;
  ASSERT_EQ(static_cast<uint16_t>(0x9576), u16);

  uint32_t u32;
  uut >> u32;
  ASSERT_EQ(static_cast<uint32_t>(0xABCD1234), u32);

  uint64_t u64;
  uut >> u64;
  ASSERT_EQ(static_cast<uint64_t>(0x58624827AFEDCCAA), u64);

  uut >> u8;
  ASSERT_EQ(static_cast<uint8_t>(0x00), u8);

  int8_t i8;
  uut >> i8;
  ASSERT_EQ(static_cast<int8_t>(0x85), i8);

  int16_t i16;
  uut >> i16;
  ASSERT_EQ(static_cast<int16_t>(0x891A), i16);

  int32_t i32;
  uut >> i32;
  ASSERT_EQ(static_cast<int32_t>(0x9AFF5673), i32);

  int64_t i64;
  uut >> i64;
  ASSERT_EQ(static_cast<int64_t>(0xA2BCDEF77625392C), i64);

  uut >> u8;
  ASSERT_EQ(static_cast<uint8_t>(0x00), u8);

  float f;
  uut >> f;
  ASSERT_TRUE((f > f1 - 0.1) && (f < f1 + 0.1));

  double d;
  uut >> d;
  ASSERT_TRUE((d > d1 - 0.1) && (d < d1 + 0.1));

  bool b;
  uut >> b;
  ASSERT_TRUE(b);
  uut >> b;
  ASSERT_TRUE(b);
  uut >> b;
  ASSERT_FALSE(b);
  uut >> b;
  ASSERT_TRUE(b);

  // 45

  char c;
  uut >> c;
  ASSERT_EQ('c', c);
  uut >> c;
  ASSERT_EQ('h', c);
  uut >> c;
  ASSERT_EQ('a', c);
  uut >> c;
  ASSERT_EQ('r', c);

  std::string s;
  uut >> s;
  ASSERT_TRUE(s == "Text");
  uut >> s;
  ASSERT_TRUE(s == "Line1\nLine2\rLine3\r\nLine4");

  ASSERT_EQ(sizeof(memory) - n, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBigFuncCalls)
{
  PrepareBigEndianTestData1();

  float f1 = 32.3;
  double d1 = 83.1;

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Big);

  ASSERT_EQ(IStreamReader::Endian::Big, uut.GetEndian());

  uint8_t u8 = uut.Read_uint8();
  ASSERT_EQ(static_cast<uint8_t>(0x32), u8);

  uint16_t u16 = uut.Read_uint16();
  ASSERT_EQ(static_cast<uint16_t>(0x9576), u16);

  uint32_t u32 = uut.Read_uint32();
  ASSERT_EQ(static_cast<uint32_t>(0xABCD1234), u32);

  uint64_t u64 = uut.Read_uint64();
  ASSERT_EQ(static_cast<uint64_t>(0x58624827AFEDCCAA), u64);

  u8 = uut.Read_uint8();
  ASSERT_EQ(static_cast<uint8_t>(0x00), u8);

  int8_t i8 = uut.Read_int8();
  ASSERT_EQ(static_cast<int8_t>(0x85), i8);

  int16_t i16 = uut.Read_int16();
  ASSERT_EQ(static_cast<int16_t>(0x891A), i16);

  int32_t i32 = uut.Read_int32();
  ASSERT_EQ(static_cast<int32_t>(0x9AFF5673), i32);

  int64_t i64 = uut.Read_int64();
  ASSERT_EQ(static_cast<int64_t>(0xA2BCDEF77625392C), i64);

  u8 = uut.Read_uint8();
  ASSERT_EQ(static_cast<uint8_t>(0x00), u8);

  float f = uut.Read_float();
  ASSERT_TRUE((f > f1 - 0.1) && (f < f1 + 0.1));

  double d = uut.Read_double();
  ASSERT_TRUE((d > d1 - 0.1) && (d < d1 + 0.1));

  bool b;
  b = uut.Read_bool();
  ASSERT_TRUE(b);
  b = uut.Read_bit();
  ASSERT_TRUE(b);
  u8 = uut.Read_bits(2);
  ASSERT_EQ(2, u8);

  // 45

  char c;
  c = uut.Read_char();
  ASSERT_EQ('c', c);
  c = uut.Read_char();
  ASSERT_EQ('h', c);
  c = uut.Read_char();
  ASSERT_EQ('a', c);
  c = uut.Read_char();
  ASSERT_EQ('r', c);

  std::string s = uut.Read_string();
  ASSERT_TRUE(s == "Text");

  s = uut.Read_line();
  ASSERT_TRUE(s == "Line1");
  s = uut.Read_line();
  ASSERT_TRUE(s == "Line2");
  s = uut.Read_line();
  ASSERT_TRUE(s == "Line3");
  s = uut.Read_line();
  ASSERT_TRUE(s == "Line4");
  // see test cases "ReadLine_NoEnd1" and "ReadLine_NoEnd2" to test Read_line() with no line ending at end of stream

  ASSERT_EQ(sizeof(memory) - n, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadMultipleElements)
{
  PrepeareLitteEndianTestData2();

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::Endian::Little, uut.GetEndian());

  uint8_t const data_u8[] = { 0x23, 0x87 };
  uint8_t read_data_u8[2];
  uut.Read_uint8(read_data_u8, 2);
  ASSERT_TRUE(memcmp(data_u8, read_data_u8, sizeof(data_u8)) == 0);

  uint16_t const data_u16[] = { 0x9576, 0xACDC };
  uint16_t read_data_u16[2];
  uut.Read_uint16(read_data_u16, 2);
  ASSERT_TRUE(memcmp(data_u16, read_data_u16, sizeof(data_u16)) == 0);

  uint32_t const data_u32[] = { 0xAB232DDC, 0x18457263 };
  uint32_t read_data_u32[2];
  uut.Read_uint32(read_data_u32, 2);
  ASSERT_TRUE(memcmp(data_u32, read_data_u32, sizeof(data_u32)) == 0);

  uint64_t const data_u64[] = { 0x736492BB2C98AE72, 0x7482BB6C401BA7EF };
  uint64_t read_data_u64[2];
  uut.Read_uint64(read_data_u64, 2);
  ASSERT_TRUE(memcmp(data_u64, read_data_u64, sizeof(data_u64)) == 0);

  // 30

  int8_t const data_i8[] = { static_cast<int8_t>(0xD5), static_cast<int8_t>(0xA2) };
  int8_t read_data_i8[2];
  uut.Read_int8(read_data_i8, 2);
  ASSERT_TRUE(memcmp(data_i8, read_data_i8, sizeof(data_i8)) == 0);

  int16_t const data_i16[] = { static_cast<int16_t>(0x0102), static_cast<int16_t>(0xA33F) };
  int16_t read_data_i16[2];
  uut.Read_int16(read_data_i16, 2);
  ASSERT_TRUE(memcmp(data_i16, read_data_i16, sizeof(data_i16)) == 0);

  int32_t const data_i32[] = { static_cast<int32_t>(0xCE33458E), static_cast<int32_t>(0x24CF2148) };
  int32_t read_data_i32[2];
  uut.Read_int32(read_data_i32, 2);
  ASSERT_TRUE(memcmp(data_i32, read_data_i32, sizeof(data_i32)) == 0);

  int64_t const data_i64[] = { static_cast<int64_t>(0x673647A638BC8DE2), static_cast<int64_t>(0xFF88F928EA3C5720) };
  int64_t read_data_i64[2];
  uut.Read_int64(read_data_i64, 2);
  ASSERT_TRUE(memcmp(data_i64, read_data_i64, sizeof(data_i64)) == 0);

  // 60

  float const data_float[] = { f1, f2 };
  float read_data_float[2];
  uut.Read_float(read_data_float, 2);
  ASSERT_TRUE(memcmp(data_float, read_data_float, sizeof(data_float)) == 0);

  double const data_double[] = { d1, d2 };
  double read_data_double[2];
  uut.Read_double(read_data_double, 2);
  ASSERT_TRUE(memcmp(data_double, read_data_double, sizeof(data_double)) == 0);

  // 84

  bool const data_bool[] = { true, true, false, true };
  bool read_data_bool[4];
  uut.Read_bool(read_data_bool, 4);
  ASSERT_TRUE(memcmp(data_bool, read_data_bool, sizeof(data_bool)) == 0);

  uint8_t const data_bits[] = { 0x7E, 0x16 };
  uint8_t read_data_bits[2];
  uut.Read_bits(read_data_bits, 13);
  ASSERT_TRUE(memcmp(data_bits, read_data_bits, sizeof(data_bits)) == 0);

  // 87

  char const data_char[] = { 'c', 'h', 'a', 'r' };
  char read_data_char[4];
  uut.Read_char(read_data_char, 4);
  ASSERT_TRUE(memcmp(data_char, read_data_char, sizeof(data_char)) == 0);

  // 91

  std::string s = uut.Read_string();
  ASSERT_TRUE(s == "Text");

  // 96

  ASSERT_EQ(sizeof(memory) - n, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBits_UpperZero)
{
  memory[0] = 0x1F;
  memory[1] = 0xFF;

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_bits(3);
  ASSERT_EQ(0x07, u8);

  uint8_t au8[2];

  uut.Read_bits(au8, 9);
  ASSERT_EQ(0xE3, au8[0]);
  ASSERT_EQ(0x01, au8[1]);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBits_NextByteAlignsProperly)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_bits(4);
  ASSERT_EQ(0x0A, u8);

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, EmptyByByteRead)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, EmptyByBitRead)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(2), uut.RemainingBytes());

  uint8_t u8;
  bool b;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(1), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_TRUE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_TRUE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadByteFromEmptyStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  ASSERT_THROW(u8 = uut.Read_uint8(), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBitFromEmptyStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;
  bool b;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  ASSERT_THROW(b = uut.Read_bit(), EmptyError);
  (void)b;

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadTooManyBitsFromAlmostEmptyStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;
  bool b;

  b = uut.Read_bit();
  ASSERT_FALSE(b);
  b = uut.Read_bit();
  ASSERT_TRUE(b);
  b = uut.Read_bit();
  ASSERT_FALSE(b);
  b = uut.Read_bit();
  ASSERT_TRUE(b);

  u8 = uut.Read_bits(8);
  ASSERT_EQ(0x2F, u8);

  // 4 bits left

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  ASSERT_THROW(u8 = uut.Read_bits(5), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadStringFromEmptyStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  ASSERT_THROW(std::string str = uut.Read_string(), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLineFromEmptyStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  ASSERT_THROW(std::string str = uut.Read_line(), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadByteInErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t au8[3];

  ASSERT_THROW(uut.Read_uint8(au8, 3), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  ASSERT_THROW(au8[0] = uut.Read_uint8(), ErrorStateError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBitInErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t au8[3];

  ASSERT_THROW(uut.Read_uint8(au8, 3), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  bool b;
  ASSERT_THROW(b = uut.Read_bit(), ErrorStateError);
  (void)b;

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadStringInErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t au8[3];

  ASSERT_THROW(uut.Read_uint8(au8, 3), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  ASSERT_THROW(std::string str = uut.Read_string(), ErrorStateError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLineInErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t au8[3];

  ASSERT_THROW(uut.Read_uint8(au8, 3), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  ASSERT_THROW(std::string str = uut.Read_line(), ErrorStateError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadByteFromClosedStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  uint8_t u8;
  ASSERT_THROW(u8 = uut.Read_uint8(), ClosedError);
  (void)u8;
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadBitFromClosedStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  bool b;
  ASSERT_THROW(b = uut.Read_bit(), ClosedError);
  (void)b;
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadStringFromClosedStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  ASSERT_THROW(std::string str = uut.Read_string(), ClosedError);
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLineFromClosedStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  ASSERT_THROW(std::string str = uut.Read_line(), ClosedError);
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, CloseStreamInErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t au8[3];

  ASSERT_THROW(uut.Read_uint8(au8, 3), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, RemainingBytesSupported)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  ASSERT_TRUE(uut.IsRemainingBytesSupported());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, RemainingBytesInDifferentStates)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;
  memory[2] = 0x45;
  memory[3] = 0xB6;

  uint8_t u8;
  uint16_t u16;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(4), uut.RemainingBytes());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(3), uut.RemainingBytes());

  u16 = uut.Read_uint16();
  ASSERT_EQ(0x4512, u16);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(1), uut.RemainingBytes());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xB6, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());
  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());

  ASSERT_THROW(u8 = uut.Read_uint8(), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());
  ASSERT_THROW(uut.RemainingBytes(), ErrorStateError);

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
  ASSERT_THROW(uut.RemainingBytes(), ClosedError);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadZeroElements)
{
  uint8_t readMem[16];
  memset(readMem, 0xFF, sizeof(readMem));

  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  ASSERT_EQ(IStreamReader::Endian::Little, uut.GetEndian());

  uut.Read_uint8(readMem, 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_uint16(reinterpret_cast<uint16_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_uint32(reinterpret_cast<uint32_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_uint64(reinterpret_cast<uint64_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));

  uut.Read_int8(reinterpret_cast<int8_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_int16(reinterpret_cast<int16_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_int32(reinterpret_cast<int32_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_int64(reinterpret_cast<int64_t*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));

  uut.Read_float(reinterpret_cast<float*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_double(reinterpret_cast<double*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));

  uut.Read_bool(reinterpret_cast<bool*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_bits(readMem, 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));
  uut.Read_char(reinterpret_cast<char*>(readMem), 0);
  ASSERT_FALSE(AnyNotFF(readMem, sizeof(readMem)));


  ASSERT_EQ(sizeof(memory), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadEmptyString1)
{
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  std::string s = uut.Read_string();

  ASSERT_EQ(static_cast<size_t>(0), s.length());

  ASSERT_EQ(sizeof(memory) - 1, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadEmptyString2)
{
  assert(sizeof(memory) >= 1);

  MemStreamReader uut(memory, 1, IStreamReader::Endian::Little);

  std::string s = uut.Read_string();

  ASSERT_EQ(static_cast<size_t>(0), s.length());

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadStringButNoNullTerminator1)
{
  assert(sizeof(memory) >= 5);

  memory[0] = 'H';
  memory[1] = 'e';
  memory[2] = 'l';
  memory[3] = 'l';
  memory[4] = 'o';

  MemStreamReader uut(memory, sizeof(5), IStreamReader::Endian::Little);

  ASSERT_THROW(std::string s = uut.Read_string(), std::runtime_error);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());
  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadStringButNoNullTerminator2)
{
  assert(sizeof(memory) >= 1);

  memory[0] = 'A';

  MemStreamReader uut(memory, 1, IStreamReader::Endian::Little);

  ASSERT_THROW(std::string s = uut.Read_string(), std::runtime_error);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());
  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_NUL)
{
  // two empty lines, terminated by NUL

  assert(sizeof(memory) > 2U);
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(sizeof(memory) - 2U, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_LF)
{
  // two empty lines, terminated by \n

  assert(sizeof(memory) > 2U);
  memory[0] = '\n';
  memory[1] = '\n';
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(sizeof(memory) - 2U, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_CR)
{
  // two empty lines, terminated by \r

  assert(sizeof(memory) > 2U);
  memory[0] = '\r';
  memory[1] = '\r';
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(sizeof(memory) - 2U, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_CRLF)
{
  // two empty lines, terminated by \r\n
  assert(sizeof(memory) > 4U);
  memory[0] = '\r';
  memory[1] = '\n';
  memory[2] = '\r';
  memory[3] = '\n';
  MemStreamReader uut(memory, sizeof(memory), IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(sizeof(memory) - 4U, uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_NUL_plus_END)
{
  // one empty line, terminated by NUL; No more data in stream
  assert(sizeof(memory) >= 1U);

  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_LF_plus_END)
{
  // one empty line, terminated by \n; No more data in stream
  assert(sizeof(memory) >= 1U);
  memory[0] = '\n';
  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_CR_plus_END)
{
  // one empty line, terminated by \r; No more data in stream
  assert(sizeof(memory) >= 1U);
  memory[0] = '\r';
  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_Empty_CRLF_plus_END)
{
  // one empty line, terminated by \r\n; No more data in stream
  assert(sizeof(memory) >= 2U);
  memory[0] = '\r';
  memory[1] = '\n';
  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s.empty());

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_NoEnd1)
{
  assert(sizeof(memory) >= 5U);
  memory[0] = 'H';
  memory[1] = 'e';
  memory[2] = 'l';
  memory[3] = 'l';
  memory[4] = 'o';

  MemStreamReader uut(memory, 5U, IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s == "Hello");

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, ReadLine_NoEnd2)
{
  assert(sizeof(memory) >= 1U);
  memory[0] = 'A';

  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  std::string s = uut.Read_line();
  ASSERT_TRUE(s == "A");

  ASSERT_EQ(static_cast<size_t>(0), uut.RemainingBytes());
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_ZeroBits)
{
  memory[0] = 0x57U;
  memory[1] = 0xE9U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;

  uut.Skip(0U);
  ASSERT_EQ(uut.RemainingBytes(), 2U);

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x57, u8);

  uut.Skip(0U);
  ASSERT_EQ(uut.RemainingBytes(), 1U);

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xE9, u8);

  uut.Skip(0U);
  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsLeft_SkipSomeBits)
{
  // There are 4 bits left that have not been read yet. We skip 3 of them.
  memory[0] = 0x8AU;

  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(3U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_bits(1U), 0x01U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsAndOneByteLeft_SkipAllBits)
{
  // There are 4 bits + 1 Byte left that have not been read yet. We skip 4 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(4U);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_uint8(), 0xDBU);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsLeft_SkipAll)
{
  // There are 4 bits left that have not been read yet. We skip them all.
  memory[0] = 0x8AU;

  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(4U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsLeft_SkipAllPlusOne)
{
  // There are 4 bits left that have not been read yet. We skip them all + 1.
  memory[0] = 0x8AU;

  MemStreamReader uut(memory, 1U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(uut.Skip(5U), EmptyError);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::error);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsAndOneByteLeft_SkipAllBitsAndOneByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 12 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(12U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsAndOneByteLeft_SkipAllBitsAndTwoByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+8=20 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(uut.Skip(20U), EmptyError);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::error);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsAndOneByteLeft_SkipAllBitsAndOneByteAndOneBit)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+1 = 13 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(uut.Skip(13U), EmptyError);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::error);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsAndTwoByteLeft_SkipAllBitsAndOneByte)
{
  // There are 4 bits + 2 byte left that have not been read yet. We skip 4+8=12 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;
  memory[2] = 0x36U;

  MemStreamReader uut(memory, 3U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 2U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(12U);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_uint8(), 0x36U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_BitsAndTwoByteLeft_SkipAllBitsAndOneByteAndOneBit)
{
  // There are 4 bits + 2 byte left that have not been read yet. We skip 4+8+1=13 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;
  memory[2] = 0x36U;

  MemStreamReader uut(memory, 3U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(4U), 0x0AU);
  ASSERT_EQ(uut.RemainingBytes(), 2U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(13U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_bits(7U), 0x1BU);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_OneByteLeft_Skip8Bits)
{
  // There is 1 byte left that has not been read yet. We skip 8 bits.
  memory[0] = 0x8AU;
  memory[1] = 0xDBU;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(8U), 0x8AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(8U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_OneByteLeft_Skip7Bits)
{
  // There is 1 byte left that has not been read yet. We skip 7 bits.
  memory[0] = 0x8AU;
  memory[1] = 0x80U;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(8U), 0x8AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  uut.Skip(7U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_bit(), 0x01U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_OneByteLeft_Skip9Bits)
{
  // There is 1 byte left that has not been read yet. We skip 9 bits.
  memory[0] = 0x8AU;
  memory[1] = 0x80U;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  ASSERT_EQ(uut.Read_bits(8U), 0x8AU);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(uut.Skip(9U), EmptyError);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::error);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_TwoByteLeft_Skip8Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 8 bits.
  memory[0] = 0x8AU;
  memory[1] = 0x80U;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  // - precondition established -

  uut.Skip(8U);
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_uint8(), 0x80U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_TwoByteLeft_Skip16Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 16 bits.
  memory[0] = 0x8AU;
  memory[1] = 0x80U;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  // - precondition established -

  uut.Skip(16U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_TwoByteLeft_Skip9Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 9 bits.
  memory[0] = 0x8AU;
  memory[1] = 0x80U;

  MemStreamReader uut(memory, 2U, IStreamReader::Endian::Little);

  // - precondition established -

  uut.Skip(9U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::open);

  ASSERT_EQ(uut.Read_bits(7U), 0x40U);
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  ASSERT_EQ(uut.GetState(), IStreamReader::States::empty);

  uut.Close();
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_EmptyStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t u8;

  u8 = uut.Read_uint8();
  ASSERT_EQ(0xFA, u8);

  ASSERT_EQ(IStreamReader::States::open, uut.GetState());

  u8 = uut.Read_uint8();
  ASSERT_EQ(0x12, u8);

  ASSERT_EQ(IStreamReader::States::empty, uut.GetState());

  ASSERT_THROW(uut.Skip(1U), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_ClosedStream)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  ASSERT_THROW(uut.Skip(1U), ClosedError);
  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Skip_StreamInErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uint8_t au8[3];

  ASSERT_THROW(uut.Read_uint8(au8, 3), EmptyError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  ASSERT_THROW(uut.Skip(1U), ErrorStateError);

  ASSERT_EQ(IStreamReader::States::error, uut.GetState());

  uut.Close();

  ASSERT_EQ(IStreamReader::States::closed, uut.GetState());
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_FrontMidBack)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;
  memory[2] = 0x34;
  memory[3] = 0x56;
  memory[4] = 0x78;
  memory[5] = 0x9A;
  memory[6] = 0xBC;
  memory[7] = 0xDE;

  MemStreamReader uut(memory, 8, IStreamReader::Endian::Little);

  // create a sub-stream at front
  auto uut2 = uut.SubStream(2);
  ASSERT_TRUE(uut2.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut2.RemainingBytes(), 2U);
  EXPECT_EQ(uut2.Read_uint8(), 0xFAU);
  EXPECT_EQ(uut2.Read_uint8(), 0x12U);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  uut2.Close();

  // uut should have 6 bytes left, read-ptr at [2] (0x34)
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 6U);
  EXPECT_EQ(uut.Read_uint8(), 0x34U);


  // uut is at [3] now. Create a sub-stream here (in the middle).
  uut2 = uut.SubStream(2);
  ASSERT_TRUE(uut2.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut2.RemainingBytes(), 2U);
  EXPECT_EQ(uut2.Read_uint8(), 0x56U);
  EXPECT_EQ(uut2.Read_uint8(), 0x78U);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  uut2.Close();

  // uut should have 3 bytes left, read-ptr at [5]
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 3U);
  EXPECT_EQ(uut.Read_uint8(), 0x9AU);

  // uut is at [6] now. Create a sub-stream here (at the end)
  uut2 = uut.SubStream(2);
  ASSERT_TRUE(uut2.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut2.RemainingBytes(), 2U);
  EXPECT_EQ(uut2.Read_uint8(), 0xBCU);
  EXPECT_EQ(uut2.Read_uint8(), 0xDEU);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  uut2.Close();

  // uut should be empty now
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_DropBits)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;
  memory[2] = 0x34;
  memory[3] = 0x56;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  EXPECT_EQ(uut.Read_bits(4), 0x0AU);

  // read-ptr of uut is at 0.4 now. Create a sub-stream here.
  auto uut2 = uut.SubStream(2);
  ASSERT_TRUE(uut2.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut2.RemainingBytes(), 2U);

  // uut should have 1 byte left now
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 1U);

  // sub-stream should read bytes [1] and [2]
  EXPECT_EQ(uut2.Read_uint8(), 0x12U);
  EXPECT_EQ(uut2.Read_uint8(), 0x34U);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  uut2.Close();

  // uut's read-ptr should be at [3] now
  EXPECT_EQ(uut.Read_uint8(), 0x56U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_ZeroSize)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;
  memory[2] = 0x34;
  memory[3] = 0x56;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  EXPECT_EQ(uut.Read_bits(4), 0x0AU);

  // uut is at 0.4 now. Create a sub-stream of size zero here.
  auto uut2 = uut.SubStream(0);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  EXPECT_EQ(uut2.RemainingBytes(), 0U);
  uut2.Close();
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::closed);

  // uut should have 3 bytes left now and read-ptr should be at [1]
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 3U);
  EXPECT_EQ(uut.Read_uint8(), 0x12U);

  // uut is at 2.0 now. Create a sub-stream of size zero here.
  uut2 = uut.SubStream(0);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  EXPECT_EQ(uut2.RemainingBytes(), 0U);
  uut2.Close();
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::closed);

  // uut should have 2 bytes left now and read-ptr should be at [2]
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 2U);

  EXPECT_EQ(uut.Read_uint8(), 0x34U);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_TooLarge)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;
  memory[2] = 0x34;
  memory[3] = 0x56;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(5); (void)uut2;, EmptyError);

  // uut should be open and read-ptr should be at [0]
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 4U);

  // move read-ptr to [3]
  uut.Skip(3 * 8);

  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(2); (void)uut2;, EmptyError);

  // uut should be open and read-ptr should be at [3]
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  ASSERT_EQ(uut.RemainingBytes(), 1U);

  // skip 4 bits
  uut.Skip(4);

  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(1); (void)uut2;, EmptyError);

  // uut should be open and read-ptr should be at 3.4
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::open);
  uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_Empty)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  uut.Skip(16);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);

  // -- precondition established --

  // check that a sub-stream of size zero can be created
  auto uut2 = uut.SubStream(0);
  EXPECT_TRUE(uut2.GetState() == IStreamReader::States::empty);
  uut2.Close();

  // state of uut should have not changed
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);

  // check that a size > 0 results in "EmptyError"
  EXPECT_THROW(uut2 = uut.SubStream(1), EmptyError);

  // state of uut should have not changed
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_Closed)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  // -- precondition established --

  // check that no sub-stream can be created
  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(0); (void)uut2;, ClosedError);
  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(1); (void)uut2;, ClosedError);

  // state of uut should have not changed
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::closed);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, SubStream_ErrorState)
{
  memory[0] = 0xFA;
  memory[1] = 0x12;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  EXPECT_THROW(uut.Skip(24), EmptyError);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::error);

  // -- precondition established --

  // check that no sub-stream can be created
  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(0); (void)uut2;, ErrorStateError);
  EXPECT_THROW(MemStreamReader uut2 = uut.SubStream(1); (void)uut2;, ErrorStateError);

  // state of uut should have not changed
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::error);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_AttemptToEnlarge)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  ASSERT_EQ(uut.RemainingBytes(), 2U);

  // attempt to enlarge in state "open"
  EXPECT_THROW(uut.Shrink(3), std::invalid_argument);

  uut.Skip(16);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);

  // attempt to enlarge in state "empty"
  EXPECT_THROW(uut.Shrink(1U), std::invalid_argument);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_NoEffect)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  ASSERT_EQ(uut.RemainingBytes(), 2U);

  // There are 2 bytes left. This shrink should have no effect.
  uut.Shrink(2U);
  EXPECT_EQ(uut.RemainingBytes(), 2U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  uint8_t data;
  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x0AU);

  // There are 4 bits and 1 byte left now. The following shrink should have no effect.
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  uut.Shrink(1U);
  EXPECT_EQ(uut.RemainingBytes(), 1U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x0FU);

  // There are 8 bits left now. The following shrink should have no effect.
  ASSERT_EQ(uut.RemainingBytes(), 1U);
  uut.Shrink(1U);
  EXPECT_EQ(uut.RemainingBytes(), 1U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x02U);

  // There are 4 bits left now. The following shrink should have no effect.
  ASSERT_EQ(uut.RemainingBytes(), 0U);
  uut.Shrink(0U);
  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x01U);

  // The stream is empty now. The following shrink should have no effect.
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);
  EXPECT_NO_THROW(uut.Shrink(0));
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_OneByte_NoBitsLeftToBeRead)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  ASSERT_EQ(uut.RemainingBytes(), 2U);

  // There are 2 bytes left to be read. Let's shrink to 1 byte.
  uut.Shrink(1);
  EXPECT_EQ(uut.RemainingBytes(), 1U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  uint8_t data;
  data = uut.Read_bits(8);
  EXPECT_EQ(data, 0xFAU);

  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_OneByte_BitsLeftToBeRead)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;
  memory[2] = 0xD7U;

  MemStreamReader uut(memory, 3, IStreamReader::Endian::Little);
  ASSERT_EQ(uut.RemainingBytes(), 3U);

  uint8_t data;
  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x0AU);

  // There are 2 bytes and 4 bits left. Let's shrink to 1 byte and 4 bits.
  ASSERT_EQ(uut.RemainingBytes(), 2U);
  uut.Shrink(1);
  EXPECT_EQ(uut.RemainingBytes(), 1U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x0FU);

  data = uut.Read_bits(8);
  EXPECT_EQ(data, 0x12U);

  ASSERT_TRUE(uut.GetState() == gpcc::Stream::IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_AllBytes_WithBitsLeft)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;
  memory[2] = 0xD7U;

  MemStreamReader uut(memory, 3, IStreamReader::Endian::Little);
  ASSERT_EQ(uut.RemainingBytes(), 3U);

  uint8_t data;
  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x0AU);

  // There are 2 bytes and 4 bits left. Let's shrink to zero bytes and 4 bits.
  ASSERT_EQ(uut.RemainingBytes(), 2U);
  uut.Shrink(0);
  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);

  data = uut.Read_bits(4);
  EXPECT_EQ(data, 0x0FU);

  ASSERT_TRUE(uut.GetState() == gpcc::Stream::IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_AllBytes_WithoutBitsLeft)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;
  memory[2] = 0xD7U;

  MemStreamReader uut(memory, 3, IStreamReader::Endian::Little);
  ASSERT_EQ(uut.RemainingBytes(), 3U);

  // There are 3 bytes left. Let's shrink to zero bytes.
  uut.Shrink(0);
  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_StreamClosed)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Close();

  EXPECT_THROW(uut.Shrink(0), ClosedError);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, Shrink_StreamInErrorState)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  ASSERT_THROW(uut.Skip(24), EmptyError);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::error);

  EXPECT_THROW(uut.Shrink(0), ErrorStateError);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_OK)
{
  memory[0] = 0x12U;
  memory[1] = 0x34U;
  memory[2] = 0x56U;
  memory[3] = 0x78U;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[0]);

  // skip memory[0]
  uut.Skip(8U);
  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[1]);

  // skip 1st bit of memory[1]
  uut.Skip(1U);
  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[2]);

  // skip remaining bits of memory[1]
  uut.Skip(7U);
  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[2]);

  // skip memory[2]
  uut.Skip(8U);
  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[3]);

  // skip memory[3]
  uut.Skip(8U);

  EXPECT_TRUE(uut.GetState() == IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_OK_LastByteBitByBit)
{
  memory[0] = 0x12U;
  memory[1] = 0x34U;
  memory[2] = 0x56U;
  memory[3] = 0x78U;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[0]);

  // skip memory[0..2]
  uut.Skip(3U * 8U);
  EXPECT_EQ(uut.GetReadPtr(memory, 4U), &memory[3]);

  // skip 1st bit of memory[3]
  uut.Skip(1U);
  EXPECT_THROW((void)uut.GetReadPtr(memory, 4U), std::logic_error);

  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_ZeroLength)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 0U, IStreamReader::Endian::Little);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);

  EXPECT_THROW((void)uut.GetReadPtr(memory, 0U), std::logic_error);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_CopyOfMSR_OK)
{
  memory[0] = 0x12U;
  memory[1] = 0x34U;
  memory[2] = 0x56U;
  memory[3] = 0x78U;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  // skip memory[0]
  uut.Skip(8U);

  MemStreamReader uut2(uut);

  EXPECT_EQ(uut2.GetReadPtr(memory, 4U), &memory[1]);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_StateEmpty)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Skip(16U);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::empty);

  EXPECT_THROW((void)uut.GetReadPtr(memory, 2U), std::logic_error);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_StateClose)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  uut.Skip(16U);
  uut.Close();
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::closed);

  EXPECT_THROW((void)uut.GetReadPtr(memory, 2U), std::logic_error);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_StateError)
{
  memory[0] = 0xFAU;
  memory[1] = 0x12U;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);
  ASSERT_THROW(uut.Skip(24U), std::exception);
  ASSERT_TRUE(uut.GetState() == IStreamReader::States::error);

  EXPECT_THROW((void)uut.GetReadPtr(memory, 2U), std::logic_error);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_ParamsNotPlausible)
{
  memory[0] = 0x12U;
  memory[1] = 0x34U;
  memory[2] = 0x56U;
  memory[3] = 0x78U;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  EXPECT_THROW((void)uut.GetReadPtr((&memory[0]) - 1U, 4U), std::logic_error);
  EXPECT_THROW((void)uut.GetReadPtr((&memory[0]) + 1U, 4U), std::logic_error);
  EXPECT_THROW((void)uut.GetReadPtr((&memory[0]) + 0U, 3U), std::logic_error);
  EXPECT_THROW((void)uut.GetReadPtr((&memory[0]) + 0U, 5U), std::logic_error);

  EXPECT_TRUE(uut.GetState() == IStreamReader::States::open);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, GetReadPtr_OtherMSRnotAccepted)
{
  memory[0] = 0x12U;
  memory[1] = 0x34U;
  memory[2] = 0x56U;
  memory[3] = 0x78U;

  MemStreamReader uut(memory, 4, IStreamReader::Endian::Little);

  uint8_t memory2[4];
  memory2[0] = 0xDEU;
  memory2[1] = 0xADU;
  memory2[2] = 0xBEU;
  memory2[3] = 0xEFU;

  EXPECT_THROW((void)uut.GetReadPtr(memory2, 4U), std::logic_error);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, EnsureAllDataConsumed_OK_1)
{
  memory[0] = 0x00;
  memory[1] = 0x00;
  memory[2] = 0x00;

  MemStreamReader uut(memory, 3, IStreamReader::Endian::Little);

  // (3 bytes left) -------------------------------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_uint16(); // (1 byte left) -------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 1U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::open);

  (void)uut.Read_bit(); // (7 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven));
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::open);

  (void)uut.Read_bit(); // (6 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (5 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (4 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (3 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (2 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (1 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::open);

  (void)uut.Read_bit(); // (0 bit left) -----------------------------------------------------------------------
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, EnsureAllDataConsumed_OK_2)
{
  memory[0] = 0x00;
  memory[1] = 0x00;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  // (2 bytes left) -------------------------------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    (void)uut.Read_bit();
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
    EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
    EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
    EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

    EXPECT_EQ(uut.RemainingBytes(), 1U);
    EXPECT_EQ(uut.GetState(), IStreamReader::States::open);
  }

  (void)uut.Read_bit(); // (7 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven));
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::open);

  (void)uut.Read_bit(); // (6 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (5 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (4 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (3 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (2 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)uut.Read_bit(); // (1 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::open);

  (void)uut.Read_bit(); // (0 bit left) -----------------------------------------------------------------------
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(uut.RemainingBytes(), 0U);
  EXPECT_EQ(uut.GetState(), IStreamReader::States::empty);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, EnsureAllDataConsumed_ErrorState)
{
  memory[0] = 0x00;
  memory[1] = 0x00;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  // create error condition
  ASSERT_THROW((void)uut.Read_uint32(), EmptyError);

  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), ErrorStateError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any), ErrorStateError);
}
TEST_F(GPCC_Stream_MemStreamReader_Tests, EnsureAllDataConsumed_ClosedState)
{
  memory[0] = 0x00;
  memory[1] = 0x00;

  MemStreamReader uut(memory, 2, IStreamReader::Endian::Little);

  // create pre-condition
  uut.Close();

  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), ClosedError);
  EXPECT_THROW(uut.EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any), ClosedError);
}

} // namespace Stream
} // namespace gpcc_tests
