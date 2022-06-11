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

#include "FreeBlockListBackup.hpp"

namespace gpcc
{
namespace file_systems
{
namespace EEPROMSectionSystem
{
namespace internal
{

FreeBlockListBackup::FreeBlockListBackup(uint16_t const _nFreeBlocks,
                                         uint16_t const _freeBlockListHeadIdx,
                                         uint16_t const _freeBlockListEndIdx) noexcept
: nFreeBlocks(_nFreeBlocks)
, freeBlockListHeadIdx(_freeBlockListHeadIdx)
, freeBlockListEndIdx(_freeBlockListEndIdx)
/**
 * \brief Constructor.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _nFreeBlocks
 * Current value of `EEPROMSectionSystem::nFreeBlocks`.
 * \param _freeBlockListHeadIdx
 * Current value of `EEPROMSectionSystem::_freeBlockListHeadIdx`.
 * \param _freeBlockListEndIdx
 * Current value of `EEPROMSectionSystem::_freeBlockListEndIdx`.
 */
{
}

} // namespace internal
} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc
