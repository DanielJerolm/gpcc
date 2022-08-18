/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
