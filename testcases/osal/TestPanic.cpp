/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/Panic.hpp>
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

#if (__GNUC__ >= 8)
  EXPECT_DEATH(PANIC(), ".*PANIC: gpcc/testcases/osal/TestPanic.cpp.*")
    << "Use -fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=/ in your CMakeLists.txt to cut-off anything in front of project name";
#else
  // -fmacro-prefix-map is not available before GCC 8
  EXPECT_DEATH(PANIC(), ".*PANIC:.*/gpcc/testcases/osal/TestPanic.cpp.*");
#endif
}

TEST(gpcc_osal_Panic_DeathTests, PanicPlusException_Macro)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::runtime_error err("Exception Error Test");

#if (__GNUC__ >= 8)
  EXPECT_DEATH(PANIC_E(err), ".*PANIC: gpcc/testcases/osal/TestPanic.cpp (.*): Exception Error Test.*")
    << "Use -fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=/ in your CMakeLists.txt to cut-off anything in front of project name";
#else
  // -fmacro-prefix-map is not available before GCC 8
  EXPECT_DEATH(PANIC_E(err), ".*PANIC:.*/gpcc/testcases/osal/TestPanic.cpp (.*): Exception Error Test.*");
#endif
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
