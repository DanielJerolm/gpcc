/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#ifndef UNITTESTDIRPROVIDER_HPP_2018090911118
#define UNITTESTDIRPROVIDER_HPP_2018090911118

#include <string>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Creates (and finally removes) a temporary directory that can be used by unit tests
 *        to create files and directories.
 *
 * Remember: GPCC unit tests are intended to be executed on Linux.
 *
 * The temporary folder provided by this is: /tmp/GPCC_unit_tests_xxxx/
 *
 * The name of the test directory contains the PID (xxxx) of the current process. This allows build-servers to run
 * multiple processes executing GPCC unit tests in parallel without interfering with each other.
 *
 * # Usage
 * The intended usage is, that either a unit test case or a unit test fixture which requires a temporary
 * directory inside the host's file system instantiates this class.
 *
 * Upon instantiation, the test directory will be created. The test directory is guaranteed to be empty.
 * If the directory is already existing (e.g. from a previously failed unit test), then its content will be deleted to
 * get an empty directory.
 *
 * The absolute path to the test directory can be retrieved via @ref GetAbsPath(). \n
 * If required, then the test directory's content can be deleted via @ref DeleteTestDirContent().
 *
 * Upon destruction, the test directory and its contents will be deleted.
 *
 * # Hints on usage in conjunction with googletest
 * ## Death tests
 * ### Recall of googletest death test styles
 * On Linux, googletest offers two death test styles:
 * - __fast-style__\n
 *   The process executing the unit tests is simply forked. The test case is _not_ re-executed by the child process.
 *   Instead the child just continues at the point where the fork took place.\n
 *   This style is suitable for single-threaded tests only.
 * - __threadsafe-style__\n
 *   A new process is spawn which _re-executes_ the whole test case.\n
 *   This style shall be used if the test case involves multiple threads.
 *
 * On Windows, only _threadsafe-style_ is available.
 *
 * The death test style is configured as shown below:
 * ~~~{.cpp}
 * TEST_F(gpcc_someNameSpace_xyz_DeathTestsF, Test_abc)
 * {
 *   ::testing::FLAGS_gtest_death_test_style = "threadsafe"; // or "fast"
 *   // [...]
 * }
 * ~~~
 *
 * For details, please refer to https://github.com/google/googletest/blob/master/googletest/docs/advanced.md
 *
 * ### Complications
 * #### threadsafe-style
 * If class @ref UnitTestDirProvider is used in death tests using "threadsafe" style, then a new process will be spawn
 * which re-executes the whole test case. This will result in creation of a second GPCC_unit_tests_xxxx directory with
 * the child's PID. This directory __will not be removed__ when the test dies, because the destructor of class
 * @ref UnitTestDirProvider will never be executed.
 *
 * #### fast-style
 * If class @ref UnitTestDirProvider is used in death tests using "fast" style, then the process will be forked and the
 * test case will _not_ be re-executed. The child process will share the GPCC_unit_tests_xxxx folder with the parent
 * process. The parent process shall be aware that the folder and its content may be modified by the child. However,
 * the GPCC_unit_tests_xxxx folder and its contents will finally be deleted by the parent process, so there are no
 * remains in /tmp.
 *
 * ## Non-death tests
 * If the process dies during execution of the unit test, then the GPCC_unit_tests_xxxx folder will not be deleted.
 *
 * If a test case fails (of course gracefully), then the GPCC_unit_tests_xxxx folder and its contents will be removed
 * when the @ref UnitTestDirProvider instance is destroyed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class UnitTestDirProvider final
{
  public:
    UnitTestDirProvider(void);
    UnitTestDirProvider(UnitTestDirProvider const &) = delete;
    UnitTestDirProvider(UnitTestDirProvider &&) = delete;
    ~UnitTestDirProvider(void);

    UnitTestDirProvider& operator=(UnitTestDirProvider const &) = delete;
    UnitTestDirProvider& operator=(UnitTestDirProvider &&) = delete;

    std::string GetAbsPath(void) const;
    void DeleteTestDirContent(void);

  private:
    /// Absolute path to the test directory. There is a trailing '/'.
    std::string const path;
};

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // UNITTESTDIRPROVIDER_HPP_2018090911118
#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
