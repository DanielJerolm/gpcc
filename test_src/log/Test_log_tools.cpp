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

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#include "gpcc/src/file_systems/exceptions.hpp"
#include "gpcc/src/file_systems/linux_fs/FileStorage.hpp"
#include "gpcc/src/file_systems/linux_fs/internal/tools.hpp"
#include "gpcc/src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include "gpcc/src/log/logfacilities/ILogFacilityCtrl.hpp"
#include "gpcc/src/log/log_tools.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <cerrno>
#include <cstdio>
#include <cstdint>
#include <cstddef>

using namespace gpcc::log;
using namespace testing;

namespace gpcc_tests {
namespace log {

class gpcc_log_log_tools_TestsF: public Test
{
  public:
    gpcc_log_log_tools_TestsF(void);

  protected:
    gpcc::file_systems::linux_fs::internal::UnitTestDirProvider testDirProvider;
    std::string const baseDir;

    std::unique_ptr<gpcc::file_systems::linux_fs::FileStorage> spFS;

    void SetUp(void) override;
    void TearDown(void) override;
};

gpcc_log_log_tools_TestsF::gpcc_log_log_tools_TestsF(void)
: Test()
, testDirProvider()
, baseDir(testDirProvider.GetAbsPath())
, spFS()
{
}

void gpcc_log_log_tools_TestsF::SetUp(void)
{
  spFS.reset(new gpcc::file_systems::linux_fs::FileStorage(baseDir));
}
void gpcc_log_log_tools_TestsF::TearDown(void)
{
  spFS.reset();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToFile_Empty)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  uint32_t version = f->Read_uint32();
  size_t nEntries  = static_cast<size_t>(f->Read_uint64());

  EXPECT_EQ(0x00000001UL, version);
  EXPECT_EQ(0UL, nEntries);
  EXPECT_TRUE(f->GetState() == gpcc::Stream::IStreamReader::States::empty);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToFile_OneEntry)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  uint32_t version = f->Read_uint32();
  size_t nEntries  = static_cast<size_t>(f->Read_uint64());

  EXPECT_EQ(0x00000001UL, version);
  ASSERT_EQ(1UL, nEntries);

  std::string e1 = f->Read_string();
  LogLevel l1 = static_cast<LogLevel>(f->Read_uint8());

  EXPECT_TRUE(e1 == "Name1");
  EXPECT_TRUE(l1 == LogLevel::WarningOrAbove);

