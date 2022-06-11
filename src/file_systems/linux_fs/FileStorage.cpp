/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2021, 2022 Daniel Jerolm

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

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#include "FileStorage.hpp"
#include "internal/StdIOFileReader.hpp"
#include "internal/StdIOFileWriter.hpp"
#include "internal/tools.hpp"
#include "gpcc/src/file_systems/exceptions.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/tools.hpp"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <system_error>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws NoSuchDirectoryError   Directory referenced by `_baseDir` is not existing or `_baseDir` does not refer to
 *                                a directory ([details](@ref NoSuchDirectoryError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _baseDir
 * Base directory or "working directory" for the new created @ref FileStorage instance. Only files inside
 * this directory and inside sub-directories of this can be accessed through the created @ref FileStorage instance.
 *
 * All filenames (and paths) passed to the methods offered by the new created @ref FileStorage instance will be
 * treated relative to this path.
 *
 * This should be an absolute path. A trailing forward slash '/' is required.\n
 * This must refer to an existing directory.
 */
FileStorage::FileStorage(std::string const & _baseDir)
: IFileAndDirectoryStorage()
, baseDir(_baseDir)
, mutex()
, fileLockManager()
{
  if ((baseDir.empty()) || (baseDir.back() != '/'))
    throw std::invalid_argument("StdIOFileStorage::StdIOFileStorage: _baseDir: empty or missing trailing '/'");

  if (!internal::CheckDirExists(baseDir))
    throw NoSuchDirectoryError(baseDir);
}

/**
 * \brief Destructor.
 *
 * \pre   All files opened or created via the @ref FileStorage object must be closed before the object is destroyed,
 *        otherwise the program will be terminated via Panic().
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
FileStorage::~FileStorage(void)
{
  osal::MutexLocker mutexLocker(mutex);
  if (fileLockManager.IsAnyLock())
    osal::Panic("FileStorage::~FileStorage: Not all files closed");
}

/// \copydoc IFileStorage::Open
std::unique_ptr<Stream::IStreamReader> FileStorage::Open(std::string const & name)
{
  BasicCheckName(name);

  std::string const fullName(baseDir + name);
  std::string const lockID('/' + name + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetReadLock(lockID))
    throw FileAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseReadLock(lockID);
  };

  auto spISR = std::unique_ptr<Stream::IStreamReader>(new internal::StdIOFileReader(fullName, *this, lockID));

  ON_SCOPE_EXIT_DISMISS();

  return spISR;
}

/// \copydoc IFileStorage::Create
std::unique_ptr<Stream::IStreamWriter> FileStorage::Create(std::string const & name, bool const overwriteIfExisting)
{
  FullCheckFileName(name);

  std::string const fullName(baseDir + name);
  std::string const lockID('/' + name + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID))
    throw FileAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID);
  };

  auto spISW = std::unique_ptr<Stream::IStreamWriter>(new internal::StdIOFileWriter(fullName, overwriteIfExisting, *this, lockID));

  ON_SCOPE_EXIT_DISMISS();

  return spISW;
}

/// \copydoc IFileStorage::Delete
void FileStorage::Delete(std::string const & name)
{
  BasicCheckName(name);

  std::string const fullName(baseDir + name);
  std::string const lockID('/' + name + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID))
    throw FileAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID);
  };

  // Note:
  // - unlink() will not dereference links.
  //   If fullName refers to a symbolic link, then the link will be deleted, not the referenced file.
  //   If fullName refers to a hard link, then the link will be deleted, and the referenced file will be deleted too if
  //   there are no other (hard) links to the file.
  // - unlink() will not delete directories.
  if (unlink(fullName.c_str()) != 0)
  {
    if (errno == EBUSY)
      throw FileAlreadyAccessedError(name);
    else if ((errno == EISDIR) || (errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchFileError(name);
    else
      throw std::system_error(errno, std::generic_category(), "FileStorage::Delete: \"unlink\" failed on \"" + name + "\"");
  }
}

/// \copydoc IFileStorage::Rename
void FileStorage::Rename(std::string const & currName, std::string const & newName)
{
  BasicCheckName(currName);
  FullCheckFileName(newName);

  std::string const fullCurrName(baseDir + currName);
  std::string const fullNewName(baseDir + newName);

  if (currName == newName)
  {
    if (!internal::CheckFileExists(fullCurrName))
      throw NoSuchFileError(currName);
    else
      return;
  }

  std::string const lockID_curr('/' + currName + '/');
  std::string const lockID_new('/' + newName + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID_curr))
    throw FileAlreadyAccessedError(currName);

  try
  {
    if (!fileLockManager.GetWriteLock(lockID_new))
      throw FileAlreadyAccessedError(newName);
  }
  catch (...)
  {
    fileLockManager.ReleaseWriteLock(lockID_curr);
    throw;
  }

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID_curr);
    fileLockManager.ReleaseWriteLock(lockID_new);
  };

  if (!internal::CheckFileExists(fullCurrName))
    throw NoSuchFileError(currName);

  if (internal::CheckFileOrDirExists(fullNewName))
    throw FileAlreadyExistingError(newName);

  // Here we are:
  // - "fullCurrName" refers to an existing FILE.
  // - "fullNewName" refers to a not-existing FILE. It is not sure if path (if any) exists.

  if (rename(fullCurrName.c_str(), fullNewName.c_str()) != 0)
  {
    // errno is evaluated according to "http://man7.org/linux/man-pages/man2/rename.2.html"

    if (errno == EISDIR)
      throw FileAlreadyExistingError(newName);
    else if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchDirectoryError(newName);
    else if ((errno == EDQUOT) || (errno == ENOSPC))
      throw InsufficientSpaceError();
    else
      throw std::system_error(errno, std::generic_category(), "FileStorage::Rename: \"rename\" failed (old: \"" + currName + "\", new: \"" + newName + "\")");
  }
}

/// \copydoc IFileStorage::Enumerate
std::list<std::string> FileStorage::Enumerate(void) const
{
  std::string const lockID("/");

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetReadLock(lockID))
    throw DirectoryAlreadyAccessedError("/");

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseReadLock(lockID);
  };

  std::list<std::string> fileList;
  internal::EnumerateFiles(baseDir, fileList, true);
  fileList.sort();
  return fileList;
}

/// \copydoc IFileStorage::DetermineSize
size_t FileStorage::DetermineSize(std::string const & name, size_t * const pTotalSize) const
{
  BasicCheckName(name);

  std::string const fullName(baseDir + name);
  std::string const lockID('/' + name + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetReadLock(lockID))
    throw FileAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseReadLock(lockID);
  };

  struct stat s;
  if (stat(fullName.c_str(), &s) != 0)
  {
    if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchFileError(name);
    else
      throw std::system_error(errno, std::generic_category(), "FileStorage::DetermineSize: \"stat\" failed on: \"" + name + "\"");
  }

  if (S_ISDIR(s.st_mode))
    throw NoSuchFileError(name);

  if (pTotalSize != nullptr)
    *pTotalSize = s.st_size;
  return s.st_size;
}

/// \copydoc IFileStorage::GetFreeSpace
size_t FileStorage::GetFreeSpace(void) const
{
  struct statfs64 s;
  if (statfs64(baseDir.c_str(), &s) != 0)
    throw std::system_error(errno, std::generic_category(), "FileStorage::GetFreeSpace: \"statfs64\" failed.");

  return s.f_bavail * s.f_bsize;
}

/// \copydoc IFileAndDirectoryStorage::IsDirectoryExisting
bool FileStorage::IsDirectoryExisting(std::string const & name)
{
  if (!name.empty())
    BasicCheckName(name);

  return internal::CheckDirExists(baseDir + name);
}

/// \copydoc IFileAndDirectoryStorage::CreateDirectory
void FileStorage::CreateDirectory(std::string const & name)
{
  FullCheckDirectoryName(name);

  std::string const lockID('/' + name + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID))
    throw DirectoryAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID);
  };

  std::string const fullName = baseDir + name;
  if (mkdir(fullName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
  {
    if (errno == EEXIST)
    {
      if (internal::CheckFileExists(fullName))
        throw FileAlreadyExistingError(name);
      else
        throw DirectoryAlreadyExistingError(name);
    }
    else if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchDirectoryError(name);
    else if ((errno == ENOSPC) || (errno == EDQUOT))
      throw InsufficientSpaceError();
    else
      throw std::system_error(errno, std::generic_category(), "FileStorage::CreateDirectory: \"mkdir\" failed on \"" + name + "\".");
  }
}

/// \copydoc IFileAndDirectoryStorage::DeleteDirectoryContent
void FileStorage::DeleteDirectoryContent(std::string const & name)
{
  std::string lockID;
  if (!name.empty())
  {
    BasicCheckName(name);
    lockID = '/' + name + '/';
  }
  else
  {
    lockID = "/";
  }

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID))
    throw DirectoryAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID);
  };

  internal::DeleteDirectoryContent(baseDir + name);
}

/// \copydoc IFileAndDirectoryStorage::DeleteDirectory
void FileStorage::DeleteDirectory(std::string const & name)
{
  BasicCheckName(name);

  std::string const fullName = baseDir + name;
  std::string const lockID('/' + name + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID))
    throw DirectoryAlreadyAccessedError(name);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID);
  };

  if (rmdir(fullName.c_str()) != 0)
  {
    if (errno == EINVAL)
      throw InvalidFileNameError(name);
    else if ((errno == EEXIST) || (errno == ENOTEMPTY))
      throw DirectoryNotEmptyError(name);
    else if (errno == ENOENT)
      throw NoSuchDirectoryError(name);
    else if (errno == ENOTDIR)
    {
      // ENOTDIR means:
      // - A component of path is not a directory.
      // OR
      // - "fullName" refers to a symbolic link (to a directory)

      // check: does "fullName" refer to an existing directory (via a symbolic link)?
      if (!internal::CheckDirExists(fullName))
        throw NoSuchDirectoryError(name);

      // ensure that "fullName" refers to a symbolic link
      struct stat s;
      if (lstat(fullName.c_str(), &s) != 0)
        throw std::system_error(errno, std::generic_category(), "FileStorage::DeleteDirectory: \"lstat\" failed on \"" + name + "\"");

      if (!S_ISLNK(s.st_mode))
        throw std::runtime_error("FileStorage::DeleteDirectory: \"" + name + "\" is assumed to be a symbolic link to a directory, but it is not a symbolic link.");

      if (unlink(fullName.c_str()) != 0)
        throw std::system_error(errno, std::generic_category(), "FileStorage::DeleteDirectory: \"unlink\" failed on symbolic link \"" + name + "\"");
    }
    else
      throw std::system_error(errno, std::generic_category(), "FileStorage::DeleteDirectory: \"rmdir\" failed on \"" + name + "\"");
  }
}

/// \copydoc IFileAndDirectoryStorage::RenameDirectory
void FileStorage::RenameDirectory(std::string const & currName, std::string const & newName)
{
  BasicCheckName(currName);
  FullCheckDirectoryName(newName);

  std::string const fullCurrName(baseDir + currName);
  std::string const fullNewName(baseDir + newName);

  if (currName == newName)
  {
    if (!internal::CheckDirExists(fullCurrName))
      throw NoSuchDirectoryError(currName);
    else
      return;
  }

  std::string const lockID_curr('/' + currName + '/');
  std::string const lockID_new('/' + newName + '/');

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetWriteLock(lockID_curr))
    throw DirectoryAlreadyAccessedError(currName);

  try
  {
    if (!fileLockManager.GetWriteLock(lockID_new))
      throw DirectoryAlreadyAccessedError(newName);
  }
  catch (...)
  {
    fileLockManager.ReleaseWriteLock(lockID_curr);
    throw;
  }

  mutexLocker.Unlock();

  ON_SCOPE_EXIT()
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseWriteLock(lockID_curr);
    fileLockManager.ReleaseWriteLock(lockID_new);
  };

  if (!internal::CheckDirExists(fullCurrName))
    throw NoSuchDirectoryError(currName);

  if (internal::CheckFileOrDirExists(fullNewName))
  {
    if (internal::CheckDirExists(fullNewName))
      throw DirectoryAlreadyExistingError(newName);
    else
      throw FileAlreadyExistingError(newName);
  }

  // Here we are:
  // - "fullCurrName" refers to an existing directory.
  // - "fullNewName" refers to a not-existing directory. It is not sure if path (if any) exists.

  if (rename(fullCurrName.c_str(), fullNewName.c_str()) != 0)
  {
    // errno is evaluated according to "http://man7.org/linux/man-pages/man2/rename.2.html"

    if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchDirectoryError(newName);
    else if ((errno == EDQUOT) || (errno == ENOSPC))
      throw InsufficientSpaceError();
    else if ((errno == ENOTEMPTY) || (errno == EEXIST))
      throw DirectoryAlreadyExistingError(newName);
    else
      throw std::system_error(errno, std::generic_category(), "FileStorage::RenameDirectory: \"rename\" failed (old: \"" + currName + "\", new: \"" + newName + "\")");
  }
}

/// \copydoc IFileAndDirectoryStorage::EnumerateSubDirectories
std::list<std::string> FileStorage::EnumerateSubDirectories(std::string const & dir)
{
  std::string lockID;
  if (!dir.empty())
  {
    BasicCheckName(dir);
    lockID = '/' + dir + '/';
  }
  else
  {
    lockID = "/";
  }

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetReadLock(lockID))
    throw DirectoryAlreadyAccessedError(dir);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT(unlock)
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseReadLock(lockID);
  };

  // open directory
  std::string fullname = baseDir + dir;
  DIR* const pDIR = opendir(fullname.c_str());
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
                                "FileStorage::EnumerateSubDirectories: \"opendir\" failed on \"" + dir + "\"");
    }
  }
  ON_SCOPE_EXIT(closeDir) { closedir(pDIR); };

  // ensure that "fullname" has a trailing '/'
  internal::EnsureTrailingForwardSlash(fullname);

  // loop through pDIR's directory entries and enumerate them
  std::list<std::string> subDirs;
  struct dirent * pEntry;
  errno = 0;
  while ((pEntry = readdir(pDIR)) != nullptr)
  {
    std::string entryName(pEntry->d_name);

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

      std::string const name = fullname + entryName;
      struct stat s;
      if (lstat(name.c_str(), &s) != 0)
        throw std::system_error(errno, std::generic_category(), "FileStorage::EnumerateSubDirectories: \"lstat\" failed on \"" + name + "\"");

      isDir = S_ISDIR(s.st_mode);
    }

    if (isDir)
      subDirs.emplace_back(std::move(entryName));

    errno = 0;
  }

  if (errno != 0)
    throw std::system_error(errno, std::generic_category(), "FileStorage::EnumerateSubDirectories: \"readdir\" failed on \"" + dir + "\"");

  subDirs.sort();
  return subDirs;
}

/// \copydoc IFileAndDirectoryStorage::EnumerateFiles
std::list<std::string> FileStorage::EnumerateFiles(std::string const & dir)
{
  std::string lockID;
  if (!dir.empty())
  {
    BasicCheckName(dir);
    lockID = '/' + dir + '/';
  }
  else
  {
    lockID = "/";
  }

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (!fileLockManager.GetReadLock(lockID))
    throw DirectoryAlreadyAccessedError(dir);

  mutexLocker.Unlock();

  ON_SCOPE_EXIT(unlock)
  {
    mutexLocker.Relock();
    fileLockManager.ReleaseReadLock(lockID);
  };

  // open directory
  std::string fullname = baseDir + dir;
  DIR* const pDIR = opendir(fullname.c_str());
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
                                "FileStorage::EnumerateFiles: \"opendir\" failed on \"" + dir + "\"");
    }
  }
  ON_SCOPE_EXIT(closeDir) { closedir(pDIR); };

  // ensure that "fullname" has a trailing '/'
  internal::EnsureTrailingForwardSlash(fullname);

  // loop through pDIR's file entries and enumerate them
  std::list<std::string> fileNames;
  struct dirent * pEntry;
  errno = 0;
  while ((pEntry = readdir(pDIR)) != nullptr)
  {
    std::string entryName(pEntry->d_name);

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

      std::string const name = fullname + entryName;
      struct stat s;
      if (lstat(name.c_str(), &s) != 0)
        throw std::system_error(errno, std::generic_category(), "FileStorage::EnumerateFiles: \"lstat\" failed on \"" + name + "\"");

      isDir = S_ISDIR(s.st_mode);
    }

    if (!isDir)
      fileNames.emplace_back(std::move(entryName));

    errno = 0;
  }

  if (errno != 0)
    throw std::system_error(errno, std::generic_category(), "FileStorage::EnumerateFiles: \"readdir\" failed on \"" + dir + "\"");

  fileNames.sort();
  return fileNames;
}

/**
 * \brief Releases a read-lock for a specific file.
 *
 * This is intended to be invoked by an @ref internal::StdIOFileReader instance previously created by this.
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
 * \param unlockID
 * ID of the read-lock.\n
 * This is the ID which has been passed to the constructor of class @ref internal::StdIOFileReader.
 */
void FileStorage::ReleaseReadLock(std::string const & unlockID)
{
  osal::MutexLocker mutexLocker(mutex);
  fileLockManager.ReleaseReadLock(unlockID);
}

/**
 * \brief Releases a write-lock for a specific file.
 *
 * This is intended to be invoked by an @ref internal::StdIOFileWriter instance previously created by this.
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
 * \param unlockID
 * ID of the write-lock.\n
 * This is the ID which has been passed to the constructor of class @ref internal::StdIOFileWriter.
 */
void FileStorage::ReleaseWriteLock(std::string const & unlockID)
{
  osal::MutexLocker mutexLocker(mutex);
  fileLockManager.ReleaseWriteLock(unlockID);
}

/**
 * \brief Checks a filename or directory name for compliance with GPCC's rules for portable file names.
 *
 * This method does not perform the full check for "portable style" required by GPCC.\n
 * Use @ref FullCheckFileName() or @ref FullCheckDirectoryName() if a full check is required.
 *
 * This method checks compliance with the following rules:
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
 * \throws InvalidFileNameError   The filename/directory name violates the rules for portable file names required by GPCC.
 *                                The "additional" rules are violated only ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param name
 * Filename or directory name (incl. path) that shall be checked.
 */
void FileStorage::BasicCheckName(std::string const & name) const
{
  if ((name.empty()) ||
      (name == ".") ||
      (name == "..") ||
      (name.front() == '/') ||
      (name.back() == '/') ||
      (name.find("//", 0, 2) != std::string::npos) ||
      (gpcc::string::StartsWith(name, "./")) ||
      (gpcc::string::EndsWith(name, "/.")) ||
      (gpcc::string::StartsWith(name, "../")) ||
      (gpcc::string::EndsWith(name, "/..")) ||
      (name.find("/./", 0, 3) != std::string::npos) ||
      (name.find("/../", 0, 4) != std::string::npos))
  {
    throw InvalidFileNameError(name);
  }
}

/**
 * \brief Checks a filename for compliance with GPCC's rules for portable file names.
 *
 * Use @ref BasicCheckName() if only the basic checks are required.
 *
 * Only the filename is full checked. Any potentially included directory names are checked for compliance
 * with the basic rules only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws InvalidFileNameError   The name violates the rules for portable file names required by GPCC
 *                                ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws std::bad_alloc         Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param name
 * Filename (incl. path) that shall be checked.
 */
void FileStorage::FullCheckFileName(std::string const & name) const
{
  if (!file_systems::linux_fs::internal::CheckFileName(name, true, true))
  {
    throw InvalidFileNameError(name);
  }
}

/**
 * \brief Checks a directory name for compliance with GPCC's rules for portable file/directory names.
 *
 * Use @ref BasicCheckName() if only the basic checks are required.
 *
 * Only the directory name is full checked. Any potentially included names of parent directories are checked
 * for compliance with the basic rules only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws InvalidFileNameError   The name violates the rules for portable file names required by GPCC
 *                                ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws std::bad_alloc         Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param name
 * Directory name (incl. path) that shall be checked.
 */
void FileStorage::FullCheckDirectoryName(std::string const & name) const
{
  if (!file_systems::linux_fs::internal::CheckDirectoryName(name, true))
  {
    throw InvalidFileNameError(name);
  }
}

} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
