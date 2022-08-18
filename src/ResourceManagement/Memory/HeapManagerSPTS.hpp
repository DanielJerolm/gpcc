/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSPTS_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSPTS_HPP_

#include "HeapManager.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include <memory>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

class MemoryDescriptorSPTS;

/**
 * @ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * @{
 */

/**
 * \brief Thread-safe heap-style memory manager for any kind of memory (physical/virtual/fictious) using
 * smart pointers and RAII.
 *
 * The memory is managed based on a given address and size. Both do not need to correlate
 * to system's physical or virtual memory. The @ref HeapManagerSPTS will not attempt to access
 * the referenced memory in any way, so even fictious memory could be managed.
 * All data structures required to manage the memory are stored on the heap.
 *
 * Typical applications:
 * - Managing RAM located in a hardware peripheral (e.g. frame or message buffers)
 *
 * There is another version of the @ref HeapManagerSPTS available: @ref HeapManager. \n
 * Both provide the same functionality, but @ref HeapManager has _no_ build-in thread-safety
 * and does _not_ use RAII memory descriptors.\n
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
 * std::shared_ptr<HeapManagerSPTS> hm(HeapManagerSPTS::Create(4, 0x5000, 1024, 16, 6));
 *
 * // Lets allocate 38 bytes of memory
 * // (in real life, you should be aware that hm.Allocate could throw)
 * std::shared_ptr<MemoryDescriptorSPTS> spMD1(hm.Allocate(38));
 *
 * // Address and size of the allocated memory can be easily retrieved:
 * uint32_t const startAddress = spMD1->GetStartAddress();
 * size_t const size           = spMD1->GetSize();
 *
 * // Lets allocate some more
 * // (Again, hm.Allocate could throw. But since we are using smart-pointers here,
 * // its easier to deal with exceptions)
 * std::shared_ptr<MemoryDescriptorSPTS> spMD2(hm.Allocate(12));
 *
 * // ...
 *
 * // To release an allocation, just drop the (last) shared pointer referencing it:
 * spMD1 = nullptr;
 *
 * // Lets drop the shared pointer to the HeapManagerSPTS:
 * hm = nullptr;
 *
 * // Note that the HeapManagerSPTS instance is not yet destroyed. It is kept alive
 * // by our second allocation that is still hanging around. Let's release it:
 * spMD2 = nullptr;
 *
 * // After release of our second allocation, the HeapManagerSPTS instance will be
 * // automatically released, too.
 * ~~~
 */
class HeapManagerSPTS: public std::enable_shared_from_this<HeapManagerSPTS>
{
    friend class MemoryDescriptorSPTS;

  public:
    HeapManagerSPTS(void) = delete;
    HeapManagerSPTS(HeapManagerSPTS const &) = delete;
    HeapManagerSPTS(HeapManagerSPTS &&) = delete;
    ~HeapManagerSPTS(void) = default;


    static std::shared_ptr<HeapManagerSPTS> Create(uint16_t const _minimumAlignment,
                                                   uint32_t const baseAddress,
                                                   size_t   const size,
                                                   size_t   const maxSizeInFirstBucket,
                                                   size_t   const nBuckets);


    HeapManagerSPTS& operator=(HeapManagerSPTS const &) = delete;
    HeapManagerSPTS& operator=(HeapManagerSPTS&&) = delete;


    bool AnyAllocations(void) const;
    HeapManagerStatistics GetStatistics(void) const;

    std::shared_ptr<MemoryDescriptorSPTS> Allocate(size_t size);

  private:
    /// Mutex used to make accesses to the encapsulated @ref HeapManager (@ref hm) thread-safe.
    mutable osal::Mutex mutex;

    /// The encapsulated @ref HeapManager instance.
    HeapManager hm;


    HeapManagerSPTS(uint16_t const _minimumAlignment,
                    uint32_t const baseAddress,
                    size_t   const size,
                    size_t   const maxSizeInFirstBucket,
                    size_t   const nBuckets);

    void Release(MemoryDescriptor* const pDescr);
};

/**
 * @}
 */

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_HEAPMANAGERSPTS_HPP_
