/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSTATISTICS_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSTATISTICS_HPP_

#include <cstddef>

namespace gpcc
{
namespace resource_management
{
namespace memory
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

} // namespace memory
} // namespace resource_management
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSTATISTICS_HPP_
