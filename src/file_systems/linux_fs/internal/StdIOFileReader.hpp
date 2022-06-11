/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2020, 2022 Daniel Jerolm

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

#ifndef STDIOFILEREADER_HPP_201805181928
#define STDIOFILEREADER_HPP_201805181928

#include "gpcc/src/Stream/StreamReaderBase.hpp"
#include <cstdio>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {

class FileStorage;

namespace internal     {

/**
 * \ingroup GPCC_FILESYSTEMS_LINUXFS_INTERNAL
 * \brief Class used to read data from a regular file via @ref gpcc::Stream::IStreamReader.
 *
 * An instance of this class is created by class @ref FileStorage if a regular file shall be opened for reading.
 * This class offers read access via @ref gpcc::Stream::IStreamReader and manages all read accesses to the storage.
 * All read accesses are done using buffered I/O operations. Finally this class takes care for unlocking of the
 * opened file at the @ref FileStorage instance.
 * [IStreamReader::RemainingBytes()](@ref gpcc::Stream::IStreamReader::RemainingBytes()) is not supported.
 *
 * After construction, the object is ready to read data from the file via the @ref gpcc::Stream::IStreamWriter
 * interface.
 *
 * # Internals
 * The constructor will open the file for reading. The file is referenced via `fd'.
 *
 * ## Read-ahead
 * This class always reads ahead one byte from the file and keeps it in attribute `nextByte`. This is necessary
 * because the EOF indicator will not be set on the underlying file after reading the last byte from the file,
 * e.g. via `fgetc()`. Instead the EOF indicator will be set after a read-operation (e.g. `fgetc()`) has failed
 * because the file is empty. The read-ahead strategy allows the stream's state to be set to @ref States::empty
 * after reading the last byte.
 *
 * The read-ahead byte is stored in `nextByte`. Read-ahead is done via `ReadAheadNextByte()`. Any method like
 * e.g. @ref Pop() that wants to read one byte from the file shall use `nextByte` first and then invoke
 * `ReadAheadNextByte()` to prepare for the next read operation.
 *
 * ## Reading single bits
 * If single bits shall be read, then 8 bits will be loaded from the file into `bitData`. Of course "load" means
 * that the content of `nextByte` is used and `ReadAheadNextByte()` is invoked afterwards in order to read a new
 * byte from the file into `nextByte`. `nbOfBitsInBitData` stores the number of bits available in `bitData`. If
 * more bits are required during a read operation (@ref PopBits()), then one further byte will be loaded from the
 * file and 8 additional bits will be added to `bitData`. Again "load" means using `nextByte` and invoking
 * `ReadAheadNextByte()` afterwards. If byte based data shall be read, then any bits left in `bitData` will be
 * discarded using `DiscardBits()`.
 *
 * ## Entering States::empty
 * After all bits and bytes have been read from the file, the stream will enter @ref States::empty.
 * `ReadAheadNextByte()` will switch the state to @ref States::empty if the file is empty and `nbOfBitsInBitData`
 * is zero. If there are any bits left, then @ref PopBits() will switch the state to @ref States::empty after all
 * bits have been read. If byte based data shall be read, then `DiscardBits()` will switch the state to
 * @ref States::empty if the file is empty.
 *
 * The table below shows the valid combinations of this class' attributes. During execution of any of the class'
 * methods, other intermediate combinations may occur, but these are the allowed ones after any method has returned.
 *
 * state               | fd state   | nextByte | bitData
 * ------------------- | ---------- | -------- | ------------
 * @ref States::open   | OK         | valid    | none
 * @ref States::open   | OK         | valid    | at least one
 * @ref States::open   | EOF        | invalid  | at least one
 * @ref States::empty  | EOF        | invalid  | none
 * @ref States::closed | closed     | invalid  | invalid
 * @ref States::error  | OK/EOF     | invalid  | invalid
 *
 * ## Closing
 * Upon close, the read-lock held inside the @ref FileStorage instance which created this @ref StdIOFileReader
 * instance will be released.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StdIOFileReader final : public Stream::StreamReaderBase
{
  public:
    StdIOFileReader(std::string const & fileName, FileStorage & _fileStorage, std::string const & _unlockID);
    StdIOFileReader(StdIOFileReader const &) = delete;
    StdIOFileReader(StdIOFileReader && other) noexcept;
    ~StdIOFileReader(void);

    StdIOFileReader& operator=(StdIOFileReader const &) = delete;
    StdIOFileReader& operator=(StdIOFileReader &&) = delete;

    // <-- IStreamReader
    bool IsRemainingBytesSupported(void) const override;
    size_t RemainingBytes(void) const override;
    void EnsureAllDataConsumed(RemainingNbOfBits const expectation) const override;
    void Close(void) override;

    void Skip(size_t nBits) override;

    std::string Read_string(void) override;
    std::string Read_line(void) override;
    // --> IStreamReader

  protected:
    // <-- StreamReaderBase
    unsigned char Pop(void) override;
    void Pop(void* p, size_t n) override;
    uint8_t PopBits(uint_fast8_t n) override;
    // --> StreamReaderBase

  private:
    /// Reference to the @ref FileStorage instance which created this @ref StdIOFileReader.
    FileStorage & fileStorage;

    /// String required to unlock the file at the @ref FileStorage instance when the file is closed.
    /** In state @ref States::closed this is an empty string. */
    std::string unlockID;

    /// File descriptor of the file that is opened and read by this.
    /** This is only valid if state is not @ref States::closed. */
    FILE* fd;

    /// One byte of data read ahead from the file referenced by @ref fd.
    /** This is only valid, if @ref fd is valid and if the EOF flag is not set on @ref fd and
        if state is not @ref States::error. \n
        (`valid = (feof(fd) == 0)`). */
    uint8_t nextByte;

    /// Number of bits from the file that have not yet been read.
    /** The bits are stored in @ref bitData. */
    uint8_t nbOfBitsInBitData;

    /// Bits of the last byte read from the file that have not yet been read.
    /** The number of bits is stored in @ref nbOfBitsInBitData. */
    uint8_t bitData;

    void DiscardBits(void) noexcept;
    void UndoReadAhead(void);
    void ReadAheadNextByte(void);
    void ThrowIOErrorPlusNestedSystemError(char const * const pDescr, int const copyOfErrno) const;
};

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // STDIOFILEREADER_HPP_201805181928
#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
