/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGER_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGER_HPP_

#include "HeapManagerStatistics.hpp"
#include <memory>
#include <cstdint>
#include <cstddef>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

class MemoryDescriptor;

namespace internal
{
  class FreeBlockPool;
  class MemoryDescriptorPool;
}

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
class HeapManager final
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
    ~HeapManager(void);

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
    std::unique_ptr<internal::FreeBlockPool> spFreeBlocks;

    /// Pool with unused memory descriptors.
    std::unique_ptr<internal::MemoryDescriptorPool> spDescriptorPool;

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
