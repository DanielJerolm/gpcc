/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef EXCEPTIONS_HPP_201805042128
#define EXCEPTIONS_HPP_201805042128

#include <stdexcept>
#include <string>

namespace gpcc         {
namespace file_systems {

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a filename, or directory name, or part of it is invalid.
 *
 * Filenames (and directory names) can be invalid for two reasons:
 * 1. The filename violates the rules for filenames required by the underlying file system.
 * 2. The filename violates the rules for portable filenames required by GPCC (rules are listed below).
 *
 * The rules required by GPCC for _file creation_, _file rename_, _directory creation_ and _directory rename_ operations are:
 * - Only characters 'A'-'Z', 'a'-'z', '0'-'9', '_', '-', '.', and ' ' are allowed.\n
 *   Note: ' ' is allowed but should be avoided.
 * - No leading ' '
 * - No trailing ' '
 * - No double ' '
 * - No trailing '.'
 * - No leading '-'
 *
 * Additional rules required by GPCC _for any operation_:
 * - No leading '/'
 * - No trailing '/'
 * - No double '/'
 * - No "." or ".." as file or directory name
 */
class InvalidFileNameError : public std::logic_error
{
  public:
    inline InvalidFileNameError(std::string const & fileName) : std::logic_error("Filename (\"" + fileName + "\") does not meet the naming rules.") {};
    virtual ~InvalidFileNameError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a file that shall be involved in an action (e.g. open) does not exist.
 *
 * This is also thrown, if:
 * - a parent directory of the file is not existing.
 * - a existing directory's pathname is used as a filename.
 */
class NoSuchFileError : public std::runtime_error
{
  public:
    inline NoSuchFileError(std::string const & fileName) : std::runtime_error("File \"" + fileName + "\" is not existing.") {};
    virtual ~NoSuchFileError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a directory that shall be involved in an action (e.g. create a new file) does not exist.
 */
class NoSuchDirectoryError : public std::runtime_error
{
  public:
    inline NoSuchDirectoryError(std::string const & path) : std::runtime_error("Directory \"" + path + "\" is not existing.") {};
    virtual ~NoSuchDirectoryError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a file that shall be created (without overwrite) is already existing.
 *
 * Note:
 * - Files can also be created by rename/move operations.
 * - This exception will also be thrown if a file shall be created, but there is already a directory with
 *   exactly the same name. @ref DirectoryAlreadyExistingError _will not_ be thrown in this case,
 *   because some file systems do not support directories.
 * - This exception will also be thrown if a directory shall be created, but there is already a file with
 *   exactly the same name.
 */
class FileAlreadyExistingError : public std::runtime_error
{
  public:
    inline FileAlreadyExistingError(std::string const & fileName) : std::runtime_error("File \"" + fileName + "\" is already existing.") {};
    virtual ~FileAlreadyExistingError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a directory that shall be created is already existing.
 *
 * Note:
 * - Directories can also be created by rename/move operations.
 */
class DirectoryAlreadyExistingError : public std::runtime_error
{
  public:
    inline DirectoryAlreadyExistingError(std::string const & dirName) : std::runtime_error("Directory \"" + dirName + "\" is already existing.") {};
    virtual ~DirectoryAlreadyExistingError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a directory that shall be deleted is not empty.
 */
class DirectoryNotEmptyError : public std::runtime_error
{
  public:
    inline DirectoryNotEmptyError(std::string const & dirName) : std::runtime_error("Directory \"" + dirName + "\" is not empty.") {};
    virtual ~DirectoryNotEmptyError(void) noexcept = default;

};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a file that shall be involved in an action (e.g. open) is already involved in
 *        another action (e.g. open for writing) and the requested action thus cannot take place.
 *
 * __Example:__\n
 * A file is currently open for reading.\n
 * The file shall be renamed. The rename operation will fail and throw this exception.
 *
 * __If directories are supported:__\n
 * This exception will also occur if the directory containing the file that shall be accessed is involved in a
 * modifying action (e.g. rename). This applies to any parent directory of the file.\n
 * Example:\n
 * A directory is about to be renamed.\n
 * At the same time, a file in the directory, or in a sub-directory, shall be opened for reading. The open-operation
 * will fail and throw this exception.
 */
class FileAlreadyAccessedError : public std::runtime_error
{
  public:
    inline FileAlreadyAccessedError(std::string const & fileName) : std::runtime_error("File \"" + fileName + "\" (or parent directory) is currently accessed.") {};
    virtual ~FileAlreadyAccessedError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a directory that shall be involved in an action (e.g. rename) is already involved in
 *        another action (e.g. deletion) and the requested action thus cannot take place.
 *
 * Note that the "other action" may also be executed on...
 * - a parent directory
 * - a sub-directory
 * - a file in the directory
 * - a file in a sub-directory
 *
 * At least one of the colliding actions is always a modifying action.
 */
class DirectoryAlreadyAccessedError : public std::runtime_error
{
  public:
    inline DirectoryAlreadyAccessedError(std::string const & dirName) : std::runtime_error("Directory \"" + dirName + "\" (or its content or its parent directory) is currently accessed.") {};
    virtual ~DirectoryAlreadyAccessedError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if a file that shall be involved in an action (e.g. open) is not a regular file.
 */
class NotARegularFileError : public std::logic_error
{
  public:
    inline NotARegularFileError(std::string const & fileName) : std::logic_error("File \"" + fileName + "\" is not a regular file.\n"\
                                                                                 "The requested operation does not support special files.") {};
    virtual ~NotARegularFileError(void) noexcept = default;
};

/**
 * \ingroup GPCC_FILESYSTEMS_EXCEPTIONS
 * \brief Exception thrown if there is not enough free space in the underlying storage device available to perform
 *        the requested action.
 *
 * On some platforms, this may also be thrown if the user's disk quota is exhausted.
 */
class InsufficientSpaceError : public std::runtime_error
{
  public:
    inline InsufficientSpaceError(void) : std::runtime_error("Insufficient free space in underlying storage device.") {};
    virtual ~InsufficientSpaceError(void) noexcept = default;
};

} // namespace file_systems
} // namespace gpcc

#endif // EXCEPTIONS_HPP_201805042128
