/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2022 Daniel Jerolm
*/

#include "src/log/internal/CStringLogMessage.hpp"
#include "gtest/gtest.h"
#include <cstdlib>
#include <cstring>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

TEST(gpcc_log_CStringLogMessage_Test, Test_OK)
{
  gpcc::string::SharedString src("SRC");

  std::unique_ptr<char[]> spBuffer(new char[64]);
  strcpy(spBuffer.get(), "Message");

  gpcc::log::internal::CStringLogMessage uut(src, LogType::Info, std::move(spBuffer));
  EXPECT_TRUE(!spBuffer) << "spBuffer was not consumed";

  std::string output = uut.BuildText();
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message");
}

TEST(gpcc_log_CStringLogMessage_Test, Test_InvalidArgs)
{
  gpcc::string::SharedString src("SRC");
  EXPECT_THROW(gpcc::log::internal::CStringLogMessage uut(src, LogType::Info, nullptr), std::invalid_argument);
}

} // namespace log
} // namespace gpcc_tests
