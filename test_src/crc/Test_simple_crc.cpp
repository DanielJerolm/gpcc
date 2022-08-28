/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/crc/simple_crc.hpp>
#include <gpcc/compiler/builtins.hpp>
#include <gpcc/string/tools.hpp>
#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include <cstring>

namespace gpcc_tests {
namespace crc {

using namespace testing;

namespace
{
  // input data for the "CHECK"
  std::string const check_data("123456789");
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_normal)
{
  uint32_t table[256];
  gpcc::crc::GenerateCRC32Table_normal(0x04C11DB7UL, table);

  ASSERT_EQ(0, memcmp(table, gpcc::crc::crc32ab_table_normal, sizeof(table)));
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_normal_ReceiverMagicValue1)
{
  // CRC used: CRC-32A (BZIP2)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_normal);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = ~crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the normal form (shift left) without data/CRC bit reverse, so the high byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  crcRx = ~crcRx;

  // in case of no error, the result must match the receiver magic value for CRC-32A (BZIP2)
  ASSERT_EQ(crcRx, 0x38FB2284UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_normal_ReceiverMagicValue2)
{
  // CRC used: CRC-32A (BZIP2)

  std::string _check_data("abcd76839290034");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_normal);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = ~crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the normal form (shift left) without data/CRC bit reverse, so the high byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_noInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  crcRx = ~crcRx;

  // in case of no error, the result must match the magic value for CRC-32A (BZIP2)
  ASSERT_EQ(crcRx, 0x38FB2284UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_normal_ReceiverMagicValue3)
{
  // CRC used: CRC-32B (Ethernet)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_normal);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = gpcc::Compiler::ReverseBits32(~crcTx);

  // The receiver includes the transmitted CRC into its calculation.
  // We use the normal form (shift left) with data/CRC bit reverse, so the low byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  crcRx = gpcc::Compiler::ReverseBits32(~crcRx);

  // in case of no error, the result must match the magic value for CRC-32B (Ethernet)
  ASSERT_EQ(crcRx, 0x2144DF1CUL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_normal_ReceiverMagicValue4)
{
  // CRC used: CRC-32B (Ethernet)

  std::string _check_data("abcd76839290034");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_normal);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = gpcc::Compiler::ReverseBits32(~crcTx);

  // The receiver includes the transmitted CRC into its calculation.
  // We use the normal form (shift left) with data/CRC bit reverse, so the low byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  gpcc::crc::CalcCRC32_normal_withInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_normal);
  crcRx = gpcc::Compiler::ReverseBits32(~crcRx);

  // in case of no error, the result must match the magic value for CRC-32B (Ethernet)
  ASSERT_EQ(crcRx, 0x2144DF1CUL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_reflected)
{
  uint32_t table[256];
  gpcc::crc::GenerateCRC32Table_reflected(0xEDB88320UL, table);

  ASSERT_EQ(0, memcmp(table, gpcc::crc::crc32ab_table_reflected, sizeof(table)));
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_reflected_ReceiverMagicValue1)
{
  // CRC used: CRC-32A (BZIP2)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_reflected);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = gpcc::Compiler::ReverseBits32(~crcTx);

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the reflected form (shift right) with data/CRC bit reverse, so the high byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  crcRx = gpcc::Compiler::ReverseBits32(~crcRx);

  // in case of no error, the result must match the magic value for CRC-32A (BZIP2)
  ASSERT_EQ(crcRx, 0x38FB2284UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_reflected_ReceiverMagicValue2)
{
  // CRC used: CRC-32A (BZIP2)

  std::string _check_data("abcd76839290034");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_reflected);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = gpcc::Compiler::ReverseBits32(~crcTx);

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the reflected form (shift right) with data/CRC bit reverse, so the high byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  crcRx = gpcc::Compiler::ReverseBits32(~crcRx);

  // in case of no error, the result must match the magic value for CRC-32A (BZIP2)
  ASSERT_EQ(crcRx, 0x38FB2284UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_reflected_ReceiverMagicValue3)
{
  // CRC used: CRC-32B (Ethernet)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_reflected);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = ~crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the reflected form (shift right) without data/CRC bit reverse, so the low byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  crcRx = ~crcRx;

  // in case of no error, the result must match the magic value for CRC-32B (Ethernet)
  ASSERT_EQ(crcRx, 0x2144DF1CUL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc32ab_table_reflected_ReceiverMagicValue4)
{
  // CRC used: CRC-32B (Ethernet)

  std::string _check_data("abcd76839290034");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint32_t crcTx = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcTx, pData, n, gpcc::crc::crc32ab_table_reflected);

  // transmitter finishes and receiver continues
  uint32_t crcRx = crcTx;
  crcTx = ~crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the reflected form (shift right) without data/CRC bit reverse, so the low byte of crcTx must be processed first
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >>  0U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >>  8U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >> 16U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crcRx, (crcTx >> 24U) & 0xFFU, gpcc::crc::crc32ab_table_reflected);
  crcRx = ~crcRx;

  // in case of no error, the result must match the magic value for CRC-32B (Ethernet)
  ASSERT_EQ(crcRx, 0x2144DF1CUL);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc16_ccitt_table_normal)
{
  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_normal(0x1021U, table);

  ASSERT_EQ(0, memcmp(table, gpcc::crc::crc16_ccitt_table_normal, sizeof(table)));
}

TEST(gpcc_crc_SimpleCRC_Tests, crc16_ccitt_table_normal_ReceiverMagicValue1)
{
  // CRC used: CRC16 XMODEM (normal)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint16_t crcTx = 0x0000U;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcTx, pData, n, gpcc::crc::crc16_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint16_t crcRx = crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the normal form (shift left) without data/CRC bit reverse, so the upper byte of crcTx must be processed first
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 8U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 0U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);

  // in case of no error, the result must match the magic value for CRC16 XMODEM
  ASSERT_EQ(crcRx, 0x0000U);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc16_ccitt_table_normal_ReceiverMagicValue2)
{
  // CRC used: CRC16 XMODEM (normal)

  std::string _check_data("abcdefghijklmn");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint16_t crcTx = 0x0000U;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcTx, pData, n, gpcc::crc::crc16_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint16_t crcRx = crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the normal form (shift left) without data/CRC bit reverse, so the upper byte of crcTx must be processed first
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 8U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 0U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);

  // in case of no error, the result must match the magic value for CRC16 XMODEM
  ASSERT_EQ(crcRx, 0x0000U);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc16_ccitt_table_normal_ReceiverMagicValue3)
{
  // CRC used: CRC16 CCITT FALSE (normal)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint16_t crcTx = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcTx, pData, n, gpcc::crc::crc16_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint16_t crcRx = crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the normal form (shift left) without data/CRC bit reverse, so the upper byte of crcTx must be processed first
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 8U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 0U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);

