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

#include "gtest/gtest.h"
#include "gpcc/src/execution/async/WorkQueue.hpp"
#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/ResourceManagement/Memory/HeapManagerSPTS.hpp"
#include "gpcc/src/ResourceManagement/Memory/HeapManagerStatistics.hpp"
#include "gpcc/src/ResourceManagement/Memory/MemoryDescriptorSPTS.hpp"
#include <exception>
#include <limits>
#include <memory>
#include <vector>
#include <functional>

namespace gpcc_tests
{
namespace ResourceManagement
{
namespace Memory
{

using namespace gpcc;
using gpcc::execution::async::WorkPackage;
using namespace gpcc::ResourceManagement::Memory;
using namespace testing;

class GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests: public Test
{
  public:
    GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests(void);

  protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void Allocate(size_t const size, uint32_t const expectedAddress, size_t const expectedSize);
    bool CheckAndAddAllocation(std::shared_ptr<MemoryDescriptorSPTS> const & spDescr);
    bool AnyOverlapWithAllocations(std::shared_ptr<MemoryDescriptorSPTS> const & spDescr);
    void ClearAllocations(void) noexcept;

    void Allocate_WQ(size_t const size);
    void ReleaseAllocation_WQ(size_t const index);
    void GetStatistics_WQ(void);
    void CheckAllocations_WQ(void);

    bool InternalAnyOverlapWithAllocations(std::shared_ptr<MemoryDescriptorSPTS> const & spDescr);

    void* ThreadEntry(void);

    // The UUT.
    std::shared_ptr<HeapManagerSPTS> uut;

    // Mutex used to make accesses to "allocations" thread-safe.
    osal::Mutex mutex;

    // List with allocations done during the tests. "mutex" is required for access.
    std::vector<std::shared_ptr<MemoryDescriptorSPTS>> allocations;

    // Workqueue used by some tests.
    execution::async::WorkQueue wq;

