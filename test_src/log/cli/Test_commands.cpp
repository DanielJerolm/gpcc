/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/Command.hpp>
#include "gpcc/src/file_systems/linux_fs/FileStorage.hpp"
#include "gpcc/src/file_systems/linux_fs/internal/tools.hpp"
#include "gpcc/src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include "gpcc/src/log/backends/Backend_CLI.hpp"
#include "gpcc/src/log/cli/commands.hpp"
#include "gpcc/src/log/logfacilities/ThreadedLogFacility.hpp"
#include "gpcc/src/log/log_levels.hpp"
#include "gpcc/src/log/Logger.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cerrno>
#include <cstdio>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

// Test fixture for unit tests on CLI commands offered by gpcc/src/log/cli/commands.hpp/.cpp
class gpcc_log_cli_commands_TestsF: public Test
{
  public:
    gpcc_log_cli_commands_TestsF(void);

  protected:
    gpcc::file_systems::linux_fs::internal::UnitTestDirProvider testDirProvider;
    gpcc_tests::cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    Logger logger1;
    Logger logger2;
    Backend_CLI backend;
    ThreadedLogFacility logFacility;

    std::string const baseDir;
    std::unique_ptr<gpcc::file_systems::linux_fs::FileStorage> spFS;

    bool setupComplete;

    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

gpcc_log_cli_commands_TestsF::gpcc_log_cli_commands_TestsF(void)
: Test()
, testDirProvider()
, terminal(80, 8)
, cli(terminal, 80, 8, "CLI", nullptr)
, logger1("logger1")
, logger2("logger2")
, backend(cli)
, logFacility("LFThread", 8)
, baseDir(testDirProvider.GetAbsPath())
, spFS()
, setupComplete(false)
{
}

void gpcc_log_cli_commands_TestsF::SetUp(void)
{
  spFS.reset(new gpcc::file_systems::linux_fs::FileStorage(baseDir));

  logger1.SetLogLevel(LogLevel::Nothing);
  logger2.SetLogLevel(LogLevel::Nothing);

  logFacility.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { logFacility.Unregister(logger1); };

  logFacility.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { logFacility.Unregister(logger2); };

  logFacility.Register(backend);
  ON_SCOPE_EXIT(unregBackend) { logFacility.Unregister(backend); };

  logFacility.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(stopLogFacility) { logFacility.Stop(); };

  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(stopCLI) { cli.Stop(); };

  terminal.WaitForInputProcessed();

  cli.AddCommand(gpcc::cli::Command::Create("logsys", "\nInteractive log system configuration.",
                 std::bind(&gpcc::log::CLI_Cmd_LogCtrl, std::placeholders::_1, std::placeholders::_2, static_cast<gpcc::log::ILogFacilityCtrl*>(&logFacility))));

  cli.AddCommand(gpcc::cli::Command::Create("storeLogConf", " FILENAME\n"\
                                            "Stores the log system configuration into a file referenced by FILENAME.\n"\
                                            "FILENAME will be overwritten if it is already existing.",
                 std::bind(&gpcc::log::CLI_Cmd_WriteConfigToFile, std::placeholders::_1, std::placeholders::_2,
                           static_cast<gpcc::log::ILogFacilityCtrl*>(&logFacility),
                           spFS.get())));

  cli.AddCommand(gpcc::cli::Command::Create("loadLogConf", " FILENAME\n"\
                                            "Loads the log system configuration from a file referenced by FILENAME.",
                 std::bind(&gpcc::log::CLI_Cmd_ReadConfigFromFile, std::placeholders::_1, std::placeholders::_2,
                           static_cast<gpcc::log::ILogFacilityCtrl*>(&logFacility),
                           spFS.get())));

  cli.AddCommand(gpcc::cli::Command::Create("storeLogConfTxt", " FILENAME\n"\
                                            "Stores the log system configuration into a file referenced by FILENAME.\n"\
                                            "FILENAME will be overwritten if it is already existing.",
                 std::bind(&gpcc::log::CLI_Cmd_WriteConfigToTextFile, std::placeholders::_1, std::placeholders::_2,
                           static_cast<gpcc::log::ILogFacilityCtrl*>(&logFacility),
                           spFS.get(),
                           "Headline")));

  cli.AddCommand(gpcc::cli::Command::Create("loadLogConfTxt", " FILENAME\n"\
                                            "Loads the log system configuration from a file referenced by FILENAME.",
                 std::bind(&gpcc::log::CLI_Cmd_ReadConfigFromTextFile, std::placeholders::_1, std::placeholders::_2,
                           static_cast<gpcc::log::ILogFacilityCtrl*>(&logFacility),
                           spFS.get())));

  setupComplete = true;
  ON_SCOPE_EXIT_DISMISS(stopCLI);
  ON_SCOPE_EXIT_DISMISS(stopLogFacility);
  ON_SCOPE_EXIT_DISMISS(unregBackend);
  ON_SCOPE_EXIT_DISMISS(unregLogger2);
  ON_SCOPE_EXIT_DISMISS(unregLogger1);
}

void gpcc_log_cli_commands_TestsF::TearDown(void)
{
  try
  {
    if (HasFailure())
      terminal.PrintToStdOut();

    if (setupComplete)
    {
      cli.Stop();
      logFacility.Stop();
      logFacility.Unregister(logger1);
      logFacility.Unregister(logger2);
      logFacility.Unregister(backend);
    }

    // delete the test folder an all its content.
    spFS.reset();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

void gpcc_log_cli_commands_TestsF::Login(void)
{
  terminal.Input("login");

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_InvokeAndExit)
{
  char const * expected[8] =
  {
    "Currently registered log sources:",
    "Idx  | Log source name | Current log level",
    "-----+-----------------+------------------",
    "0    | logger1         | nothing",
    "1    | logger2         | nothing",
    "Available choices: (Enter nothing in order to leave)",
    "(lower | raise | [set]) D|I|W|E|F|N (index1 [index2 ... n]) | all",
    "Change log settings>"
  };

  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));

  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::Nothing, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_SetOne)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set D 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_SetMultiple)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set D 0 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::DebugOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_SetAll)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set D all");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::DebugOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_SetOne_SetIsDefault)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("D 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_SetMultiple_SetIsDefault)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("D 0 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::DebugOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_SetAll_SetIsDefault)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("D all");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::DebugOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_Set_D)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set D 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_Set_I)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set I 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_Set_W)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set W 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::WarningOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_Set_E)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set E 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::ErrorOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_Set_F)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set F 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::FatalOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_Set_N)
{
  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("set N 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::Nothing, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::Nothing, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_RaiseAll)
{
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("raise W all");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::WarningOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::WarningOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_LowerAll)
{
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("lower I all");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::InfoOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_RaiseOne)
{
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("raise E 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::ErrorOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_LogCtrl_LowerOne)
{
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  Login();
  terminal.Input("logsys");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("lower D 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_EQ(LogLevel::DebugOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::WarningOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_WriteConfigToFile_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">storeLogConf logConfig.dat",
   "done",
   ">",
   "",
   "",
   ""
  };

  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("storeLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // check file content
  auto f = spFS->Open("logConfig.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  uint32_t version = f->Read_uint32();
  size_t nEntries  = static_cast<size_t>(f->Read_uint64());

  EXPECT_EQ(0x00000001UL, version);
  ASSERT_EQ(2UL, nEntries);

  std::string e1 = f->Read_string();
  LogLevel l1 = static_cast<LogLevel>(f->Read_uint8());

  std::string e2 = f->Read_string();
  LogLevel l2 = static_cast<LogLevel>(f->Read_uint8());

  EXPECT_TRUE(e1 == "logger1");
  EXPECT_TRUE(l1 == LogLevel::InfoOrAbove);

  EXPECT_TRUE(e2 == "logger2");
  EXPECT_TRUE(l2 == LogLevel::WarningOrAbove);

  EXPECT_TRUE(f->GetState() == gpcc::Stream::IStreamReader::States::empty);

  // check terminal output
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_WriteConfigToFile_InvalidFileName)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">storeLogConf log Config.dat",
   "Error: Invalid filename",
   ">",
   "",
   "",
   ""
  };

  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("storeLogConf log Config.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConf logConfig.dat",
   "Done",
   ">",
   "",
   "",
   ""
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(2);
  *f << std::string("logger1");
  *f << static_cast<uint8_t>(LogLevel::InfoOrAbove);
  *f << std::string("logger2");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::WarningOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_UnknownLogSources)
{
  char const * expected[8] =
  {
   ">loadLogConf logConfig.dat",
   "The following log sources are unknown:",
   "  abc",
   "  logger11",
   "  logger12",
   "  logger23",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(6);
  *f << std::string("abc");
  *f << static_cast<uint8_t>(LogLevel::InfoOrAbove);
  *f << std::string("logger1");
  *f << static_cast<uint8_t>(LogLevel::InfoOrAbove);
  *f << std::string("logger11");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);
  *f << std::string("logger12");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);
  *f << std::string("logger2");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);
  *f << std::string("logger23");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::WarningOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_CompleteMismatch)
{
  char const * expected[8] =
  {
   "The following log sources are unknown:",
   "  log1",
   "  log2",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "  logger2",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(2);
  *f << std::string("log1");
  *f << static_cast<uint8_t>(LogLevel::InfoOrAbove);
  *f << std::string("log2");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_FirstLogSourceNotInFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConf logConfig.dat",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "Done",
   ">",
   ""
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(1);
  *f << std::string("logger2");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_SecondLogSourceNotInFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConf logConfig.dat",
   "There were no settings provided for the following log sources:",
   "  logger2",
   "Done",
   ">",
   ""
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(1);
  *f << std::string("logger1");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_TwoLogSourcesNotInFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConf logConfig.dat",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "  logger2",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(1);
  *f << std::string("logger3");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();

  Logger logger3("logger3");
  logFacility.Register(logger3);
  ON_SCOPE_EXIT(unregisterLogger3) { logFacility.Unregister(logger3); };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromFile_Mix)
{
  char const * expected[8] =
  {
   "The following log sources are unknown:",
   "  logger0",
   "  logger4",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "  logger2",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.dat", true);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(3);
  *f << std::string("logger0");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);
  *f << std::string("logger3");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);
  *f << std::string("logger4");
  *f << static_cast<uint8_t>(LogLevel::WarningOrAbove);

  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();

  Logger logger3("logger3");
  logFacility.Register(logger3);
  ON_SCOPE_EXIT(unregisterLogger3) { logFacility.Unregister(logger3); };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConf logConfig.dat");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_WriteConfigToTextFile_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">storeLogConfTxt logConfig.txt",
   "done",
   ">",
   "",
   "",
   ""
  };

  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("storeLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // check file content
  auto f = spFS->Open("logConfig.txt");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  // read all lines from the file into a single string
  std::string allLines;
  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    allLines += f->Read_line();
    allLines += '\n';
  }

  EXPECT_TRUE(gpcc::string::StartsWith(allLines, "# Headline")) << "Headline is missing";
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(allLines, "*logger1 : info\n*", true)) << "Entry for logger1 is missing";
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(allLines, "*logger2 : warning\n*", true)) << "Entry for logger2 is missing";
  EXPECT_EQ(gpcc::string::CountChar(allLines, ':'), (6U + 2U)) << "Number of entries is not 2";

  // check terminal output
  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_WriteConfigToTextFile_InvalidFileName)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">storeLogConfTxt log Config.txt",
   "Error: Invalid filename",
   ">",
   "",
   "",
   ""
  };

  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("storeLogConfTxt log Config.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_OK)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConfTxt logConfig.txt",
   "Done",
   ">",
   "",
   "",
   ""
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("logger1 : info");
  f->Write_line("logger2 : warning");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::WarningOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_UnknownLogSources)
{
  char const * expected[8] =
  {
   ">loadLogConfTxt logConfig.txt",
   "The following log sources are unknown:",
   "  abc",
   "  logger11",
   "  logger12",
   "  logger23",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("abc : info");
  f->Write_line("logger1 : info");
  f->Write_line("logger11 : warning");
  f->Write_line("logger12 : warning");
  f->Write_line("logger2 : warning");
  f->Write_line("logger23 : warning");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));

  ASSERT_EQ(LogLevel::InfoOrAbove, logger1.GetLogLevel());
  ASSERT_EQ(LogLevel::WarningOrAbove, logger2.GetLogLevel());
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_CompleteMismatch)
{
  char const * expected[8] =
  {
   "The following log sources are unknown:",
   "  log1",
   "  log2",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "  logger2",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("log1 : info");
  f->Write_line("log2 : warning");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_FirstLogSourceNotInFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConfTxt logConfig.txt",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "Done",
   ">",
   ""
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("logger2 : warning");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_SecondLogSourceNotInFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConfTxt logConfig.txt",
   "There were no settings provided for the following log sources:",
   "  logger2",
   "Done",
   ">",
   ""
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("logger1 : warning");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_TwoLogSourcesNotInFile)
{
  char const * expected[8] =
  {
   "Type 'login' or password>login",
   "Welcome. Type 'help' for assistance.",
   ">loadLogConfTxt logConfig.txt",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "  logger2",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("logger3 : warning");

  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();

  Logger logger3("logger3");
  logFacility.Register(logger3);
  ON_SCOPE_EXIT(unregisterLogger3) { logFacility.Unregister(logger3); };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

TEST_F(gpcc_log_cli_commands_TestsF, CLI_Cmd_ReadConfigFromTextFile_Mix)
{
  char const * expected[8] =
  {
   "The following log sources are unknown:",
   "  logger0",
   "  logger4",
   "There were no settings provided for the following log sources:",
   "  logger1",
   "  logger2",
   "Done",
   ">"
  };

  auto f = spFS->Create("logConfig.txt", true);
  ON_SCOPE_EXIT(closeFile) { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("logger0 : warning");
  f->Write_line("logger3 : warning");
  f->Write_line("logger4 : warning");

  ON_SCOPE_EXIT_DISMISS(closeFile);
  f->Close();

  Logger logger3("logger3");
  logFacility.Register(logger3);
  ON_SCOPE_EXIT(unregisterLogger3) { logFacility.Unregister(logger3); };

  terminal.Input("login");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();
  terminal.Input("loadLogConfTxt logConfig.txt");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  ASSERT_TRUE(terminal.Compare(expected));
}

} // namespace log
} // namespace gpcc_tests