  // in case of no error, the result must match the magic value for CRC16 CCITT FALSE
  ASSERT_EQ(crcRx, 0x0000U);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc16_ccitt_table_normal_ReceiverMagicValue4)
{
  // CRC used: CRC16 CCITT FALSE (normal)

  std::string _check_data("abcdefghijklmno9383838");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint16_t crcTx = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcTx, pData, n, gpcc::crc::crc16_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint16_t crcRx = crcTx;

  // The receiver inclues the transmitted CRC into its calculation.
  // We use the normal form (shift left) without data/CRC bit reverse, so the upper byte of crcTx must be processed first
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 8U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);
  gpcc::crc::CalcCRC16_normal_noInputReverse(crcRx, (crcTx >> 0U) & 0xFFU, gpcc::crc::crc16_ccitt_table_normal);

  // in case of no error, the result must match the magic value for CRC16 CCITT FALSE
  ASSERT_EQ(crcRx, 0x0000U);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc8_ccitt_table_normal)
{
  uint8_t table[256];
  gpcc::crc::GenerateCRC8Table_normal(0x07U, table);

  ASSERT_EQ(0, memcmp(table, gpcc::crc::crc8_ccitt_table_normal, sizeof(table)));
}

TEST(gpcc_crc_SimpleCRC_Tests, crc8_ccitt_table_normal_ReceiverMagicValue1)
{
  // CRC used: CRC8-ITU (normal)

  std::string _check_data("ABCDEFGH");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint8_t crcTx = 0x00U;
  gpcc::crc::CalcCRC8_noInputReverse(crcTx, pData, n, gpcc::crc::crc8_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint8_t crcRx = crcTx;
  crcTx ^= 0x55U;

  // the receiver inclues the transmitted CRC into its calculation
  gpcc::crc::CalcCRC8_noInputReverse(crcRx, crcTx, gpcc::crc::crc8_ccitt_table_normal);
  crcRx ^= 0x55U;

  // in case of no error, the result must match the magic value for CRC8-ITU
  ASSERT_EQ(crcRx, 0xF9U);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc8_ccitt_table_normal_ReceiverMagicValue2)
{
  // CRC used: CRC8-ITU (normal)

  std::string _check_data("abcdefgh");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint8_t crcTx = 0x00U;
  gpcc::crc::CalcCRC8_noInputReverse(crcTx, pData, n, gpcc::crc::crc8_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint8_t crcRx = crcTx;
  crcTx ^= 0x55U;

  // the receiver inclues the transmitted CRC into its calculation
  gpcc::crc::CalcCRC8_noInputReverse(crcRx, crcTx, gpcc::crc::crc8_ccitt_table_normal);
  crcRx ^= 0x55U;

  // in case of no error, the result must match the magic value for CRC8-ITU
  ASSERT_EQ(crcRx, 0xF9U);
}

TEST(gpcc_crc_SimpleCRC_Tests, crc8_ccitt_table_normal_ReceiverMagicValue3)
{
  // CRC used: CRC8-ITU (normal)

  std::string _check_data("ABCDEFGHi");

  void const * pData = _check_data.data();
  auto const n       = _check_data.length();

  uint8_t crcTx = 0x00U;
  gpcc::crc::CalcCRC8_noInputReverse(crcTx, pData, n, gpcc::crc::crc8_ccitt_table_normal);

  // transmitter finishes and receiver continues
  uint8_t crcRx = crcTx;
  crcTx ^= 0x55U;

  // the receiver inclues the transmitted CRC into its calculation
  gpcc::crc::CalcCRC8_noInputReverse(crcRx, crcTx, gpcc::crc::crc8_ccitt_table_normal);
  crcRx ^= 0x55U;

  // in case of no error, the result must match the magic value for CRC8-ITU
  ASSERT_EQ(crcRx, 0xF9U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC8_noInputReverse_singleByte)
{
  // CRC used: CRC8-ITU (normal)

  uint8_t table[256];
  gpcc::crc::GenerateCRC8Table_normal(0x07U, table);

  uint8_t crc = 0x00U;
  for (auto const c : check_data)
    gpcc::crc::CalcCRC8_noInputReverse(crc, static_cast<uint8_t>(c), table);
  crc ^= 0x55U;

  ASSERT_EQ(crc, 0xA1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC8_noInputReverse_block)
{
  // CRC used: CRC8-ITU (normal)

  uint8_t table[256];
  gpcc::crc::GenerateCRC8Table_normal(0x07U, table);

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  uint8_t crc = 0x00U;
  gpcc::crc::CalcCRC8_noInputReverse(crc, pData, n, table);
  crc ^= 0x55U;

  ASSERT_EQ(crc, 0xA1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC8_withInputReverse_singleByte)
{
  // CRC used: CRC8-ITU (reflected)

  // CRC8-ITU (normal) does not require reversal of data bytes and final CRC.
  // We can use the reflected form, if we reverse both data bytes and final CRC.

  uint8_t table[256];
  gpcc::crc::GenerateCRC8Table_reflected(gpcc::Compiler::ReverseBits8(0x07U), table);

  uint8_t crc = 0x00U;
  for (auto const c : check_data)
    gpcc::crc::CalcCRC8_withInputReverse(crc, c, table);
  crc = gpcc::Compiler::ReverseBits8(crc);
  crc ^= 0x55U;

  ASSERT_EQ(crc, 0xA1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC8_withInputReverse_block)
{
  // CRC used: CRC8-ITU (reflected)

  // CRC8-ITU (normal) does not require reversal of data bytes and final CRC.
  // We can use the reflected form, if we reverse both data bytes and final CRC.

  uint8_t table[256];
  gpcc::crc::GenerateCRC8Table_reflected(gpcc::Compiler::ReverseBits8(0x07U), table);

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  uint8_t crc = 0x00U;
  gpcc::crc::CalcCRC8_withInputReverse(crc, pData, n, table);
  crc = gpcc::Compiler::ReverseBits8(crc);
  crc ^= 0x55U;

  ASSERT_EQ(crc, 0xA1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_normal_noInputReverse_singleByte)
{
  // CRC used: CRC-16 CCITT FALSE

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_normal(0x1021U, table);

  // calculate CRC
  uint16_t crc = 0xFFFFU;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC16_normal_noInputReverse(crc, c, table);

  ASSERT_EQ(crc, 0x29B1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_normal_noInputReverse_block)
{
  // CRC used: CRC-16 CCITT FALSE

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_normal(0x1021U, table);

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, pData, n, table);

  ASSERT_EQ(crc, 0x29B1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_normal_withInputReverse_singleByte)
{
  // CRC used: CRC-16 X-25

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_normal(0x1021U, table);

  // calculate CRC
  uint16_t crc = 0xFFFFU;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC16_normal_withInputReverse(crc, c, table);
  crc = gpcc::Compiler::ReverseBits16(crc);
  crc = ~crc;

  ASSERT_EQ(crc, 0x906EU);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_normal_withInputReverse_block)
{
  // CRC used: CRC-16 X-25

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_normal(0x1021U, table);

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_withInputReverse(crc, pData, n, table);
  crc = gpcc::Compiler::ReverseBits16(crc);
  crc = ~crc;

  ASSERT_EQ(crc, 0x906EU);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_reflected_noInputReverse_singleByte)
{
  // CRC used: CRC-16 ARC

  // CRC-16 ARC (normal) requires reversal of data bytes and final CRC.
  // If we use the reflected form, then we do not need to reverse data bytes and final CRC.

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_reflected(gpcc::Compiler::ReverseBits16(0x8005U), table);

  // calculate CRC
  uint16_t crc = 0x0000U;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC16_reflected_noInputReverse(crc, c, table);

  ASSERT_EQ(crc, 0xBB3DU);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_reflected_noInputReverse_block)
{
  // CRC used: CRC-16 ARC

  // CRC-16 ARC (normal) requires reversal of data bytes and final CRC.
  // If we use the reflected form, then we do not need to reverse data bytes and final CRC.

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_reflected(gpcc::Compiler::ReverseBits16(0x8005U), table);

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint16_t crc = 0x0000U;
  gpcc::crc::CalcCRC16_reflected_noInputReverse(crc, pData, n, table);

  ASSERT_EQ(crc, 0xBB3DU);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_reflected_withInputReverse_singleByte)
{
  // CRC used: CRC-16 CCITT FALSE

  // CRC-16 CCITT FALSE (normal) does not require reversal of data bytes and final CRC.
  // We can use the reflected form, if we reverse both data bytes and final CRC.

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_reflected(gpcc::Compiler::ReverseBits16(0x1021U), table);

  // calculate CRC
  uint16_t crc = 0xFFFFU;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC16_reflected_withInputReverse(crc, c, table);
  crc = gpcc::Compiler::ReverseBits16(crc);

  ASSERT_EQ(crc, 0x29B1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC16_reflected_withInputReverse_block)
{
  // CRC used: CRC-16 CCITT FALSE

  // CRC-16 CCITT FALSE (normal) does not require reversal of data bytes and final CRC.
  // We can use the reflected form, if we reverse both data bytes and final CRC.

  uint16_t table[256];
  gpcc::crc::GenerateCRC16Table_reflected(gpcc::Compiler::ReverseBits16(0x1021U), table);

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_reflected_withInputReverse(crc, pData, n, table);
  crc = gpcc::Compiler::ReverseBits16(crc);

  ASSERT_EQ(crc, 0x29B1U);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_normal_noInputReverse_singleByte)
{
  // CRC used: CRC-32A (BZIP2)

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC32_normal_noInputReverse(crc, c, gpcc::crc::crc32ab_table_normal);
  crc = ~crc;

  ASSERT_EQ(crc, 0xFC891918UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_normal_noInputReverse_block)
{
  // CRC used: CRC-32A (BZIP2)

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_normal_noInputReverse(crc, pData, n, gpcc::crc::crc32ab_table_normal);
  crc = ~crc;

  ASSERT_EQ(crc, 0xFC891918UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_normal_withInputReverse_singleByte)
{
  // CRC used: CRC-32B (Ethernet)

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC32_normal_withInputReverse(crc, c, gpcc::crc::crc32ab_table_normal);
  crc = gpcc::Compiler::ReverseBits32(crc);
  crc = ~crc;

  ASSERT_EQ(crc, 0xCBF43926UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_normal_withInputReverse_block)
{
  // CRC used: CRC-32B (Ethernet)

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_normal_withInputReverse(crc, pData, n, gpcc::crc::crc32ab_table_normal);
  crc = gpcc::Compiler::ReverseBits32(crc);
  crc = ~crc;

  ASSERT_EQ(crc, 0xCBF43926UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_reflected_noInputReverse_singleByte)
{
  // CRC used: CRC32-B (Ethernet)

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC32_reflected_noInputReverse(crc, c, gpcc::crc::crc32ab_table_reflected);
  crc = ~crc;

  ASSERT_EQ(crc, 0xCBF43926UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_reflected_noInputReverse_block)
{
  // CRC used: CRC32-B (Ethernet)

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_reflected_noInputReverse(crc, pData, n, gpcc::crc::crc32ab_table_reflected);
  crc = ~crc;

  ASSERT_EQ(crc, 0xCBF43926UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_reflected_withInputReverse_singleByte)
{
  // CRC used: CRC-32A (BZIP2)

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  for (auto const c: check_data)
    gpcc::crc::CalcCRC32_reflected_withInputReverse(crc, c, gpcc::crc::crc32ab_table_reflected);
  crc = gpcc::Compiler::ReverseBits32(crc);
  crc = ~crc;

  ASSERT_EQ(crc, 0xFC891918UL);
}

TEST(gpcc_crc_SimpleCRC_Tests, CalcCRC32_reflected_withInputReverse_block)
{
  // CRC used: CRC-32A (BZIP2)

  void const * pData = check_data.data();
  auto const n       = check_data.length();

  // calculate CRC
  uint32_t crc = 0xFFFFFFFFUL;
  gpcc::crc::CalcCRC32_reflected_withInputReverse(crc, pData, n, gpcc::crc::crc32ab_table_reflected);
  crc = gpcc::Compiler::ReverseBits32(crc);
  crc = ~crc;

  ASSERT_EQ(crc, 0xFC891918UL);
}

} // namespace crc
} // namespace gpcc_tests
