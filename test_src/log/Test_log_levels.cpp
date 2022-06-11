/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2020, 2021 Daniel Jerolm

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

#include "gpcc/src/log/log_levels.hpp"
#include "gtest/gtest.h"

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_log_levels_Tests, LogType2LogMsgHeader)
{
  std::string s;

  auto check = [&]()
  {
    ASSERT_TRUE(s.size() == logMsgHeaderLength);
    EXPECT_EQ(s.front(), '[');
    EXPECT_EQ(s.back(), ']');
  };

  s = LogType2LogMsgHeader(LogType::Debug);
  ASSERT_NO_FATAL_FAILURE(check());

  s = LogType2LogMsgHeader(LogType::Info);
  ASSERT_NO_FATAL_FAILURE(check());

  s = LogType2LogMsgHeader(LogType::Warning);
  ASSERT_NO_FATAL_FAILURE(check());

  s = LogType2LogMsgHeader(LogType::Error);
  ASSERT_NO_FATAL_FAILURE(check());

  s = LogType2LogMsgHeader(LogType::Fatal);
  ASSERT_NO_FATAL_FAILURE(check());
}

TEST(gpcc_log_log_levels_Tests, LogLevel2String)
{
  EXPECT_STREQ(LogLevel2String(LogLevel::DebugOrAbove), "debug");
  EXPECT_STREQ(LogLevel2String(LogLevel::InfoOrAbove), "info");
  EXPECT_STREQ(LogLevel2String(LogLevel::WarningOrAbove), "warning");
  EXPECT_STREQ(LogLevel2String(LogLevel::ErrorOrAbove), "error");
  EXPECT_STREQ(LogLevel2String(LogLevel::FatalOrAbove), "fatal");
  EXPECT_STREQ(LogLevel2String(LogLevel::Nothing), "nothing");
}

TEST(gpcc_log_log_levels_Tests, String2LogLevel)
{
  EXPECT_EQ(String2LogLevel("debug"), LogLevel::DebugOrAbove);
  EXPECT_EQ(String2LogLevel("info"), LogLevel::InfoOrAbove);
  EXPECT_EQ(String2LogLevel("warning"), LogLevel::WarningOrAbove);
  EXPECT_EQ(String2LogLevel("error"), LogLevel::ErrorOrAbove);
  EXPECT_EQ(String2LogLevel("fatal"), LogLevel::FatalOrAbove);
  EXPECT_EQ(String2LogLevel("nothing"), LogLevel::Nothing);

  EXPECT_THROW((void)String2LogLevel("bad"), std::runtime_error);
}

TEST(gpcc_log_log_levels_Tests, LogLevelConversionCounterparts)
{
  EXPECT_EQ(String2LogLevel(LogLevel2String(LogLevel::DebugOrAbove)),   LogLevel::DebugOrAbove);
  EXPECT_EQ(String2LogLevel(LogLevel2String(LogLevel::InfoOrAbove)),    LogLevel::InfoOrAbove);
  EXPECT_EQ(String2LogLevel(LogLevel2String(LogLevel::WarningOrAbove)), LogLevel::WarningOrAbove);
  EXPECT_EQ(String2LogLevel(LogLevel2String(LogLevel::ErrorOrAbove)),   LogLevel::ErrorOrAbove);
  EXPECT_EQ(String2LogLevel(LogLevel2String(LogLevel::FatalOrAbove)),   LogLevel::FatalOrAbove);
}

} // namespace log
} // namespace gpcc_tests