    // Thread for wq.
    osal::Thread thread;
};

GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests(void)
: Test()
, uut()
, mutex()
, allocations()
, wq()
, thread("HeapManagerSPTS_Tests")
{
}

void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::SetUp(void)
{
  thread.Start(std::bind(&GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::ThreadEntry, this), osal::Thread::SchedPolicy::Other, 0, osal::Thread::GetDefaultStackSize());
  wq.FlushNonDeferredWorkPackages();
}
void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::TearDown(void)
{
  ClearAllocations();

  wq.RequestTermination();
  thread.Join(nullptr);
}

void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::Allocate(size_t const size, uint32_t const expectedAddress, size_t const expectedSize)
{
  // Allocates memory from UUT, checks the MemoryDescriptorSPTS (address, size, and overlap with existing allocations),
  // and finally enqueues the descriptor in "allocations".
  //
  // ------------------------------------------------------------------
  // Note: The caller must invoke HasFatalFailure() after calling this!
  // ------------------------------------------------------------------

  std::shared_ptr<MemoryDescriptorSPTS> spMD = uut->Allocate(size);
  ASSERT_TRUE(spMD != nullptr);

  ASSERT_EQ(expectedAddress, spMD->GetStartAddress());
  ASSERT_EQ(expectedSize, spMD->GetSize());

  ASSERT_TRUE(CheckAndAddAllocation(spMD));
}
bool GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::CheckAndAddAllocation(std::shared_ptr<MemoryDescriptorSPTS> const & spDescr)
{
  // Checks is a given descriptor (spDesc) refers to memory that is already allocated.
  // If it is not, then the descriptor is added to the list of allocations.

  osal::MutexLocker mutexLocker(mutex);
  if (InternalAnyOverlapWithAllocations(spDescr))
    return false;

  allocations.push_back(spDescr);

  return true;
}
bool GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::AnyOverlapWithAllocations(std::shared_ptr<MemoryDescriptorSPTS> const & spDescr)
{
  // Checks if a given descriptor (spDescr) refers to memory that is already allocated.

  osal::MutexLocker mutexLocker(mutex);
  return InternalAnyOverlapWithAllocations(spDescr);
}
void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::ClearAllocations(void) noexcept
{
  // Clears all allocations. Managed memory is returned to the UUT.

  osal::MutexLocker mutexLocker(mutex);
  allocations.clear();
}

void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::Allocate_WQ(size_t const size)
{
  // Allocates memory from the UUT, checks it and adds it to the list of allocations.
  // This may be executed in workqueue context.

  std::shared_ptr<MemoryDescriptorSPTS> spMD = uut->Allocate(size);
  if (!spMD)
    throw std::runtime_error("Allocate_WQ failed at #1");

  if (!CheckAndAddAllocation(spMD))
    throw std::runtime_error("Allocate_WQ failed at #2");
}
void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::ReleaseAllocation_WQ(size_t const index)
{
  // Releases an allocation from the list of allocation. The list slot will be nullptr afterwared.
  // This may be executed in workqueue context.

  osal::AdvancedMutexLocker mutexLocker(mutex);

  if (index >= allocations.size())
    throw std::runtime_error("ReleaseAllocation_WQ: Bad index");

  auto spMD = allocations[index];
  allocations[index] = nullptr;

  mutexLocker.Unlock();

  // now release it
  spMD = nullptr;
}
void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::GetStatistics_WQ(void)
{
  // Requests statistics from the UUT.
  // This may be executed in workqueue context.

  HeapManagerStatistics stat = uut->GetStatistics();
  (void)stat;
}
void GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::CheckAllocations_WQ(void)
{
  // Checks if there are any allocations.
  // This may be executed in workqueue context.

  bool const anyAllocs = uut->AnyAllocations();
  (void)anyAllocs;
}
bool GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::InternalAnyOverlapWithAllocations(std::shared_ptr<MemoryDescriptorSPTS> const & spDescr)
{
  // Checks if a given descriptor (spDescr) refers to memory that is already allocated.
  // This must only be invoked with "mutex" being locked by the calling thread.

  uint32_t const a1 = spDescr->GetStartAddress();
  size_t const s1   = spDescr->GetSize();

  for (auto & e: allocations)
  {
    if (e != nullptr)
    {
      uint32_t const a2 = e->GetStartAddress();
      size_t const   s2 = e->GetSize();

      if (((a1 >= a2) && (a1 < a2 + s2)) ||
          ((a2 >= a1) && (a2 < a1 + s1)))
        return true;
    }
  }

  return false;
}

void* GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests::ThreadEntry(void)
{
  wq.Work();
  return nullptr;
}

TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, Configuration)
{
  // HeapManagerSPTS::Create(uint16_t const _minimumAlignment,      >0, Power of 2
  //                         uint32_t const baseAddress,            Meet _minimumAlignment
  //                         size_t   const size,                   n * _minimumAlignment, n >= 1; baseAddress+size <= max_uint_32
  //                         size_t   const maxSizeInFirstBucket,   >= _minimumAlignment, <= size
  //                         size_t   const nBuckets);              1..24, 2^(this-2)*maxSizeInFirstBucket < size

  // buckets (64,4):
  // 0     1      2      3
  // <= 64 <= 128 <= 256 >256

  // _minimumAlignment (zero)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(0,   0, 1024, 64, 4), std::invalid_argument);

  // _minimumAlignment (not power of 2)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(3,   0, 1024, 64, 4), std::invalid_argument);
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(15,  0, 1024, 64, 4), std::invalid_argument);
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(17,  0, 1024, 64, 4), std::invalid_argument);

