/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gtest/gtest.h"
#include "gpcc/src/ResourceManagement/Memory/HeapManager.hpp"
#include "gpcc/src/ResourceManagement/Memory/HeapManagerStatistics.hpp"
#include "gpcc/src/ResourceManagement/Memory/MemoryDescriptor.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <limits>
#include <memory>
#include <vector>

namespace gpcc_tests
{
namespace ResourceManagement
{
namespace Memory
{

using namespace gpcc::ResourceManagement::Memory;
using namespace testing;

class GPCC_ResourceManagement_Memory_HeapManager_Tests: public Test
{
  public:
    GPCC_ResourceManagement_Memory_HeapManager_Tests(void);

  protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void Allocate(size_t const size, uint32_t const expectedAddress, size_t const expectedSize);
    bool AnyOverlapWithAllocations(MemoryDescriptor const * const pDescr);
    void ReleaseAllocations(void);

    std::unique_ptr<HeapManager> uut;
    std::vector<MemoryDescriptor*> allocations;
};

GPCC_ResourceManagement_Memory_HeapManager_Tests::GPCC_ResourceManagement_Memory_HeapManager_Tests(void)
: Test()
, uut()
, allocations()
{
}

void GPCC_ResourceManagement_Memory_HeapManager_Tests::SetUp(void)
{
}
void GPCC_ResourceManagement_Memory_HeapManager_Tests::TearDown(void)
{
  ReleaseAllocations();
}

void GPCC_ResourceManagement_Memory_HeapManager_Tests::Allocate(size_t const size, uint32_t const expectedAddress, size_t const expectedSize)
{
  // Note: Caller must use HasFatalFailure() after invoking this!

  MemoryDescriptor* const pMD = uut->Allocate(size);
  ASSERT_TRUE(pMD != nullptr);
  ON_SCOPE_EXIT() { uut->Release(pMD); };
  ASSERT_EQ(expectedAddress, pMD->GetStartAddress());
  ASSERT_EQ(expectedSize, pMD->GetSize());
  ASSERT_FALSE(AnyOverlapWithAllocations(pMD));
  allocations.push_back(pMD);
  ON_SCOPE_EXIT_DISMISS();
}
bool GPCC_ResourceManagement_Memory_HeapManager_Tests::AnyOverlapWithAllocations(MemoryDescriptor const * const pDescr)
{
  uint32_t const a1 = pDescr->GetStartAddress();
  size_t const s1   = pDescr->GetSize();

  for (auto e: allocations)
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
void GPCC_ResourceManagement_Memory_HeapManager_Tests::ReleaseAllocations(void)
{
  if (uut)
  {
    for (auto e: allocations)
      if (e != nullptr)
        uut->Release(e);
  }
  allocations.clear();
}

TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, Configuration)
{
  // HeapManager(uint16_t const _minimumAlignment,      >0, Power of 2
  //             uint32_t const baseAddress,            Meet _minimumAlignment
  //             size_t   const size,                   n * _minimumAlignment, n >= 1; baseAddress+size <= max_uint_32
  //             size_t   const maxSizeInFirstBucket,   >= _minimumAlignment, <= size
  //             size_t   const nBuckets);              1..24, 2^(this-2)*maxSizeInFirstBucket < size

  // buckets (64,4):
  // 0     1      2      3
  // <= 64 <= 128 <= 256 >256

  // _minimumAlignment (zero)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(0,   0, 1024, 64, 4)), std::invalid_argument);

  // _minimumAlignment (not power of 2)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(3,   0, 1024, 64, 4)), std::invalid_argument);
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(15,  0, 1024, 64, 4)), std::invalid_argument);
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(17,  0, 1024, 64, 4)), std::invalid_argument);

  // _minimumAlignment (OK)
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  0,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024, 64, 4)));

  // baseAddress (does not meet _minimumAlignment)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 15, 1024, 64, 4)), std::invalid_argument);
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 17, 1024, 64, 4)), std::invalid_argument);

  // baseAddress (OK)
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  0,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  1,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  2,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  3,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  4,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 16, 1024, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 32, 1024, 64, 4)));

  // size (zero)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  0,    64, 4)), std::invalid_argument);

  // size (not multiple of _minimumAlignment)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  1023, 64, 4)), std::invalid_argument);
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  1025, 64, 4)), std::invalid_argument);
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  2044, 64, 4)), std::invalid_argument);

  // size (baseAddress + size too large)
  size_t const bigBlock = (std::numeric_limits<uint32_t>::max() / 32U) * 32U;
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  bigBlock, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 16, bigBlock, 64, 4)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 32, bigBlock, 64, 4)));
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 48, bigBlock, 64, 4)), std::invalid_argument);

  // size (OK)
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  16,   16, 1)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  32,   16, 1)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024, 64, 4)));

  // maxSizeInFirstBucket (< _minimumAlignment or > size)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  1024, 15, 1)), std::invalid_argument);
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  1024, 1025, 1)), std::invalid_argument);

  // maxSizeInFirstBucket (OK)
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024, 16, 1)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024, 1024, 1)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024, 99, 1)));

  // nBuckets (1..24)
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(1,  0,  bigBlock, 1,  0)), std::invalid_argument);
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  0,  bigBlock, 1,  1)));
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(1,  0,  bigBlock, 1,  24)));
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(1,  0,  bigBlock, 1,  25)), std::invalid_argument);


  // nBuckets (2^(this-2)*maxSizeInFirstBucket < size)
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024,     16, 1)));

  // buckets (16,7):
  // 0      1      2      3       4       5       6
  // <= 16  <= 32  <= 64  <= 128  <= 256  <= 512  > 512
  ASSERT_NO_THROW(uut = std::unique_ptr<HeapManager>(new HeapManager(16, 0,  1024,     16, 7)));

  // buckets (16,8):
  // 0      1      2      3       4       5       6        7
  // <= 16  <= 32  <= 64  <= 128  <= 256  <= 512  <= 1024  > 1024
  ASSERT_THROW(uut = std::unique_ptr<HeapManager>(new    HeapManager(16, 0,  1024,     16, 8)), std::invalid_argument);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, AllocateZero)
{
  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

  MemoryDescriptor* pMD = nullptr;
  ASSERT_THROW(pMD = uut->Allocate(0), std::invalid_argument);
  ASSERT_TRUE(pMD == nullptr);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, AllocateTooMany)
{
  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

  MemoryDescriptor* pMD = uut->Allocate(1025);
  ASSERT_TRUE(pMD == nullptr);

  pMD = uut->Allocate(std::numeric_limits<size_t>::max());
  ASSERT_TRUE(pMD == nullptr);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, AllocateAllIn1Block)
{
  // This test allocates the whole managed memory in 1 chunk, releases it, and allocates it a second time.
  // Expected address of allocation, statistics, and AnyAllocations() are checked.

  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

  // check statistics, no allocations
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);

  ASSERT_FALSE(uut->AnyAllocations());

  for (int j = 0; j < 2; j++)
  {
    MemoryDescriptor* const pMD = uut->Allocate(1024);
    ASSERT_TRUE(pMD != nullptr);
    ON_SCOPE_EXIT() { uut->Release(pMD); };

    ASSERT_EQ(0U,    pMD->GetStartAddress());
    ASSERT_EQ(1024U, pMD->GetSize());

    ASSERT_TRUE(uut->AnyAllocations());

    // check statistics, 0% free
    stat = uut->GetStatistics();
    ASSERT_EQ(0U,     stat.nbOfFreeBlocks);
    ASSERT_EQ(1U,     stat.nbOfAllocatedBlocks);
    ASSERT_EQ(0U,     stat.totalFreeSpace);
    ASSERT_EQ(1024U,  stat.totalUsedSpace);

    ON_SCOPE_EXIT_DISMISS();
    uut->Release(pMD);

    ASSERT_FALSE(uut->AnyAllocations());

    // check statistics, no allocations
    stat = uut->GetStatistics();
    ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
    ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
    ASSERT_EQ(1024U, stat.totalFreeSpace);
    ASSERT_EQ(0U,    stat.totalUsedSpace);
  }
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, AllocateAllIn32Blocks)
{
  // This test allocates the whole managed memory in 32 chunks, releases it, and allocates it a second time.
  // Expected addresses of allocations, statistics, and AnyAllocations() are checked.

  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

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
      MemoryDescriptor* const pMD = uut->Allocate(32);
      ASSERT_TRUE(pMD != nullptr);
      ON_SCOPE_EXIT() { uut->Release(pMD); };
      ASSERT_FALSE(AnyOverlapWithAllocations(pMD));
      allocations.push_back(pMD);
      ON_SCOPE_EXIT_DISMISS();

      ASSERT_EQ(i * 32U,  pMD->GetStartAddress());
      ASSERT_EQ(32U, pMD->GetSize());

      ASSERT_TRUE(uut->AnyAllocations());
    }

    // check statistics, 0% free
    stat = uut->GetStatistics();
    ASSERT_EQ(0U,     stat.nbOfFreeBlocks);
    ASSERT_EQ(32U,    stat.nbOfAllocatedBlocks);
    ASSERT_EQ(0U,     stat.totalFreeSpace);
    ASSERT_EQ(1024U,  stat.totalUsedSpace);

    // release all allocations
    ReleaseAllocations();

    // check statistics, 100% free
    stat = uut->GetStatistics();
    ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
    ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
    ASSERT_EQ(1024U, stat.totalFreeSpace);
    ASSERT_EQ(0U,    stat.totalUsedSpace);
  }
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, Alignment)
{
  // This test allocates blocks of different size and checks address alignment and size
  // of allocated blocks.

  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

  for (size_t s = 1; s <= 43; s++)
  {
    MemoryDescriptor* const pMD = uut->Allocate(s);
    ASSERT_TRUE(pMD != nullptr);
    ON_SCOPE_EXIT() { uut->Release(pMD); };
    ASSERT_FALSE(AnyOverlapWithAllocations(pMD));
    allocations.push_back(pMD);
    ON_SCOPE_EXIT_DISMISS();

    ASSERT_TRUE((pMD->GetStartAddress() % 4) == 0);
    ASSERT_TRUE(pMD->GetSize() >= s);
  }

  ReleaseAllocations();

  // check statistics, 100% free
  HeapManagerStatistics stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, Release_nullptr)
{
  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

  ASSERT_THROW(uut->Release(nullptr), std::invalid_argument);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, Release_twice)
{
  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 16, 6));

  MemoryDescriptor* pMD = uut->Allocate(12);
  ASSERT_TRUE(pMD != nullptr);
  ASSERT_NO_THROW(uut->Release(pMD));
  ASSERT_THROW(uut->Release(pMD), std::logic_error);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, Buckets)
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
  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 8, 7));

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
  std::vector<MemoryDescriptor*> remainingAllocs;
  for (auto & e: allocations)
  {
    if (e->GetSize() == 4)
      remainingAllocs.push_back(e);
    else
    {
      uut->Release(e);
      e = nullptr;
    }
  }
  allocations = std::move(remainingAllocs);

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

  // release all
  ReleaseAllocations();

  // check statistics, 100% free
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);
}
TEST_F(GPCC_ResourceManagement_Memory_HeapManager_Tests, Recombination)
{
  // buckets (16,6):
  // 0     1      2      3      4       5       6
  // <= 8  <= 16  <= 32  <= 64  <= 128  <= 256  > 256
  uut = std::unique_ptr<HeapManager>(new HeapManager(4, 0, 1024, 8, 7));

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
  uut->Release(allocations[0]);
  allocations[0] = nullptr;
  uut->Release(allocations[1]);
  allocations[1] = nullptr;

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(2U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(8U,   stat.nbOfAllocatedBlocks);
  ASSERT_EQ(756U, stat.totalFreeSpace);
  ASSERT_EQ(268U, stat.totalUsedSpace);

  // release the 32 byte blocks (recombination with right block)
  uut->Release(allocations[4]);
  allocations[4] = nullptr;
  uut->Release(allocations[3]);
  allocations[3] = nullptr;

  // check statistics
  stat = uut->GetStatistics();
  ASSERT_EQ(3U,   stat.nbOfFreeBlocks);
  ASSERT_EQ(6U,  stat.nbOfAllocatedBlocks);
  ASSERT_EQ(820U, stat.totalFreeSpace);
  ASSERT_EQ(204U, stat.totalUsedSpace);

  // release the 64 byte blocks (recombination with both left and right)
  uut->Release(allocations[6]);
  allocations[6] = nullptr;
  uut->Release(allocations[8]);
  allocations[8] = nullptr;
  uut->Release(allocations[7]);
  allocations[7] = nullptr;

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

  ReleaseAllocations();

  // check statistics, 100% free
  stat = uut->GetStatistics();
  ASSERT_EQ(1U,    stat.nbOfFreeBlocks);
  ASSERT_EQ(0U,    stat.nbOfAllocatedBlocks);
  ASSERT_EQ(1024U, stat.totalFreeSpace);
  ASSERT_EQ(0U,    stat.totalUsedSpace);
}

} // namespace Memory
} // namespace ResourceManagement
} // namespace gpcc_tests
