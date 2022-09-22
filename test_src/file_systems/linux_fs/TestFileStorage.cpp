/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#include <gpcc/file_systems/linux_fs/FileStorage.hpp>
#include <gpcc/file_systems/exceptions.hpp>
#include <gpcc/stream/stream_errors.hpp>
#include "src/file_systems/linux_fs/internal/tools.hpp"
#include "src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include "gtest/gtest.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <cerrno>

namespace gpcc_tests    {
namespace file_systems  {
namespace linux_fs      {

using namespace testing;
using namespace gpcc::file_systems;
using namespace gpcc::file_systems::linux_fs;
using namespace gpcc::file_systems::linux_fs::internal;

// Test fixture for class FileStorage.
// The test fixture creates a test folder referenced by "baseDir" where the unit test
// can do whatever it likes. Afterwards, the test folder and its contents are removed again.
class gpcc_file_systems_linux_fs_FileStorage_TestsF: public Test
{
  public:
    gpcc_file_systems_linux_fs_FileStorage_TestsF(void);

  protected:
    // Manages creation and removal of test directory.
    UnitTestDirProvider testDirProvider;

    // path of the test folder, with trailing '/'
    std::string const baseDir;

    // unit under test
    std::unique_ptr<FileStorage> spUUT;

    void SetUp(void) override;
    void TearDown(void) override;

