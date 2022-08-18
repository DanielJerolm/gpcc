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
#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gtest/gtest.h"
#include <stdexcept>

namespace gpcc_tests {
namespace cli {

using namespace testing;
using namespace gpcc::cli;
using namespace gpcc::time;

static void DummyCmdHandler(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;
  cli.WriteLine("DCH");
}
static void ArgCheckCmdHandler(std::string const & restOfLine, cli::CLI & cli)
{
  (void)cli;

  if (restOfLine == "without details")
  {
    throw UserEnteredInvalidArgsError();
  }
  else if (restOfLine == "details (c-string)")
  {
    throw UserEnteredInvalidArgsError("Test2");
  }
  else if (restOfLine == "details (string copy)")
  {
    std::string const s("Test3");
    throw UserEnteredInvalidArgsError(s);
  }
  else if (restOfLine == "details (string move)")
  {
    std::string s("Test4");
    throw UserEnteredInvalidArgsError(std::move(s));
  }
}
static void FailingCmdHandler(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;
  (void)cli;

  throw std::runtime_error("Intentional error");
}
static void AttemptToUnregisterItselfCmdHandler(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;

  cli.RemoveCommand("UnregisterItselfCommand");
}
static void UnregisterTestCmdCmdHandler(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;

  cli.RemoveCommand("Test");
}
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

// Test fixture for CLI unit tests WITHOUT ICLINotifiable registered
// See TestCLI_withICLINotifiable.cpp for an additional test fixture with additional test cases.
class gpcc_cli_CLI_TestsF: public Test
{
  public:
    gpcc_cli_CLI_TestsF(void);

  protected:
    FakeTerminal terminal;
    cli::CLI uut;
    bool uutRunning;

    std::string params_passed_to_TestCmd;
    std::string result_from_readline;

    gpcc::osal::Mutex sleepyMutex;
    bool sleepyCmdEntered;
    gpcc::osal::ConditionVariable sleepyCmdEnteredCV;

    void SetUp(void) override;
    void TearDown(void) override;

    void StartUUT(void);
    void StopUUT(void);
    void Login(void);

