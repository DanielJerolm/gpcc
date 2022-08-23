/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#include "tools.hpp"
#include "gpcc/src/file_systems/exceptions.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/string/tools.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <system_error>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

/**
 * \brief Enumerates all files in a given directory and optionally recursively in sub-directories.
 *
 * Note:
 * - This only enumerates files. Directories are not enumerated.
 * - All types of files (regular and special) are enumerated.
 * - This follows symbolic links.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - elements may have been added to `out`
 *
 * \throws NoSuchDirectoryError   The directory `currDir` is not existing or it is a file ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 * \throws std::bad_alloc         Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - elements may have been added to `out`
 *
 * - - -
 *
 * \param currDir
 * Directory whose content shall be enumerated.\n
 * The string may or may not have a trailing forward slash ('/').\n
 * Example: /tmp/Test/subdir
 *
 * \param prefixForOut
 * This string is prepended to any enumerated item added to `out`.\n
 * This should be either an empty string, or it should have a trailing forward slash ('\').\n
 * Example: subdir/
 *
 * \param out
 * Enumerated files (incl. path) are appended to the list referenced by this parameter.\n
 * All paths are relative to `currDir`, with `prefixForOut` being prepended.\n
 * Added entries are not sorted in any way.
 *
 * \param recursive
 * Controls if the content of sub-directories shall be enumerated too:\n
 * true  = The content of sub-directories shall be enumerated recursively too.\n
 * false = The content of any sub-directory shall not be enumerated.
 */
static void Enumerate(std::string currDir, std::string const & prefixForOut, std::list<std::string> & out, bool const recursive)
{
  // open "currDir"
  DIR* const pDIR = opendir(currDir.c_str());
  if (pDIR == nullptr)
  {
    switch (errno)
    {
      case ENOENT:
        throw NoSuchDirectoryError(currDir);

      case ENOMEM:
        throw std::bad_alloc();

      case ENOTDIR:
        throw NoSuchDirectoryError(currDir);

      default:
        throw std::system_error(errno,
                                std::generic_category(),
                                "Enumerate: \"opendir\" failed on \"" + currDir + "\"");
    }
  }
  ON_SCOPE_EXIT() { closedir(pDIR); };

  // ensure that "currDir" has a trailing '/'
  EnsureTrailingForwardSlash(currDir);

  // loop through pDIR's entries and enumerate them and their children
  struct dirent * pEntry;
  errno = 0;
  while ((pEntry = readdir(pDIR)) != nullptr)
  {
    std::string const entryName(pEntry->d_name);

    // skip ".", "..", and empty entries
    if ((entryName.length() == 0) || (entryName == ".") || (entryName == ".."))
      continue;

    // determine if pEntry is a directory or not
    bool isDir;
    if (pEntry->d_type != DT_UNKNOWN)
    {
      isDir = (pEntry->d_type == DT_DIR);
    }
    else
    {
      // The underlying filesystem does not support d_type, so we fall-back on lstat.

      std::string const name = currDir + entryName;
      struct stat s;
      if (lstat(name.c_str(), &s) != 0)
        throw std::system_error(errno, std::generic_category(), "Enumerate: \"lstat\" failed on \"" + name + "\"");

      isDir = S_ISDIR(s.st_mode);
    }

    if (!isDir)
      out.emplace_back(prefixForOut + entryName);
    else if (recursive)
      Enumerate(currDir + entryName, prefixForOut + entryName + '/', out, true);

    errno = 0;
  }

  if (errno != 0)
    throw std::system_error(errno, std::generic_category(), "Enumerate: \"readdir\" failed on \"" + currDir + "\"");
}

/**
 * \brief Checks a single file/directory name for compliance with GPCC's rules for portable file names.
 *
 * On Linux, directory names and file names may be comprised of any characters except '/' and NUL.
 * However, certain characters should be avoided to minimize issues with files, especially when they are
 * exchanged between different platforms.
 *
 * This function checks a file or directory name for compliance with the following rules:
 * - Only characters 'A'-'Z', 'a'-'z', '0'-'9', '_', '-', '.', and ' ' are used.\n
 *   Note: ' ' is allowed but should be avoided.
 * - No leading ' '
 * - No trailing ' '
 * - No double ' '
 * - No trailing '.'
 * - No leading '-'
 * - The names "." and ".." are not allowed
 * - An empty string is not allowed
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param name
 * File or directory name that shall be checked.
 *
 * \return
 * Result of the check:\n
 * true  = OK\n
 * false = At least one of the rules is violated (see details).
 */
