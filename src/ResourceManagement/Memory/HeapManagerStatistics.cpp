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
