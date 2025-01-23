/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include <gpcc/string/BinaryDumper.hpp>
#include <gpcc/compiler/definitions.hpp>
#include <gtest/gtest.h>
#include <stdexcept>

static_assert(GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE, "Tests dumping U16, U32, U64 will fail due to wrong byte order");

namespace gpcc_tests {
namespace string     {

TEST(gpcc_string_BinaryDumper, CTOR_DataPtrInvalid)
{
  // pV is aligned to 64 bit
  uint64_t v = 0;
  uint8_t const * pV = reinterpret_cast<uint8_t const*>(&v);

  // good case
  ASSERT_NO_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 8U));

  // test invalid alignment
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV + 1U, 8U, 0x00U, 2U), std::invalid_argument);
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV + 1U, 8U, 0x00U, 4U), std::invalid_argument);
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV + 1U, 8U, 0x00U, 8U), std::invalid_argument);
}

TEST(gpcc_string_BinaryDumper, CTOR_nBytesInvalid)
{
  // pV is aligned to 64 bit
  uint64_t v[2] = {0, 0};
  uint8_t const * pV = reinterpret_cast<uint8_t const*>(v);

  // good case
  ASSERT_NO_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 8U));

  // test invalid alignment
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV, 3U, 0x00U, 2U), std::invalid_argument);
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV, 6U, 0x00U, 4U), std::invalid_argument);
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV, 9U, 0x00U, 8U), std::invalid_argument);
}

TEST(gpcc_string_BinaryDumper, CTOR_WordSizeInvalid)
{
  // pV is aligned to 64 bit
  uint64_t v = 0;
  uint8_t const * pV = reinterpret_cast<uint8_t const*>(&v);

  // good case
  ASSERT_NO_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 1U));
  ASSERT_NO_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 2U));
  ASSERT_NO_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 4U));
  ASSERT_NO_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 8U));

  // test invalid values
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 3U), std::invalid_argument);
  EXPECT_THROW(gpcc::string::BinaryDumper uut(pV, 8U, 0x00U, 9U), std::invalid_argument);
}

TEST(gpcc_string_BinaryDumper, Headlines_1ByteAddress)
{
  // use CTOR that takes pointer and size
  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10U, 1U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F | 0123456789ABCDEF");
    //                       0x0010: xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx | xxxxxxxxxxxxxxxx
  }

  // use CTOR that does not bind data
  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10U, 2U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address +0   +2   +4   +6   +8   +A   +C   +E   | 0123456789ABCDEF");
    //                       0x0010: xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | xxxxxxxxxxxxxxxx
  }

  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10U, 4U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address +0       +4       +8       +C       | 0123456789ABCDEF");
    //                       0x0010: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | xxxxxxxxxxxxxxxx
  }

  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10U, 8U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address +0               +8               | 0123456789ABCDEF");
    //                       0x0010: xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxx | xxxxxxxxxxxxxxxx
  }
}

TEST(gpcc_string_BinaryDumper, Headlines_4ByteAddress)
{
  // use CTOR that takes pointer and size
  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10000000UL, 1U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address     +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F | 0123456789ABCDEF");
    //                       0x10000000: xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx xx | xxxxxxxxxxxxxxxx
  }

  // use CTOR that does not bind data
  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10000000UL, 2U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address     +0   +2   +4   +6   +8   +A   +C   +E   | 0123456789ABCDEF");
    //                       0x10000000: xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx | xxxxxxxxxxxxxxxx
  }

  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10000000UL, 4U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address     +0       +4       +8       +C       | 0123456789ABCDEF");
    //                       0x10000000: xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | xxxxxxxxxxxxxxxx
  }

  {
    gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10000000UL, 8U);
    auto const s = uut.GetHeadLine();
    EXPECT_STREQ(s.c_str(), "Address     +0               +8               | 0123456789ABCDEF");
    //                       0x10000000: xxxxxxxxxxxxxxxx xxxxxxxxxxxxxxxx | xxxxxxxxxxxxxxxx
  }
}

