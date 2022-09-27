/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "src/cood/cli/internal/InfoArgsParser.hpp"
#include <gpcc/cli/exceptions.hpp>
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace cood       {
namespace internal   {

using gpcc::cood::internal::InfoArgsParser;

TEST(gpcc_cood_cli_internal_InfoArgsParser, ValidArgs)
{
  std::unique_ptr<InfoArgsParser> spUUT;

  ASSERT_NO_THROW(spUUT = std::make_unique<InfoArgsParser>("0x1000"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetInclASM(), false);

  ASSERT_NO_THROW(spUUT = std::make_unique<InfoArgsParser>("0x10aB"));

  EXPECT_EQ(spUUT->GetIndex(), 0x10ABU);
  EXPECT_EQ(spUUT->GetInclASM(), false);

  ASSERT_NO_THROW(spUUT = std::make_unique<InfoArgsParser>(" 0x1000 "));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetInclASM(), false);

  ASSERT_NO_THROW(spUUT = std::make_unique<InfoArgsParser>("0x1000 asm"));

  EXPECT_EQ(spUUT->GetIndex(), 0x1000U);
  EXPECT_EQ(spUUT->GetInclASM(), true);
}

TEST(gpcc_cood_cli_internal_InfoArgsParser, InvalidArgs)
{
  std::unique_ptr<InfoArgsParser> spUUT;

  EXPECT_THROW(spUUT = std::make_unique<InfoArgsParser>("0x100G"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<InfoArgsParser>("0x1000 ASM"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<InfoArgsParser>("22"), gpcc::cli::UserEnteredInvalidArgsError);
  EXPECT_THROW(spUUT = std::make_unique<InfoArgsParser>("0x1000 asm bla"), gpcc::cli::UserEnteredInvalidArgsError);
}

} // namespace internal
} // namespace cood
} // namespace gpcc_tests
