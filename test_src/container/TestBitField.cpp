/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/container/BitField.hpp>
#include "gtest/gtest.h"
#include <string>

namespace gpcc_tests {
namespace container  {

using namespace gpcc::container;
using namespace testing;

static bool TestBits(BitField const & bf, size_t const n, uint8_t const * pExpectedData)
{
  // Checks the content of a BitField
  // true = same, false = not the same

  if (bf.GetSize() != n)
    return false;

  for (size_t i = 0; i < n; i++)
  {
    bool const expected = ((pExpectedData[i / 8U] & (1U << (i % 8U))) != 0);
    if (bf.GetBit(i) != expected)
      return false;
  }

  return true;
}
#ifndef SKIP_VERYBIGMEM_TESTS
static bool TestBitsIncomplete(BitField const & bf, size_t const n, uint8_t const * pExpectedData)
{
  // Checks the content of a BitField. Only the first n bits are tested.
  // Any number of additional bits in "bf" is accepted.
  // true = same, false = not the same

  if (bf.GetSize() < n)
    return false;

  for (size_t i = 0; i < n; i++)
  {
    bool const expected = ((pExpectedData[i / 8U] & (1U << (i % 8U))) != 0);
    if (bf.GetBit(i) != expected)
      return false;
  }

  return true;
}
#endif

TEST(gpcc_container_BitField_Tests, DefaultConstructor)
{
  BitField uut;
  ASSERT_EQ(0U, uut.GetSize());
}
TEST(gpcc_container_BitField_Tests, Constructor_nBits_Zero)
{
  BitField uut(0);
  ASSERT_EQ(0U, uut.GetSize());
}
TEST(gpcc_container_BitField_Tests, Constructor_nBits_CheckBitsCleared)
{
  for (size_t s = 1; s <= 128; s++)
  {
    BitField uut(s);
    ASSERT_EQ(s, uut.GetSize());
    for (size_t i = 0; i < s; i++)
    {
      ASSERT_FALSE(uut.GetBit(i));
    }
  }
}
#ifndef SKIP_VERYBIGMEM_TESTS
TEST(gpcc_container_BitField_Tests, Constructor_nBits_MaxSize)
{
  size_t const max = std::numeric_limits<size_t>::max() - (BitField::storage_t_size_in_bit - 1U);

  ASSERT_THROW(BitField uut1(max + 1U), std::length_error);

  try
  {
    BitField uut2(max);
    ASSERT_EQ(max, uut2.GetSize());
  }
  catch (std::length_error&)
  {
    ASSERT_FALSE(true);
  }
  catch (std::bad_alloc const &)
  {
    // ok
  }
}
#endif
TEST(gpcc_container_BitField_Tests, Constructor_FromBinaryData_nullptrNotAccepted)
{
  ASSERT_THROW(BitField uut1(0, nullptr), std::invalid_argument);
  ASSERT_THROW(BitField uut2(1, nullptr), std::invalid_argument);
}
TEST(gpcc_container_BitField_Tests, Constructor_FromBinaryData_Zero)
{
  uint8_t data = 0U;
  BitField uut(0, &data);
  ASSERT_EQ(0U, uut.GetSize());
}
TEST(gpcc_container_BitField_Tests, Constructor_FromBinaryData)
{
  uint8_t  data[16] = { 0x12, 0x82, 0xA6, 0xBC, 0xF7, 0x9C, 0xCD, 0x2B,
                        0x82, 0x28, 0xB6, 0x3D, 0xAB, 0xA5, 0x5A, 0x22 };

  // take 2 bits from data into uut1
  BitField uut1(2, data);
  ASSERT_FALSE(uut1.GetBit(0));
  ASSERT_TRUE(uut1.GetBit(1));

  // take complete data into uut2
  BitField uut2(128, data);
  ASSERT_TRUE(TestBits(uut2, 128, data));

  // take all but last 8 bits of data into uut3
  BitField uut3(120, data);
  ASSERT_TRUE(TestBits(uut3, 120, data));

  // take 2 bits from data into uut4
  BitField uut4(2, data);
  ASSERT_FALSE(uut4.GetBit(0));
  ASSERT_TRUE(uut4.GetBit(1));

  // enlarge uut3. Resize() should not reallocate (16/32/64/128 boundary). Top 8 bits must be zero.
  uut3.Resize(128);
  data[15] = 0x00;
  ASSERT_TRUE(TestBits(uut3, 128, data));

  // enlarge uut1 to 128. Resize() will likely reallocate. Top 126 bits must be zero.
  data[0] = 0x02;
  memset(&data[1], 0x0, sizeof(data)-1);
  uut1.Resize(128);
  ASSERT_TRUE(TestBits(uut1, 128, data));

  // enlarge uut4 to 8. Resize() should not reallocate. Top 6 bits must be zero.
  uut4.Resize(8);
  ASSERT_TRUE(TestBits(uut4, 8, data));
}
TEST(gpcc_container_BitField_Tests, Constructor_FromBinaryData_CopyNoRef)
{
  uint8_t data = 0xFF;

  BitField uut(8, &data);

  // manipulate data
  data = 0;

  // uut must still contain the original data
  for (size_t i = 0; i < 8; i++)
  {
    ASSERT_TRUE(uut.GetBit(i));
  }
}
TEST(gpcc_container_BitField_Tests, CopyConstructor)
{
  uint8_t data = 0x21;

  BitField uut1(8, &data);
  BitField uut2(uut1);

  uut1.SetAll();

  ASSERT_TRUE(TestBits(uut2, 8, &data));
}
TEST(gpcc_container_BitField_Tests, CopyConstructor_Zero)
{
  uint8_t data = 0x21;

  BitField uut1(0, &data);
  BitField uut2(uut1);

  ASSERT_EQ(0U, uut2.GetSize());
}
TEST(gpcc_container_BitField_Tests, MoveConstructor)
{
  uint8_t data = 0x21;

  BitField uut1(8, &data);
  BitField uut2(std::move(uut1));

  ASSERT_EQ(0U, uut1.GetSize());

  ASSERT_TRUE(TestBits(uut2, 8, &data));
}
TEST(gpcc_container_BitField_Tests, CopyAssign_Self)
{
  uint8_t data = 0x21;

  BitField uut(8, &data);

  // this construct avoids complaints from the eclipse indexer
  #define SELFASSIGN uut = uut
  SELFASSIGN;
  #undef SELFASSIGN

  ASSERT_TRUE(TestBits(uut, 8, &data));
}
TEST(gpcc_container_BitField_Tests, CopyAssign_NewLengthZero)
{
  uint8_t data1 = 0x21;

  BitField uut1(8, &data1);
  BitField const uut2;
  uut1 = uut2;

  ASSERT_EQ(0U, uut1.GetSize());
}
TEST(gpcc_container_BitField_Tests, CopyAssign_SameLength)
{
  uint8_t data = 0x21;

  BitField uut1(8, &data);
  BitField uut2(8);
  uut2 = uut1;

  uut1.ClearAll();

  ASSERT_TRUE(TestBits(uut2, 8, &data));
}
TEST(gpcc_container_BitField_Tests, CopyAssign_DifferentLength)
{
  uint8_t const data1[16] = { 0x12, 0x82, 0xA6, 0xBC, 0xF7, 0x9C, 0xCD, 0x2B,
                              0x82, 0x28, 0xB6, 0x3D, 0xAB, 0xA5, 0x5A, 0x22 };

  uint8_t data2 = 0x21;

  BitField uut1(128, data1);
  BitField uut2(1, &data2);
  uut2 = uut1;

  uut1.ClearAll();

  ASSERT_TRUE(TestBits(uut2, 128, data1));
}
TEST(gpcc_container_BitField_Tests, MoveAssign_Self)
{
  uint8_t data = 0x21;

  BitField uut(8, &data);

  uut = std::move(uut);

  ASSERT_TRUE(TestBits(uut, 8, &data));
}
TEST(gpcc_container_BitField_Tests, MoveAssign_NewLengthZero)
{
  uint8_t data = 0x21;

  BitField uut1(8, &data);
  BitField uut2;
  uut1 = std::move(uut2);

  ASSERT_EQ(0U, uut1.GetSize());
  ASSERT_EQ(0U, uut2.GetSize());
}
TEST(gpcc_container_BitField_Tests, MoveAssign)
{
  uint8_t data = 0x21;

  BitField uut1(8, &data);
  BitField uut2(128);

  uut2 = std::move(uut1);

  ASSERT_EQ(0U, uut1.GetSize());

  ASSERT_TRUE(TestBits(uut2, 8, &data));
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_CompareToSelf)
{
  BitField uut1(0);

  ASSERT_TRUE(uut1 == uut1);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_ZeroSize)
{
  BitField uut1(0);
  BitField uut2(0);

  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_DifferentSize)
{
  BitField uut1(0);
  BitField uut2(1);
  BitField uut3(2);

  ASSERT_FALSE(uut1 == uut2);
  ASSERT_FALSE(uut1 == uut3);
  ASSERT_FALSE(uut2 == uut3);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_1Bit)
{
  uint8_t const data[] = { 0xA5U };
  BitField uut1(1, data);
  BitField uut2(1, data);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];

