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

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_FREEBLOCKLISTBACKUP_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_FREEBLOCKLISTBACKUP_HPP_

#include <cstdint>

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
 * \brief Snapshot of the state of the free-block lists of an @ref EEPROMSectionSystem instance.
 *
 * _Implicit capabilities: copy-construction, copy-assignment, move-construction, move-assignment_
 *
 * This is internally used by class @ref EEPROMSectionSystem to allow for roll-back of free-block
 * allocations in case of an error.
 */
class FreeBlockListBackup
{
    friend class gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem;

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
} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_INTERNAL_FREEBLOCKLISTBACKUP_HPP_
