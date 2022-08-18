/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#include "UnitTestDirProvider.hpp"
#include "tools.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <stdexcept>
#include <system_error>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

/**
 * \brief Constructor. Creates the object and the test folder managed by the created object.
 *
 * \pre    The test folder may or may not exist. If it exists, then this will delete its contents.
 *
 * \post   An empty folder "/tmp/GPCC_unit_tests_xxxx" (xxxx = PID of current process) is present.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is not thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the test folder may be existing (only if it was already existing before construction)
 * - the test folder's may not be empty (only if it was already existing before construction)
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the test folder may be existing (only if it was already existing before construction)
 * - the test folder's may not be empty (only if it was already existing before construction)
 *
 */
UnitTestDirProvider::UnitTestDirProvider(void)
: path("/tmp/GPCC_unit_tests_" + std::to_string(gpcc::osal::Thread::GetPID()) + "/")
{
  // check if test directory is not yet existing
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
  {
    // any other error than "file not existing" is not anticipated
    if (errno != ENOENT)
      throw std::system_error(errno, std::generic_category(), "UnitTestDirProvider::UnitTestDirProvider: \"stat\" failed on \"" + path + "\".");

    // create the test folder
    if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
      throw std::system_error(errno, std::generic_category(), "UnitTestDirProvider::UnitTestDirProvider: \"mkdir\" failed on \"" + path + "\".");
  }
  else
  {
    if (!S_ISDIR(st.st_mode))
      throw std::runtime_error("UnitTestDirProvider::UnitTestDirProvider: A file with name \"" + path + "\" is already existing, but we expected a directory.");

    DeleteTestDirContent();
  }
}

/**
 * \brief Destructor. Destroys the object and deletes the test folder and its contents.
 *
 * \post   The test folder created by the constructor has been deleted.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is not thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 */
UnitTestDirProvider::~UnitTestDirProvider(void)
{
  try
  {
    DeleteTestDirContent();

    if (rmdir(path.c_str()) != 0)
      throw std::system_error(errno, std::generic_category(), "UnitTestDirProvider::~UnitTestDirProvider: \"rmdir\" failed on \"" + path + "\".");
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Retrieves the absolute path to the test folder managed by this object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * String containing the absolute path to the test folder managed by this object.\n
 * The path has a trailing '/'.
 */
std::string UnitTestDirProvider::GetAbsPath(void) const
{
  return path;
}

/**
 * \brief Deletes the content of the test folder. And sub-directories and their content will be deleted recursively.
 *
 * \post   The test folder managed by this object is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is not thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Not all files/dirs in the test folder may have been deleted.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Not all files/dirs in the test folder may have been deleted.
 *
 */
void UnitTestDirProvider::DeleteTestDirContent(void)
{
  DeleteDirectoryContent(path);
}

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
