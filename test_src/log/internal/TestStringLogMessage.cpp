/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2021 Daniel Jerolm

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

#include "gpcc/src/log/internal/StringLogMessage.hpp"
#include "gtest/gtest.h"

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_StringLogMessage_Test, CTOR_Copy)
{
  gpcc::string::SharedString src("SRC");
  std::string const msg("Message");
  gpcc::log::internal::StringLogMessage uut(src, LogType::Info, msg);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
}

TEST(gpcc_log_StringLogMessage_Test, CTOR_Move_1)
{
  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::StringLogMessage uut(src, LogType::Info, "Message");
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
}

TEST(gpcc_log_StringLogMessage_Test, CTOR_Move_2)
{
  gpcc::string::SharedString src("SRC");
  std::string msg("Message");
  gpcc::log::internal::StringLogMessage uut(src, LogType::Info, std::move(msg));
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
  EXPECT_TRUE(msg.empty()) << "String should be empty after move";
}

} // namespace log
} // namespace gpcc_tests
