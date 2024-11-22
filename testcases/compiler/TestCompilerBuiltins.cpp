/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#include <gpcc/compiler/builtins.hpp>
#include <gtest/gtest.h>
#include <limits>
#include <cstdint>

namespace gpcc_tests
{
namespace Compiler
{

using gpcc::compiler::OverflowAwareAdd;
using gpcc::compiler::OverflowAwareSub;
using gpcc::compiler::CountLeadingZeros;
using gpcc::compiler::CountLeadingOnes;
using gpcc::compiler::CountTrailingZeros;
using gpcc::compiler::CountTrailingOnes;
using gpcc::compiler::ReverseBits8;
using gpcc::compiler::ReverseBits16;
using gpcc::compiler::ReverseBits32;
using namespace testing;

TEST(GPCC_Compiler_CompilerBuiltins_Tests, OverflowAwareAdd_i64_i64_i64)
{
  int64_t a, b, c;

  // basic stuff
  a = 5;
  b = 10;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(15, c);

  a = -5;
  b = 10;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(5, c);

  a = 5;
  b = -10;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-5, c);

  // bounds (positive)
  a = std::numeric_limits<int64_t>::max();
  b = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::max(), c);

  b = std::numeric_limits<int64_t>::max();
  a = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::max(), c);

  a = std::numeric_limits<int64_t>::max();
  b = 1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int64_t>::max();
  a = 1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::max();
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-1, c);

  b = std::numeric_limits<int64_t>::max();
  a = std::numeric_limits<int64_t>::min();
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-1, c);

  // bounds (negative)
  a = std::numeric_limits<int64_t>::min();
  b = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::min(), c);

  b = std::numeric_limits<int64_t>::min();
  a = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::min(), c);

  a = std::numeric_limits<int64_t>::min();
  b = -1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int64_t>::min();
  a = -1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, OverflowAwareAdd_i64_i64_i32)
{
  int64_t a, b;
  int32_t c;

  // basic stuff
  a = 5;
  b = 10;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(15, c);

  a = -5;
  b = 10;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(5, c);

  a = 5;
  b = -10;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-5, c);

  // 32 bit bounds (positive)
  a = std::numeric_limits<int32_t>::max();
  b = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::max(), c);

  b = std::numeric_limits<int32_t>::max();
  a = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::max(), c);

  a = std::numeric_limits<int32_t>::max();
  b = 1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int32_t>::max();
  a = 1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int32_t>::max();
  b = std::numeric_limits<int32_t>::max();
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int32_t>::max();
  b = std::numeric_limits<int32_t>::min();
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-1, c);

  b = std::numeric_limits<int32_t>::max();
  a = std::numeric_limits<int32_t>::min();
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-1, c);

  // 32 bit bounds (negative)
  a = std::numeric_limits<int32_t>::min();
  b = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::min(), c);

  b = std::numeric_limits<int32_t>::min();
  a = 0;
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::min(), c);

  a = std::numeric_limits<int32_t>::min();
  b = -1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int32_t>::min();
  a = -1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int32_t>::min();
  b = std::numeric_limits<int32_t>::min();
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  // 64 bit bounds (positive)
  a = std::numeric_limits<int64_t>::max();
  b = 0;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int64_t>::max();
  a = 0;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = 1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int64_t>::max();
  a = 1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::max();
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-1, c);

  b = std::numeric_limits<int64_t>::max();
  a = std::numeric_limits<int64_t>::min();
  ASSERT_FALSE(OverflowAwareAdd(a, b, &c));
  ASSERT_EQ(-1, c);

  // 64 bit bounds (negative)
  a = std::numeric_limits<int64_t>::min();
  b = 0;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int64_t>::min();
  a = 0;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = -1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  b = std::numeric_limits<int64_t>::min();
  a = -1;
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_TRUE(OverflowAwareAdd(a, b, &c));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, OverflowAwareSub_i64_i64_i64)
{
  int64_t a, b, c;

  // basic stuff
  a = 5;
  b = 10;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(-5, c);

  a = -5;
  b = 10;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(-15, c);

  a = 5;
  b = -10;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(15, c);

  // bounds (positive)
  a = std::numeric_limits<int64_t>::max();
  b = 1;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::max() - 1, c);

  a = std::numeric_limits<int64_t>::max();
  b = 0;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::max(), c);

  a = std::numeric_limits<int64_t>::max();
  b = -1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::max();
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(0, c);

  // bounds (negative)
  a = std::numeric_limits<int64_t>::min();
  b = -1;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::min() + 1, c);

  a = std::numeric_limits<int64_t>::min();
  b = 0;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int64_t>::min(), c);

  a = std::numeric_limits<int64_t>::min();
  b = 1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = std::numeric_limits<int64_t>::max();
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(0, c);
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, OverflowAwareSub_i64_i64_i32)
{
  int64_t a, b;
  int32_t c;

  // basic stuff
  a = 5;
  b = 10;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(-5, c);

  a = -5;
  b = 10;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(-15, c);

  a = 5;
  b = -10;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(15, c);

  // 32 bit bounds (positive)
  a = std::numeric_limits<int32_t>::max();
  b = 1;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::max() - 1, c);

  a = std::numeric_limits<int32_t>::max();
  b = 0;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::max(), c);

  a = std::numeric_limits<int32_t>::max();
  b = -1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int32_t>::max();
  b = std::numeric_limits<int32_t>::max();
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(0, c);

  a = std::numeric_limits<int32_t>::max();
  b = std::numeric_limits<int32_t>::min();
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  // 32 bit bounds (negative)
  a = std::numeric_limits<int32_t>::min();
  b = -1;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::min() + 1, c);

  a = std::numeric_limits<int32_t>::min();
  b = 0;
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(std::numeric_limits<int32_t>::min(), c);

  a = std::numeric_limits<int32_t>::min();
  b = 1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int32_t>::min();
  b = std::numeric_limits<int32_t>::min();
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(0, c);

  a = std::numeric_limits<int32_t>::min();
  b = std::numeric_limits<int32_t>::max();
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  // 64 bit bounds (positive)
  a = std::numeric_limits<int64_t>::max();
  b = -1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = 0;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = 1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::max();
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(0, c);

  a = std::numeric_limits<int64_t>::max();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  // 64 bit bounds (negative)
  a = std::numeric_limits<int64_t>::min();
  b = 1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = 0;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = -1;
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));

  a = std::numeric_limits<int64_t>::min();
  b = std::numeric_limits<int64_t>::min();
  ASSERT_FALSE(OverflowAwareSub(a, b, &c));
  ASSERT_EQ(0, c);

  a = std::numeric_limits<int64_t>::min();
  b = std::numeric_limits<int64_t>::max();
  ASSERT_TRUE(OverflowAwareSub(a, b, &c));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingZeros_uchar)
{
  EXPECT_EQ(8,     CountLeadingZeros(static_cast<unsigned char>(0x00U)));
  EXPECT_EQ(7,     CountLeadingZeros(static_cast<unsigned char>(0x01U)));
  EXPECT_EQ(6,     CountLeadingZeros(static_cast<unsigned char>(0x02U)));
  EXPECT_EQ(4,     CountLeadingZeros(static_cast<unsigned char>(0x08U)));
  EXPECT_EQ(1,     CountLeadingZeros(static_cast<unsigned char>(0x40U)));
  EXPECT_EQ(0,     CountLeadingZeros(static_cast<unsigned char>(0x81U)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingZeros_u8)
{
  EXPECT_EQ(8,     CountLeadingZeros(static_cast<uint8_t>(0x00U)));
  EXPECT_EQ(7,     CountLeadingZeros(static_cast<uint8_t>(0x01U)));
  EXPECT_EQ(6,     CountLeadingZeros(static_cast<uint8_t>(0x02U)));
  EXPECT_EQ(4,     CountLeadingZeros(static_cast<uint8_t>(0x08U)));
  EXPECT_EQ(1,     CountLeadingZeros(static_cast<uint8_t>(0x40U)));
  EXPECT_EQ(0,     CountLeadingZeros(static_cast<uint8_t>(0x81U)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingZeros_u16)
{
  EXPECT_EQ(16,     CountLeadingZeros(static_cast<uint16_t>(0x0000U)));
  EXPECT_EQ(15,     CountLeadingZeros(static_cast<uint16_t>(0x0001U)));
  EXPECT_EQ(14,     CountLeadingZeros(static_cast<uint16_t>(0x0002U)));
  EXPECT_EQ(12,     CountLeadingZeros(static_cast<uint16_t>(0x0008U)));
  EXPECT_EQ( 1,     CountLeadingZeros(static_cast<uint16_t>(0x4001U)));
  EXPECT_EQ( 0,     CountLeadingZeros(static_cast<uint16_t>(0x8000U)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingZeros_u32)
{
  EXPECT_EQ(32,     CountLeadingZeros(static_cast<uint32_t>(0x00000000UL)));
  EXPECT_EQ(31,     CountLeadingZeros(static_cast<uint32_t>(0x00000001UL)));
  EXPECT_EQ(30,     CountLeadingZeros(static_cast<uint32_t>(0x00000002UL)));
  EXPECT_EQ(28,     CountLeadingZeros(static_cast<uint32_t>(0x00000008UL)));
  EXPECT_EQ( 1,     CountLeadingZeros(static_cast<uint32_t>(0x40000000UL)));
  EXPECT_EQ( 0,     CountLeadingZeros(static_cast<uint32_t>(0x80000000UL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingZeros_u64)
{
  EXPECT_EQ(64,     CountLeadingZeros(static_cast<uint64_t>(0x0000000000000000ULL)));
  EXPECT_EQ(63,     CountLeadingZeros(static_cast<uint64_t>(0x0000000000000001ULL)));
  EXPECT_EQ(62,     CountLeadingZeros(static_cast<uint64_t>(0x0000000000000002ULL)));
  EXPECT_EQ(60,     CountLeadingZeros(static_cast<uint64_t>(0x000000000000000FULL)));
  EXPECT_EQ( 1,     CountLeadingZeros(static_cast<uint64_t>(0x4000000000000000ULL)));
  EXPECT_EQ( 0,     CountLeadingZeros(static_cast<uint64_t>(0x8000000000000000ULL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingOnes_uchar)
{
  EXPECT_EQ(8,     CountLeadingOnes(static_cast<unsigned char>(0xFFU)));
  EXPECT_EQ(7,     CountLeadingOnes(static_cast<unsigned char>(0xFEU)));
  EXPECT_EQ(6,     CountLeadingOnes(static_cast<unsigned char>(0xFDU)));
  EXPECT_EQ(4,     CountLeadingOnes(static_cast<unsigned char>(0xF7U)));
  EXPECT_EQ(1,     CountLeadingOnes(static_cast<unsigned char>(0xBFU)));
  EXPECT_EQ(0,     CountLeadingOnes(static_cast<unsigned char>(0x7FU)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingOnes_u8)
{
  EXPECT_EQ(8,     CountLeadingOnes(static_cast<uint8_t>(0xFFU)));
  EXPECT_EQ(7,     CountLeadingOnes(static_cast<uint8_t>(0xFEU)));
  EXPECT_EQ(6,     CountLeadingOnes(static_cast<uint8_t>(0xFDU)));
  EXPECT_EQ(4,     CountLeadingOnes(static_cast<uint8_t>(0xF7U)));
  EXPECT_EQ(1,     CountLeadingOnes(static_cast<uint8_t>(0xBFU)));
  EXPECT_EQ(0,     CountLeadingOnes(static_cast<uint8_t>(0x7FU)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingOnes_u16)
{
  EXPECT_EQ(16,     CountLeadingOnes(static_cast<uint16_t>(0xFFFFU)));
  EXPECT_EQ(15,     CountLeadingOnes(static_cast<uint16_t>(0xFFFEU)));
  EXPECT_EQ(14,     CountLeadingOnes(static_cast<uint16_t>(0xFFFDU)));
  EXPECT_EQ(12,     CountLeadingOnes(static_cast<uint16_t>(0xFFF7U)));
  EXPECT_EQ( 1,     CountLeadingOnes(static_cast<uint16_t>(0xBFFFU)));
  EXPECT_EQ( 0,     CountLeadingOnes(static_cast<uint16_t>(0x7FFFU)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingOnes_u32)
{
  EXPECT_EQ(32,     CountLeadingOnes(static_cast<uint32_t>(0xFFFFFFFFUL)));
  EXPECT_EQ(31,     CountLeadingOnes(static_cast<uint32_t>(0xFFFFFFFEUL)));
  EXPECT_EQ(30,     CountLeadingOnes(static_cast<uint32_t>(0xFFFFFFFDUL)));
  EXPECT_EQ(28,     CountLeadingOnes(static_cast<uint32_t>(0xFFFFFFF0UL)));
  EXPECT_EQ( 1,     CountLeadingOnes(static_cast<uint32_t>(0xBFFFFFFFUL)));
  EXPECT_EQ( 0,     CountLeadingOnes(static_cast<uint32_t>(0x7FFFFFFFUL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingOnes_u64)
{
  EXPECT_EQ(64,     CountLeadingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFULL)));
  EXPECT_EQ(63,     CountLeadingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFEULL)));
  EXPECT_EQ(62,     CountLeadingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFDULL)));
  EXPECT_EQ(60,     CountLeadingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFFF0ULL)));
  EXPECT_EQ( 1,     CountLeadingOnes(static_cast<uint64_t>(0xBFFFFFFFFFFFFFFFULL)));
  EXPECT_EQ( 0,     CountLeadingOnes(static_cast<uint64_t>(0x7FFFFFFFFFFFFFFFULL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingZeros_uchar)
{
  ASSERT_EQ(8, CountTrailingZeros(static_cast<unsigned char>(0x00U)));
  ASSERT_EQ(7, CountTrailingZeros(static_cast<unsigned char>(0x80U)));
  ASSERT_EQ(4, CountTrailingZeros(static_cast<unsigned char>(0x70U)));
  ASSERT_EQ(1, CountTrailingZeros(static_cast<unsigned char>(0x8EU)));
  ASSERT_EQ(0, CountTrailingZeros(static_cast<unsigned char>(0xFFU)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingZeros_u8)
{
  ASSERT_EQ(8, CountTrailingZeros(static_cast<uint8_t>(0x00U)));
  ASSERT_EQ(7, CountTrailingZeros(static_cast<uint8_t>(0x80U)));
  ASSERT_EQ(4, CountTrailingZeros(static_cast<uint8_t>(0x70U)));
  ASSERT_EQ(1, CountTrailingZeros(static_cast<uint8_t>(0x8EU)));
  ASSERT_EQ(0, CountTrailingZeros(static_cast<uint8_t>(0xFFU)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingZeros_u16)
{
  ASSERT_EQ(16, CountTrailingZeros(static_cast<uint16_t>(0x0000U)));
  ASSERT_EQ(15, CountTrailingZeros(static_cast<uint16_t>(0x8000U)));
  ASSERT_EQ(4,  CountTrailingZeros(static_cast<uint16_t>(0x00F0U)));
  ASSERT_EQ(1,  CountTrailingZeros(static_cast<uint16_t>(0xEFFEU)));
  ASSERT_EQ(0,  CountTrailingZeros(static_cast<uint16_t>(0xFFFFU)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingZeros_u32)
{
  ASSERT_EQ(32, CountTrailingZeros(static_cast<uint32_t>(0x00000000UL)));
  ASSERT_EQ(31, CountTrailingZeros(static_cast<uint32_t>(0x80000000UL)));
  ASSERT_EQ(4,  CountTrailingZeros(static_cast<uint32_t>(0x0F0000F0UL)));
  ASSERT_EQ(1,  CountTrailingZeros(static_cast<uint32_t>(0xF0FFFFFEUL)));
  ASSERT_EQ(0,  CountTrailingZeros(static_cast<uint32_t>(0xFFFFFFFFUL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingZeros_u64)
{
  ASSERT_EQ(64, CountTrailingZeros(static_cast<uint64_t>(0x0000000000000000ULL)));
  ASSERT_EQ(63, CountTrailingZeros(static_cast<uint64_t>(0x8000000000000000ULL)));
  ASSERT_EQ(4,  CountTrailingZeros(static_cast<uint64_t>(0x000F0000000000F0ULL)));
  ASSERT_EQ(1,  CountTrailingZeros(static_cast<uint64_t>(0xFFF0FFFFFFFFFFFEULL)));
  ASSERT_EQ(0,  CountTrailingZeros(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFULL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingOnes_uchar)
{
  ASSERT_EQ(8, CountTrailingOnes(static_cast<unsigned char>(0xFFU)));
  ASSERT_EQ(7, CountTrailingOnes(static_cast<unsigned char>(0x7FU)));
  ASSERT_EQ(4, CountTrailingOnes(static_cast<unsigned char>(0x8FU)));
  ASSERT_EQ(1, CountTrailingOnes(static_cast<unsigned char>(0x11U)));
  ASSERT_EQ(0, CountTrailingOnes(static_cast<unsigned char>(0x00U)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingOnes_u8)
{
  ASSERT_EQ(8, CountTrailingOnes(static_cast<uint8_t>(0xFFU)));
  ASSERT_EQ(7, CountTrailingOnes(static_cast<uint8_t>(0x7FU)));
  ASSERT_EQ(4, CountTrailingOnes(static_cast<uint8_t>(0x8FU)));
  ASSERT_EQ(1, CountTrailingOnes(static_cast<uint8_t>(0x11U)));
  ASSERT_EQ(0, CountTrailingOnes(static_cast<uint8_t>(0x00U)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingOnes_u16)
{
  ASSERT_EQ(16, CountTrailingOnes(static_cast<uint16_t>(0xFFFFU)));
  ASSERT_EQ(15, CountTrailingOnes(static_cast<uint16_t>(0x7FFFU)));
  ASSERT_EQ(4,  CountTrailingOnes(static_cast<uint16_t>(0xFF0FU)));
  ASSERT_EQ(1,  CountTrailingOnes(static_cast<uint16_t>(0xFFF1U)));
  ASSERT_EQ(0,  CountTrailingOnes(static_cast<uint16_t>(0x0000U)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingOnes_u32)
{
  ASSERT_EQ(32, CountTrailingOnes(static_cast<uint32_t>(0xFFFFFFFFUL)));
  ASSERT_EQ(31, CountTrailingOnes(static_cast<uint32_t>(0x7FFFFFFFUL)));
  ASSERT_EQ(4,  CountTrailingOnes(static_cast<uint32_t>(0xFFFFFF0FUL)));
  ASSERT_EQ(1,  CountTrailingOnes(static_cast<uint32_t>(0xFFFFFFF1UL)));
  ASSERT_EQ(0,  CountTrailingOnes(static_cast<uint32_t>(0x00000000UL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingOnes_u64)
{
  ASSERT_EQ(64, CountTrailingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFFFFULL)));
  ASSERT_EQ(63, CountTrailingOnes(static_cast<uint64_t>(0x7FFFFFFFFFFFFFFFULL)));
  ASSERT_EQ(4,  CountTrailingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFF0FULL)));
  ASSERT_EQ(1,  CountTrailingOnes(static_cast<uint64_t>(0xFFFFFFFFFFFFFF01ULL)));
  ASSERT_EQ(0,  CountTrailingOnes(static_cast<uint64_t>(0x0000000000000000ULL)));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, ReverseBits8)
{
  for (uint_fast16_t i = 0U; i < 256U; i++)
  {
    uint_fast8_t expected = 0U;
    for (uint_fast8_t j = 0U; j < 8U; j++)
    {
      if ((i & (1U << j)) != 0U)
        expected |= 0x80U >> j;
    }

    uint_fast8_t const actual = ReverseBits8(i);
    ASSERT_EQ(actual, expected);
  }
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, ReverseBits16)
{
  for (uint_fast8_t i = 0U; i < 16U; i++)
  {
    uint16_t const in       = (1U << i);
    uint16_t const expected = (0x8000U >> i);

    uint16_t result = ReverseBits16(in);
    ASSERT_EQ(result, expected);
  }
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, ReverseBits32)
{
  for (uint_fast8_t shift = 0U; shift < 4U; shift++)
  {
    for (uint_fast16_t i = 0U; i < 256U; i++)
    {
      uint_fast32_t const input = static_cast<uint_fast32_t>(i) << (shift * 8U);
      uint_fast32_t expected = 0U;
      for (uint_fast8_t j = 0U; j < 32U; j++)
      {
        if ((input & (1UL << j)) != 0U)
          expected |= 0x80000000UL >> j;
      }

      uint_fast32_t const actual = ReverseBits32(input);
      ASSERT_EQ(actual, expected);
    }
  }
}

} // namespace Compiler
} // namespace gpcc_tests
