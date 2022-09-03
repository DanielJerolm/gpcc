/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/resource_management/memory/MemoryDescriptor.hpp>
#include "MemoryDescriptorPool.hpp"

namespace gpcc
{
namespace resource_management
{
namespace memory
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
} // namespace memory
} // namespace resource_management
} // namespace gpcc
