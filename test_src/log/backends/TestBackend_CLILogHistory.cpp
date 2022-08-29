/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2020 Daniel Jerolm
*/

#include <gpcc/cli/CLI.hpp>
#include <gpcc/log/backends/Backend_CLILogHistory.hpp>
#include <gpcc/log/log_levels.hpp>
#include "gpcc/src/osal/Thread.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include <gpcc/string/tools.hpp>
#include "gpcc/test_src/fakes/cli/FakeTerminal.hpp"
#include "gtest/gtest.h"
#include <functional>
#include <string>
#include <memory>

namespace gpcc_tests {
namespace log {

using namespace testing;
using gpcc::log::Backend_CLILogHistory;

// Test fixture for Backend_CLILogHistory
class gpcc_log_Backend_CLILogHistory_TestsF: public Test
{
  public:
    gpcc_log_Backend_CLILogHistory_TestsF(void);

  protected:
    gpcc_tests::cli::FakeTerminal terminal;
    gpcc::cli::CLI cli;
    bool cliRunning;
    std::unique_ptr<Backend_CLILogHistory> spUUT;

    char buffer[1024];
    gpcc::Stream::MemStreamWriter buffer_msw;

    void SetUp(void) override;
    void TearDown(void) override;

    void Login(void);
};

gpcc_log_Backend_CLILogHistory_TestsF::gpcc_log_Backend_CLILogHistory_TestsF(void)
: Test()
, terminal(80, 8)
, cli(terminal, 80, 8, "CLI", nullptr)
, cliRunning(false)
, spUUT()
, buffer()
, buffer_msw(buffer, sizeof(buffer) - 1U, gpcc::Stream::MemStreamWriter::nativeEndian)
{
}

void gpcc_log_Backend_CLILogHistory_TestsF::SetUp(void)
{
  cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  cliRunning = true;

  terminal.WaitForInputProcessed();
}

void gpcc_log_Backend_CLILogHistory_TestsF::TearDown(void)
{
  if (cliRunning)
    cli.Stop();

  if (HasFailure())
    terminal.PrintToStdOut();
}

void gpcc_log_Backend_CLILogHistory_TestsF::Login(void)
{
  terminal.Input("login");

  for (uint_fast8_t i = 0U; i < 8U; i++)
  {
    terminal.Input_ENTER();
    terminal.WaitForInputProcessed();
  }
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, TestFixture)
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

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, Instantiation)
{
  ASSERT_THROW(spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 0U, 128U), std::invalid_argument);
  ASSERT_THROW(spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 1U, 127U), std::invalid_argument);

  // instantiate UUT with minimum capacity
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 1U, 128U);
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryEmpty)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "Log history empty.\n" \
                                                        "Remaining capacity: 8 entries or 1024 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryEmpty_n0)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  Login();

  terminal.Input("LogHistory 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 0\n" \
                                                        "Remaining capacity: 8 entries or 1024 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryEmpty_n1)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  Login();

  terminal.Input("LogHistory 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 1\n" \
                                                        "*Log history empty.\n" \
                                                        "Remaining capacity: 8 entries or 1024 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryOneEntry)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "History -1: [DEBUG] Msg_A\n" \
                                                        "Remaining capacity: 7 entries or 1011 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryTwoEntries)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "History -2: [DEBUG] Msg_A\n" \
                                                        "History -1: [INFO ] Msg_B\n" \
                                                        "Remaining capacity: 6 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryTwoEntries_n0)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 0\n" \
                                                        "Remaining capacity: 6 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryTwoEntries_n1)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 1\n" \
                                                        "History: Skipping 1 record(s).\n" \
                                                        "History -1: [INFO ] Msg_B\n" \
                                                        "Remaining capacity: 6 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryTwoEntries_n2)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 2\n" \
                                                        "History -2: [DEBUG] Msg_A\n" \
                                                        "History -1: [INFO ] Msg_B\n" \
                                                        "Remaining capacity: 6 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryTwoEntries_n3)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 3");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 3\n" \
                                                        "History -2: [DEBUG] Msg_A\n" \
                                                        "History -1: [INFO ] Msg_B\n" \
                                                        "Remaining capacity: 6 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithDroppedEntries)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[INFO ] Msg_C", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "History: At least one old message has been discarded.\n" \
                                                        "History -2: [INFO ] Msg_B\n" \
                                                        "History -1: [INFO ] Msg_C\n" \
                                                        "Remaining capacity: 0 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithDroppedEntries_n0)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[INFO ] Msg_C", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 0");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 0\n" \
                                                        "Remaining capacity: 0 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithDroppedEntries_n1)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[INFO ] Msg_C", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 1");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 1\n" \
                                                        "History: Skipping 1 record(s).\n"
                                                        "History -1: [INFO ] Msg_C\n" \
                                                        "Remaining capacity: 0 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithDroppedEntries_n2)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[INFO ] Msg_C", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 2");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 2\n" \
                                                        "History: At least one old message has been discarded.\n" \
                                                        "History -2: [INFO ] Msg_B\n" \
                                                        "History -1: [INFO ] Msg_C\n" \
                                                        "Remaining capacity: 0 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithDroppedEntries_n3)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[INFO ] Msg_C", gpcc::log::LogType::Info);

  Login();

  terminal.Input("LogHistory 3");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();

  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 3\n" \
                                                        "History: At least one old message has been discarded.\n" \
                                                        "History -2: [INFO ] Msg_B\n" \
                                                        "History -1: [INFO ] Msg_C\n" \
                                                        "Remaining capacity: 0 entries or 998 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogLevelsProperlyProcessed_CLI)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*" \
                                                        "History -5: [DEBUG] Msg_A\n" \
                                                        "History -4: [INFO ] Msg_B\n" \
                                                        "History -3: [WARN ] Msg_C\n" \
                                                        "History -2: [ERROR] Msg_D\n" \
                                                        "History -1: [FATAL] Msg_E\n" \
                                                        "*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogLevelsProperlyProcessed_Export)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  spUUT->Export(buffer_msw, false);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "*" \
                                                           "[DEBUG] Msg_A\n" \
                                                           "[INFO ] Msg_B\n" \
                                                           "[WARN ] Msg_C\n" \
                                                           "[ERROR] Msg_D\n" \
                                                           "[FATAL] Msg_E\n" \
                                                           "*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithoutClear)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  // expectation: LogHistory did not clear the log history buffer
  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*" \
                                                        "History -5: [DEBUG] Msg_A\n" \
                                                        "History -4: [INFO ] Msg_B\n" \
                                                        "History -3: [WARN ] Msg_C\n" \
                                                        "History -2: [ERROR] Msg_D\n" \
                                                        "History -1: [FATAL] Msg_E\n" \
                                                        "*", true));

  spUUT->Export(buffer_msw, false);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "*" \
                                                           "[DEBUG] Msg_A\n" \
                                                           "[INFO ] Msg_B\n" \
                                                           "[WARN ] Msg_C\n" \
                                                           "[ERROR] Msg_D\n" \
                                                           "[FATAL] Msg_E\n" \
                                                           "*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithClear)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  Login();

  terminal.Input("LogHistory clear");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*" \
                                                        "History -5: [DEBUG] Msg_A\n" \
                                                        "History -4: [INFO ] Msg_B\n" \
                                                        "History -3: [WARN ] Msg_C\n" \
                                                        "History -2: [ERROR] Msg_D\n" \
                                                        "History -1: [FATAL] Msg_E\n" \
                                                        "Log history cleared.\n*", true));

  // expectation: LogHistory did clear the buffer
  spUUT->Export(buffer_msw, false);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "Log history empty.\n", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, LogHistoryWithClearAndN)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  Login();

  terminal.Input("LogHistory 2 clear");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*" \
                                                        "History: Skipping 3 record(s).\n"
                                                        "History -2: [ERROR] Msg_D\n" \
                                                        "History -1: [FATAL] Msg_E\n" \
                                                        "Log history cleared.\n*", true));

  // expectation: LogHistory did clear the buffer
  spUUT->Export(buffer_msw, false);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "Log history empty.\n", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, Clear1)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);
  spUUT->Clear();

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "Log history empty.\n" \
                                                        "Remaining capacity: 2 entries or 1024 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, Clear2)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);
  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Clear();

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "Log history empty.\n" \
                                                        "Remaining capacity: 2 entries or 1024 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, Clear3)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);
  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Clear();

  spUUT->Process("[INFO ] Msg_D", gpcc::log::LogType::Warning);

  Login();

  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory\n" \
                                                        "History -1: [INFO ] Msg_D\n" \
                                                        "Remaining capacity: 1 entries or 1011 bytes.\n*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, ExportButEmpty)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Export(buffer_msw, true);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "Log history empty.\n", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, ExportWithoutClear)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  Login();

  spUUT->Export(buffer_msw, false);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "*" \
                                                           "[DEBUG] Msg_A\n" \
                                                           "[INFO ] Msg_B\n" \
                                                           "[WARN ] Msg_C\n" \
                                                           "[ERROR] Msg_D\n" \
                                                           "[FATAL] Msg_E\n" \
                                                           "*", true));

  // expecatation: The export did not clear the buffer
  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*" \
                                                        "History -5: [DEBUG] Msg_A\n" \
                                                        "History -4: [INFO ] Msg_B\n" \
                                                        "History -3: [WARN ] Msg_C\n" \
                                                        "History -2: [ERROR] Msg_D\n" \
                                                        "History -1: [FATAL] Msg_E\n" \
                                                        "*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, ExportWithClear)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 8U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[WARN ] Msg_C", gpcc::log::LogType::Warning);
  spUUT->Process("[ERROR] Msg_D", gpcc::log::LogType::Error);
  spUUT->Process("[FATAL] Msg_E", gpcc::log::LogType::Fatal);

  Login();

  spUUT->Export(buffer_msw, true);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "*" \
                                                           "[DEBUG] Msg_A\n" \
                                                           "[INFO ] Msg_B\n" \
                                                           "[WARN ] Msg_C\n" \
                                                           "[ERROR] Msg_D\n" \
                                                           "[FATAL] Msg_E\n" \
                                                           "*", true));

  // expecatation: The export did clear the buffer
  terminal.Input("LogHistory");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*Log history empty*", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, ExportWithDroppedEntries)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  spUUT->Process("[DEBUG] Msg_A", gpcc::log::LogType::Debug);
  spUUT->Process("[INFO ] Msg_B", gpcc::log::LogType::Info);
  spUUT->Process("[INFO ] Msg_C", gpcc::log::LogType::Info);

  spUUT->Export(buffer_msw, false);
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(buffer, "Note: At least one old log message has been removed from the buffer.\n" \
                                                           "[INFO ] Msg_B\n" \
                                                           "[INFO ] Msg_C\n", true));
}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, BadParams1)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  Login();

  terminal.Input("LogHistory A");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory A\n" \
                                                        "*Error*\n" \
                                                        ">\n", false));

}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, BadParams2)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  Login();

  terminal.Input("LogHistory -4");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory -4\n" \
                                                        "*Error*\n" \
                                                        ">\n", false));

}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, BadParams3)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  Login();

  terminal.Input("LogHistory 3 blah");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 3 blah\n" \
                                                        "*Error*\n" \
                                                        ">\n", false));

}

TEST_F(gpcc_log_Backend_CLILogHistory_TestsF, BadParams4)
{
  spUUT = std::make_unique<Backend_CLILogHistory>(&cli, 2U, 1024U);

  Login();

  terminal.Input("LogHistory 3 clear blah");
  terminal.Input_ENTER();
  terminal.WaitForInputProcessed();

  auto str = terminal.GetScreenContent();
  EXPECT_TRUE(gpcc::string::TestSimplePatternMatch(str, "*\n" \
                                                        ">LogHistory 3 clear blah\n" \
                                                        "*Error*\n" \
                                                        ">\n", false));
}

} // namespace log
} // namespace gpcc_tests
