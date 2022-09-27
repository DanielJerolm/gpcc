/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_SECTIONWRITER_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_SECTIONWRITER_HPP_

#include <gpcc/stream/StreamWriterBase.hpp>
#include <memory>

namespace gpcc
{
namespace file_systems
{
namespace eeprom_section_system
{

class EEPROMSectionSystem;

namespace internal
{

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL
 * \class SectionWriter SectionWriter.hpp "src/file_systems/eeprom_section_system/internal/SectionWriter.hpp"
 * \brief Class used to write data to a new section via @ref gpcc::stream::IStreamWriter.
 *
 * An instance of this class is created by class @ref EEPROMSectionSystem if a new section shall
 * be created or if an existing section shall be overwritten. This class offers write access to
 * the new section via @ref gpcc::stream::IStreamWriter and manages all write accesses to the storage.
 * Finally this class cares for proper creation of the section head and it cares for unlocking of
 * the new section at the @ref EEPROMSectionSystem.
 *
 * [IStreamWriter::RemainingCapacity()](@ref gpcc::stream::IStreamWriter::RemainingCapacity()) is not supported.
 *
 * # Internals
 * After construction, the object is ready to receive data written via the @ref gpcc::stream::StreamWriterBase
 * interface. `wrPtr` refers to the next written byte in `spMem`. `spMem` operates as a write buffer. `spMem`
 * holds a complete Section System block, inclusive header. `remainingBytesInCurrentBlock` is used to keep
 * track of the remaining number of data bytes that can be written into `spMem`.
 *
 * If `spMem` is full and at least one more byte shall be written, then `StoreCurrentBlockAndReserveNextBlock()`
 * is invoked before data is written into the buffer via `wrPtr`. `StoreCurrentBlockAndReserveNextBlock()`
 * finishes the Section System block stored in `spMem` (header) and writes it to the storage.
 * Afterwards a new block is allocated and `wrPtr` and `remainingBytesInCurrentBlock` are reset.
 *
 * Single bits are written into `bitData`. If at least 8 bits have been accumulated in `bitData`, then one
 * byte is written into the buffer via `wrPtr`. `StoreCurrentBlockAndReserveNextBlock()` is invoked before writing
 * if the buffer is full. In case of writing bits, `StoreCurrentBlockAndReserveNextBlock()` is also invoked after
 * writing, if the buffer is full after writing 8 bits and if at least one bit is left in `bitData`.
 *
 * If `bitData` contains less than 8 bits and a byte shall be written, then `bitData` is filled up with zeros
 * and `bitData` is written into the buffer. Afterwards the byte is written.
 */
class SectionWriter: public gpcc::stream::StreamWriterBase
{
  public:
    SectionWriter(EEPROMSectionSystem & _ESS,
                  std::string const & _sectionName,
                  uint16_t const _oldSectionHeadIndex,
                  uint16_t const _sectionHeadIndex,
                  uint16_t const _version,
                  uint16_t const _nextBlockIndex,
                  std::unique_ptr<char[]> _spMem);
    SectionWriter(SectionWriter const &) = delete;
    SectionWriter(SectionWriter && other) noexcept;
    ~SectionWriter(void);

    SectionWriter& operator=(SectionWriter const &) = delete;
    SectionWriter& operator=(SectionWriter&&) = delete;

    // --> StreamWriterBase
    bool IsRemainingCapacitySupported(void) const override;
    size_t RemainingCapacity(void) const override;
    uint_fast8_t GetNbOfCachedBits(void) const override;

    void Close(void) override;
    // <--

  protected:
    // --> StreamWriterBase
    void Push(char c) override;
    void Push(void const * pData, size_t n) override;
    void PushBits(uint8_t bits, uint_fast8_t n) override;
    // <--

  private:
    /// Pointer to the @ref EEPROMSectionSystem instance to which this stream writer writes data.
    /** In state @ref States::closed this is nullptr. */
    EEPROMSectionSystem* pESS;

    /// Name of the section that shall be created or overwritten.
    /** In state @ref States::closed this is an empty string. */
    std::string sectionName;

    /// Index of the section header of the section that shall be overwritten.
    /** If a new section is created instead of overwriting an existing section, then this is NOBLOCK.\n
        In state @ref States::closed this is invalid. */
    uint16_t const oldSectionHeadIndex;

    /// Index of the storage block reserved for the new section head.
    /** In state @ref States::closed this is invalid. */
    uint16_t const sectionHeadIndex;

    /// Version for the new section head.
    /** In state @ref States::closed this is invalid. */
    uint16_t const version;

    /// Index of the first data block of the new section.
    /** In state @ref States::closed this is invalid. */
    uint16_t const firstDataBlockIndex;

    /// Index of the storage block reserved for the next data block.
    /** In state @ref States::closed this is invalid. */
    uint16_t nextBlockIndex;

    /// Sequence number for the next stored data block.
    /** In state @ref States::closed this is invalid. */
    uint16_t seqNb;

    /// Pointer to a buffer used to build Section System blocks.
    /** This has been passed to the constructor. The size of the memory block is equal
        to or larger than the block size of the underlying storage.\n
        When the stream is closed, then the referenced memory is released.\n
        In state @ref States::closed this is nullptr. */
    std::unique_ptr<char[]> spMem;

    /// Write-pointer into the memory block referenced by @ref spMem.
    /** In state @ref States::closed this is nullptr. */
    char* wrPtr;

    /// Number of bits written via bit based write methods. The bits are stored in @ref bitData.
    /** This is only valid if state is @ref States::open. */
    uint8_t nbOfBitsWritten;

    /// Bits written via bit based write methods. The number of bits is stored in @ref nbOfBitsWritten.
    /** This is only valid if state is @ref States::open. */
    uint8_t bitData;

    /// Remaining number of bytes that can be written into the buffer referenced by @ref spMem before it is full.
    /** In state @ref States::closed this is invalid. */
    uint16_t remainingBytesInCurrentBlock;

    void PushBitsPlusGap(void);
    void StoreCurrentBlock(uint16_t const blockSize, uint16_t const nextBlock);
    void StoreCurrentBlockAndReserveNextBlock(void);
    void EnterClosedState(void);
    void CloseAnOpenSectionWriter(void);
    void CloseCrashedSectionWriter(void);
};

} // namespace internal
} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_SECTIONWRITER_HPP_
