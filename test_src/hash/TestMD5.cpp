/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2020 Daniel Jerolm
*/

#include <gpcc/hash/md5.hpp>
#include "gpcc/src/string/tools.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <stdexcept>
#include <string>

namespace gpcc_tests {
namespace hash       {

using gpcc::hash::MD5Sum;

// Compares the expected and the actual MD5 with each other.
// Both must be comprised of 16 bytes, otherwise this will throw.
// If both MD5 values match, then "true" will be returned.
// In case of any mismatch, false will be returned and both MD5 values will be printed to stderr.
static bool CompareMD5(std::vector<uint8_t> const & expected,
                       std::vector<uint8_t> const & actual)
{
  if (expected.size() != 16U)
    throw std::invalid_argument("CompareMD5: Length of 'expected' should be 16 bytes");

  if (actual.size() != 16U)
    throw std::invalid_argument("CompareMD5: Length of 'actual' should be 16 bytes");

  if (expected == actual)
    return true;

  std::string message;

  message = "Expected and calculated MD5 are different!\n"\
            "Offset:  ";
  for (uint_fast8_t i = 0U; i < 16U; i++)
  {
    message += ' ';
    if (i < 10U)
      message += ' ';
    message += std::to_string(static_cast<unsigned int>(i));
  }

  message += "\n"\
             "Expected:";

  for (uint_fast8_t i = 0U; i < 16U; i++)
  {
    message += ' ';
    message += gpcc::string::ToHex(expected[i], 2U);
  }

  message += "\n"\
             "Actual:  ";

  for (uint_fast8_t i = 0U; i < 16U; i++)
  {
    message += ' ';
    message += gpcc::string::ToHex(actual[i], 2U);
  }

  message += '\n';

  std::cerr << message;
  return false;
}

// Converts a string containing a chain of hex-values into an std::vector containing uint8_t.
// The hex values are expected with no prefix and no separating spaces. Example: "ab54fe22c6"
static std::vector<uint8_t> StringToVec(std::string const & s)
{
  if ((s.size() % 2U) != 0U)
    throw std::invalid_argument("StringToVec: 's' invalid. Size must be multiple of 2");

  std::vector<uint8_t> retVal;

  size_t pos = 0;
  while (pos != s.size())
  {
    auto val = s.substr(pos, 2U);
    retVal.push_back(gpcc::string::TwoDigitHexToU8(val));
    pos += 2U;
  }

  return retVal;
}


TEST(gpcc_hash_md5_Tests, TestHelper_StringToVec)
{
  // test: All potential digits should be properly recognized
  auto data = StringToVec("0123456789ABCDEFabcdef");
  std::vector<uint8_t> expect = { 0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU, 0xABU, 0xCDU, 0xEFU };

  // test: No data
  data = StringToVec("");
  EXPECT_TRUE(data.empty());

  // test: Invalid number of digits
  EXPECT_THROW((void)StringToVec("012"), std::invalid_argument);

  // test: Invalid digits
  EXPECT_THROW((void)StringToVec("01G3"), std::invalid_argument);
}

TEST(gpcc_hash_md5_Tests, TestHelper_CompareMD5)
{
  std::vector<uint8_t> data1 = StringToVec("d41d8cd98f00b204e9800998ecf8427e");
  std::vector<uint8_t> data2 = data1;

  std::cerr << "<== The following output is by intention" << std::endl;

  // test: Equal data
  EXPECT_TRUE(CompareMD5(data1, data2));

  // test: Last byte does not match
  data2.back() = ~(data1.back());
  EXPECT_FALSE(CompareMD5(data1, data2));
  data2.back() = data1.back();

  // test: Length is not 16 for any of the parameters
  data2.pop_back();
  EXPECT_THROW(CompareMD5(data1, data2), std::invalid_argument);
  EXPECT_THROW(CompareMD5(data2, data1), std::invalid_argument);

  std::cerr << "==> End of intentional error output" << std::endl;
}

TEST(gpcc_hash_md5_Tests, MD5Sum1_NotZeroLengthButnullptr)
{
  EXPECT_THROW((void)MD5Sum(nullptr, 1U), std::invalid_argument);
}

TEST(gpcc_hash_md5_Tests, MD5Sum1_ZeroLength_nullptr)
{
  auto const result = MD5Sum(nullptr, 0U);
  ASSERT_EQ(result.size(), 16U);

  auto const expectation = StringToVec("d41d8cd98f00b204e9800998ecf8427e");
  EXPECT_TRUE(CompareMD5(expectation, result));
}

TEST(gpcc_hash_md5_Tests, MD5Sum1_ZeroLength_not_nullptr)
{
  uint32_t const dummy = 0U;
  auto const result = MD5Sum(&dummy, 0U);
  ASSERT_EQ(result.size(), 16U);

  auto const expectation = StringToVec("d41d8cd98f00b204e9800998ecf8427e");
  EXPECT_TRUE(CompareMD5(expectation, result));
}

TEST(gpcc_hash_md5_Tests, MD5Sum1_InvalidAlignment)
{
  uint32_t const dummy = 0U;
  uint8_t const * const p = reinterpret_cast<uint8_t const *>(&dummy) + 1U;
  ASSERT_TRUE((reinterpret_cast<uintptr_t>(p) % 4U) != 0U) << "Test case failed to setup unaligned pointer.";

  EXPECT_THROW((void)MD5Sum(p, 0U), std::invalid_argument);
}

TEST(gpcc_hash_md5_Tests, MD5Sum1_TestSuite)
{
  // Test patterns from RFC-1321

  auto test = [&](std::string const & data, std::string const & expectedChecksum) -> bool
  {
    auto const expectation = StringToVec(expectedChecksum);
    auto const result = MD5Sum(data.c_str(), data.size());
    return CompareMD5(expectation, result);
  };

  ASSERT_TRUE(test("", "d41d8cd98f00b204e9800998ecf8427e"));
  ASSERT_TRUE(test("a", "0cc175b9c0f1b6a831c399e269772661"));
  ASSERT_TRUE(test("abc", "900150983cd24fb0d6963f7d28e17f72"));
  ASSERT_TRUE(test("message digest", "f96b697d7cb7938d525a2f31aaf161d0"));
  ASSERT_TRUE(test("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"));
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f"));
  ASSERT_TRUE(test("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a"));
}

TEST(gpcc_hash_md5_Tests, MD5Sum1_CornerCases)
{
  auto test = [&](std::string const & data, std::string const & expectedChecksum) -> bool
  {
    auto const expectation = StringToVec(expectedChecksum);
    auto const result = MD5Sum(data.c_str(), data.size());
    return CompareMD5(expectation, result);
  };

  // 63 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789x", "5ab3e2fb8deb311db33030fd3a89bae0"));

  // 64 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789xy", "4dc221a77ac6392aa80726189e06fe4e"));

  // 65 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789xyz", "306026caddffec5f619c60862959ccab"));
}

TEST(gpcc_hash_md5_Tests, MD5Sum2_TestSuite)
{
  // Test patterns from RFC-1321

  auto test = [&](std::string const & data, std::string const & expectedChecksum) -> bool
  {
    std::vector<uint8_t> data_vec;
    data_vec.reserve(data.size());
    for (auto const c : data)
      data_vec.push_back(static_cast<uint8_t>(c));

    auto const expectation = StringToVec(expectedChecksum);
    auto const result = MD5Sum(data_vec);
    return CompareMD5(expectation, result);
  };

  ASSERT_TRUE(test("", "d41d8cd98f00b204e9800998ecf8427e"));
  ASSERT_TRUE(test("a", "0cc175b9c0f1b6a831c399e269772661"));
  ASSERT_TRUE(test("abc", "900150983cd24fb0d6963f7d28e17f72"));
  ASSERT_TRUE(test("message digest", "f96b697d7cb7938d525a2f31aaf161d0"));
  ASSERT_TRUE(test("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"));
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f"));
  ASSERT_TRUE(test("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a"));
}

TEST(gpcc_hash_md5_Tests, MD5Sum2_CornerCases)
{
  // Test patterns from RFC-1321

  auto test = [&](std::string const & data, std::string const & expectedChecksum) -> bool
  {
    std::vector<uint8_t> data_vec;
    data_vec.reserve(data.size());
    for (auto const c : data)
      data_vec.push_back(static_cast<uint8_t>(c));

    auto const expectation = StringToVec(expectedChecksum);
    auto const result = MD5Sum(data_vec);
    return CompareMD5(expectation, result);
  };

  // 63 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789x", "5ab3e2fb8deb311db33030fd3a89bae0"));

  // 64 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789xy", "4dc221a77ac6392aa80726189e06fe4e"));

  // 65 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789xyz", "306026caddffec5f619c60862959ccab"));
}

TEST(gpcc_hash_md5_Tests, MD5Sum3_TestSuite)
{
  // Test patterns from RFC-1321

  auto test = [&](std::string const & data, std::string const & expectedChecksum) -> bool
  {
    std::vector<char> data_vec;
    data_vec.reserve(data.size());
    for (auto const c : data)
      data_vec.push_back(c);

    auto const expectation = StringToVec(expectedChecksum);
    auto const result = MD5Sum(data_vec);
    return CompareMD5(expectation, result);
  };

  ASSERT_TRUE(test("", "d41d8cd98f00b204e9800998ecf8427e"));
  ASSERT_TRUE(test("a", "0cc175b9c0f1b6a831c399e269772661"));
  ASSERT_TRUE(test("abc", "900150983cd24fb0d6963f7d28e17f72"));
  ASSERT_TRUE(test("message digest", "f96b697d7cb7938d525a2f31aaf161d0"));
  ASSERT_TRUE(test("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"));
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f"));
  ASSERT_TRUE(test("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a"));
}

TEST(gpcc_hash_md5_Tests, MD5Sum3_CornerCases)
{
  // Test patterns from RFC-1321

  auto test = [&](std::string const & data, std::string const & expectedChecksum) -> bool
  {
    std::vector<char> data_vec;
    data_vec.reserve(data.size());
    for (auto const c : data)
      data_vec.push_back(c);

    auto const expectation = StringToVec(expectedChecksum);
    auto const result = MD5Sum(data_vec);
    return CompareMD5(expectation, result);
  };

  // 63 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789x", "5ab3e2fb8deb311db33030fd3a89bae0"));

  // 64 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789xy", "4dc221a77ac6392aa80726189e06fe4e"));

  // 65 byte
  ASSERT_TRUE(test("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789xyz", "306026caddffec5f619c60862959ccab"));
}

} // namespace hash
} // namespace gpcc_tests
