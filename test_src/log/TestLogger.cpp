/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "logfacilities/FakeBackend.hpp"
#include "gpcc/src/log/logfacilities/ThreadedLogFacility.hpp"
#include "gpcc/src/log/Logger.hpp"
#include "gpcc/src/log/log_levels.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gtest/gtest.h"
#include <exception>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

namespace
{
  void Throwing1(void)
  {
    throw std::runtime_error("Throwing 1");
  }

  void Throwing2(void)
  {
    try
    {
      Throwing1();
    }
    catch (std::runtime_error const &)
    {
      std::throw_with_nested(std::runtime_error("Throwing 2"));
    }
  }
}

// Test fixture for class Logger
class gpcc_log_Logger_TestsF: public Test
{
  public:
    gpcc_log_Logger_TestsF(void);

  protected:
    Logger uut;
    FakeBackend backend;
    ThreadedLogFacility logFacility;
    bool setupComplete;

    void SetUp(void) override;
    void TearDown(void) override;

    void PrintBackendRecords(void);
};

gpcc_log_Logger_TestsF::gpcc_log_Logger_TestsF(void)
: uut("uut")
, backend()
, logFacility("LFThread", 8U)
, setupComplete(false)
{
}

void gpcc_log_Logger_TestsF::SetUp(void)
{
  uut.SetLogLevel(LogLevel::InfoOrAbove);

  logFacility.Register(uut);
  ON_SCOPE_EXIT(unregisterUUT) { logFacility.Unregister(uut); };

  logFacility.Register(backend);
  ON_SCOPE_EXIT(unregisterBackend) { logFacility.Unregister(backend); };

  logFacility.Start(gpcc::osal::Thread::SchedPolicy::Other, 0U, gpcc::osal::Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT_DISMISS(unregisterBackend);
  ON_SCOPE_EXIT_DISMISS(unregisterUUT);
  setupComplete = true;
}

void gpcc_log_Logger_TestsF::TearDown(void)
{
  try
  {
    if (setupComplete)
    {
      logFacility.Stop();
      logFacility.Unregister(uut);
      logFacility.Unregister(backend);
    }
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

void gpcc_log_Logger_TestsF::PrintBackendRecords(void)
{
  std::cout << backend.records.size() << " records in fake backend:" << std::endl;
  for (auto & e: backend.records)
    std::cout << e << std::endl;
  std::cout << "END" << std::endl;
}


TEST(gpcc_log_Logger_Tests, Instantiation)
{
  Logger uut("uut");
  ASSERT_EQ(nullptr, uut.GetLogFacility());
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());
}

TEST(gpcc_log_Logger_Tests, BadSourceNames)
{
  std::unique_ptr<Logger> spLogger;

  EXPECT_THROW(spLogger = std::make_unique<Logger>(" uut"), std::invalid_argument);
  EXPECT_THROW(spLogger = std::make_unique<Logger>("uut "), std::invalid_argument);
  EXPECT_THROW(spLogger = std::make_unique<Logger>(""), std::invalid_argument);
  EXPECT_THROW(spLogger = std::make_unique<Logger>(" "), std::invalid_argument);
  EXPECT_THROW(spLogger = std::make_unique<Logger>("u ut"), std::invalid_argument);
  EXPECT_THROW(spLogger = std::make_unique<Logger>("all"), std::invalid_argument);
}

TEST(gpcc_log_Logger_Tests, GetName)
{
  Logger uut("uut");
  ASSERT_TRUE(uut.GetName() == "uut");
}

TEST(gpcc_log_Logger_Tests, SetGetLogLevel)
{
  Logger uut("uut");

  uut.SetLogLevel(LogLevel::DebugOrAbove);
  ASSERT_EQ(LogLevel::DebugOrAbove, uut.GetLogLevel());

  uut.SetLogLevel(LogLevel::InfoOrAbove);
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());

  uut.SetLogLevel(LogLevel::WarningOrAbove);
  ASSERT_EQ(LogLevel::WarningOrAbove, uut.GetLogLevel());

  uut.SetLogLevel(LogLevel::ErrorOrAbove);
  ASSERT_EQ(LogLevel::ErrorOrAbove, uut.GetLogLevel());

  uut.SetLogLevel(LogLevel::FatalOrAbove);
  ASSERT_EQ(LogLevel::FatalOrAbove, uut.GetLogLevel());

  uut.SetLogLevel(LogLevel::Nothing);
  ASSERT_EQ(LogLevel::Nothing, uut.GetLogLevel());
}

TEST(gpcc_log_Logger_Tests, IsAboveLogLevel)
{
  Logger uut("uut");

  uut.SetLogLevel(LogLevel::DebugOrAbove);
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Debug));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Info));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Warning));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Error));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Fatal));

  uut.SetLogLevel(LogLevel::InfoOrAbove);
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Debug));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Info));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Warning));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Error));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Fatal));

  uut.SetLogLevel(LogLevel::WarningOrAbove);
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Debug));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Info));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Warning));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Error));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Fatal));

  uut.SetLogLevel(LogLevel::ErrorOrAbove);
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Debug));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Info));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Warning));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Error));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Fatal));

  uut.SetLogLevel(LogLevel::FatalOrAbove);
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Debug));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Info));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Warning));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Error));
  ASSERT_TRUE(uut.IsAboveLevel(LogType::Fatal));

  uut.SetLogLevel(LogLevel::Nothing);
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Debug));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Info));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Warning));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Error));
  ASSERT_FALSE(uut.IsAboveLevel(LogType::Fatal));
}