    void CreateDir(std::string const & name);
};

gpcc_file_systems_linux_fs_FileStorage_TestsF::gpcc_file_systems_linux_fs_FileStorage_TestsF(void)
: Test()
, testDirProvider()
, baseDir(testDirProvider.GetAbsPath())
, spUUT()
{
}

void gpcc_file_systems_linux_fs_FileStorage_TestsF::SetUp(void)
{
  spUUT.reset(new FileStorage(baseDir));
}

void gpcc_file_systems_linux_fs_FileStorage_TestsF::TearDown(void)
{
  spUUT.reset();
}

void gpcc_file_systems_linux_fs_FileStorage_TestsF::CreateDir(std::string const & name)
{
  // Creates a folder in the test directory.
  // This offers an alternative way to create folders instead of using the UUT.

  std::string const s = baseDir + name;
  if (mkdir(s.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_FileStorage_TestsF.CreateDir:  mkdir failed");
}

// alias for death tests
using gpcc_file_systems_linux_fs_FileStorage_DeathTestsF = gpcc_file_systems_linux_fs_FileStorage_TestsF;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Construction)
{
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Construction_Error_MissingTrailingFwdSlash)
{
  std::string s = baseDir;
  RemoveTrailingForwardSlash(s);

  std::unique_ptr<FileStorage> spUUT2;
  ASSERT_THROW(spUUT2.reset(new FileStorage(s)), std::invalid_argument);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Construction_Error_DirectoryNotExisting)
{
  std::unique_ptr<FileStorage> spUUT2;
  ASSERT_THROW(spUUT2.reset(new FileStorage(baseDir + "notExistingDirectory/")), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Construction_Error_ExistingButFile)
{
  auto spWriter = spUUT->Create("Test", true);
  *spWriter << "Test";
  spWriter->Close();
  spWriter.reset();

  std::unique_ptr<FileStorage> spUUT2;
  ASSERT_THROW(spUUT2.reset(new FileStorage(baseDir + "Test/")), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_DeathTestsF, Destruction_FileStillOpenForReading)
{
  ::testing::FLAGS_gtest_death_test_style = "fast";

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(55U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  EXPECT_DEATH(spUUT.reset(), ".*Not all files closed.*");

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_DeathTestsF, Destruction_FileStillOpenForWriting)
{
  ::testing::FLAGS_gtest_death_test_style = "fast";

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(55U);

  EXPECT_DEATH(spUUT.reset(), ".*Not all files closed.*");

  spISW->Close();
  spISW.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_InvalidFileName1)
{
  // This checks that file names violating the basic rules are not accepted

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;

  ASSERT_THROW(spISW = spUUT->Create("-BadFile.txt", false), InvalidFileNameError);
  ASSERT_THROW(spISW = spUUT->Create("/GoodFile.txt", false), InvalidFileNameError);
  ASSERT_THROW(spISW = spUUT->Create("folder_name/../folder_name/GoodFile.txt", false), InvalidFileNameError);
  ASSERT_THROW(spISW = spUUT->Create("-bad_folder_name/../-bad_folder_name/GoodFile.txt", false), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_InvalidFileName2)
{
  // This checks that non-portable folder names are not accepted

  CreateDir("-bad_folder_name");

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  ASSERT_NO_THROW(spISW = spUUT->Create("-bad_folder_name/GoodFile.txt", true));
  spISW->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_NoOverwrite)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spISW = spUUT->Create("Test.dat", false), FileAlreadyExistingError);

  auto spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_Overwrite)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test 1");
  spISW->Close();

  spISW = spUUT->Create("Test.dat", true);
  *spISW << std::string("Test 2");
  spISW->Close();

  auto spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test 2");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_AttemptToOverwriteEmptyDir)
{
  CreateDir("dir");

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  ASSERT_THROW(spISW = spUUT->Create("dir", false), FileAlreadyExistingError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_AttemptToOverwriteNotEmptyDir)
{
  CreateDir("dir");

  auto spISW = spUUT->Create("dir/file.dat", false);
  *spISW << "Test";
  spISW->Close();
  spISW.reset();

  ASSERT_THROW(spISW = spUUT->Create("dir", false), FileAlreadyExistingError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_DirectoryNotExisting)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  ASSERT_THROW(spISW = spUUT->Create("dir/Test.dat", true), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_LockedByWriter)
{
  auto spISW1 = spUUT->Create("Test.dat", false);
  *spISW1 << std::string("Test");

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW2;
  ASSERT_THROW(spISW2 = spUUT->Create("Test.dat", true), FileAlreadyAccessedError);
  spISW1->Close();

  auto spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Create_LockedByReader)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");
  ASSERT_THROW(spISW = spUUT->Create("Test.dat", true), FileAlreadyAccessedError);
  spISR->Close();

  spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_DestroyWithoutClose)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << static_cast<uint8_t>(0xDE);
  *spISW << static_cast<uint8_t>(0xAD);
  *spISW << static_cast<uint8_t>(0xBE);
  *spISW << static_cast<uint8_t>(0xEF);
  spISW.reset(); // note: no Close()

  auto spISR = spUUT->Open("Test.dat");
  uint8_t data[4];
  *spISR >> data[0];
  *spISR >> data[1];
  *spISR >> data[2];
  *spISR >> data[3];
  spISR->Close();

  EXPECT_EQ(data[0], 0xDEU);
  EXPECT_EQ(data[1], 0xADU);
  EXPECT_EQ(data[2], 0xBEU);
  EXPECT_EQ(data[3], 0xEFU);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_WriteBitsOneByOne)
{
  uint8_t const someBits[3] = { 0x24U, 0xB6U, 0xF2U };

  auto spISW = spUUT->Create("Test.dat", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bits(0x0E, 4U);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bits(someBits, 20);
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");
  uint8_t readData[4];
  spISR->Read_uint8(readData, 4);
  spISR->Close();
  spISR.reset();

  EXPECT_EQ(0xB9, readData[0]);
  EXPECT_EQ(0x90, readData[1]);
  EXPECT_EQ(0xD8, readData[2]);
  EXPECT_EQ(0x0A, readData[3]);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_WriteBits4Plus1Byte)
{
  auto spISW = spUUT->Create("Test.dat", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_uint8(0xABU);
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");
  uint8_t readData[2];
  spISR->Read_uint8(readData, 2U);
  spISR->Close();
  spISR.reset();

  EXPECT_EQ(0x09U, readData[0]);
  EXPECT_EQ(0xABU, readData[1]);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_WriteBits4Plus2Bytes)
{
  uint8_t const someData[2] = { 0xACU, 0x6FU };
  auto spISW = spUUT->Create("Test.dat", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_uint8(someData, 2U);
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");
  uint8_t readData[3];
  spISR->Read_uint8(readData, 3U);
  spISR->Close();
  spISR.reset();

  EXPECT_EQ(0x09U, readData[0]);
  EXPECT_EQ(0xACU, readData[1]);
  EXPECT_EQ(0x6FU, readData[2]);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_WriteBits4ThenClose)
{
  auto spISW = spUUT->Create("Test.dat", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");
  uint8_t readData;
  *spISR >> readData;
  spISR->Close();
  spISR.reset();

  EXPECT_EQ(0x09U, readData);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_Endian)
{
  auto spISW = spUUT->Create("Test.dat", false);
  EXPECT_EQ(gpcc::stream::IStreamWriter::Endian::Little, spISW->GetEndian());
  spISW->Close();
  spISW.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_RemainingCapacitySupported)
{
  auto spISW = spUUT->Create("Test.dat", false);

  ASSERT_FALSE(spISW->IsRemainingCapacitySupported());

  spISW->Close();
  spISW.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_RemainingCapacity)
{
  auto spISW = spUUT->Create("Test.dat", false);

  EXPECT_THROW((void)spISW->RemainingCapacity(), std::logic_error);

  spISW->Close();
  EXPECT_THROW((void)spISW->RemainingCapacity(), gpcc::stream::ClosedError);

  spISW.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_GetNbOfCachedBits)
{
  auto spISW = spUUT->Create("Test.dat", false);

  ASSERT_EQ(gpcc::stream::IStreamWriter::States::open, spISW->GetState());

  spISW->Write_uint8(0xAB);
  EXPECT_EQ(0U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(1U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(2U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(3U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(4U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(5U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(6U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(7U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(0U, spISW->GetNbOfCachedBits());

  spISW->Close();

  ASSERT_EQ(gpcc::stream::IStreamWriter::States::closed, spISW->GetState());
  EXPECT_THROW(spISW->GetNbOfCachedBits(), gpcc::stream::ClosedError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_States)
{
  auto spISW = spUUT->Create("Test.dat", false);
  EXPECT_EQ(gpcc::stream::IStreamWriter::States::open, spISW->GetState());

  spISW->Write_uint8(0x12U);
  EXPECT_EQ(gpcc::stream::IStreamWriter::States::open, spISW->GetState());

  spISW->Close();
  EXPECT_EQ(gpcc::stream::IStreamWriter::States::closed, spISW->GetState());

  spISW.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_InvalidFileName1)
{
  // This checks that file names violating the basic rules are not accepted

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;

  ASSERT_THROW(spISR = spUUT->Open("/BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("BadFile.txt/"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("dir//BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("."), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open(".."), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("./dir/BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("../dir/BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("dir/./BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("dir/../BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("BadFile/."), InvalidFileNameError);
  ASSERT_THROW(spISR = spUUT->Open("BadFile/.."), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_InvalidFileName2)
{
  // This checks that file names violating the rules for portable file names (but not the "basic rules")
  // are accepted

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = spUUT->Open("-BadFile.txt"), NoSuchFileError);
  ASSERT_THROW(spISR = spUUT->Open("-BadFolder/file.txt"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_FileNotExisting)
{
  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = spUUT->Open("NotExistingFile.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_DirNotExisting)
{
  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = spUUT->Open("NotExistingDir/File.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_Directory)
{
  CreateDir("dir");

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;

  ASSERT_THROW(spISR = spUUT->Open("dir"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_LockedByWriter)
{
  // ensure that a file is existing before overwriting it
  auto spISW = spUUT->Create("Test.dat", true);
  *spISW << std::string("Test 1");
  spISW->Close();
  spISW.reset();

  spISW = spUUT->Create("Test.dat", true);
  *spISW << std::string("Test 2");

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = spUUT->Open("Test.dat"), FileAlreadyAccessedError);

  spISW->Close();

  spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test 2");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Open_LockedByReader)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test 1");
  *spISW << std::string("Test 2");
  spISW->Close();

  std::string s;
  auto spISR1 =  spUUT->Open("Test.dat");
  *spISR1 >> s;
  EXPECT_TRUE(s == "Test 1");

  auto spISR2 = spUUT->Open("Test.dat");
  *spISR2 >> s;
  EXPECT_TRUE(s == "Test 1");
  *spISR2 >> s;
  EXPECT_TRUE(s == "Test 2");

  *spISR1 >> s;
  EXPECT_TRUE(s == "Test 2");

  spISR1->Close();
  spISR2->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_DestroyReaderWithoutClose)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint32(0x12345678U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  spISR.reset(); // note: no Close()

  spISR = spUUT->Open("Test.dat");
  uint8_t data[4];
  spISR->Read_uint8(data, 4U);
  EXPECT_EQ(0x78U, data[0]);
  EXPECT_EQ(0x56U, data[1]);
  EXPECT_EQ(0x34U, data[2]);
  EXPECT_EQ(0x12U, data[3]);
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_EmptyFile)
{
  auto spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadByteFromEmptyFile)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  uint8_t data;
  ASSERT_THROW(*spISR >> data, gpcc::stream::EmptyError);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_IsRemainingBytesSupported)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_FALSE(spISR->IsRemainingBytesSupported());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_RemainingBytes)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x12U);
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), std::logic_error);

  spISR->Skip(8U);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), std::logic_error);

  ASSERT_THROW(spISR->Skip(8U), gpcc::stream::EmptyError);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), gpcc::stream::ErrorStateError);

  spISR->Close();
  EXPECT_THROW((void)spISR->RemainingBytes(), gpcc::stream::ClosedError);

  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBytesFromEmptyFile)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  uint8_t data[2];
  ASSERT_THROW(spISR->Read_uint8(data, 2U), gpcc::stream::EmptyError);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBitFromEmptyFile)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  bool data;
  ASSERT_THROW(data = spISR->Read_bit(), gpcc::stream::EmptyError);
  (void)data;

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBitsFromEmptyFile)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  uint8_t data;
  ASSERT_THROW(data = spISR->Read_bits(3U), gpcc::stream::EmptyError);
  (void)data;

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadManyBitsFromEmptyFile)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  uint8_t data[2];
  ASSERT_THROW(spISR->Read_bits(data, 9U), gpcc::stream::EmptyError);
  (void)data;

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadByteAndFileBecomesEmpty)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(55U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  uint8_t data;
  *spISR >> data;
  EXPECT_EQ(55U, data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_Read2ByteAndFileBecomesEmpty)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(55U);
  spISW->Write_uint8(66U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  uint8_t data;
  *spISR >> data;
  EXPECT_EQ(55U, data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_EQ(66U, data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBytesAndFileBecomesEmpty)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(55U);
  spISW->Write_uint8(66U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  uint8_t data[2];
  spISR->Read_uint8(data, 2U);
  EXPECT_EQ(55U, data[0]);
  EXPECT_EQ(66U, data[1]);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBitsAndFileBecomesEmpty)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x55U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  bool data;

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadByteAndBitsAndFileBecomesEmpty)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x33U);
  spISW->Write_uint8(0x55U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  uint8_t u8;
  *spISR >> u8;
  EXPECT_EQ(0x33U, u8);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  bool data;

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_TRUE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  EXPECT_FALSE(data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadByte_ErrorOnlyFewBitsLeft)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  *spISW << static_cast<uint8_t>(1U);
  *spISW << static_cast<uint8_t>(2U);
  *spISW << static_cast<uint8_t>(3U);
  *spISW << static_cast<uint8_t>(0xEEU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;

  *spISR >> data;
  EXPECT_EQ(1U, data);
  *spISR >> data;
  EXPECT_EQ(2U, data);
  *spISR >> data;
  EXPECT_EQ(3U, data);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  spISR->Read_bits(&data, 4U);

  EXPECT_EQ(0x0E, data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  EXPECT_THROW(*spISR >> data, gpcc::stream::EmptyError);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBytes_ErrorOnlyFewBitsLeft)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  *spISW << static_cast<uint8_t>(1U);
  *spISW << static_cast<uint8_t>(2U);
  *spISW << static_cast<uint8_t>(3U);
  *spISW << static_cast<uint8_t>(0xEEU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;

  *spISR >> data;
  EXPECT_EQ(1U, data);
  *spISR >> data;
  EXPECT_EQ(2U, data);
  *spISR >> data;
  EXPECT_EQ(3U, data);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  spISR->Read_bits(&data, 4U);

  EXPECT_EQ(0x0EU, data);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  uint8_t data2[2];
  EXPECT_THROW(spISR->Read_uint8(data2, 2U), gpcc::stream::EmptyError);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadBits_ErrorEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  spISR->Read_bits(&data, 4U);
  EXPECT_EQ(0x02U, data);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  EXPECT_THROW(spISR->Read_bits(&data, 8), gpcc::stream::EmptyError);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, ReadWriteBits_ProperInsertionOfGaps)
{
  uint8_t data = 0;

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);

  spISW->Write_uint8(0x12U);

  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);

  spISW->Write_uint8(0xDEU);
  spISW->Write_uint8(0xADU);

  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(true);

  spISW->Write_uint8(&data, 0);

  spISW->Write_Bit(false);
  spISW->Write_Bit(true);

  spISW->Write_uint8(0xBEU);
  spISW->Write_uint8(0xEFU);

  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  bool bit;
  *spISR >> data;
  EXPECT_EQ(0x12U, data);

  *spISR >> bit;
  EXPECT_TRUE(bit);
  *spISR >> bit;
  EXPECT_FALSE(bit);
  *spISR >> bit;
  EXPECT_TRUE(bit);
  *spISR >> bit;
  EXPECT_FALSE(bit);

  *spISR >> data;
  EXPECT_EQ(0xDEU, data);
  *spISR >> data;
  EXPECT_EQ(0xADU, data);

  *spISR >> bit;
  EXPECT_FALSE(bit);
  *spISR >> bit;
  EXPECT_FALSE(bit);

  // Note that the read(0) and write(0) are placed at different positions within the read/written bits.
  spISR->Read_uint8(&data, 0);

  *spISR >> bit;
  EXPECT_TRUE(bit);
  *spISR >> bit;
  EXPECT_TRUE(bit);
  *spISR >> bit;
  EXPECT_FALSE(bit);
  *spISR >> bit;
  EXPECT_TRUE(bit);

  *spISR >> data;
  EXPECT_EQ(0xBEU, data);
  *spISR >> data;
  EXPECT_EQ(0xEFU, data);

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, ReadWriteString_Basic)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Str1");
  *spISW << std::string("Str2");
  *spISW << std::string("");
  spISW->Write_char("Str3", 5U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Str1");
  *spISR >> s;
  EXPECT_TRUE(s == "Str2");
  *spISR >> s;
  EXPECT_TRUE(s == "");
  *spISR >> s;
  EXPECT_TRUE(s == "Str3");
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, ReadWriteString_NoNullTerminator)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Str1");
  *spISW << std::string("Str2");
  spISW->Write_char("Str3", 4U); // note: no null-terminator is written
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Str1");
  *spISR >> s;
  EXPECT_TRUE(s == "Str2");
  EXPECT_THROW(*spISR >> s, gpcc::stream::EmptyError);
  EXPECT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyErrorClosed)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s;
  EXPECT_THROW(s = spISR->Read_line(), gpcc::stream::EmptyError);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  EXPECT_THROW(s = spISR->Read_line(), gpcc::stream::ErrorStateError);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();

  EXPECT_THROW(s = spISR->Read_line(), gpcc::stream::ClosedError);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());

  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_DifferentLineEndings)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_string("Line1\nLine2\rLine3\r\nLine4");
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "Line1");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Line2");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Line3");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Line4");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_NUL_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x00);
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_LF_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('\n');
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_CR_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('\r');
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_CRLF_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_NUL_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x00);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_LF_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('\n');
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_CR_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('\r');
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_EmptyStr_CRLF_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_NUL_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_uint8(0x00);
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_LF_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_char('\n');
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_CR_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_char('\r');
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_CRLF_notEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  *spISW << static_cast<uint8_t>(0x55);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_NUL_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_uint8(0x00);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_LF_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_char('\n');
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_CR_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_char('\r');
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_CRLF_EOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char('A');
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "A");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileReader_ReadLine_NoTermCharAtEOF)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_char("ABC", 3);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_WriteLine)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_line("ABC");
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  char buffer[4];
  spISR->Read_char(buffer, 4);

  EXPECT_EQ(buffer[0], 'A');
  EXPECT_EQ(buffer[1], 'B');
  EXPECT_EQ(buffer[2], 'C');
  EXPECT_EQ(buffer[3], '\n');

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIOFileWriter_WriteLine_EmptyLine)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_line("");
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  char buffer[1];
  spISR->Read_char(buffer, 1);

  EXPECT_EQ(buffer[0], '\n');

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, AlignToByteBoundary_OK)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);

  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  EXPECT_EQ(6U, spISW->AlignToByteBoundary(false));