  EXPECT_TRUE(f->GetState() == gpcc::Stream::IStreamReader::States::empty);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToFile_TwoEntry)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::InfoOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  uint32_t version = f->Read_uint32();
  size_t nEntries  = static_cast<size_t>(f->Read_uint64());

  EXPECT_EQ(0x00000001UL, version);
  ASSERT_EQ(2UL, nEntries);

  std::string e1 = f->Read_string();
  LogLevel l1 = static_cast<LogLevel>(f->Read_uint8());

  std::string e2 = f->Read_string();
  LogLevel l2 = static_cast<LogLevel>(f->Read_uint8());

  EXPECT_TRUE(e1 == "Name1");
  EXPECT_TRUE(l1 == LogLevel::WarningOrAbove);

  EXPECT_TRUE(e2 == "Name2");
  EXPECT_TRUE(l2 == LogLevel::InfoOrAbove);

  EXPECT_TRUE(f->GetState() == gpcc::Stream::IStreamReader::States::empty);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToFile_TwoEntry_ZeroLengthName)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("", LogLevel::WarningOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::InfoOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  uint32_t version = f->Read_uint32();
  size_t nEntries  = static_cast<size_t>(f->Read_uint64());

  EXPECT_EQ(0x00000001UL, version);
  ASSERT_EQ(2UL, nEntries);

  std::string e1 = f->Read_string();
  LogLevel l1 = static_cast<LogLevel>(f->Read_uint8());

  std::string e2 = f->Read_string();
  LogLevel l2 = static_cast<LogLevel>(f->Read_uint8());

  EXPECT_TRUE(e1 == "");
  EXPECT_TRUE(l1 == LogLevel::WarningOrAbove);

  EXPECT_TRUE(e2 == "Name2");
  EXPECT_TRUE(l2 == LogLevel::InfoOrAbove);

  EXPECT_TRUE(f->GetState() == gpcc::Stream::IStreamReader::States::empty);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToFile_OverwriteExistingFile)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  data.clear();
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::DebugOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name3", LogLevel::Nothing));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  uint32_t version = f->Read_uint32();
  size_t nEntries  = static_cast<size_t>(f->Read_uint64());

  EXPECT_EQ(0x00000001UL, version);
  ASSERT_EQ(2UL, nEntries);

  std::string e1 = f->Read_string();
  LogLevel l1 = static_cast<LogLevel>(f->Read_uint8());

  std::string e2 = f->Read_string();
  LogLevel l2 = static_cast<LogLevel>(f->Read_uint8());

  EXPECT_TRUE(e1 == "Name2");
  EXPECT_TRUE(l1 == LogLevel::DebugOrAbove);

  EXPECT_TRUE(e2 == "Name3");
  EXPECT_TRUE(l2 == LogLevel::Nothing);

  EXPECT_TRUE(f->GetState() == gpcc::Stream::IStreamReader::States::empty);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_Empty)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(data == loadedData);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_OneEntry)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(data == loadedData);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_TwoEntry)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::InfoOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(data == loadedData);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_TwoEntry_ZeroLengthName)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::InfoOrAbove));

  WriteLogSrcConfigToFile(data, *(spFS.get()), "Test.dat");

  auto loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(data == loadedData);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_NoSuchFile)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> loadedData;

  ASSERT_THROW(loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat"), gpcc::file_systems::NoSuchFileError);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_InvalidVersion)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000002UL);
  *f << static_cast<uint64_t>(0);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<ILogFacilityCtrl::tLogSrcConfig> loadedData;

  ASSERT_THROW(loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat"), InvalidVersionError);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromFile_InvalidLogLevel)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  *f << static_cast<uint32_t>(0x00000001UL);
  *f << static_cast<uint64_t>(1);
  *f << std::string("Name1");
  *f << static_cast<uint8_t>(static_cast<uint8_t>(LogLevel::Nothing) + 1U);

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<ILogFacilityCtrl::tLogSrcConfig> loadedData;

  ASSERT_THROW(loadedData = ReadLogSrcConfigFromFile(*(spFS.get()), "Test.dat"), std::runtime_error);
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_Empty_NoExplicitHeadline)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  bool firstLine = true;
  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    if (firstLine)
    {
      EXPECT_STREQ(line.c_str(), "# Log Levels");
      firstLine = false;
    }

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    FAIL() << "File is not empty";
  }

  EXPECT_FALSE(firstLine) << "File was completely empty";

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_Empty_EmptyHeadline)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat", "");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  bool firstLine = true;
  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    if (firstLine)
    {
      EXPECT_STREQ(line.c_str(), "# Log Levels");
      firstLine = false;
    }

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    FAIL() << "File is not empty";
  }

  EXPECT_FALSE(firstLine) << "File was completely empty";

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_Empty_ExplicitHeadline)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat", "Headline");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  bool firstLine = true;
  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    if (firstLine)
    {
      EXPECT_STREQ(line.c_str(), "# Headline");
      firstLine = false;
    }

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    FAIL() << "File is not empty";
  }

  EXPECT_FALSE(firstLine) << "File was completely empty";

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_Empty_InvalidHeadline)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  ASSERT_THROW(WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat", "Headline\nLine2"), std::invalid_argument);
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_OneEntry)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  std::vector<std::string> extracted;

  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    extracted.emplace_back(std::move(line));
  }

  ASSERT_EQ(extracted.size(), 1U);
  EXPECT_STREQ(extracted[0].c_str(), "Name1 : warning");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_TwoEntry)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::InfoOrAbove));

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  std::vector<std::string> extracted;

  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    extracted.emplace_back(std::move(line));
  }

  ASSERT_EQ(extracted.size(), 2U);
  EXPECT_STREQ(extracted[0].c_str(), "Name1 : warning");
  EXPECT_STREQ(extracted[1].c_str(), "Name2 : info");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, WriteLogSrcConfigToTextFile_OverwriteExistingFile)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  data.clear();
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name2", LogLevel::DebugOrAbove));
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name3", LogLevel::Nothing));

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  auto f = spFS->Open("Test.dat");
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  std::vector<std::string> extracted;

  while (f->GetState() != gpcc::Stream::IStreamReader::States::empty)
  {
    std::string line = f->Read_line();

    if (line.empty())
      continue;

    if (line.front() == '#')
      continue;

    extracted.emplace_back(std::move(line));
  }

  ASSERT_EQ(extracted.size(), 2U);
  EXPECT_STREQ(extracted[0].c_str(), "Name2 : debug");
  EXPECT_STREQ(extracted[1].c_str(), "Name3 : nothing");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_Empty1)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(loadedData.empty());
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_Empty2)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;

  auto f = spFS->Create("Test.dat", true);
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(loadedData.empty());
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_OneEntry1)
{
  std::vector<ILogFacilityCtrl::tLogSrcConfig> data;
  data.push_back(ILogFacilityCtrl::tLogSrcConfig("Name1", LogLevel::WarningOrAbove));

  WriteLogSrcConfigToTextFile(data, *(spFS.get()), "Test.dat");

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_TRUE(loadedData == data);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_OneEntry2)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("Name1 : info");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_EQ(loadedData.size(), 1U);
  EXPECT_STREQ(loadedData[0].first.c_str(), "Name1");
  EXPECT_EQ(loadedData[0].second, LogLevel::InfoOrAbove);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_OneEntry_Whitespaces)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("  Name1    :   info ");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_EQ(loadedData.size(), 1U);
  EXPECT_STREQ(loadedData[0].first.c_str(), "Name1");
  EXPECT_EQ(loadedData[0].second, LogLevel::InfoOrAbove);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_TwoEntry)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("Name1 : info");
  f->Write_line("Name2 : debug");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_EQ(loadedData.size(), 2U);

  EXPECT_STREQ(loadedData[0].first.c_str(), "Name1");
  EXPECT_EQ(loadedData[0].second, LogLevel::InfoOrAbove);

  EXPECT_STREQ(loadedData[1].first.c_str(), "Name2");
  EXPECT_EQ(loadedData[1].second, LogLevel::DebugOrAbove);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_TypicalFile)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name1  : info   ");
  f->Write_line("Name2  : debug");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name3  :  warning");
  f->Write_line("# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_EQ(loadedData.size(), 3U);

  EXPECT_STREQ(loadedData[0].first.c_str(), "Name1");
  EXPECT_EQ(loadedData[0].second, LogLevel::InfoOrAbove);

  EXPECT_STREQ(loadedData[1].first.c_str(), "Name2");
  EXPECT_EQ(loadedData[1].second, LogLevel::DebugOrAbove);

  EXPECT_STREQ(loadedData[2].first.c_str(), "Name3");
  EXPECT_EQ(loadedData[2].second, LogLevel::WarningOrAbove);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_TypicalFile_Windows1)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment\r");
  f->Write_line("\r");
  f->Write_line("# comment\r");
  f->Write_line("Name1  : info   \r");
  f->Write_line("Name2  : debug\r");
  f->Write_line("\r");
  f->Write_line("# comment\r");
  f->Write_line("Name3  :  warning\r");
  f->Write_line("# comment\r");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_EQ(loadedData.size(), 3U);

  EXPECT_STREQ(loadedData[0].first.c_str(), "Name1");
  EXPECT_EQ(loadedData[0].second, LogLevel::InfoOrAbove);

  EXPECT_STREQ(loadedData[1].first.c_str(), "Name2");
  EXPECT_EQ(loadedData[1].second, LogLevel::DebugOrAbove);

  EXPECT_STREQ(loadedData[2].first.c_str(), "Name3");
  EXPECT_EQ(loadedData[2].second, LogLevel::WarningOrAbove);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_TypicalFile_Windows2)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("\r");
  f->Write_line("\r# comment");
  f->Write_line("\rName1  : info   ");
  f->Write_line("\rName2  : debug");
  f->Write_line("\r");
  f->Write_line("\r# comment");
  f->Write_line("\rName3  :  warning");
  f->Write_line("\r# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  auto loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat");

  ASSERT_EQ(loadedData.size(), 3U);

  EXPECT_STREQ(loadedData[0].first.c_str(), "Name1");
  EXPECT_EQ(loadedData[0].second, LogLevel::InfoOrAbove);

  EXPECT_STREQ(loadedData[1].first.c_str(), "Name2");
  EXPECT_EQ(loadedData[1].second, LogLevel::DebugOrAbove);

  EXPECT_STREQ(loadedData[2].first.c_str(), "Name3");
  EXPECT_EQ(loadedData[2].second, LogLevel::WarningOrAbove);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_InvalidName)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name1  : info   ");
  f->Write_line("       : debug");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name3  :  warning");
  f->Write_line("# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<gpcc::log::ILogFacilityCtrl::tLogSrcConfig> loadedData;
  ASSERT_THROW(loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat"), std::runtime_error);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_InvalidLogLevel1)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name1  : info   ");
  f->Write_line("Name2  : DEBUG");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name3  :  warning");
  f->Write_line("# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<gpcc::log::ILogFacilityCtrl::tLogSrcConfig> loadedData;
  ASSERT_THROW(loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat"), std::runtime_error);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_InvalidLogLevel2)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name1  : info   ");
  f->Write_line("Name2  : ");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name3  :  warning");
  f->Write_line("# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<gpcc::log::ILogFacilityCtrl::tLogSrcConfig> loadedData;
  ASSERT_THROW(loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat"), std::runtime_error);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_InvalidFormat1)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name1  : info   ");
  f->Write_line("Name2  = debug");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name3  :  warning");
  f->Write_line("# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<gpcc::log::ILogFacilityCtrl::tLogSrcConfig> loadedData;
  ASSERT_THROW(loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat"), std::runtime_error);
}

TEST_F(gpcc_log_log_tools_TestsF, ReadConfigFromTextFile_InvalidFormat2)
{
  auto f = spFS->Create("Test.dat", true);
  ON_SCOPE_EXIT() { try { f->Close(); } catch (std::exception const &) {}; };

  f->Write_line("# comment");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name1  : info   ");
  f->Write_line("Name2  :: debug");
  f->Write_line("");
  f->Write_line("# comment");
  f->Write_line("Name3  :  warning");
  f->Write_line("# comment");

  ON_SCOPE_EXIT_DISMISS();
  f->Close();

  std::vector<gpcc::log::ILogFacilityCtrl::tLogSrcConfig> loadedData;
  ASSERT_THROW(loadedData = ReadLogSrcConfigFromTextFile(*(spFS.get()), "Test.dat"), std::runtime_error);
}

} // namespace log
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
