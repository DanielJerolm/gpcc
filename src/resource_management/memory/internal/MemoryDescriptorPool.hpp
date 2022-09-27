/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_MEMORYDESCRIPTORPOOL_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_MEMORYDESCRIPTORPOOL_HPP_

#include <cstddef>
#include <cstdint>

namespace gpcc
{
namespace resource_management
{
namespace memory
{

class MemoryDescriptor;

namespace internal
{

/**
 * @ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * @{
 */

/**
 * \class MemoryDescriptorPool MemoryDescriptorPool.hpp "src/resource_management/memory/internal/MemoryDescriptorPool.hpp"
 * \brief A grow-only pool for recycling unused @ref MemoryDescriptor instances that do not reference to any memory.
 *
 * _Implicit capabilities: default-construction_
 *
 * The pool internally implements a LIFO based on a single linked list of @ref MemoryDescriptor
 * instances. The pool uses `MemoryDescriptor::pNextInList` to build the list.
 *
 * Note that all attributes `pPrevInList`, `pNextInList`, `pPrevInMem`, and `pNextInMem` of the
 * `MemoryDescriptor` are altered by this.
 *
 * @ref MemoryDescriptor instances can be retrieved via @ref Get(). If the pool is empty, then a new
 * @ref MemoryDescriptor instance is allocated on the heap. Otherwise all calls to @ref Get() are
 * satisfied by recycling @ref MemoryDescriptor instances from the pool.
 *
 * @ref MemoryDescriptor instances that are no longer used by clients can be passed to the pool for
 * recycling using @ref Recycle(). Any instances passed to @ref Recycle() are added to the pool and
 * are not released. The pool is therefore grow-only.
 *
 * When the pool is finally released, then all @ref MemoryDescriptor instances enqueued in it are
 * also released.
 */
class MemoryDescriptorPool
{
  public:
    MemoryDescriptorPool(void) = default;
    MemoryDescriptorPool(MemoryDescriptorPool const &) = delete;
    MemoryDescriptorPool(MemoryDescriptorPool&&) = delete;
    ~MemoryDescriptorPool(void);

    MemoryDescriptorPool& operator=(MemoryDescriptorPool const &) = delete;
    MemoryDescriptorPool& operator=(MemoryDescriptorPool&&) = delete;

    MemoryDescriptor* Get(uint32_t const startAddress, size_t const size, bool const free);
    void Recycle(MemoryDescriptor* const pDescr) noexcept;

  private:
    /// Pointer to the head of a single linked list of @ref MemoryDescriptor instances.
    /** nullptr = pool empty.\n
        The single linked list is made up using @ref MemoryDescriptor::pNextInList. \n
        @ref MemoryDescriptor::pNextInList of the last list element is nullptr. */
    MemoryDescriptor* pPool = nullptr;
};

/**
 * @}
 */

} // namespace internal
} // namespace memory
} // namespace resource_management
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_MEMORYDESCRIPTORPOOL_HPP_
