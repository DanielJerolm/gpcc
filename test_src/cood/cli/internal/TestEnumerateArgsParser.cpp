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
