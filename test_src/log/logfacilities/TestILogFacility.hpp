/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TESTILOGFACILITY_HPP_201601061558
#define TESTILOGFACILITY_HPP_201601061558

#include "FakeBackend.hpp"
#include <gpcc/log/logfacilities/ILogFacility.hpp>
#include <gpcc/log/logfacilities/ILogFacilityCtrl.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/log/log_levels.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "gtest/gtest.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

// Test fixture for gpcc::log::ILogFacility related tests.
// This test fixture can be used to test Logger and Backend registration and
// unregistration. There is a derived test fixture "ILogFacility_Log_TestsF",
// which focuses on testing message logging itself.
template <typename T>
class ILogFacility_TestsF: public Test
{
  public:
    ILogFacility_TestsF(void);
    virtual ~ILogFacility_TestsF(void) = default;

  protected:
    T uut;
    bool uutRunning;

    void SetUp(void) override;
    void TearDown(void) override;

    void StartUUT(void);
    void StopUUT(void);
};

template <typename T>
ILogFacility_TestsF<T>::ILogFacility_TestsF()
: Test()
, uut("LFThread", 8)
, uutRunning(false)
{
}
template <typename T>
void ILogFacility_TestsF<T>::SetUp(void)
{
  StartUUT();
}
template <typename T>
void ILogFacility_TestsF<T>::TearDown(void)
{
  StopUUT();
}

template <typename T>
void ILogFacility_TestsF<T>::StartUUT()
{
  if (!uutRunning)
  {
    uut.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
    uutRunning = true;
  }
}
template <typename T>
void ILogFacility_TestsF<T>::StopUUT()
{
  if (uutRunning)
  {
    uut.Stop();
    uutRunning = false;
  }
}

// ====================================================================================================================
// ====================================================================================================================

// Test fixture for gpcc::log::ILogFacility related tests which focus on testing message logging.
template <typename T>
class ILogFacility_Log_TestsF: public ILogFacility_TestsF<T>
{
  public:
    ILogFacility_Log_TestsF(void);

  protected:
    Logger logger;
    FakeBackend backend;

    bool setupComplete;

    void SetUp(void) override;
    void TearDown(void) override;

    void PrintBackendRecords(void);
};

template <typename T>
ILogFacility_Log_TestsF<T>::ILogFacility_Log_TestsF()
: ILogFacility_TestsF<T>()
, logger("TL1")
, backend()
, setupComplete(false)
{
}

template <typename T>
void ILogFacility_Log_TestsF<T>::SetUp(void)
{
  ILogFacility_TestsF<T>::SetUp();

  logger.SetLogLevel(LogLevel::DebugOrAbove);

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };

  this->uut.Register(backend);

  ON_SCOPE_EXIT_DISMISS(unregLogger);

  setupComplete = true;
}