    void TestCmd(std::string const & restOfLine, cli::CLI & cli);
    void ReadLineCmd(std::string const & restOfLine, cli::CLI & cli);
    void SleepyCmd(std::string const & restOfLine, cli::CLI & cli);
};

gpcc_cli_CLI_TestsF::gpcc_cli_CLI_TestsF(void)
: Test()
, terminal(80, 8)
, uut(terminal, 80, 8, "CLI", nullptr)
, uutRunning(false)
, params_passed_to_TestCmd()
, result_from_readline()
, sleepyMutex()
, sleepyCmdEntered(false)
, sleepyCmdEnteredCV()
{
}

void gpcc_cli_CLI_TestsF::SetUp(void)
{
  StartUUT();
  terminal.WaitForInputProcessed();

  uut.AddCommand(Command::Create("Test", " [P1..Pn]\nTest-command", std::bind(&gpcc_cli_CLI_TestsF::TestCmd, this, std::placeholders::_1, std::placeholders::_2)));
  uut.AddCommand(Command::Create("ReadLn", "\nTest-command", std::bind(&gpcc_cli_CLI_TestsF::ReadLineCmd, this, std::placeholders::_1, std::placeholders::_2)));
  uut.AddCommand(Command::Create("Sleep", "\nTest-command", std::bind(&gpcc_cli_CLI_TestsF::SleepyCmd, this, std::placeholders::_1, std::placeholders::_2)));
}
void gpcc_cli_CLI_TestsF::TearDown(void)
{
  StopUUT();

  if (HasFailure())
    terminal.PrintToStdOut();
}

void gpcc_cli_CLI_TestsF::StartUUT(void)
{
  if (uutRunning)
    throw std::logic_error("UUT already started");

  uut.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  uutRunning = true;
}
void gpcc_cli_CLI_TestsF::StopUUT(void)
{
  if (uutRunning)
  {
    uut.Stop();
    uutRunning = false;
  }
}
void gpcc_cli_CLI_TestsF::Login(void)
{
  terminal.Input("login");

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

void gpcc_cli_CLI_TestsF::TestCmd(std::string const & restOfLine, cli::CLI & cli)
{
  params_passed_to_TestCmd = restOfLine;
  cli.WriteLine("ACK");
}
void gpcc_cli_CLI_TestsF::ReadLineCmd(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;
  gpcc::osal::Thread::Sleep_ms(500);
  try
  {
    result_from_readline = cli.ReadLine("Test: ");
  }
  catch (CLIStopError const &)
  {
    cli.WriteLine("Caught CLIStopError");
    throw;
  }
  catch (CtrlCError const &)
  {
    cli.WriteLine("Caught CtrlCError");
  }
}
void gpcc_cli_CLI_TestsF::SleepyCmd(std::string const & restOfLine, cli::CLI & cli)
{
  (void)restOfLine;
  (void)cli;

  gpcc::osal::AdvancedMutexLocker ml(sleepyMutex);
  sleepyCmdEntered = true;
  sleepyCmdEnteredCV.Signal();
  ml.Unlock();

  gpcc::osal::Thread::Sleep_ms(500);
}

TEST_F(gpcc_cli_CLI_TestsF, CreateStartStop)
{
}

// <== Login/Logout related tests.
// Note: There are additional test cases in TestCLI_withICLINotifiable.cpp
TEST_F(gpcc_cli_CLI_TestsF, Login)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">",
   "",
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
TEST_F(gpcc_cli_CLI_TestsF, Login_and_logout)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">logout",
   "Type 'login' or password>",
   "",
   "",
   "",
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
TEST_F(gpcc_cli_CLI_TestsF, Login_WrongInput)
{
  char const * expected[8] =
  {
   "Type 'login' or password>wrong",
   "Type 'login' or password>",
   "",
   "",
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
TEST_F(gpcc_cli_CLI_TestsF, Login_with_password)
{
  char const * expected[8] =
  {
   "Type 'login' or password>PWD",
   "Welcome. Type 'help' for assistance.",
   ">",
   "",
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
TEST_F(gpcc_cli_CLI_TestsF, Login_with_password_but_wrong)
{
  char const * expected[8] =
  {
   "Type 'login' or password>pwd",
   "Wrong password.",
   "Type 'login' or password>",
   "",
   "",
   "",
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
TEST_F(gpcc_cli_CLI_TestsF, Login_with_password_first_wrong_second_right)
{
  char const * expected[8] =
  {
   "Type 'login' or password>pwd",
   "Wrong password.",
   "Type 'login' or password>PWD",
   "Welcome. Type 'help' for assistance.",
   ">",
   "",
   "",
   ""
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
TEST_F(gpcc_cli_CLI_TestsF, Login_with_password_and_logout)
{
  char const * expected[8] =
  {
   "Type 'login' or password>PWD",
   "Welcome. Type 'help' for assistance.",
   ">logout",
   "Type 'login' or password>",
   "",
   "",
   "",
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

TEST_F(gpcc_cli_CLI_TestsF, SetPassword_OK)
{
  uut.SetPassword("PWD");
  ASSERT_TRUE(uut.GetPassword() == "PWD");
}
TEST_F(gpcc_cli_CLI_TestsF, SetPassword_LeadingWhiteSpace)
{
  EXPECT_THROW(uut.SetPassword(" PWD"), std::invalid_argument);
  ASSERT_TRUE(uut.GetPassword() == "");
}
TEST_F(gpcc_cli_CLI_TestsF, SetPassword_TrailingWhiteSpace)
{
  EXPECT_THROW(uut.SetPassword("PWD "), std::invalid_argument);
  ASSERT_TRUE(uut.GetPassword() == "");
}
// ==> Login/Logout related tests.

// <== Tests related to text entry, command entry, and basic command execution
TEST_F(gpcc_cli_CLI_TestsF, Enter_with_no_entry)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">"
  };

  Login();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_TestCmd_with_0param)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_TestCmd_with_1param)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Param",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Param");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Param");
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_TestCmd_with_2param)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Param1 Param2",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Param1 Param2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Param1 Param2");
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_UnknownCommand)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ABC",
   "Unknown command! Enter 'help'!",
   ">"
  };

  Login();
  terminal.Input("ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_Help)
{
  char const * expected[8] =
  {
// "Help:",
// "=====",
// "This is a command line interface compatible with most terminal programs.",
// "For PuTTY, choose the following settings:",
// "* Terminal->Keyboard->Function Keys: \"ESC[n~\" or \"Linux\"",
// "* Terminal: Set checkbox \"Implicit CR in every LF\"",
   "",
   "Implemented commands:",
   "=====================",
   "help, logout, ReadLn, Sleep, Test",
   "",
   "Some commands require parameters. For details about a command, enter the",
   "command plus \"help\". Example: \"HeapStat help\".",
   ">"
  };

  Login();
  terminal.Input("help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_CommandHelp)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test help",
   "Test [P1..Pn]",
   "Test-command",
   ">"
  };

  Login();
  terminal.Input("Test help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_cli_CLI_TestsF, Enter_FlushBefore)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep",
   ">"
  };

  Login();
  terminal.Input("Sleep");
  terminal.Input_ENTER();
  gpcc::osal::Thread::Sleep_ms(250);
  terminal.Input("Flushed stuff...");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
#endif
TEST_F(gpcc_cli_CLI_TestsF, Enter_CaseSensitive)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">test",
   "Unknown command! Enter 'help'!",
   ">"
  };

  Login();
  terminal.Input("test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_UnprintableChars)
{
  char const input[] = { 'T', 0x15, 'e', 0x16, 's', 't', 0x00 };

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input(input);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, Enter_CmdSeqStartNotLost)
{
  char const input[] = { 0x1B, '[', '1', 0x1B, '[', 'D', 'x', 0x00 };

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">[x1"
  };

  Login();
  terminal.Input(input);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Tests related to text entry, command entry, and basic command execution

// <== Tests related to addition and removal of commands
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_Head)
{
  auto spCMD = Command::Create("A_DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">A_DummyCmd",
   "DCH",
   ">"
  };

  Login();
  terminal.Input("A_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_Mid)
{
  auto spCMD = Command::Create("M_DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">M_DummyCmd",
   "DCH",
   ">"
  };

  Login();
  terminal.Input("M_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_Last)
{
  auto spCMD = Command::Create("Z_DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Z_DummyCmd",
   "DCH",
   ">"
  };

  Login();
  terminal.Input("Z_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_Twice)
{
  auto spCMD = Command::Create("DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  spCMD = Command::Create("DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  ASSERT_THROW(uut.AddCommand(std::move(spCMD)), std::logic_error);
}
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_Twice_TestCaseInSensitive)
{
  auto spCMD = Command::Create("DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  spCMD = Command::Create("dummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  ASSERT_THROW(uut.AddCommand(std::move(spCMD)), std::logic_error);
}
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_nullptr)
{
  std::unique_ptr<Command> spCmd;
  ASSERT_THROW(uut.AddCommand(std::move(spCmd)), std::invalid_argument);
}
TEST_F(gpcc_cli_CLI_TestsF, AddCommand_Bad_pNext)
{
  auto spCMD = Command::Create("DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  spCMD->pNext = spCMD.get();
  ASSERT_THROW(uut.AddCommand(std::move(spCMD)), std::logic_error);
}

TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_NotExistOrEmpyStr)
{
  uut.RemoveCommand("ABC");
  uut.RemoveCommand("");

  char const * expected[8] =
  {
// "Help:",
// "=====",
// "This is a command line interface compatible with most terminal programs.",
// "For PuTTY, choose the following settings:",
// "* Terminal->Keyboard->Function Keys: \"ESC[n~\" or \"Linux\"",
// "* Terminal: Set checkbox \"Implicit CR in every LF\"",
   "",
   "Implemented commands:",
   "=====================",
   "help, logout, ReadLn, Sleep, Test",
   "",
   "Some commands require parameters. For details about a command, enter the",
   "command plus \"help\". Example: \"HeapStat help\".",
   ">"
  };

  Login();
  terminal.Input("help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_nullptr)
{
  EXPECT_THROW(uut.RemoveCommand(nullptr), std::invalid_argument);

  char const * expected[8] =
  {
// "Help:",
// "=====",
// "This is a command line interface compatible with most terminal programs.",
// "For PuTTY, choose the following settings:",
// "* Terminal->Keyboard->Function Keys: \"ESC[n~\" or \"Linux\"",
// "* Terminal: Set checkbox \"Implicit CR in every LF\"",
   "",
   "Implemented commands:",
   "=====================",
   "help, logout, ReadLn, Sleep, Test",
   "",
   "Some commands require parameters. For details about a command, enter the",
   "command plus \"help\". Example: \"HeapStat help\".",
   ">"
  };

  Login();
  terminal.Input("help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_Head)
{
  uut.AddCommand(Command::Create("A_DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2)));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">A_DummyCmd",
   "DCH",
   ">A_DummyCmd",
   "Unknown command! Enter 'help'!",
   ">"
  };

  Login();
  terminal.Input("A_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  uut.RemoveCommand("A_DummyCmd");

  terminal.Input("A_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_Mid)
{
  uut.AddCommand(Command::Create("M_DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2)));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">M_DummyCmd",
   "DCH",
   ">M_DummyCmd",
   "Unknown command! Enter 'help'!",
   ">"
  };

  Login();
  terminal.Input("M_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  uut.RemoveCommand("M_DummyCmd");

  terminal.Input("M_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_Last)
{
  uut.AddCommand(Command::Create("Z_DummyCmd", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2)));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">Z_DummyCmd",
   "DCH",
   ">Z_DummyCmd",
   "Unknown command! Enter 'help'!",
   ">"
  };

  Login();
  terminal.Input("Z_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  uut.RemoveCommand("Z_DummyCmd");

  terminal.Input("Z_DummyCmd");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_CaseSensitive)
{
  uut.RemoveCommand("test");

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_BuildIn)
{
  ASSERT_THROW(uut.RemoveCommand("help"), std::logic_error);
  ASSERT_THROW(uut.RemoveCommand("logout"), std::logic_error);

  char const * expected[8] =
  {
// "Help:",
// "=====",
// "This is a command line interface compatible with most terminal programs.",
// "For PuTTY, choose the following settings:",
// "* Terminal->Keyboard->Function Keys: \"ESC[n~\" or \"Linux\"",
// "* Terminal: Set checkbox \"Implicit CR in every LF\"",
   "",
   "Implemented commands:",
   "=====================",
   "help, logout, ReadLn, Sleep, Test",
   "",
   "Some commands require parameters. For details about a command, enter the",
   "command plus \"help\". Example: \"HeapStat help\".",
   ">"
  };

  Login();
  terminal.Input("help");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_BlocksTillExecuted)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep",
   ">"
  };

  Login();
  terminal.Input("Sleep");

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input_ENTER();

  // block until execution of "Sleep" is in process
  gpcc::osal::AdvancedMutexLocker ml(sleepyMutex);
  while (!sleepyCmdEntered)
    sleepyCmdEnteredCV.Wait(sleepyMutex);
  ml.Unlock();

  uut.RemoveCommand("Sleep");
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));

  terminal.WaitForInputProcessed();

  ASSERT_TRUE((end - start).ms() >= 499);
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_Self)
{
  auto spCMD = Command::Create("UnregisterItselfCommand", "\nCommand attempting to unregister itself", std::bind(&AttemptToUnregisterItselfCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">UnregisterItselfCommand",
   "",
   "Error! Caught an exception:",
   "0: CLI::RemoveCommand: Command attempted to remove itself",
   ">"
  };

  Login();
  terminal.Input("UnregisterItselfCommand");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, RemoveCommand_Other)
{
  auto spCMD = Command::Create("UnregisterTestCommand", "\nCommand unregistering command \"Test\"", std::bind(&UnregisterTestCmdCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">UnregisterTestCommand",
   ">Test",
   "Unknown command! Enter 'help'!",
   ">"
  };

  Login();
  terminal.Input("UnregisterTestCommand");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Tests related to addition and removal of commands

// <== Set line head related tests
TEST_F(gpcc_cli_CLI_TestsF, SetLineHead)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   "::ABC"
  };

  Login();
  uut.SetLineHead("::");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ABC");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, SetLineHead_OneWhiteSpace)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   " :ABC"
  };

  Login();
  uut.SetLineHead(" :");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ABC");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, SetLineHead_WhitespacesOnly)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ABC"
  };

  Login();
  ASSERT_THROW(uut.SetLineHead("  "), std::invalid_argument);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ABC");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, SetLineHead_ZeroLength)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ABC"
  };

  Login();
  ASSERT_THROW(uut.SetLineHead(""), std::invalid_argument);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ABC");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, SetLineHead_TooLong)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ABC"
  };

  Login();
  ASSERT_THROW(uut.SetLineHead("1234567890123456789012345678901234567890123456789012345678901234567890123456789"), std::invalid_argument);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ABC");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Set line head related tests

// <== WriteLineComposed() related tests
TEST_F(gpcc_cli_CLI_TestsF, WriteLineComposed1)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "WriteLine",
   ">Test",
   "ACK",
   ">"
  };

  char const * fragments[3];
  fragments[0] = "Write";
  fragments[1] = "Line";
  fragments[2] = nullptr;

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLineComposed(fragments);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLineComposed2)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "WriteLine",
   ">Test",
   "ACK",
   ">"
  };

  std::string const text("Line");
  char const * fragments[3];
  fragments[0] = "Write";
  fragments[1] = text.c_str();
  fragments[2] = nullptr;

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLineComposed(fragments);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLineComposed3)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "WriteLine",
   ">Test",
   "ACK",
   ">"
  };

  std::string const text;

  char const * fragments[5];
  fragments[0] = "Write";
  fragments[1] = text.c_str();
  fragments[2] = "";
  fragments[3] = "Line";
  fragments[4] = nullptr;

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLineComposed(fragments);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLineComposed_nothing)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "",
   ">Test",
   "ACK",
   ">"
  };

  char const * fragments[1];
  fragments[0] = nullptr;

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLineComposed(fragments);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLineComposed_nullptr)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  EXPECT_THROW(uut.WriteLineComposed(nullptr), std::invalid_argument);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
// ==> WriteLineComposed() related tests

// <== WriteLine() related tests
TEST_F(gpcc_cli_CLI_TestsF, WriteLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "WriteLine",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLine("WriteLine");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLine_emptyString)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLine("");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLine_nullptr)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  EXPECT_THROW(uut.WriteLine(nullptr), std::invalid_argument);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLine_newline)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   "WriteLineA",
   "WriteLineB",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLine("WriteLineA\nWriteLineB");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLine_std_string)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "WriteLine",
   ">Test",
   "ACK",
   ">"
  };

  std::string text("WriteLine");
  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLine(text);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLine_std_string_empty)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   "",
   ">Test",
   "ACK",
   ">"
  };

  std::string text;
  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLine(text);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
TEST_F(gpcc_cli_CLI_TestsF, WriteLine_std_string_newline)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   "WriteLineA",
   "WriteLineB",
   ">Test",
   "ACK",
   ">"
  };

