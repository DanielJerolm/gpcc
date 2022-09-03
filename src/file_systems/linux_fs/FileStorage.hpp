/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#ifndef FILESTORAGE_HPP_201805102328
#define FILESTORAGE_HPP_201805102328

#include "gpcc/src/file_systems/IFileAndDirectoryStorage.hpp"
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/resource_management/objects/HierarchicNamedRWLock.hpp>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {

namespace internal
{
  class StdIOFileReader;
  class StdIOFileWriter;
}

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS
 * \brief Class offering access to files stored on a Linux-based platform.
 *
 * This class implements the @ref IFileAndDirectoryStorage (and therefore also the @ref IFileStorage) interface and
 * allows to create, open, read, write, delete, and rename files on a Linux-based platform. The access is limited to
 * the content of a specific directory that is passed to the constructor of class @ref FileStorage. Only files and
 * files referenced by links located in the referenced directory or a sub-directory can be accessed through the
 * @ref FileStorage instance.
 *
 * Files can be read and written via the [IStreamReader](@ref gpcc::Stream::IStreamReader) and
 * [IStreamWriter](@ref gpcc::Stream::IStreamWriter) interfaces.
 * [IStreamReader::RemainingBytes()](@ref gpcc::Stream::IStreamReader::RemainingBytes) and
 * [IStreamWriter::RemainingCapacity()](@ref gpcc::Stream::IStreamWriter::RemainingCapacity) are not supported.
 *
 * __Note:__\n
 * The methods of this interface dereference links. Please refer to the documentation of each method for details
 * about the behavior regarding links.\n
 * _Links allow to access files and directories outside the directory passed to this class' constructor._\n
 * _This breaks the "sandbox" for file storage provided by this class._
 *
 * # Example
 * In the following example, a @ref FileStorage instance is created that allows to access
 * files located in "/home/someone/demo".
 *
 * ~~~{.cpp}
 * std::unique_ptr<FileStorage> fs(new FileStorage("/home/someone/demo");
 *
 * // Let's create a file.
 * // (The absolute path will be: /home/someone/demo/Test.txt")
 * auto spISW = fs->Create("Test.txt", false);
 * *spISW << std::string("Hello!");
 * spISW->Close();
 * ~~~
 *
 * # Locking
 * Linux already offers access arbitration for files and directories, but this class puts a strict access arbitration
 * on top of that:
 * - Multiple readers can access the same file or directory at the same time.
 * - Only one writer can access the same file or directory at the same time.
 * - If a directory shall be accessed by a writer, then there must be no readers or writers accessing files or
 *   sub-directories of the directory that shall be accessed.
 *
 * The access arbitration implemented by this class applies to users of this class only. Any access done
 * by other processes or other users using a bypass to this class (e.g. fopen()) are not affected by the access
 * arbitration.
 *
 * The access arbitration is present, because it is required by other implementations of the @ref IFileStorage interface
 * and because all implementations of the @ref IFileStorage interface shall implement the same behavior.
 *
 * # Portable file names
 * This class strictly requires portable directory and file names for _file creation_, _directory creation_, and _rename_
 * operations, though Linux itself is quite tolerant regarding file and directory names. See @ref GPCC_FILESYSTEMS for details.
 *
 * Operations working on _existing files or directories_ (e.g. _open_) do not require portable file names.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class FileStorage final: public IFileAndDirectoryStorage
{
    friend class internal::StdIOFileReader;
    friend class internal::StdIOFileWriter;

  public:
    FileStorage(std::string const & _baseDir);
    FileStorage(FileStorage const &) = delete;
    FileStorage(FileStorage &&) = delete;
    ~FileStorage(void);

    FileStorage& operator=(FileStorage const &) = delete;
    FileStorage& operator=(FileStorage &&) = delete;

    // <-- IFileStorage
    std::unique_ptr<Stream::IStreamReader> Open(std::string const & name) override;
    std::unique_ptr<Stream::IStreamWriter> Create(std::string const & name, bool const overwriteIfExisting) override;
    void Delete(std::string const & name) override;
    void Rename(std::string const & currName, std::string const & newName) override;

    std::list<std::string> Enumerate(void) const override;
    size_t DetermineSize(std::string const & name, size_t * const pTotalSize) const override;
    size_t GetFreeSpace(void) const override;
    // --> IFileStorage

    // <-- IFileAndDirectoryStorage
    bool IsDirectoryExisting(std::string const & name) override;
    void CreateDirectory(std::string const & name) override;
    void DeleteDirectoryContent(std::string const & name) override;
    void DeleteDirectory(std::string const & name) override;
    void RenameDirectory(std::string const & currName, std::string const & newName) override;
    std::list<std::string> EnumerateSubDirectories(std::string const & dir) override;
    std::list<std::string> EnumerateFiles(std::string const & dir) override;
    // --> IFileAndDirectoryStorage

  private:
    /// Base directory.
    /** The content of this directory is visible and accessible through this @ref FileStorage object.\n
        This has a trailing '/'. */
    std::string const baseDir;

    /// Mutex used to make the API thread-safe.
    osal::Mutex mutable mutex;

    /// File/directory lock.
    /** @ref mutex is required.\n
        Any modifying operation requires acquisition of a write-lock on the involved file or directory before
        the operation starts.\n
        Any non-modifying operation requires acquisition of a read-lock on the involved file or directory before
        the operation starts.\n
        __HierarchicNamedRWLock specific:__\n
        Class HierarchicNamedRWLock requires a special separating character in the lock names. Here we use '/'
        as the separating character. This means, that a '/' must be appended to the names of directories and even
        files in order to use them as lock-names. */
    ResourceManagement::Objects::HierarchicNamedRWLock mutable fileLockManager;


    void ReleaseReadLock(std::string const & unlockID);
    void ReleaseWriteLock(std::string const & unlockID);

    void BasicCheckName(std::string const & name) const;
    void FullCheckFileName(std::string const & name) const;
    void FullCheckDirectoryName(std::string const & name) const;
};

} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // FILESTORAGE_HPP_201805102328
#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
