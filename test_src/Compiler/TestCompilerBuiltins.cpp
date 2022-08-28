/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/compiler/builtins.hpp>

#include "gtest/gtest.h"
#include <cstdint>
#include <limits>

namespace gpcc_tests
{
namespace Compiler
{

using gpcc::Compiler::OverflowAwareAdd;
using gpcc::Compiler::OverflowAwareSub;
using gpcc::Compiler::CountLeadingZeros;
using gpcc::Compiler::CountLeadingOnes;
using gpcc::Compiler::CountTrailingZeros;
using gpcc::Compiler::CountTrailingOnes;
using gpcc::Compiler::ReverseBits8;
using gpcc::Compiler::ReverseBits16;
using gpcc::Compiler::ReverseBits32;
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

TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingZeros)
{
  unsigned int const max  = std::numeric_limits<unsigned int>::max();
  int const digits        = std::numeric_limits<unsigned int>::digits;

  ASSERT_EQ(digits,     CountLeadingZeros(0));
  ASSERT_EQ(digits - 1, CountLeadingZeros(0x1));
  ASSERT_EQ(digits - 4, CountLeadingZeros(0x8));
  ASSERT_EQ(digits - 4, CountLeadingZeros(0xF));
  ASSERT_EQ(2,          CountLeadingZeros(max >> 2U));
  ASSERT_EQ(1,          CountLeadingZeros(max >> 1U));
  ASSERT_EQ(0,          CountLeadingZeros(max));
}
TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountLeadingOnes)
{
  unsigned int const max  = std::numeric_limits<unsigned int>::max();
  int const digits        = std::numeric_limits<unsigned int>::digits;

  ASSERT_EQ(digits,     CountLeadingOnes(max));
  ASSERT_EQ(digits - 1, CountLeadingOnes(max & (~static_cast<unsigned int>(0x1U))));
  ASSERT_EQ(digits - 4, CountLeadingOnes(max & (~static_cast<unsigned int>(0x8U))));
  ASSERT_EQ(digits - 4, CountLeadingOnes(max & (~static_cast<unsigned int>(0xFU))));
  ASSERT_EQ(2,          CountLeadingOnes(max << (digits - 2U)));
  ASSERT_EQ(1,          CountLeadingOnes(max << (digits - 1U)));
  ASSERT_EQ(0,          CountLeadingOnes(0x1));
  ASSERT_EQ(0,          CountLeadingOnes(0));
}
TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingZeros)
{
  ASSERT_EQ(std::numeric_limits<unsigned int>::digits,  CountTrailingZeros(0));
  ASSERT_EQ(0,                                          CountTrailingZeros(1));
  ASSERT_EQ(3,                                          CountTrailingZeros(8));
  ASSERT_EQ(3,                                          CountTrailingZeros(24));
  ASSERT_EQ(0,                                          CountTrailingZeros(std::numeric_limits<unsigned int>::max()));

}
TEST(GPCC_Compiler_CompilerBuiltins_Tests, CountTrailingOnes)
{
  ASSERT_EQ(0,                                          CountTrailingOnes(0));
  ASSERT_EQ(1,                                          CountTrailingOnes(1));
  ASSERT_EQ(3,                                          CountTrailingOnes(7));
  ASSERT_EQ(0,                                          CountTrailingOnes(8));
  ASSERT_EQ(2,                                          CountTrailingOnes(3));
  ASSERT_EQ(std::numeric_limits<unsigned int>::digits,  CountTrailingOnes(std::numeric_limits<unsigned int>::max()));
}

TEST(GPCC_Compiler_CompilerBuiltins_Tests, ReverseBits8)
{
  for (uint_fast16_t i = 0; i < 256; i++)
  {
    uint_fast8_t expected = 0;
    for (uint_fast8_t j = 0; j < 8; j++)
    {
      if ((i & (1U << j)) != 0)
        expected |= 0x80U >> j;
    }

    uint_fast8_t const actual = ReverseBits8(i);
    ASSERT_EQ(actual, expected);
  }
}
TEST(GPCC_Compiler_CompilerBuiltins_Tests, ReverseBits16)
{
  for (uint_fast8_t i = 0; i < 16; i++)
  {
    uint16_t const in       = (1U << i);
    uint16_t const expected = (0x8000U >> i);

    uint16_t result = ReverseBits16(in);
    ASSERT_EQ(result, expected);
  }
}
TEST(GPCC_Compiler_CompilerBuiltins_Tests, ReverseBits32)
{
  for (uint_fast8_t shift = 0; shift < 4; shift++)
  {
    for (uint_fast16_t i = 0; i < 256; i++)
    {
      uint_fast32_t const input = static_cast<uint_fast32_t>(i) << (shift * 8U);
      uint_fast32_t expected = 0;
      for (uint_fast8_t j = 0; j < 32; j++)
      {
        if ((input & (1U << j)) != 0)
          expected |= 0x80000000UL >> j;
      }

      uint_fast32_t const actual = ReverseBits32(input);
      ASSERT_EQ(actual, expected);
    }
  }
}


} // namespace Compiler
} // namespace gpcc_tests
