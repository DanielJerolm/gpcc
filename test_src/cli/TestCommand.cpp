/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "gpcc/src/cli/Command.hpp"
#include "gtest/gtest.h"

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
