/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2020 Daniel Jerolm
*/

#include <gpcc/log/log_levels.hpp>
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
