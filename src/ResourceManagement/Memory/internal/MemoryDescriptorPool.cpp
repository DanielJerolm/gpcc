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

#include "../MemoryDescriptor.hpp"
#include "MemoryDescriptorPool.hpp"

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{
namespace internal
{

MemoryDescriptorPool::~MemoryDescriptorPool(void)
/**
 * \brief Destructor. The pool and all @ref MemoryDescriptor instances in it are released.
 *
 * __Thread safety:__\n
 * Do not access object after invocation of destructor.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  while (pPool != nullptr)
  {
    MemoryDescriptor* const pNext = pPool->pNextInList;
    delete pPool;
    pPool = pNext;
  }
}

MemoryDescriptor* MemoryDescriptorPool::Get(uint32_t const startAddress, size_t const size, bool const free)
/**
 * \brief Retrieves an @ref MemoryDescriptor instance from the pool.
 *
 * If the pool is empty, then a new @ref MemoryDescriptor instance is allocated on the heap.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.\n
 * Be aware of the following exceptions:
 * - bad_alloc (thrown if pool is empty and allocation on heap has failed)
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param startAddress Value for initializing `MemoryDescriptor::startAddress`.
 * \param size Value for initializing `MemoryDescriptor::size`.
 * \param free Value for initializing `MemoryDescriptor::free`.
 * \return
 * Pointer to the @ref MemoryDescriptor instance.\n
 * _Ownership moves from the pool to the caller._\n
 * Attributes `pPrevInList`, `pNextInList`, `pPrevInMem`, and `pNextInMem` are nullptr.
 */
{
  // pool not empty?
  if (pPool != nullptr)
  {
    // fetch from head of list
    MemoryDescriptor* const pDescr = pPool;
    pPool = pDescr->pNextInList;

    // setup descriptor
    pDescr->startAddress  = startAddress;
    pDescr->size          = size;
    pDescr->free          = free;
    pDescr->pNextInList   = nullptr;

    return pDescr;
  }
  else
  {
    // pool empty, allocate a descriptor from the heap
    return new MemoryDescriptor(startAddress, size, free);
  }
}
void MemoryDescriptorPool::Recycle(MemoryDescriptor* const pDescr) noexcept
/**
 * \brief Recycles an @ref MemoryDescriptor instance and puts it into the pool for reuse.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
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
 * \param pDescr
 * Pointer to the @ref MemoryDescriptor instance that shall be recycled.\n
 * _Ownership moves from the caller to the pool._\n
 *  All attributes `pPrevInList`, `pNextInList`, `pPrevInMem`, and `pNextInMem` of the
 * `MemoryDescriptor` are altered by this.
 */
{
  pDescr->pPrevInMem  = nullptr;
  pDescr->pNextInMem  = nullptr;
  pDescr->pPrevInList = nullptr;
  pDescr->pNextInList = pPool;

  pPool = pDescr;
}

} // namespace internal
} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc
