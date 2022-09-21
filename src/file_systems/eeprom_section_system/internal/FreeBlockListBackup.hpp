/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_FREEBLOCKLISTBACKUP_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_FREEBLOCKLISTBACKUP_HPP_

#include <cstdint>

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
 * \brief Snapshot of the state of the free-block lists of an @ref EEPROMSectionSystem instance.
 *
 * _Implicit capabilities: copy-construction, copy-assignment, move-construction, move-assignment_
 *
 * This is internally used by class @ref EEPROMSectionSystem to allow for roll-back of free-block
 * allocations in case of an error.
 */
class FreeBlockListBackup
{
    friend class gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem;

  private:
    /// Backup of `EEPROMSectionSystem::nFreeBlocks`.
    uint16_t const nFreeBlocks;
    /// Backup of `EEPROMSectionSystem::freeBlockListHeadIdx`.
    uint16_t const freeBlockListHeadIdx;
    /// Backup of `EEPROMSectionSystem::freeBlockListEndIdx`.
    uint16_t const freeBlockListEndIdx;

    FreeBlockListBackup(uint16_t const _nFreeBlocks,
                        uint16_t const _freeBlockListHeadIdx,
                        uint16_t const _freeBlockListEndIdx) noexcept;
};

} // namespace internal
} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_FREEBLOCKLISTBACKUP_HPP_
