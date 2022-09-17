/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IFILEANDDIRECTORYSTORAGE_HPP_201808041954
#define IFILEANDDIRECTORYSTORAGE_HPP_201808041954

#include <gpcc/file_systems/IFileStorage.hpp>

namespace gpcc         {
namespace file_systems {

/**
 * \ingroup GPCC_FILESYSTEMS
 * \brief Interface for accessing files and directories in a storage location.
 *
 * This interface extends the @ref IFileStorage interface for file access by methods for directory manipulation.
 *
 * This interface offers the following operations on directories:
 * - creation
 * - (recursive) deletion
 * - rename/move
 * - test for existence of a directory
 * - enumeration of files in a specific directory
 * - enumeration of sub-directories in a specific directory
 *
 * # Restrictions on directory names
 * Please refer to @ref GPCC_FILESYSTEMS for details.
 *
 * # Links
 * Please refer to the documentation of the methods offered by this interface. If not otherwise stated, then the
 * methods will dereference links which may be part of path and directory/file names.
 *
 * Please also refer to the documentation of the class which realizes this interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 *
 * __Exception safety__ and __Thread cancellation safety:__\n
 * All methods offer at least the strong guarantee in terms of exception safety and thread cancellation safety.
 *
 * Depending on the underlying file system, some implementations may switch to a "defect state" and need a re-mount
 * or repair in case of certain error conditions or in some cases of deferred thread cancellation.
 *
 * In general, deferred thread cancellation should be avoided if possible.
 */
class IFileAndDirectoryStorage : public IFileStorage
{
  public:
    virtual bool IsDirectoryExisting(std::string const & name) = 0;
    virtual void CreateDirectory(std::string const & name) = 0;
    virtual void DeleteDirectoryContent(std::string const & name) = 0;
    virtual void DeleteDirectory(std::string const & name) = 0;
    virtual void RenameDirectory(std::string const & currName, std::string const & newName) = 0;
    virtual std::list<std::string> EnumerateSubDirectories(std::string const & dir) = 0;
    virtual std::list<std::string> EnumerateFiles(std::string const & dir) = 0;

  protected:
    IFileAndDirectoryStorage(void) = default;
    IFileAndDirectoryStorage(IFileAndDirectoryStorage const &) = default;
    IFileAndDirectoryStorage(IFileAndDirectoryStorage &&) = default;
    virtual ~IFileAndDirectoryStorage(void) = default;

