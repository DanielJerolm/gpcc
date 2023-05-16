/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "src/cli/internal/ReturnKeyFilter.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cli        {
namespace internal   {

using namespace testing;
using gpcc::cli::internal::TerminalRxParser;
using gpcc::cli::internal::ReturnKeyFilter;

TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, Instantiation)
{
  ReturnKeyFilter uut;
}

TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, NormalInputWithCR)
{
  ReturnKeyFilter uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, NormalInputWithLF)
{
  ReturnKeyFilter uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, NormalInputWithCRLF)
{
  ReturnKeyFilter uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::LF));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, NormalInputWithLFCR)
{
  ReturnKeyFilter uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::CR));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, NeedMoreData)
{
  ReturnKeyFilter uut;

  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NeedMoreData));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::CR));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NeedMoreData));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NeedMoreData));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::CR));
}

TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, CopyConstruction)
{
  ReturnKeyFilter uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));

  ReturnKeyFilter uut2(uut);

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut2.Filter(TerminalRxParser::Result::LF));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, MoveConstruction)
{
  ReturnKeyFilter uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));

  ReturnKeyFilter uut2(std::move(uut));

  EXPECT_FALSE(uut2.Filter(TerminalRxParser::Result::LF));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, CopyAssignment)
{
  ReturnKeyFilter uut;
  ReturnKeyFilter uut2;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));

  uut2 = uut;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::LF));
  EXPECT_FALSE(uut2.Filter(TerminalRxParser::Result::LF));
}
TEST(gpcc_cli_internal_ReturnKeyFilter_Tests, MoveAssignment)
{
  ReturnKeyFilter uut;
  ReturnKeyFilter uut2;

  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_FALSE(uut.Filter(TerminalRxParser::Result::NoCommand));
  EXPECT_TRUE(uut.Filter(TerminalRxParser::Result::CR));

  uut2 = std::move(uut);

  EXPECT_FALSE(uut2.Filter(TerminalRxParser::Result::LF));
}

} // namespace internal
} // namespace cli
} // namespace gpcc_tests