  std::string text("WriteLineA\nWriteLineB");
  Login();
  terminal.Input("Test");
  terminal.WaitForInputProcessed();
  uut.WriteLine(text);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
}
// ==> WriteLine() related tests

// <== ReadLine() related tests
TEST_F(gpcc_cli_CLI_TestsF, ReadLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   ">"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(result_from_readline == "Stuff");
}
#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_FlushBefore)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   ">"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  gpcc::osal::Thread::Sleep_ms(250);
  terminal.Input("Invisible Input");
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(result_from_readline == "Stuff");
}
#endif
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_WrongThread)
{
  Login();
  ASSERT_THROW(uut.ReadLine("Test: "), std::runtime_error);
}
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_IgnoreArrowUp)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   ">"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.Input_ArrowUp(1);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(result_from_readline == "Stuff");
}
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_IgnoreArrowDown)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   ">"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.Input_ArrowUp(1);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(result_from_readline == "Stuff");
}
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_IgnoreTAB)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   ">"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.Input_TAB(1);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(result_from_readline == "Stuff");
}
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_CTRL_C)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   "Caught CtrlCError",
   ">"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, ReadLine_Stop)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">ReadLn",
   "Test: Stuff",
   "Caught CLIStopError",
   "Type 'login' or password>"
  };

  Login();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Stuff");
  terminal.WaitForInputProcessed();

  StopUUT();
  StartUUT();

  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> WriteLine() related tests

