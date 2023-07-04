/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "src/log/internal/StringExceptionLogMessage.hpp"
#include <gtest/gtest.h>

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

TEST(gpcc_log_StringExceptionLogMessage_Test, CTOR_Copy_TestWithException)
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
  std::string const msg("Message");
  gpcc::log::internal::StringExceptionLogMessage uut(src, LogType::Info, msg, ePtr);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message\n"\
                        "        1: Throwing 2\n"\
                        "        2: Throwing 1");
}

TEST(gpcc_log_StringExceptionLogMessage_Test, CTOR_Copy_TestWithoutException)
{
  std::exception_ptr ePtr;

  gpcc::string::SharedString src("SRC");
  std::string const msg("Message");
  gpcc::log::internal::StringExceptionLogMessage uut(src, LogType::Info, msg, ePtr);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
}

TEST(gpcc_log_StringExceptionLogMessage_Test, CTOR_Move_1_TestWithoutException)
{
  std::exception_ptr ePtr;

  gpcc::string::SharedString src("SRC");
  gpcc::log::internal::StringExceptionLogMessage uut(src, LogType::Info, "Message", ePtr);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
}

TEST(gpcc_log_StringExceptionLogMessage_Test, CTOR_Move_2_TestWithoutException)
{
  std::exception_ptr ePtr;

  gpcc::string::SharedString src("SRC");
  std::string msg("Message");
  gpcc::log::internal::StringExceptionLogMessage uut(src, LogType::Info, std::move(msg), ePtr);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message");
  EXPECT_TRUE(msg.empty()) << "String should be empty after move";
}

TEST(gpcc_log_StringExceptionLogMessage_Test, TestWithUnknownException)
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
  gpcc::log::internal::StringExceptionLogMessage uut(src, LogType::Info, "Message", ePtr);
  std::string output = uut.BuildText();
  ASSERT_TRUE(output == "[INFO ] SRC: Message\n"\
                        "        1: Unknown exception");
}

} // namespace log
} // namespace gpcc_tests
