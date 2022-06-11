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
