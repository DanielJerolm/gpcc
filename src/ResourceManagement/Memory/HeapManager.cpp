/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "HeapManager.hpp"
#include "MemoryDescriptor.hpp"
#include <gpcc/math/checks.hpp>
#include "gpcc/src/raii/scope_guard.hpp"
#include <stdexcept>
#include <limits>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

HeapManager::HeapManager(uint16_t const _minimumAlignment,
                         uint32_t const baseAddress,
                         size_t   const size,
                         size_t   const maxSizeInFirstBucket,
                         size_t   const nBuckets)
: minimumAlignment(_minimumAlignment)
, freeBlocks(maxSizeInFirstBucket, nBuckets)
, descriptorPool()
, statistics(1, size)
/**
 * \brief Constructor.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _minimumAlignment
 * Minimum alignment of the addresses of each block of memory allocated via @ref Allocate(). \n
 * _Constraints:_
 * - This must be larger than 0.
 * - This must be a power of 2.
 * \param baseAddress
 * Start address for the memory managed by the @ref HeapManager. \n
 * This may be any value you like. The @ref HeapManager will not associate it with physical or virtual memory
 * of the system, neither will it try to access it.\n
 * _Constraints:_
 * - This must be aligned to the minimum alignment.
 * \param size
 * Size of the memory managed by the @ref HeapManager. \n
 * _Constraints:_
 * - This must be equal to or larger than the minimum alignment.
 * - This must be a multiple of the minimum alignment.
 * - The sum of `baseAddress` and `size` must not exceed the value range of uint32_t.
 * \param maxSizeInFirstBucket
 * Internally the @ref HeapManager organizes free blocks of memory in buckets ordered by their size.
 * This is the maximum size for chunks of memory in the first bucket. Each further bucket contains chunks of
 * memory up to twice the size of the chunks of memory in the previous bucket.\n
 * For details, please refer to @ref internal::FreeBlockPool::FreeBlockPool(size_t, size_t).\n
 * _Constraints:_
 * - This must be equal to or larger than the minimum alignment.
 * - This must be equal to or less than the size of the managed memory.
 * \param nBuckets
 * Internally the @ref HeapManager organizes free blocks of memory in buckets ordered by their size.
 * This is the number of buckets used. The maximum size for chunks of memory in the first bucket is given by
 * parameter `maxSizeInFirstBucket`. Each further bucket contains chunks of memory up to twice the size of the
 * chunks of memory in the previous bucket.\n
 * For details, please refer to @ref internal::FreeBlockPool::FreeBlockPool(size_t, size_t).\n
 * _Constraints:_
 * - This must be [1;24]
 * - 2^(this-2) * `maxSizeInFirstBucket` must not exceed `size`.
 *
 * __Examples:__
 * ~~~{.cpp}
 * HeapManager(16, 0, 2048, 32, 5);
 * ~~~
 * ...creates an heap manager managing 2kB of memory starting at address 0x00000000. Each allocated memory will be
 * aligned to a 16-byte address. Empty blocks are organized in 5 buckets:\n
 * Bucket 0: 1..32 byte\n
 * Bucket 1: 33..64 byte\n
 * Bucket 2: 65..128 byte\n
 * Bucket 3: 129..256 byte\n
 * Bucket 4: > 256 byte\n
 *
 * ~~~{.cpp}
 * HeapManager(4, 0x1000, 4096, 8, 8);
 * ~~~
 * ...creates an heap manager managing 4kB of memory starting at address 0x00001000. Each allocated memory will be
 * aligned to a 4-byte address. Empty blocks are organized in 8 buckets:\n
 * Bucket 0: 1..8 byte\n
 * Bucket 1: 9..16 byte\n
 * Bucket 2: 17..32 byte\n
 * Bucket 3: 33..64 byte\n
 * Bucket 4: 65..128 byte\n
 * Bucket 5: 129..256 byte\n
 * Bucket 6: 257..512 byte\n
 * Bucket 7: > 512 byte\n
 *
 */
{
  // check constraints
  if ((minimumAlignment == 0) ||
      (!math::IsPowerOf2(minimumAlignment)))
    throw std::invalid_argument("HeapManager::HeapManager: \"_minimumAlignment\" violates constrains");

  if ((baseAddress % minimumAlignment) != 0)
    throw std::invalid_argument("HeapManager::HeapManager: \"baseAddress\" violates constraints");

  if ((size < minimumAlignment) || ((size % minimumAlignment) != 0))
    throw std::invalid_argument("HeapManager::HeapManager: \"size\" violates constraints");

  if ((static_cast<size_t>(std::numeric_limits<uint32_t>::max()) - size) + 1U < baseAddress)
    throw std::invalid_argument("HeapManager::HeapManager: address overflow possible");

  if ((maxSizeInFirstBucket < minimumAlignment) || (maxSizeInFirstBucket > size))
    throw std::invalid_argument("HeapManager::HeapManager: \"maxSizeInFirstBucket\" violates constraints");

  if ((nBuckets < 1) || (nBuckets > 24) ||
      ((nBuckets > 1) && (((1U << (nBuckets - 2U)) * maxSizeInFirstBucket) >= size)))
    throw std::invalid_argument("HeapManager::HeapManager \"nBuckets\" violates constraints");

  // create the very first descriptor and put it into the list of free blocks
  freeBlocks.Add(descriptorPool.Get(baseAddress, size, true));
}

