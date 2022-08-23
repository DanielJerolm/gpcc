/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/cood/cli/internal/WriteArgsParser.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace cood       {
namespace internal   {

using gpcc::cood::internal::WriteArgsParser;

TEST(gpcc_cood_cli_internal_WriteArgsParser, CTOR_ValidArgs)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 \"Test\""));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:255 0x34"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 255U);

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x55:2 0x34"));

  EXPECT_EQ(spUUT->GetIndex(), 0x55U);
  EXPECT_EQ(spUUT->GetSubIndex(), 2U);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, ExtractData_visiblestring_emptyStr)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 \"\""));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  spUUT->ExtractData(gpcc::cood::DataType::visible_string, 16U * 8U, gpcc::Stream::IStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->GetDataSize(), 1U * 8U);
  std::vector<uint8_t> const expected = { 0U };
  EXPECT_TRUE(spUUT->GetData() == expected);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, ExtractData_visiblestring_half)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 \"Test\""));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  spUUT->ExtractData(gpcc::cood::DataType::visible_string, 16U * 8U, gpcc::Stream::IStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->GetDataSize(), 4U * 8U);
  std::vector<uint8_t> const expected = { static_cast<uint8_t>('T'),
                                          static_cast<uint8_t>('e'),
                                          static_cast<uint8_t>('s'),
                                          static_cast<uint8_t>('t') };
  EXPECT_TRUE(spUUT->GetData() == expected);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, ExtractData_visiblestring_full)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 \"Test\""));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  spUUT->ExtractData(gpcc::cood::DataType::visible_string, 4U * 8U, gpcc::Stream::IStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->GetDataSize(), 4U * 8U);
  std::vector<uint8_t> const expected = { static_cast<uint8_t>('T'),
                                          static_cast<uint8_t>('e'),
                                          static_cast<uint8_t>('s'),
                                          static_cast<uint8_t>('t') };
  EXPECT_TRUE(spUUT->GetData() == expected);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, ExtractData_octetstring)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 03 40 05"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  spUUT->ExtractData(gpcc::cood::DataType::octet_string, 0U, gpcc::Stream::IStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->GetDataSize(), 3U * 8U);
  std::vector<uint8_t> const expected = { 0x03U, 0x40U, 0x05U };
  EXPECT_TRUE(spUUT->GetData() == expected);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, ExtractData_unicodestring)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 0340 0512"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  spUUT->ExtractData(gpcc::cood::DataType::unicode_string, 0U, gpcc::Stream::IStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->GetDataSize(), 2 * 16U);
  std::vector<uint8_t> const expected = { 0x40U, 0x03U, 0x12U, 0x05U };
  EXPECT_TRUE(spUUT->GetData() == expected);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, ExtractData_otherTypes)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0 130"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  spUUT->ExtractData(gpcc::cood::DataType::unsigned8, 8U, gpcc::Stream::IStreamWriter::Endian::Little);

  EXPECT_EQ(spUUT->GetDataSize(), 1 * 8U);
  std::vector<uint8_t> const expected = { 130U };
  EXPECT_TRUE(spUUT->GetData() == expected);
}

TEST(gpcc_cood_cli_internal_WriteArgsParser, CTOR_InvalidArgs)
{
  std::unique_ptr<WriteArgsParser> spUUT;

  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("55:12 D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000 D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x100G:0 D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:A D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:0xA D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000 : 2 D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:256 D"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<WriteArgsParser>("0x1000:255"), gpcc::cli::UserEnteredInvalidArgsError);
}

} // namespace internal
} // namespace cood
} // namespace gpcc_tests
