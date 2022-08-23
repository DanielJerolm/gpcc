/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/cood/cli/internal/ReadArgsParser.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace cood       {
namespace internal   {

using gpcc::cood::internal::ReadArgsParser;

TEST(gpcc_cood_cli_internal_ReadArgsParser, ValidArgs)
{
  std::unique_ptr<ReadArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000:0"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 0U);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000:255"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetSubIndex(), 255U);

  ASSERT_NO_THROW(spUUT = std::make_unique<ReadArgsParser>("0x55:2"));

  EXPECT_EQ(spUUT->GetIndex(), 0x55U);
  EXPECT_EQ(spUUT->GetSubIndex(), 2U);
}

TEST(gpcc_cood_cli_internal_ReadArgsParser, InvalidArgs)
{
  std::unique_ptr<ReadArgsParser> spUUT;

  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("55:12"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("0x100G:0"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000:A"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000:0xA"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000 : 2"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<ReadArgsParser>("0x1000:256"), gpcc::cli::UserEnteredInvalidArgsError);
}

} // namespace internal
} // namespace cood
} // namespace gpcc_tests