bool HeapManager::AnyAllocations(void) const noexcept
/**
 * \brief Retrieves if there is currently any memory allocated from the HeapManager.
 *
 * Intentional use: It might be useful to check if all allocations have been released before
 * a @ref HeapManager instance is released.
 *
 * ---
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
 * true  = At least one allocation has not yet been released\n
 * false = No allocations done or all allocations have been released
 */
{
  return (statistics.nbOfAllocatedBlocks != 0);
}
HeapManagerStatistics HeapManager::GetStatistics(void) const noexcept
/**
 * \brief Retrieves statistical information.
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
 * \return Statistical information capturing the current state of the @ref HeapManager.
 */
{
  return statistics;
}

MemoryDescriptor* HeapManager::Allocate(size_t size)
/**
 * \brief Allocates memory from the @ref HeapManager.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 * Be aware of the following exceptions:
 * - bad_alloc (System's heap (__not__ memory managed by HeapManager) is exhausted)
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param size
 * Minimum size for the requested memory. This must be larger than zero.\n
 * The allocated size will be equal to or (slightly) larger than this.
 * \return
 * Pointer to an @ref MemoryDescriptor instance referencing the allocated memory.\n
 * _nullptr, if no memory could be allocated (out-of-memory of the HeapManager)._\n
 * _Ownership moves from the HeapManager to the caller._\n
 * The caller cannot destroy the @ref MemoryDescriptor (it has a private destructor). Instead, the
 * @ref MemoryDescriptor must be finally passed back to the @ref HeapManager via @ref Release(). \n
 * If the pointer to the @ref MemoryDescriptor is dropped, then the referenced memory is lost.\n
 * If the @ref HeapManager is released without passing the @ref MemoryDescriptor back to the
 * @ref Release() method, then the @ref MemoryDescriptor instance itself will never be released.
 */
{
  if (size == 0)
    throw std::invalid_argument("HeapManager::Allocate: size == 0");

  // round size up to meet the address alignment
  size_t const nBlocks = ((size - 1U) / minimumAlignment) + 1U;
  if (nBlocks > (std::numeric_limits<size_t>::max() / minimumAlignment))
    return nullptr;
  size = nBlocks * minimumAlignment;

  // search for a suitable free block
  MemoryDescriptor* const pBlock = freeBlocks.Get(size);
  if (pBlock == nullptr)
    return nullptr;

  // block larger than required?
  if (pBlock->size > size)
  {
    // split block and move unused memory back to pool of free blocks
    ON_SCOPE_EXIT() { freeBlocks.Add(pBlock); };
    MemoryDescriptor* const pNewBlock = descriptorPool.Get(pBlock->startAddress + size, pBlock->size - size, true);
    ON_SCOPE_EXIT_DISMISS();

    pBlock->InsertIntoMemListBehindThis(pNewBlock);
    pBlock->size = size;

    freeBlocks.Add(pNewBlock);
  }
  else
  {
    statistics.nbOfFreeBlocks--;
  }

  statistics.nbOfAllocatedBlocks++;
  statistics.totalFreeSpace -= size;
  statistics.totalUsedSpace += size;
  return pBlock;
}
void HeapManager::Release(MemoryDescriptor* const pDescr)
/**
 * \brief Releases previously allocated memory.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pDescr
 * Pointer to a @ref MemoryDescriptor instance previously retrieved from this @ref HeapManager
 * instance via @ref Allocate(). \n
 * _Ownership moves from the caller to the HeapManager instance._\n
 * _The memory must have been allocated from this HeapManager instance, otherwise behaviour is undefined._\n
 * _nullptr is not allowed._\n
 * _After release, the referenced memory must no longer be used._\n
 * _Allocated memory must not be released twice._
 */
{
  if (pDescr == nullptr)
    throw std::invalid_argument("HeapManager::Release: !pDescr");

  if (pDescr->free)
    throw std::invalid_argument("HeapManager::Release: unexpected pDescr->free");

  statistics.nbOfFreeBlocks++;
  statistics.nbOfAllocatedBlocks--;
  statistics.totalFreeSpace += pDescr->size;
  statistics.totalUsedSpace -= pDescr->size;

  // check for chance to combine with previous block
  if ((pDescr->pPrevInMem != nullptr) && (pDescr->pPrevInMem->free))
  {
    MemoryDescriptor* const pPrev = pDescr->pPrevInMem;
    freeBlocks.Remove(pPrev);

    pDescr->startAddress = pPrev->startAddress;
    pDescr->size += pPrev->size;

    pPrev->RemoveFromMemList();

    descriptorPool.Recycle(pPrev);

    statistics.nbOfFreeBlocks--;
  }

  // check for chance to combine with next block
  if ((pDescr->pNextInMem != nullptr) && (pDescr->pNextInMem->free))
  {
    MemoryDescriptor* const pNext = pDescr->pNextInMem;
    freeBlocks.Remove(pNext);

    pDescr->size += pNext->size;

    pNext->RemoveFromMemList();

    descriptorPool.Recycle(pNext);

    statistics.nbOfFreeBlocks--;
  }

 freeBlocks.Add(pDescr);
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc
