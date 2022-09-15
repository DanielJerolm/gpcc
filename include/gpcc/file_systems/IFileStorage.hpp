/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IFILESTORAGE_HPP_201805041946
#define IFILESTORAGE_HPP_201805041946

#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <list>
#include <memory>
#include <string>
#include <cstddef>

namespace gpcc         {
namespace file_systems {

/**
 * \ingroup GPCC_FILESYSTEMS
 * \brief Interface for accessing files in a storage location.
 *
 * This interface offers the following operations on files:
 * - open for reading
 * - create new files and write into them
 * - overwrite existing files and write into them
 * - delete files
 * - rename files (incl. move between directories, if directories are supported)
 * - enumerate files
 * - determine file size
 * - determine free space in underlying storage
 *
 * This interface does not offer any functionality for manipulating directories. However, files can be
 * located in directories and accessed through this interface. If directory manipulation is required, then
 * check out if the underlying implementation of this interface also offers the @ref IFileAndDirectoryStorage
 * interface, which is derived from this interface and which offers functionality for directory manipulation.
 *
 * # Restrictions on file names
 * Please refer to @ref GPCC_FILESYSTEMS for details.
 *
 * # Special files and links
 * Please refer to the documentation of the methods offered by this interface. If not otherwise stated, then the
 * methods will work with special files and they will dereference links which may be part of path and file names.
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
 * Depending on the underlying file system, some implementations (e.g. GPCC's EEPROM Section System) may
 * switch to a "defect state" and need a re-mount or repair in case of certain error conditions or in some
 * cases of deferred thread cancellation.
 *
 * In general, deferred thread cancellation should be avoided if possible.
 */
class IFileStorage
{
  public:
    virtual std::unique_ptr<stream::IStreamReader> Open(std::string const & name) = 0;
    virtual std::unique_ptr<stream::IStreamWriter> Create(std::string const & name, bool const overwriteIfExisting) = 0;
    virtual void Delete(std::string const & name) = 0;
    virtual void Rename(std::string const & currName, std::string const & newName) = 0;

    virtual std::list<std::string> Enumerate(void) const = 0;
    virtual size_t DetermineSize(std::string const & name, size_t * const pTotalSize) const = 0;
    virtual size_t GetFreeSpace(void) const = 0;

  protected:
    IFileStorage(void) = default;
    IFileStorage(IFileStorage const &) = default;
    IFileStorage(IFileStorage &&) = default;
    virtual ~IFileStorage(void) = default;

    IFileStorage& operator=(IFileStorage const &) = delete;
    IFileStorage& operator=(IFileStorage &&) = delete;
};

/**
 * \fn IFileStorage::Open
 * \brief Opens an existing file for reading.
 *
 * This is intended to be used with regular files only. If buffered I/O is available on the platform,
 * then buffered I/O will be used. If this is invoked on non-regular files, then this may throw an exception
 * on some platforms, because buffered I/O makes no sense with non-regular files.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws FileAlreadyAccessedError   File (or parent directory) is already accessed
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyAccessedError)).
 *
 * \throws InvalidFileNameError       Filename violates rules of underlying file system
 *                                    ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws IOError                    Reading from the file has failed ([details](@ref gpcc::stream::IOError)).
 *
 * \throws NoSuchFileError            File is not existing ([details](@ref gpcc::file_systems::NoSuchFileError)).
 *
 * \throws NotARegularFileError       File is not a regular file ([details](@ref gpcc::file_systems::NotARegularFileError)).
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws std::system_error          Opening the file has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name of the file that shall be opened for reading.\n
 * If the underlying file system supports directories, then the filename may include a path relative to the
 * base directory configured at the class providing this interface.\n
 * Links included in the path/filename will be dereferenced.
 *
 * \return
 * A std::unique_ptr to an @ref gpcc::stream::IStreamReader interface for reading from the opened file.\n
 * The calling function must finally close the @ref gpcc::stream::IStreamReader and release it.
 */

/**
 * \fn IFileStorage::Create
 * \brief Creates a new (regular) file or overwrites an existing file and opens it for writing.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws FileAlreadyAccessedError   File (or parent directory) is already accessed
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyAccessedError)).
 *
 * \throws FileAlreadyExistingError   File is already existing and overwriting is not configured, or a directory with
 *                                    exactly the same name is already existing
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyExistingError)).
 *
 * \throws InsufficientSpaceError     Not enough free space available in underlying storage ([details](@ref gpcc::file_systems::InsufficientSpaceError)).
 *
 * \throws InvalidFileNameError       Filename violates rules of underlying file system, or the file name is not portable
 *                                    ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError       Directory where the file shall be created is not existing
 *                                    ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws std::system_error          Creating the file has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name of the file that shall be created or overwritten and opened for writing.\n
 * If the underlying file system supports directories, then the filename may include a path relative to the
 * base directory configured at the class providing this interface.
 *
 * \param overwriteIfExisting
 * This determines the behavior if a file with the given name is already existing:
 * - `true` = overwrite
 * - `false` = do not overwrite (will throw [FileAlreadyExistingError](@ref gpcc::file_systems::FileAlreadyExistingError)).
 *
 * If the file is already existing and if the file is a link, then the file referenced by the link will be overwritten.
 * The link itself will not be modified.
 *
 * \return
 * A std::unique_ptr to an @ref gpcc::stream::IStreamWriter for writing to the new file.\n
 * The calling function must finally close the @ref gpcc::stream::IStreamWriter and release it.
 */

/**
 * \fn IFileStorage::Delete
 * \brief Deletes a file.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws FileAlreadyAccessedError   File (or parent directory) is already accessed
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyAccessedError)).
 *
 * \throws InvalidFileNameError       Filename violates rules of underlying file system
 *                                    ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchFileError            File is not existing ([details](@ref gpcc::file_systems::NoSuchFileError)).
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws std::system_error          Deleting the file has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name of the file that shall be deleted.\n
 * If the underlying file system supports directories, then the filename may include a path relative to the
 * base directory configured at the class providing this interface.\n
 * If the file is a symbolic link, then the link will be removed. The file referenced by the symbolic link will not be removed.\n
 * If the file is a hard link, then the link will be removed. The file referenced by the hard link will be removed if there are
 * no other (hard) links to the file.\n
 * If `name` refers to a directory, then `NoSuchFileError` will be thrown.
 */

/**
 * \fn IFileStorage::Rename
 * \brief Renames a file and/or changes its location.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws FileAlreadyAccessedError   Any of the two files (or a parent directory) is already accessed
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyAccessedError)).
 *
 * \throws FileAlreadyExistingError   File `newName` is already existing ([details](@ref gpcc::file_systems::FileAlreadyExistingError)).
 *
 * \throws InsufficientSpaceError     Not enough free space available in underlying storage ([details](@ref gpcc::file_systems::InsufficientSpaceError)).
 *
 * \throws InvalidFileNameError       Any of the file names violate the rules of the underlying file system, or `newName` is not
 *                                    portable ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchDirectoryError       A directory component in `newName` is not existing ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws NoSuchFileError            File `currName` is not existing ([details](@ref gpcc::file_systems::NoSuchFileError)).
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws std::system_error          Rename operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param currName
 * Name of the file that shall be renamed.\n
 * If the underlying file system supports directories, then the filename may include a path relative to the
 * base directory configured at the class providing this interface.\n
 * If the file is a link, then the link will be renamed. The file referenced by the link will not be renamed.\n
 * If this refers to a directory, then `NoSuchFileError` will be thrown.
 *
 * \param newName
 * New name for the file.\n
 * If the underlying file system supports directories, then the filename may include a path relative to the
 * base directory configured at the class providing this interface.\n
 * If this is the same as `currName`, then this method will do nothing.\n
 * If this refers to a different directory than `currName`, then the file will be moved to that directory.
 */

/**
 * \fn IFileStorage::Enumerate
 * \brief Enumerates all files accessible through this interface.
 *
 * Note:
 * - This only enumerates files.
 * - Directories (if supported by the underlying file system) are not enumerated, but the content of directories
 *   is enumerated recursively.
 * - All types of files (regular and special) are enumerated, though special files might not be fully
 *   accessible through this interface.
 * - This follows links.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DirectoryAlreadyAccessedError   The base directory is already accessed
 *                                         ([details](@ref gpcc::file_systems::DirectoryAlreadyAccessedError)).
 *                                         This can only occur if directories are supported.
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
 * \return
 * List containing path and name of all currently existing files, which can be accessed through this interface.\n
 * If the underlying file system supports directories, then the filenames in the list include the path to the
 * file relative to the base directory configured at the class providing this interface.\n
 * The list is sorted alphabetically and by upper/lower-case.
 */

/**
 * \fn IFileStorage::DetermineSize
 * \brief Determines the size of a file.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws FileAlreadyAccessedError   File (or parent directory) is already accessed
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyAccessedError)).
 *
 * \throws InvalidFileNameError       File name violates rules of underlying file system
 *                                    ([details](@ref gpcc::file_systems::InvalidFileNameError)).
 *
 * \throws NoSuchFileError            File is not existing ([details](@ref gpcc::file_systems::NoSuchFileError)).
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws std::system_error          Operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param name
 * Name of the file.\n
 * If the underlying file system supports directories, then the filename may include a path relative to the
 * base directory configured at the class providing this interface.\n
 * If the file is a link, then it will be dereferenced.
 *
 * \param pTotalSize
 * Pointer to a memory location into which the total number of bytes occupied by the file inside the underlying
 * storage shall be written. This includes storage occupied by management data. Thus this value is equal to or
 * larger than the return value of this method. This value is not precise on all implementations of this interface.\n
 * nullptr, if the caller is not interested in this value.
 *
 * \return
 * Number of data bytes stored inside the file.\n
 * This value is precise in any implementation of this interface.
 */

/**
 * \fn IFileStorage::GetFreeSpace
 * \brief Retrieves the amount of free space available for data in the underlying storage device.
 *
 * If the underlying class is @ref gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem, then this method
 * retrieves the amount of free space available for user data, as it would be _after creation of a new file_.
 *
 * If the underlying class is @ref gpcc::file_systems::linux_fs::FileStorage, then this function retrieves the amount
 * of free space available for _both user data and file system management data_.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::system_error   Operation has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Amount of free space in bytes.\n
 * Note:\n
 * On some platforms, the returned value might be the user's disk quota, which may be less than the actual
 * amount of free space.
 */

} // namespace file_systems
} // namespace gpcc

#endif // IFILESTORAGE_HPP_201805041946
