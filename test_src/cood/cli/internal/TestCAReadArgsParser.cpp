/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/cood/cli/internal/CAReadArgsParser.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace cood       {
namespace internal   {

using gpcc::cood::internal::CAReadArgsParser;

TEST(gpcc_cood_cli_internal_CAReadArgsParser, ValidArgs)
{
  std::unique_ptr<CAReadArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<CAReadArgsParser>("0x1000"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetVerbose(), false);

  ASSERT_NO_THROW(spUUT = std::make_unique<CAReadArgsParser>("0x1"));

  EXPECT_EQ(spUUT->GetIndex(), 0x0001U);
  EXPECT_EQ(spUUT->GetVerbose(), false);

  ASSERT_NO_THROW(spUUT = std::make_unique<CAReadArgsParser>("0x10 v"));

  EXPECT_EQ(spUUT->GetIndex(), 0x0010U);
  EXPECT_EQ(spUUT->GetVerbose(), true);
}

TEST(gpcc_cood_cli_internal_CAReadArgsParser, InvalidArgs)
{
  std::unique_ptr<CAReadArgsParser> spUUT;

  EXPECT_THROW(spUUT = std::make_unique<CAReadArgsParser>("0x1000 V"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<CAReadArgsParser>("1000"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<CAReadArgsParser>("0x100G"), gpcc::cli::UserEnteredInvalidArgsError);
}

} // namespace internal
} // namespace cood
} // namespace gpcc_tests
