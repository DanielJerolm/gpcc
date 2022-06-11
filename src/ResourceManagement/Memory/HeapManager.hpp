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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGER_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGER_HPP_

#include "internal/FreeBlockPool.hpp"
#include "internal/MemoryDescriptorPool.hpp"
#include "HeapManagerStatistics.hpp"
#include <cstdint>
#include <cstddef>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

class MemoryDescriptor;

/**
 * @ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * @{
 */

/**
 * \brief Heap-style memory manager for any kind of memory (physical/virtual/fictious).
 *
 * The memory is managed based on a given address and size. Both do not need to correlate
 * to system's physical or virtual memory. The @ref HeapManager will not attempt to access
 * the referenced memory in any way, so even fictious memory could be managed.
 * All data structures required to manage the memory are stored on the heap.
 *
 * Typical applications:
 * - Managing RAM located in a hardware peripheral (e.g. frame or message buffers)
 *
 * There is another version of the @ref HeapManager available: @ref HeapManagerSPTS. \n
 * Both provide the same functionality, but @ref HeapManagerSPTS has build-in thread-safety
 * and uses RAII memory descriptors.\n
 * Usage of @ref HeapManagerSPTS should be considered if you want to pass descriptors to
 * allocated memory through APIs to other code.\n
 * @ref HeapManager is recommended if speed and small memory usage counts and if RAII
 * and thread-safety are not required.
 *
 * Example:
 * ~~~{.cpp}
 * // We want to manage 1024bytes of memory starting at the imaginary address 0x5000.
 * // Any allocated memory shall be aligned to 4-byte.
 * // To optimize management of unused memory, we want to work with 6 buckets:
 * // up to 16, 32, 64, 128, 256 bytes + more than 256 bytes
 * HeapManager hm(4, 0x5000, 1024, 16, 6);
 *
 * // Lets allocate 38 bytes of memory
 * // (in real life, you should be aware that hm.Allocate could throw)
 * MemoryDescriptor* pMD = hm.Allocate(38);
 *
 * // Address and size of the allocated memory can be easily retrieved:
 * uint32_t const startAddress = pMD->GetStartAddress();
 * size_t const size           = pMD->GetSize();
 *
 * // ...
 *
 * // Finally, the allocated memory must be released.
 * // "startAddress" and "size" become invalid now. "pMD" must no longer be accessed.
 * hm.Release(pMD);
 * ~~~
 */
class HeapManager
{
  public:
    HeapManager(void) = delete;
    HeapManager(uint16_t const _minimumAlignment,
                uint32_t const baseAddress,
                size_t   const size,
                size_t   const maxSizeInFirstBucket,
                size_t   const nBuckets);
    HeapManager(HeapManager const &) = delete;
    HeapManager(HeapManager &&) = delete;
    ~HeapManager(void) = default;

    HeapManager& operator=(HeapManager const &) = delete;
    HeapManager& operator=(HeapManager&&) = delete;

    bool AnyAllocations(void) const noexcept;
    HeapManagerStatistics GetStatistics(void) const noexcept;

    MemoryDescriptor* Allocate(size_t size);
    void Release(MemoryDescriptor* const pDescr);

  private:
    /// Minimum required address alignment for allocated blocks of memory.
    uint16_t const minimumAlignment;

    /// Pool with free memory blocks.
    internal::FreeBlockPool freeBlocks;

    /// Pool with unused memory descriptors.
    internal::MemoryDescriptorPool descriptorPool;

    /// Statistics.
    HeapManagerStatistics statistics;
};

/**
 * @}
 */

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGER_HPP_
