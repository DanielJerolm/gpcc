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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_MEMORYDESCRIPTORPOOL_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_MEMORYDESCRIPTORPOOL_HPP_

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

/**
 * @ingroup GPCC_RESOURCEMANAGEMENT_MEMORY
 * @{
 */

/**
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
} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_MEMORYDESCRIPTORPOOL_HPP_
