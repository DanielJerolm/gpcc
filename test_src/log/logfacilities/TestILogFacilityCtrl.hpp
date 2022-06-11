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

#ifndef TESTILOGFACILITYCTRL_HPP_201701061629
#define TESTILOGFACILITYCTRL_HPP_201701061629

#include "FakeBackend.hpp"
#include "gpcc/src/log/logfacilities/ILogFacility.hpp"
#include "gpcc/src/log/logfacilities/ILogFacilityCtrl.hpp"
#include "gpcc/src/log/Logger.hpp"
#include "gpcc/src/log/log_levels.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

/// Test fixture for gpcc::log::ILogFacilityCtrl related tests.
template <typename T>
class ILogFacilityCtrl_TestsF: public Test
{
  public:
    ILogFacilityCtrl_TestsF(void);

  protected:
    FakeBackend backend;
    T uut;
    bool uutRunning;

    void SetUp(void) override;
    void TearDown(void) override;

    void StartUUT(void);
    void StopUUT(void);

    void PrintBackendRecords(void);
};

template <typename T>
ILogFacilityCtrl_TestsF<T>::ILogFacilityCtrl_TestsF()
: Test()
, backend()
, uut("LFThread", 8)
, uutRunning(false)
{
}
template <typename T>
void ILogFacilityCtrl_TestsF<T>::SetUp(void)
{
  this->uut.Register(backend);
  StartUUT();
}
template <typename T>
void ILogFacilityCtrl_TestsF<T>::TearDown(void)
{
  try
  {
    StopUUT();
    this->uut.Unregister(backend);
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

template <typename T>
void ILogFacilityCtrl_TestsF<T>::StartUUT()
{
  if (!uutRunning)
  {
    uut.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
    uutRunning = true;
  }
}
template <typename T>
void ILogFacilityCtrl_TestsF<T>::StopUUT()
{
  if (uutRunning)
  {
    uut.Stop();
    uutRunning = false;
  }
}

template <typename T>
void ILogFacilityCtrl_TestsF<T>::PrintBackendRecords(void)
{
  std::cout << this->backend.records.size() << " records in fake backend:" << std::endl;
  for (auto & e: this->backend.records)
    std::cout << e << std::endl;
  std::cout << "END" << std::endl;
}

TYPED_TEST_SUITE_P(ILogFacilityCtrl_TestsF);

TYPED_TEST_P(ILogFacilityCtrl_TestsF, Instantiation)
{
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, EnumerateLogSources_None)
{
  auto v = this->uut.EnumerateLogSources();
  EXPECT_EQ(0U, v.size());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, EnumerateLogSources_One)
{
  Logger logger("TL1");
  logger.SetLogLevel(LogLevel::ErrorOrAbove);

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(1U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
  EXPECT_TRUE(v[0].second == LogLevel::ErrorOrAbove);
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, EnumerateLogSources_Two)
{
  Logger logger1("TL1");
  Logger logger2("TL2");
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };

  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };

  auto v = this->uut.EnumerateLogSources();
  ASSERT_EQ(2U, v.size());
  EXPECT_TRUE(v[0].first == "TL1");
  EXPECT_TRUE(v[0].second == LogLevel::InfoOrAbove);
  EXPECT_TRUE(v[1].first == "TL2");
  EXPECT_TRUE(v[1].second == LogLevel::WarningOrAbove);
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, GetLogLevel_OK)
{
  Logger logger1("TL1");
  Logger logger2("TL2");
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::WarningOrAbove);

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };

  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };

  EXPECT_EQ(this->uut.GetLogLevel("TL1"), LogLevel::InfoOrAbove);
  EXPECT_EQ(this->uut.GetLogLevel("TL2"), LogLevel::WarningOrAbove);
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, GetLogLevel_LogSrcNotExisting)
{
  EXPECT_THROW((void)this->uut.GetLogLevel("ABC"), std::invalid_argument);
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, SetLogLevel)
{
  Logger logger1("TL1");
  Logger logger2("TL2");
  logger1.SetLogLevel(LogLevel::InfoOrAbove);
  logger2.SetLogLevel(LogLevel::InfoOrAbove);

  this->uut.Register(logger1);
  ON_SCOPE_EXIT(unregLogger1) { this->uut.Unregister(logger1); };

  this->uut.Register(logger2);
  ON_SCOPE_EXIT(unregLogger2) { this->uut.Unregister(logger2); };

  ASSERT_TRUE(this->uut.SetLogLevel("TL1", LogLevel::WarningOrAbove));

  EXPECT_TRUE(logger1.GetLogLevel() == LogLevel::WarningOrAbove);
  EXPECT_TRUE(logger2.GetLogLevel() == LogLevel::InfoOrAbove);

  logger1.Log(LogType::Debug, "Invisible");
  logger2.Log(LogType::Info, "Logged");
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[INFO ] TL2: Logged");
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, SetLogLevel_NoSuchSource)
{
  Logger logger("TL1");
  logger.SetLogLevel(LogLevel::DebugOrAbove);

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };

  ASSERT_FALSE(this->uut.SetLogLevel("TL2", LogLevel::InfoOrAbove));

  logger.Log(LogType::Debug, "Logged");
  logger.Log(LogType::Info, "Logged");
  this->uut.Flush();

  ASSERT_EQ(2U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[DEBUG] TL1: Logged");
  ASSERT_TRUE(this->backend.records[1] == "[INFO ] TL1: Logged");
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, LowerLogLevel)
{
  Logger logger("TL1");
  logger.SetLogLevel(LogLevel::InfoOrAbove);

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };

  ASSERT_TRUE(this->uut.LowerLogLevel("TL1", LogLevel::WarningOrAbove));
  ASSERT_EQ(LogLevel::InfoOrAbove, logger.GetLogLevel());

  ASSERT_TRUE(this->uut.LowerLogLevel("TL1", LogLevel::InfoOrAbove));
  ASSERT_EQ(LogLevel::InfoOrAbove, logger.GetLogLevel());

  ASSERT_TRUE(this->uut.LowerLogLevel("TL1", LogLevel::DebugOrAbove));
  ASSERT_EQ(LogLevel::DebugOrAbove, logger.GetLogLevel());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RaiseLogLevel)
{
  Logger logger("TL1");
  logger.SetLogLevel(LogLevel::InfoOrAbove);

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };

  ASSERT_TRUE(this->uut.RaiseLogLevel("TL1", LogLevel::DebugOrAbove));
  ASSERT_EQ(LogLevel::InfoOrAbove, logger.GetLogLevel());

  ASSERT_TRUE(this->uut.RaiseLogLevel("TL1", LogLevel::InfoOrAbove));
  ASSERT_EQ(LogLevel::InfoOrAbove, logger.GetLogLevel());

  ASSERT_TRUE(this->uut.RaiseLogLevel("TL1", LogLevel::WarningOrAbove));
  ASSERT_EQ(LogLevel::WarningOrAbove, logger.GetLogLevel());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_NoDefaultSettingsSet)
{
  Logger logger("NewLogger");

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(0U, this->backend.records.size());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_DefaultSettingsRemoved)
{
  Logger logger("NewLogger");

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  this->uut.SetDefaultSettings(std::move(defaultSettings));

  defaultSettings = this->uut.RemoveDefaultSettings();
  ASSERT_EQ(0U, defaultSettings.size());

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(0U, this->backend.records.size());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_DefaultSettingsRemovedTwice)
{
  Logger logger("NewLogger");

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL1", LogLevel::WarningOrAbove));
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL2", LogLevel::ErrorOrAbove));
  this->uut.SetDefaultSettings(std::move(defaultSettings));

  defaultSettings = this->uut.RemoveDefaultSettings();
  ASSERT_EQ(2U, defaultSettings.size());
  EXPECT_TRUE(defaultSettings[0].first == "TL1");
  EXPECT_TRUE(defaultSettings[0].second == LogLevel::WarningOrAbove);
  EXPECT_TRUE(defaultSettings[1].first == "TL2");
  EXPECT_TRUE(defaultSettings[1].second == LogLevel::ErrorOrAbove);

  defaultSettings = this->uut.RemoveDefaultSettings();
  ASSERT_EQ(0U, defaultSettings.size());

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(0U, this->backend.records.size());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_DefaultSettingsNeverSetButRemoved)
{
  Logger logger("NewLogger");

  auto defaultSettings = this->uut.RemoveDefaultSettings();
  ASSERT_EQ(0U, defaultSettings.size());

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(0U, this->backend.records.size());
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_NoMatchingDefaultSetting)
{
  Logger logger("NewLogger");

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL1", LogLevel::WarningOrAbove));
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL2", LogLevel::ErrorOrAbove));
  this->uut.SetDefaultSettings(std::move(defaultSettings));

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[WARN ] NewLogger: No default log level deposited.");
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_ReplaceOfDefaultSettings)
{
  Logger logger("NewLogger");

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL1", LogLevel::WarningOrAbove));
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("TL2", LogLevel::ErrorOrAbove));
  this->uut.SetDefaultSettings(std::move(defaultSettings));

  defaultSettings.clear();
  defaultSettings.push_back(ILogFacilityCtrl::tLogSrcConfig("NewLogger", LogLevel::WarningOrAbove));
  this->uut.SetDefaultSettings(std::move(defaultSettings));

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(0U, this->backend.records.size());
  ASSERT_TRUE(logger.GetLogLevel() == LogLevel::WarningOrAbove);
}
TYPED_TEST_P(ILogFacilityCtrl_TestsF, RegisterLogger_AllDefaultSettingConsumed)
{
  Logger logger("NewLogger");

  std::vector<ILogFacilityCtrl::tLogSrcConfig> defaultSettings;
  this->uut.SetDefaultSettings(std::move(defaultSettings));

  this->uut.Register(logger);
  ON_SCOPE_EXIT(unregLogger) { this->uut.Unregister(logger); };
  this->uut.Flush();

  ASSERT_EQ(1U, this->backend.records.size());
  ASSERT_TRUE(this->backend.records[0] == "[WARN ] NewLogger: No default log level deposited.");
}

REGISTER_TYPED_TEST_SUITE_P(ILogFacilityCtrl_TestsF,
                            Instantiation,
                            EnumerateLogSources_None,
                            EnumerateLogSources_One,
                            EnumerateLogSources_Two,
                            GetLogLevel_OK,
                            GetLogLevel_LogSrcNotExisting,
                            SetLogLevel,
                            SetLogLevel_NoSuchSource,
                            LowerLogLevel,
                            RaiseLogLevel,
                            RegisterLogger_NoDefaultSettingsSet,
                            RegisterLogger_DefaultSettingsRemoved,
                            RegisterLogger_DefaultSettingsRemovedTwice,
                            RegisterLogger_DefaultSettingsNeverSetButRemoved,
                            RegisterLogger_NoMatchingDefaultSetting,
                            RegisterLogger_ReplaceOfDefaultSettings,
                            RegisterLogger_AllDefaultSettingConsumed);

} // namespace log
} // namespace gpcc_tests

#endif // TESTILOGFACILITYCTRL_HPP_201701061629
