/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_EEPROMSECTIONSYSTEMINTERNALS_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_EEPROMSECTIONSYSTEMINTERNALS_HPP_

#include "gpcc/src/Compiler/definitions.hpp"
#include <cstdint>
#include <cstddef>

namespace gpcc
{
namespace file_systems
{
namespace EEPROMSectionSystem
{
namespace internal
{

/**
 * @ingroup GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL
 * @{
 */

/// Minimum supported block size of the underlying storage in bytes.
static size_t const MinimumBlockSize = 32U;

/// Maximum supported block size of the underlying storage in bytes.
static size_t const MaximumBlockSize = 4096U; // Limited by hamming-distance of CRC-CCITT16

/// Minimum required number of blocks in the underlying storage.
static size_t const MinimumNbOfBlocks = 3U;

/// Maximum supported number of blocks in the underlying storage.
static size_t const MaximumNbOfBlocks = 65535U;

/// Block index referencing to no block.
static uint16_t const NOBLOCK = 0xFFFFU;

/// Enumeration with storage block types.
enum class BlockTypes
{
  sectionSystemInfo = 0,
  freeBlock         = 1,
  sectionHead       = 2,
  sectionData       = 3
};

/// Storage blocks: Common header
typedef PACKED1 struct CommonBlockHead_t
{
  /// Storage block type.
  /** This is a value of the BlockTypes enumeration casted to `uint8_t`. */
  uint8_t type;

  /// Section name hash.
  /** If the block is a section head (type == BlockTypes::sectionHead), then
      this is a hash of the section's name. For all other block types, this is 0x00. */
  uint8_t sectionNameHash;

  /// Number of bytes stored in the block, inclusive header and CRC.
  uint16_t nBytes;

  /// Total number of writes done to the block for wear-leveling purposes.
  uint32_t totalNbOfWrites;

  /// Reference to the next block of a section or to the next block in a list of free blocks.
  /** If there is no next block (end of section, end of list of free blocks,
      or block type BlockTypes::sectionSystemInfo), then this is NOBLOCK. */
  uint16_t nextBlock;

} PACKED2 CommonBlockHead_t; // 10 Bytes

/// Storage blocks: Section-System info block.
typedef PACKED1 struct SectionSystemInfoBlock_t
{
  /// Common header.
  CommonBlockHead_t head;

  /// Section System Version.
  uint16_t sectionSystemVersion;

  /// Storage block size in bytes.
  /** Remember: This is not necessarily the page size of the underlying storage device! */
  uint16_t blockSize;

  /// Number of storage blocks occupied by the section system inside the underlying storage device.
  uint16_t nBlocks;

} PACKED2 SectionSystemInfoBlock_t; // 16 Bytes

/// Storage blocks: Section head block
typedef PACKED1 struct SectionHeadBlock_t
{
  /// Common header.
  CommonBlockHead_t head;

  /// Version of the section head.
  /** Wear-leveling requires that a section head is recreated at a different storage block
      during some operations. In case of a power loss, multiple section heads with valid CRC
      for the same section might exist. The version-value is used in this case to identify
      the latest section head. Be aware of the wrap-around at 0xFFFF. */
  uint16_t version;

} PACKED2 SectionHeadBlock_t; // 12 Bytes

/// Storage blocks: Section data block
typedef PACKED1 struct DataBlock_t
{
  /// Common header.
  CommonBlockHead_t head;

  /// Sequence number.
  /** The data blocks that make up a section must have consecutive sequence numbers. */
  uint16_t seqNb;

} PACKED2 DataBlock_t; // 12 Bytes

uint8_t CalcHash(char const * s) noexcept;

/**
 * @}
 */

} // namespace internal
} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_EEPROMSECTIONSYSTEMINTERNALS_HPP_
