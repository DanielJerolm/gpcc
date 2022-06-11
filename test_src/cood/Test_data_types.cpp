/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018 Daniel Jerolm

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

#include "gpcc/src/cood/data_types.hpp"
#include "gpcc/src/cood/exceptions.hpp"
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <cstring>

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;

// Checks binary data for equality and prints it to std::cout in case of any difference
static bool CompareBinary(void const * _pActual, void const * _pExpected, size_t n)
{
  uint8_t const * pActual = static_cast<uint8_t const*>(_pActual);
  uint8_t const * pExpected = static_cast<uint8_t const*>(_pExpected);

  if (memcmp(pActual, pExpected, n) != 0U)
  {
    std::cout << "Created binary does not match expected binary:" << std::endl;

    size_t i = 0U;
    while (n != 0U)
    {
      uint8_t const v1 = *pActual++;
      uint8_t const v2 = *pExpected++;

      std::cout << i << ": " << gpcc::string::ToHex(v1, 2U) << ((v1 == v2) ? " == " : " != ") << gpcc::string::ToHex(v2, 2U) << std::endl;
      ++i;
      --n;
    };

    return false;
  }

  return true;
}

TEST(gpcc_cood_data_types_Tests, DataTypeToString)
{
  // we just try one type here
  char const * p = DataTypeToString(DataType::null);
  ASSERT_TRUE(p != nullptr);
  EXPECT_STREQ(p, "NULL");
}

TEST(gpcc_cood_data_types_Tests, DataType_ToUint16)
{
  for (uint8_t i = 0; i <= 0x0040U; i++)
  {
    ASSERT_EQ(i, ToUint16(ToDataType(i)));
  }
}

