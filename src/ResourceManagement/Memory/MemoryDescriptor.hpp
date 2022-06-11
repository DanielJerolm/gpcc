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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTOR_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTOR_HPP_

#include <cstdint>
#include <cstddef>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

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
 * \brief A memory descriptor referencing a chunk of memory managed by class @ref HeapManager.
 *
 * The referenced memory can be physical, virtual, or even fictious.
 *
 * # Usage outside of GPCC (user code)
 * Instances of this class are retrieved from an @ref HeapManager upon allocation of memory.
 * @ref MemoryDescriptor instances act as an handle to the allocated memory that must be kept by the owner
 * of the allocated memory. If the memory shall be released, then the @ref MemoryDescriptor instance
 * must be passed back to the @ref HeapManager instance from which it had been allocated.
 *
 * Instances of this class cannot be deleted (private destructor). They __must__ be finally passed back to
 * the @ref HeapManager from which they had been retrieved. If instances of this are dropped, then both the
 * referenced memory (the memory managed by the @ref HeapManager) and the memory occupied by this class'
 * instance are both lost (memory leak).
 *
 * The address and size of the referenced memory can be retrieved via @ref GetStartAddress() and @ref GetSize().
 *
 * # Usage inside GPCC
 * This is used by class @ref HeapManager.
 *
 * An instance of this class describes a piece of memory with the following attributes:
 * - `startAddress`: Start address/base address of the referenced chunk of memory.
 * - `size`: Size of the referenced memory in byte.
 * - `free`: Flag indicating if the referenced memory is free or in use.
 *
 * Further two pairs of pointers are provided to create two independent double-linked
 * lists of @ref MemoryDescriptor instances:
 * - `pPrevInMem` and `pNextInMem`: Intended to refer to @ref MemoryDescriptor instances
 *   managing the previous and next neighboring chunks of managed memory.
 * - `pPrevInList` and `pNextInList`: Intended to create lists of @ref MemoryDescriptor
 *   instances for management purposes, e.g. lists of empty memory blocks, ...
 *
 * Further reading:\n
 * Class @ref internal::MemoryDescriptorPool provides a pool for recycling of unused @ref MemoryDescriptor instances.\n
 * Class @ref internal::FreeBlockPool provides an pool for @ref MemoryDescriptor instances which reference free memory.
 */
class MemoryDescriptor
{
    friend class internal::FreeBlockPool;
    friend class internal::MemoryDescriptorPool;
    friend class HeapManager;

  public:
    inline uint32_t GetStartAddress(void) const noexcept { return startAddress; }
    inline size_t GetSize(void) const noexcept { return size; }

  private:
    /// Start address of the referenced memory. This is a byte-address.
    uint32_t startAddress;

    /// Size of the referenced memory in bytes.
    size_t size;

    /// Flag indicating if the referenced memory is free (true) or used (false).
    bool free;


    /// Pointer to the @ref MemoryDescriptor managing the left neighbour managed memory block.
    /** nullptr = none. */
    MemoryDescriptor* pPrevInMem;
    /// Pointer to the @ref MemoryDescriptor managing the right neighbour managed memory block.
    /** nullptr = none. */
    MemoryDescriptor* pNextInMem;


    /// Previous @ref MemoryDescriptor instance in a double-linked list for management purposes.
    /** nullptr = none. */
    MemoryDescriptor* pPrevInList;
    /// Next @ref MemoryDescriptor instance in a double-linked list for management purposes.
    /** nullptr = none. */
    MemoryDescriptor* pNextInList;


    MemoryDescriptor(void) = delete;
    MemoryDescriptor(uint32_t const _startAddress, size_t const _size, bool const _free) noexcept;
    MemoryDescriptor(MemoryDescriptor const &) = delete;
    MemoryDescriptor(MemoryDescriptor &&) = delete;
    ~MemoryDescriptor(void) = default;

    MemoryDescriptor& operator=(MemoryDescriptor const &) = delete;
    MemoryDescriptor& operator=(MemoryDescriptor&&) = delete;

    void RemoveFromMemList(void) noexcept;
    void InsertIntoMemListBehindThis(MemoryDescriptor* const pNewDescr) noexcept;
    void RemoveFromManagementList(void) noexcept;
};

/**
 * \fn uint32_t MemoryDescriptor::GetStartAddress(void) const
 * \brief Retrieves the start address of the referenced piece of memory.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \return
 * Start address of the referenced piece of memory.\n
 * The start address is valid until the memory is returned to the @ref HeapManager.
 */
/**
 * \fn size_t MemoryDescriptor::GetSize(void) const
 * \brief Retrieves the size of the referenced piece of memory.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \return Size of the referenced piece of memory.\n
 * The value is valid until the memory is returned to the @ref HeapManager.
 */

/**
 * @}
 */

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_MEMORYDESCRIPTOR_HPP_