TEST(gpcc_log_Logger_Tests, LowerLogLevel)
{
  Logger uut("uut");
  uut.SetLogLevel(LogLevel::InfoOrAbove);

  uut.LowerLogLevel(LogLevel::Nothing);
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());

  uut.LowerLogLevel(LogLevel::WarningOrAbove);
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());

  uut.LowerLogLevel(LogLevel::InfoOrAbove);
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());

  uut.LowerLogLevel(LogLevel::DebugOrAbove);
  ASSERT_EQ(LogLevel::DebugOrAbove, uut.GetLogLevel());
}

TEST(gpcc_log_Logger_Tests, RaiseLogLevel)
{
  Logger uut("uut");
  uut.SetLogLevel(LogLevel::InfoOrAbove);

  uut.RaiseLogLevel(LogLevel::DebugOrAbove);
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());

  uut.RaiseLogLevel(LogLevel::InfoOrAbove);
  ASSERT_EQ(LogLevel::InfoOrAbove, uut.GetLogLevel());

  uut.RaiseLogLevel(LogLevel::WarningOrAbove);
  ASSERT_EQ(LogLevel::WarningOrAbove, uut.GetLogLevel());

  uut.RaiseLogLevel(LogLevel::Nothing);
  ASSERT_EQ(LogLevel::Nothing, uut.GetLogLevel());
}

TEST(gpcc_log_Logger_Tests, LogButNoLogFacility)
{
  Logger uut("uut");
  uut.SetLogLevel(LogLevel::DebugOrAbove);

  std::string s("Test");

  uut.Log(LogType::Info, "Test");
  uut.Log(LogType::Info, "Test", std::exception_ptr());
  uut.Log(LogType::Info, s);
  uut.Log(LogType::Info, std::move(s));
  s = "Test";

  uut.Log(LogType::Info, s, std::exception_ptr());
  uut.Log(LogType::Info, std::move(s), std::exception_ptr());
  s = "Test";

  uut.LogTS(LogType::Info, "Test");
  uut.LogTS(LogType::Info, "Test", std::exception_ptr());
  uut.LogTS(LogType::Info, s);
  uut.LogTS(LogType::Info, std::move(s));
  s = "Test";

  uut.LogTS(LogType::Info, s, std::exception_ptr());
  uut.LogTS(LogType::Info, std::move(s), std::exception_ptr());

}

TEST_F(gpcc_log_Logger_TestsF, GetLogFacility)
{
  ASSERT_TRUE(uut.GetLogFacility() == &logFacility);
}

TEST_F(gpcc_log_Logger_TestsF, Log_cstring)
{
  uut.Log(LogType::Debug, "This should be dropped.");
  uut.Log(LogType::Info, "Log1");
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
}

TEST_F(gpcc_log_Logger_TestsF, Log_cstring_plus_eptr)
{
  uut.Log(LogType::Debug, "This should be dropped.", std::exception_ptr());
  uut.Log(LogType::Info, "Log1", std::exception_ptr());

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    uut.Log(LogType::Info, "Log2", std::current_exception());
  }

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log2\n"\
                                    "        1: Throwing 2\n"\
                                    "        2: Throwing 1");
}

