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
