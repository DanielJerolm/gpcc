/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc_test/log/backends/Backend_Recorder.hpp>
#include <gpcc/file_systems/linux_fs/FileStorage.hpp>
#include "src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace log {

using namespace testing;
using gpcc_tests::log::Backend_Recorder;

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
  EXPECT_TRUE(file->GetState() == gpcc::stream::IStreamReader::States::empty);
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
  EXPECT_TRUE(file->GetState() == gpcc::stream::IStreamReader::States::empty);
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
  EXPECT_TRUE(file->GetState() == gpcc::stream::IStreamReader::States::empty);
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
