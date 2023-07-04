/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "src/log/internal/StringLogMessageTS.hpp"
#include <gtest/gtest.h>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_StringLogMessageTS_Test, CTOR_Copy)
{
  gpcc::string::SharedString src("SRC");
  std::string const msg("Message");
  gpcc::log::internal::StringLogMessageTS uut(src, LogType::Info, msg);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output.size() > 41U);
  output.erase(13, 28);
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
}

TEST(gpcc_log_StringLogMessageTS_Test, CTOR_Move_1)
{
  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::StringLogMessageTS uut(src, LogType::Info, "Message");
  std::string output = uut.BuildText();
  ASSERT_TRUE(output.size() > 41U);
  output.erase(13, 28);
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
}

TEST(gpcc_log_StringLogMessageTS_Test, CTOR_Move_2)
{
  gpcc::string::SharedString src("SRC");
  std::string msg("Message");
  gpcc::log::internal::StringLogMessageTS uut(src, LogType::Info, std::move(msg));
  std::string output = uut.BuildText();
  ASSERT_TRUE(output.size() > 41U);
  output.erase(13, 28);
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
  EXPECT_TRUE(msg.empty()) << "String should be empty after move";
}

} // namespace log
} // namespace gpcc_tests