template <typename T>
void ILogFacility_Log_TestsF<T>::TearDown(void)
{
  try
  {
    if (setupComplete)
    {
      this->uut.Unregister(logger);
      this->uut.Unregister(backend);
    }

    ILogFacility_TestsF<T>::TearDown();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

template <typename T>
void ILogFacility_Log_TestsF<T>::PrintBackendRecords(void)
{
  std::cout << this->backend.records.size() << " records in fake backend:" << std::endl;
  for (auto & e: this->backend.records)
    std::cout << e << std::endl;
  std::cout << "END" << std::endl;
}

// ====================================================================================================================
// ====================================================================================================================

template <typename T>
using ILogFacility_Tests1F = ILogFacility_TestsF<T>;

template <typename T>
using ILogFacility_Tests2F = ILogFacility_Log_TestsF<T>;

TYPED_TEST_SUITE_P(ILogFacility_Tests1F);
TYPED_TEST_SUITE_P(ILogFacility_Tests2F);

TYPED_TEST_P(ILogFacility_Tests1F, Instantiation)
{
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_OK)
{
  Logger logger("TL1");

  this->uut.Register(logger);
  EXPECT_TRUE(logger.GetLogFacility() == &this->uut);

  this->uut.Unregister(logger);
  EXPECT_TRUE(logger.GetLogFacility() == nullptr);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_OK_uutNotStarted)
{
  this->StopUUT();

  Logger logger("TL1");

  this->uut.Register(logger);
  EXPECT_TRUE(logger.GetLogFacility() == &this->uut);

  this->uut.Unregister(logger);
  EXPECT_TRUE(logger.GetLogFacility() == nullptr);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_TwiceAtSameLogFacility)
{
  Logger logger("TL1");

  this->uut.Register(logger);
  EXPECT_TRUE(logger.GetLogFacility() == &this->uut);

  // try to register once more at the same log facility
  EXPECT_THROW(this->uut.Register(logger), std::logic_error);

  this->uut.Unregister(logger);
  EXPECT_TRUE(logger.GetLogFacility() == nullptr);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_TwiceAtDifferentLogFacilities)
{
  std::unique_ptr<gtest_TypeParam_> spUUT2(new gtest_TypeParam_("LFThread", 8));

  Logger logger("TL1");

  // register at other log facility
  spUUT2->Register(logger);
  ON_SCOPE_EXIT(unregLogger) { spUUT2->Unregister(logger); };
  EXPECT_TRUE(logger.GetLogFacility() == spUUT2.get());

  // try to register at uut, though the logger is already registered at the othe log facility
  ASSERT_THROW(this->uut.Register(logger), std::logic_error);

  ON_SCOPE_EXIT_DISMISS(unregLogger);
  spUUT2->Unregister(logger);
  EXPECT_TRUE(logger.GetLogFacility() == nullptr);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_DifferentLoggersButSameName)
{
  Logger logger1("TL1");
  Logger logger2("TL1");

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };
  EXPECT_TRUE(logger1.GetLogFacility() == &this->uut);

  // try to register the second logger, which has the same name
  EXPECT_THROW(this->uut.Register(logger2), std::logic_error);
  if (logger2.GetLogFacility() != nullptr)
  {
    EXPECT_TRUE(false);
    this->uut.Unregister(logger2);
  }

  ON_SCOPE_EXIT_DISMISS(unregLogger1);
  this->uut.Unregister(logger1);
  EXPECT_TRUE(logger1.GetLogFacility() == nullptr);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterLogger_ButNotRegisteredAnywhere)
{
  Logger logger("TL1");

  EXPECT_THROW(this->uut.Unregister(logger), std::logic_error);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterLogger_ButRegisteredSomewhereElse)
{
  std::unique_ptr<gtest_TypeParam_> spUUT2(new gtest_TypeParam_("LFThread", 8));
  Logger logger("TL1");

  // register logger at the other log facility
  spUUT2->Register(logger);
  ON_SCOPE_EXIT(unregLogger) { spUUT2->Unregister(logger); };
  EXPECT_TRUE(logger.GetLogFacility() == spUUT2.get());

  // try to unregister at uut, but the logger is registered at an different log facility
  EXPECT_THROW(this->uut.Unregister(logger), std::logic_error);

  // logger must still be registered at the other log facility
  EXPECT_TRUE(logger.GetLogFacility() == spUUT2.get());

  ON_SCOPE_EXIT_DISMISS(unregLogger);
  spUUT2->Unregister(logger);
  EXPECT_TRUE(logger.GetLogFacility() == nullptr);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_BeforeFirst)
{
  Logger logger1("TL1");
  Logger logger2("TL2");

  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };
  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(2U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
  EXPECT_TRUE(v[1].first == "TL2");
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_BehindLast)
{
  Logger logger1("TL1");
  Logger logger2("TL2");

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };
  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(2U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
  EXPECT_TRUE(v[1].first == "TL2");
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_BetweenTwo)
{
  Logger logger1("TL1");
  Logger logger2("TL2");
  Logger logger3("TL3");

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };
  this->uut.Register(logger3);
  ON_SCOPE_EXIT(unregLogger3) { this->uut.Unregister(logger3); };
  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(3U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
  EXPECT_TRUE(v[1].first == "TL2");
  EXPECT_TRUE(v[2].first == "TL3");
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterLogger_First)
{
  Logger logger1("TL1");
  Logger logger2("TL2");

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };
  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };

  ON_SCOPE_EXIT_DISMISS(unregLogger1);
  this->uut.Unregister(logger1);

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(1U, v.size());
  EXPECT_TRUE(v[0].first == "TL2");
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterLogger_Last)
{
  Logger logger1("TL1");
  Logger logger2("TL2");

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };
  this->uut.Register(logger2);

  this->uut.Unregister(logger2);

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(1U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterLogger_Mid)
{
  Logger logger1("TL1");
  Logger logger2("TL2");
  Logger logger3("TL3");

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };
  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };
  this->uut.Register(logger3);
  ON_SCOPE_EXIT(unregLogger3) { this->uut.Unregister(logger3); };

  ON_SCOPE_EXIT_DISMISS(unregLogger2);
  this->uut.Unregister(logger2);

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(2U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
  EXPECT_TRUE(v[1].first == "TL3");
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_SetDefaults)
{
  Logger logger1("TL1");
  Logger logger2("TL2");

  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::InfoOrAbove);

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL1", LogLevel::WarningOrAbove));
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL2", LogLevel::ErrorOrAbove));

  this->uut.SetDefaultSettings(std::move(defaultSettings));

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregisterLogger1) { this->uut.Unregister(logger1); };
  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregisterLogger2) { this->uut.Unregister(logger2); };

  // check that log levels have been set
  EXPECT_EQ(LogLevel::WarningOrAbove, logger1.GetLogLevel());
  EXPECT_EQ(LogLevel::ErrorOrAbove, logger2.GetLogLevel());

  // check that configuration has been consumed
  defaultSettings = this->uut.RemoveDefaultSettings();
  EXPECT_EQ(0U, defaultSettings.size());
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterLogger_NotAllDefaultsConsumed)
{
  Logger logger1("TL1");

  logger1.SetLogLevel(LogLevel::InfoOrAbove);

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL1", LogLevel::WarningOrAbove));
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL2", LogLevel::ErrorOrAbove));

  this->uut.SetDefaultSettings(std::move(defaultSettings));

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregisterLogger1) { this->uut.Unregister(logger1); };

  // check that log levels have been set
  EXPECT_EQ(LogLevel::WarningOrAbove, logger1.GetLogLevel());

  // check that configuration has been consumed
  defaultSettings = this->uut.RemoveDefaultSettings();
  ASSERT_EQ(1U, defaultSettings.size());
  ASSERT_TRUE(defaultSettings[0].first == "TL2");
  ASSERT_TRUE(defaultSettings[0].second == LogLevel::ErrorOrAbove);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterBackend_OK)
{
  FakeBackend backend;

  this->uut.Register(backend);
  this->uut.Unregister(backend);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterBackend_OK_uutNotStarted)
{
  this->StopUUT();
  FakeBackend backend;

  this->uut.Register(backend);
  this->uut.Unregister(backend);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterBackend_TwiceAtSameLogFacility)
{
  FakeBackend backend;

  this->uut.Register(backend);
  ON_SCOPE_EXIT(unregBackend) { this->uut.Unregister(backend); };

  // attempt to register a second time at the same log facility
  EXPECT_THROW(this->uut.Register(backend), std::logic_error);

  ON_SCOPE_EXIT_DISMISS(unregBackend);
  this->uut.Unregister(backend);
}
TYPED_TEST_P(ILogFacility_Tests1F, RegisterBackend_TwiceAtDifferentLogFacilities)
{
  std::unique_ptr<gtest_TypeParam_> spUUT2(new gtest_TypeParam_("LFThread", 8));

  FakeBackend backend;

  // register backend at the other log facility
  spUUT2->Register(backend);
  ON_SCOPE_EXIT(unregBackend) { spUUT2->Unregister(backend); };

  // try to register the backend at the uut, though it is already registered at the other
  // log facility
  EXPECT_THROW(this->uut.Register(backend), std::logic_error);

  ON_SCOPE_EXIT_DISMISS(unregBackend);
  spUUT2->Unregister(backend);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterBackend_ButNotRegisteredAnywhere)
{
  FakeBackend backend;

  EXPECT_THROW(this->uut.Unregister(backend), std::logic_error);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterBackend_ButRegisteredSomewhereElse)
{
  std::unique_ptr<gtest_TypeParam_> spUUT2(new gtest_TypeParam_("LFThread", 8));
  FakeBackend backend;

  // register backend at the other log facility
  spUUT2->Register(backend);
  ON_SCOPE_EXIT(unregBackend) { spUUT2->Unregister(backend); };

  // try to unregister backend at the uut, though it is already registered at the
  // other log facility
  EXPECT_THROW(this->uut.Unregister(backend), std::logic_error);

  ON_SCOPE_EXIT_DISMISS(unregBackend);
  spUUT2->Unregister(backend);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterBackend_First)
{
  FakeBackend backend1;
  FakeBackend backend2;
  FakeBackend backend3;

  this->uut.Register(backend1);
  ON_SCOPE_EXIT(unregBackend1) { this->uut.Unregister(backend1); };
  this->uut.Register(backend2);
  ON_SCOPE_EXIT(unregBackend2) { this->uut.Unregister(backend2); };
  this->uut.Register(backend3);
  ON_SCOPE_EXIT(unregBackend3) { this->uut.Unregister(backend3); };

  ON_SCOPE_EXIT_DISMISS(unregBackend1);
  this->uut.Unregister(backend1);
  ON_SCOPE_EXIT_DISMISS(unregBackend2);
  this->uut.Unregister(backend2);
  ON_SCOPE_EXIT_DISMISS(unregBackend3);
  this->uut.Unregister(backend3);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterBackend_Last)
{
  FakeBackend backend1;
  FakeBackend backend2;
  FakeBackend backend3;

  this->uut.Register(backend1);
  ON_SCOPE_EXIT(unregBackend1) { this->uut.Unregister(backend1); };
  this->uut.Register(backend2);
  ON_SCOPE_EXIT(unregBackend2) { this->uut.Unregister(backend2); };
  this->uut.Register(backend3);
  ON_SCOPE_EXIT(unregBackend3) { this->uut.Unregister(backend3); };

  ON_SCOPE_EXIT_DISMISS(unregBackend3);
  this->uut.Unregister(backend3);
  ON_SCOPE_EXIT_DISMISS(unregBackend1);
  this->uut.Unregister(backend1);
  ON_SCOPE_EXIT_DISMISS(unregBackend2);
  this->uut.Unregister(backend2);
}
TYPED_TEST_P(ILogFacility_Tests1F, UnregisterBackend_Mid)
{
  FakeBackend backend1;
  FakeBackend backend2;
  FakeBackend backend3;

  this->uut.Register(backend1);
  ON_SCOPE_EXIT(unregBackend1) { this->uut.Unregister(backend1); };
  this->uut.Register(backend2);
  ON_SCOPE_EXIT(unregBackend2) { this->uut.Unregister(backend2); };
  this->uut.Register(backend3);
  ON_SCOPE_EXIT(unregBackend3) { this->uut.Unregister(backend3); };

  ON_SCOPE_EXIT_DISMISS(unregBackend2);
  this->uut.Unregister(backend2);
  ON_SCOPE_EXIT_DISMISS(unregBackend1);
  this->uut.Unregister(backend1);
  ON_SCOPE_EXIT_DISMISS(unregBackend3);
  this->uut.Unregister(backend3);
}

