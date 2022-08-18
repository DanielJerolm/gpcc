/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#include "gpcc/src/file_systems/linux_fs/internal/tools.hpp"
#include "gpcc/src/file_systems/linux_fs/internal/UnitTestDirProvider.hpp"
#include "gpcc/src/file_systems/exceptions.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gtest/gtest.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <system_error>
#include <cerrno>
#include <cstdio>
#include <cstring>

namespace gpcc_tests   {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

using namespace testing;
using namespace gpcc::file_systems::linux_fs::internal;

// Test fixture for methods in gpcc::file_systems::linux_fs::internal (tools.hpp/cpp).
// The test fixture creates a test folder referenced by "baseDir" and fills it with some
// dummy directories and files (see SetUp()). The unit test cases can do whatever they like
// inside the test directory. Finally the test folder and its contents will be removed again.
class gpcc_file_systems_linux_fs_tools_TestsF: public Test
{
  public:
    gpcc_file_systems_linux_fs_tools_TestsF(void);

  protected:
    // manages creation and removal of test directory
    UnitTestDirProvider testDirProvider;

    // path of the test folder, with trailing '/'
    std::string const baseDir;


    void SetUp(void) override;
    void TearDown(void) override;

    void CreateDir(std::string const & name);
    void CreateFile(std::string const & name);
};

gpcc_file_systems_linux_fs_tools_TestsF::gpcc_file_systems_linux_fs_tools_TestsF(void)
: Test()
, testDirProvider()
, baseDir(testDirProvider.GetAbsPath())
{
}

void gpcc_file_systems_linux_fs_tools_TestsF::SetUp(void)
{
  // This creates the following directories and files inside the test dir (referenced by "baseDir"):
  // file1.txt
  // file2.txt
  // Folder2/file2.txt
  // Folder2/file3.txt

  CreateDir("Folder2");

  CreateFile("file1.txt");
  CreateFile("file2.txt");
  CreateFile("Folder2/file2.txt");
  CreateFile("Folder2/file3.txt");
}

void gpcc_file_systems_linux_fs_tools_TestsF::TearDown(void)
{
}

void gpcc_file_systems_linux_fs_tools_TestsF::CreateDir(std::string const & name)
{
  // Creates a folder in the test directory.

  std::string const s = baseDir + name;
  if (mkdir(s.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_tools_TestsF.CreateDir:  mkdir failed on: " + s);
}

void gpcc_file_systems_linux_fs_tools_TestsF::CreateFile(std::string const & name)
{
  // Creates an empty file in the test directory.

  std::string const s = baseDir + name;

  FILE* fd = fopen(s.c_str(), "w");
  if (fd == nullptr)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_tools_TestsF::CreateFile: fopen() failed on: " + s);
  if (fclose(fd) != 0)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_tools_TestsF::CreateFile: fclose() failed on: " + s);
}


TEST(gpcc_file_systems_linux_fs_tools_Tests, RemoveTrailingForwardSlash)
{
  std::string s;

  s = "";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s.empty());

  s = "/";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s.empty());

  s = "//";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s.empty());

  s = "Test";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Test");

  s = "Abc/";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc");

  s = "Abc//";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc");

  s = "Abc/def";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc/def");

  s = "Abc/123/";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc/123");

  s = "Abc//123/";
  RemoveTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc//123");
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, EnsureTrailingForwardSlash)
{
  std::string s;

  s = "";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "");

  s = "/";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "/");

  s = "//";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "//");

  s = "A";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "A/");

  s = "Test";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Test/");

  s = "Abc/";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc/");

  s = "Abc//";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc//");

  s = "Abc///";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc///");

  s = "Abc/def";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc/def/");

  s = "Abc//def";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc//def/");

  s = "Abc/123/";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc/123/");

  s = "Abc//123/";
  EnsureTrailingForwardSlash(s);
  EXPECT_TRUE(s == "Abc//123/");
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, DeleteDirectoryContent_NoFwdSlash)
{
  std::string dir = baseDir;
  RemoveTrailingForwardSlash(dir);

  // <-- UUT
  DeleteDirectoryContent(dir);
  // -->

  // check: TEST_FOLDER should be empty now
  DIR* const pDIR = opendir(baseDir.c_str());
  if (pDIR == nullptr)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_tools_TestsF.DeleteDirectoryContent: \"opendir\" failed");
  ON_SCOPE_EXIT(close_pDIR) { closedir(pDIR); };

  struct dirent * pEntry;
  errno = 0;
  while ((pEntry = readdir(pDIR)) != nullptr)
  {
    EXPECT_TRUE((strcmp(pEntry->d_name, ".") == 0) || (strcmp(pEntry->d_name, "..") == 0));
    errno = 0;
  }
  EXPECT_TRUE(errno == 0);

  // (scope-guard will close pDIR)
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, DeleteDirectoryContent_WithFwdSlash)
{
  // <-- UUT
  DeleteDirectoryContent(baseDir);
  // -->

  // check: TEST_FOLDER should be empty now
  DIR* const pDIR = opendir(baseDir.c_str());
  if (pDIR == nullptr)
    throw std::system_error(errno, std::generic_category(), "gpcc_file_systems_linux_fs_tools_TestsF.DeleteDirectoryContent_WithFwdSlash: \"opendir\" failed");
  ON_SCOPE_EXIT(close_pDIR) { closedir(pDIR); };

  struct dirent * pEntry;
  errno = 0;
  while ((pEntry = readdir(pDIR)) != nullptr)
  {
    EXPECT_TRUE((strcmp(pEntry->d_name, ".") == 0) || (strcmp(pEntry->d_name, "..") == 0));
    errno = 0;
  }
  EXPECT_TRUE(errno == 0);

  // (scope-guard will close pDIR)
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, DeleteDirectoryContent_FileName)
{
  EXPECT_THROW(DeleteDirectoryContent(baseDir + "file1.txt"), gpcc::file_systems::NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, DeleteDirectoryContent_DirNotExisting)
{
  EXPECT_THROW(DeleteDirectoryContent(baseDir + "NotExistingDir/"), gpcc::file_systems::NoSuchDirectoryError);
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, EnumerateFiles_Recursive_NoFwdSlash)
{
  std::string dir = baseDir;
  RemoveTrailingForwardSlash(dir);

  std::list<std::string> entries;
  EnumerateFiles(dir, entries, true);

  entries.sort();

  ASSERT_EQ(4U, entries.size());

  auto it = entries.begin();
  EXPECT_TRUE((*it) == "Folder2/file2.txt");
  ++it;
  EXPECT_TRUE((*it) == "Folder2/file3.txt");
  ++it;
  EXPECT_TRUE((*it) == "file1.txt");
  ++it;
  EXPECT_TRUE((*it) == "file2.txt");
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, EnumerateFiles_Recursive_WithFwdSlash)
{
  std::list<std::string> entries;
  EnumerateFiles(baseDir, entries, true);

  entries.sort();

  ASSERT_EQ(4U, entries.size());

  auto it = entries.begin();
  EXPECT_TRUE((*it) == "Folder2/file2.txt");
  ++it;
  EXPECT_TRUE((*it) == "Folder2/file3.txt");
  ++it;
  EXPECT_TRUE((*it) == "file1.txt");
  ++it;
  EXPECT_TRUE((*it) == "file2.txt");
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, EnumerateFiles_NonRecursive_NoFwdSlash)
{
  std::string dir = baseDir;
  RemoveTrailingForwardSlash(dir);

  std::list<std::string> entries;
  EnumerateFiles(dir, entries, false);

  entries.sort();

  ASSERT_EQ(2U, entries.size());

  auto it = entries.begin();
  EXPECT_TRUE((*it) == "file1.txt");
  ++it;
  EXPECT_TRUE((*it) == "file2.txt");
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, EnumerateFiles_NonRecursive_WithFwdSlash)
{
  std::list<std::string> entries;
  EnumerateFiles(baseDir, entries, false);

  entries.sort();

  ASSERT_EQ(2U, entries.size());

  auto it = entries.begin();
  EXPECT_TRUE((*it) == "file1.txt");
  ++it;
  EXPECT_TRUE((*it) == "file2.txt");
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, EnumerateFiles_FileName)
{
  std::list<std::string> entries;
  EXPECT_THROW(EnumerateFiles(baseDir + "file1.txt", entries, false), gpcc::file_systems::NoSuchDirectoryError);

  EXPECT_TRUE(entries.empty());
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, EnumerateFiles_DirNotExisting)
{
  std::list<std::string> entries;
  EXPECT_THROW(EnumerateFiles(baseDir + "NotExistingDir/", entries, false), gpcc::file_systems::NoSuchDirectoryError);

  EXPECT_TRUE(entries.empty());
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, CheckFileOrDirExists)
{
  std::string dir = baseDir;
  RemoveTrailingForwardSlash(dir);

  EXPECT_TRUE(CheckFileOrDirExists(dir));
  EXPECT_TRUE(CheckFileOrDirExists(baseDir));
  EXPECT_TRUE(CheckFileOrDirExists(baseDir + "file1.txt"));
  EXPECT_FALSE(CheckFileOrDirExists(baseDir + "fileX.txt"));
  EXPECT_TRUE(CheckFileOrDirExists(baseDir + "Folder2"));
  EXPECT_TRUE(CheckFileOrDirExists(baseDir + "Folder2/"));
  EXPECT_FALSE(CheckFileOrDirExists(baseDir + "FolderX"));
  EXPECT_FALSE(CheckFileOrDirExists(baseDir + "FolderX/"));
  EXPECT_TRUE(CheckFileOrDirExists(baseDir + "Folder2/file3.txt"));
  EXPECT_FALSE(CheckFileOrDirExists(baseDir + "Folder2/file3.txt/"));
  EXPECT_FALSE(CheckFileOrDirExists(baseDir + "Folder2/file4.txt"));
  EXPECT_FALSE(CheckFileOrDirExists(baseDir + "Folder2/file4.txt/"));
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, CheckFileExists)
{
  std::string dir = baseDir;
  RemoveTrailingForwardSlash(dir);

  EXPECT_FALSE(CheckFileExists(dir));
  EXPECT_FALSE(CheckFileExists(baseDir));
  EXPECT_FALSE(CheckFileExists(baseDir + "Folder2"));
  EXPECT_FALSE(CheckFileExists(baseDir + "Folder2/"));

  EXPECT_FALSE(CheckFileExists(baseDir + "Folder3"));
  EXPECT_FALSE(CheckFileExists(baseDir + "Folder3/"));

  EXPECT_TRUE(CheckFileExists(baseDir + "file1.txt"));
  EXPECT_FALSE(CheckFileExists(baseDir + "file1.txt/"));
  EXPECT_TRUE(CheckFileExists(baseDir + "Folder2/file2.txt"));
  EXPECT_FALSE(CheckFileExists(baseDir + "Folder2/file2.txt/"));
  EXPECT_FALSE(CheckFileExists(baseDir + "Folder2/file4.txt"));
  EXPECT_FALSE(CheckFileExists(baseDir + "Folder2/file4.txt/"));
}
TEST_F(gpcc_file_systems_linux_fs_tools_TestsF, CheckDirExists)
{
  std::string dir = baseDir;
  RemoveTrailingForwardSlash(dir);

  EXPECT_TRUE(CheckDirExists(dir));
  EXPECT_TRUE(CheckDirExists(baseDir));
  EXPECT_TRUE(CheckDirExists(baseDir + "Folder2"));
  EXPECT_TRUE(CheckDirExists(baseDir + "Folder2/"));

  EXPECT_FALSE(CheckDirExists(baseDir + "Folder3"));
  EXPECT_FALSE(CheckDirExists(baseDir + "Folder3/"));

  EXPECT_FALSE(CheckDirExists(baseDir + "file1.txt"));
  EXPECT_FALSE(CheckDirExists(baseDir + "file1.txt/"));
  EXPECT_FALSE(CheckDirExists(baseDir + "Folder2/file2.txt"));
  EXPECT_FALSE(CheckDirExists(baseDir + "Folder2/file2.txt/"));
  EXPECT_FALSE(CheckDirExists(baseDir + "Folder2/file4.txt"));
  EXPECT_FALSE(CheckDirExists(baseDir + "Folder2/file4.txt/"));
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, CheckFileName_NoPath)
{
  // bool CheckFileName(std::string const & name, bool const acceptPath, bool const checkFileOnly)

  EXPECT_TRUE(CheckFileName("Test.txt", false, false));
  EXPECT_TRUE(CheckFileName(".Test.txt", false, false));
  EXPECT_TRUE(CheckFileName("..Test.txt", false, false));
  EXPECT_TRUE(CheckFileName("...Test.txt", false, false));
  EXPECT_TRUE(CheckFileName("Test", false, false));
  EXPECT_TRUE(CheckFileName("Test-File.txt", false, false));
  EXPECT_TRUE(CheckFileName("Test_File.txt", false, false));
  EXPECT_TRUE(CheckFileName("_Test_File.txt", false, false));
  EXPECT_TRUE(CheckFileName("Test_File_.txt", false, false));
  EXPECT_TRUE(CheckFileName("A", false, false));
  EXPECT_TRUE(CheckFileName("Test File 123.txt", false, false));
  EXPECT_TRUE(CheckFileName("Test.File.123.txt", false, false));

  EXPECT_FALSE(CheckFileName("", false, false));               // empty string
  EXPECT_FALSE(CheckFileName("/", false, false));              // path
  EXPECT_FALSE(CheckFileName("Test/Test.txt", false, false));  // path
  EXPECT_FALSE(CheckFileName("/Test/Test.txt", false, false)); // path + leading '/'
  EXPECT_FALSE(CheckFileName("Test/Test.txt/", false, false)); // path + trailing '/'
  EXPECT_FALSE(CheckFileName("Test//Test.txt", false, false)); // path + double '//'
  EXPECT_FALSE(CheckFileName(" Test", false, false));          // leading ' '
  EXPECT_FALSE(CheckFileName("Test ", false, false));          // trailing ' '
  EXPECT_FALSE(CheckFileName("Test  A", false, false));        // double ' '
  EXPECT_FALSE(CheckFileName("Test.", false, false));          // trailing '.'
  EXPECT_FALSE(CheckFileName("-Test", false, false));          // leading '-'
  EXPECT_FALSE(CheckFileName("/Test", false, false));          // leading '/'
  EXPECT_FALSE(CheckFileName("Test/", false, false));          // trailing '/'

  EXPECT_FALSE(CheckFileName(".", false, false));              // filename must not be '.'
  EXPECT_FALSE(CheckFileName("..", false, false));             // filename must not be '..'

  // some non-recommended characters
  EXPECT_FALSE(CheckFileName("Test (T).txt", false, false));
  EXPECT_FALSE(CheckFileName("Test {T}.txt", false, false));
  EXPECT_FALSE(CheckFileName("Test [T].txt", false, false));
  EXPECT_FALSE(CheckFileName("Test*T.txt", false, false));
  EXPECT_FALSE(CheckFileName("Test~T.txt", false, false));
  EXPECT_FALSE(CheckFileName("Test@T.txt", false, false));
  EXPECT_FALSE(CheckFileName("Test!", false, false));
  EXPECT_FALSE(CheckFileName("Test 100%", false, false));
  EXPECT_FALSE(CheckFileName("Test^100", false, false));
  EXPECT_FALSE(CheckFileName("Test|ABC", false, false));
  EXPECT_FALSE(CheckFileName("TestWithö", false, false));
  EXPECT_FALSE(CheckFileName("TestWithü", false, false));
  EXPECT_FALSE(CheckFileName("TestWithä", false, false));
  EXPECT_FALSE(CheckFileName("TestWithÖ", false, false));
  EXPECT_FALSE(CheckFileName("TestWithÜ", false, false));
  EXPECT_FALSE(CheckFileName("TestWithÄ", false, false));
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, CheckFileName_InclPath_FullCheck)
{
  // bool CheckFileName(std::string const & name, bool const acceptPath, bool const checkFileOnly)

  EXPECT_TRUE(CheckFileName("Test.txt", true, false));
  EXPECT_TRUE(CheckFileName(".Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("..Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("...Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test", true, false));
  EXPECT_TRUE(CheckFileName("Test-File.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test_File.txt", true, false));
  EXPECT_TRUE(CheckFileName("_Test_File.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test_File_.txt", true, false));
  EXPECT_TRUE(CheckFileName("A", true, false));
  EXPECT_TRUE(CheckFileName("Test File 123.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test.File.123.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test/Test.txt", true, false));
  EXPECT_TRUE(CheckFileName(".Test/Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test/.Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test/..Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test/...Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test_/.Test.txt", true, false));
  EXPECT_TRUE(CheckFileName("Test.A.B/.Test.txt", true, false));

  EXPECT_FALSE(CheckFileName("/", true, false));              // leading/trailing '/'
  EXPECT_FALSE(CheckFileName("Test/", true, false));          // trailing '/'
  EXPECT_FALSE(CheckFileName("Test.txt/", true, false));      // trailing '/'
  EXPECT_FALSE(CheckFileName("", true, false));               // empty string
  EXPECT_FALSE(CheckFileName(" Test", true, false));          // leading ' '
  EXPECT_FALSE(CheckFileName("/ Test", true, false));         // leading '/' / ' '
  EXPECT_FALSE(CheckFileName("/Test", true, false));          // leading '/'
  EXPECT_FALSE(CheckFileName("/Test/A", true, false));        // leading '/'
  EXPECT_FALSE(CheckFileName("/Test/.Test.txt", true, false));// leading '/'
  EXPECT_FALSE(CheckFileName("A/ Test", true, false));        // leading ' '
  EXPECT_FALSE(CheckFileName(" Test/A", true, false));        // leading ' ' in path
  EXPECT_FALSE(CheckFileName("Test ", true, false));          // trailing ' '
  EXPECT_FALSE(CheckFileName("/Test ", true, false));         // trailing ' '
  EXPECT_FALSE(CheckFileName("Test /", true, false));         // trailing ' ' in path
  EXPECT_FALSE(CheckFileName("B/Test ", true, false));        // trailing ' '
  EXPECT_FALSE(CheckFileName("Test /B", true, false));        // trailing ' ' in path
  EXPECT_FALSE(CheckFileName("Test  A", true, false));        // double ' '
  EXPECT_FALSE(CheckFileName("A/Test  A", true, false));      // double ' '
  EXPECT_FALSE(CheckFileName("Test  A/A", true, false));      // double ' ' in path
  EXPECT_FALSE(CheckFileName("Test.", true, false));          // trailing '.'
  EXPECT_FALSE(CheckFileName("Test./A", true, false));        // trailing '.' in path
  EXPECT_FALSE(CheckFileName("A/Test.", true, false));        // trailing '.'
  EXPECT_FALSE(CheckFileName("-Test", true, false));          // leading '-'
  EXPECT_FALSE(CheckFileName("A/-Test", true, false));        // leading '-'
  EXPECT_FALSE(CheckFileName("-Test/A", true, false));        // leading '-' in path
  EXPECT_FALSE(CheckFileName("Test/../A/B/Test.txt", true, false)); // ".." in path
  EXPECT_FALSE(CheckFileName("Test/A/./B/Test.txt", true, false));  // "." in path

  EXPECT_FALSE(CheckFileName("A/.", true, false));            // filename must not be '.'
  EXPECT_FALSE(CheckFileName("A/..", true, false));           // filename must not be '..'

  EXPECT_FALSE(CheckFileName(".Test//Test.txt", true, false));  // double "/"
  EXPECT_FALSE(CheckFileName(".Test///Test.txt", true, false)); // double "/"
  EXPECT_FALSE(CheckFileName("//Test/.Test.txt", true, false)); // double "/"

  // some non-recommended characters in filename without path
  EXPECT_FALSE(CheckFileName("Test (T).txt", true, false));
  EXPECT_FALSE(CheckFileName("Test {T}.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test [T].txt", true, false));
  EXPECT_FALSE(CheckFileName("Test*T.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test~T.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test@T.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test!", true, false));
  EXPECT_FALSE(CheckFileName("Test 100%", true, false));
  EXPECT_FALSE(CheckFileName("Test^100", true, false));
  EXPECT_FALSE(CheckFileName("Test|ABC", true, false));
  EXPECT_FALSE(CheckFileName("TestWithö", true, false));
  EXPECT_FALSE(CheckFileName("TestWithü", true, false));
  EXPECT_FALSE(CheckFileName("TestWithä", true, false));
  EXPECT_FALSE(CheckFileName("TestWithÖ", true, false));
  EXPECT_FALSE(CheckFileName("TestWithÜ", true, false));
  EXPECT_FALSE(CheckFileName("TestWithÄ", true, false));

  // some non-recommended characters in filename with path
  EXPECT_FALSE(CheckFileName("A/Test (T).txt", true, false));
  EXPECT_FALSE(CheckFileName("A/Test {T}.txt", true, false));
  EXPECT_FALSE(CheckFileName("A/Test [T].txt", true, false));
  EXPECT_FALSE(CheckFileName("A/Test*T.txt", true, false));
  EXPECT_FALSE(CheckFileName("A/Test~T.txt", true, false));
  EXPECT_FALSE(CheckFileName("A/Test@T.txt", true, false));
  EXPECT_FALSE(CheckFileName("A/Test!", true, false));
  EXPECT_FALSE(CheckFileName("A/Test 100%", true, false));
  EXPECT_FALSE(CheckFileName("A/Test^100", true, false));
  EXPECT_FALSE(CheckFileName("A/Test|ABC", true, false));
  EXPECT_FALSE(CheckFileName("A/TestWithö", true, false));
  EXPECT_FALSE(CheckFileName("A/TestWithü", true, false));
  EXPECT_FALSE(CheckFileName("A/TestWithä", true, false));
  EXPECT_FALSE(CheckFileName("A/TestWithÖ", true, false));
  EXPECT_FALSE(CheckFileName("A/TestWithÜ", true, false));
  EXPECT_FALSE(CheckFileName("A/TestWithÄ", true, false));

  // some non-recommended characters in path-name
  EXPECT_FALSE(CheckFileName("Test (T)/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test {T}/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test [T]/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test*T/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test~T/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test@T/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test!/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test 100%/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test^100/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("Test|ABC/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("TestWithö/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("TestWithü/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("TestWithä/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("TestWithÖ/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("TestWithÜ/A.txt", true, false));
  EXPECT_FALSE(CheckFileName("TestWithÄ/A.txt", true, false));
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, CheckFileName_InclPath_CheckFileNameOnly)
{
  // bool CheckFileName(std::string const & name, bool const acceptPath, bool const checkFileOnly)

  EXPECT_TRUE(CheckFileName("Test.txt", true, true));
  EXPECT_TRUE(CheckFileName(".Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("..Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("...Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test", true, true));
  EXPECT_TRUE(CheckFileName("Test-File.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test_File.txt", true, true));
  EXPECT_TRUE(CheckFileName("_Test_File.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test_File_.txt", true, true));
  EXPECT_TRUE(CheckFileName("A", true, true));
  EXPECT_TRUE(CheckFileName("Test File 123.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test.File.123.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test/Test.txt", true, true));
  EXPECT_TRUE(CheckFileName(".Test/Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test/.Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("_Test/.Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test_/.Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("_Test/.Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("_Test/..Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("_Test/...Test.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test.A.B/.Test.txt", true, true));

  EXPECT_FALSE(CheckFileName("/", true, true));              // leading/trailing '/'
  EXPECT_FALSE(CheckFileName("Test/", true, true));          // trailing '/'
  EXPECT_FALSE(CheckFileName("Test.txt/", true, true));      // trailing '/'
  EXPECT_FALSE(CheckFileName("", true, true));               // empty string
  EXPECT_FALSE(CheckFileName(" Test", true, true));          // leading ' '
  EXPECT_FALSE(CheckFileName("/ Test", true, true));         // leading '/' / ' '
  EXPECT_FALSE(CheckFileName("/Test", true, true));          // leading '/'
  EXPECT_FALSE(CheckFileName("/Test/A", true, true));        // leading '/'
  EXPECT_FALSE(CheckFileName("A/ Test", true, true));        // leading ' '
  EXPECT_TRUE (CheckFileName(" Test/A", true, true));        // leading ' '    <- non-portable directory name accepted
  EXPECT_FALSE(CheckFileName("Test ", true, true));          // trailing ' '
  EXPECT_FALSE(CheckFileName("/Test ", true, true));         // leading '/' + trailing ' '
  EXPECT_FALSE(CheckFileName("Test /", true, true));         // trailing ' ' / trailing '/'
  EXPECT_FALSE(CheckFileName("B/Test ", true, true));        // trailing ' '
  EXPECT_TRUE (CheckFileName("Test /B", true, true));        // trailing ' '   <- non-portable directory name accepted
  EXPECT_FALSE(CheckFileName("Test  A", true, true));        // double ' '
  EXPECT_FALSE(CheckFileName("A/Test  A", true, true));      // double ' '
  EXPECT_TRUE (CheckFileName("Test  A/A", true, true));      // double ' '     <- non-portable directory name accepted
  EXPECT_FALSE(CheckFileName("Test.", true, true));          // trailing '.'
  EXPECT_TRUE (CheckFileName("Test./A", true, true));        // trailing '.'   <- non-portable directory name accepted
  EXPECT_FALSE(CheckFileName("A/Test.", true, true));        // trailing '.'
  EXPECT_FALSE(CheckFileName("-Test", true, true));          // leading '-'
  EXPECT_FALSE(CheckFileName("A/-Test", true, true));        // leading '-'
  EXPECT_TRUE (CheckFileName("-Test/A", true, true));        // leading '-'    <- non-portable directory name accepted
  EXPECT_FALSE(CheckFileName("Test/../A/B/Test.txt", true, true)); // ".." in path
  EXPECT_FALSE(CheckFileName("Test/A/./B/Test.txt", true, true));  // "." in path

  EXPECT_FALSE(CheckFileName("A/.", true, true));            // filename must not be '.'
  EXPECT_FALSE(CheckFileName("A/..", true, true));           // filename must not be '..'

  EXPECT_FALSE(CheckFileName(".Test//Test.txt", true, true));  // double "/"
  EXPECT_FALSE(CheckFileName(".Test///Test.txt", true, true)); // double "/"
  EXPECT_FALSE(CheckFileName("//Test/.Test.txt", true, true)); // double "/"

  // some non-recommended characters in filename without path
  EXPECT_FALSE(CheckFileName("Test (T).txt", true, true));
  EXPECT_FALSE(CheckFileName("Test {T}.txt", true, true));
  EXPECT_FALSE(CheckFileName("Test [T].txt", true, true));
  EXPECT_FALSE(CheckFileName("Test*T.txt", true, true));
  EXPECT_FALSE(CheckFileName("Test~T.txt", true, true));
  EXPECT_FALSE(CheckFileName("Test@T.txt", true, true));
  EXPECT_FALSE(CheckFileName("Test!", true, true));
  EXPECT_FALSE(CheckFileName("Test 100%", true, true));
  EXPECT_FALSE(CheckFileName("Test^100", true, true));
  EXPECT_FALSE(CheckFileName("Test|ABC", true, true));
  EXPECT_FALSE(CheckFileName("TestWithö", true, true));
  EXPECT_FALSE(CheckFileName("TestWithü", true, true));
  EXPECT_FALSE(CheckFileName("TestWithä", true, true));
  EXPECT_FALSE(CheckFileName("TestWithÖ", true, true));
  EXPECT_FALSE(CheckFileName("TestWithÜ", true, true));
  EXPECT_FALSE(CheckFileName("TestWithÄ", true, true));

  // some non-recommended characters in filename with path
  EXPECT_FALSE(CheckFileName("A/Test (T).txt", true, true));
  EXPECT_FALSE(CheckFileName("A/Test {T}.txt", true, true));
  EXPECT_FALSE(CheckFileName("A/Test [T].txt", true, true));
  EXPECT_FALSE(CheckFileName("A/Test*T.txt", true, true));
  EXPECT_FALSE(CheckFileName("A/Test~T.txt", true, true));
  EXPECT_FALSE(CheckFileName("A/Test@T.txt", true, true));
  EXPECT_FALSE(CheckFileName("A/Test!", true, true));
  EXPECT_FALSE(CheckFileName("A/Test 100%", true, true));
  EXPECT_FALSE(CheckFileName("A/Test^100", true, true));
  EXPECT_FALSE(CheckFileName("A/Test|ABC", true, true));
  EXPECT_FALSE(CheckFileName("A/TestWithö", true, true));
  EXPECT_FALSE(CheckFileName("A/TestWithü", true, true));
  EXPECT_FALSE(CheckFileName("A/TestWithä", true, true));
  EXPECT_FALSE(CheckFileName("A/TestWithÖ", true, true));
  EXPECT_FALSE(CheckFileName("A/TestWithÜ", true, true));
  EXPECT_FALSE(CheckFileName("A/TestWithÄ", true, true));

  // some non-recommended characters in path-name
  EXPECT_TRUE(CheckFileName("Test (T)/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test {T}/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test [T]/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test*T/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test~T/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test@T/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test!/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test 100%/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test^100/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("Test|ABC/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("TestWithö/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("TestWithü/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("TestWithä/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("TestWithÖ/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("TestWithÜ/A.txt", true, true));
  EXPECT_TRUE(CheckFileName("TestWithÄ/A.txt", true, true));
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, CheckDirectoryName_FullCheck)
{
  // bool CheckDirectoryName(std::string const & name, bool const checkDirectoryOnly)

  EXPECT_TRUE(CheckDirectoryName("Test", false));
  EXPECT_TRUE(CheckDirectoryName(".Test", false));
  EXPECT_TRUE(CheckDirectoryName("..Test", false));
  EXPECT_TRUE(CheckDirectoryName("...Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test-Dir", false));
  EXPECT_TRUE(CheckDirectoryName("Test_Dir", false));
  EXPECT_TRUE(CheckDirectoryName("_Test_Dir", false));
  EXPECT_TRUE(CheckDirectoryName("Test_Dir.Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test_Dir.Test.test", false));
  EXPECT_TRUE(CheckDirectoryName("A", false));
  EXPECT_TRUE(CheckDirectoryName("Test Dir 123", false));
  EXPECT_TRUE(CheckDirectoryName("Test.Dir.123", false));
  EXPECT_TRUE(CheckDirectoryName("Test/Test", false));
  EXPECT_TRUE(CheckDirectoryName(".Test/Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test/.Test", false));
  EXPECT_TRUE(CheckDirectoryName("_Test/.Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test_/.Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test/.Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test/..Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test/...Test", false));
  EXPECT_TRUE(CheckDirectoryName("Test_/.Test.Abc", false));
  EXPECT_TRUE(CheckDirectoryName("Test.A.B/.Test", false));

  EXPECT_FALSE(CheckDirectoryName("", false));               // empty string
  EXPECT_FALSE(CheckDirectoryName("/", false));              // '/' only
  EXPECT_FALSE(CheckDirectoryName("Test/", false));          // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("Test//", false));         // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("dir/Test/", false));      // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("dir/Test//", false));     // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("/dir/Test", false));      // leading '/'
  EXPECT_FALSE(CheckDirectoryName("//dir/Test", false));     // leading '/'
  EXPECT_FALSE(CheckDirectoryName(" Test", false));          // leading ' '
  EXPECT_FALSE(CheckDirectoryName("/ Test", false));         // leading ' '
  EXPECT_FALSE(CheckDirectoryName("A/ Test", false));        // leading ' '
  EXPECT_FALSE(CheckDirectoryName(" Test/A", false));        // leading ' ' in path
  EXPECT_FALSE(CheckDirectoryName("Test ", false));          // trailing ' '
  EXPECT_FALSE(CheckDirectoryName("/Test ", false));         // leading '/' + trailing ' '
  EXPECT_FALSE(CheckDirectoryName("/Test", false));          // leading '/'
  EXPECT_FALSE(CheckDirectoryName("B/Test ", false));        // trailing ' '
  EXPECT_FALSE(CheckDirectoryName("Test /B", false));        // trailing ' ' in path
  EXPECT_FALSE(CheckDirectoryName("Test  A", false));        // double ' '
  EXPECT_FALSE(CheckDirectoryName("A/Test  A", false));      // double ' '
  EXPECT_FALSE(CheckDirectoryName("Test  A/A", false));      // double ' ' in path
  EXPECT_FALSE(CheckDirectoryName("Test.", false));          // trailing '.'
  EXPECT_FALSE(CheckDirectoryName("Test./A", false));        // trailing '.' in path
  EXPECT_FALSE(CheckDirectoryName("A/Test.", false));        // trailing '.'
  EXPECT_FALSE(CheckDirectoryName("-Test", false));          // leading '-'
  EXPECT_FALSE(CheckDirectoryName("A/-Test", false));        // leading '-'
  EXPECT_FALSE(CheckDirectoryName("-Test/A", false));        // leading '-' in path
  EXPECT_FALSE(CheckDirectoryName("Test/../A/B/Test", false)); // ".." in path
  EXPECT_FALSE(CheckDirectoryName("Test/A/./B/Test", false));  // "." in path

  EXPECT_FALSE(CheckDirectoryName("A/.", false));            // directory name must not be '.'
  EXPECT_FALSE(CheckDirectoryName("A/..", false));           // directory name must not be '..'
  EXPECT_FALSE(CheckDirectoryName("A/./", false));           // directory name must not be '.' / trailing '/'
  EXPECT_FALSE(CheckDirectoryName("A/../", false));          // directory name must not be '.. / trailing '/''

  EXPECT_FALSE(CheckDirectoryName(".Test//Test", false));      // double "/"
  EXPECT_FALSE(CheckDirectoryName(".Test///Test", false));     // double "/"
  EXPECT_FALSE(CheckDirectoryName("//Test/.Test.Abc", false)); // double "/"

  // some non-recommended characters in directory name without path
  EXPECT_FALSE(CheckDirectoryName("Test (T)", false));
  EXPECT_FALSE(CheckDirectoryName("Test {T}", false));
  EXPECT_FALSE(CheckDirectoryName("Test [T]", false));
  EXPECT_FALSE(CheckDirectoryName("Test*T", false));
  EXPECT_FALSE(CheckDirectoryName("Test~T", false));
  EXPECT_FALSE(CheckDirectoryName("Test@T", false));
  EXPECT_FALSE(CheckDirectoryName("Test!", false));
  EXPECT_FALSE(CheckDirectoryName("Test 100%", false));
  EXPECT_FALSE(CheckDirectoryName("Test^100", false));
  EXPECT_FALSE(CheckDirectoryName("Test|ABC", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithö", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithü", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithä", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithÖ", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithÜ", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithÄ", false));

  // some non-recommended characters in directory name with path
  EXPECT_FALSE(CheckDirectoryName("A/Test (T)", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test {T}", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test [T]", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test*T", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test~T", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test@T", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test!", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test 100%", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test^100", false));
  EXPECT_FALSE(CheckDirectoryName("A/Test|ABC", false));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithö", false));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithü", false));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithä", false));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithÖ", false));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithÜ", false));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithÄ", false));

  // some non-recommended characters in path-name
  EXPECT_FALSE(CheckDirectoryName("Test (T)/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test {T}/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test [T]/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test*T/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test~T/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test@T/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test!/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test 100%/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test^100/A", false));
  EXPECT_FALSE(CheckDirectoryName("Test|ABC/A", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithö/A", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithü/A", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithä/A", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithÖ/A", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithÜ/A", false));
  EXPECT_FALSE(CheckDirectoryName("TestWithÄ/A", false));
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, CheckDirectoryName_CheckDirectoryNameOnly)
{
  // bool CheckDirectoryName(std::string const & name, bool const checkDirectoryOnly)

  EXPECT_TRUE(CheckDirectoryName("Test", true));
  EXPECT_TRUE(CheckDirectoryName(".Test", true));
  EXPECT_TRUE(CheckDirectoryName("..Test", true));
  EXPECT_TRUE(CheckDirectoryName("...Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test-Dir", true));
  EXPECT_TRUE(CheckDirectoryName("Test_Dir", true));
  EXPECT_TRUE(CheckDirectoryName("_Test_Dir", true));
  EXPECT_TRUE(CheckDirectoryName("Test_Dir_", true));
  EXPECT_TRUE(CheckDirectoryName("A", true));
  EXPECT_TRUE(CheckDirectoryName("Test Dir 123", true));
  EXPECT_TRUE(CheckDirectoryName("Test.Dir.123", true));
  EXPECT_TRUE(CheckDirectoryName("Test/Test", true));
  EXPECT_TRUE(CheckDirectoryName(".Test/Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test/.Test", true));
  EXPECT_TRUE(CheckDirectoryName("_Test/.Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test_/.Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test/.Test", true));
  EXPECT_TRUE(CheckDirectoryName("_Test/.Test", true));
  EXPECT_TRUE(CheckDirectoryName("_Test/..Test", true));
  EXPECT_TRUE(CheckDirectoryName("_Test/...Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test_/.Test", true));
  EXPECT_TRUE(CheckDirectoryName("Test.A.B/.Test", true));

  EXPECT_FALSE(CheckDirectoryName("", true));               // empty string
  EXPECT_FALSE(CheckDirectoryName("/", true));              // '/' only
  EXPECT_FALSE(CheckDirectoryName("Test/", false));         // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("Test//", false));        // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("dir/Test/", false));     // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("dir/Test//", false));    // trailing '/'
  EXPECT_FALSE(CheckDirectoryName("/dir/Test", false));     // leading '/'
  EXPECT_FALSE(CheckDirectoryName("//dir/Test", false));    // leading '/'
  EXPECT_FALSE(CheckDirectoryName(" Test", true));          // leading ' '
  EXPECT_FALSE(CheckDirectoryName("/ Test", true));         // leading ' '
  EXPECT_FALSE(CheckDirectoryName("A/ Test", true));        // leading ' '
  EXPECT_TRUE (CheckDirectoryName(" Test/A", true));        // leading ' '    <- non-portable directory name accepted
  EXPECT_FALSE(CheckDirectoryName("Test ", true));          // trailing ' '
  EXPECT_FALSE(CheckDirectoryName("/Test ", true));         // trailing ' '
  EXPECT_FALSE(CheckDirectoryName("/Test", true));          // leading '/'
  EXPECT_FALSE(CheckDirectoryName("Test /", true));         // trailing ' ' + trailing '/'
  EXPECT_FALSE(CheckDirectoryName("B/Test ", true));        // trailing ' '
  EXPECT_TRUE (CheckDirectoryName("Test /B", true));        // trailing ' '   <- non-portable directory name accepted
  EXPECT_FALSE(CheckDirectoryName("Test  A", true));        // double ' '
  EXPECT_FALSE(CheckDirectoryName("A/Test  A", true));      // double ' '
  EXPECT_TRUE (CheckDirectoryName("Test  A/A", true));      // double ' '     <- non-portable directory name accepted
  EXPECT_FALSE(CheckDirectoryName("Test.", true));          // trailing '.'
  EXPECT_TRUE (CheckDirectoryName("Test./A", true));        // trailing '.'   <- non-portable directory name accepted
  EXPECT_FALSE(CheckDirectoryName("A/Test.", true));        // trailing '.'
  EXPECT_FALSE(CheckDirectoryName("-Test", true));          // leading '-'
  EXPECT_FALSE(CheckDirectoryName("A/-Test", true));        // leading '-'
  EXPECT_TRUE (CheckDirectoryName("-Test/A", true));        // leading '-'    <- non-portable directory name accepted
  EXPECT_FALSE(CheckDirectoryName("Test/../A/B/Test", true)); // ".." in path
  EXPECT_FALSE(CheckDirectoryName("Test/A/./B/Test", true));  // "." in path

  EXPECT_FALSE(CheckDirectoryName("A/.", true));            // directory name must not be '.'
  EXPECT_FALSE(CheckDirectoryName("A/..", true));           // directory name must not be '..'
  EXPECT_FALSE(CheckDirectoryName("A/./", true));           // directory name must not be '.' / trailing '/'
  EXPECT_FALSE(CheckDirectoryName("A/../", true));          // directory name must not be '..' / trailing '/'

  EXPECT_FALSE(CheckDirectoryName(".Test//Test", true));    // double "/"
  EXPECT_FALSE(CheckDirectoryName(".Test///Test", true));   // double "/"
  EXPECT_FALSE(CheckDirectoryName("//Test/.Test", true));   // double "/"

  // some non-recommended characters in directory name without path
  EXPECT_FALSE(CheckDirectoryName("Test (T)", true));
  EXPECT_FALSE(CheckDirectoryName("Test {T}", true));
  EXPECT_FALSE(CheckDirectoryName("Test [T]", true));
  EXPECT_FALSE(CheckDirectoryName("Test*T", true));
  EXPECT_FALSE(CheckDirectoryName("Test~T", true));
  EXPECT_FALSE(CheckDirectoryName("Test@T", true));
  EXPECT_FALSE(CheckDirectoryName("Test!", true));
  EXPECT_FALSE(CheckDirectoryName("Test 100%", true));
  EXPECT_FALSE(CheckDirectoryName("Test^100", true));
  EXPECT_FALSE(CheckDirectoryName("Test|ABC", true));
  EXPECT_FALSE(CheckDirectoryName("TestWithö", true));
  EXPECT_FALSE(CheckDirectoryName("TestWithü", true));
  EXPECT_FALSE(CheckDirectoryName("TestWithä", true));
  EXPECT_FALSE(CheckDirectoryName("TestWithÖ", true));
  EXPECT_FALSE(CheckDirectoryName("TestWithÜ", true));
  EXPECT_FALSE(CheckDirectoryName("TestWithÄ", true));

  // some non-recommended characters in directory name with path
  EXPECT_FALSE(CheckDirectoryName("A/Test (T)", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test {T}", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test [T]", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test*T", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test~T", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test@T", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test!", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test 100%", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test^100", true));
  EXPECT_FALSE(CheckDirectoryName("A/Test|ABC", true));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithö", true));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithü", true));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithä", true));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithÖ", true));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithÜ", true));
  EXPECT_FALSE(CheckDirectoryName("A/TestWithÄ", true));

  // some non-recommended characters in path-name
  EXPECT_TRUE(CheckDirectoryName("Test (T)/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test {T}/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test [T]/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test*T/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test~T/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test@T/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test!/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test 100%/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test^100/A", true));
  EXPECT_TRUE(CheckDirectoryName("Test|ABC/A", true));
  EXPECT_TRUE(CheckDirectoryName("TestWithö/A", true));
  EXPECT_TRUE(CheckDirectoryName("TestWithü/A", true));
  EXPECT_TRUE(CheckDirectoryName("TestWithä/A", true));
  EXPECT_TRUE(CheckDirectoryName("TestWithÖ/A", true));
  EXPECT_TRUE(CheckDirectoryName("TestWithÜ/A", true));
  EXPECT_TRUE(CheckDirectoryName("TestWithÄ/A", true));
}
TEST(gpcc_file_systems_linux_fs_tools_Tests, CheckNotTopDir)
{
  EXPECT_TRUE(CheckNotTopDir(""));
  EXPECT_TRUE(CheckNotTopDir("/"));
  EXPECT_TRUE(CheckNotTopDir("."));
  EXPECT_TRUE(CheckNotTopDir("./"));
  EXPECT_TRUE(CheckNotTopDir("./."));
  EXPECT_TRUE(CheckNotTopDir("/Folder"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/File.txt"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/../File.txt"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/././File.txt"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/././../File.txt"));
  EXPECT_TRUE(CheckNotTopDir("Folder"));
  EXPECT_TRUE(CheckNotTopDir("Folder/File.txt"));
  EXPECT_TRUE(CheckNotTopDir("Folder/../File.txt"));
  EXPECT_TRUE(CheckNotTopDir("Folder/././File.txt"));
  EXPECT_TRUE(CheckNotTopDir("Folder/././../File.txt"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/.."));
  EXPECT_TRUE(CheckNotTopDir("/Folder/../"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/..//"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/"));
  EXPECT_TRUE(CheckNotTopDir("/Folder/AnotherFolder/"));
  EXPECT_TRUE(CheckNotTopDir("/Folder"));
  EXPECT_TRUE(CheckNotTopDir("//"));
  EXPECT_TRUE(CheckNotTopDir("Folder//.."));

  EXPECT_FALSE(CheckNotTopDir(".."));
  EXPECT_FALSE(CheckNotTopDir("../"));
  EXPECT_FALSE(CheckNotTopDir("/.."));
  EXPECT_FALSE(CheckNotTopDir("/../"));
  EXPECT_FALSE(CheckNotTopDir("//.."));
  EXPECT_FALSE(CheckNotTopDir("..//"));
  EXPECT_FALSE(CheckNotTopDir("//..//"));
  EXPECT_FALSE(CheckNotTopDir("Folder/../../SomeFile.txt"));
  EXPECT_FALSE(CheckNotTopDir("Folder/..//../SomeFile.txt"));
  EXPECT_FALSE(CheckNotTopDir("Folder/../Blah/../../Folder/"));
  EXPECT_FALSE(CheckNotTopDir("/Folder/../../SomeFile.txt"));
  EXPECT_FALSE(CheckNotTopDir("/Folder/./../../SomeFile.txt"));
  EXPECT_FALSE(CheckNotTopDir("/Folder/././../../SomeFile.txt"));
  EXPECT_FALSE(CheckNotTopDir("/Folder/../Blah/../.."));
}

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
