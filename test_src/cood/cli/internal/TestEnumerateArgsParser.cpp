/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/cood/cli/internal/EnumerateArgsParser.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace cood       {
namespace internal   {

using gpcc::cood::internal::EnumerateArgsParser;

TEST(gpcc_cood_cli_internal_EnumerateArgsParser, ValidArgs)
{
  std::unique_ptr<EnumerateArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<EnumerateArgsParser>(""));

  EXPECT_EQ(spUUT->GetFirstIndex(), 0x0000U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0xFFFFU);

  ASSERT_NO_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x0000-0xFFFF"));

  EXPECT_EQ(spUUT->GetFirstIndex(), 0x0000U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0xFFFFU);

  ASSERT_NO_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x0001-0x0001"));

  EXPECT_EQ(spUUT->GetFirstIndex(), 0x0001U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0x0001U);

  ASSERT_NO_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x100-0x0200"));

  EXPECT_EQ(spUUT->GetFirstIndex(), 0x0100U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0x0200U);

  ASSERT_NO_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x100 - 0x0200"));

  EXPECT_EQ(spUUT->GetFirstIndex(), 0x0100U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0x0200U);

  ASSERT_NO_THROW(spUUT = std::make_unique<EnumerateArgsParser>("  0x110 - 0x0202  "));

  EXPECT_EQ(spUUT->GetFirstIndex(), 0x0110U);
  EXPECT_EQ(spUUT->GetLastIndex(), 0x0202U);
}

TEST(gpcc_cood_cli_internal_EnumerateArgsParser, InvalidArgs)
{
  std::unique_ptr<EnumerateArgsParser> spUUT;

  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x10--0x20"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x10-0x20-0x30"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x10-20"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("10-0x20"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x10"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x10-"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("-0x10"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<EnumerateArgsParser>("0x10 0x20"), gpcc::cli::UserEnteredInvalidArgsError);
}

} // namespace internal
} // namespace cood
} // namespace gpcc_tests
