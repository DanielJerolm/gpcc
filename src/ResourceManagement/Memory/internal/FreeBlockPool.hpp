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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_FREEBLOCKPOOL_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_FREEBLOCKPOOL_HPP_

#include <vector>
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
 * \brief A pool for @ref MemoryDescriptor instances referencing (unused/free) memory of different size.
 *
 * The pool organizes the @ref MemoryDescriptor instances in different lists based on the size of the
 * memory referenced by the @ref MemoryDescriptor instances. The lists are called buckets.
 *
 * Each bucket contains @ref MemoryDescriptor instances which reference memory of up to twice the size
 * of the @ref MemoryDescriptor instances in the previous bucket.
 *
 * The pool uses `MemoryDescriptor::pNextInList` and `MemoryDescriptor::pPrevInList` to build the lists.\n
 * `MemoryDescriptor::pPrevInMem` and `MemoryDescriptor::pNextInMem` are not accessed by this.
 */
class FreeBlockPool
{
  public:
    FreeBlockPool(void) = delete;
    FreeBlockPool(size_t const _maxSizeInFirstBucket, size_t const nBuckets);
    FreeBlockPool(FreeBlockPool const &) = delete;
    FreeBlockPool(FreeBlockPool&&) = delete;
    ~FreeBlockPool(void);

    FreeBlockPool& operator=(FreeBlockPool const &) = delete;
    FreeBlockPool& operator=(FreeBlockPool&&) = delete;

    void Add(MemoryDescriptor* const pDescr) noexcept;
    void Remove(MemoryDescriptor* const pDescr) noexcept;
    MemoryDescriptor* Get(size_t const minimumRequiredSize) noexcept;

  private:
    /// Maximum size for @ref MemoryDescriptor instances in the first bucket.
    size_t const maxSizeInFirstBucket;

    /// List with buckets.
    /** Each list entry represents one bucket that contains @ref MemoryDescriptor instances of a specific size:\n
        Bucket #0 contains @ref MemoryDescriptor instances up to size 1 * @ref maxSizeInFirstBucket. \n
        Bucket #1 contains @ref MemoryDescriptor instances up to size 2 * @ref maxSizeInFirstBucket. \n
        Bucket #2 contains @ref MemoryDescriptor instances up to size 4 * @ref maxSizeInFirstBucket. \n
        ...\n
        Bucket #i contains @ref MemoryDescriptor instances up to size (2^i) * @ref maxSizeInFirstBucket. \n
        ...\n
        Bucket #n-1 contains @ref MemoryDescriptor instances up to size (2^(n-1)) * @ref maxSizeInFirstBucket. \n
        Bucket #n contains @ref MemoryDescriptor instances larger than (2^(n-1)) * @ref maxSizeInFirstBucket. \n
        \n
        Any @ref MemoryDescriptor located in bucket `i` does not fit into bucket `i-1`.\n
        \n
        The @ref MemoryDescriptor instances in a bucket are organized in a double linked list made up by
        @ref MemoryDescriptor::pPrevInList and @ref MemoryDescriptor::pNextInList.
        Each list entry refers to either nothing (nullptr, empty bucket) or the head of a double linked list of
        @ref MemoryDescriptor instances. */
    std::vector<MemoryDescriptor*> buckets;


    size_t DetermineBucketIndex(size_t const size) const noexcept;
    void RemoveFromBucket(MemoryDescriptor* const pDescr, size_t const index) noexcept;
    void ReleaseAllDescriptorsInBucket(MemoryDescriptor* pHead) noexcept;
};

/**
 * @}
 */

} // namespace internal
} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_MEMORY_INTERNAL_FREEBLOCKPOOL_HPP_
