/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2022 Daniel Jerolm

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

#include "gpcc/src/log/internal/CStringLogMessageTS.hpp"
#include "gtest/gtest.h"
#include <cstdlib>
#include <cstring>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_CStringLogMessageTS_Test, Test_OK)
{
  gpcc::string::SharedString src("SRC");

  std::unique_ptr<char[]> spBuffer(new char[64]);
  strcpy(spBuffer.get(), "Message");

  gpcc::log::internal::CStringLogMessageTS uut(src, LogType::Info, std::move(spBuffer));
  EXPECT_TRUE(!spBuffer) << "spBuffer was not consumed";

  std::string output = uut.BuildText();
  ASSERT_TRUE(output.size() > 41U);
  output.erase(13, 28);
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message");
}

TEST(gpcc_log_CStringLogMessageTS_Test, Test_InvalidArgs)
{
  gpcc::string::SharedString src("SRC");
  EXPECT_THROW(gpcc::log::internal::CStringLogMessageTS uut(src, LogType::Info, nullptr), std::invalid_argument);
}

} // namespace log
} // namespace gpcc_tests
