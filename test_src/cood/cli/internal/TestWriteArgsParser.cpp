/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "gpcc/src/cli/exceptions.hpp"
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