static bool CheckFileOrDirNameElement(std::string const & name) noexcept
{
  if (name.empty())
    return false;

  // name "." or ".."?
  if ((name == ".") || (name == ".."))
    return false;

  // leading or trailing ' '?
  if ((name.front() == ' ') || (name.back() == ' '))
    return false;

  // leading '-'?
  if (name.front() == '-')
    return false;

  // trailing '.'?
  if (name.back() == '.')
    return false;

  // double ' '?
  if (name.find("  ") != std::string::npos)
    return false;

  // check for not allowed characters:
  for (char const c: name)
  {
    if (((c < 'A') || (c > 'Z')) &&
        ((c < 'a') || (c > 'z')) &&
        ((c < '0') || (c > '9')) &&
         (c != '_') &&
         (c != '-') &&
         (c != '.') &&
         (c != ' '))
      return false;
  }

  return true;
}


/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Removes one or more potential trailing forward slashes ('/') at the end of a string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String from which a potential trailing forward slash ('/') shall be removed.\n
 * If there are multiple forward slashes, then all of them will be removed.
 */
void RemoveTrailingForwardSlash(std::string & s)
{
  while ((!s.empty()) && (s.back() == '/'))
    s.resize(s.length() - 1U);
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Ensures that a not-empty-string has at least one trailing forward slash ('/').
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc         Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * A forward slash ('/') will be appended to `s`, if `s` is not empty and if there is no trailing forward slash yet.
 */
void EnsureTrailingForwardSlash(std::string & s)
{
  if ((!s.empty()) && (s.back() != '/'))
    s += '/';
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Recursively deletes the content of a given directory, inclusive sub-directories and their content.
 *
 * Note:
 * - This does not follow symbolic links.
 * - This removes _every_ file:
 *   + regular files
 *   + sub-directories
 *   + symbolic links
 *   + sockets
 *   + FIFOs
 *   + devices
 * - All files are immediately removed from the file system. If any process has a file open, then
 *   the deletion of the file will be deferred until all processes have closed the file.
 *
 * \post   The directory specified by `dir` is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Not all items may have been deleted
 *
 * \throws NoSuchDirectoryError   The directory `dir` is not existing or it is a file ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 * \throws std::bad_alloc         Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Not all items may have been deleted
 *
 * - - -
 *
 * \param dir
 * String containing the path and name of the directory whose content shall be deleted.\n
 * The referenced directory itself will _not_ be deleted.\n
 * The string may or may not have a trailing forward slash ('/').
 */
void DeleteDirectoryContent(std::string dir)
{
  // open "dir"
  DIR* const pDIR = opendir(dir.c_str());
  if (pDIR == nullptr)
  {
    switch (errno)
    {
      case ENOENT:
        throw NoSuchDirectoryError(dir);

      case ENOMEM:
        throw std::bad_alloc();

      case ENOTDIR:
        throw NoSuchDirectoryError(dir);

      default:
        throw std::system_error(errno,
                                std::generic_category(),
                                "DeleteDirectoryContent: \"opendir\" failed on \"" + dir + "\"");
    }
  }
  ON_SCOPE_EXIT() { closedir(pDIR); };

  // ensure that "dir" has a trailing '/'
  EnsureTrailingForwardSlash(dir);

  // loop through pDIR's entries and delete them
  struct dirent * pEntry;
  errno = 0;
  while ((pEntry = readdir(pDIR)) != nullptr)
  {
    std::string const entryName(pEntry->d_name);

    // skip ".", "..", and empty entries
    if ((entryName.length() == 0) || (entryName == ".") || (entryName == ".."))
      continue;

    std::string const fullEntryName(dir + entryName);

    // determine if pEntry is a directory or not
    bool isDir;
    if (pEntry->d_type != DT_UNKNOWN)
    {
      isDir = (pEntry->d_type == DT_DIR);
    }
    else
    {
      // The underlying filesystem does not support d_type, so we fall-back on lstat.

      struct stat s;
      if (lstat(fullEntryName.c_str(), &s) != 0)
        throw std::system_error(errno, std::generic_category(), "DeleteDirectoryContent: \"lstat\" failed on \"" + fullEntryName + "\"");

      isDir = S_ISDIR(s.st_mode);
    }

    // if it is a directory, then delete its contents first
    if (isDir)
    {
      DeleteDirectoryContent(fullEntryName);

      if (rmdir(fullEntryName.c_str()) != 0)
        throw std::system_error(errno, std::generic_category(), "DeleteDirectoryContent: \"rmdir\" failed on \"" + fullEntryName + "\"");
    }
    else
    {
      if (unlink(fullEntryName.c_str()) != 0)
        throw std::system_error(errno, std::generic_category(), "DeleteDirectoryContent: \"unlink\" failed on \"" + fullEntryName + "\"");
    }

    errno = 0;
  }

  if (errno != 0)
    throw std::system_error(errno, std::generic_category(), "DeleteDirectoryContent: \"readdir\" failed on \"" + dir + "\"");
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Enumerates all files in a given directory and optionally recursively in sub-directories.
 *
 * Note:
 * - This only enumerates files. Directories are not enumerated.
 * - All types of files (regular and special) are enumerated.
 * - This follows symbolic links.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - elements may have been added to `out`
 *
 * \throws NoSuchDirectoryError   The directory `currDir` is not existing or it is a file ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 * \throws std::bad_alloc         Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - elements may have been added to `out`
 *
 * - - -
 *
 * \param dir
 * Directory whose content shall be enumerated.\n
 * The string may or may not have a trailing forward slash ('/').\n
 * Example: /tmp/Test
 *
 * \param out
 * Enumerated files (incl. path) are appended to the list referenced by this parameter.\n
 * All paths are relative to `dir`.\n
 * Added entries are not sorted in any way.
 *
 * \param recursive
 * Controls if the content of sub-directories shall be enumerated too:\n
 * true  = The content of sub-directories shall be enumerated recursively too.\n
 * false = The content of any sub-directory shall not be enumerated.
 */
void EnumerateFiles(std::string const & dir, std::list<std::string> & out, bool const recursive)
{
  Enumerate(dir, "", out, recursive);
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Checks if a file or directory is existing or not.
 *
 * This checks _if a file or directory_ with given name exists.\n
 * Use @ref CheckFileExists() to check _if a file_ with given name is existing or not.\n
 * Use @ref CheckDirExists() to check _if a directory_ with given name is existing or not.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name (and optional path) of the file or directory whose existence shall be tested.\n
 * A trailing forward slash '/' is not mandatory.\n
 * If this refers to a symbolic link, then it will be dereferenced.
 *
 * \return
 * true  = file/directory exists\n
 * false = file/directory does not exist
 */
bool CheckFileOrDirExists(std::string const & name)
{
  if (access(name.c_str(), F_OK) == 0)
    return true;

  if ((errno == ENOENT) || (errno == ENOTDIR))
    return false;

  throw std::system_error(errno, std::generic_category(), "CheckFileOrDirExists: \"access\" failed on \"" + name + "\"");
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Checks if a file is existing or not.
 *
 * This checks _if a file_ with given name exists.\n
 * Use @ref CheckFileOrDirExists() to check _if a file or directory_ with given name is existing or not.\n
 * Use @ref CheckDirExists() to check _if a directory_ with given name is existing or not.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name (and optional path) of the file whose existence shall be tested.\n
 * If this refers to a symbolic link, then it will be dereferenced.
 *
 * \return
 * true  = file exists\n
 * false = file does not exist or `name` refers to a directory.
 */
bool CheckFileExists(std::string const & name)
{
  struct stat s;
  if (stat(name.c_str(), &s) != 0)
  {
    if ((errno == ENOENT) || (errno == ENOTDIR))
      return false;
    else
      throw std::system_error(errno, std::generic_category(), "CheckFileExists: \"stat\" failed on \"" + name + "\"");
  }

  return !S_ISDIR(s.st_mode);
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Checks if a directory is existing or not.
 *
 * This checks _if a directory_ with given name exists.\n
 * Use @ref CheckFileOrDirExists() to check _if a file or directory_ with given name is existing or not.\n
 * Use @ref CheckFileExists() to check _if a file_ with given name is existing or not.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name (and optional path) of the directory whose existence shall be tested.\n
 * A trailing forward slash '/' is not mandatory.\n
 * If this refers to a symbolic link, then it will be dereferenced.
 *
 * \return
 * true  = directory exists\n
 * false = directory does not exist or `name` refers to a file.
 */
bool CheckDirExists(std::string const & name)
{
  struct stat s;
  if (stat(name.c_str(), &s) != 0)
  {
    if ((errno == ENOENT) || (errno == ENOTDIR))
      return false;
    else
      throw std::system_error(errno, std::generic_category(), "CheckDirExists: \"stat\" failed on \"" + name + "\"");
  }

  return S_ISDIR(s.st_mode);
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Checks a given filename (incl. path) for "portable style" (see details).
 *
 * On Linux, directory names and file names may be comprised of any characters except '/' and NUL.
 * However, certain characters should be avoided to minimize issues with files, especially when they are
 * exchanged between different platforms. GPCC provides some requirements for portable file and directory names
 * (see @ref GPCC_FILESYSTEMS section "Limitations" -> "File and directory names").
 *
 * This function checks the name of the file referenced by `name` for compliance with the following rules:
 * - Only characters 'A'-'Z', 'a'-'z', '0'-'9', '_', '-', '.', and ' ' are allowed.\n
 *   Note: ' ' is allowed but should be avoided.
 * - No leading ' '
 * - No trailing ' '
 * - No double ' '
 * - No trailing '.'
 * - No leading '-'
 *
 * Depending on parameter `checkFileOnly`, any directory names contained in `name` can also be checked for
 * compliance with the rules listed above.
 *
 * Regardless of parameter `checkFileOnly`, `name` is always checked for compliance with the following rules:
 * - No leading '/'
 * - No trailing '/'
 * - No double '/'
 * - No "." or ".." as file or directory name
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param name
 * Filename (may include a path) that shall be examined.
 *
 * \param acceptPath
 * Controls, if `name` is allowed to include a path.\n
 * If this is false and `name` includes a path, then this will return false.
 *
 * \param checkFileOnly
 * Controls, if the names of directories contained in `name` shall be checked, or if only the
 * last portion of `name` (the filename) shall be checked:\n
 * true  = check filename only\n
 * false = check the filename and the names of all directories (if any)
 *
 * If `acceptPath` is false, then this parameter is don't care.
 *
 * \return
 * Result of the check:\n
 * true  = OK\n
 * false = At least one of the rules is violated (see details), or `name` contains an unexpected
 *         path specification, or `name` is an empty string.
 */
bool CheckFileName(std::string const & name, bool const acceptPath, bool const checkFileOnly)
{
  // special case: Empty string
  if (name.empty())
    return false;

  if (!acceptPath)
  {
    // special case: No path accepted, but path present
    if (name.find_first_of('/') != std::string::npos)
      return false;
  }
  else
  {
    // check for double "/"
    if (name.find("//", 0, 2) != std::string::npos)
      return false;
  }

  // special case: leading or trailing "/"
  if ((name.front() == '/') || (name.back() == '/'))
    return false;

  // parse directory names and file name one by one
  auto const names = gpcc::string::Split(name, '/', true);

  // (note: "names" cannot be empty, because name.length != 0 and there is no leading/trailing '/')

  if (checkFileOnly)
  {
    // only check the file name for portability
    if (!CheckFileOrDirNameElement(names.back()))
      return false;

    // all others must not be "." or ".."
    for (size_t i = 0; i < names.size() - 1U; i++)
    {
      if ((names[i] == ".") || (names[i] == ".."))
        return false;
    }
  }
  else
  {
    // check all directory names and the file name for portability
    for (auto const & name: names)
    {
      if (!CheckFileOrDirNameElement(name))
        return false;
    }
  }

  return true;
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Checks a given directory name for "portable style" (see details).
 *
 * On Linux, directory names and file names may be comprised of any characters except '/' and NUL.
 * However, certain characters should be avoided to minimize issues with files, especially when they are
 * exchanged between different platforms. GPCC provides some requirements for portable file and directory names
 * (see @ref GPCC_FILESYSTEMS section "Limitations" -> "File and directory names").
 *
 * This function checks the name of the directory referenced by `name` for compliance with the following rules:
 * - Only characters 'A'-'Z', 'a'-'z', '0'-'9', '_', '-', '.', and ' ' are allowed.\n
 *   Note: ' ' is allowed but should be avoided.
 * - No leading ' '
 * - No trailing ' '
 * - No double ' '
 * - No trailing '.'
 * - No leading '-'
 *
 * Depending on parameter `checkDirectoryOnly`, any directory names contained in `name` that form a path specification
 * can also be checked for compliance with the rules listed above.
 *
 * Regardless of parameter `checkDirectoryOnly`, `name` is always checked for compliance with the following rules:
 * - No leading '/'
 * - No trailing '/'
 * - No double '/'
 * - No "." or ".." as directory name
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param name
 * Directory name (may include a path with names of parent directories) that shall be examined.
 *
 * \param checkDirectoryOnly
 * Controls, if the names of all directories (incl. parent directories, if any) shall be checked,
 * or if only the last portion of `name` (the directory referenced by `name`) shall be checked:\n
 * true  = check referenced directory only\n
 * false = check the names of all directories, incl. potential parent directories
 *
 * \return
 * Result of the check:\n
 * true  = OK\n
 * false = At least one of the rules is violated (see details), or `name` is an empty string.
 */
bool CheckDirectoryName(std::string const & name, bool const checkDirectoryOnly)
{
  // special case: Empty string
  if (name.empty())
    return false;

  // special case: leading or trailing "/"
  if ((name.front() == '/') || (name.back() == '/'))
    return false;

  // check for double "/"
  if (name.find("//", 0, 2) != std::string::npos)
    return false;

  // parse directory names one by one
  auto const names = gpcc::string::Split(name, '/', true);

  // (note: "names" cannot be empty, because name.length != 0 and there is no leading/trailing '/')

  if (checkDirectoryOnly)
  {
    // only check the directory name for portability
    if (!CheckFileOrDirNameElement(names.back()))
      return false;

    // all others must not be "." or ".."
    for (size_t i = 0; i < names.size() - 1U; i++)
    {
      if ((names[i] == ".") || (names[i] == ".."))
        return false;
    }
  }
  else
  {
    // check all directory names for portability
    for (auto const & name: names)
    {
      if (!CheckFileOrDirNameElement(name))
        return false;
    }
  }

  return true;
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Checks if a relative path (optionally plus filename) or part of it refers to a directory or crosses a
 *        directory which is the parent directory of the directory where the given relative path starts at.
 *
 * Paths may contain ".." which refers to the parent directory. This allows to refer to directories and
 * files which are not below the directory to which the relative path is applied to.
 *
 * Examples (good):
 * - /testFolder/xyz
 * - /testFolder/xyz/
 * - /testFolder/xyz/..
 * - /testFolder/..
 * - /testFolder/../xyz
 *
 * Examples (bad):
 * - /testFolder/../../xyz
 * - /testFolder/../../xyz/
 * - /../xyz
 * - ../xyz
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param path
 * Path/filename that shall be examined.\n
 * A trailing '/' is optional for directory names.\n
 * __Note:__\n
 * If any portion of `path` is a link in real-life, then this function is not applicable.
 *
 * \return
 * true  = `path` and none of its parts refer to the parent directory of the directory `path` is relative to.\n
 * false = `path` or parts of it refer to the parent directory of the directory `path` is relative to.
 */
bool CheckNotTopDir(std::string const & path) noexcept
{
  int32_t level = 0;

  enum
  {
    firstChar,
    checkForSecondDot,
    gotSecondDot,
    lookForFwdSlash
  } state = firstChar;

  for (char const c: path)
  {
    switch (state)
    {
      case firstChar:
        // first character after '/' or the very beginning
        if (c == '.')
          state = checkForSecondDot;
        else if (c != '/')
          state = lookForFwdSlash;
        break;

      case checkForSecondDot:
        // first character after '/' (or the very beginning) was a '.'
        if (c == '.')
          state = gotSecondDot;
        else if (c == '/')
          state = firstChar;
        else
          state = lookForFwdSlash;
        break;

      case gotSecondDot:
        // first two characters after '/' (or the very beginning) were ".."
        if (c == '/')
        {
          level--;
          if (level < 0)
            return false;
          state = firstChar;
        }
        else
          state = lookForFwdSlash;
        break;

      case lookForFwdSlash:
        // we're inside a file or directory name, look for '/'
        if (c == '/')
        {
          level++;
          state = firstChar;
        }
        break;
    } // switch (state)
  }

  if (state == gotSecondDot)
    level--;

  return (level >= 0);
}

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