TEST_F(gpcc_log_Logger_TestsF, Log_stdstring_copy)
{
  std::string const s1 = "This should be dropped.";
  uut.Log(LogType::Debug, s1);

  std::string const s2 = "Log1";
  uut.Log(LogType::Info, s2);
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
}

TEST_F(gpcc_log_Logger_TestsF, Log_stdstring_move)
{
  std::string s1 = "This should be dropped.";
  uut.Log(LogType::Debug, std::move(s1));

  std::string s2 = "Log1";
  uut.Log(LogType::Info, std::move(s2));
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
}

TEST_F(gpcc_log_Logger_TestsF, Log_stdstring_copy_plus_eptr)
{
  std::string const s0 = "This should be dropped.";
  uut.Log(LogType::Debug, s0, std::exception_ptr());

  std::string const s1 = "Log1";
  std::string const s2 = "Log2";

  uut.Log(LogType::Info, s1, std::exception_ptr());

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    uut.Log(LogType::Info, s2, std::current_exception());
  }

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log2\n"\
                                    "        1: Throwing 2\n"\
                                    "        2: Throwing 1");
}

TEST_F(gpcc_log_Logger_TestsF, Log_stdstring_move_plus_eptr)
{
  std::string const s0 = "This should be dropped.";
  uut.Log(LogType::Debug, std::move(s0), std::exception_ptr());

  std::string s1 = "Log1";
  std::string s2 = "Log2";

  uut.Log(LogType::Info, std::move(s1), std::exception_ptr());

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    uut.Log(LogType::Info, std::move(s2), std::current_exception());
  }

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log2\n"\
                                    "        1: Throwing 2\n"\
                                    "        2: Throwing 1");
}

TEST_F(gpcc_log_Logger_TestsF, Log_VariableArgs)
{
  uint32_t u32a = 48U;
  uint32_t u32b = 21U;

  uut.LogV(LogType::Debug, "This should be dropped.");

  uut.LogV(LogType::Info, "Log");
  uut.LogV(LogType::Info, "Log %u", static_cast<unsigned int>(u32a));
  uut.LogV(LogType::Info, "Log %u %u %%", static_cast<unsigned int>(u32a), static_cast<unsigned int>(u32b));
  logFacility.Flush();

  ASSERT_EQ(3U, backend.records.size());
  ASSERT_STREQ(backend.records[0].c_str(), "[INFO ] uut: Log");
  ASSERT_STREQ(backend.records[1].c_str(), "[INFO ] uut: Log 48");
  ASSERT_STREQ(backend.records[2].c_str(), "[INFO ] uut: Log 48 21 %");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_cstring)
{
  uut.LogTS(LogType::Debug, "This should be dropped.");
  uut.LogTS(LogType::Info, "Log1");
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());

  ASSERT_GT(backend.records[0].size(), 13U+28U+1U);
  backend.records[0].erase(13, 28);

  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_cstring_plus_eptr)
{
  uut.LogTS(LogType::Debug, "This should be dropped.", std::exception_ptr());
  uut.LogTS(LogType::Info, "Log1", std::exception_ptr());

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    uut.LogTS(LogType::Info, "Log2", std::current_exception());
  }

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());

  ASSERT_GT(backend.records[0].size(), 13U+28U+1U);
  backend.records[0].erase(13, 28);
  ASSERT_GT(backend.records[1].size(), 13U+28U+1U);
  backend.records[1].erase(13, 28);

  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log2\n"\
                                    "        1: Throwing 2\n"\
                                    "        2: Throwing 1");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_stdstring_copy)
{
  std::string const s1 = "This should be dropped.";
  uut.LogTS(LogType::Debug, s1);

  std::string const s2 = "Log1";
  uut.LogTS(LogType::Info, s2);
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());

  ASSERT_GT(backend.records[0].size(), 13U+28U+1U);
  backend.records[0].erase(13, 28);

  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_stdstring_move)
{
  std::string s1 = "This should be dropped.";
  uut.LogTS(LogType::Debug, std::move(s1));

  std::string s2 = "Log1";
  uut.LogTS(LogType::Info, std::move(s2));
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());

  ASSERT_GT(backend.records[0].size(), 13U+28U+1U);
  backend.records[0].erase(13, 28);

  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_stdstring_copy_plus_eptr)
{
  std::string const s0 = "This should be dropped.";
  uut.LogTS(LogType::Debug, s0, std::exception_ptr());

  std::string const s1 = "Log1";
  std::string const s2 = "Log2";

  uut.LogTS(LogType::Info, s1, std::exception_ptr());

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    uut.LogTS(LogType::Info, s2, std::current_exception());
  }

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());

  ASSERT_GT(backend.records[0].size(), 13U+28U+1U);
  backend.records[0].erase(13, 28);
  ASSERT_GT(backend.records[1].size(), 13U+28U+1U);
  backend.records[1].erase(13, 28);

  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log2\n"\
                                    "        1: Throwing 2\n"\
                                    "        2: Throwing 1");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_stdstring_move_plus_eptr)
{
  std::string s0 = "This should be dropped.";
  uut.LogTS(LogType::Debug, std::move(s0), std::exception_ptr());

  std::string s1 = "Log1";
  std::string s2 = "Log2";

  uut.LogTS(LogType::Info, std::move(s1), std::exception_ptr());

  try
  {
    Throwing2();
  }
  catch (std::exception const &)
  {
    uut.LogTS(LogType::Info, std::move(s2), std::current_exception());
  }

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());

  ASSERT_GT(backend.records[0].size(), 13U+28U+1U);
  backend.records[0].erase(13, 28);
  ASSERT_GT(backend.records[1].size(), 13U+28U+1U);
  backend.records[1].erase(13, 28);

  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log1");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log2\n"\
                                    "        1: Throwing 2\n"\
                                    "        2: Throwing 1");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_VariableArgs)
{
  uint32_t u32a = 48U;
  uint32_t u32b = 21U;

  uut.LogVTS(LogType::Debug, "This should be dropped.");

  uut.LogVTS(LogType::Info, "Log");
  uut.LogVTS(LogType::Info, "Log %u", static_cast<unsigned int>(u32a));
  uut.LogVTS(LogType::Info, "Log %u %u %%", static_cast<unsigned int>(u32a), static_cast<unsigned int>(u32b));
  logFacility.Flush();

  ASSERT_EQ(3U, backend.records.size());
  backend.records[0].erase(13, 28);
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log");
  backend.records[1].erase(13, 28);
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log 48");
  backend.records[2].erase(13, 28);
  ASSERT_TRUE(backend.records[2] == "[INFO ] uut: Log 48 21 %");
}

