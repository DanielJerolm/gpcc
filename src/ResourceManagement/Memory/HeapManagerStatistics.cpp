/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "HeapManagerStatistics.hpp"

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

HeapManagerStatistics::HeapManagerStatistics(size_t const _nbOfFreeBlocks, size_t const _totalFreeSpace) noexcept
: nbOfFreeBlocks(_nbOfFreeBlocks)
, nbOfAllocatedBlocks(0)
, totalFreeSpace(_totalFreeSpace)
, totalUsedSpace(0)
/**
 * \brief Constructor. Creates an @ref HeapManagerStatistics instance describing a @ref HeapManager or
 * @ref HeapManagerSPTS with no allocations done yet.
 *
 * The attributes @ref nbOfAllocatedBlocks and @ref totalUsedSpace are initialized with zero.
 * Attributes @ref nbOfFreeBlocks and @ref totalFreeSpace are initialized with parameters passed to
 * this method.
 *
 * ---
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
 * \param _nbOfFreeBlocks Initial value for attribute @ref nbOfFreeBlocks.
 * \param _totalFreeSpace Initial value for attribute @ref totalFreeSpace.
 */
{
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc
