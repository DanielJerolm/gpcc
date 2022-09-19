/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "FreeBlockPool.hpp"
#include <gpcc/resource_management/memory/MemoryDescriptor.hpp>
#include <limits>
#include <stdexcept>

namespace gpcc
{
namespace resource_management
{
namespace memory
{
namespace internal
{

FreeBlockPool::FreeBlockPool(size_t const _maxSizeInFirstBucket, size_t const nBuckets)
: maxSizeInFirstBucket(_maxSizeInFirstBucket)
, buckets(nBuckets, nullptr)
/**
 * \brief Constructor. Creates an empty pool with a custom bucket configuration.
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
 * \param _maxSizeInFirstBucket
 * Maximum size (in bytes) of @ref MemoryDescriptor instances stored in the first bucket.\n
 * _This must be larger than zero._
 * \param nBuckets
 * Number of buckets.\n
 * _This must be larger than zero._
 *
 * Example:
 * ~~~{.cpp}
 * FreeBlockPool fbp(16,4);
 * ~~~
 * ...creates a @ref FreeBlockPool instance with 4 buckets for @ref MemoryDescriptor instances:\n
 * Bucket 0: @ref MemoryDescriptor instances referencing up to 16 byte of memory\n
 * Bucket 1: @ref MemoryDescriptor instances referencing up to 32 byte of memory\n
 * Bucket 2: @ref MemoryDescriptor instances referencing up to 64 byte of memory\n
 * Bucket 3: @ref MemoryDescriptor instances referencing more than 64 byte of memory
 *
 * Note that each bucket has twice the size of the previous one. The size associated with the last
 * bucket must fit into an `size_t`.
 */
{
  if (_maxSizeInFirstBucket < 1)
    throw std::invalid_argument("FreeBlockPool::FreeBlockPool: _maxSizeInFirstBucket violates constraints.");
  if (nBuckets < 1)
    throw std::invalid_argument("FreeBlockPool::FreeBlockPool: nBuckets violates constraints.");

  // check that _maxSizeInFirstBucket^(nBuckets-1) does not overflow
  size_t const MSB = static_cast<size_t>(1U) << (std::numeric_limits<size_t>::digits - 1U);
  for (size_t i = 0; i < nBuckets - 1U; i++)
  {
    if (((_maxSizeInFirstBucket << i) & MSB) != 0)
      throw std::invalid_argument("FreeBlockPool::FreeBlockPool: Too many buckets");
  }
}
FreeBlockPool::~FreeBlockPool(void)
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
  for (auto e: buckets)
    ReleaseAllDescriptorsInBucket(e);
}

void FreeBlockPool::Add(MemoryDescriptor* const pDescr) noexcept
/**
 * \brief Adds an @ref MemoryDescriptor instance to the pool.
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
 * Pointer to the @ref MemoryDescriptor that shall be added to this pool.\n
 * _Ownership moves from the caller to the pool._\n
 * `pDescr->free` is set to true.\n
 * `MemoryDescriptor::pNextInList` and `MemoryDescriptor::pPrevInList` are altered.\n
 * `MemoryDescriptor::pPrevInMem` and `MemoryDescriptor::pNextInMem` are not accessed.
 */
{
  size_t const index = DetermineBucketIndex(pDescr->size);

  pDescr->free = true;

  pDescr->pPrevInList = nullptr;
  pDescr->pNextInList = buckets[index];

  if (pDescr->pNextInList != nullptr)
    pDescr->pNextInList->pPrevInList = pDescr;

  buckets[index] = pDescr;
}
void FreeBlockPool::Remove(MemoryDescriptor* const pDescr) noexcept
/**
 * \brief Removes an @ref MemoryDescriptor instance from the pool.
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
 * Pointer to the @ref MemoryDescriptor instance that shall be removed.\n
 * _The MemoryDescriptor instance must be inside this pool instance, otherwise behavior is undefined._\n
 * _Ownership moves from the pool to the caller._\n
 * `pDescr->free` is set to false.\n
 * `MemoryDescriptor::pNextInList` and `MemoryDescriptor::pPrevInList` are both nullptr.
 */
{
  RemoveFromBucket(pDescr, DetermineBucketIndex(pDescr->size));
  pDescr->free = false;
}
MemoryDescriptor* FreeBlockPool::Get(size_t const minimumRequiredSize) noexcept
/**
 * \brief Requests an @ref MemoryDescriptor instance from the pool.
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
 * \param minimumRequiredSize
 * Minimum size of the requested memory.
 * \return
 * Pointer to a memory descriptor referencing a chunk of memory whose size is equal to or larger than
 * parameter `minimumRequiredSize`.\n
 * _nullptr, if there is no suitable memory descriptor available._\n
 * _Ownership moves from the pool to the caller._\n
 * `pDescr->free` is set to false.\n
 * `MemoryDescriptor::pNextInList` and `MemoryDescriptor::pPrevInList` are both nullptr.
 */
{
  // determine index of bucket where to start looking for a suitable MemoryDescriptor.
  size_t index = DetermineBucketIndex(minimumRequiredSize);

  // look for a suitable descriptor
  MemoryDescriptor* pDescr = buckets[index];
  do
  {
    // move to bucket containing larger blocks if necessary
    while (pDescr == nullptr)
    {
      index++;
      if (index >= buckets.size())
        return nullptr;
      pDescr = buckets[index];
    }

    // search bucket for suitable block
    while ((pDescr != nullptr) && (pDescr->size < minimumRequiredSize))
      pDescr = pDescr->pNextInList;
  }
  while (pDescr == nullptr);

  RemoveFromBucket(pDescr, index);
  pDescr->free = false;

  return pDescr;
}

size_t FreeBlockPool::DetermineBucketIndex(size_t const size) const noexcept
/**
 * \brief Determines the bucket index corresponding to a given memory block size.
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
 * \param size Size of the memory block.
 * \return Bucket index corresponding to parameter `size`.
 */
{
  size_t maxSizeInCurrentBucket = maxSizeInFirstBucket;
  size_t index = 1;

  while ((index < buckets.size()) && (size > maxSizeInCurrentBucket))
  {
    maxSizeInCurrentBucket <<= 1U;
    index++;
  }

  return index - 1U;
}
void FreeBlockPool::RemoveFromBucket(MemoryDescriptor* const pDescr, size_t const index) noexcept
/**
 * \brief Removes an @ref MemoryDescriptor instance from an bucket.
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
 * \param pDescr Pointer to the @ref MemoryDescriptor instance that shall be removed.
 * \param index Index of the bucket, in which `pDescr` is contained.
 */
{
  // update the bucket if we removed the first block
  if (buckets[index] == pDescr)
    buckets[index] = pDescr->pNextInList;

  pDescr->RemoveFromManagementList();
}
void FreeBlockPool::ReleaseAllDescriptorsInBucket(MemoryDescriptor* pHead) noexcept
/**
 * \brief Releases all @ref MemoryDescriptor instances currently inside a bucket.
 *
 * Release is done based on the management list (`MemoryDescriptor::pPrevInList` and
 * `MemoryDescriptor::pNextInList`). The vector `buckets` is not altered by this.
 * The caller is responsible for setting `buckets[x]` to `nullptr` if necessary.
 *
 * ---
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
 * \param pHead
 * Pointer to the first @ref MemoryDescriptor instance in a bucket.\n
 * If this is nullptr, then this method does nothing.
 */
{
  while (pHead != nullptr)
  {
    MemoryDescriptor* const pNext = pHead->pNextInList;
    delete pHead;
    pHead = pNext;
  }
}

} // namespace internal
} // namespace memory
} // namespace resource_management
} // namespace gpcc