TEST(gpcc_cood_data_types_Tests, ToDataType)
{
  for (uint8_t i = 0; i <= 0x0040U; i++)
  {
    ASSERT_EQ(i, static_cast<uint8_t>(ToDataType(i)));
  }

  EXPECT_THROW((void)ToDataType(0x0041U), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_DataTypeNotSupported)
{
  uint8_t const mem[] = {0, 0, 0, 0, 0, 0, 0, 0};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  ASSERT_THROW(str = CANopenEncodedDataToString(msr, 40U, DataType::unsigned40), DataTypeNotSupportedError);
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_NULL)
{
  uint8_t const mem[] = {0};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 0U, DataType::null);
  EXPECT_STREQ(str.c_str(), "");

  str = CANopenEncodedDataToString(msr, 1U, DataType::null);
  EXPECT_STREQ(str.c_str(), "");

  str = CANopenEncodedDataToString(msr, 2U, DataType::null);
  EXPECT_STREQ(str.c_str(), "");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::five));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BOOLEAN)
{
  uint8_t const mem[] = {0x02U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 1U, DataType::boolean);
  EXPECT_STREQ(str.c_str(), "FALSE");
  str = CANopenEncodedDataToString(msr, 1U, DataType::boolean);
  EXPECT_STREQ(str.c_str(), "TRUE");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::six));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_INTEGER8)
{
  int8_t const mem[] = {-128, 0, 127};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 8U, DataType::integer8);
  EXPECT_STREQ(str.c_str(), "-128");
  str = CANopenEncodedDataToString(msr, 8U, DataType::integer8);
  EXPECT_STREQ(str.c_str(), "0");
  str = CANopenEncodedDataToString(msr, 8U, DataType::integer8);
  EXPECT_STREQ(str.c_str(), "127");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_INTEGER16)
{
  uint8_t const mem[] = {0x00U, 0x80U,
                         0x00U, 0x00U,
                         0xFFU, 0x7FU};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 16U, DataType::integer16);
  EXPECT_STREQ(str.c_str(), "-32768");
  str = CANopenEncodedDataToString(msr, 16U, DataType::integer16);
  EXPECT_STREQ(str.c_str(), "0");
  str = CANopenEncodedDataToString(msr, 16U, DataType::integer16);
  EXPECT_STREQ(str.c_str(), "32767");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_INTEGER32)
{
  uint8_t const mem[] = {0x00U, 0x00U, 0x00U, 0x80U,
                         0x00U, 0x00U, 0x00U, 0x00U,
                         0xFFU, 0xFFU, 0xFFU, 0x7F};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 32U, DataType::integer32);
  EXPECT_STREQ(str.c_str(), "-2147483648");
  str = CANopenEncodedDataToString(msr, 32U, DataType::integer32);
  EXPECT_STREQ(str.c_str(), "0");
  str = CANopenEncodedDataToString(msr, 32U, DataType::integer32);
  EXPECT_STREQ(str.c_str(), "2147483647");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNSIGNED8)
{
  uint8_t const mem[] = {0, 255};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 8U, DataType::unsigned8);
  EXPECT_STREQ(str.c_str(), "0 (0x00)");
  str = CANopenEncodedDataToString(msr, 8U, DataType::unsigned8);
  EXPECT_STREQ(str.c_str(), "255 (0xFF)");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNSIGNED16)
{
  uint8_t const mem[] = {0x00U, 0x00U,
                         0xFFU, 0xFFU};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 16U, DataType::unsigned16);
  EXPECT_STREQ(str.c_str(), "0 (0x0000)");
  str = CANopenEncodedDataToString(msr, 16U, DataType::unsigned16);
  EXPECT_STREQ(str.c_str(), "65535 (0xFFFF)");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNSIGNED32)
{
  uint8_t const mem[] = {0x00U, 0x00U, 0x00U, 0x00U,
                         0xFFU, 0xFFU, 0xFFU, 0xFFU};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 32U, DataType::unsigned32);
  EXPECT_STREQ(str.c_str(), "0 (0x00000000)");
  str = CANopenEncodedDataToString(msr, 32U, DataType::unsigned32);
  EXPECT_STREQ(str.c_str(), "4294967295 (0xFFFFFFFF)");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_REAL32)
{
  uint8_t mem[4];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);
  msw.Write_float(10.5);
  msw.Close();

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 32U, DataType::real32);
  EXPECT_STREQ(str.c_str(), "10.5");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::MemStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_VISIBLE_STRING_ShortStr)
{
  char const mem[] = {'t', 'e', 's', 't', 0x00, 0x00, 0x00, 0x00};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, sizeof(mem) * 8U, DataType::visible_string);
  ASSERT_EQ(str.length(), 6U);
  EXPECT_STREQ(str.c_str(), "\"test\"");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_VISIBLE_STRING_ShortStr_DataBehindNT)
{
  char const mem[] = {'t', 'e', 's', 't', 0x00, '!', '!', '!'};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, sizeof(mem) * 8U, DataType::visible_string);
  ASSERT_EQ(str.length(), 6U);
  EXPECT_STREQ(str.c_str(), "\"test\"");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_VISIBLE_STRING_FullLength)
{
  char const mem[] = {'t', 'e', 's', 't', 'A', 'B', 'C', 'D'};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, sizeof(mem) * 8U, DataType::visible_string);
  ASSERT_EQ(str.length(), 10U);
  EXPECT_STREQ(str.c_str(), "\"testABCD\"");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_VISIBLE_STRING_ZeroChars)
{
  char const mem[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, sizeof(mem) * 8U, DataType::visible_string);
  ASSERT_EQ(str.length(), 2U);
  EXPECT_STREQ(str.c_str(), "\"\"");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_VISIBLE_STRING_ZeroChars_DataBehindNT)
{
  char const mem[] = {0x00, '!', '!', '!', '!', '!', '!', '!'};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, sizeof(mem) * 8U, DataType::visible_string);
  ASSERT_EQ(str.length(), 2U);
  EXPECT_STREQ(str.c_str(), "\"\"");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_VISIBLE_STRING_ZeroLength)
{
  char const mem[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 0U, DataType::visible_string);
  ASSERT_EQ(str.length(), 2U);
  EXPECT_STREQ(str.c_str(), "\"\"");

  EXPECT_EQ(sizeof(mem), msr.RemainingBytes());
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_OCTET_STRING_1)
{
  uint8_t const mem[] = {0x3EU};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 1U * 8U, DataType::octet_string);
  EXPECT_STREQ(str.c_str(), "(hex) 3E");
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_OCTET_STRING_2)
{
  uint8_t const mem[] = {0x00U, 0x01U, 0xFFU};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 3U * 8U, DataType::octet_string);
  EXPECT_STREQ(str.c_str(), "(hex) 00 01 FF");
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_OCTET_STRING_ZeroLength)
{
  uint8_t const mem[] = {0x00U, 0x01U, 0xFFU};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  ASSERT_THROW(str = CANopenEncodedDataToString(msr, 0U * 8U, DataType::octet_string), std::logic_error);
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNICODE_STRING_1)
{
  uint8_t const mem[] = {0x3EU, 0x45U};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 1U * 16U, DataType::unicode_string);
  EXPECT_STREQ(str.c_str(), "(hex) 453E");
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNICODE_STRING_2)
{
  uint8_t const mem[] = {0x3EU, 0x45U, 0xABU, 0xCDU, 0xFEU, 0x87U};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 3U * 16U, DataType::unicode_string);
  EXPECT_STREQ(str.c_str(), "(hex) 453E CDAB 87FE");
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNICODE_STRING_ZeroLength)
{
  uint8_t const mem[] = {0x00U, 0x01U, 0xFFU};

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  ASSERT_THROW(str = CANopenEncodedDataToString(msr, 0U * 8U, DataType::unicode_string), std::logic_error);
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_REAL64)
{
  uint8_t mem[8];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);
  msw.Write_double(10.5);
  msw.Close();

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 64U, DataType::real64);
  EXPECT_STREQ(str.c_str(), "10.5");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_INTEGER64)
{
  uint8_t const mem[] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U,
                         0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
                         0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x7F};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 64U, DataType::integer64);
  EXPECT_STREQ(str.c_str(), "-9223372036854775808");
  str = CANopenEncodedDataToString(msr, 64U, DataType::integer64);
  EXPECT_STREQ(str.c_str(), "0");
  str = CANopenEncodedDataToString(msr, 64U, DataType::integer64);
  EXPECT_STREQ(str.c_str(), "9223372036854775807");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_UNSIGNED64)
{
  uint8_t const mem[] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
                         0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFF};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 64U, DataType::unsigned64);
  EXPECT_STREQ(str.c_str(), "0 (0x00000000.00000000)");
  str = CANopenEncodedDataToString(msr, 64U, DataType::unsigned64);
  EXPECT_STREQ(str.c_str(), "18446744073709551615 (0xFFFFFFFF.FFFFFFFF)");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT1)
{
  uint8_t const mem[] = {0x02U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 1U, DataType::bit1);
  EXPECT_STREQ(str.c_str(), "0b0");
  str = CANopenEncodedDataToString(msr, 1U, DataType::bit1);
  EXPECT_STREQ(str.c_str(), "0b1");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::six));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT2)
{
  uint8_t const mem[] = {0x0BU};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 2U, DataType::bit2);
  EXPECT_STREQ(str.c_str(), "0b11");
  str = CANopenEncodedDataToString(msr, 2U, DataType::bit2);
  EXPECT_STREQ(str.c_str(), "0b10");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::four));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT3)
{
  uint8_t const mem[] = {0x27U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 3U, DataType::bit3);
  EXPECT_STREQ(str.c_str(), "0b111");
  str = CANopenEncodedDataToString(msr, 3U, DataType::bit3);
  EXPECT_STREQ(str.c_str(), "0b100");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::two));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT4)
{
  uint8_t const mem[] = {0x8FU};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 4U, DataType::bit4);
  EXPECT_STREQ(str.c_str(), "0b1111");
  str = CANopenEncodedDataToString(msr, 4U, DataType::bit4);
  EXPECT_STREQ(str.c_str(), "0b1000");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT5)
{
  uint8_t const mem[] = {0x1FU, 0x02U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 5U, DataType::bit5);
  EXPECT_STREQ(str.c_str(), "0b11111");
  str = CANopenEncodedDataToString(msr, 5U, DataType::bit5);
  EXPECT_STREQ(str.c_str(), "0b10000");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::six));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT6)
{
  uint8_t const mem[] = {0x3FU, 0x08U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 6U, DataType::bit6);
  EXPECT_STREQ(str.c_str(), "0b111111");
  str = CANopenEncodedDataToString(msr, 6U, DataType::bit6);
  EXPECT_STREQ(str.c_str(), "0b100000");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::four));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT7)
{
  uint8_t const mem[] = {0x7FU, 0x20U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 7U, DataType::bit7);
  EXPECT_STREQ(str.c_str(), "0b1111111");
  str = CANopenEncodedDataToString(msr, 7U, DataType::bit7);
  EXPECT_STREQ(str.c_str(), "0b1000000");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::two));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_BIT8)
{
  uint8_t const mem[] = {0xFFU, 0x80U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  str = CANopenEncodedDataToString(msr, 8U, DataType::bit8);
  EXPECT_STREQ(str.c_str(), "0b11111111");
  str = CANopenEncodedDataToString(msr, 8U, DataType::bit8);
  EXPECT_STREQ(str.c_str(), "0b10000000");

  EXPECT_NO_THROW(msr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::zero));
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_AdditionalDataTypes)
{
  uint8_t const mem[] = {0x00U, 0x01U, 0x02U, 0x03U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  EXPECT_THROW(str = CANopenEncodedDataToString(msr, 1U, DataType::boolean_native_bit1), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, CANopenEncodedDataToString_NotEnoughDataInStream)
{
  uint8_t const mem[] = {0xFFU, 0x80U};
  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);
  std::string str;

  ASSERT_THROW(str = CANopenEncodedDataToString(msr, 32U, DataType::unsigned32), std::runtime_error);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_DataTypeNotSupported)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("12", 40U, DataType::unsigned40, msw), DataTypeNotSupportedError);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_NULL)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("",  0U, DataType::null, msw), DataTypeNotSupportedError);
  EXPECT_THROW(StringToCANOpenEncodedData("",  1U, DataType::null, msw), DataTypeNotSupportedError);

  msw.Close();
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BOOLEAN)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("TRUE",  1U, DataType::boolean, msw);
  StringToCANOpenEncodedData("true",  1U, DataType::boolean, msw);
  StringToCANOpenEncodedData("FALSE", 1U, DataType::boolean, msw);
  StringToCANOpenEncodedData("false", 1U, DataType::boolean, msw);
  StringToCANOpenEncodedData("TRUE",  1U, DataType::boolean, msw);

  msw.Close();

  uint8_t const expected[] = { 0x13U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BOOLEAN_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("T",     1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("TRUEE", 1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("1",     1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0",     1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("",      1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData(" ",     1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData(" TRUE", 1U, DataType::boolean, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("TRUE ", 1U, DataType::boolean, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER8)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("-128", 8U, DataType::integer8, msw);
  StringToCANOpenEncodedData("-1",   8U, DataType::integer8, msw);
  StringToCANOpenEncodedData("0",    8U, DataType::integer8, msw);
  StringToCANOpenEncodedData("1",    8U, DataType::integer8, msw);
  StringToCANOpenEncodedData("127",  8U, DataType::integer8, msw);
  StringToCANOpenEncodedData("+1",   8U, DataType::integer8, msw);

  msw.Close();

  uint8_t const expected[] = {0x80U, 0xFFU, 0x00U, 0x01U, 0x7FU, 0x01U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER8_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-129",  8U, DataType::integer8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("128",   8U, DataType::integer8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-128 ", 8U, DataType::integer8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0x50",  8U, DataType::integer8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.6",   8U, DataType::integer8, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER16)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("-32768", 16U, DataType::integer16, msw);
  StringToCANOpenEncodedData("-1",     16U, DataType::integer16, msw);
  StringToCANOpenEncodedData("0",      16U, DataType::integer16, msw);
  StringToCANOpenEncodedData("1",      16U, DataType::integer16, msw);
  StringToCANOpenEncodedData("32767",  16U, DataType::integer16, msw);

  msw.Close();

  uint8_t const expected[] = {0x00U, 0x80U,
                              0xFFU, 0xFFU,
                              0x00U, 0x00U,
                              0x01U, 0x00U,
                              0xFFU, 0x7FU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER16_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-32769", 16U, DataType::integer16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("32768",  16U, DataType::integer16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-100 ",  16U, DataType::integer16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0x50",   16U, DataType::integer16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.6",    16U, DataType::integer16, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER32)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("-2147483648", 32U, DataType::integer32, msw);
  StringToCANOpenEncodedData("-1",          32U, DataType::integer32, msw);
  StringToCANOpenEncodedData("0",           32U, DataType::integer32, msw);
  StringToCANOpenEncodedData("1",           32U, DataType::integer32, msw);
  StringToCANOpenEncodedData("2147483647",  32U, DataType::integer32, msw);

  msw.Close();

  uint8_t const expected[] = {0x00U, 0x00U, 0x00U, 0x80U,
                              0xFFU, 0xFFU, 0xFFU, 0xFFU,
                              0x00U, 0x00U, 0x00U, 0x00U,
                              0x01U, 0x00U, 0x00U, 0x00U,
                              0xFFU, 0xFFU, 0xFFU, 0x7FU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER32_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-2147483649", 32U, DataType::integer32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("2147483648",  32U, DataType::integer32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-100 ",       32U, DataType::integer32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0x50",        32U, DataType::integer32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.6",         32U, DataType::integer32, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNSIGNED8)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0",     8U, DataType::unsigned8, msw);
  StringToCANOpenEncodedData("1",     8U, DataType::unsigned8, msw);
  StringToCANOpenEncodedData("255",   8U, DataType::unsigned8, msw);
  StringToCANOpenEncodedData("+1",    8U, DataType::unsigned8, msw);
  StringToCANOpenEncodedData("0x12",  8U, DataType::unsigned8, msw);
  StringToCANOpenEncodedData("0b101", 8U, DataType::unsigned8, msw);

  msw.Close();

  uint8_t const expected[] = {0x00U, 0x01U, 0xFFU, 0x01U, 0x12U, 0x05U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNSIGNED8_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-",           8U, DataType::unsigned8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("256",         8U, DataType::unsigned8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-1 ",         8U, DataType::unsigned8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0x500",       8U, DataType::unsigned8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0b100000000", 8U, DataType::unsigned8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0b020",       8U, DataType::unsigned8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.6",         8U, DataType::unsigned8, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNSIGNED16)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0",     16U, DataType::unsigned16, msw);
  StringToCANOpenEncodedData("1",     16U, DataType::unsigned16, msw);
  StringToCANOpenEncodedData("65535", 16U, DataType::unsigned16, msw);
  StringToCANOpenEncodedData("+1",    16U, DataType::unsigned16, msw);
  StringToCANOpenEncodedData("0x12",  16U, DataType::unsigned16, msw);
  StringToCANOpenEncodedData("0b101", 16U, DataType::unsigned16, msw);

  msw.Close();

  uint8_t const expected[] = {0x00U, 0x00U,
                              0x01U, 0x00U,
                              0xFFU, 0xFFU,
                              0x01U, 0x00U,
                              0x12U, 0x00U,
                              0x05U, 0x00U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNSIGNED16_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-",                   16U, DataType::unsigned16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("65536",               16U, DataType::unsigned16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-1 ",                 16U, DataType::unsigned16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0x50000",             16U, DataType::unsigned16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0b10000000000000000", 16U, DataType::unsigned16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0b020",               16U, DataType::unsigned16, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.6",                 16U, DataType::unsigned16, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNSIGNED32)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0",          32U, DataType::unsigned32, msw);
  StringToCANOpenEncodedData("1",          32U, DataType::unsigned32, msw);
  StringToCANOpenEncodedData("4294967295", 32U, DataType::unsigned32, msw);
  StringToCANOpenEncodedData("+1",         32U, DataType::unsigned32, msw);
  StringToCANOpenEncodedData("0x12",       32U, DataType::unsigned32, msw);
  StringToCANOpenEncodedData("0b101",      32U, DataType::unsigned32, msw);

  msw.Close();

  uint8_t const expected[] = {0x00U, 0x00U, 0x00U, 0x00U,
                              0x01U, 0x00U, 0x00U, 0x00U,
                              0xFFU, 0xFFU, 0xFFU, 0xFFU,
                              0x01U, 0x00U, 0x00U, 0x00U,
                              0x12U, 0x00U, 0x00U, 0x00U,
                              0x05U, 0x00U, 0x00U, 0x00U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNSIGNED32_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-",           32U, DataType::unsigned32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("4294967296",  32U, DataType::unsigned32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-1 ",         32U, DataType::unsigned32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0x500000000", 32U, DataType::unsigned32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0b100000000000000000000000000000000", 32U, DataType::unsigned32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0b020",       32U, DataType::unsigned32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.6",         32U, DataType::unsigned32, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_REAL32)
{
  uint8_t mem[16];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0.55", 32U, DataType::real32, msw);
  StringToCANOpenEncodedData("3E15", 32U, DataType::real32, msw);

  msw.Close();

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);

  float f;

  f = msr.Read_float();
  f = 0.55 - f;
  if (f < 0)
    f = -f;
  EXPECT_LE(f, 0.01);

  f = msr.Read_float();
  f = 3E15 - f;
  if (f < 0)
    f = -f;
  EXPECT_LE(f, 3E8);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_REAL32_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("0,5",  32U, DataType::real32, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("13F4", 32U, DataType::real32, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_VISIBLE_STRING)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  //                         "0......7"
  StringToCANOpenEncodedData("",         8U * 8U, DataType::visible_string, msw);
  StringToCANOpenEncodedData("Half",     8U * 8U, DataType::visible_string, msw);
  StringToCANOpenEncodedData("--Full--", 8U * 8U, DataType::visible_string, msw);

  msw.Close();

  char const expected[] = {  0,   0,   0,   0,   0,   0,   0,   0,
                            'H', 'a', 'l', 'f',  0,   0,   0,   0,
                            '-', '-', 'F', 'u', 'l', 'l', '-', '-' };

  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_VISIBLE_STRING_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  //                                      "0......7"
  EXPECT_THROW(StringToCANOpenEncodedData("",          0U * 8U, DataType::visible_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("TooLongXX", 8U * 8U, DataType::visible_string, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_OCTET_STRING_1)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  // (number of bits decrements from call to call)
  StringToCANOpenEncodedData("12", 4U * 8U, DataType::octet_string, msw);
  StringToCANOpenEncodedData("A5", 3U * 8U, DataType::octet_string, msw);
  StringToCANOpenEncodedData("b6", 2U * 8U, DataType::octet_string, msw);
  StringToCANOpenEncodedData("FF", 1U * 8U, DataType::octet_string, msw);

  msw.Close();

  uint8_t const expected[] = { 0x12U, 0xA5U, 0xB6U, 0xFFU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_OCTET_STRING_2)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  // (number of bits does not decrement from call to call)
  StringToCANOpenEncodedData("12", 4U * 8U, DataType::octet_string, msw);
  StringToCANOpenEncodedData("A5", 4U * 8U, DataType::octet_string, msw);
  StringToCANOpenEncodedData("b6", 4U * 8U, DataType::octet_string, msw);
  StringToCANOpenEncodedData("FF", 4U * 8U, DataType::octet_string, msw);

  msw.Close();

  uint8_t const expected[] = { 0x12U, 0xA5U, 0xB6U, 0xFFU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_OCTET_STRING_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("",      0U * 8U, DataType::octet_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0",     4U * 8U, DataType::octet_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("00 00", 4U * 8U, DataType::octet_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("000",   4U * 8U, DataType::octet_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("GG",    4U * 8U, DataType::octet_string, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNICODE_STRING_1)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  // (number of bits decrements from call to call)
  StringToCANOpenEncodedData("1234", 4U * 16U, DataType::unicode_string, msw);
  StringToCANOpenEncodedData("A5F4", 3U * 16U, DataType::unicode_string, msw);
  StringToCANOpenEncodedData("b6c2", 2U * 16U, DataType::unicode_string, msw);
  StringToCANOpenEncodedData("FFff", 1U * 16U, DataType::unicode_string, msw);

  msw.Close();

  uint8_t const expected[] = { 0x34U, 0x12U, 0xF4U, 0xA5U, 0xC2U, 0xB6U, 0xFFU, 0xFFU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNICODE_STRING_2)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  // (number of bits does not decrement from call to call)
  StringToCANOpenEncodedData("1234", 4U * 16U, DataType::unicode_string, msw);
  StringToCANOpenEncodedData("A5F4", 4U * 16U, DataType::unicode_string, msw);
  StringToCANOpenEncodedData("b6c2", 4U * 16U, DataType::unicode_string, msw);
  StringToCANOpenEncodedData("FFff", 4U * 16U, DataType::unicode_string, msw);

  msw.Close();

  uint8_t const expected[] = { 0x34U, 0x12U, 0xF4U, 0xA5U, 0xC2U, 0xB6U, 0xFFU, 0xFFU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_UNICODE_STRING_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("",      0U * 16U, DataType::unicode_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0",     4U * 16U, DataType::unicode_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("00 00", 4U * 16U, DataType::unicode_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("00000", 4U * 16U, DataType::unicode_string, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("GGFF",  4U * 16U, DataType::unicode_string, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_REAL64)
{
  uint8_t mem[16];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0.55", 64U, DataType::real64, msw);
  StringToCANOpenEncodedData("3E15", 64U, DataType::real64, msw);

  msw.Close();

  gpcc::Stream::MemStreamReader msr(mem, sizeof(mem), gpcc::Stream::MemStreamReader::Endian::Little);

  double d;

  d = msr.Read_double();
  d = 0.55 - d;
  if (d < 0)
    d = -d;
  EXPECT_LE(d, 0.01);

  d = msr.Read_double();
  d = 3E15 - d;
  if (d < 0)
    d = -d;
  EXPECT_LE(d, 3E1);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_REAL64_invVal)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("0,5",  64U, DataType::real64, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("13F4", 64U, DataType::real64, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER64)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("-9223372036854775808", 64U, DataType::integer64, msw);
  StringToCANOpenEncodedData("-1",                   64U, DataType::integer64, msw);
  StringToCANOpenEncodedData("0",                    64U, DataType::integer64, msw);
  StringToCANOpenEncodedData("1",                    64U, DataType::integer64, msw);
  StringToCANOpenEncodedData("9223372036854775807",  64U, DataType::integer64, msw);

  msw.Close();

  uint8_t const expected[] = {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U,
                              0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU,
                              0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
                              0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
                              0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x7FU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_INTEGER64_invValue)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("-9223372036854775809", 64U, DataType::integer64, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("9223372036854775808",  64U, DataType::integer64, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("3.5",                  64U, DataType::integer64, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("0xAB",                 64U, DataType::integer64, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT1)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b0", 1U, DataType::bit1, msw);
  StringToCANOpenEncodedData("0b1", 1U, DataType::bit1, msw);

  StringToCANOpenEncodedData("0x0", 1U, DataType::bit1, msw);
  StringToCANOpenEncodedData("0x1", 1U, DataType::bit1, msw);

  StringToCANOpenEncodedData("0", 1U, DataType::bit1, msw);
  StringToCANOpenEncodedData("1", 1U, DataType::bit1, msw);

  msw.Close();

  uint8_t const expected[] = {0x2AU };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT1_invValue)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("0b11", 1U, DataType::bit1, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("a", 1U, DataType::bit1, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("", 1U, DataType::bit1, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-", 1U, DataType::bit1, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT2)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b00", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("0b10", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("0b01", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("0b11", 2U, DataType::bit2, msw);

  StringToCANOpenEncodedData("0x0", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("0x2", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("0x1", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("0x3", 2U, DataType::bit2, msw);

  StringToCANOpenEncodedData("0", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("2", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("1", 2U, DataType::bit2, msw);
  StringToCANOpenEncodedData("3", 2U, DataType::bit2, msw);

  msw.Close();

  uint8_t const expected[] = { 0xD8U, 0xD8U, 0xD8U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT2_invValue)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("0b100", 2U, DataType::bit2, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("a", 2U, DataType::bit2, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("", 2U, DataType::bit2, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-", 2U, DataType::bit2, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT3)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b000", 3U, DataType::bit3, msw);
  StringToCANOpenEncodedData("0b001", 3U, DataType::bit3, msw);
  StringToCANOpenEncodedData("0b010", 3U, DataType::bit3, msw);
  StringToCANOpenEncodedData("0b100", 3U, DataType::bit3, msw);

  msw.Close();

  uint8_t const expected[] = { 0x88U, 0x08U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT4)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b0001", 4U, DataType::bit4, msw);
  StringToCANOpenEncodedData("0b0010", 4U, DataType::bit4, msw);
  StringToCANOpenEncodedData("0b0100", 4U, DataType::bit4, msw);
  StringToCANOpenEncodedData("0b1000", 4U, DataType::bit4, msw);

  msw.Close();

  uint8_t const expected[] = { 0x21U, 0x84U};
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT5)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b00001", 5U, DataType::bit5, msw);
  StringToCANOpenEncodedData("0b00010", 5U, DataType::bit5, msw);
  StringToCANOpenEncodedData("0b00100", 5U, DataType::bit5, msw);
  StringToCANOpenEncodedData("0b01000", 5U, DataType::bit5, msw);
  StringToCANOpenEncodedData("0b10000", 5U, DataType::bit5, msw);

  msw.Close();

  uint8_t const expected[] = { 0x41U, 0x10U, 0x04U, 0x01U};
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT6)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b000001", 6U, DataType::bit6, msw);
  StringToCANOpenEncodedData("0b000010", 6U, DataType::bit6, msw);
  StringToCANOpenEncodedData("0b000100", 6U, DataType::bit6, msw);
  StringToCANOpenEncodedData("0b001000", 6U, DataType::bit6, msw);
  StringToCANOpenEncodedData("0b010000", 6U, DataType::bit6, msw);
  StringToCANOpenEncodedData("0b100000", 6U, DataType::bit6, msw);

  msw.Close();

  uint8_t const expected[] = { 0x81U, 0x40U, 0x20U, 0x10U, 0x08U};
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT7)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b0000001", 7U, DataType::bit7, msw);
  StringToCANOpenEncodedData("0b0000010", 7U, DataType::bit7, msw);
  StringToCANOpenEncodedData("0b0000100", 7U, DataType::bit7, msw);
  StringToCANOpenEncodedData("0b0001000", 7U, DataType::bit7, msw);
  StringToCANOpenEncodedData("0b0010000", 7U, DataType::bit7, msw);
  StringToCANOpenEncodedData("0b0100000", 7U, DataType::bit7, msw);
  StringToCANOpenEncodedData("0b1000000", 7U, DataType::bit7, msw);

  msw.Close();

  uint8_t const expected[] = { 0x01U, 0x01U, 0x01U, 0x01U, 0x01U, 0x01U, 0x01U};
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT8)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  StringToCANOpenEncodedData("0b00000001", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b00000010", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b00000100", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b00001000", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b00010000", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b00100000", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b01000000", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0b10000000", 8U, DataType::bit8, msw);

  StringToCANOpenEncodedData("0x01", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x02", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x04", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x08", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x10", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x20", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x40", 8U, DataType::bit8, msw);
  StringToCANOpenEncodedData("0x80", 8U, DataType::bit8, msw);

  msw.Close();

  uint8_t const expected[] = { 0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
                               0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U };
  EXPECT_TRUE(CompareBinary(mem, expected, sizeof(expected)));
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_AdditionalDataTypes)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("TRUE", 8U, DataType::boolean_native_bit1, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, StringToCANOpenEncodedData_BIT8_invValue)
{
  uint8_t mem[128];
  gpcc::Stream::MemStreamWriter msw(mem, sizeof(mem), gpcc::Stream::MemStreamWriter::Endian::Little);

  EXPECT_THROW(StringToCANOpenEncodedData("0x100", 8U, DataType::bit8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("256", 8U, DataType::bit8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("x43", 8U, DataType::bit8, msw), std::invalid_argument);
  EXPECT_THROW(StringToCANOpenEncodedData("-", 8U, DataType::bit8, msw), std::invalid_argument);
}

TEST(gpcc_cood_data_types_Tests, MapAlternativeDataTypesToOriginalTypes)
{
  // check all mapped data types
  EXPECT_EQ(MapAlternativeDataTypesToOriginalTypes(DataType::boolean_native_bit1), DataType::boolean);

  // check some data types that will not be mapped
  EXPECT_EQ(MapAlternativeDataTypesToOriginalTypes(DataType::unsigned32), DataType::unsigned32);
  EXPECT_EQ(MapAlternativeDataTypesToOriginalTypes(DataType::boolean), DataType::boolean);
  EXPECT_EQ(MapAlternativeDataTypesToOriginalTypes(DataType::bit1), DataType::bit1);
}

TEST(gpcc_cood_data_types_Tests, IsDataTypeBitBased)
{
  // check for "true"
  EXPECT_TRUE(IsDataTypeBitBased(DataType::null));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::boolean));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit1));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit2));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit3));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit4));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit5));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit6));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit7));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::bit8));
  EXPECT_TRUE(IsDataTypeBitBased(DataType::boolean_native_bit1));

  // some random checks for "false"
  EXPECT_FALSE(IsDataTypeBitBased(DataType::unsigned8));
  EXPECT_FALSE(IsDataTypeBitBased(DataType::unsigned16));
  EXPECT_FALSE(IsDataTypeBitBased(DataType::octet_string));
}

TEST(gpcc_cood_data_types_Tests, IsNativeDataStuffed)
{
  // checks for "true"
  EXPECT_TRUE(IsNativeDataStuffed(DataType::null));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit1));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit2));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit3));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit4));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit5));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit6));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit7));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::bit8));
  EXPECT_TRUE(IsNativeDataStuffed(DataType::boolean_native_bit1));

  // some random checks for "false"
  EXPECT_FALSE(IsNativeDataStuffed(DataType::boolean));
  EXPECT_FALSE(IsNativeDataStuffed(DataType::unsigned8));
  EXPECT_FALSE(IsNativeDataStuffed(DataType::unsigned16));
  EXPECT_FALSE(IsNativeDataStuffed(DataType::octet_string));
}

} // namespace cood
} // namespace gpcc_tests