  spISW->FillBits(12, false);
  EXPECT_EQ(4U, spISW->AlignToByteBoundary(true));

  spISW->Write_uint8(0xDEU);
  EXPECT_EQ(0U, spISW->AlignToByteBoundary(false));

  spISW->Close();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  EXPECT_EQ(0x01U, spISR->Read_uint8());
  EXPECT_EQ(0x00U, spISR->Read_uint8());
  EXPECT_EQ(0xF0U, spISR->Read_uint8());
  EXPECT_EQ(0xDEU, spISR->Read_uint8());

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, AlignToByteBoundary_StateClosed)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);

  spISW->Write_uint32(0xDEADBEEFUL);

  spISW->Close();

  EXPECT_THROW((void)spISW->AlignToByteBoundary(false), gpcc::stream::ClosedError);

  EXPECT_EQ(gpcc::stream::IStreamWriter::States::closed, spISW->GetState());

  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  EXPECT_EQ(0xDEADBEEFUL, spISR->Read_uint32());

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, FillBitsAndBytes_OK)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);

  spISW->FillBits(1, true);
  spISW->FillBits(1, false);
  spISW->Write_Bits(static_cast<uint8_t>(0x0FU), 4);
  spISW->FillBytes(1, 0xFFU);
  spISW->FillBytes(2, 0x55U);
  spISW->FillBits(16, false);

  spISW->FillBits(0, false);
  spISW->FillBytes(0, 0);

  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  EXPECT_EQ(0x3DU, spISR->Read_uint8());
  EXPECT_EQ(0xFFU, spISR->Read_uint8());
  EXPECT_EQ(0x55U, spISR->Read_uint8());
  EXPECT_EQ(0x55U, spISR->Read_uint8());
  EXPECT_EQ(0x00U, spISR->Read_uint8());
  EXPECT_EQ(0x00U, spISR->Read_uint8());

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, FillBitsAndBytes_StateClosed)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);

  spISW->Write_uint32(0xDEADBEEFUL);

  spISW->Close();

  EXPECT_THROW(spISW->FillBits(1, true), gpcc::stream::ClosedError);
  EXPECT_THROW(spISW->FillBytes(1, 0x55U), gpcc::stream::ClosedError);

  EXPECT_EQ(gpcc::stream::IStreamWriter::States::closed, spISW->GetState());

  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  EXPECT_EQ(0xDEADBEEFUL, spISR->Read_uint32());

  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_ZeroBits)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x57U);
  spISW->Write_uint8(0xE9U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t u8;

  spISR->Skip(0U);

  u8 = spISR->Read_uint8();
  ASSERT_EQ(0x57U, u8);

  spISR->Skip(0U);

  u8 = spISR->Read_uint8();
  ASSERT_EQ(0xE9U, u8);

  spISR->Skip(0U);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsLeft_SkipSomeBits)
{
  // There are 4 bits left that have not been read yet. We skip 3 of them.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(3U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bits(1U), 0x01U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndOneByteLeft_SkipAllBits)
{
  // There are 4 bits + 1 Byte left that have not been read yet. We skip 4 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(4U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0xDBU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsLeft_SkipAll)
{
  // There are 4 bits left that have not been read yet. We skip them all.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(4U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsLeft_SkipAllPlusOne)
{
  // There are 4 bits left that have not been read yet. We skip them all + 1.
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(5U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndOneByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 12 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(12U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndTwoByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+8=20 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(20U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndThreeByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+8+8=28 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(28U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndFourByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+8+8+8=36 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(36U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndOneByteAndOneBit)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+1 = 13 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(13U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndTwoByteLeft_SkipAllBitsAndOneByte)
{
  // There are 4 bits + 2 byte left that have not been read yet. We skip 4+8=12 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Write_uint8(0x36U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(12U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x36U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_BitsAndTwoByteLeft_SkipAllBitsAndOneByteAndOneBit)
{
  // There are 4 bits + 2 byte left that have not been read yet. We skip 4+8+1=13 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Write_uint8(0x36U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(13U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bits(7U), 0x1BU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_OneByteLeft_Skip8Bits)
{
  // There is 1 byte left that has not been read yet. We skip 8 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(8U), 0x8AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(8U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_OneByteLeft_Skip7Bits)
{
  // There is 1 byte left that has not been read yet. We skip 7 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(8U), 0x8AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(7U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bit(), 0x01U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_OneByteLeft_Skip9Bits)
{
  // There is 1 byte left that has not been read yet. We skip 9 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(8U), 0x8AU);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(9U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_TwoByteLeft_Skip8Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 8 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  spISR->Skip(8U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x80U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_TwoByteLeft_Skip16Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 16 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  spISR->Skip(16U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_TwoByteLeft_Skip9Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 9 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  spISR->Skip(9U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bits(7U), 0x40U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_ThreeByteLeft_Skip2Bytes)
{
  // There are 3 bytes left that have not been read yet. We skip 16 bits.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x23U);
  spISW->Write_uint8(0x34U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  spISR->Skip(16U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x34U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_FourByteLeft_Skip3Bytes)
{
  // There are 4 bytes left that have not been read yet. We skip 3 bytes.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x23U);
  spISW->Write_uint8(0x34U);
  spISW->Write_uint8(0x45U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  spISR->Skip(24U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x45U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_FiveByteLeft_Skip4Bytes)
{
  // There are 5 bytes left that have not been read yet. We skip 4 bytes.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x23U);
  spISW->Write_uint8(0x34U);
  spISW->Write_uint8(0x45U);
  spISW->Write_uint8(0x56U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  spISR->Skip(32U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x56U);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_OneByteLeft_Skip4Bytes)
{
  // There are 5 bytes left that have not been read yet. We skip 4 bytes.

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // - precondition established -

  ASSERT_THROW(spISR->Skip(32U), gpcc::stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_EmptyStream)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  ASSERT_EQ(spISR->Read_uint8(), 0xFAU);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::open, spISR->GetState());

  ASSERT_EQ(spISR->Read_uint8(), 0x12U);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());

  ASSERT_THROW(spISR->Skip(1U), gpcc::stream::EmptyError);

  ASSERT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_ClosedStream)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");
  spISR->Close();

  ASSERT_THROW(spISR->Skip(1U), gpcc::stream::ClosedError);
  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Skip_StreamInErrorState)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  uint8_t au8[3];
  ASSERT_THROW(spISR->Read_uint8(au8, 3), gpcc::stream::EmptyError);

  ASSERT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  ASSERT_THROW(spISR->Skip(1U), gpcc::stream::ErrorStateError);

  ASSERT_EQ(gpcc::stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();

  ASSERT_EQ(gpcc::stream::IStreamReader::States::closed, spISR->GetState());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIoFileReader_EnsureAllDataConsumed_OK_1)
{
  using namespace gpcc::stream;

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x13U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // (3 bytes left) -------------------------------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_uint16(); // (1 byte left) -------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (7 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (6 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (5 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (4 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (3 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (2 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (1 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (0 bit left) -----------------------------------------------------------------------
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::empty);

  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIoFileReader_EnsureAllDataConsumed_OK_2)
{
  using namespace gpcc::stream;

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // (2 bytes left) -------------------------------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    (void)spISR->Read_bit();
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
    EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
    EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

    EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);
  }

  (void)spISR->Read_bit(); // (7 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (6 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (5 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (4 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (3 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (2 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (1 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (0 bit left) -----------------------------------------------------------------------
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::empty);

  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIoFileReader_EnsureAllDataConsumed_ErrorState)
{
  using namespace gpcc::stream;

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // create error condition
  ASSERT_THROW((void)spISR->Read_uint32(), EmptyError);

  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any), ErrorStateError);

  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, StdIoFileReader_EnsureAllDataConsumed_ClosedState)
{
  using namespace gpcc::stream;

  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  spISW = spUUT->Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  // create pre-condition
  spISR->Close();

  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any), ClosedError);
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_InvalidFileName1)
{
  // This checks that file names violating the "basic rules" are not accepted

  ASSERT_THROW(spUUT->Delete("/BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("BadFile.txt/"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("dir//BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("."), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete(".."), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("./dir/BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("../dir/BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("dir/./BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("dir/../BadFile.txt"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("BadFile/."), InvalidFileNameError);
  ASSERT_THROW(spUUT->Delete("BadFile/.."), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_InvalidFileName2)
{
  // This checks that file names violating the rules for portable file names (but not the "basic rules")
  // are accepted
  ASSERT_THROW(spUUT->Delete("-Test.dat"), NoSuchFileError);
  ASSERT_THROW(spUUT->Delete("-Folder/Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_NoSuchFile)
{
  ASSERT_THROW(spUUT->Delete("Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_Directory)
{
  CreateDir("dir");

  ASSERT_THROW(spUUT->Delete("dir"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_LockedByWriter)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test 1");

  ASSERT_THROW(spUUT->Delete("Test.dat"), FileAlreadyAccessedError);

  spISW->Close();
  spISW.reset();

  spISW = spUUT->Create("Test.dat", true);
  *spISW << std::string("Test 2");

  ASSERT_THROW(spUUT->Delete("Test.dat"), FileAlreadyAccessedError);

  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");

  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test 2");
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_LockedByReader)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");

  ASSERT_THROW(spUUT->Delete("Test.dat"), FileAlreadyAccessedError);

  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test");
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Delete_OK)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();
  spISW.reset();

  spUUT->Delete("Test.dat");

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = spUUT->Open("Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_InvalidFileName1)
{
  // This checks that "old file names" violating the "basic rules" are not accepted

  ASSERT_THROW(spUUT->Rename("/BadFile.txt", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("BadFile.txt/", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("dir//BadFile.txt", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename(".", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("..", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("./dir/BadFile.txt", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("../dir/BadFile.txt", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("dir/./BadFile.txt", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("dir/../BadFile.txt", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("BadFile/.", "File.dat"), InvalidFileNameError);
  ASSERT_THROW(spUUT->Rename("BadFile/..", "File.dat"), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_InvalidFileName2)
{
  // This checks that "old file names" violating the rules for portable file names (but not the "basic rules")
  // are accepted
  ASSERT_THROW(spUUT->Rename("-File.dat", "File2.dat"), NoSuchFileError);
  ASSERT_THROW(spUUT->Rename("-Folder/File.dat", "File2.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_InvalidFileName3)
{
  // This test checks that "new file names" violating any rules are not accepted. A path specification inside the new
  // name only needs to meet the "basic" rules.

  ASSERT_THROW(spUUT->Rename("File.dat", "-BadFile.dat"), InvalidFileNameError);

  CreateDir("-dir");
  ASSERT_THROW(spUUT->Rename("File.dat", "-dir/NewName.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SrcLockedByReader)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  auto spISR = spUUT->Open("Test.dat");

  ASSERT_THROW(spUUT->Rename("Test.dat", "Test2.dat"), FileAlreadyAccessedError);

  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SrcLockedByWriter)
{
  auto spISW = spUUT->Create("Test.dat", true);
  *spISW << std::string("Test 1");
  spISW->Close();

  spISW = spUUT->Create("Test.dat", true);
  *spISW << std::string("Test 2");

  ASSERT_THROW(spUUT->Rename("Test.dat", "Test2.dat"), FileAlreadyAccessedError);

  spISW->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_DestLockedByReader)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test SRC");
  spISW->Close();

  spISW = spUUT->Create("Test2.dat", false);
  *spISW << std::string("Test DEST");
  spISW->Close();

  auto spISR = spUUT->Open("Test2.dat");

  ASSERT_THROW(spUUT->Rename("Test.dat", "Test2.dat"), FileAlreadyAccessedError);

  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_DestLockedByWriter)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test SRC");
  spISW->Close();

  spISW = spUUT->Create("Test2.dat", false);
  *spISW << std::string("Test DEST");

  ASSERT_THROW(spUUT->Rename("Test.dat", "Test2.dat"), FileAlreadyAccessedError);

  spISW->Close();
  spISW.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SrcNotExisting)
{
  ASSERT_THROW(spUUT->Rename("Test.dat", "Test2.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SrcNotExisting_SrcNameNotPortable)
{
  ASSERT_THROW(spUUT->Rename("-Test.dat", "Test2.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SrcIsDirectory)
{
  CreateDir("dir");

  auto spISW = spUUT->Create("dir/Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->Rename("dir", "dir2"), NoSuchFileError);

  auto spISR = spUUT->Open("dir/Test.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_DestAlreadyExisting)
{
  auto spISW = spUUT->Create("Test-src.dat", false);
  *spISW << std::string("Test SRC");
  spISW->Close();

  spISW = spUUT->Create("Test-dest.dat", false);
  *spISW << std::string("Test DEST");
  spISW->Close();

  ASSERT_THROW(spUUT->Rename("Test-src.dat", "Test-dest.dat"), FileAlreadyExistingError);

  auto spISR = spUUT->Open("Test-dest.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test DEST");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_DestIsAnExistingDirectory)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  CreateDir("dir");

  ASSERT_THROW(spUUT->Rename("Test.dat", "dir"), FileAlreadyExistingError);
  ASSERT_THROW(spUUT->Rename("Test.dat", "dir/"), InvalidFileNameError);

  auto spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_DestIsANotExistingDirectory)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->Rename("Test.dat", "dir/"), InvalidFileNameError);

  auto spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_DestDirIsNotExisting)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->Rename("Test.dat", "dir/Test.dat"), NoSuchDirectoryError);

  auto spISR = spUUT->Open("Test.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SameName_FileNotExisting)
{
  ASSERT_THROW(spUUT->Rename("Test.dat", "Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_SameName_FileExisting)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  spUUT->Rename("Test.dat", "Test.dat");

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("Test.dat");

  std::string s;
  *spISR >> s;
  EXPECT_TRUE(s == "Test");
  EXPECT_EQ(gpcc::stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_OK)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();


  spUUT->Rename("Test.dat", "Test2.dat");

  auto spISR = spUUT->Open("Test2.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();

  ASSERT_THROW(spISR = spUUT->Open("Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_MoveToOtherDir_sameName)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  CreateDir("dir");

  spUUT->Rename("Test.dat", "dir/Test.dat");

  auto spISR = spUUT->Open("dir/Test.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();

  ASSERT_THROW(spISR = spUUT->Open("Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Rename_MoveToOtherDir_otherName)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();

  CreateDir("dir");

  spUUT->Rename("Test.dat", "dir/Test2.dat");

  auto spISR = spUUT->Open("dir/Test2.dat");
  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Test");
  spISR->Close();

  ASSERT_THROW(spISR = spUUT->Open("Test.dat"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Enumerate_Zero)
{
  auto list = spUUT->Enumerate();
  EXPECT_TRUE(list.empty());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Enumerate_One)
{
  auto spISW = spUUT->Create("Test.dat", false);
  *spISW << std::string("Test");
  spISW->Close();
  spISW.reset();

  auto list = spUUT->Enumerate();
  ASSERT_TRUE(!list.empty());
  EXPECT_TRUE(list.back() == "Test.dat");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Enumerate_n)
{
  auto spISW = spUUT->Create("Test1.dat", false);
  *spISW << std::string("Test");
  spISW->Close();
  spISW.reset();

  spISW = spUUT->Create("File5.dat", false);
  *spISW << std::string("F5");
  spISW->Close();
  spISW.reset();

  spISW = spUUT->Create("SomeData.dat", false);
  *spISW << std::string("Data");
  spISW->Close();
  spISW.reset();

  auto list = spUUT->Enumerate();
  ASSERT_TRUE(list.size() == 3U);

  auto it = list.begin();
  EXPECT_TRUE(*it == "File5.dat");
  ++it;
  EXPECT_TRUE(*it == "SomeData.dat");
  ++it;
  EXPECT_TRUE(*it == "Test1.dat");
  ++it;
  EXPECT_TRUE(it == list.end());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, Enumerate_WithDirs)
{
  auto spISW = spUUT->Create("Test1.dat", false);
  *spISW << std::string("Test");
  spISW->Close();
  spISW.reset();

  spISW = spUUT->Create("File5.dat", false);
  *spISW << std::string("F5");
  spISW->Close();
  spISW.reset();

  CreateDir("dir");
  CreateDir("dir2");

  spISW = spUUT->Create("dir/SomeData.dat", false);
  *spISW << std::string("Data");
  spISW->Close();
  spISW.reset();

  auto list = spUUT->Enumerate();
  ASSERT_TRUE(list.size() == 3U);

  auto it = list.begin();
  EXPECT_TRUE(*it == "File5.dat");
  ++it;
  EXPECT_TRUE(*it == "Test1.dat");
  ++it;
  EXPECT_TRUE(*it == "dir/SomeData.dat");
  ++it;
  EXPECT_TRUE(it == list.end());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_InvalidFileName)
{
  size_t s;
  ASSERT_THROW(s = spUUT->DetermineSize("/BadFile.dat", nullptr), InvalidFileNameError);

  // check that non-portable filename is accepted
  ASSERT_THROW(s = spUUT->DetermineSize("-BadFile.dat", nullptr), NoSuchFileError);
  (void)s;
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_OK)
{
  auto spISW = spUUT->Create("Test.dat", false);
  for (size_t i = 0; i < 43U; i++)
    spISW->Write_uint8(i);
  spISW->Close();
  spISW.reset();

  size_t totalSize;
  size_t size = spUUT->DetermineSize("Test.dat", &totalSize);

  EXPECT_EQ(43U, size);
  EXPECT_EQ(43U, totalSize);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_OK_noTotalSize)
{
  auto spISW = spUUT->Create("Test.dat", false);
  for (size_t i = 0; i < 43U; i++)
    spISW->Write_uint8(i);
  spISW->Close();
  spISW.reset();

  size_t size = spUUT->DetermineSize("Test.dat", nullptr);

  EXPECT_EQ(43U, size);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_FileNotExisting)
{
  size_t totalSize;
  size_t size;

  ASSERT_THROW(size = spUUT->DetermineSize("notExistingFile.dat", &totalSize), NoSuchFileError);
  (void)size;
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_OK_ZeroLength)
{
  auto spISW = spUUT->Create("Test.dat", false);
  spISW->Close();
  spISW.reset();

  size_t totalSize;
  size_t size = spUUT->DetermineSize("Test.dat", &totalSize);

  EXPECT_EQ(0U, size);
  EXPECT_EQ(0U, totalSize);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_LockedByReader)
{
  auto spISW = spUUT->Create("Test.dat", false);
  for (size_t i = 0; i < 43U; i++)
    spISW->Write_uint8(i);
  spISW->Close();
  spISW.reset();

  auto spISR = spUUT->Open("Test.dat");

  size_t totalSize;
  size_t size = spUUT->DetermineSize("Test.dat", &totalSize);

  EXPECT_EQ(43U, size);
  EXPECT_EQ(43U, totalSize);

  for (size_t i = 0; i < 43U; i++)
  {
    EXPECT_EQ(i, spISR->Read_uint8());
  }
  spISR->Close();
  spISR.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DetermineSize_LockedByWriter)
{
  auto spISW = spUUT->Create("Test.dat", false);
  for (size_t i = 0; i < 43U; i++)
    spISW->Write_uint8(i);
  spISW->Close();
  spISW.reset();

  spISW = spUUT->Create("Test.dat", true);

  size_t size;
  ASSERT_THROW(size = spUUT->DetermineSize("Test.dat", nullptr), FileAlreadyAccessedError);
  (void)size;

  spISW->Close();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, MultipleReadersAndWritersAtTheSameTime)
{
  std::unique_ptr<gpcc::stream::IStreamWriter> spISW;
  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  uint8_t u8;
  std::string s;

  spISW = spUUT->Create("Data1.dat", false);
  *spISW << static_cast<uint8_t>(0x12U);
  *spISW << static_cast<uint8_t>(0x13U);
  *spISW << static_cast<uint8_t>(0x14U);
  spISW->Close();

  spISW = spUUT->Create("Data2.dat", false);
  *spISW << static_cast<uint8_t>(0xF5U);
  *spISW << static_cast<uint8_t>(0x33U);
  *spISW << static_cast<uint8_t>(0xC4U);
  *spISW << static_cast<uint8_t>(0xD3U);
  spISW->Close();

  spISW = spUUT->Create("Data3.dat", false);
  *spISW << static_cast<uint8_t>(0x89U);
  *spISW << static_cast<uint8_t>(0x12U);
  *spISW << static_cast<uint8_t>(0x3EU);
  *spISW << static_cast<uint8_t>(0xF7U);
  spISW->Close();


  auto spISW1 = spUUT->Create("File1.dat", false);
  *spISW1 << std::string("ABC");
  auto spISW2 = spUUT->Create("File2.dat", false);
  auto spISW3 = spUUT->Create("File3.dat", false);


  *spISW2 << std::string("DEF");

  spISR = spUUT->Open("Data1.dat");
  *spISR >> u8;
  EXPECT_EQ(0x12U, u8);
  *spISR >> u8;
  EXPECT_EQ(0x13U, u8);
  *spISR >> u8;
  EXPECT_EQ(0x14U, u8);
  spISR->Close();

  *spISW3 << std::string("GHI");
  spISW1->Write_uint8(12U);
  spISW2->Write_uint8(13U);


  spISR = spUUT->Open("Data2.dat");
  *spISR >> u8;
  EXPECT_EQ(0xF5U, u8);
  *spISR >> u8;
  EXPECT_EQ(0x33U, u8);
  *spISR >> u8;
  EXPECT_EQ(0xC4U, u8);
  *spISR >> u8;
  EXPECT_EQ(0xD3U, u8);
  spISR->Close();


  spISW3->Write_uint8(14U);

  spISW1->Close();
  spISW1.reset();

  spISR = spUUT->Open("Data3.dat");
  *spISR >> u8;
  EXPECT_EQ(0x89U, u8);
  *spISR >> u8;
  EXPECT_EQ(0x12U, u8);
  *spISR >> u8;
  EXPECT_EQ(0x3EU, u8);
  *spISR >> u8;
  EXPECT_EQ(0xF7U, u8);
  spISR->Close();

  spISW3->Close();
  spISW3.reset();
  spISW2->Close();
  spISW2.reset();
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, GetFreeSpace)
{
  size_t const s = spUUT->GetFreeSpace();
  std::cout << "Free space: " << s << "bytes (approx. " << s / (1024UL*1024UL) << "MB (+0/-1MB))" << std::endl;
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, IsDirectoryExisting)
{
  // test "/"
  EXPECT_TRUE(spUUT->IsDirectoryExisting(""));

  // test (dir not existing)
  EXPECT_FALSE(spUUT->IsDirectoryExisting("not_existing_dir"));

  // test (dir existing)
  spUUT->CreateDirectory("test_dir");
  EXPECT_TRUE(spUUT->IsDirectoryExisting("test_dir"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, IsDirectoryExisting_InvalidDirName1)
{
  bool b;
  ASSERT_THROW(b = spUUT->IsDirectoryExisting("dir/../A"), InvalidFileNameError);
  (void)b;
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, IsDirectoryExisting_InvalidDirName2)
{
  ASSERT_FALSE(spUUT->IsDirectoryExisting("-dir1"));

  // create the test folder
  CreateDir("-dir1");

  EXPECT_TRUE(spUUT->IsDirectoryExisting("-dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, IsDirectoryExisting_InvokeOnFile)
{
  // test (file)
  auto spISW = spUUT->Create("test_file", false);
  spISW->Close();
  EXPECT_FALSE(spUUT->IsDirectoryExisting("test_file"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory)
{
  // test (create in base dir)
  spUUT->CreateDirectory("dir1");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));

  // test (create as a sub-dir)
  spUUT->CreateDirectory("dir1/dir2");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory_DirAlreadyExisting)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  ASSERT_THROW(spUUT->CreateDirectory("dir1/dir2"), DirectoryAlreadyExistingError);
  ASSERT_THROW(spUUT->CreateDirectory("dir1"), DirectoryAlreadyExistingError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory_FileWithSameNameAlreadyExisting)
{
  auto spISW = spUUT->Create("test_file", false);
  spISW->Close();

  ASSERT_THROW(spUUT->CreateDirectory("test_file"), FileAlreadyExistingError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory_InvalidName)
{
  EXPECT_THROW(spUUT->CreateDirectory(""), InvalidFileNameError);
  EXPECT_THROW(spUUT->CreateDirectory("/"), InvalidFileNameError);
  EXPECT_THROW(spUUT->CreateDirectory("dir/"), InvalidFileNameError);
  EXPECT_THROW(spUUT->CreateDirectory("-dir"), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory_InvalidParentDirName)
{
  // create the test folder
  CreateDir("-dir1");

  spUUT->CreateDirectory("-dir1/dir2");
  EXPECT_TRUE(spUUT->IsDirectoryExisting("-dir1/dir2"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory_ParentDirNotExisting)
{
  ASSERT_THROW(spUUT->CreateDirectory("dir1/dir2"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, CreateDirectory_ParentDirIsFile)
{
  auto spISW = spUUT->Create("test_file", false);
  spISW->Close();

  ASSERT_THROW(spUUT->CreateDirectory("test_file/dir2"), NoSuchDirectoryError);
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_Empty)
{
  spUUT->CreateDirectory("dir1");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));

  spUUT->DeleteDirectoryContent("dir1");

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_NotEmpty)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  auto spISW = spUUT->Create("dir1/file1", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/dir2/file2", true);
  spISW->Close();

  spUUT->DeleteDirectoryContent("dir1");

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1/dir2"));

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  EXPECT_THROW(spISR = spUUT->Open("dir1/file1"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_BaseDir)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  auto spISW = spUUT->Create("dir1/file1", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/dir2/file2", true);
  spISW->Close();

  spUUT->DeleteDirectoryContent("");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1/dir2"));

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  EXPECT_THROW(spISR = spUUT->Open("dir1/file1"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_InvalidName1)
{
  // This checks that directory names violating the "basic rules" are not accepted

  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  auto spISW = spUUT->Create("dir1/file1", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/dir2/file2", true);
  spISW->Close();

  ASSERT_THROW(spUUT->DeleteDirectoryContent("dir1/"), InvalidFileNameError);
  ASSERT_THROW(spUUT->DeleteDirectoryContent("dir1/../dir1"), InvalidFileNameError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  spISR = spUUT->Open("dir1/file1");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_InvalidName2)
{
  // This checks that directory names violating the rules for portable file/directory names
  // (but not the "basic rules") are accepted

  ASSERT_FALSE(spUUT->IsDirectoryExisting("-dir1"));

  // create the test folder
  CreateDir("-dir1");

  auto spISW = spUUT->Create("-dir1/test_file.txt", true);
  spISW->Close();

  spUUT->DeleteDirectoryContent("-dir1");

  EXPECT_TRUE(spUUT->IsDirectoryExisting("-dir1"));
  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = spUUT->Open("-dir1/test_file.txt"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_DirNotExisting)
{
  ASSERT_THROW(spUUT->DeleteDirectoryContent("not_existing_dir"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_ParentDirNotExisting)
{
  ASSERT_THROW(spUUT->DeleteDirectoryContent("not_existing_dir1/not_existing_dir2"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_InvokeOnFile)
{
  auto spISW = spUUT->Create("testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->DeleteDirectoryContent("testfile.txt"), NoSuchDirectoryError);

  auto spISR = spUUT->Open("testfile.txt");
  auto const s = spISR->Read_string();
  spISR->Close();

  EXPECT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_FileInDirOpen)
{
  spUUT->CreateDirectory("dir1");
  auto spISW = spUUT->Create("dir1/file1", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/file2", true);

  // file2 is open for writing
  ASSERT_THROW(spUUT->DeleteDirectoryContent("dir1"), DirectoryAlreadyAccessedError);

  spISW->Close();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/file1"));
  ASSERT_NO_THROW(spISR = spUUT->Open("dir1/file2"));

  // file2 is open for reading
  ASSERT_THROW(spUUT->DeleteDirectoryContent("dir1"), DirectoryAlreadyAccessedError);

  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/file1"));
  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/file2"));
  spISR->Close();

  // all files closed
  spUUT->DeleteDirectoryContent("dir1");

  EXPECT_THROW(spISR = spUUT->Open("dir1/file1"), NoSuchFileError);
  EXPECT_THROW(spISR = spUUT->Open("dir1/file2"), NoSuchFileError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectoryContent_FileInSubDirOpen)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");
  auto spISW = spUUT->Create("dir1/dir2/file1", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/dir2/file2", true);

  // file2 is open for writing
  ASSERT_THROW(spUUT->DeleteDirectoryContent("dir1"), DirectoryAlreadyAccessedError);

  spISW->Close();

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/dir2/file1"));
  ASSERT_NO_THROW(spISR = spUUT->Open("dir1/dir2/file2"));

  // file2 is open for reading
  ASSERT_THROW(spUUT->DeleteDirectoryContent("dir1"), DirectoryAlreadyAccessedError);

  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/dir2/file1"));
  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/dir2/file2"));
  spISR->Close();

  // all files closed
  spUUT->DeleteDirectoryContent("dir1");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1/dir2"));
  EXPECT_THROW(spISR = spUUT->Open("dir1/dir2/file1"), NoSuchFileError);
  EXPECT_THROW(spISR = spUUT->Open("dir1/dir2/file2"), NoSuchFileError);
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  spUUT->DeleteDirectory("dir1/dir2");
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_FALSE(spUUT->IsDirectoryExisting("dir1/dir2"));

  spUUT->DeleteDirectory("dir1");
  ASSERT_FALSE(spUUT->IsDirectoryExisting("dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_NotEmpty_ContainsDir)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  ASSERT_THROW(spUUT->DeleteDirectory("dir1"), DirectoryNotEmptyError);

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_NotEmpty_ContainsFile)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("dir1/test_file", true);
  spISW->Close();

  ASSERT_THROW(spUUT->DeleteDirectory("dir1"), DirectoryNotEmptyError);

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));

  std::unique_ptr<gpcc::stream::IStreamReader> spISR;
  EXPECT_NO_THROW(spISR = spUUT->Open("dir1/test_file"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_DirNotExisting)
{
  ASSERT_THROW(spUUT->DeleteDirectory("not_existing_dir"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_ParentDirNotExisting)
{
  ASSERT_THROW(spUUT->DeleteDirectory("not_existing_dir1/not_existing_dir2"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_InvokeOnFile)
{
  auto spISW = spUUT->Create("testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->DeleteDirectory("testfile.txt"), NoSuchDirectoryError);

  auto spISR = spUUT->Open("testfile.txt");
  auto const s = spISR->Read_string();
  spISR->Close();

  EXPECT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_BaseDirRejected)
{
  ASSERT_THROW(spUUT->DeleteDirectory(""), InvalidFileNameError);
  ASSERT_TRUE(spUUT->IsDirectoryExisting(""));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_InvalidName1)
{
  // This checks that directory names violating the "basic rules" are not accepted

  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  ASSERT_THROW(spUUT->DeleteDirectory("dir1/dir2/"), InvalidFileNameError);
  ASSERT_THROW(spUUT->DeleteDirectory("dir1/../dir1/dir2/"), InvalidFileNameError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));

  ASSERT_THROW(spUUT->DeleteDirectory("dir1/"), InvalidFileNameError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1/dir2"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_InvalidName2)
{
  // This checks that directory names violating the rules for portable file/directory names
  // (but not the "basic rules") are accepted

  ASSERT_FALSE(spUUT->IsDirectoryExisting("-dir1"));

  // create the test folder
  CreateDir("-dir1");

  spUUT->DeleteDirectory("-dir1");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("-dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_DirIsSymlinkToDir)
{
  spUUT->CreateDirectory("original_dir");

  auto spISW = spUUT->Create("original_dir/file.txt", true);
  *spISW << std::string("test");
  spISW->Close();

  std::string const fullName1 = baseDir + "original_dir";
  std::string const fullName2 = baseDir + "link";
  int status = symlink(fullName1.c_str(), fullName2.c_str());
  if (status != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_FileStorage_TestsF.DeleteDirectory_DirIsSymlink: \"symlink\" failed on \"" + fullName1 + "\" and \"" + fullName2 + "\".");

  EXPECT_TRUE(spUUT->IsDirectoryExisting("link"));

  auto spISR = spUUT->Open("link/file.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "test");

  spUUT->DeleteDirectory("link");

  EXPECT_TRUE(spUUT->IsDirectoryExisting("original_dir"));
  spISR = spUUT->Open("original_dir/file.txt");
  s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "test");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("link"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, DeleteDirectory_DirIsSymlinkToFile)
{
  auto spISW = spUUT->Create("original_file.txt", true);
  *spISW << std::string("test");
  spISW->Close();

  std::string const fullName1 = baseDir + "original_file.txt";
  std::string const fullName2 = baseDir + "link.txt";
  int status = symlink(fullName1.c_str(), fullName2.c_str());
  if (status != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_FileStorage_TestsF.DeleteDirectory_DirIsSymlink: \"symlink\" failed on \"" + fullName1 + "\" and \"" + fullName2 + "\".");

  auto spISR = spUUT->Open("link.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "test");

  EXPECT_THROW(spUUT->DeleteDirectory("link.txt"), NoSuchDirectoryError);

  spISR = spUUT->Open("original_file.txt");
  s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "test");

  spISR = spUUT->Open("link.txt");
  s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "test");
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_OK)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("dir1/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  spUUT->RenameDirectory("dir1", "dir2");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2"));

  auto spISR = spUUT->Open("dir2/testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_Move)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir2");

  auto spISW = spUUT->Create("dir1/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  spUUT->RenameDirectory("dir1", "dir2/dir1");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2/dir1"));

  auto spISR = spUUT->Open("dir2/dir1/testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_MoveAndRename)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir2");

  auto spISW = spUUT->Create("dir1/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  spUUT->RenameDirectory("dir1", "dir2/dir3");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2/dir3"));

  auto spISR = spUUT->Open("dir2/dir3/testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_InvalidNewName)
{
  spUUT->CreateDirectory("dir1");
  ASSERT_THROW(spUUT->RenameDirectory("dir1", ""), InvalidFileNameError);
  ASSERT_THROW(spUUT->RenameDirectory("dir1", "/"), InvalidFileNameError);
  ASSERT_THROW(spUUT->RenameDirectory("dir1", "-dir2"), InvalidFileNameError);
  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir2/"), InvalidFileNameError);
  ASSERT_THROW(spUUT->RenameDirectory("dir1", "/dir2"), InvalidFileNameError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_InvalidCurrName)
{
  ASSERT_FALSE(spUUT->IsDirectoryExisting("-dir1"));

  // create the test folder
  CreateDir("-dir1");

  ASSERT_THROW(spUUT->RenameDirectory("/-dir1", "dir2"), InvalidFileNameError); // basic naming rules violated
  ASSERT_THROW(spUUT->RenameDirectory("-dir1/", "dir2"), InvalidFileNameError); // basic naming rules violated

  ASSERT_TRUE(spUUT->IsDirectoryExisting("-dir1"));
  ASSERT_FALSE(spUUT->IsDirectoryExisting("dir2"));

  spUUT->RenameDirectory("-dir1", "dir2");

  EXPECT_FALSE(spUUT->IsDirectoryExisting("-dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_NewNameEqualsCurrName)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("dir1/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  spUUT->RenameDirectory("dir1", "dir1");

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));

  auto spISR = spUUT->Open("dir1/testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_NewNameEqualsCurrName_DirNotExisting)
{
  ASSERT_THROW(spUUT->RenameDirectory("not_existing_dir", "not_existing_dir"), NoSuchDirectoryError);
  ASSERT_FALSE(spUUT->IsDirectoryExisting("not_existing_dir"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_NewNameEqualsCurrName_InvokeOnFile)
{
  auto spISW = spUUT->Create("testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->RenameDirectory("testfile.txt", "testfile.txt"), NoSuchDirectoryError);

  auto spISR = spUUT->Open("testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_DirNotExisting)
{
  ASSERT_THROW(spUUT->RenameDirectory("not_existing_dir", "dir1"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_InvokeOnFile)
{
  auto spISW = spUUT->Create("testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->RenameDirectory("testfile.txt", "dir1"), NoSuchDirectoryError);

  auto spISR = spUUT->Open("testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_FileWithNewNameAlreadyExisting)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "testfile.txt"), FileAlreadyExistingError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));

  auto spISR = spUUT->Open("testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_EmptyDirWithNewNameAlreadyExisting)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir2");

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir2"), DirectoryAlreadyExistingError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_NotEmptyDirWithNewNameAlreadyExisting)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir2");

  auto spISW = spUUT->Create("dir2/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir2"), DirectoryAlreadyExistingError);

  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir1"));
  EXPECT_TRUE(spUUT->IsDirectoryExisting("dir2"));

  auto spISR = spUUT->Open("dir2/testfile.txt");
  auto s = spISR->Read_string();
  spISR->Close();

  ASSERT_TRUE(s == "Test");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_BlockedByFileRead)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("dir1/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  auto spISR = spUUT->Open("dir1/testfile.txt");

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir2"), DirectoryAlreadyAccessedError);

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_BlockedByFileReadInSubDir)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  auto spISW = spUUT->Create("dir1/dir2/testfile.txt", true);
  *spISW << std::string("Test");
  spISW->Close();

  auto spISR = spUUT->Open("dir1/dir2/testfile.txt");

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir3"), DirectoryAlreadyAccessedError);

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_BlockedByFileWrite)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("dir1/testfile.txt", true);
  *spISW << std::string("Test");

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir2"), DirectoryAlreadyAccessedError);

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, RenameDir_BlockedByFileWriteInSubDir)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  auto spISW = spUUT->Create("dir1/dir2/testfile.txt", true);
  *spISW << std::string("Test");

  ASSERT_THROW(spUUT->RenameDirectory("dir1", "dir3"), DirectoryAlreadyAccessedError);

  ASSERT_TRUE(spUUT->IsDirectoryExisting("dir1"));
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_EmptyRoot)
{
  auto dirs = spUUT->EnumerateSubDirectories("");
  ASSERT_TRUE(dirs.empty());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_EmptyDir)
{
  spUUT->CreateDirectory("dir1");
  auto dirs = spUUT->EnumerateSubDirectories("dir1");
  ASSERT_TRUE(dirs.empty());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_OneDir)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  auto dirs = spUUT->EnumerateSubDirectories("");
  ASSERT_EQ(1U, dirs.size());
  EXPECT_TRUE(dirs.front() == "dir1");

  dirs = spUUT->EnumerateSubDirectories("dir1");
  ASSERT_EQ(1U, dirs.size());
  EXPECT_TRUE(dirs.front() == "dir2");

  dirs = spUUT->EnumerateSubDirectories("dir1/dir2");
  ASSERT_EQ(0U, dirs.size());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_OneDirPlusSomeFiles)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  auto spISW = spUUT->Create("dir1/file1.txt", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/file2.txt", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/dir2/file3.txt", true);
  spISW->Close();

  auto dirs = spUUT->EnumerateSubDirectories("");
  ASSERT_EQ(1U, dirs.size());
  EXPECT_TRUE(dirs.front() == "dir1");

  dirs = spUUT->EnumerateSubDirectories("dir1");
  ASSERT_EQ(1U, dirs.size());
  EXPECT_TRUE(dirs.front() == "dir2");

  dirs = spUUT->EnumerateSubDirectories("dir1/dir2");
  ASSERT_EQ(0U, dirs.size());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_OutputSorted)
{
  spUUT->CreateDirectory("G_dir");
  spUUT->CreateDirectory("Z_dir");
  spUUT->CreateDirectory("A_dir");

  auto dirs = spUUT->EnumerateSubDirectories("");
  ASSERT_EQ(3U, dirs.size());

  auto it = dirs.begin();
  EXPECT_TRUE(*it == "A_dir");
  ++it;
  EXPECT_TRUE(*it == "G_dir");
  ++it;
  EXPECT_TRUE(*it == "Z_dir");
  ++it;
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_DirNotExisting)
{
  std::list<std::string> dirs;
  EXPECT_THROW(dirs = spUUT->EnumerateSubDirectories("not_existing_dir"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_InvokedOnFile)
{
  auto spISW = spUUT->Create("file.txt", true);
  spISW->Close();

  std::list<std::string> dirs;
  EXPECT_THROW(dirs = spUUT->EnumerateSubDirectories("file.txt"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_InvalidName1)
{
  spUUT->CreateDirectory("dir1");
  spUUT->CreateDirectory("dir1/dir2");

  std::list<std::string> dirs;
  EXPECT_THROW(dirs = spUUT->EnumerateSubDirectories("dir1/"), InvalidFileNameError);
  EXPECT_THROW(dirs = spUUT->EnumerateSubDirectories("dir1/../dir1"), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumSubDirs_InvalidName2)
{
  ASSERT_FALSE(spUUT->IsDirectoryExisting("-dir1"));

  // create the test folder
  CreateDir("-dir1");

  spUUT->CreateDirectory("-dir1/dir2");

  auto dirs = spUUT->EnumerateSubDirectories("-dir1");
  ASSERT_EQ(1U, dirs.size());
  EXPECT_TRUE(dirs.front() == "dir2");
}

TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_EmptyRoot)
{
  auto files = spUUT->EnumerateFiles("");
  ASSERT_TRUE(files.empty());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_EmptyDir)
{
  spUUT->CreateDirectory("dir1");

  auto files = spUUT->EnumerateFiles("dir1");
  ASSERT_TRUE(files.empty());

  files = spUUT->EnumerateFiles("");
  ASSERT_TRUE(files.empty());
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_OneFile)
{
  auto spISW = spUUT->Create("file1", true);
  spISW->Close();

  spUUT->CreateDirectory("dir1");

  spISW = spUUT->Create("dir1/file2", true);
  spISW->Close();

  spUUT->CreateDirectory("dir1/dir2");

  spISW = spUUT->Create("dir1/dir2/file3", true);
  spISW->Close();

  auto files = spUUT->EnumerateFiles("");
  ASSERT_EQ(1U, files.size());
  EXPECT_TRUE(files.front() == "file1");

  files = spUUT->EnumerateFiles("dir1");
  ASSERT_EQ(1U, files.size());
  EXPECT_TRUE(files.front() == "file2");

  files = spUUT->EnumerateFiles("dir1/dir2");
  ASSERT_EQ(1U, files.size());
  EXPECT_TRUE(files.front() == "file3");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_Sorted)
{
  spUUT->CreateDirectory("dir1");

  auto spISW = spUUT->Create("dir1/G_file", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/Z_file", true);
  spISW->Close();

  spISW = spUUT->Create("dir1/A_file", true);
  spISW->Close();

  auto files = spUUT->EnumerateFiles("dir1");
  ASSERT_EQ(3U, files.size());

  auto it = files.begin();
  EXPECT_TRUE(*it == "A_file");
  ++it;
  EXPECT_TRUE(*it == "G_file");
  ++it;
  EXPECT_TRUE(*it == "Z_file");
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_DirectoryNotExisting)
{
  std::list<std::string> files;
  ASSERT_THROW(files = spUUT->EnumerateFiles("not_existing_dir"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_InvokedOnFile)
{
  auto spISW = spUUT->Create("file", true);
  spISW->Close();

  std::list<std::string> files;
  ASSERT_THROW(files = spUUT->EnumerateFiles("file"), NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_InvalidName1)
{
  spUUT->CreateDirectory("dir1");

  std::list<std::string> files;
  ASSERT_THROW(files = spUUT->EnumerateFiles("dir1/"), InvalidFileNameError);
  ASSERT_THROW(files = spUUT->EnumerateFiles("dir1/../dir1"), InvalidFileNameError);
}
TEST_F(gpcc_file_systems_linux_fs_FileStorage_TestsF, EnumFiles_InvalidName2)
{
  ASSERT_FALSE(spUUT->IsDirectoryExisting("-dir1"));

  // create the test folder
  CreateDir("-dir1");

  auto spISW = spUUT->Create("-dir1/file", true);
  spISW->Close();

  auto files = spUUT->EnumerateFiles("-dir1");
  ASSERT_EQ(1U, files.size());
  EXPECT_TRUE(files.front() == "file");
}

} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
