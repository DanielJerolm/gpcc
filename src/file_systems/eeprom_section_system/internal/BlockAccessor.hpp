/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_BLOCKACCESSOR_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_BLOCKACCESSOR_HPP_

#include "EEPROMSectionSystemInternals.hpp"
#include <cstddef>
#include <cstdint>

namespace gpcc
{

namespace stdif
{
  class IRandomAccessStorage;
}

namespace file_systems
{
namespace eeprom_section_system
{
namespace internal
{

/**
 * \ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL
 * \class BlockAccessor BlockAccessor.hpp "src/file_systems/eeprom_section_system/internal/BlockAccessor.hpp"
 * \brief Helper for [EEPROMSectionSystem](@ref eeprom_section_system::EEPROMSectionSystem): Divides the storage into blocks and offers read- and write-access.
 *
 * This class manages the storage an @ref EEPROMSectionSystem is working on:
 * - partition into blocks
 * - read/write access to blocks
 * - CRC generation and CRC check
 * - Basic checks on section system control data after reading from storage and before writing to storage
 *
 * Constraints:
 * - Block size must be equal to or less than the storage's page size
 * - The storage's page size must be a whole numbered multiple of the block size
 * - The block size and the number of blocks must meet the following:
 *   - @ref EEPROMSectionSystem::MinimumBlockSize
 *   - @ref EEPROMSectionSystem::MinimumNbOfBlocks
 *   - @ref EEPROMSectionSystem::MaximumNbOfBlocks
 */
class BlockAccessor
{
  public:
    BlockAccessor(stdif::IRandomAccessStorage & _storage, uint32_t const _startAddressInStorage, size_t const _sizeInStorage);
    BlockAccessor(BlockAccessor const &) = delete;
    BlockAccessor(BlockAccessor &&) = delete;
    ~BlockAccessor(void) = default;

    BlockAccessor& operator=(BlockAccessor const &) = delete;
    BlockAccessor& operator=(BlockAccessor &&) = delete;

    size_t GetSizeInStorage(void) const;
    size_t GetPageSize(void) const;

    void SetBlockSize(uint16_t const _blockSize);
    uint16_t GetBlockSize(void) const;
    uint16_t GetnBlocks(void) const;
    size_t GetMaxSectionNameLength(void) const;

    uint16_t LoadFields_type_sectionNameHash(uint16_t const blockIndex) const;
    uint8_t LoadField_type(uint16_t const blockIndex) const;
    uint32_t LoadField_totalNbOfWrites(uint16_t const blockIndex) const;
    uint16_t LoadField_nextBlock(uint16_t const blockIndex) const;

    void LoadBlock(uint16_t const blockIndex, void* const pBuffer, size_t const maxLength) const;
    void StoreBlock(uint16_t const blockIndex, void* const pBuffer, void* const pAuxBuf, bool const recoverEndian);

  private:
    /// Storage on which the @ref EEPROMSectionSystem is working.
    stdif::IRandomAccessStorage & storage;

    /// Start address inside @ref storage where the @ref EEPROMSectionSystem resides.
    /** This is aligned to a page boundary of @ref storage. */
    uint32_t const startAddressInStorage;

    /// Number of bytes granted to the @ref EEPROMSectionSystem, starting at @ref startAddressInStorage.
    /** This is a whole numbered multiple of the page size reported by @ref storage. */
    size_t const sizeInStorage;

    /// Size of the blocks in @ref storage in bytes. Zero = undefined.
    uint16_t blockSize;

    /// Number of blocks in @ref storage.
    uint16_t nBlocks;


    uint32_t CalcBlockStartAddress(uint16_t const blockIndex) const;

    void CalcCRC(void* const pData, size_t const n) const noexcept;
    bool CheckCRC(const void* const pData, size_t const n) const noexcept;

    void SwapEndian(uint16_t& u16) const noexcept;
    void SwapEndian(uint32_t& u32) const noexcept;
    void SwapEndian(CommonBlockHead_t* const pData) const noexcept;
    void SwapEndian(SectionSystemInfoBlock_t* const pData) const noexcept;
    void SwapEndian(SectionHeadBlock_t* const pData) const noexcept;
    void SwapEndian(DataBlock_t* const pData) const noexcept;
    bool SwapEndian(void* const pBlock) const noexcept;
};

} // namespace internal
} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_BLOCKACCESSOR_HPP_