// <== Tests related to input manipulation and cursor control
TEST_F(gpcc_cli_CLI_TestsF, InsertCharsAtBeginOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Entry");
  terminal.Input_ArrowLeft(5);
  terminal.Input("Test ");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, InsertCharsInMiddleOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test ABC Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Entry");
  terminal.Input_ArrowLeft(6);
  terminal.Input(" ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "ABC Entry");
}

TEST_F(gpcc_cli_CLI_TestsF, BackspaceAtBeginOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Entry");
  terminal.Input_ArrowLeft(10);
  terminal.Input_Backspace(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1,7));
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, BackspaceAt2ndChar)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("ATest Entry");
  terminal.Input_ArrowLeft(10);
  terminal.Input_Backspace(1);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, BackspaceInMiddleOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test ABC Entry");
  terminal.Input_ArrowLeft(6);
  terminal.Input_Backspace(4);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, BackspaceAtEndOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test EntryABC");
  terminal.Input_Backspace(3);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, BackspaceAtEmptyLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input_Backspace(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1,7));
  terminal.Input("Test Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}

TEST_F(gpcc_cli_CLI_TestsF, DELAtBeginOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("ABC Test Entry");
  terminal.Input_ArrowLeft(14);
  terminal.Input_DEL(4);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, DELInMiddleOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test ABC Entry");
  terminal.Input_ArrowLeft(10);
  terminal.Input_DEL(4);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, DELAtLastCharOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test EntryA");
  terminal.Input_ArrowLeft(1);
  terminal.Input_DEL(1);
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, DELAtEndOfLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Entry");
  terminal.Input_DEL(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(11,7));
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, DELAtEmptyLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input_DEL(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1,7));
  terminal.Input("Test Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry");
}

