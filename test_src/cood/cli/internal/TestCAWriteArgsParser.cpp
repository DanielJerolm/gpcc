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
#include "gpcc/src/cood/cli/internal/CAWriteArgsParser.hpp"
#include "gtest/gtest.h"
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
