/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "src/cood/cli/internal/CAWriteArgsParser.hpp"
#include <gpcc/cli/exceptions.hpp>
#include <gtest/gtest.h>
#include <memory>

namespace gpcc_tests {
namespace cood       {
namespace internal   {

using gpcc::cood::internal::CAWriteArgsParser;

TEST(gpcc_cood_cli_internal_CAWriteArgsParser, ValidArgs)
{
  std::unique_ptr<CAWriteArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<CAWriteArgsParser>("0x1000"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);

  ASSERT_NO_THROW(spUUT = std::make_unique<CAWriteArgsParser>("0x1"));

  EXPECT_EQ(spUUT->GetIndex(), 0x0001U);
}

TEST(gpcc_cood_cli_internal_CAWriteArgsParser, InvalidArgs)
{
  std::unique_ptr<CAWriteArgsParser> spUUT;

  EXPECT_THROW(spUUT = std::make_unique<CAWriteArgsParser>("0x1000:1"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<CAWriteArgsParser>("1000"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<CAWriteArgsParser>("0x100G"), gpcc::cli::UserEnteredInvalidArgsError);
}

} // namespace internal
} // namespace cood
} // namespace gpcc_tests