TEST_F(gpcc_cli_CLI_TestsF, MoveCursorBeyondLeftEnd)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test ABC Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("ABC Entry");
  terminal.Input_ArrowLeft(40);
  terminal.Input("Test ");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "ABC Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, MoveCursorBeyondRightEnd)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry ABC",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Entry");
  terminal.Input_ArrowRight(40);
  terminal.Input(" ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry ABC");
}
TEST_F(gpcc_cli_CLI_TestsF, MoveCursorLeftAndBeyondRightEnd)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry ABC",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test Entry");
  terminal.Input_ArrowLeft(2);
  terminal.Input_ArrowRight(40);
  terminal.Input(" ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry ABC");
}
TEST_F(gpcc_cli_CLI_TestsF, MoveCursorLeftAndRight)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry ABC",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test ABC");
  terminal.Input_ArrowLeft(6);
  terminal.Input_ArrowRight(3);
  terminal.Input("Entry ");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry ABC");
}
TEST_F(gpcc_cli_CLI_TestsF, MoveCursorLeftAtEmptyLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry ABC",
   "ACK",
   ">"
  };

  Login();
  terminal.Input_ArrowLeft(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1,7));
  terminal.Input("Test Entry ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry ABC");
}
TEST_F(gpcc_cli_CLI_TestsF, MoveCursorRightAtEmptyLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry ABC",
   "ACK",
   ">"
  };

  Login();
  terminal.Input_ArrowRight(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1,7));
  terminal.Input("Test Entry ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry ABC");
}

