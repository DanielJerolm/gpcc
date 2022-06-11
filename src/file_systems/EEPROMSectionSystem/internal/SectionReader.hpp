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

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_SECTIONREADER_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_SECTIONREADER_HPP_

#include "gpcc/src/Stream/StreamReaderBase.hpp"
#include <memory>

namespace gpcc
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

class EEPROMSectionSystem;

namespace internal
{

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL
 * \brief Class used to read data from an existing section via @ref gpcc::Stream::IStreamReader.
 *
 * An instance of this class is created by class @ref EEPROMSectionSystem if a section shall
 * be opened for reading. This class offers read access via @ref gpcc::Stream::IStreamReader and
 * manages loading of storage blocks and final unlocking of the read section at the @ref EEPROMSectionSystem.
 *
 * [IStreamReader::RemainingBytes()](@ref gpcc::Stream::IStreamReader::RemainingBytes()) is not supported.
 *
 * # Internals
 * The constructor will load the first data block from the storage. It will be stored in `spMem` and
 * `rdPtr` will be used to read from it.
 *
 * If single bits shall be read, then `rdPtr` will be advanced and 8 bits will be loaded into `bitData`.
 * `nbOfBitsInBitData` will store the number of bits available in `bitData`. If more bits are required,
 * then `rdPtr` will be advanced again and more bits will be loaded into `bitData`. If byte based data
 * shall be read, then any bits left in `bitData` will be discarded.
 *
 * After all bytes have been read from the section, `ReleaseBuffer()` will be invoked to release the
 * buffer referenced by `spMem`. If there are still bits left to be read in `bitData`, then
 * the stream's state will remain States::open. After all bytes and bits have been read, the
 * stream's state will be switched to States::empty.
 *
 * Finally the stream must be closed via `Close()`. `Close()` will release the section lock
 * at the @ref EEPROMSectionSystem instance. After closing, the @ref SectionReader can be released.
 */
class SectionReader: public Stream::StreamReaderBase
{
  public:
    SectionReader(EEPROMSectionSystem & _ESS,
                  std::string const & _sectionName,
                  std::unique_ptr<unsigned char[]> _spMem);
    SectionReader(SectionReader const &) = delete;
    SectionReader(SectionReader && other) noexcept;
    ~SectionReader(void);

    SectionReader& operator=(SectionReader const &) = delete;
    SectionReader& operator=(SectionReader &&) = delete;

    // --> IStreamReader
    bool IsRemainingBytesSupported(void) const override;
    size_t RemainingBytes(void) const override;
    void EnsureAllDataConsumed(RemainingNbOfBits const expectation) const override;
    void Close(void) override;

    void Skip(size_t nBits) override;

    std::string Read_string(void) override;
    std::string Read_line(void) override;
    // <-- IStreamReader

  protected:
    // --> StreamReaderBase
    unsigned char Pop(void) override;
    void Pop(void* p, size_t n) override;
    uint8_t PopBits(uint_fast8_t n) override;
    // <-- StreamReaderBase

  private:
    /// Pointer to the @ref EEPROMSectionSystem instance from which this stream reader reads data.
    /** In state @ref States::closed this is nullptr. */
    EEPROMSectionSystem* pESS;

    /// Name of the section that is read by this stream reader.
    /** In state @ref States::closed this is an empty string. */
    std::string sectionName;

    /// Pointer to a buffer containing the storage block that contains the next byte to be read.
    /** This has been passed to the constructor. The size of the memory block is equal
        to or larger than the block size of the underlying storage.\n
        After all bytes have been read from the section, the referenced memory will be released
        and this pointer will refer to nothing (nullptr). However, there may still be bits left
        to be read in @ref bitData. */
    std::unique_ptr<unsigned char[]> spMem;

    /// Read-pointer into the memory block referenced by @ref spMem.
    /** This points to the next byte to be read.\n
        This is nullptr if @ref spMem refers to nothing (nullptr; no more bytes to be read).\n
        Note that even though this is nullptr, there may still be some bits left in @ref bitData
        to be read. */
    unsigned char const * rdPtr;

    /// Number of bits of the last byte read from the stream that have not yet been read.
    /** The bits are stored in @ref bitData. */
    uint8_t nbOfBitsInBitData;

    /// Bits of the last byte read from the stream that have not yet been read.
    /** The number of bits is stored in @ref nbOfBitsInBitData. */
    uint8_t bitData;

    /// Remaining number of bytes inside the currently loaded storage block.
    /** This is zero if @ref spMem refers to nothing (nullptr).\n
        Note that if this is zero, there may still be some bits left in @ref bitData
        to be read. */
    uint16_t remainingBytesInCurrentBlock;

    void ReleaseBuffer(void) noexcept;
    void LoadNextBlock(void);
};

} // namespace internal
} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_SECTIONREADER_HPP_
