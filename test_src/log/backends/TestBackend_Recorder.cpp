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

#include "Backend_Recorder.hpp"
#include "gpcc/src/file_systems/linux_fs/FileStorage.hpp"
#include "gpcc/src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace log {

using namespace testing;
using gpcc::log::Backend_Recorder;

TEST(gpcc_log_Backend_Recorder_Tests, Instantiation)
{
  Backend_Recorder uut;
  ASSERT_EQ(0U, uut.GetNbOfRecords());
}

TEST(gpcc_log_Backend_Recorder_Tests, InstantiationWithReserve)
{
  Backend_Recorder uut(16);
  ASSERT_EQ(0U, uut.GetNbOfRecords());
}

TEST(gpcc_log_Backend_Recorder_Tests, AccessRecords_AccessOutOfBounds_Empty)
{
  Backend_Recorder uut;

  std::string s;
  ASSERT_THROW(s = uut[0], std::out_of_range);
}

TEST(gpcc_log_Backend_Recorder_Tests, AccessRecords_AccessOutOfBounds_NotEmpty)
{
  Backend_Recorder uut;
  uut.Process("Test", gpcc::log::LogType::Debug);

  std::string s;
  ASSERT_THROW(s = uut[1], std::out_of_range);
}

TEST(gpcc_log_Backend_Recorder_Tests, RecordAndPrint_NoClear)
{
  Backend_Recorder uut;
  uut.Process("Test", gpcc::log::LogType::Debug);
  EXPECT_EQ(1U, uut.GetNbOfRecords());

  uut.PrintToStdout(false);
  EXPECT_EQ(1U, uut.GetNbOfRecords());
}

TEST(gpcc_log_Backend_Recorder_Tests, RecordAndPrint_Clear)
{
  Backend_Recorder uut;
  uut.Process("Test", gpcc::log::LogType::Debug);
  EXPECT_EQ(1U, uut.GetNbOfRecords());

  uut.PrintToStdout(true);
  EXPECT_EQ(0U, uut.GetNbOfRecords());
}

TEST(gpcc_log_Backend_Recorder_Tests, RecordAndWriteToFile_NoClear)
{
  gpcc::file_systems::linux_fs::internal::UnitTestDirProvider utdp;
  gpcc::file_systems::linux_fs::FileStorage fs(utdp.GetAbsPath());

  Backend_Recorder uut;
  uut.Process("Test1", gpcc::log::LogType::Debug);
  uut.Process("Test2", gpcc::log::LogType::Debug);
  EXPECT_EQ(2U, uut.GetNbOfRecords());

  uut.WriteToFile(false, utdp.GetAbsPath() + "Test.txt");
  EXPECT_EQ(2U, uut.GetNbOfRecords());

  // Check file's content
  auto file = fs.Open("Test.txt");

  std::string line;
  line = file->Read_line();
  EXPECT_TRUE(line == "Test1");
  line = file->Read_line();
  EXPECT_TRUE(line == "Test2");
  EXPECT_TRUE(file->GetState() == gpcc::Stream::IStreamReader::States::empty);
  file->Close();
}

TEST(gpcc_log_Backend_Recorder_Tests, RecordAndWriteToFile_Clear)
{
  gpcc::file_systems::linux_fs::internal::UnitTestDirProvider utdp;
  gpcc::file_systems::linux_fs::FileStorage fs(utdp.GetAbsPath());

  Backend_Recorder uut;
  uut.Process("Test1", gpcc::log::LogType::Debug);
  uut.Process("Test2", gpcc::log::LogType::Debug);
  EXPECT_EQ(2U, uut.GetNbOfRecords());

  uut.WriteToFile(true, utdp.GetAbsPath() + "Test.txt");
  EXPECT_EQ(0U, uut.GetNbOfRecords());

  // Check file's content
  auto file = fs.Open("Test.txt");

  std::string line;
  line = file->Read_line();
  EXPECT_TRUE(line == "Test1");
  line = file->Read_line();
  EXPECT_TRUE(line == "Test2");
  EXPECT_TRUE(file->GetState() == gpcc::Stream::IStreamReader::States::empty);
  file->Close();
}

TEST(gpcc_log_Backend_Recorder_Tests, RecordAndWriteToFile_OverwriteFile)
{
  gpcc::file_systems::linux_fs::internal::UnitTestDirProvider utdp;
  gpcc::file_systems::linux_fs::FileStorage fs(utdp.GetAbsPath());

  // create a file with some content
  {
    auto file = fs.Create("Test.txt", true);
    file->Write_string("Some content");
    file->Close();
  }

  Backend_Recorder uut;
  uut.Process("Test1", gpcc::log::LogType::Debug);
  uut.Process("Test2", gpcc::log::LogType::Debug);
  EXPECT_EQ(2U, uut.GetNbOfRecords());

  uut.WriteToFile(true, utdp.GetAbsPath() + "Test.txt");

  // Check file's content
  auto file = fs.Open("Test.txt");

  std::string line;
  line = file->Read_line();
  EXPECT_TRUE(line == "Test1");
  line = file->Read_line();
  EXPECT_TRUE(line == "Test2");
  EXPECT_TRUE(file->GetState() == gpcc::Stream::IStreamReader::States::empty);
  file->Close();
}

TEST(gpcc_log_Backend_Recorder_Tests, Clear)
{
  Backend_Recorder uut;
  uut.Process("Test", gpcc::log::LogType::Debug);
  EXPECT_EQ(1U, uut.GetNbOfRecords());

  uut.Clear();
  EXPECT_EQ(0U, uut.GetNbOfRecords());
}

TEST(gpcc_log_Backend_Recorder_Tests, AccessRecords)
{
  Backend_Recorder uut;
  uut.Process("Test1", gpcc::log::LogType::Debug);
  uut.Process("Test2", gpcc::log::LogType::Debug);
  EXPECT_EQ(2U, uut.GetNbOfRecords());
  EXPECT_TRUE("Test1" == uut[0]);
  EXPECT_TRUE("Test2" == uut[1]);
}

} // namespace log
} // namespace gpcc_tests
