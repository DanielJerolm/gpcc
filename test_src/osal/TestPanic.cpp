/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#include "gpcc/src/osal/Panic.hpp"
#include "gtest/gtest.h"
#include <iostream>

namespace gpcc_tests {
namespace osal {

using gpcc::osal::Panic;
using gpcc::osal::GetPanicHandler;
using gpcc::osal::SetPanicHandler;

using namespace testing;

static void TestPanicHandler(char const * const pMessage) noexcept
{
  try
  {
    std::cerr << "TestPanicHandler: ";

    if (pMessage != nullptr)
      std::cerr << pMessage;
    else
      std::cerr << "TestPanicHandler invoked with no message";
  }
  catch (...)
  {
    abort();
  }

  abort();
}

TEST(gpcc_osal_Panic_DeathTests, PanicWithNoMessage)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(Panic(), ".*PANIC: No message.*");
}

TEST(gpcc_osal_Panic_DeathTests, PanicWithMessage)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(Panic("Expected death in unit test"), ".*Expected death in unit test.*");
}

TEST(gpcc_osal_Panic_DeathTests, PanicWithMessage_nullptr)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(Panic(nullptr), ".*PANIC: No message.*");
}

TEST(gpcc_osal_Panic_DeathTests, PanicWithMessageAndException)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::runtime_error err("Exception Error Test");
  EXPECT_DEATH(Panic("Expected death in unit test: ", err), ".*Expected death in unit test: Exception Error Test.*");
}

TEST(gpcc_osal_Panic_DeathTests, PanicWithMessageAndException_nullptr)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::runtime_error err("Exception Error Test");
  EXPECT_DEATH(Panic(nullptr, err), ".*Exception Error Test.*");
}

TEST(gpcc_osal_Panic_DeathTests, Panic_Macro)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(PANIC(), ".*PANIC: gpcc/test_src/osal/TestPanic.cpp.*")
    << "Use -fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=/ in your CMakeLists.txt to cut-off anything in front of project name";
}

TEST(gpcc_osal_Panic_DeathTests, PanicPlusException_Macro)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::runtime_error err("Exception Error Test");
  EXPECT_DEATH(PANIC_E(err), ".*PANIC: gpcc/test_src/osal/TestPanic.cpp (.*): Exception Error Test.*")
    << "Use -fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=/ in your CMakeLists.txt to cut-off anything in front of project name";
}

TEST(gpcc_osal_Panic_DeathTests, SetPanicHandler)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto originalHandler = GetPanicHandler();

  SetPanicHandler(TestPanicHandler);
  EXPECT_DEATH(Panic(), ".*TestPanicHandler: TestPanicHandler invoked with no message.*");
  EXPECT_DEATH(Panic("Expected death in unit test"), ".*TestPanicHandler: Expected death in unit test.*");

  SetPanicHandler(originalHandler);
}

TEST(gpcc_osal_Panic_DeathTests, SetPanicHandler_nullptr)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  EXPECT_DEATH(SetPanicHandler(nullptr), ".*Panic.cpp.*");
}

} // namespace osal
} // namespace gpcc_tests