TEST_F(gpcc_cli_CLI_TestsF, WriteBeyondEndOfLine)
{
  auto spCMD = Command::Create("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzAbCdEfGhIjKlMnOpQrStUvWxYz", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
  //0         1         2         3         4         5         6         7         8
   ">ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzAbCdEfGhIjKlMnOpQrStUvWxYz",
   "DCH",
   ">"
  };

  Login();
  terminal.Input("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzAbCdEfGhIjKlMnOpQrStUvWxYzE");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(79, 7));
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, InsertIntoFullLine)
{
  auto spCMD = Command::Create("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzAbCdEfGhIjKlMnOpQrStUvWxYz", "\nDummy-command", std::bind(&DummyCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
  //0         1         2         3         4         5         6         7         8
   ">ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzAbCdEfGhIjKlMnOpQrStUvWxYz",
   "DCH",
   ">"
  };

  Login();
  terminal.Input("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwAbCdEfGhIjKlMnOpQrStUvWxYz");
  terminal.Input_ArrowLeft(26);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(76-26, 7));
  terminal.Input("xyzEEE");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(76-26+3, 7));
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_TestsF, POS1)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test ABC Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("ABC Entry");
  terminal.Input_POS1();
  terminal.Input("Test ");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "ABC Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, POS1_emptyLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test ABC Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input_POS1();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1, 7));
  terminal.Input("Test ABC Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "ABC Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, END)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test Entry ABC",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("est Entry");
  terminal.Input_ArrowLeft(30);
  terminal.Input("T");
  terminal.Input_END();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(11, 7));
  terminal.Input(" ABC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "Entry ABC");
}
TEST_F(gpcc_cli_CLI_TestsF, END_emptyLine)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test ABC Entry",
   "ACK",
   ">"
  };

  Login();
  terminal.Input_END();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1, 7));
  terminal.Input("Test ABC Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "ABC Entry");
}
TEST_F(gpcc_cli_CLI_TestsF, POS1_END_OneChar)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">T"
  };

  Login();
  terminal.Input("T");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(2, 7));

  terminal.Input_POS1();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(1, 7));

  terminal.Input_END();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(2, 7));

  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Tests related to input manipulation and cursor control

// <== Tests related to CR/LF sequences
TEST_F(gpcc_cli_CLI_TestsF, CR_LF)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">",
   ">"
  };

  char const cr[] = { 0x0D, 0x00 };
  char const lf[] = { 0x0A, 0x00 };

  Login();
  terminal.Input("Test");
  terminal.Input(cr);
  terminal.WaitForInputProcessed();
  terminal.Input(lf);
  terminal.WaitForInputProcessed();
  terminal.Input(cr);
  terminal.WaitForInputProcessed();
  terminal.Input(lf);
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, LF_CR)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">",
   ">"
  };

  char const cr[] = { 0x0D, 0x00 };
  char const lf[] = { 0x0A, 0x00 };

  Login();
  terminal.Input("Test");
  terminal.Input(lf);
  terminal.WaitForInputProcessed();
  terminal.Input(cr);
  terminal.WaitForInputProcessed();
  terminal.Input(lf);
  terminal.WaitForInputProcessed();
  terminal.Input(cr);
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, CR_CR)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">",
   ">"
  };

  char const cr[] = { 0x0D, 0x00 };
  // char const lf[] = { 0x0A, 0x00 };

  Login();
  terminal.Input("Test");
  terminal.Input(cr);
  terminal.WaitForInputProcessed();
  terminal.Input(cr);
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, LF_LF)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">",
   ">"
  };

  // char const cr[] = { 0x0D, 0x00 };
  char const lf[] = { 0x0A, 0x00 };

  Login();
  terminal.Input("Test");
  terminal.Input(lf);
  terminal.WaitForInputProcessed();
  terminal.Input(lf);
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}
// ==> Tests related to CR/LF sequences

// <== Tests related to exceptions thrown by command callbacks
TEST_F(gpcc_cli_CLI_TestsF, CommandCallbackThrows)
{
  auto spCMD = Command::Create("ITC", "\nIntentionally throwing command", std::bind(&FailingCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">ITC",
   "",
   "Error! Caught an exception:",
   "0: Intentional error",
   ">"
  };

  Login();
  terminal.Input("ITC");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_TestsF, CommandCallbackThrowsUserEnteredInvalidArgsError1)
{
  auto spCMD = Command::Create("ACC", "\nArgument checking command", std::bind(&ArgCheckCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">ACC without details",
   "",
   "Invalid arguments. Try 'ACC help'.",
   "Details:",
   "0: User entered invalid arguments.",
   ">"
  };

  Login();
  terminal.Input("ACC without details");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_TestsF, CommandCallbackThrowsUserEnteredInvalidArgsError2)
{
  auto spCMD = Command::Create("ACC", "\nArgument checking command", std::bind(&ArgCheckCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">ACC details (c-string)",
   "",
   "Invalid arguments. Try 'ACC help'.",
   "Test2",
   ">"
  };

  Login();
  terminal.Input("ACC details (c-string)");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_TestsF, CommandCallbackThrowsUserEnteredInvalidArgsError3)
{
  auto spCMD = Command::Create("ACC", "\nArgument checking command", std::bind(&ArgCheckCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">ACC details (string copy)",
   "",
   "Invalid arguments. Try 'ACC help'.",
   "Test3",
   ">"
  };

  Login();
  terminal.Input("ACC details (string copy)");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_cli_CLI_TestsF, CommandCallbackThrowsUserEnteredInvalidArgsError4)
{
  auto spCMD = Command::Create("ACC", "\nArgument checking command", std::bind(&ArgCheckCmdHandler, std::placeholders::_1, std::placeholders::_2));
  uut.AddCommand(std::move(spCMD));

  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">ACC details (string move)",
   "",
   "Invalid arguments. Try 'ACC help'.",
   "Test4",
   ">"
  };

  Login();
  terminal.Input("ACC details (string move)");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}

// ==> Tests related to exceptions thrown by command callbacks

// <== Tests related to exceptions thrown by ITerminal
TEST_F(gpcc_cli_CLI_TestsF, TerminalReadThrows)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">Blah",
   "ERROR IN CLI:",
   "0: Intentionally thrown exception",
   "",
   "RETRY",
   ">New Input"
  };

  Login();
  terminal.Input("Blah");
  terminal.WaitForInputProcessed();

  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.RequestThrowUponRead();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));

  terminal.Input("New Input");
  terminal.WaitForInputProcessed();

  ASSERT_TRUE((end - start).ms() >= 999);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, TerminalWriteThrows)
{
  char const * expected[8] =
  {
   "Welcome. Type 'help' for assistance.",
   ">",
   "ERROR IN CLI:",
   "0: Terminal Output Error.",
   "1: Intentionally thrown exception",
   "",
   "RETRY",
   ">"
  };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.RequestThrowUponWrite();
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("T");
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE((end - start).ms() >= 999);
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, TerminalFlushThrows)
{
  char const * expected[8] =
  {
   ">Test",
   "ACK",
   ">",
   "ERROR IN CLI:",
   "0: Intentionally thrown exception",
   "",
   "RETRY",
   ">"
  };

  Login();
  terminal.RequestThrowUponFlush();
  TimePoint start(TimePoint::FromSystemClock(Clocks::monotonic));
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  TimePoint end(TimePoint::FromSystemClock(Clocks::monotonic));
  ASSERT_TRUE(terminal.Compare(expected));
  ASSERT_TRUE(params_passed_to_TestCmd == "");
  ASSERT_TRUE((end - start).ms() >= 999);
}
// ==> Tests related to exceptions thrown by ITerminal

