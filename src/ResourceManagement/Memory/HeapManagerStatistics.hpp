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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSTATISTICS_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSTATISTICS_HPP_

#include <cstddef>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

/**
 * @ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * @{
 */

/**
 * \brief Container for statistics that can be retrieved from an @ref HeapManager or @ref HeapManagerSPTS instance.
 *
 * _Implicit capabilities: copy-construction, copy-assignment, move-construction, move-assignment_
 */
class HeapManagerStatistics
{
  public:
    size_t nbOfFreeBlocks;      ///<Number of free blocks.
    size_t nbOfAllocatedBlocks; ///<Number of allocated blocks.
    size_t totalFreeSpace;      ///<Total free storage in bytes.
                                /**<Note: Memory required for management is allocated on the system's heap and
                                    does not contribute to this. */
    size_t totalUsedSpace;      ///<Total used storage in bytes.
                                /**<Note: Memory required for management is allocated on the system's heap and
                                    does not contribute to this. */

    HeapManagerStatistics(void) = delete;
    HeapManagerStatistics(size_t const _nbOfFreeBlocks, size_t const _totalFreeSpace) noexcept;
};

/**
 * @}
 */

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSTATISTICS_HPP_
