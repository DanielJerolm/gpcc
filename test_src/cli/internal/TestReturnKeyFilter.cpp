/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "gpcc/src/cli/internal/ReturnKeyFilter.hpp"
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
