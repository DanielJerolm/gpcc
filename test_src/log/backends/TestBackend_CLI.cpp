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

#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/log/backends/Backend_CLI.hpp"
#include "gpcc/src/log/log_levels.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gtest/gtest.h"
#include <functional>

namespace gpcc_tests {
namespace log {

using namespace testing;

// Test fixture for Backend_CLI
class gpcc_log_Backend_CLI_TestsF: public Test
{
  public:
    gpcc_log_Backend_CLI_TestsF(void);

  protected:
    gpcc_tests::cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliRunning;
    gpcc::log::Backend_CLI uut;

    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

gpcc_log_Backend_CLI_TestsF::gpcc_log_Backend_CLI_TestsF(void)
: Test()
, terminal(80, 8)
, cli(terminal, 80, 8, "CLI", nullptr)
, cliRunning(false)
, uut(cli)
{
}
void gpcc_log_Backend_CLI_TestsF::SetUp(void)
{
  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliRunning = true;
  terminal.WaitForInputProcessed();
}
void gpcc_log_Backend_CLI_TestsF::TearDown(void)
{
  if (cliRunning)
    cli.Stop();

  if (HasFailure())
    terminal.PrintToStdOut();
}
void gpcc_log_Backend_CLI_TestsF::Login(void)
{
  terminal.Input("login");

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(gpcc_log_Backend_CLI_TestsF, Instantiation)
{
}
TEST_F(gpcc_log_Backend_CLI_TestsF, Log)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   "Log Message",
   ">"
  };

  Login();
  uut.Process("Log Message", gpcc::log::LogType::Info);
  ASSERT_TRUE(terminal.Compare(expected));
}

} // namespace log
} // namespace gpcc_tests