  ASSERT_FALSE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_7Bits)
{
  uint8_t const data[] = { 0xA5U };
  BitField uut1(7, data);
  BitField uut2(7, data);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];
  ASSERT_FALSE(uut1 == uut2);

  uut1[0] = !uut1[0];
  uut1[6] = !uut1[6];
  ASSERT_FALSE(uut1 == uut2);

  uut1[6] = !uut1[6];
  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_8Bits)
{
  uint8_t const data[] = { 0xA5U };
  BitField uut1(8, data);
  BitField uut2(8, data);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];
  ASSERT_FALSE(uut1 == uut2);

  uut1[0] = !uut1[0];
  uut1[7] = !uut1[7];
  ASSERT_FALSE(uut1 == uut2);

  uut1[7] = !uut1[7];
  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_9Bits)
{
  uint8_t const data[] = { 0xA5U, 0x01U };
  BitField uut1(9, data);
  BitField uut2(9, data);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];
  ASSERT_FALSE(uut1 == uut2);

  uut1[0] = !uut1[0];
  uut1[7] = !uut1[7];
  ASSERT_FALSE(uut1 == uut2);

  uut1[7] = !uut1[7];
  uut1[8] = !uut1[8];
  ASSERT_FALSE(uut1 == uut2);

  uut1[8] = !uut1[8];
  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_9Bits_UpperUnusedBitsDiffer)
{
  uint8_t const data[] = { 0xA5U, 0xF1U };
  BitField uut1(16, data);
  BitField uut2(9, data);

  uut1.Resize(9);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];
  ASSERT_FALSE(uut1 == uut2);

  uut1[0] = !uut1[0];
  uut1[7] = !uut1[7];
  ASSERT_FALSE(uut1 == uut2);

  uut1[7] = !uut1[7];
  uut1[8] = !uut1[8];
  ASSERT_FALSE(uut1 == uut2);

  uut1[8] = !uut1[8];
  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_15Bits)
{
  uint8_t const data[] = { 0xA5U, 0x01U };
  BitField uut1(15, data);
  BitField uut2(15, data);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];
  ASSERT_FALSE(uut1 == uut2);

  uut1[0] = !uut1[0];
  uut1[7] = !uut1[7];
  ASSERT_FALSE(uut1 == uut2);

  uut1[7] = !uut1[7];
  uut1[8] = !uut1[8];
  ASSERT_FALSE(uut1 == uut2);

  uut1[8] = !uut1[8];
  uut1[9] = !uut1[9];
  ASSERT_FALSE(uut1 == uut2);

  uut1[9] = !uut1[9];
  uut1[14] = !uut1[14];
  ASSERT_FALSE(uut1 == uut2);

  uut1[14] = !uut1[14];
  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, Operator_Equal_SameSize_16Bits)
{
  uint8_t const data[] = { 0xA5U, 0x01U };
  BitField uut1(16, data);
  BitField uut2(16, data);

  ASSERT_TRUE(uut1 == uut2);

  uut1[0] = !uut1[0];
  ASSERT_FALSE(uut1 == uut2);

  uut1[0] = !uut1[0];
  uut1[7] = !uut1[7];
  ASSERT_FALSE(uut1 == uut2);

  uut1[7] = !uut1[7];
  uut1[8] = !uut1[8];
  ASSERT_FALSE(uut1 == uut2);

  uut1[8] = !uut1[8];
  uut1[9] = !uut1[9];
  ASSERT_FALSE(uut1 == uut2);

  uut1[9] = !uut1[9];
  uut1[15] = !uut1[15];
  ASSERT_FALSE(uut1 == uut2);

  uut1[15] = !uut1[15];
  ASSERT_TRUE(uut1 == uut2);
}
TEST(gpcc_container_BitField_Tests, BitProxy_read)
{
  uint8_t const data[] = { 0x72, 0xA6 };
  BitField uut1(16, data);

  ASSERT_FALSE(uut1[0]);
  ASSERT_TRUE(uut1[1]);
  ASSERT_TRUE(uut1[15]);

  BitField const uut2(16, data);

  ASSERT_FALSE(uut2[0]);
  ASSERT_TRUE(uut2[1]);
  ASSERT_TRUE(uut2[15]);
}
TEST(gpcc_container_BitField_Tests, BitProxy_assign)
{
  uint8_t data[] = { 0x72, 0xA6 };
  BitField uut(16, data);

  ASSERT_FALSE(uut[0]);
  ASSERT_TRUE(uut[1]);
  ASSERT_TRUE(uut[15]);

  uut[0] = true;
  uut[1] = false;
  uut[15] = false;

  data[0] = 0x71;
  data[1] = 0x26;

  ASSERT_TRUE(TestBits(uut, 16, data));
}
TEST(gpcc_container_BitField_Tests, BitProxy_assign_from_other_BitProxy)
{
  uint8_t data[] = { 0x72, 0xA6 };
  BitField uut1(16, data);
  BitField uut2(16);

  uut2[15] = uut1[1];

  data[0] = 0x00;
  data[1] = 0x80;
  ASSERT_TRUE(TestBits(uut2, 16, data));
}
TEST(gpcc_container_BitField_Tests, BitProxy_BadIndex)
{
  uint8_t const data[] = { 0x72, 0xA6 };
  BitField uut1(16, data);

  bool b;
  ASSERT_THROW(b = uut1[16], std::out_of_range);

  BitField const uut2(16, data);

  ASSERT_THROW(b = uut2[16], std::out_of_range);

  (void)b;
}
TEST(gpcc_container_BitField_Tests, Resize_NoChange)
{
  uint8_t const data[] = { 0x72, 0xA6 };
  BitField uut(16, data);
  uut.Resize(16);
  ASSERT_TRUE(TestBits(uut, 16, data));
}
TEST(gpcc_container_BitField_Tests, Resize_NewSizeZero)
{
  uint8_t data[2] = { 0xFF, 0xFF };

  BitField uut(16, data);
  ASSERT_TRUE(TestBits(uut, 16, data));

  // Resize to zero length.
  uut.Resize(0);
  ASSERT_EQ(0U, uut.GetSize());

  // Resize up again. All bits must be zero.
  uut.Resize(16);
  data[0] = 0;
  data[1] = 0;
  ASSERT_TRUE(TestBits(uut, 16, data));
}
#ifndef SKIP_VERYBIGMEM_TESTS
TEST(gpcc_container_BitField_Tests, Resize_NewSizeMax)
{
  size_t const max = std::numeric_limits<size_t>::max() - (BitField::storage_t_size_in_bit - 1U);

  uint8_t const data[] = { 0x72, 0xA6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  BitField uut(16, data);

  // Try to grow beyond max. Exception must be thrown. Due to strong guarantee, no change must happen to uut.
  ASSERT_THROW(uut.Resize(max + 1U), std::length_error);
  ASSERT_TRUE(TestBits(uut, 16, data));

  // Try to grow to max. Will likely fail with std::bad_alloc.
  try
  {
    uut.Resize(max);
    ASSERT_EQ(max, uut.GetSize());
    ASSERT_TRUE(TestBitsIncomplete(uut, 64, data));
  }
  catch (std::length_error&)
  {
    // this should not happen
    ASSERT_FALSE(true);
  }
  catch (std::bad_alloc const &)
  {
    // Resize failed. Due to strong guarantee, no change must happen to uut.
    ASSERT_TRUE(TestBits(uut, 16, data));
  }
}
#endif
TEST(gpcc_container_BitField_Tests, Resize_ZeroUpperBitsOnEnlarge)
{
  uint8_t data[] = { 0x72, 0xA6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  BitField uut1(16, data);

  // Grow to 32 bits. Upper bits must be zero.
  uut1.Resize(32);
  ASSERT_TRUE(TestBits(uut1, 32, data));

  // Shrink to 8.
  uut1.Resize(8);
  ASSERT_TRUE(TestBits(uut1, 8, data));

  // Grow to 16. Upper 8 bit must be zero.
  uut1.Resize(16);
  data[1] = 0;
  ASSERT_TRUE(TestBits(uut1, 16, data));

  BitField uut2(16, data);

  // Grow to 128 bits. Upper bits must be zero.
  uut2.Resize(128);
  ASSERT_TRUE(TestBits(uut2, 128, data));

  // Shrink to 8.
  uut2.Resize(8);
  ASSERT_TRUE(TestBits(uut2, 8, data));

  // Grow to 16. Upper 8 bit must be zero.
  uut2.Resize(16);
  data[1] = 0;
  ASSERT_TRUE(TestBits(uut2, 16, data));
}
TEST(gpcc_container_BitField_Tests, ClearAll_ZeroLength)
{
  BitField uut;
  uut.ClearAll();
  ASSERT_EQ(0U, uut.GetSize());
}
TEST(gpcc_container_BitField_Tests, ClearAll)
{
  uint8_t  data[16] = { 0x12, 0x82, 0xA6, 0xBC, 0xF7, 0x9C, 0xCD, 0x2B,
                        0x82, 0x28, 0xB6, 0x3D, 0xAB, 0xA5, 0x5A, 0x22 };

  BitField uut(128, data);
  ASSERT_TRUE(TestBits(uut, 128, data));

  memset(data, 0x00, sizeof(data));
  uut.ClearAll();
  ASSERT_TRUE(TestBits(uut, 128, data));
}
TEST(gpcc_container_BitField_Tests, SetAll_ZeroLength)
{
  BitField uut;
  uut.SetAll();
  ASSERT_EQ(0U, uut.GetSize());
}
TEST(gpcc_container_BitField_Tests, SetAll)
{
  uint8_t  data[16] = { 0x12, 0x82, 0xA6, 0xBC, 0xF7, 0x9C, 0xCD, 0x2B,
                        0x82, 0x28, 0xB6, 0x3D, 0xAB, 0xA5, 0x5A, 0x22 };

  BitField uut(128, data);
  ASSERT_TRUE(TestBits(uut, 128, data));

  memset(data, 0xFF, sizeof(data));
  uut.SetAll();
  ASSERT_TRUE(TestBits(uut, 128, data));
}
TEST(gpcc_container_BitField_Tests, SetAllAndEnlarge_UpperBitsNotAffected)
{
  uint8_t  data[16] = { 0x12, 0x82, 0xA6, 0xBC, 0xF7, 0x9C, 0xCD, 0x2B,
                        0x82, 0x28, 0xB6, 0x3D, 0xAB, 0xA5, 0x5A, 0x00 };

  BitField uut(120, data);
  ASSERT_TRUE(TestBits(uut, 120, data));

  memset(data, 0xFF, sizeof(data)-1);
  uut.SetAll();
  ASSERT_TRUE(TestBits(uut, 120, data));

  // Resize up to 128. Reallocation should not be performed. Upper 8 bits must be zero.
  uut.Resize(128);
  ASSERT_TRUE(TestBits(uut, 128, data));
}
TEST(gpcc_container_BitField_Tests, Assign_nullptr)
{
  BitField uut(16);
  ASSERT_THROW(uut.Assign(8, nullptr), std::invalid_argument);
}
TEST(gpcc_container_BitField_Tests, Assign_zero)
{
  uint8_t data = 0x12;

  BitField uut(16);
  uut.Assign(0, &data);

  ASSERT_EQ(0U, uut.GetSize());
}
TEST(gpcc_container_BitField_Tests, Assign_SameSize)
{
  uint8_t data1[2] = { 0x12, 0xA9 };
  uint8_t data2[2] = { 0x77, 0x34 };

  BitField uut(16, data1);
  uut.Assign(16, data2);

  ASSERT_EQ(16U, uut.GetSize());
  ASSERT_TRUE(TestBits(uut, 16, data2));
}
TEST(gpcc_container_BitField_Tests, Assign_OtherSizeNoAlloc)
{
  uint8_t data1[2] = { 0x12, 0xA9 };
  uint8_t data2[2] = { 0x77, 0x34 };

  BitField uut(16, data1);
  uut.Assign(15, data2);

  ASSERT_EQ(15U, uut.GetSize());
  ASSERT_TRUE(TestBits(uut, 15, data2));
}
TEST(gpcc_container_BitField_Tests, Assign_OtherSize)
{
  uint8_t data1[2] = { 0x12, 0xA9 };
  uint8_t data2[16] = { 0x77, 0x34, 0xBF, 0xA7, 0x99, 0xAF, 0x12, 0x29,
                        0xC5, 0xDB, 0x8A, 0x81, 0x1D, 0xF1, 0xC3, 0x5A};

  BitField uut(16, data1);
  uut.Assign(128, data2);

  ASSERT_EQ(128U, uut.GetSize());
  ASSERT_TRUE(TestBits(uut, 128, data2));
}
TEST(gpcc_container_BitField_Tests, ClrSetWriteGet)
{
  uint8_t data[2];

  BitField uut(16);

  // set bits
  uut.SetBit(3);
  uut.SetBit(15);
  data[0] = 0x08;
  data[1] = 0x80;
  ASSERT_TRUE(TestBits(uut, 16, data));

  // clear bits
  uut.SetAll();
  uut.ClearBit(0);
  uut.ClearBit(7);
  uut.ClearBit(8);
  uut.ClearBit(9);
  uut.ClearBit(15);
  data[0] = 0x7E;
  data[1] = 0x7C;
  ASSERT_TRUE(TestBits(uut, 16, data));

  // set and clear bits that are already set/cleared
  uut.ClearBit(0);
  uut.SetBit(1);
  ASSERT_TRUE(TestBits(uut, 16, data));

  // write bits
  uut.WriteBit(0, false);
  uut.WriteBit(1, true);
  uut.WriteBit(2, false);
  uut.WriteBit(7, true);
  data[0] = 0xFA;
  data[1] = 0x7C;
  ASSERT_TRUE(TestBits(uut, 16, data));

  // note: uut.GetBit has been excessively stressed by TestBits() among this and other test cases
}
TEST(gpcc_container_BitField_Tests, ClrSetWriteGet_OutOfRange)
{
  uint8_t data[2] = { 0x12, 0xB5 };

  BitField uut(16, data);

  // set bits
  ASSERT_THROW(uut.SetBit(16), std::out_of_range);
  ASSERT_TRUE(TestBits(uut, 16, data));

  // clear bits
  ASSERT_THROW(uut.ClearBit(16), std::out_of_range);
  ASSERT_TRUE(TestBits(uut, 16, data));

  // write bit
  ASSERT_THROW(uut.WriteBit(16, true), std::out_of_range);
  ASSERT_TRUE(TestBits(uut, 16, data));
  ASSERT_THROW(uut.WriteBit(16, false), std::out_of_range);
  ASSERT_TRUE(TestBits(uut, 16, data));

  // get bit
  bool b;
  ASSERT_THROW(b = uut.GetBit(16), std::out_of_range);
  (void)b;
}
TEST(gpcc_container_BitField_Tests, FindFirstSetBit)
{
  size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x81, 0x00, 0x01, 0x80, 0x01, 0x40, 0x03, 0xC0 };

  BitField uut(64, data);

  ASSERT_EQ(0U,     uut.FindFirstSetBit(0));
  ASSERT_EQ(7U,     uut.FindFirstSetBit(1));
  ASSERT_EQ(16U,    uut.FindFirstSetBit(8));
  ASSERT_EQ(31U,    uut.FindFirstSetBit(17));
  ASSERT_EQ(32U,    uut.FindFirstSetBit(32));
  ASSERT_EQ(46U,    uut.FindFirstSetBit(33));
  ASSERT_EQ(48U,    uut.FindFirstSetBit(47));
  ASSERT_EQ(49U,    uut.FindFirstSetBit(49));
  ASSERT_EQ(62U,    uut.FindFirstSetBit(50));
  ASSERT_EQ(63U,    uut.FindFirstSetBit(63));
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBit(64));
}
TEST(gpcc_container_BitField_Tests, FindFirstSetBit_UnusedTopBitsIgnored)
{
  size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x81, 0x00, 0x01, 0x80, 0x01, 0x40, 0x03, 0xC0 };

  BitField uut(60, data);

  ASSERT_EQ(0U,     uut.FindFirstSetBit(0));
  ASSERT_EQ(7U,     uut.FindFirstSetBit(1));
  ASSERT_EQ(16U,    uut.FindFirstSetBit(8));
  ASSERT_EQ(31U,    uut.FindFirstSetBit(17));
  ASSERT_EQ(32U,    uut.FindFirstSetBit(32));
  ASSERT_EQ(46U,    uut.FindFirstSetBit(33));
  ASSERT_EQ(48U,    uut.FindFirstSetBit(47));
  ASSERT_EQ(49U,    uut.FindFirstSetBit(49));
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBit(50));
}
TEST(gpcc_container_BitField_Tests, FindFirstSetBit_MaxStartIndex)
{
  size_t const NO_BIT = BitField::NO_BIT;

  BitField uut;
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBit(std::numeric_limits<size_t>::max()));
}
TEST(gpcc_container_BitField_Tests, FindFirstClearedBit)
{
  size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x7E, 0xFF, 0xFE, 0x7F, 0xFE, 0xBF, 0xFC, 0x3F };

  BitField uut(64, data);

  ASSERT_EQ(0U,     uut.FindFirstClearedBit(0));
  ASSERT_EQ(7U,     uut.FindFirstClearedBit(1));
  ASSERT_EQ(16U,    uut.FindFirstClearedBit(8));
  ASSERT_EQ(31U,    uut.FindFirstClearedBit(17));
  ASSERT_EQ(32U,    uut.FindFirstClearedBit(32));
  ASSERT_EQ(46U,    uut.FindFirstClearedBit(33));
  ASSERT_EQ(48U,    uut.FindFirstClearedBit(47));
  ASSERT_EQ(49U,    uut.FindFirstClearedBit(49));
  ASSERT_EQ(62U,    uut.FindFirstClearedBit(50));
  ASSERT_EQ(63U,    uut.FindFirstClearedBit(63));
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBit(64));
}
TEST(gpcc_container_BitField_Tests, FindFirstClearedBit_UnusedTopBitsIgnored)
{
  size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x7E, 0xFF, 0xFE, 0x7F, 0xFE, 0xBF, 0xFC, 0x3F };

  BitField uut(60, data);

  ASSERT_EQ(0U,     uut.FindFirstClearedBit(0));
  ASSERT_EQ(7U,     uut.FindFirstClearedBit(1));
  ASSERT_EQ(16U,    uut.FindFirstClearedBit(8));
  ASSERT_EQ(31U,    uut.FindFirstClearedBit(17));
  ASSERT_EQ(32U,    uut.FindFirstClearedBit(32));
  ASSERT_EQ(46U,    uut.FindFirstClearedBit(33));
  ASSERT_EQ(48U,    uut.FindFirstClearedBit(47));
  ASSERT_EQ(49U,    uut.FindFirstClearedBit(49));
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBit(50));
}
TEST(gpcc_container_BitField_Tests, FindFirstClearedBit_MaxStartIndex)
{
  size_t const NO_BIT = BitField::NO_BIT;

  BitField uut;
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBit(std::numeric_limits<size_t>::max()));
}
TEST(gpcc_container_BitField_Tests, FindFirstSetBitReverse)
{
  size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x81, 0x00, 0x01, 0x80, 0x01, 0x40, 0x03, 0xC0 };

  BitField uut(64, data);
  ASSERT_EQ(63U,    uut.FindFirstSetBitReverse(63));
  ASSERT_EQ(62U,    uut.FindFirstSetBitReverse(62));
  ASSERT_EQ(49U,    uut.FindFirstSetBitReverse(61));
  ASSERT_EQ(48U,    uut.FindFirstSetBitReverse(48));
  ASSERT_EQ(46U,    uut.FindFirstSetBitReverse(47));
  ASSERT_EQ(32U,    uut.FindFirstSetBitReverse(45));
  ASSERT_EQ(31U,    uut.FindFirstSetBitReverse(31));
  ASSERT_EQ(16U,    uut.FindFirstSetBitReverse(30));
  ASSERT_EQ(7U,     uut.FindFirstSetBitReverse(15));
  ASSERT_EQ(0U,     uut.FindFirstSetBitReverse(6));

  ASSERT_EQ(0U,     uut.FindFirstSetBitReverse(0));

  uut[0] = false;
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBitReverse(0));
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBitReverse(6));

  ASSERT_EQ(63U,    uut.FindFirstSetBitReverse(64));

  uut.ClearAll();
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBitReverse(63));
}
TEST(gpcc_container_BitField_Tests, FindFirstSetBitReverse_UnusedTopBitsIgnored)
{
  //size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x81, 0x00, 0x01, 0x80, 0x01, 0x40, 0x03, 0xC0 };

  BitField uut(60, data);
  ASSERT_EQ(49U,    uut.FindFirstSetBitReverse(59));
  ASSERT_EQ(48U,    uut.FindFirstSetBitReverse(48));
  ASSERT_EQ(46U,    uut.FindFirstSetBitReverse(47));
  ASSERT_EQ(32U,    uut.FindFirstSetBitReverse(45));
  ASSERT_EQ(31U,    uut.FindFirstSetBitReverse(31));
  ASSERT_EQ(16U,    uut.FindFirstSetBitReverse(30));
  ASSERT_EQ(7U,     uut.FindFirstSetBitReverse(15));
  ASSERT_EQ(0U,     uut.FindFirstSetBitReverse(6));
}
TEST(gpcc_container_BitField_Tests, FindFirstSetBitReverse_MaxStartIndex)
{
  size_t const NO_BIT = BitField::NO_BIT;

  BitField uut;
  ASSERT_EQ(NO_BIT, uut.FindFirstSetBitReverse(std::numeric_limits<size_t>::max()));
}
TEST(gpcc_container_BitField_Tests, FindFirstClearedBitReversed)
{
  size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x7E, 0xFF, 0xFE, 0x7F, 0xFE, 0xBF, 0xFC, 0x3F };

  BitField uut(64, data);
  ASSERT_EQ(63U,    uut.FindFirstClearedBitReverse(63));
  ASSERT_EQ(62U,    uut.FindFirstClearedBitReverse(62));
  ASSERT_EQ(49U,    uut.FindFirstClearedBitReverse(61));
  ASSERT_EQ(48U,    uut.FindFirstClearedBitReverse(48));
  ASSERT_EQ(46U,    uut.FindFirstClearedBitReverse(47));
  ASSERT_EQ(32U,    uut.FindFirstClearedBitReverse(45));
  ASSERT_EQ(31U,    uut.FindFirstClearedBitReverse(31));
  ASSERT_EQ(16U,    uut.FindFirstClearedBitReverse(30));
  ASSERT_EQ(7U,     uut.FindFirstClearedBitReverse(15));
  ASSERT_EQ(0U,     uut.FindFirstClearedBitReverse(6));

  ASSERT_EQ(0U,     uut.FindFirstClearedBitReverse(0));

  uut[0] = true;
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBitReverse(0));
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBitReverse(6));

  ASSERT_EQ(63U,    uut.FindFirstClearedBitReverse(64));

  uut.SetAll();
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBitReverse(63));
}
TEST(gpcc_container_BitField_Tests, FindFirstClearedBitReversed_UnusedTopBitsIgnored)
{
  // size_t const NO_BIT = BitField::NO_BIT;

  uint8_t  data[8] = { 0x7E, 0xFF, 0xFE, 0x7F, 0xFE, 0xBF, 0xFC, 0x3F };

  BitField uut(60, data);
  ASSERT_EQ(49U,    uut.FindFirstClearedBitReverse(59));
  ASSERT_EQ(48U,    uut.FindFirstClearedBitReverse(48));
  ASSERT_EQ(46U,    uut.FindFirstClearedBitReverse(47));
  ASSERT_EQ(32U,    uut.FindFirstClearedBitReverse(45));
  ASSERT_EQ(31U,    uut.FindFirstClearedBitReverse(31));
  ASSERT_EQ(16U,    uut.FindFirstClearedBitReverse(30));
  ASSERT_EQ(7U,     uut.FindFirstClearedBitReverse(15));
  ASSERT_EQ(0U,     uut.FindFirstClearedBitReverse(6));

  ASSERT_EQ(49U,    uut.FindFirstClearedBitReverse(63));
}
TEST(gpcc_container_BitField_Tests, FindFirstClearedBitReversed_MaxStartIndex)
{
  size_t const NO_BIT = BitField::NO_BIT;

  BitField uut;
  ASSERT_EQ(NO_BIT, uut.FindFirstClearedBitReverse(std::numeric_limits<size_t>::max()));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_1_typical)
{
  uint8_t data[2] = { 0x21, 0x8E };

  BitField uut(16, data);

  std::string str(uut.EnumerateBits(true));
  ASSERT_EQ(0, str.compare("0, 5, 9, 10, 11, 15"));

  uut.Resize(15);
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare("0, 5, 9, 10, 11"));

  uut.Resize(16);
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare("0, 5, 9, 10, 11"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_0_typical)
{
  uint8_t data[2] = { 0x21, 0x8E };

  BitField uut(16, data);

  std::string str(uut.EnumerateBits(false));
  ASSERT_EQ(0, str.compare("1, 2, 3, 4, 6, 7, 8, 12, 13, 14"));

  uut.Resize(14);
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare("1, 2, 3, 4, 6, 7, 8, 12, 13"));

  uut.Resize(16);
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare("1, 2, 3, 4, 6, 7, 8, 12, 13, 14, 15"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_0_NoMatch)
{
  uint8_t data[2] = { 0x00, 0x00 };

  BitField uut(16, data);

  std::string str(uut.EnumerateBits(true));

  ASSERT_EQ(0, str.compare(""));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_1_NoMatch)
{
  uint8_t data[2] = { 0xFF, 0xFF };

  BitField uut(16, data);

  std::string str(uut.EnumerateBits(false));

  ASSERT_EQ(0, str.compare(""));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_1_Length)
{
  BitField uut;

  // zero length
  std::string str(uut.EnumerateBits(true));
  ASSERT_EQ(0, str.compare(""));

  // length 1
  uut.Resize(1);
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare(""));
  uut.SetBit(0);
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare("0"));

  // length 2
  uut.Resize(2);
  uut.ClearAll();
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare(""));
  uut.SetBit(0);
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare("0"));
  uut.SetBit(1);
  str = uut.EnumerateBits(true);
  ASSERT_EQ(0, str.compare("0, 1"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_0_Length)
{
  BitField uut;

  // zero length
  std::string str(uut.EnumerateBits(false));
  ASSERT_EQ(0, str.compare(""));

  // length 1
  uut.Resize(1);
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare("0"));
  uut.SetBit(0);
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare(""));

  // length 2
  uut.Resize(2);
  uut.SetAll();
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare(""));
  uut.ClearBit(0);
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare("0"));
  uut.ClearBit(1);
  str = uut.EnumerateBits(false);
  ASSERT_EQ(0, str.compare("0, 1"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBits_NoWhitespaces)
{
  uint8_t data[2] = { 0x21, 0x8E };

  BitField uut(16, data);

  std::string str(uut.EnumerateBits(true, true));
  ASSERT_EQ(0, str.compare("0,5,9,10,11,15"));

  uut.ClearAll();
  str = uut.EnumerateBits(true, true);
  ASSERT_EQ(0, str.compare(""));

  uut[0] = true;
  str = uut.EnumerateBits(true, true);
  ASSERT_EQ(0, str.compare("0"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_1_typical)
{
  uint8_t data[2] = { 0x21, 0x8E };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(true));
  ASSERT_EQ(0, str.compare("0, 5, 9-11, 15"));

  uut.Resize(14);
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare("0, 5, 9-11"));

  uut.Resize(16);
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare("0, 5, 9-11"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_0_typical)
{
  uint8_t data[2] = { 0x21, 0x8E };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(false));
  ASSERT_EQ(0, str.compare("1-4, 6-8, 12-14"));

  uut.Resize(14);
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare("1-4, 6-8, 12-13"));

  uut.Resize(16);
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare("1-4, 6-8, 12-15"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_0_NoMatch)
{
  uint8_t data[2] = { 0x00, 0x00 };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(true));

  ASSERT_EQ(0, str.compare(""));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_1_NoMatch)
{
  uint8_t data[2] = { 0xFF, 0xFF };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(false));

  ASSERT_EQ(0, str.compare(""));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_1_all)
{
  uint8_t data[2] = { 0xFF, 0xFF };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(true));

  ASSERT_EQ(0, str.compare("0-15"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_0_all)
{
  uint8_t data[2] = { 0x00, 0x00 };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(false));

  ASSERT_EQ(0, str.compare("0-15"));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_1_Length)
{
  BitField uut;

  // zero length
  std::string str(uut.EnumerateBitsCompressed(true));
  ASSERT_EQ(0, str.compare(""));

  // length 1
  uut.Resize(1);
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare(""));
  uut.SetBit(0);
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare("0"));

  // length 2
  uut.Resize(2);
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare("0"));
  uut.SetBit(1);
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare("0-1"));
  uut.ClearAll();
  str = uut.EnumerateBitsCompressed(true);
  ASSERT_EQ(0, str.compare(""));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_0_Length)
{
  BitField uut;

  // zero length
  std::string str(uut.EnumerateBitsCompressed(false));
  ASSERT_EQ(0, str.compare(""));

  // length 1
  uut.Resize(1);
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare("0"));
  uut.SetBit(0);
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare(""));

  // length 2
  uut.Resize(2);
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare("1"));
  uut.ClearBit(0);
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare("0-1"));
  uut.SetAll();
  str = uut.EnumerateBitsCompressed(false);
  ASSERT_EQ(0, str.compare(""));
}
TEST(gpcc_container_BitField_Tests, EnumerateBitsCompressed_NoWhitespaces)
{
  uint8_t data[2] = { 0x21, 0x8E };

  BitField uut(16, data);

  std::string str(uut.EnumerateBitsCompressed(true, true));
  ASSERT_EQ(0, str.compare("0,5,9-11,15"));

  uut.ClearAll();
  str = uut.EnumerateBitsCompressed(true, true);
  ASSERT_EQ(0, str.compare(""));

  uut[0] = true;
  str = uut.EnumerateBitsCompressed(true, true);
  ASSERT_EQ(0, str.compare("0"));

  uut.SetAll();
  str = uut.EnumerateBitsCompressed(true, true);
  ASSERT_EQ(0, str.compare("0-15"));
}
TEST(gpcc_container_BitField_Tests, AccessInternalStorage_typical)
{
  uint8_t const dataForBf1[] = { 0x23, 0xF8 };
  uint8_t const dataForBf2[] = { 0x32, 0xF9 };
  uint8_t const dataForBf3[] = { 0xF2, 0xA3 };

  BitField const bf1(16, dataForBf1);
  BitField const bf2(16, dataForBf2);
  BitField bf3(16, dataForBf3);

  // get pointers to internal storage of bit fields
  size_t n;
  BitField::storage_t const * pS1 = bf1.GetInternalStorage(&n);
  BitField::storage_t const * pS2 = bf2.GetInternalStorage(nullptr);
  BitField::storage_t       * pS3 = bf3.GetInternalStorage();

  // perform operations
  for (size_t i = 0; i < n; i++)
    pS3[i] &= pS1[i] | pS2[i];

  // check for expected result
  uint8_t const expectedResult[] = { 0x32, 0xA1 };
  ASSERT_TRUE(TestBits(bf1, 16, dataForBf1));
  ASSERT_TRUE(TestBits(bf2, 16, dataForBf2));
  ASSERT_TRUE(TestBits(bf3, 16, expectedResult));
}
TEST(gpcc_container_BitField_Tests, AccessInternalStorage_ZeroLength)
{
  BitField bf1;
  BitField const bf2;

  size_t n;
  BitField::storage_t* const pS1 = bf1.GetInternalStorage(&n);
  ASSERT_TRUE(pS1 == nullptr);
  ASSERT_EQ(0U, n);

  BitField::storage_t const * const pS2 = bf2.GetInternalStorage(&n);
  ASSERT_TRUE(pS2 == nullptr);
  ASSERT_EQ(0U, n);
}
TEST(gpcc_container_BitField_Tests, AccessInternalStorage_UpperBitsClearedOnEnlarge)
{
  BitField uut(8);

  BitField::storage_t* const pS = uut.GetInternalStorage();
  *pS = std::numeric_limits<BitField::storage_t>::max();

  uint8_t const expectedData[] = { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  uut.Resize(128);
  ASSERT_TRUE(TestBits(uut, 128, expectedData));
}

} // namespace container
} // namespace gpcc_tests
