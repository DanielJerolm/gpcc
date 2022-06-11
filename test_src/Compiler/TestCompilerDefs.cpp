/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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
#include "gpcc/src/Compiler/definitions.hpp"
#include "gpcc/src/osal/Panic.hpp"

#include "gtest/gtest.h"
#include <cstdint>

namespace gpcc_tests
{
namespace Compiler
{

using namespace testing;

namespace {
NORETURN1 void NonReturningFunction(void) NORETURN2;

void NonReturningFunction(void)
{
  gpcc::osal::Panic("NonReturningFunction: Intentional Panic");
}

} // anonymus namespace

TEST(GPCC_Compiler_CompilerDefs_Tests, Endian)
{
  uint32_t const data = 0x12345678;
  uint8_t const *pDataU8 = reinterpret_cast<uint8_t const*>(&data);

#if (GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE)
  ASSERT_EQ(0x78, *pDataU8);
  pDataU8++;
  ASSERT_EQ(0x56, *pDataU8);
  pDataU8++;
  ASSERT_EQ(0x34, *pDataU8);
  pDataU8++;
  ASSERT_EQ(0x12, *pDataU8);
  pDataU8++;
#else
  ASSERT_EQ(0x12, *pDataU8);
  pDataU8++;
  ASSERT_EQ(0x34, *pDataU8);
  pDataU8++;
  ASSERT_EQ(0x56, *pDataU8);
  pDataU8++;
  ASSERT_EQ(0x78, *pDataU8);
  pDataU8++;
#endif
}

TEST(GPCC_Compiler_CompilerDefs_Tests, PacketTypedefStruct1)
{
  typedef PACKED1 struct packetStruct
  {
    int8_t i8;
    int16_t i16;
    int32_t i32;
  } PACKED2 packetStruct;

  packetStruct uut;

  uint8_t const * const pBase = reinterpret_cast<uint8_t const *>(&uut);

  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i8 )) == (pBase + 0));
  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i16)) == (pBase + 1));
  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i32)) == (pBase + 3));
}

TEST(GPCC_Compiler_CompilerDefs_Tests, PacketTypedefStruct2)
{
  typedef PACKED1 struct
  {
    int8_t i8;
    int16_t i16;
    int32_t i32;
  } PACKED2 packetStruct;

  packetStruct uut;

  uint8_t const * const pBase = reinterpret_cast<uint8_t const *>(&uut);

  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i8 )) == (pBase + 0));
  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i16)) == (pBase + 1));
  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i32)) == (pBase + 3));
}

TEST(GPCC_Compiler_CompilerDefs_Tests, PacketStruct)
{
  PACKED1 struct
  {
    int8_t i8;
    int16_t i16;
    int32_t i32;
  } PACKED2 uut;

  uint8_t const * const pBase = reinterpret_cast<uint8_t const *>(&uut);

  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i8 )) == (pBase + 0));
  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i16)) == (pBase + 1));
  ASSERT_TRUE(reinterpret_cast<uint8_t const *>(&(uut.i32)) == (pBase + 3));
}

TEST(GPCC_Compiler_CompilerDefs_DeathTests, NORETURN)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(NonReturningFunction(), ".*NonReturningFunction: Intentional Panic.*");
}

} // namespace Compiler
} // namespace gpcc_tests