TEST(gpcc_string_BinaryDumper, Dump_U8_0Bytes)
{
  uint8_t const data[1] =
  {
    0xFFU
  };

  gpcc::string::BinaryDumper uut(data, 0U, 0x10000000UL, 1U);
  std::string s;

  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");

  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U8_0Bytes_nullptr)
{
  gpcc::string::BinaryDumper uut(nullptr, 0U, 0x10000000UL, 1U);
  std::string s;

  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");

  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U8_8Bytes_1ByteAddress)
{
  uint8_t const data[8] =
  {
    0x41U, 0x42U, 0x61U, 0xFFU, 0xABU, 0x21U, 0x7EU, 0x12U
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10U, 1U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x0010: 41 42 61 FF AB 21 7E 12                         | ABa..!~.");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x0020: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x0020: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U8_8Bytes_4ByteAddress)
{
  uint8_t const data[8] =
  {
    0x41U, 0x42U, 0x61U, 0xFFU, 0xABU, 0x21U, 0x7EU, 0x12U
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 1U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 41 42 61 FF AB 21 7E 12                         | ABa..!~.");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U8_16Bytes)
{
  uint8_t const data[16] =
  {
    0x41U, 0x42U, 0x61U, 0xFFU, 0xABU, 0x21U, 0x7EU, 0x12U, 0x30U, 0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 1U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 41 42 61 FF AB 21 7E 12 30 31 32 33 34 35 36 37 | ABa..!~.01234567");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U8_20Bytes)
{
  uint8_t const data[20] =
  {
    0x41U, 0x42U, 0x61U, 0xFFU, 0xABU, 0x21U, 0x7EU, 0x12U, 0x30U, 0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U,
    0x54U, 0x65U, 0x73U, 0x74U
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 1U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 41 42 61 FF AB 21 7E 12 30 31 32 33 34 35 36 37 | ABa..!~.01234567");
  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: 54 65 73 74                                     | Test");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000020: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U16_0Bytes)
{
  uint16_t const data[1] =
  {
    0xFFFFU
  };

  gpcc::string::BinaryDumper uut(data, 0U, 0x10000000UL, 2U);
  std::string s;

  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U16_8Bytes)
{
  uint16_t const data[4] =
  {
    0x4241U, 0xFF61U, 0x21ABU, 0x127EU
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 2U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 4241 FF61 21AB 127E                     | ABa..!~.");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U16_16Bytes)
{
  uint16_t const data[8] =
  {
    0x4241U, 0xFF61U, 0x21ABU, 0x127EU, 0x3130U, 0x3332U, 0x3534U, 0x3736U
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 2U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 4241 FF61 21AB 127E 3130 3332 3534 3736 | ABa..!~.01234567");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U16_20Bytes)
{
  uint16_t const data[10] =
  {
    0x4241U, 0xFF61U, 0x21ABU, 0x127EU, 0x3130U, 0x3332U, 0x3534U, 0x3736U,
    0x6554U, 0x7473U
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 2U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 4241 FF61 21AB 127E 3130 3332 3534 3736 | ABa..!~.01234567");
  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: 6554 7473                               | Test");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000020: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U32_0Bytes)
{
  uint32_t const data[1] =
  {
    0xFFFFFFFFUL
  };

  gpcc::string::BinaryDumper uut(data, 0U, 0x10000000UL, 4U);
  std::string s;

  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U32_8Bytes)
{
  uint32_t const data[2] =
  {
    0xFF614241UL, 0x127E21ABUL
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 4U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: FF614241 127E21AB                   | ABa..!~.");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U32_16Bytes)
{
  uint32_t const data[4] =
  {
    0xFF614241UL, 0x127E21ABUL, 0x33323130UL, 0x37363534UL
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 4U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: FF614241 127E21AB 33323130 37363534 | ABa..!~.01234567");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U32_20Bytes)
{
  uint32_t const data[5] =
  {
    0xFF614241UL, 0x127E21ABUL, 0x33323130UL, 0x37363534UL,
    0x74736554UL
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 4U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: FF614241 127E21AB 33323130 37363534 | ABa..!~.01234567");
  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: 74736554                            | Test");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000020: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U64_0Bytes)
{
  uint64_t const data[1] =
  {
    0xFFFFFFFFFFFFFFFFULL
  };

  gpcc::string::BinaryDumper uut(data, 0U, 0x10000000UL, 8U);
  std::string s;

  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U64_8Bytes)
{
  uint64_t const data[1] =
  {
    0x127E21ABFF614241ULL
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 8U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 127E21ABFF614241                  | ABa..!~.");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U64_16Bytes)
{
  uint64_t const data[2] =
  {
    0x127E21ABFF614241ULL, 0x3736353433323130ULL
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 8U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 127E21ABFF614241 3736353433323130 | ABa..!~.01234567");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

TEST(gpcc_string_BinaryDumper, Dump_U64_24Bytes)
{
  uint64_t const data[3] =
  {
    0x127E21ABFF614241ULL, 0x3736353433323130ULL,
    0x0000000074736554ULL
  };

  gpcc::string::BinaryDumper uut(data, sizeof(data), 0x10000000UL, 8U);
  std::string s;

  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000000: 127E21ABFF614241 3736353433323130 | ABa..!~.01234567");
  ASSERT_FALSE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000010: 0000000074736554                  | Test....");
  ASSERT_TRUE(uut.IsAllDataDumped());

  s = uut.GetLine();
  EXPECT_STREQ(s.c_str(), "0x10000020: ");
  ASSERT_TRUE(uut.IsAllDataDumped());
}

} // namespace string
} // namespace gpcc_tests
