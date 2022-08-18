/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/log/internal/RomConstExceptionLogMessage.hpp"
#include "gtest/gtest.h"

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log        {

namespace
{
  void Throwing1(void)
  {
    throw std::runtime_error("Throwing 1");
  }

  void Throwing2(void)
  {
    try
    {
      Throwing1();
    }
    catch (std::runtime_error const &)
    {
      std::throw_with_nested(std::runtime_error("Throwing 2"));
    }
  }

  void Throwing3(void)
  {
    int i = 5;
    throw i;
  }
}

TEST(gpcc_log_RomConstExceptionLogMessage_Test, TestWithException)
{
  std::exception_ptr ePtr;

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    ePtr = std::current_exception();
  }

  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::RomConstExceptionLogMessage uut(src, LogType::Info, "Message", ePtr);
  std::string output = uut.BuildText();
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message\n"
                               "        1: Throwing 2\n"
                               "        2: Throwing 1");
}

TEST(gpcc_log_RomConstExceptionLogMessage_Test, TestWithoutException)
{
  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::RomConstExceptionLogMessage uut(src, LogType::Info, "Message", nullptr);
  std::string output = uut.BuildText();
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message");
}

TEST(gpcc_log_RomConstExceptionLogMessage_Test, TestWithUnknownException)
{
  std::exception_ptr ePtr;

  try
  {
    Throwing3();
  }
  catch (...)
  {
    ePtr = std::current_exception();
  }

  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::RomConstExceptionLogMessage uut(src, LogType::Info, "Message", ePtr);
  std::string output = uut.BuildText();
  ASSERT_STREQ(output.c_str(), "[INFO ] SRC: Message\n"
                               "        1: Unknown exception");
}

TEST(gpcc_log_RomConstExceptionLogMessage_Test, Test_InvalidArgs)
{
  gpcc::string::SharedString src("SRC");
  EXPECT_THROW(gpcc::log::internal::RomConstExceptionLogMessage uut(src, LogType::Info, nullptr, nullptr), std::invalid_argument);
}

} // namespace log
} // namespace gpcc_tests
