/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "src/log/internal/RomConstLogMessageTS.hpp"
#include "gtest/gtest.h"

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_RomConstLogMessageTS_Test, Test_OK)
{
  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::RomConstLogMessageTS uut(src, LogType::Info, "Message");
  std::string output = uut.BuildText();
  ASSERT_TRUE(output.size() > 41U);
  output.erase(13, 28);
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message");
}

TEST(gpcc_log_RomConstLogMessageTS_Test, Test_InvalidArgs)
{
  gpcc::string::SharedString src("SRC");
  EXPECT_THROW(gpcc::log::internal::RomConstLogMessageTS uut(src, LogType::Info, nullptr), std::invalid_argument);
}

} // namespace log
} // namespace gpcc_tests