// <== Tests related to command history
TEST_F(gpcc_cli_CLI_TestsF, History_InitiallyEmpty_UpDn)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">"
  };

  Login();
  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));

  terminal.Input_ArrowDown(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, History_OneEntry_Up_Enter)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Test"
  };

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Test",
   "ACK",
   ">"
  };

  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));
}
TEST_F(gpcc_cli_CLI_TestsF, History_OneEntry_Down_Enter)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  terminal.Input_ArrowDown(1); // (toward old entries, but we are not iterating command history)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">",
   ">"
  };

  terminal.Input_ENTER(); // (nothing entered, so nothing should be executed)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));
}
TEST_F(gpcc_cli_CLI_TestsF, History_OneEntry_Down_ContentNotChanged)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Sle"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Sle");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Sle"
  };

  terminal.Input_ArrowDown(1); // (toward old entries, but we are not iterating command history)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Sleep",
   ">"
  };

  terminal.Input("ep");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));
}
TEST_F(gpcc_cli_CLI_TestsF, History_OneEntry_Cycle_Up)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Test"
  };

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));
}
TEST_F(gpcc_cli_CLI_TestsF, History_OneEntry_Cycle_Down)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Sle"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Sle");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Test"
  };

  terminal.Input_ArrowUp(1); // (toward latest entered command -> enter command history)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  terminal.Input_ArrowDown(1); // (-> command history is left)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  terminal.Input_ArrowDown(1); // (we are not in command history, nothing should happen)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">Sleep",
   ">"
  };

  terminal.Input("ep");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));
}
TEST_F(gpcc_cli_CLI_TestsF, History_TwoEntry_Cycle_Up)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">ReadLn"
  };

  terminal.Input_ArrowUp(1); // (toward latest entered command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test"
  };

  terminal.Input_ArrowUp(1); // (toward latest entered command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  terminal.Input_ArrowUp(1); // (toward latest entered command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  terminal.Input_ArrowUp(1); // (toward latest entered command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));
}
TEST_F(gpcc_cli_CLI_TestsF, History_TwoEntry_Down)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Bla"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Bla");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test"
  };

  terminal.Input_ArrowUp(2); // (enter command history and move to oldest comman)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">ReadLn"
  };

  terminal.Input_ArrowDown(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  terminal.Input_ArrowDown(1); // (command history is left)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  terminal.Input_ArrowDown(1); // (we are not in command history, nothing should happen)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));
}
TEST_F(gpcc_cli_CLI_TestsF, History_TwoEntry_UpDown)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">"
  };

  Login();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test"
  };

  terminal.Input_ArrowUp(2); // (enter command history and move to oldest command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">ReadLn"
  };

  terminal.Input_ArrowDown(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  terminal.Input_ArrowUp(1); // (move back to oldest command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  terminal.Input_ArrowDown(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));
}
TEST_F(gpcc_cli_CLI_TestsF, History_Update)
{
  char const * expected1[8] =
  {
   ">Sleep",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Sleep");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("ReadLn");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Entry");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">Sleep",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test",
   "ACK",
   ">Test"
  };

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">Sleep",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test",
   "ACK",
   ">ReadLn"
  };

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  char const * expected4[8] =
  {
   ">Sleep",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: Entry",
   ">Test",
   "ACK",
   ">Sleep"
  };

  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected4));

  terminal.Input_ArrowUp(1); // (back to latest command)
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));
}
TEST_F(gpcc_cli_CLI_TestsF, History_EditLeavesHistory)
{
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">Readln",
   "Unknown command! Enter 'help'!",
   ">Test",
   "ACK",
   ">"
  };

  Login();
  terminal.Input("Readln");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Test");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">Readln",
   "Unknown command! Enter 'help'!",
   ">Test",
   "ACK",
   ">Readln"
  };

  terminal.Input_ArrowUp(2);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">Readln",
   "Unknown command! Enter 'help'!",
   ">Test",
   "ACK",
   ">ReadLn"
  };

  terminal.Input_Backspace(2); // edit -> leave command history
  terminal.Input_ArrowDown(1); // we are not in command history -> nothing should happen
  terminal.Input("Ln");
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  char const * expected4[8] =
  {
   ">",
   ">",
   ">Readln",
   "Unknown command! Enter 'help'!",
   ">Test",
   "ACK",
   ">ReadLn",
   "Test: "
  };

  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected4));
}
TEST_F(gpcc_cli_CLI_TestsF, History_SuggestionsEndHistory)
{
  // registered commands: logout, help, Test, Sleep, ReadLn
  //
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">tess",
   "Unknown command! Enter 'help'!",
   ">Sleep",
   ">"
  };

  Login();
  terminal.Input("tess");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("Sleep");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">tess",
   "Unknown command! Enter 'help'!",
   ">Sleep",
   ">tess"
  };
  terminal.Input_ArrowUp(2);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">tess",
   "Unknown command! Enter 'help'!",
   ">Sleep",
   ">Test"
  };
  terminal.Input_TAB(1); // expectation: command history is left
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  char const * expected4[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">tess",
   "Unknown command! Enter 'help'!",
   ">Sleep",
   ">Test"
  };
  terminal.Input_ArrowDown(1); // no reaction expected
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected4));

  char const * expected5[8] =
  {
   ">",
   ">",
   ">tess",
   "Unknown command! Enter 'help'!",
   ">Sleep",
   ">Test",
   "ACK",
   ">"
  };
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected5));
}
// ==> Tests related to command history