  // _minimumAlignment (OK)
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   0, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 64, 4));

  // baseAddress (does not meet _minimumAlignment)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16, 15, 1024, 64, 4), std::invalid_argument);
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16, 17, 1024, 64, 4), std::invalid_argument);

  // baseAddress (OK)
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   0, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   1, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   2, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   3, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   4, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16, 16, 1024, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16, 32, 1024, 64, 4));

  // size (zero)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0,    0, 64, 4), std::invalid_argument);

  // size (not multiple of _minimumAlignment)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0, 1023, 64, 4), std::invalid_argument);
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0, 1025, 64, 4), std::invalid_argument);
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0, 2044, 64, 4), std::invalid_argument);

  // size (baseAddress + size too large)
  size_t const bigBlock = (std::numeric_limits<uint32_t>::max() / 32U) * 32U;
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, bigBlock, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16, 16, bigBlock, 64, 4));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16, 32, bigBlock, 64, 4));
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16, 48, bigBlock, 64, 4), std::invalid_argument);

  // size (OK)
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0,   16, 16, 1));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0,   32, 16, 1));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 64, 4));

  // maxSizeInFirstBucket (< _minimumAlignment or > size)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0, 1024, 15,   1), std::invalid_argument);
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0, 1024, 1025, 4), std::invalid_argument);

  // maxSizeInFirstBucket (OK)
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 16, 1));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 1024, 1));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 99, 1));

  // nBuckets (1..24)
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(1,   0, bigBlock, 1,  0), std::invalid_argument);
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   0, bigBlock, 1,  1));
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(1,   0, bigBlock, 1,  24));
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(1,   0, bigBlock, 1,  25), std::invalid_argument);

  // nBuckets (2^(this-2)*maxSizeInFirstBucket < size)
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 16, 1));

  // buckets (16,7):
  // 0      1      2      3       4       5       6
  // <= 16  <= 32  <= 64  <= 128  <= 256  <= 512  > 512
  ASSERT_NO_THROW(uut = HeapManagerSPTS::Create(16,  0, 1024, 16, 7));

  // buckets (16,8):
  // 0      1      2      3       4       5       6        7
  // <= 16  <= 32  <= 64  <= 128  <= 256  <= 512  <= 1024  > 1024
  ASSERT_THROW(   uut = HeapManagerSPTS::Create(16,  0, 1024, 16,  8), std::invalid_argument);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, AllocateZero)
{
  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  std::shared_ptr<MemoryDescriptorSPTS> spMD;
  ASSERT_THROW(spMD = uut->Allocate(0), std::invalid_argument);
  ASSERT_TRUE(spMD == nullptr);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, AllocateTooMany)
{
  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  std::shared_ptr<MemoryDescriptorSPTS> spMD = uut->Allocate(1025);
  ASSERT_TRUE(spMD == nullptr);

  spMD = uut->Allocate(std::numeric_limits<size_t>::max());
  ASSERT_TRUE(spMD == nullptr);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, AllocateAllIn1Block)
{
  // This test allocates the whole managed memory in 1 chunk, releases it, and allocates it a second time.
  // Expected address of allocation, statistics, and AnyAllocations() are checked.

  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  // check statistics, no allocations
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);

  ASSERT_FALSE(uut->AnyAllocations());

  for (int j = 0; j < 2; j++)
  {
    std::shared_ptr<MemoryDescriptorSPTS> spMD = uut->Allocate(1024);
    ASSERT_TRUE(spMD != nullptr);

    ASSERT_EQ(0U,    spMD->GetStartAddress());
    ASSERT_EQ(1024U, spMD->GetSize());

    ASSERT_TRUE(uut->AnyAllocations());

    // check statistics, 0% free
    stat = uut->GetStatistics();
    ASSERT_EQ(0U,     stat.nbOfFreeBlocks);
    ASSERT_EQ(1U,     stat.nbOfAllocatedBlocks);
    ASSERT_EQ(0U,     stat.totalFreeSpace);
    ASSERT_EQ(1024U,  stat.totalUsedSpace);

    spMD = nullptr;

    ASSERT_FALSE(uut->AnyAllocations());

    // check statistics, no allocations
    stat = uut->GetStatistics();
    ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
    ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
    ASSERT_EQ(1024U, stat.totalFreeSpace);
    ASSERT_EQ(0U,    stat.totalUsedSpace);
  }
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, AllocateAllIn32Blocks)
{
  // This test allocates the whole managed memory in 32 chunks, releases it, and allocates it a second time.
  // Expected addresses of allocations, statistics, and AnyAllocations() are checked.

  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  // check statistics, no allocations
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);

  for (int j = 0; j < 2; j++)
  {
    ASSERT_FALSE(uut->AnyAllocations());

    // allocate all memory
    for (size_t i = 0; i < 32; i++)
    {
      Allocate(32U, i * 32U, 32U);
      if (HasFatalFailure())
        return;

      ASSERT_TRUE(uut->AnyAllocations());
    }

    // check statistics, 0% free
    stat = uut->GetStatistics();
    ASSERT_EQ(0U,     stat.nbOfFreeBlocks);
    ASSERT_EQ(32U,    stat.nbOfAllocatedBlocks);
    ASSERT_EQ(0U,     stat.totalFreeSpace);
    ASSERT_EQ(1024U,  stat.totalUsedSpace);

    ClearAllocations();

    // check statistics, 100% free
    stat = uut->GetStatistics();
    ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
    ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
    ASSERT_EQ(1024U, stat.totalFreeSpace);
    ASSERT_EQ(0U,    stat.totalUsedSpace);
  }
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, Alignment)
{
  // This test allocates blocks of different size and checks address alignment and size
  // of allocated blocks.

  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  for (size_t s = 1; s <= 43; s++)
  {
    std::shared_ptr<MemoryDescriptorSPTS> spMD = uut->Allocate(s);
    ASSERT_TRUE(spMD != nullptr);

    ASSERT_TRUE((spMD->GetStartAddress() % 4) == 0);
    ASSERT_TRUE(spMD->GetSize() >= s);

    CheckAndAddAllocation(spMD);
  }

  ClearAllocations();

  // check statistics, 100% free
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, Buckets)
{
  // This test allocates memory with different sizes corresponding to the
  // HeapManager's bucket sizes. Between the allocations are extra allocations
  // of 4 byte each. Then the first mentioned allocations are released. The extra
  // allocations prevent recombination of the free space.
  // Finally the memory is reallocated, but in inverse order. The inverse order
  // ensures, that we can see that the HeapManager trys small buckets first.
  // The addresses of the allocations in the second run must match the addresses
  // from first allocation.

  // buckets (16,6):
  // 0     1      2      3      4       5       6
  // <= 8  <= 16  <= 32  <= 64  <= 128  <= 256  > 256
  uut = HeapManagerSPTS::Create(4, 0, 1024, 8, 7);

  // check statistics, 100% free
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);

  // Allocate some memory: 256, 128, 64, 32, 16, 8 bytes
  // Between each allocation, a 4 byte allocation is inserted, so we get:
  // 256 - 4 - 128 - 4 - 64 - 4 - 32 - 4 - 16 - 4 - 8 - 4
  Allocate(256, 0, 256);
  if (HasFatalFailure())
    return;

  Allocate(4, 256, 4);
  if (HasFatalFailure())
    return;

  Allocate(128, 260, 128);
  if (HasFatalFailure())
    return;

  Allocate(4, 388, 4);
  if (HasFatalFailure())
    return;

  Allocate(64, 392, 64);
  if (HasFatalFailure())
    return;

  Allocate(4, 456, 4);
  if (HasFatalFailure())
    return;

  Allocate(32, 460, 32);
  if (HasFatalFailure())
    return;

  Allocate(4, 492, 4);
  if (HasFatalFailure())
    return;

  Allocate(16, 496, 16);
  if (HasFatalFailure())
    return;

  Allocate(4, 512, 4);
  if (HasFatalFailure())
    return;

  Allocate(8, 516, 8);
  if (HasFatalFailure())
    return;

  Allocate(4, 524, 4);
  if (HasFatalFailure())
    return;

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(12U,  stat.nbOfAllocatedBlocks);
  ASSERT_EQ(496U, stat.totalFreeSpace);
  ASSERT_EQ(528U, stat.totalUsedSpace);

  // Remove all non-4-byte allocations:
  std::vector<std::shared_ptr<MemoryDescriptorSPTS>> remainingAllocs;
  {
    osal::MutexLocker mutexLocker(mutex);
    for (auto & e: allocations)
    {
      if (e->GetSize() == 4)
        remainingAllocs.push_back(e);
    }
    allocations = std::move(remainingAllocs);
  }

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(7U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(6U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1000U, stat.totalFreeSpace);
  ASSERT_EQ(24U,   stat.totalUsedSpace);

  // Reallocate in reverse order and watch start addresses!
  Allocate(16, 496, 16);
  if (HasFatalFailure())
    return;

  Allocate(32, 460, 32);
  if (HasFatalFailure())
    return;

  Allocate(64, 392, 64);
  if (HasFatalFailure())
    return;

  Allocate(128, 260, 128);
  if (HasFatalFailure())
    return;

  Allocate(256, 0, 256);
  if (HasFatalFailure())
    return;

  Allocate(8, 516, 8);
  if (HasFatalFailure())
    return;

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(12U,  stat.nbOfAllocatedBlocks);
  ASSERT_EQ(496U, stat.totalFreeSpace);
  ASSERT_EQ(528U, stat.totalUsedSpace);

  ClearAllocations();

  // check statistics, 100% free
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, Recombination)
{
  // buckets (16,6):
  // 0     1      2      3      4       5       6
  // <= 8  <= 16  <= 32  <= 64  <= 128  <= 256  > 256
  uut = HeapManagerSPTS::Create(4, 0, 1024, 8, 7);

  // check statistics, 100% free
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);

  // Allocate some memory:
  // 16 - 16 - 4 - 32 - 32 - 4 - 64 - 64 - 64 - 4
  // 0    1    2   3    4    5   6    7    8    9
  Allocate(16, 0, 16);
  if (HasFatalFailure())
    return;

  Allocate(16, 16, 16);
  if (HasFatalFailure())
    return;

  Allocate(4, 32, 4);
  if (HasFatalFailure())
    return;

  Allocate(32, 36, 32);
  if (HasFatalFailure())
    return;

  Allocate(32, 68, 32);
  if (HasFatalFailure())
    return;

  Allocate(4, 100, 4);
  if (HasFatalFailure())
    return;

  Allocate(64, 104, 64);
  if (HasFatalFailure())
    return;

  Allocate(64, 168, 64);
  if (HasFatalFailure())
    return;

  Allocate(64, 232, 64);
  if (HasFatalFailure())
    return;

  Allocate(4, 296, 4);
  if (HasFatalFailure())
    return;

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(10U,  stat.nbOfAllocatedBlocks);
  ASSERT_EQ(724U, stat.totalFreeSpace);
  ASSERT_EQ(300U, stat.totalUsedSpace);

  // release the 16 byte blocks (recombination with left block)
  {
    osal::MutexLocker mutexLocker(mutex);
    allocations[0] = nullptr;
    allocations[1] = nullptr;
  }

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(2U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(8U,   stat.nbOfAllocatedBlocks);
  ASSERT_EQ(756U, stat.totalFreeSpace);
  ASSERT_EQ(268U, stat.totalUsedSpace);

  // release the 32 byte blocks (recombination with right block)
  {
    osal::MutexLocker mutexLocker(mutex);
    allocations[4] = nullptr;
    allocations[3] = nullptr;
  }

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(3U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(6U,  stat.nbOfAllocatedBlocks);
  ASSERT_EQ(820U, stat.totalFreeSpace);
  ASSERT_EQ(204U, stat.totalUsedSpace);

  // release the 64 byte blocks (recombination with both left and right)
  {
    osal::MutexLocker mutexLocker(mutex);
    allocations[6] = nullptr;
    allocations[8] = nullptr;
    allocations[7] = nullptr;
  }

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(4U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(3U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1012U, stat.totalFreeSpace);
  ASSERT_EQ(12U,   stat.totalUsedSpace);

  // allocate storage and check addresses
  Allocate(32, 0, 32);
  if (HasFatalFailure())
    return;

  Allocate(64, 36, 64);
  if (HasFatalFailure())
    return;

  Allocate(192, 104, 192);
  if (HasFatalFailure())
    return;

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(6U,   stat.nbOfAllocatedBlocks);
  ASSERT_EQ(724U, stat.totalFreeSpace);
  ASSERT_EQ(300U, stat.totalUsedSpace);

  ClearAllocations();

  // check statistics, 100% free
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, HeapManagerReleasedFirst)
{
  // During this test, the last shared pointer to the HeapManagerSPTS is dropped before
  // the last pointers to the allocations are dropped.

  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  Allocate(16, 0, 16);
  if (HasFatalFailure())
    return;

  Allocate(16, 16, 16);
  if (HasFatalFailure())
    return;

  uut = nullptr;
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)
{
  // This test checks access to the UUT from different threads.
  // This allows checks by Helgrind and similar tools.

  uut = HeapManagerSPTS::Create(4, 0, 1024, 16, 6);

  // allocate some memory in the context of a different thread
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)::Allocate_WQ, this, 32)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)::Allocate_WQ, this, 48)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)::Allocate_WQ, this, 16)));

  wq.FlushNonDeferredWorkPackages();

  // allocate some memory from this thread
  std::shared_ptr<MemoryDescriptorSPTS> spMD = uut->Allocate(32);
  ASSERT_TRUE(spMD != nullptr);
  ASSERT_TRUE(CheckAndAddAllocation(spMD));

  // check allocations from this thread
  osal::AdvancedMutexLocker mutexLocker(mutex);
  ASSERT_EQ(4U,  allocations.size());
  ASSERT_EQ(0U,  allocations[0]->GetStartAddress());
  ASSERT_EQ(32U, allocations[0]->GetSize());
  ASSERT_EQ(32U, allocations[1]->GetStartAddress());
  ASSERT_EQ(48U, allocations[1]->GetSize());
  ASSERT_EQ(80U, allocations[2]->GetStartAddress());
  ASSERT_EQ(16U, allocations[2]->GetSize());
  ASSERT_EQ(96U, allocations[3]->GetStartAddress());
  ASSERT_EQ(32U, allocations[3]->GetSize());
  mutexLocker.Unlock();

  // Do the following in the context of a different thread:
  // - release one allocation
  // - retrieve statistics
  // - check for any allocation
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)::ReleaseAllocation_WQ, this, 1)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)::GetStatistics_WQ, this)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(GPCC_ResourceManagement_Memory_HeapManagerSPTS_Tests, DifferentThreads)::CheckAllocations_WQ, this)));
  wq.FlushNonDeferredWorkPackages();

  // release one allocation in the context of this thread
  mutexLocker.Relock();
  allocations[0] = nullptr;
  mutexLocker.Unlock();

  // check statistics from this thread
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(2U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(2U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(976U,  stat.totalFreeSpace);
  ASSERT_EQ(48U,   stat.totalUsedSpace);

  // check for allocations from this thread
  ASSERT_TRUE(uut->AnyAllocations());
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc_tests
