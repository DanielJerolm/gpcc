/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/log/internal/RomConstLogMessage.hpp"
#include "gtest/gtest.h"

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_RomConstLogMessage_Test, Test_OK)
{
  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::RomConstLogMessage uut(src, LogType::Info, "Message");
  std::string output = uut.BuildText();
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message");
}

TEST(gpcc_log_RomConstLogMessage_Test, Test_InvalidArgs)
{
  gpcc::string::SharedString src("SRC");
  EXPECT_THROW(gpcc::log::internal::RomConstLogMessage uut(src, LogType::Info, nullptr), std::invalid_argument);
}

} // namespace log
} // namespace gpcc_tests
