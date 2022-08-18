/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/cli/Command.hpp"
#include "gpcc/src/cli/exceptions.hpp"
#include "gpcc/src/cli/ICLINotifiable.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <stdexcept>

namespace gpcc_tests {
namespace cli {

using namespace testing;
using namespace gpcc::cli;
using namespace gpcc::time;

static void LongRunCmdHandler(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;

  for (uint_fast8_t i = 0; i < 10; i++)
  {
    cli.TestTermination();
    gpcc::osal::Thread::Sleep_ms(100);
  }

  cli.WriteLine("DONE");
}

// Test fixture for CLI unit tests WITH ICLINotifiable registered.
class gpcc_cli_CLI_withICLINotifiable_TestsF: public Test, private gpcc::cli::ICLINotifiable
{
  public:

    gpcc_cli_CLI_withICLINotifiable_TestsF(void);

  protected:
    enum class SpecialActions
    {
      none,
      OnBeforePasswordPromptThrows,
      OnWrongPasswordEnteredThrows,
      OnLoginThrows,
      OnLogoutThrows,
      OnCTRL_C_Throws
    };

    FakeTerminal terminal;
    cli::CLI uut;
    bool uutRunning;

    std::atomic<SpecialActions> specialAction;

    void SetUp(void) override;
    void TearDown(void) override;

    void StartUUT(void);
    void StopUUT(void);
    void Login(void);

  private:
    // <-- ICLINotifiable
    void OnBeforePasswordPrompt(CLI & cli) override;
    void OnWrongPasswordEntered(CLI & cli) override;
    void OnLogin(CLI & cli) override;
    void OnLogout(CLI & cli) override;
    void OnCTRL_C(CLI & cli) override;
    // --> ICLINotifable
};

gpcc_cli_CLI_withICLINotifiable_TestsF::gpcc_cli_CLI_withICLINotifiable_TestsF(void)
: Test()
, ICLINotifiable()
, terminal(80, 8)
, uut(terminal, 80, 8, "CLI", this)
, uutRunning(false)
, specialAction(SpecialActions::none)
{
}

void gpcc_cli_CLI_withICLINotifiable_TestsF::SetUp(void)
{
  StartUUT();
  terminal.WaitForInputProcessed();
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::TearDown(void)
{
  StopUUT();

  if (HasFailure())
    terminal.PrintToStdOut();
}

void gpcc_cli_CLI_withICLINotifiable_TestsF::StartUUT(void)
{
  if (uutRunning)
    throw std::logic_error("UUT already running");

  uut.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  uutRunning = true;
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::StopUUT(void)
{
  if (uutRunning)
  {
    uut.Stop();
    uutRunning = false;
  }
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::Login(void)
{
  terminal.Input("login");

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

void gpcc_cli_CLI_withICLINotifiable_TestsF::OnBeforePasswordPrompt(CLI & cli)
{
  cli.WriteLine("OnBeforePasswordPrompt");

  if (specialAction == SpecialActions::OnBeforePasswordPromptThrows)
  {
    specialAction = SpecialActions::none;
    throw std::runtime_error("Intentionally thrown exception");
  }
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::OnWrongPasswordEntered(CLI & cli)
{
  cli.WriteLine("OnWrongPasswordEntered");

  if (specialAction == SpecialActions::OnWrongPasswordEnteredThrows)
  {
    specialAction = SpecialActions::none;
    throw std::runtime_error("Intentionally thrown exception");
  }
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::OnLogin(CLI & cli)
{
  cli.WriteLine("OnLogin");

  if (specialAction == SpecialActions::OnLoginThrows)
  {
    specialAction = SpecialActions::none;
    throw std::runtime_error("Intentionally thrown exception");
  }
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::OnLogout(CLI & cli)
{
  cli.WriteLine("OnLogout");

  if (specialAction == SpecialActions::OnLogoutThrows)
  {
    specialAction = SpecialActions::none;
    throw std::runtime_error("Intentionally thrown exception");
  }
}
void gpcc_cli_CLI_withICLINotifiable_TestsF::OnCTRL_C(CLI & cli)
{
  cli.WriteLine("OnCTRL_C");

  if (specialAction == SpecialActions::OnCTRL_C_Throws)
  {
    specialAction = SpecialActions::none;
    throw std::runtime_error("Intentionally thrown exception");
  }
}


TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, CreateStartStop)
{
}

// <== Login/Logout related tests.
// Note: These are additional test cases for the tests in TestCLI.cpp
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>login",
   "OnLogin",
   ">",
   "",
   "",
   "",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login_and_logout)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>login",
   "OnLogin",
   ">logout",
   "OnLogout",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   ""
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("logout");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login_WrongInput)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>wrong",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   "",
   "",
   "",
   ""
  };

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("wrong");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));

  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 999);
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login_with_password)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>PWD",
   "OnLogin",
   ">",
   "",
   "",
   "",
   ""
  };

  uut.SetPassword("PWD");
  ASSERT_TRUE(uut.GetPassword() == "PWD");

  terminal.Input("PWD");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login_with_password_but_wrong)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>pwd",
   "Wrong password.",
   "OnWrongPasswordEntered",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   "",
   ""
  };

  uut.SetPassword("PWD");
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("pwd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 999);
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login_with_password_first_wrong_second_right)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>pwd",
   "Wrong password.",
   "OnWrongPasswordEntered",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>PWD",
   "OnLogin",
   ">"
  };

  uut.SetPassword("PWD");

  terminal.Input("pwd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("PWD");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, Login_with_password_and_logout)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>PWD",
   "OnLogin",
   ">logout",
   "OnLogout",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   ""
  };

  uut.SetPassword("PWD");

  terminal.Input("PWD");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("logout");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, OnBeforePasswordPromptThrows)
{
  specialAction = SpecialActions::OnBeforePasswordPromptThrows;

  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>bla",
   "OnBeforePasswordPrompt",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>login",
   "OnLogin",
   ">",
   ""
  };

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("bla");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 1999);
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, OnWrongPasswordEnteredThrows)
{
  specialAction = SpecialActions::OnWrongPasswordEnteredThrows;

  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>pwd",
   "Wrong password.",
   "OnWrongPasswordEntered",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   "",
   ""
  };

  uut.SetPassword("PWD");
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("pwd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 999);
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, OnLoginThrows)
{
  specialAction = SpecialActions::OnLoginThrows;

  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>login",
   "OnLogin",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>login",
   "OnLogin",
   ">",
   ""
  };

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() > 999);
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, OnLogoutThrows)
{
  specialAction = SpecialActions::OnLogoutThrows;

  char const * expected[8] =
  {
   "OnLogin",
   ">logout",
   "OnLogout",
   "",
   "Error! Caught an exception:",
   "0: Intentionally thrown exception",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("logout");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Login/Logout related tests.

// <== Tests related to CTRL+C keystrokes on command prompt
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, CtrlC_LoggedIn_NoCommand)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Input",
   "OnCTRL_C",
   ">"
  };

  Login();
  terminal.Input("Input");
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, CtrlC_NotLoggedIn_NoPasswordSetup)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>login",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   "",
   ""
  };

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("login");
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 1999);
}
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, CtrlC_NotLoggedIn_PasswordSetup)
{
  char const * expected[8] =
  {
   "OnBeforePasswordPrompt",
   "Type 'login' or password>PWD",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>",
   "Wrong password.",
   "OnWrongPasswordEntered",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>"
  };

  uut.SetPassword("PWD");

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("PWD");
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 1999);
}

TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, OnCTRL_C_Throws)
{
  specialAction = SpecialActions::OnCTRL_C_Throws;

  char const * expected[8] =
  {
   ">Input",
   "OnCTRL_C",
   "",
   "ERROR IN CLI:",
   "0: Intentionally thrown exception",
   "",
   "RETRY",
   ">"
  };

  Login();
  terminal.Input("Input");
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Tests related to CTRL+C keystrokes on command prompt

// <== Tests related to TestTermination()
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, TestTermination_NoKey)
{
  auto spCMD = Command::Create("LRC", "\nLong running test command", std::bind(&LongRunCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">LRC",
   "DONE",
   ">"
  };

  Login();
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("LRC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 999);
}
#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, TestTermination_RandomUninterestingKeys)
{
  auto spCMD = Command::Create("LRC", "\nLong running test command", std::bind(&LongRunCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">LRC",
   "DONE",
   ">"
  };

  Login();
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("LRC");
  terminal.Input_ENTER();
  gpcc::osal::Thread::Sleep_ms(500);
  terminal.Input("ABC");
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE((end - start).ms() >= 999);
}
#endif
#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, TestTermination_CTRLC)
{
  auto spCMD = Command::Create("LRC", "\nLong running test command", std::bind(&LongRunCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">LRC",
   "Aborted by CTRL+C",
   ">"
  };

  Login();
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("LRC");
  terminal.Input_ENTER();
  gpcc::osal::Thread::Sleep_ms(500);
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));

  ASSERT_TRUE(terminal.Compare(expected));
  TimeSpan const delta = end - start;
  ASSERT_TRUE(delta.ms() >= 499);
  ASSERT_TRUE(delta.ms() < 900);
}
#endif
#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_cli_CLI_withICLINotifiable_TestsF, TestTermination_Stop)
{
  auto spCMD = Command::Create("LRC", "\nLong running test command", std::bind(&LongRunCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">LRC",
   "OnBeforePasswordPrompt",
   "Type 'login' or password>"
  };

  Login();
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("LRC");
  terminal.Input_ENTER();
  gpcc::osal::Thread::Sleep_ms(500);

  StopUUT();
  StartUUT();

  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));

  ASSERT_TRUE(terminal.Compare(expected));
  TimeSpan const delta = end - start;
  ASSERT_TRUE(delta.ms() >= 499);
  ASSERT_TRUE(delta.ms() < 900);
}
#endif
// ==> Tests related to TestTermination()

} // namespace cli
} // namespace gpcc_tests
