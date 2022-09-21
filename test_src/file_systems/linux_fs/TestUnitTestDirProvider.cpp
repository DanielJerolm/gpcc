/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#include "src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include <gpcc/file_systems/linux_fs/FileStorage.hpp>
#include "src/file_systems/linux_fs/internal/tools.hpp"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <memory>
#include <system_error>


using namespace testing;
using namespace gpcc::file_systems::linux_fs;
using namespace gpcc::file_systems::linux_fs::internal;

namespace gpcc_tests   {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

TEST(gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests, CreateAndDestroy)
{
  std::unique_ptr<UnitTestDirProvider> spUUT(new UnitTestDirProvider);

  std::string const path = spUUT->GetAbsPath();

  EXPECT_TRUE(CheckDirExists(path));

  spUUT.reset();

  EXPECT_FALSE(CheckDirExists(path));
}
TEST(gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests, DeleteContentEmpty)
{
  std::unique_ptr<UnitTestDirProvider> spUUT(new UnitTestDirProvider);

  std::string const path = spUUT->GetAbsPath();

  EXPECT_TRUE(CheckDirExists(path));

  spUUT->DeleteTestDirContent();

  EXPECT_TRUE(CheckDirExists(path));

  spUUT.reset();

  EXPECT_FALSE(CheckDirExists(path));
}
TEST(gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests, DeleteContent)
{
  UnitTestDirProvider uut;
  FileStorage fs(uut.GetAbsPath());

  fs.CreateDirectory("dir1");
  fs.CreateDirectory("dir1/dir2");

  auto spISW = fs.Create("file1", true);
  spISW->Close();

  spISW = fs.Create("dir1/file2", true);
  spISW->Close();

  spISW = fs.Create("dir1/dir2/file3", true);
  spISW->Close();

  uut.DeleteTestDirContent();

  auto files = fs.EnumerateFiles("");
  EXPECT_TRUE(files.empty());

  auto dirs = fs.EnumerateSubDirectories("");
  EXPECT_TRUE(dirs.empty());
}
TEST(gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests, DeleteContentUponDestruction)
{
  std::unique_ptr<UnitTestDirProvider> spUUT(new UnitTestDirProvider);
  std::string const path = spUUT->GetAbsPath();
  FileStorage fs(path);

  fs.CreateDirectory("dir1");
  fs.CreateDirectory("dir1/dir2");

  auto spISW = fs.Create("file1", true);
  spISW->Close();

  spISW = fs.Create("dir1/file2", true);
  spISW->Close();

  spISW = fs.Create("dir1/dir2/file3", true);
  spISW->Close();

  spUUT.reset();

  EXPECT_FALSE(CheckDirExists(path));
}
TEST(gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests, EmptyFolderAlreadyExisting)
{
  std::unique_ptr<UnitTestDirProvider> spUUT(new UnitTestDirProvider);
  std::string const path = spUUT->GetAbsPath();
  spUUT.reset();

  ASSERT_FALSE(CheckDirExists(path));

  if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests.EmptyFolderAlreadyExisting \"mkdir\" failed on \"" + path + "\".");

  spUUT.reset(new UnitTestDirProvider);
  EXPECT_TRUE(CheckDirExists(path));
  spUUT.reset();
  EXPECT_FALSE(CheckDirExists(path));
}
TEST(gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests, NotEmptyFolderAlreadyExisting)
{
  std::unique_ptr<UnitTestDirProvider> spUUT(new UnitTestDirProvider);
  std::string const path = spUUT->GetAbsPath();
  spUUT.reset();

  ASSERT_FALSE(CheckDirExists(path));

  if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_UnitTestDirProvider_Tests.EmptyFolderAlreadyExisting \"mkdir\" failed on \"" + path + "\".");

  // create some directories and fils
  {
    FileStorage fs(path);

    fs.CreateDirectory("dir1");
    fs.CreateDirectory("dir1/dir2");

    auto spISW = fs.Create("file1", true);
    spISW->Close();

    spISW = fs.Create("dir1/file2", true);
    spISW->Close();

    spISW = fs.Create("dir1/dir2/file3", true);
    spISW->Close();
  }

  // create a UnitTestDirProvider
  spUUT.reset(new UnitTestDirProvider);
  EXPECT_TRUE(CheckDirExists(path));

  // check that the folder is empty
  {
    FileStorage fs(path);

    auto files = fs.EnumerateFiles("");
    EXPECT_TRUE(files.empty());

    auto dirs = fs.EnumerateSubDirectories("");
    EXPECT_TRUE(dirs.empty());
  }

  spUUT.reset();
  EXPECT_FALSE(CheckDirExists(path));
}


} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