// <== Tests related to command suggestion (TAB)
TEST_F(gpcc_cli_CLI_TestsF, Suggestions)
{
  // registered commands: logout, help, Test, Sleep, ReadLn
  //
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Test"
  };

  Login();
  terminal.Input("tess");
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">help"
  };
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep"
  };
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  char const * expected4[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">ReadLn"
  };
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected4));

  char const * expected5[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">logout"
  };
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected5));

  char const * expected6[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">tess"
  };
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected6));

  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));
}
TEST_F(gpcc_cli_CLI_TestsF, Suggestions_HistoryEndsSuggestions)
{
  // registered commands: logout, help, Test, Sleep, ReadLn
  //
  char const * expected1[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep",
   ">Test"
  };

  Login();
  terminal.Input("Sleep");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  terminal.Input("tess");
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected1));

  // Order of suggestion: "tess" => test -> help -> Sleep

  char const * expected2[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep",
   ">Sleep"
  };
  terminal.Input_ArrowUp(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected2));

  char const * expected3[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep",
   ">Sleep"
  };
  terminal.Input_TAB(1);
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected3));

  char const * expected4[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Sleep",
   ">Sleep",
   ">"
  };
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected4));
}
// ==> Tests related to command suggestion (TAB)

// <== Tests related to CTRL+C keystrokes on command prompt
// Note: There are additional test cases in TestCLI_withICLINotifiable.cpp
TEST_F(gpcc_cli_CLI_TestsF, CtrlC_LoggedIn_NoCommand)
{
  char const * expected[8] =
  {
   ">",
   ">",
   ">",
   ">",
   ">",
   ">Input",
   "CTRL+C ignored",
   ">"
  };

  Login();
  terminal.Input("Input");
  terminal.Input_CtrlC();
  terminal.WaitForInputProcessed();
  ASSERT_TRUE(terminal.Compare(expected));
}
TEST_F(gpcc_cli_CLI_TestsF, CtrlC_NotLoggedIn_NoPasswordSetup)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Type 'login' or password>",
   "Type 'login' or password>",
   "",
   "",
   "",
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
TEST_F(gpcc_cli_CLI_TestsF, CtrlC_NotLoggedIn_PasswordSetup)
{
  char const * expected[8] =
  {
   "Type 'login' or password>PWD",
   "Type 'login' or password>",
   "Wrong password.",
   "Type 'login' or password>",
   "",
   "",
   "",
   ""
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
// ==> Tests related to CTRL+C keystrokes on command prompt

// <== Tests related to TestTermination()
TEST_F(gpcc_cli_CLI_TestsF, TestTermination_NoKey)
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
TEST_F(gpcc_cli_CLI_TestsF, TestTermination_RandomUninterestingKeys)
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
TEST_F(gpcc_cli_CLI_TestsF, TestTermination_CTRLC)
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
TEST_F(gpcc_cli_CLI_TestsF, TestTermination_Stop)
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
   ">",
   ">LRC",
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
