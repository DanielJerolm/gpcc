/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/compiler/definitions.hpp>
#include <gpcc/osal/Panic.hpp>

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