TEST_F(gpcc_log_Logger_TestsF, LogFailed)
{
  uut.LogFailed();
  logFacility.Flush();

  ASSERT_EQ(1U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[ERROR] *** Logger: 1 error(s) during log message creation (e.g. out-of-memory) ***");
}

TEST_F(gpcc_log_Logger_TestsF, Log_VariableArgs_Macro)
{
  volatile bool t = true;

  uint32_t u32a = 48U;
  uint32_t u32b = 21U;
  LOGV(uut, LogType::Debug, "This should be dropped. %u", static_cast<unsigned int>(u32a));
  LOGV(uut, LogType::Info, "Log %u", static_cast<unsigned int>(u32a));

  if (t)
    LOGV(uut, LogType::Info, "Log %u %u %%", static_cast<unsigned int>(u32a), static_cast<unsigned int>(u32b));

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log 48");
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log 48 21 %");
}

TEST_F(gpcc_log_Logger_TestsF, LogTS_VariableArgs_Macro)
{
  volatile bool t = true;

  uint32_t u32a = 48U;
  uint32_t u32b = 21U;
  LOGVTS(uut, LogType::Debug, "This should be dropped. %u", static_cast<unsigned int>(u32a));
  LOGVTS(uut, LogType::Info, "Log %u", static_cast<unsigned int>(u32a));

  if (t)
    LOGVTS(uut, LogType::Info, "Log %u %u %%", static_cast<unsigned int>(u32a), static_cast<unsigned int>(u32b));

  logFacility.Flush();

  ASSERT_EQ(2U, backend.records.size());
  backend.records[0].erase(13, 28);
  ASSERT_TRUE(backend.records[0] == "[INFO ] uut: Log 48");
  backend.records[1].erase(13, 28);
  ASSERT_TRUE(backend.records[1] == "[INFO ] uut: Log 48 21 %");
}

} // namespace log
} // namespace gpcc_tests
