/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/resource_management/memory/HeapManagerSPTS.hpp>
#include <gpcc/resource_management/memory/MemoryDescriptorSPTS.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>

namespace gpcc
{
namespace ResourceManagement
{
namespace Memory
{

std::shared_ptr<HeapManagerSPTS> HeapManagerSPTS::Create(uint16_t const _minimumAlignment,
                                                         uint32_t const baseAddress,
                                                         size_t   const size,
                                                         size_t   const maxSizeInFirstBucket,
                                                         size_t   const nBuckets)
/**
 * \brief Factory method. Creates an HeapManagerSPTS instance.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * Start address for the memory managed by the @ref HeapManagerSPTS. \n
 * This may be any value you like. The @ref HeapManagerSPTS will not associate it with physical or virtual memory
 * of the system, neither will it try to access it.\n
 * _Constraints:_
 * - This must be aligned to the minimum alignment.
 * \param size
 * Size of the memory managed by the @ref HeapManagerSPTS. \n
 * _Constraints:_
 * - This must be equal to or larger than the minimum alignment.
 * - This must be a multiple of the minimum alignment.
 * - The sum of `baseAddress` and `size` must not exceed the value range of uint32_t.
 * \param maxSizeInFirstBucket
 * Internally the @ref HeapManagerSPTS organizes free blocks of memory in buckets ordered by their size.
 * This is the maximum size for chunks of memory in the first bucket. Each further bucket contains chunks of
 * memory up to twice the size of the chunks of memory in the previous bucket.\n
 * For details, please refer to @ref internal::FreeBlockPool::FreeBlockPool(size_t, size_t).\n
 * _Constraints:_
 * - This must be equal to or larger than the minimum alignment.
 * - This must be equal to or less than the size of the managed memory.
 * \param nBuckets
 * Internally the @ref HeapManagerSPTS organizes free blocks of memory in buckets ordered by their size.
 * This is the number of buckets used. The maximum size for chunks of memory in the first bucket is given by
 * parameter `maxSizeInFirstBucket`. Each further bucket contains chunks of memory up to twice the size of the
 * chunks of memory in the previous bucket.\n
 * For details, please refer to @ref internal::FreeBlockPool::FreeBlockPool(size_t, size_t).\n
 * _Constraints:_
 * - This must be [1;24]
 * - 2^(this-2) * `maxSizeInFirstBucket` must not exceed `size`.
 * \return A shared pointer to a new created @ref HeapManagerSPTS instance.
 *
 * __Examples:__
 * ~~~{.cpp}
 * std::shared_ptr<HeapManagerSPTS> spHM(HeapManagerSPTS::Create(16, 0, 2048, 32, 5));
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
 * std::shared_ptr<HeapManagerSPTS> spHM(HeapManagerSPTS::Create(4, 0x1000, 4096, 8, 8));
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
  return std::shared_ptr<HeapManagerSPTS>(new HeapManagerSPTS(_minimumAlignment, baseAddress, size, maxSizeInFirstBucket, nBuckets));
}

bool HeapManagerSPTS::AnyAllocations(void) const
/**
 * \brief Retrieves if there is currently any memory allocated from the HeapManagerSPTS.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \return
 * true  = At least one allocation has not yet been released\n
 * false = No allocations done or all allocations have been released
 */
{
  osal::MutexLocker mutexLocker(mutex);
  return hm.AnyAllocations();
}
HeapManagerStatistics HeapManagerSPTS::GetStatistics(void) const
/**
 * \brief Retrieves statistical information.
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \return Statistical information capturing the current state of the @ref HeapManagerSPTS.
 */
{
  osal::MutexLocker mutexLocker(mutex);
  return hm.GetStatistics();
}

std::shared_ptr<MemoryDescriptorSPTS> HeapManagerSPTS::Allocate(size_t size)
/**
 * \brief Allocates memory from the @ref HeapManagerSPTS.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 * Be aware of the following exceptions:
 * - bad_alloc (System's heap (__not__ memory managed by HeapManagerSPTS) is exhausted)
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
 * Shared pointer to an @ref MemoryDescriptorSPTS instance referencing the allocated memory.\n
 * _nullptr, if no memory could be allocated (out-of-memory of the HeapManagerSPTS)._\n
 * _Ownership moves from the HeapManagerSPTS to the caller._\n
 * The memory will be released when the @ref MemoryDescriptorSPTS instance is destroyed.
 */
{
  osal::AdvancedMutexLocker mutexLocker(mutex);
  MemoryDescriptor* const pMD = hm.Allocate(size);
  mutexLocker.Unlock();
  if (pMD != nullptr)
    return std::make_shared<MemoryDescriptorSPTS>(shared_from_this(), pMD, MemoryDescriptorSPTSKey());
  else
    return std::shared_ptr<MemoryDescriptorSPTS>();
}

HeapManagerSPTS::HeapManagerSPTS(uint16_t const _minimumAlignment,
                                 uint32_t const baseAddress,
                                 size_t   const size,
                                 size_t   const maxSizeInFirstBucket,
                                 size_t   const nBuckets)
: std::enable_shared_from_this<HeapManagerSPTS>()
, mutex()
, hm(_minimumAlignment, baseAddress, size, maxSizeInFirstBucket, nBuckets)
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
 * Start address for the memory managed by the @ref HeapManagerSPTS. \n
 * This may be any value you like. The @ref HeapManagerSPTS will not associate it with physical or virtual memory
 * of the system, neither will it try to access it.\n
 * _Constraints:_
 * - This must be aligned to the minimum alignment.
 * \param size
 * Size of the memory managed by the @ref HeapManagerSPTS. \n
 * _Constraints:_
 * - This must be equal to or larger than the minimum alignment.
 * - This must be a multiple of the minimum alignment.
 * - The sum of `baseAddress` and `size` must not exceed the value range of uint32_t.
 * \param maxSizeInFirstBucket
 * Internally the @ref HeapManagerSPTS organizes free blocks of memory in buckets ordered by their size.
 * This is the maximum size for chunks of memory in the first bucket. Each further bucket contains chunks of
 * memory up to twice the size of the chunks of memory in the previous bucket.\n
 * For details, please refer to @ref internal::FreeBlockPool::FreeBlockPool(). \n
 * _Constraints:_
 * - This must be equal to or larger than the minimum alignment.
 * - This must be equal to or less than the size of the managed memory.
 * \param nBuckets
 * Internally the @ref HeapManagerSPTS organizes free blocks of memory in buckets ordered by their size.
 * This is the number of buckets used. The maximum size for chunks of memory in the first bucket is given by
 * parameter `maxSizeInFirstBucket`. Each further bucket contains chunks of memory up to twice the size of the
 * chunks of memory in the previous bucket.\n
 * For details, please refer to @ref internal::FreeBlockPool::FreeBlockPool(). \n
 * _Constraints:_
 * - This must be [1;24]
 * - 2^(this-2) * `maxSizeInFirstBucket` must not exceed `size`.
 *
 * __Examples:__
 * ~~~{.cpp}
 * HeapManagerSPTS(16, 0, 2048, 32, 5);
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
 * HeapManagerSPTS(4, 0x1000, 4096, 8, 8);
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
}

void HeapManagerSPTS::Release(MemoryDescriptor* const pDescr)
/**
 * \brief Releases previously allocated memory.
 *
 * This is a private function offered to friend class @ref MemoryDescriptorSPTS only. It is invoked by
 * class @ref MemoryDescriptorSPTS upon destruction.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * Pointer to a @ref MemoryDescriptor instance encapsulated in a @ref MemoryDescriptorSPTS instance previously
 * retrieved from this @ref HeapManagerSPTS instance via @ref Allocate(). \n
 * _Ownership moves from the caller to the HeapManagerSPTS._
 */
{
  osal::MutexLocker mutexLocker(mutex);
  hm.Release(pDescr);
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc
