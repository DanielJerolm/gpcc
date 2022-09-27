/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#ifndef STDIOFILEWRITER_HPP_201805182120
#define STDIOFILEWRITER_HPP_201805182120

#include <gpcc/stream/StreamWriterBase.hpp>
#include <cstdio>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {

class FileStorage;

namespace internal     {

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \class StdIOFileWriter StdIOFileWriter.hpp "src/file_systems/linux_fs/internal/StdIOFileWriter.hpp"
 * \brief Class used to create or overwrite a regular file and write to it via @ref gpcc::stream::IStreamWriter.
 *
 * An instance of this class is created by class @ref FileStorage if a new regular file shall be created or if an
 * existing regular file shall be overwritten. This class offers write access to the new file via
 * @ref gpcc::stream::IStreamWriter and manages all write accesses to the storage. All write accesses are done
 * using buffered I/O operations. Finally this class takes care for unlocking of the file at the @ref FileStorage
 * instance.
 *
 * After construction, the object is ready to receive data written via the @ref gpcc::stream::StreamWriterBase
 * interface.
 *
 * [IStreamWriter::RemainingCapacity()](@ref gpcc::stream::IStreamWriter::RemainingCapacity()) is not supported.
 *
 * # Internals
 * Byte-based data is immediately written to the underlying file referenced by `fd`. However, all writes are
 * done using buffered I/O operations offered by "stdio.h".
 *
 * Single bits are written into `bitData`. If at least 8 bits have been accumulated in `bitData`, then one
 * byte will be written into the underlying file referenced by `fd`. If `bitData` contains less than 8 bits and
 * a byte shall be written, then `bitData` will be filled up with zeros and `bitData` will then be written into
 * the file. Afterwards the byte is written to the file.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StdIOFileWriter final : public gpcc::stream::StreamWriterBase
{
  public:
    StdIOFileWriter(std::string const & fileName, bool const overwriteIfExist, FileStorage & _fileStorage, std::string const & _unlockID);
    StdIOFileWriter(StdIOFileWriter const &) = delete;
    StdIOFileWriter(StdIOFileWriter && other) noexcept;
    ~StdIOFileWriter(void);

    StdIOFileWriter& operator=(StdIOFileWriter const &) = delete;
    StdIOFileWriter& operator=(StdIOFileWriter&&) = delete;

    // <-- IStreamWriter
    bool IsRemainingCapacitySupported(void) const override;
    size_t RemainingCapacity(void) const override;
    uint_fast8_t GetNbOfCachedBits(void) const override;

    void Close(void) override;
    // --> IStreamWriter

  protected:
    // <-- StreamWriterBase
    void Push(char c) override;
    void Push(void const * pData, size_t n) override;
    void PushBits(uint8_t bits, uint_fast8_t n) override;
    // --> StreamWriterBase

  private:
    /// Reference to the @ref FileStorage instance which created this @ref StdIOFileWriter.
    FileStorage & fileStorage;

    /// String required to unlock the file at the @ref FileStorage instance when the file is closed.
    /** In state @ref States::closed this is an empty string. */
    std::string unlockID;

    /// File descriptor of the file that is created or overwritten.
    /** This is only valid if state is not @ref States::closed. */
    FILE* fd;

    /// Number of bits written via bit based write methods. The bits are stored in @ref bitData.
    /** This is only valid if state is @ref States::open. */
    uint8_t nbOfBitsWritten;

    /// Bits written via bit based write methods. The number of bits is stored in @ref nbOfBitsWritten.
    /** This is only valid if state is @ref States::open. \n
        This is filled with bits starting at the LSB. */
    uint8_t bitData;

    void PushBitsPlusGap(void);

    void ThrowIOError(char const * const pDescr, int const copyOfErrno) const;

    void CloseFile(void);
    void CloseFileNoThrow(void) noexcept;
    void FlushBitsAndClose(void);
};

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // STDIOFILEWRITER_HPP_201805182120
#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
