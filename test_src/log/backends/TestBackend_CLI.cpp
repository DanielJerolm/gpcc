/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/cli/CLI.hpp>
#include "gpcc/src/log/backends/Backend_CLI.hpp"
#include "gpcc/src/log/log_levels.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <gpcc/raii/scope_guard.hpp>
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