TYPED_TEST_P(ILogFacility_Tests2F, Instantiation)
{
}
TYPED_TEST_P(ILogFacility_Tests2F, Log)
{
  this->logger.Log(LogType::Debug, "Test");
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0]== "[DEBUG] TL1: Test");
}

#ifndef SKIP_VERYBIGMEM_TESTS
TYPED_TEST_P(ILogFacility_Tests2F, Log_Performance)
{
  this->StopUUT();

  for (uint_fast32_t i = 0U; i < 1000000U; ++i)
  {
    this->logger.Log(LogType::Error, "Test");
  }
}
#endif

TYPED_TEST_P(ILogFacility_Tests2F, Log_WhileStopped)
{
  this->StopUUT();
  this->logger.Log(LogType::Debug, "Test");
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[DEBUG] TL1: Test");
}
TYPED_TEST_P(ILogFacility_Tests2F, Log_ButNoBackend)
{
  // unregister backend
  this->uut.Unregister(this->backend);
  ON_SCOPE_EXIT(unregBackend) { this->uut.Register(this->backend); };

  // log something
  this->logger.Log(LogType::Debug, "Test");
  this->uut.Flush();

  // register backend again
  ON_SCOPE_EXIT_DISMISS(unregBackend);
  this->uut.Register(this->backend);

  // verify that nothing is logged
  this->uut.Flush();
  ASSERT_EQ(0U, this->backend.records.size());
}
TYPED_TEST_P(ILogFacility_Tests2F, ReportLogMessageCreationFailed_ButNoBackend)
{
  // unregister backend
  this->uut.Unregister(this->backend);
  ON_SCOPE_EXIT(unregBackend) { this->uut.Register(this->backend); };

  // log something
  this->logger.LogFailed();
  this->uut.Flush();

  // register backend again
  ON_SCOPE_EXIT_DISMISS(unregBackend);
  this->uut.Register(this->backend);

  // verify that nothing is logged
  this->uut.Flush();
  ASSERT_EQ(0U, this->backend.records.size());
}
TYPED_TEST_P(ILogFacility_Tests2F, LogTypes)
{
  this->logger.Log(LogType::Debug, "Test1");
  this->logger.Log(LogType::Info, "Test2");
  this->logger.Log(LogType::Warning, "Test3");
  this->logger.Log(LogType::Error, "Test4");
  this->logger.Log(LogType::Fatal, "Test5");
  this->uut.Flush();

  ASSERT_EQ(5U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[DEBUG] TL1: Test1");
  ASSERT_TRUE(this->backend.records[1] == "[INFO ] TL1: Test2");
  ASSERT_TRUE(this->backend.records[2] == "[WARN ] TL1: Test3");
  ASSERT_TRUE(this->backend.records[3] == "[ERROR] TL1: Test4");
  ASSERT_TRUE(this->backend.records[4] == "[FATAL] TL1: Test5");
}
TYPED_TEST_P(ILogFacility_Tests2F, Filtering)
{
  // DebugOrAbove
  this->uut.SetLogLevel("TL1", LogLevel::DebugOrAbove);
  this->logger.Log(LogType::Debug, "L1.1");
  this->logger.Log(LogType::Info, "L1.2");
  this->logger.Log(LogType::Warning, "L1.3");
  this->logger.Log(LogType::Error, "L1.4");
  this->logger.Log(LogType::Fatal, "L1.5");
  this->uut.Flush();

  // InfoOrAbove
  this->uut.SetLogLevel("TL1", LogLevel::InfoOrAbove);
  this->logger.Log(LogType::Debug, "L2.1");
  this->logger.Log(LogType::Info, "L2.2");
  this->logger.Log(LogType::Warning, "L2.3");
  this->logger.Log(LogType::Error, "L2.4");
  this->logger.Log(LogType::Fatal, "L2.5");
  this->uut.Flush();

  // WarningOrAbove
  this->uut.SetLogLevel("TL1", LogLevel::WarningOrAbove);
  this->logger.Log(LogType::Debug, "L3.1");
  this->logger.Log(LogType::Info, "L3.2");
  this->logger.Log(LogType::Warning, "L3.3");
  this->logger.Log(LogType::Error, "L3.4");
  this->logger.Log(LogType::Fatal, "L3.5");
  this->uut.Flush();

  // ErrorOrAbove
  this->uut.SetLogLevel("TL1", LogLevel::ErrorOrAbove);
  this->logger.Log(LogType::Debug, "L4.1");
  this->logger.Log(LogType::Info, "L4.2");
  this->logger.Log(LogType::Warning, "L4.3");
  this->logger.Log(LogType::Error, "L4.4");
  this->logger.Log(LogType::Fatal, "L4.5");
  this->uut.Flush();

  // FatalOrAbove
  this->uut.SetLogLevel("TL1", LogLevel::FatalOrAbove);
  this->logger.Log(LogType::Debug, "L5.1");
  this->logger.Log(LogType::Info, "L5.2");
  this->logger.Log(LogType::Warning, "L5.3");
  this->logger.Log(LogType::Error, "L5.4");
  this->logger.Log(LogType::Fatal, "L5.5");
  this->uut.Flush();

  // Nothing
  this->uut.SetLogLevel("TL1", LogLevel::Nothing);
  this->logger.Log(LogType::Debug, "L6.1");
  this->logger.Log(LogType::Info, "L6.2");
  this->logger.Log(LogType::Warning, "L6.3");
  this->logger.Log(LogType::Error, "L6.4");
  this->logger.Log(LogType::Fatal, "L6.5");
  this->uut.Flush();

  ASSERT_EQ(15U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0]  == "[DEBUG] TL1: L1.1");
  ASSERT_TRUE(this->backend.records[1]  == "[INFO ] TL1: L1.2");
  ASSERT_TRUE(this->backend.records[2]  == "[WARN ] TL1: L1.3");
  ASSERT_TRUE(this->backend.records[3]  == "[ERROR] TL1: L1.4");
  ASSERT_TRUE(this->backend.records[4]  == "[FATAL] TL1: L1.5");

  ASSERT_TRUE(this->backend.records[5]  == "[INFO ] TL1: L2.2");
  ASSERT_TRUE(this->backend.records[6]  == "[WARN ] TL1: L2.3");
  ASSERT_TRUE(this->backend.records[7]  == "[ERROR] TL1: L2.4");
  ASSERT_TRUE(this->backend.records[8]  == "[FATAL] TL1: L2.5");

  ASSERT_TRUE(this->backend.records[9]  == "[WARN ] TL1: L3.3");
  ASSERT_TRUE(this->backend.records[10] == "[ERROR] TL1: L3.4");
  ASSERT_TRUE(this->backend.records[11] == "[FATAL] TL1: L3.5");

  ASSERT_TRUE(this->backend.records[12] == "[ERROR] TL1: L4.4");
  ASSERT_TRUE(this->backend.records[13] == "[FATAL] TL1: L4.5");

  ASSERT_TRUE(this->backend.records[14] == "[FATAL] TL1: L5.5");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop1)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 1; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(9U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[ERROR] *** Logger: 1 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop2)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 2; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(9U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[ERROR] *** Logger: 2 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop253)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 253; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(9U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[ERROR] *** Logger: 253 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop254)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 254; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(9U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[ERROR] *** Logger: 254 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop255)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 255; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(9U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[ERROR] *** Logger: At least 255 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop255_ErrorNotDropped)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 255; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->logger.Log(LogType::Error, "Not Dropped");

  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(10U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[ERROR] TL1: Not Dropped");
  ASSERT_TRUE(this->backend.records[9] == "[ERROR] *** Logger: At least 255 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, Drop255_FatalNotDropped)
{
  this->StopUUT();
  for (int i = 0; i < 8 + 255; i++)
  {
    std::ostringstream s;
    s << "Test" << i;
    this->logger.Log(LogType::Debug, s.str());
  }
  this->logger.Log(LogType::Fatal, "Not Dropped");

  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(10U, this->backend.records.size());
  for (int i = 0; i < 8; i++)
  {
    std::ostringstream s;
    s << "[DEBUG] TL1: Test" << i;
    ASSERT_TRUE(this->backend.records[i] == s.str());

  }
  ASSERT_TRUE(this->backend.records[8] == "[FATAL] TL1: Not Dropped");
  ASSERT_TRUE(this->backend.records[9] == "[ERROR] *** Logger: At least 255 not (properly) delivered message(s)! ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, BackendThrows)
{
  this->backend.logsTillThrow = 2;
  this->logger.Log(LogType::Debug, "Test1");
  this->logger.Log(LogType::Debug, "Test2");
  this->logger.Log(LogType::Debug, "Test3");
  this->uut.Flush();

  ASSERT_EQ(3U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[DEBUG] TL1: Test1");

  bool variant1 = (this->backend.records[1] == "[DEBUG] TL1: Test3") &&
                  (this->backend.records[2] == "[ERROR] *** Logger: 1 not (properly) delivered message(s)! ***");
  bool variant2 = (this->backend.records[1] == "[ERROR] *** Logger: 1 not (properly) delivered message(s)! ***") &&
                  (this->backend.records[2] == "[DEBUG] TL1: Test3");

  ASSERT_TRUE(variant1 || variant2);
}
TYPED_TEST_P(ILogFacility_Tests2F, LogFailed1)
{
  this->StopUUT();
  this->logger.LogFailed();
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[ERROR] *** Logger: 1 error(s) during log message creation (e.g. out-of-memory) ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, LogFailed2)
{
  this->StopUUT();
  this->logger.LogFailed();
  this->logger.LogFailed();
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[ERROR] *** Logger: 2 error(s) during log message creation (e.g. out-of-memory) ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, LogFailed254)
{
  this->StopUUT();
  for (uint_fast16_t i = 0; i < 254; i++)
    this->logger.LogFailed();
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[ERROR] *** Logger: 254 error(s) during log message creation (e.g. out-of-memory) ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, LogFailed255)
{
  this->StopUUT();
  for (uint_fast16_t i = 0; i < 255; i++)
    this->logger.LogFailed();
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[ERROR] *** Logger: At least 255 error(s) during log message creation (e.g. out-of-memory) ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, LogFailed256)
{
  this->StopUUT();
  for (uint_fast16_t i = 0; i < 256; i++)
    this->logger.LogFailed();
  this->StartUUT();
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[ERROR] *** Logger: At least 255 error(s) during log message creation (e.g. out-of-memory) ***");
}
TYPED_TEST_P(ILogFacility_Tests2F, LogFailedAndBackendThrows)
{
  this->backend.logsTillThrow = 1;
  this->logger.LogFailed();
  this->uut.Flush();
  this->logger.Log(LogType::Debug, "Test1");
  this->uut.Flush();

  ASSERT_EQ(2U, this->backend.records.size());

  bool variant1 = (this->backend.records[0] == "[DEBUG] TL1: Test1") &&
                  (this->backend.records[1] == "[ERROR] *** Logger: 1 not (properly) delivered message(s)! ***");
  bool variant2 = (this->backend.records[1] == "[ERROR] *** Logger: 1 not (properly) delivered message(s)! ***") &&
                  (this->backend.records[0] == "[DEBUG] TL1: Test1");

  ASSERT_TRUE(variant1 || variant2);
}

REGISTER_TYPED_TEST_SUITE_P(ILogFacility_Tests1F,
                            Instantiation,
                            RegisterLogger_OK,
                            RegisterLogger_OK_uutNotStarted,
                            RegisterLogger_TwiceAtSameLogFacility,
                            RegisterLogger_TwiceAtDifferentLogFacilities,
                            RegisterLogger_DifferentLoggersButSameName,
                            UnregisterLogger_ButNotRegisteredAnywhere,
                            UnregisterLogger_ButRegisteredSomewhereElse,
                            RegisterLogger_BeforeFirst,
                            RegisterLogger_BehindLast,
                            RegisterLogger_BetweenTwo,
                            UnregisterLogger_First,
                            UnregisterLogger_Last,
                            UnregisterLogger_Mid,
                            RegisterLogger_SetDefaults,
                            RegisterLogger_NotAllDefaultsConsumed,
                            RegisterBackend_OK,
                            RegisterBackend_OK_uutNotStarted,
                            RegisterBackend_TwiceAtSameLogFacility,
                            RegisterBackend_TwiceAtDifferentLogFacilities,
                            UnregisterBackend_ButNotRegisteredAnywhere,
                            UnregisterBackend_ButRegisteredSomewhereElse,
                            UnregisterBackend_First,
                            UnregisterBackend_Last,
                            UnregisterBackend_Mid);

REGISTER_TYPED_TEST_SUITE_P(ILogFacility_Tests2F,
                            Instantiation,
                            Log,
#ifndef SKIP_VERYBIGMEM_TESTS
                            Log_Performance,
#endif
                            Log_WhileStopped,
                            Log_ButNoBackend,
                            ReportLogMessageCreationFailed_ButNoBackend,
                            LogTypes,
                            Filtering,
                            Drop1,
                            Drop2,
                            Drop253,
                            Drop254,
                            Drop255,
                            Drop255_ErrorNotDropped,
                            Drop255_FatalNotDropped,
                            BackendThrows,
                            LogFailed1,
                            LogFailed2,
                            LogFailed254,
                            LogFailed255,
                            LogFailed256,
                            LogFailedAndBackendThrows);

} // namespace log
} // namespace gpcc_tests

#endif // TESTILOGFACILITY_HPP_201601061558