    IFileAndDirectoryStorage& operator=(IFileAndDirectoryStorage const &) = delete;
    IFileAndDirectoryStorage& operator=(IFileAndDirectoryStorage &&) = delete;
};

/**
 * \fn IFileAndDirectoryStorage::IsDirectoryExisting
 * \brief Checks if a directory is existing or not.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws InvalidFileNameError   Filename violates rules of underlying file system
 *                                ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Path and name of the directory whose existence shall be checked.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * An empty string will test existence of the base directory.\n
 * Links contained in the path will be dereferenced.\n
 * If this refers to a link, then it will be dereferenced, too.
 *
 * \return
 * true  = directory exists\n
 * false = directory does not exist or `name` refers to a file.
 */

/**
 * \fn IFileAndDirectoryStorage::CreateDirectory
 * \brief Creates a new empty directory.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DirectoryAlreadyAccessedError   The directory or a parent directory is already accessed
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *
 * \throws DirectoryAlreadyExistingError   A directory with the same name is already existing
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyExistingError)).
 *
 * \throws FileAlreadyExistingError        A file with exactly the same name is already existing
 *                                         ([details](@ref gpcc::file_systems::FileAlreadyExistingError)).
 *
 * \throws InsufficientSpaceError          Not enough free space available in underlying storage ([details](@ref gpcc::file_systems::InsufficientSpaceError)).
 *
 * \throws InvalidFileNameError            The name for the new directory violates the rules of the underlying file system,
 *                                         or the directory name is not portable ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError            Directory where the new directory shall be created is not existing
 *                                         ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::bad_alloc                  Out of memory.
 *
 * \throws std::system_error               Creating the directory has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Path and name for the new directory.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * Links contained in the path will be dereferenced.
 */

/**
 * \fn IFileAndDirectoryStorage::DeleteDirectoryContent
 * \brief Deletes the content of a directory (all files and sub-directories).\n
 *        The directory itself will not be deleted.
 *
 * \post   The directory is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Not all files/sub-directories may have been deleted.
 *
 * \throws DirectoryAlreadyAccessedError   The directory, a parent directory, a file, a sub-directory, or a
 *                                         file in a sub-directory is already accessed
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *
 * \throws InvalidFileNameError            Directory name violates the rules of the underlying file system
 *                                         ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError            The directory referenced by `name` is not existing or it is a file
 *                                         ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::bad_alloc                  Out of memory.
 *
 * \throws std::system_error               Operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Not all files/sub-directories may have been deleted.
 *
 * - - -
 *
 * \param name
 * Path and name of the directory whose content shall be deleted.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * __An empty string will delete anything inside the base directory.__\n
 * Links contained in the path will be dereferenced.\n
 * Any symbolic links contained in the referenced directory or in a sub-directory will not be dereferenced. Instead the
 * symbolic link will be deleted, but the file or directory referenced by the link will not be deleted.
 */

/**
 * \fn IFileAndDirectoryStorage::DeleteDirectory
 * \brief Deletes a directory.
 *
 * \pre   The directory must be empty. Use [DeleteDirectoryContent()](@ref gpcc::file_systems::IFileAndDirectoryStorage::DeleteDirectoryContent) to
 *        accomplish this, if necessary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DirectoryAlreadyAccessedError   The directory, a parent directory, a file, a sub-directory, or a
 *                                         file in a sub-directory is already accessed
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *
 * \throws DirectoryNotEmptyError          The directory referenced by `name` is not empty
 *                                         ([details](@ref gpcc::file_systems::DirectoryNotEmptyError)).
 *
 * \throws InvalidFileNameError            Directory name violates the rules of the underlying file system
 *                                         ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError            The directory referenced by `name` is not existing or it is a file
 *                                         ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::system_error               Operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Path and name of the directory that shall be deleted.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * Links contained in the path will be dereferenced.\n
 * If this refers to a symbolic link which refers to a directory, then the symbolic link will be deleted and
 * the directory referenced by the link will not be deleted.
 */

/**
 * \fn IFileAndDirectoryStorage::RenameDirectory
 * \brief Renames a directory and/or changes its location.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DirectoryAlreadyAccessedError   The directory (`newName` or `currName`), a parent directory, a file in any of the two,
 *                                         a sub-directory in any of the two, or a file in a sub-directory in any of the two is
 *                                         already accessed ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *
 * \throws DirectoryAlreadyExistingError   Directory `newName` is already existing ([details](@ref gpcc::file_systems::DirectoryAlreadyExistingError)).
 *
 * \throws FileAlreadyExistingError        `newName` is already existing and it is a file ([details](@ref gpcc::file_systems::FileAlreadyExistingError)).
 *
 * \throws InsufficientSpaceError          Not enough free space available in underlying storage ([details](@ref gpcc::file_systems::InsufficientSpaceError)).
 *
 * \throws InvalidFileNameError            Any of the directory names violate the rules of the underlying file system, or `newName` is not
 *                                         portable ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError            Either `currName` is not existing, or a directory component in `newName` is not existing
 *                                         ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::bad_alloc                  Out of memory.
 *
 * \throws std::system_error               Rename operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param currName
 * Path and name of the directory that shall be renamed.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * If this refers to a a link, then the link will be renamed. The directory referenced by the link will not be renamed.\n
 * If this refers to a file, then `NoSuchDirectoryError` will be thrown.
 *
 * \param newName
 * New path and name for the directory.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * If this is the same as `currName`, then this method will do nothing.\n
 * If this refers to a different directory than `currName`, then the directory will be moved to that directory.
 */

/**
 * \fn IFileAndDirectoryStorage::EnumerateSubDirectories
 * \brief Enumerates all sub-directories in a given directory.
 *
 * Enumeration is not recursive.\n
 * Both empty and not-empty directories are enumerated.\n
 * Files are not enumerated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DirectoryAlreadyAccessedError   The directory, a parent directory, a file, a sub-directory, or a
 *                                         file in a sub-directory is already accessed
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *
 * \throws InvalidFileNameError            `dir` violates rules of underlying file system
 *                                         ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError            The directory `dir` is not existing or it is a file ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::bad_alloc                  Out of memory.
 *
 * \throws std::system_error               Operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param dir
 * Path and name of the directory whose sub-directories shall be enumerated.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * Links contained in the path will be dereferenced.
 *
 * \return
 * List containing the names of all currently existing sub-directories in directory `dir`, which can be accessed through
 * this interface.\n
 * The names in the list do not include a path. The names are relative to `dir`.\n
 * The list is sorted alphabetically and by upper/lower-case.
 */

/**
 * \fn IFileAndDirectoryStorage::EnumerateFiles
 * \brief Enumerates all files in a given directory.
 *
 * Enumeration is not recursive.\n
 * Both regular and special files are enumerated.\n
 * Directories are not enumerated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DirectoryAlreadyAccessedError   The directory, a parent directory, a file, a sub-directory, or a
 *                                         file in a sub-directory is already accessed
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *
 * \throws InvalidFileNameError            `dir` violates rules of underlying file system
 *                                         ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError            The directory `dir` is not existing or it is a file ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::bad_alloc                  Out of memory.
 *
 * \throws std::system_error               Operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param dir
 * Path and name of the directory whose files shall be enumerated.\n
 * This is treated relative to the base directory configured at the class providing this interface.\n
 * Links contained in the path will be dereferenced.
 *
 * \return
 * List containing the names of all currently existing files in directory `dir`, which can be accessed through
 * this interface.\n
 * The names in the list do not include a path. The names are relative to `dir`.\n
 * The list is sorted alphabetically and by upper/lower-case.
 */

} // namespace file_systems
} // namespace gpcc

#endif // IFILEANDDIRECTORYSTORAGE_HPP_201808041954
