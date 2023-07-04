/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/cli/Command.hpp>
#include <gtest/gtest.h>

namespace gpcc {
namespace cli  {
  class CLI;
}
}

namespace gpcc_tests {
namespace cli        {

using namespace testing;
using namespace gpcc::cli;

static void DummyCmdHandler(std::string const & params, gpcc::cli::CLI& cli)
{
  (void)params;
  (void)cli;
}

auto const dummyCmdHandlerFunctor = std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2);

TEST(gpcc_cli_Command_Tests, Construct_nullptr)
{
  std::unique_ptr<Command> spUUT;

  ASSERT_THROW(spUUT = Command::Create(nullptr, "helpText", dummyCmdHandlerFunctor), std::invalid_argument);
  ASSERT_THROW(spUUT = Command::Create("Command", nullptr, dummyCmdHandlerFunctor), std::invalid_argument);
}
TEST(gpcc_cli_Command_Tests, Construct_NoCmdHandler)
{
  std::unique_ptr<Command> spUUT;

  ASSERT_THROW(spUUT = Command::Create("Command", "helpText", Command::tCommandFunc()), std::invalid_argument);
}
TEST(gpcc_cli_Command_Tests, Construct_BadCommandStrings)
{
  std::unique_ptr<Command> spUUT;

  ASSERT_THROW(spUUT = Command::Create("5Test", "helpText", dummyCmdHandlerFunctor), std::invalid_argument);
  ASSERT_THROW(spUUT = Command::Create(" Test", "helpText", dummyCmdHandlerFunctor), std::invalid_argument);
  ASSERT_THROW(spUUT = Command::Create("Test ", "helpText", dummyCmdHandlerFunctor), std::invalid_argument);
  ASSERT_THROW(spUUT = Command::Create("Te st", "helpText", dummyCmdHandlerFunctor), std::invalid_argument);
  ASSERT_THROW(spUUT = Command::Create(""     , "helpText", dummyCmdHandlerFunctor), std::invalid_argument);
}
TEST(gpcc_cli_Command_Tests, Construct_OK)
{
  char const * const pCMD = "Command";
  char const * const pHelp = "help text";

  std::unique_ptr<Command> spUUT(Command::Create(pCMD, pHelp, dummyCmdHandlerFunctor));

  ASSERT_TRUE(pCMD == spUUT->GetCommand());
  ASSERT_TRUE(pHelp == spUUT->GetHelpText());
  ASSERT_TRUE(spUUT->pNext == nullptr);
}
TEST(gpcc_cli_Command_Tests, Construct_OK_noHelpText)
{
  char const * const pCMD = "Command";
  char const * const pHelp = "";

  std::unique_ptr<Command> spUUT(Command::Create(pCMD, pHelp, dummyCmdHandlerFunctor));

  ASSERT_TRUE(pCMD == spUUT->GetCommand());
  ASSERT_TRUE(pHelp == spUUT->GetHelpText());
  ASSERT_TRUE(spUUT->pNext == nullptr);
}

} // namespace cli
} // namespace gpcc_tests
