/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "EEPROMSectionSystemTestFixture.hpp"
#include "RandomData.hpp"
#include "gpcc/src/file_systems/EEPROMSectionSystem/Exceptions.hpp"
#include "gpcc/src/file_systems/exceptions.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstdlib>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

using namespace testing;
using namespace gpcc::file_systems::EEPROMSectionSystem;
using namespace gpcc::file_systems::EEPROMSectionSystem::internal;

typedef EEPROMSectionSystemTestFixture GPCC_FileSystems_EEPROMSectionSystem_TestsF;
typedef GPCC_FileSystems_EEPROMSectionSystem_TestsF GPCC_FileSystems_EEPROMSectionSystem_DeathTestsF;

static void BasicTest_WriteRead(EEPROMSectionSystem::EEPROMSectionSystem* pUUT, uint16_t const blockSize, uint16_t const additionalDepth)
{
  // Performs basic tests on an UUT:
  // - Write sections until all space exhausted.
  // - Read all sections back & delete them.
  // - if additionalDepth is not zero, then this will be recursively invoked with additionalDepth-1 before
  //   each section creation and after each section deletion.
  try
  {
    if (pUUT == nullptr)
      throw std::runtime_error("BasicTests_WriteRead: !pUUT");

    std::list<RandomData> referenceCopy;
    size_t const initialFreeSpace = pUUT->GetFreeSpace();

    size_t freeSpace = initialFreeSpace;
    uint16_t i = 0;
    while (freeSpace >= 8U)
    {
      if (additionalDepth != 0)
        BasicTest_WriteRead(pUUT, blockSize, additionalDepth - 1);

      std::string const secName = "Section" + std::to_string(i) + "R" + std::to_string(additionalDepth);
      RandomData rndData(0, freeSpace - 8U);
      rndData.Write(secName, false, *pUUT);

      referenceCopy.push_back(std::move(rndData));
      freeSpace = pUUT->GetFreeSpace();
      i++;
    }

    i = 0;
    for (auto const & e: referenceCopy)
    {
      std::string const secName = "Section" + std::to_string(i) + "R" + std::to_string(additionalDepth);
      e.Compare(secName, *pUUT);
      pUUT->Delete(secName);
      i++;

      if (additionalDepth != 0)
        BasicTest_WriteRead(pUUT, blockSize, additionalDepth - 1);
    }

    if (initialFreeSpace != pUUT->GetFreeSpace())
      throw std::runtime_error("BasicTest_WriteRead: Free space before/after mismatch");
  }
  catch (std::exception const & e)
  {
    std::cout << "BasicTest_WriteRead failed: " << e.what() << std::endl;
    throw;
  }
}
static void BasicTest_FormatWriteRead(EEPROMSectionSystem::EEPROMSectionSystem* pUUT, uint16_t const blockSize, uint16_t const additionalDepth)
{
  // Performs basic tests on an UUT:
  // - Format
  // - Write sections until all space exhausted
  // - Read all sections back & delete them
  // - Unmount
  // - if additionalDepth is not zero, then BasicTest_WriteRead() will be recursively invoked with
  //   additionalDepth-1 before each section creation and after each section deletion
  try
  {
    if (pUUT == nullptr)
      throw std::runtime_error("BasicTest_FormatWriteRead: !pUUT");

    pUUT->Format(blockSize);
    ON_SCOPE_EXIT() { pUUT->Unmount(); };

    std::list<RandomData> referenceCopy;
    size_t const initialFreeSpace = pUUT->GetFreeSpace();

    size_t freeSpace = initialFreeSpace;
    uint16_t i = 0;
    while (freeSpace >= 8U)
    {
      if (additionalDepth != 0)
        BasicTest_WriteRead(pUUT, blockSize, additionalDepth - 1);

      std::string const secName = "Section" + std::to_string(i) + "R" + std::to_string(additionalDepth);
      RandomData rndData(0, freeSpace - 8U);
      rndData.Write(secName, false, *pUUT);

      referenceCopy.push_back(std::move(rndData));
      freeSpace = pUUT->GetFreeSpace();
      i++;
    }

    i = 0;
    for (auto const & e: referenceCopy)
    {
      std::string const secName = "Section" + std::to_string(i) + "R" + std::to_string(additionalDepth);
      e.Compare(secName, *pUUT);
      pUUT->Delete(secName);
      i++;

      if (additionalDepth != 0)
        BasicTest_WriteRead(pUUT, blockSize, additionalDepth - 1);
    }

    if (initialFreeSpace != pUUT->GetFreeSpace())
      throw std::runtime_error("BasicTest_FormatWriteRead: Free space before/after mismatch");
  }
  catch (std::exception const & e)
  {
    std::cout << "BasicTest_FormatWriteRead failed: " << e.what() << std::endl;
    throw;
  }
}

TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Construction_StartAddress)
{
  FakeEEPROM fakeStorage(1024, 64);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT;

  ASSERT_NO_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, 960)));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), 64, 1));

  ASSERT_NO_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 64, 960)));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), 64, 1));

  ASSERT_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 32, 960)), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Construction_Size)
{
  FakeEEPROM fakeStorageMBS(1024, EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize);
  FakeEEPROM fakeStorage64(1024, 64);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT;

  // different sizes
  ASSERT_NO_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage64, 0, 960)));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), 64, 1));

  ASSERT_NO_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage64, 0, 1024)));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), 64, 1));

  // not a whole numbered multiple of the page size
  ASSERT_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage64, 0, 1000)), std::invalid_argument);

  // minimum number of blocks
  ASSERT_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorageMBS, 0, 2 * EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize)), std::invalid_argument);
  ASSERT_NO_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorageMBS, 0, 3 * EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize)));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize, 1));

  // out-of-bounds
  ASSERT_THROW(spUUT.reset(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage64, 64, 1024)), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_DeathTestsF, Destruction_BadState)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  FakeEEPROM fakeStorage64(1024, 64);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage64, 0, 960));
  spUUT->Format(64);
  EXPECT_DEATH(spUUT.reset(), ".*gpcc/src/file_systems/EEPROMSectionSystem/EEPROMSectionSystem.cpp.*");

  spUUT->Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_BadState)
{
  Format(storagePageSize);

  ASSERT_THROW(uut.MountStep1(), InsufficientStateError);

  uut.Unmount();

  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_THROW(uut.MountStep1(), InsufficientStateError);

  uut.Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, MountStep1_StoragePageSizeTooSmall)
{
  FakeEEPROM fakeStorage(1024, 16);
  EEPROMSectionSystem::EEPROMSectionSystem uut(fakeStorage, 0, 1024);

  ASSERT_THROW(uut.MountStep1(), std::logic_error);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_BlankStorage)
{
  // note: fakeStorage is initialized with zeros by FakeEEPROM::FakeEEPROM()
  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::BadSectionSystemInfoBlockError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_BadCRC)
{
  Format(128);
  uut.Unmount();

  InvalidateCRC(0);

  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::BadSectionSystemInfoBlockError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_WrongType_freeBlock)
{
  Format(128);
  uut.Unmount();

  pBuffer[0] = static_cast<uint8_t>(BlockTypes::freeBlock); // type
  pBuffer[1] = 0; // sectionNameHash
  pBuffer[2] = 12; // nBytes (LB)
  pBuffer[3] = 0; // nBytes (HB)
  pBuffer[4] = 0; // totalNbOfWrites LB
  pBuffer[5] = 0; // ...
  pBuffer[6] = 0; // ...
  pBuffer[7] = 0; // totalNbOfWrites
  pBuffer[8] = NOBLOCK & 0xFFU; // nextBlock LB
  pBuffer[9] = NOBLOCK >> 8U; // nextBlock HB

  fakeStorage.Write(0, 10, pBuffer);
  UpdateCRC(0);

  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::BadSectionSystemInfoBlockError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_WrongType_sectionHead)
{
  Format(128);
  uut.Unmount();

  pBuffer[0] = static_cast<uint8_t>(BlockTypes::sectionHead); // type
  pBuffer[1] = 'A'; // sectionNameHash
  pBuffer[2] = 16; // nBytes (LB)
  pBuffer[3] = 0; // nBytes (HB)
  pBuffer[4] = 0; // totalNbOfWrites LB
  pBuffer[5] = 0; // ...
  pBuffer[6] = 0; // ...
  pBuffer[7] = 0; // totalNbOfWrites
  pBuffer[8] = 2; // nextBlock LB
  pBuffer[9] = 0; // nextBlock HB
  pBuffer[10] = 1; // version LB
  pBuffer[11] = 0; // version HB
  pBuffer[12] = 'A';
  pBuffer[13] = 0;

  fakeStorage.Write(0, 14, pBuffer);
  UpdateCRC(0);

  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::BadSectionSystemInfoBlockError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_WrongType_sectionData)
{
  Format(128);
  uut.Unmount();

  pBuffer[0] = static_cast<uint8_t>(BlockTypes::sectionData); // type
  pBuffer[1] = 0; // sectionNameHash
  pBuffer[2] = 16; // nBytes (LB)
  pBuffer[3] = 0; // nBytes (HB)
  pBuffer[4] = 0; // totalNbOfWrites LB
  pBuffer[5] = 0; // ...
  pBuffer[6] = 0; // ...
  pBuffer[7] = 0; // totalNbOfWrites
  pBuffer[8] = 2; // nextBlock LB
  pBuffer[9] = 0; // nextBlock HB
  pBuffer[10] = 1; // seqNb LB
  pBuffer[11] = 0; // seqNb HB
  pBuffer[12] = 0x01;
  pBuffer[13] = 0x02;

  fakeStorage.Write(0, 14, pBuffer);
  UpdateCRC(0);

  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::BadSectionSystemInfoBlockError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_BadVersion)
{
  Format(128);
  uut.Unmount();

  fakeStorage.Read(offsetof(SectionSystemInfoBlock_t, sectionSystemVersion), 2, pBuffer);
  uint16_t version = pBuffer[0] | (static_cast<uint16_t>(pBuffer[1]) << 8U);
  version++;
  pBuffer[0] = version & 0xFF;
  pBuffer[1] = version >> 8U;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, sectionSystemVersion), 2, pBuffer);

  UpdateCRC(0);

  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::InvalidVersionError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_BadBlockSize)
{
  Format(128);
  uut.Unmount();

  pBuffer[0] = 16;
  pBuffer[1] = 0;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, blockSize), 2, pBuffer);
  UpdateCRC(0);
  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::InvalidHeaderError);

  pBuffer[0] = (blockSize + 1U) & 0xFFU;
  pBuffer[1] = (blockSize + 1U) >> 8U;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, blockSize), 2, pBuffer);
  UpdateCRC(0);
  ASSERT_THROW(uut.MountStep1(), std::invalid_argument);

  pBuffer[0] = (blockSize - 1U) & 0xFFU;
  pBuffer[1] = (blockSize - 1U) >> 8U;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, blockSize), 2, pBuffer);
  UpdateCRC(0);
  ASSERT_THROW(uut.MountStep1(), std::invalid_argument);

  pBuffer[0] = MinimumBlockSize;
  pBuffer[1] = 0;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, blockSize), 2, pBuffer);
  UpdateCRC(0);
  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::StorageSizeMismatchError);

  pBuffer[0] = (2 * MaximumBlockSize) & 0xFFU;
  pBuffer[1] = (2 * MaximumBlockSize) >> 8U;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, blockSize), 2, pBuffer);
  UpdateCRC(0);
  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep1_SSIB_BadNumberOfBlocks)
{
  Format(128);
  uut.Unmount();

  pBuffer[0] = ((storageSize / blockSize) + 1U) & 0xFFU;
  pBuffer[1] = ((storageSize / blockSize) + 1U) >> 8U;
  fakeStorage.Write(offsetof(SectionSystemInfoBlock_t, nBlocks), 2, pBuffer);
  UpdateCRC(0);
  ASSERT_THROW(uut.MountStep1(), EEPROMSectionSystem::StorageSizeMismatchError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Format_WrongState)
{
  // state is not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  Format(128);

  // state is mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_THROW(uut.Format(128), EEPROMSectionSystem::InsufficientStateError);

  uut.Unmount();

  // bring uut into state "ro_mount"
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());
  ASSERT_THROW(uut.Format(128), EEPROMSectionSystem::InsufficientStateError);

  // bring uut into state "defect"
  uut.MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  fakeStorage.Invalidate(blockSize, blockSize);
  RandomData data1(8,8);
  ASSERT_THROW(data1.Write("Section1", false, uut), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());
  ASSERT_THROW(uut.Format(128), EEPROMSectionSystem::InsufficientStateError);

  uut.Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_BlockSizeTooSmall)
{
  FakeEEPROM fakeStorage(1024, 0);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, 1024));

  ASSERT_THROW(spUUT->Format(MinimumBlockSize-1), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_MinimumBlockSize)
{
  FakeEEPROM fakeStorage(1024, 0);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, 1024));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize, 1));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize + 1, 1));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_BlockSizeTooLarge)
{
  size_t const size = 32*1024;
  FakeEEPROM fakeStorage(size, 0);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_THROW(spUUT->Format(MaximumBlockSize+1), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_MaximumBlockSize)
{
  size_t const size = 32*1024;
  FakeEEPROM fakeStorage(size, 0);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MaximumBlockSize, 1));
  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MaximumBlockSize-1, 1));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_BlockSizeLargerThanPageSize)
{
  size_t const size = 2*1024;
  FakeEEPROM fakeStorage(size, MinimumBlockSize);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_THROW(spUUT->Format(MinimumBlockSize+1), std::invalid_argument);

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize, 1));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_PageSizeNotDividedByBlockSize)
{
  size_t const size = 2*1024;
  FakeEEPROM fakeStorage(size, 2 * MinimumBlockSize);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize, 0));
  ASSERT_THROW(spUUT->Format(MinimumBlockSize+1), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_StorageHasNoPageSize)
{
  size_t const size = 2*1024;
  FakeEEPROM fakeStorage(size, 0);
  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize+1, 1));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_ResultingNbOfBlocksTooSmall)
{
  FakeEEPROM fakeStorage(3 * MinimumBlockSize, 0);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, 3 * MinimumBlockSize));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize, 1));
  ASSERT_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize + 1, 0), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_ResultingNbOfBlocksTooLarge)
{
  size_t const size = 2 * MaximumNbOfBlocks * MinimumBlockSize;
  FakeEEPROM fakeStorage(size, 0);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize * 2, 0));
  ASSERT_THROW(BasicTest_FormatWriteRead(spUUT.get(), MinimumBlockSize, 0), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, Format_TypicalEEPROM64kB)
{
  size_t const size = 64 * 1024;
  size_t const pageSize = 128;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  ASSERT_NO_THROW(BasicTest_FormatWriteRead(spUUT.get(), pageSize, 2));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithPageSize_SmallPage256B)
{
  size_t const size = 32 * 8U;
  size_t const pageSize = 32U;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithPageSize_SmallPage64kB)
{
  size_t const size = 64UL * 1024UL;
  size_t const pageSize = 32U;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithPageSize_SmallPageMaxNbOfBlocks)
{
  size_t const size = 32UL * 65535UL;
  size_t const pageSize = 32U;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithPageSize_LargePage64kB)
{
  size_t const size = 64UL * 1024UL;
  size_t const pageSize = 4096U;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithPageSize_LargePage1MB)
{
  size_t const size = 1024UL * 1024UL;
  size_t const pageSize = 4096U;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithPageSize_LargePage16MB)
{
  size_t const size = 16U * 1024UL * 1024UL;
  size_t const pageSize = 4096U;
  FakeEEPROM fakeStorage(size, pageSize);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithoutPageSize_SmallPage256B)
{
  size_t const size = 32 * 8U;
  size_t const pageSize = 32U;
  FakeEEPROM fakeStorage(size, 0U);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithoutPageSize_SmallPage64kB)
{
  size_t const size = 64UL * 1024UL;
  size_t const pageSize = 32U;
  FakeEEPROM fakeStorage(size, 0U);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithoutPageSize_SmallPageMaxNbOfBlocks)
{
  size_t const size = 32UL * 65535UL;
  size_t const pageSize = 32U;
  FakeEEPROM fakeStorage(size, 0U);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithoutPageSize_LargePage64kB)
{
  size_t const size = 64UL * 1024UL;
  size_t const pageSize = 4096U;
  FakeEEPROM fakeStorage(size, 0U);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithoutPageSize_LargePage1MB)
{
  size_t const size = 1024UL * 1024UL;
  size_t const pageSize = 4096U;
  FakeEEPROM fakeStorage(size, 0U);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_Tests, FormatUnmountMount_WithoutPageSize_LargePage16MB)
{
  size_t const size = 16U * 1024UL * 1024UL;
  size_t const pageSize = 4096U;
  FakeEEPROM fakeStorage(size, 0U);

  std::unique_ptr<EEPROMSectionSystem::EEPROMSectionSystem> spUUT(new EEPROMSectionSystem::EEPROMSectionSystem(fakeStorage, 0, size));

  spUUT->Format(pageSize);
  spUUT->Unmount();
  spUUT->MountStep1();
  spUUT->MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, spUUT->GetState());
  spUUT->Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, GetState)
{
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  Format(128);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  uut.Unmount();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_CircleOfFreeBlocks)
{
  Format(128);
  uut.Unmount();

  uint16_t const lastBlockIdx = ((storageSize / blockSize) - 1);

  // make nextBlock of last free block refer to first free block
  UpdateNextBlock(lastBlockIdx, 1);

  // mount
  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_NO_THROW(uut.MountStep2());

  // check that nextBlock of last free block has been fixed
  fakeStorage.Read(blockSize * lastBlockIdx + offsetof(CommonBlockHead_t, nextBlock), 2, pBuffer);
  ASSERT_EQ(NOBLOCK & 0xFFU, pBuffer[0]);
  ASSERT_EQ(NOBLOCK >> 8U, pBuffer[1]);

  // check that expected free storage is available
  ASSERT_EQ(((storageSize / blockSize) - 2U) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  // use it
  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_FreeBlocks1)
{
  // Scenario in storage:
  // One block + a few next blocks used to make up the initial list of free blocks,
  // then one block + a few next blocks added to head of initial list

  Format(128);
  uut.Unmount();

  uint16_t const nBlocks = storageSize / blockSize;
  ASSERT_TRUE(nBlocks > 10);

  for (uint16_t idx = 1; idx < 10; idx++)
  {
    if (idx != 9)
      UpdateNextBlock(idx, idx + 1);
    else
      UpdateNextBlock(idx, NOBLOCK);
  }
  for (uint16_t idx = 10; idx < nBlocks; idx++)
  {
    if (idx != nBlocks-1)
      UpdateNextBlock(idx, idx + 1);
    else
      UpdateNextBlock(idx, 1);
  }

  // mount it
  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_NO_THROW(uut.MountStep2());

  // check that expected free storage is available
  ASSERT_EQ(((storageSize / blockSize) - 2U) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  // use it
  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_FreeBlocks2)
{
  // Scenario in storage:
  // One block + a few next blocks used to make up the initial list of free blocks,
  // then one block added to head of initial list

  Format(128);
  uut.Unmount();

  uint16_t const nBlocks = storageSize / blockSize;
  ASSERT_TRUE(nBlocks > 3);

  for (uint16_t idx = 1; idx < nBlocks-1; idx++)
  {
    if (idx != nBlocks-2)
      UpdateNextBlock(idx, idx + 1);
    else
      UpdateNextBlock(idx, NOBLOCK);
  }
  UpdateNextBlock(nBlocks-1, 1);

  // mount it
  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_NO_THROW(uut.MountStep2());

  // check that expected free storage is available
  ASSERT_EQ(((storageSize / blockSize) - 2U) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  // use it
  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_FreeBlocks3)
{
  // Scenario in storage:
  // One block + a few next blocks used to make up the initial list of free blocks,
  // then one block + a few next blocks that are stand alone and marked as garbage

  Format(128);
  uut.Unmount();

  uint16_t const nBlocks = storageSize / blockSize;
  ASSERT_TRUE(nBlocks > 10);

  // blocks 5..7 shall be stand-alone free blocks
  for (uint16_t idx = 1; idx < nBlocks; idx++)
  {
    if ((idx >= 5) && (idx <= 7))
      continue;

    if (idx != 4)
      UpdateNextBlock(idx, idx + 1);
    else
      UpdateNextBlock(idx, 8);
  }

  // mount it
  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_NO_THROW(uut.MountStep2());

  // check that expected free storage is available
  ASSERT_EQ(((storageSize / blockSize) - 2U) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  // use it
  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_FreeBlocks4)
{
  // Scenario in storage:
  // One block + a few next blocks used to make up the initial list of free blocks,
  // then one block stand alone and marked ad garbage

  Format(128);
  uut.Unmount();

  uint16_t const nBlocks = storageSize / blockSize;
  ASSERT_TRUE(nBlocks > 10);

  // block 5 shall be stand-alone free block
  for (uint16_t idx = 1; idx < nBlocks; idx++)
  {
    if (idx == 5)
      continue;

    if (idx != 4)
      UpdateNextBlock(idx, idx + 1);
    else
      UpdateNextBlock(idx, 6);
  }

  // mount it
  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_NO_THROW(uut.MountStep2());

  // check that expected free storage is available
  ASSERT_EQ(((storageSize / blockSize) - 2U) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  // use it
  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_LastFreeBlockRefersToSection)
{
  // Scenario in storage:
  // Last free block refers to a section. Mount_CheckLastFreeBlock must fix the block.

  Format(128);
  uint16_t const nBlocks = storageSize / blockSize;
  RandomData dataBlock(8,8);
  dataBlock.Write("Section", false, uut);
  uut.Unmount();

  // ensure that the section's data is located in block 2
  fakeStorage.Read(blockSize * 2 + offsetof(CommonBlockHead_t, type), 1, pBuffer);
  ASSERT_EQ(static_cast<uint8_t>(BlockTypes::sectionData), pBuffer[0]);

  UpdateNextBlock(nBlocks - 1, 2);

  // mount
  ASSERT_NO_THROW(uut.MountStep1());
  ASSERT_NO_THROW(uut.MountStep2());

  dataBlock.Compare("Section", uut);

  // use it
  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_diffName_sameVersion)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // Version of the section heads is the same.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 1;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  ASSERT_THROW(uut.MountStep2(), BlockLinkageError);

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_diffName_2ndOlder)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an lower version. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  size_t const freeSpace = uut.GetFreeSpace();
  uut.Unmount();

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  data.Compare("Section1", uut);
  ASSERT_THROW(data.Compare("Section2", uut), gpcc::file_systems::NoSuchFileError);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_diffName_2ndNewer)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an higher version. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  size_t const freeSpace = uut.GetFreeSpace();
  uut.Unmount();

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 2;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_diffName_2ndOlder_WithWrapAround)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an lower version. There is a version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  size_t const freeSpace = uut.GetFreeSpace();
  uut.Unmount();

  // set version of first section head
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  data.Compare("Section1", uut);
  ASSERT_THROW(data.Compare("Section2", uut), gpcc::file_systems::NoSuchFileError);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_diffName_2ndNewer_WithWrapAround)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an higher version. There is a version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  size_t const freeSpace = uut.GetFreeSpace();
  uut.Unmount();

  // set version of first section head
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_sameName_sameVersion)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Name and version of the section heads are the same.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7] = '1';            // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 1;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  ASSERT_THROW(uut.MountStep2(), BlockLinkageError);

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_sameName_2ndOlder)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is lower. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  size_t const freeSpace = uut.GetFreeSpace();
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7] = '1';            // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  data1.Compare("Section1", uut);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_sameName_2ndNewer)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is higher. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  size_t const freeSpace = uut.GetFreeSpace();
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7] = '1';            // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 2;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  data2.Compare("Section1", uut);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_sameName_2ndOlder_WithWrapAround)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is lower. With version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  size_t const freeSpace = uut.GetFreeSpace();
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0x00;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0x00;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7] = '1';            // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  data1.Compare("Section1", uut);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MountStep2_SectionWith2Heads_sameName_2ndNewer_WithWrapAround)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is higher. With version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  size_t const freeSpace = uut.GetFreeSpace();
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7] = '1';            // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0x00; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0x00; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  uut.MountStep2();

  data2.Compare("Section1", uut);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_BadNames)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;

  ASSERT_THROW(spISW = uut.Create("", false), std::invalid_argument);
  ASSERT_THROW(spISW = uut.Create(" Sec1", false), std::invalid_argument);
  ASSERT_THROW(spISW = uut.Create("Sec2 ", false), std::invalid_argument);
  ASSERT_THROW(spISW = uut.Create(" Sec3 ", false), std::invalid_argument);
  ASSERT_THROW(spISW = uut.Create(" ", false), std::invalid_argument);

  ASSERT_NO_THROW(spISW = uut.Create("A", false));
  ASSERT_NO_THROW(spISW = uut.Create("A B", false));
  spISW.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_WrongState)
{
  Format(128);
  uut.Unmount();

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;

  // not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_THROW(spISW = uut.Create("Sec1", false), InsufficientStateError);

  // ro_mount
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());
  ASSERT_THROW(spISW = uut.Create("Sec1", false), InsufficientStateError);

  // mounted
  uut.MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  // defect
  fakeStorage.Invalidate(blockSize * 1U, blockSize);
  ASSERT_THROW(spISW = uut.Create("Sec1", false), InvalidHeaderError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  ASSERT_THROW(spISW = uut.Create("Sec1", false), InsufficientStateError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_SectionLocked)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW1;
  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW2;

  // locked by writer
  spISW1 = uut.Create("Sec1", false);
  ASSERT_THROW(spISW2 = uut.Create("Sec1", false), gpcc::file_systems::FileAlreadyAccessedError);
  spISW1->Close();

  // locked by reader
  auto spISR1 = uut.Open("Sec1");
  ASSERT_THROW(spISW2 = uut.Create("Sec1", false), gpcc::file_systems::FileAlreadyAccessedError);
  spISR1->Close();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_SectionAlreadyExistingAndNoOverwrite)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW1;
  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW2;

  spISW1 = uut.Create("Sec1", false);
  spISW1->Close();

  ASSERT_THROW(spISW2 = uut.Create("Sec1", false), gpcc::file_systems::FileAlreadyExistingError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_NoFreeBlocks)
{
  Format(128);

  size_t const n = uut.GetFreeSpace();
  ASSERT_TRUE(n > 8U);
  RandomData data(n - 8U, n - 8U);
  data.Write("Section1", false, uut);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW1;
  ASSERT_THROW(spISW1 = uut.Create("Section2", false), gpcc::file_systems::InsufficientSpaceError);

  data.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_OneFreeBlock)
{
  Format(128);

  size_t const bytesPerBlock = blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t));
  size_t const n = uut.GetFreeSpace();
  ASSERT_TRUE(n > 8U + bytesPerBlock);
  RandomData data(n - (8U + bytesPerBlock), n - (8U + bytesPerBlock));
  data.Write("Section1", false, uut);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW1;
  ASSERT_THROW(spISW1 = uut.Create("Section2", false), gpcc::file_systems::InsufficientSpaceError);

  data.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_TwoFreeBlocks)
{
  Format(128);

  size_t const bytesPerBlock = blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t));
  ASSERT_TRUE(bytesPerBlock > 8U);
  size_t const n = uut.GetFreeSpace();
  ASSERT_TRUE(n > 8U + 2U * bytesPerBlock);
  RandomData data(n - (8U + 2U * bytesPerBlock), n - (8U + 2U * bytesPerBlock));
  data.Write("Section1", false, uut);

  RandomData data2(bytesPerBlock - 8, bytesPerBlock - 8);
  data2.Write("Section2", false, uut);

  data.Compare("Section1", uut);
  data2.Compare("Section2", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_OverwriteExistingSection)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);
  *spISW << static_cast<uint8_t>(0xDE);
  *spISW << static_cast<uint8_t>(0xAD);
  *spISW << static_cast<uint8_t>(0xBE);
  *spISW << static_cast<uint8_t>(0xEF);
  spISW->Close();

  spISW = uut.Create("Section1", true);
  *spISW << static_cast<uint8_t>(0x12);
  *spISW << static_cast<uint8_t>(0x34);
  *spISW << static_cast<uint8_t>(0x56);
  *spISW << static_cast<uint8_t>(0x78);
  spISW->Close();

  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t data[4];
  *spISR >> data[0];
  *spISR >> data[1];
  *spISR >> data[2];
  *spISR >> data[3];

  ASSERT_EQ(data[0], 0x12U);
  ASSERT_EQ(data[1], 0x34U);
  ASSERT_EQ(data[2], 0x56U);
  ASSERT_EQ(data[3], 0x78U);

  spISR->Close();

  ASSERT_EQ(((storageSize / blockSize) - 4) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Create_VersionWrapAroundDuringOverwrite)
{
  // Scenario in EEPROM:
  // First Section: Head (1), Data (2)
  // 2nd Section: Head (3), Data (4)
  Format(128);

  // create first section
  fakeStorage.writeAccessCnt = 0;
  auto spISW = uut.Create("Section1", false);
  *spISW << static_cast<uint8_t>(0xDE);
  *spISW << static_cast<uint8_t>(0xAD);
  *spISW << static_cast<uint8_t>(0xBE);
  *spISW << static_cast<uint8_t>(0xEF);
  spISW->Close();
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // create second section, overwrites first section
  fakeStorage.writeAccessCnt = 0;
  spISW = uut.Create("Section1", true);
  *spISW << static_cast<uint8_t>(0x12);
  *spISW << static_cast<uint8_t>(0x34);
  *spISW << static_cast<uint8_t>(0x56);
  *spISW << static_cast<uint8_t>(0x78);
  spISW->Close();
  ASSERT_EQ(5U, fakeStorage.writeAccessCnt);

  spISW.reset();

  // check version of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);
  ASSERT_EQ(0x00, pBuffer[offsetof(SectionHeadBlock_t, version) + 0]);
  ASSERT_EQ(0x00, pBuffer[offsetof(SectionHeadBlock_t, version) + 1]);

  auto spISR = uut.Open("Section1");
  uint8_t data[4];
  *spISR >> data[0];
  *spISR >> data[1];
  *spISR >> data[2];
  *spISR >> data[3];

  ASSERT_EQ(data[0], 0x12U);
  ASSERT_EQ(data[1], 0x34U);
  ASSERT_EQ(data[2], 0x56U);
  ASSERT_EQ(data[3], 0x78U);

  spISR->Close();

  ASSERT_EQ(((storageSize / blockSize) - 4) * (blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t))), uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_RemainingCapacitySupported)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  ASSERT_FALSE(spISW->IsRemainingCapacitySupported());

  spISW->Close();
  spISW.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_RemainingCapacity)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  EXPECT_THROW((void)spISW->RemainingCapacity(), std::logic_error);

  spISW->Close();
  EXPECT_THROW((void)spISW->RemainingCapacity(), gpcc::Stream::ClosedError);

  spISW.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_DestroyWithoutClose)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);
  *spISW << static_cast<uint8_t>(0xDE);
  *spISW << static_cast<uint8_t>(0xAD);
  *spISW << static_cast<uint8_t>(0xBE);
  *spISW << static_cast<uint8_t>(0xEF);
  spISW.reset(); // note: no Close()

  auto spISR = uut.Open("Section1");
  uint8_t data[4];
  *spISR >> data[0];
  *spISR >> data[1];
  *spISR >> data[2];
  *spISR >> data[3];

  ASSERT_EQ(data[0], 0xDEU);
  ASSERT_EQ(data[1], 0xADU);
  ASSERT_EQ(data[2], 0xBEU);
  ASSERT_EQ(data[3], 0xEFU);

  spISR->Close();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteStrings)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);
  spISW->Write_string("Text");
  spISW->Write_string("");
  spISW->Write_line("Line");
  spISW->Write_line("");
  spISW->Write_uint8(0xFF);
  spISW->Close();
  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData[13];
  spISR->Read_uint8(readData, sizeof(readData));
  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();

  uint8_t expected[] = {'T', 'e', 'x', 't', 0x00,
                        0x00,
                        'L', 'i', 'n', 'e', '\n',
                        '\n',
                        0xFF};
  ASSERT_TRUE(memcmp(readData, expected, sizeof(expected)) == 0);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBitsOneByOne)
{
  Format(128);

  uint8_t const someBits[3] = { 0x24, 0xB6, 0xF2 };

  auto spISW = uut.Create("Section1", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bits(0x0E, 4U);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bits(someBits, 20);
  spISW->Close();
  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData[4];
  spISR->Read_uint8(readData, 4);
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(0xB9, readData[0]);
  ASSERT_EQ(0x90, readData[1]);
  ASSERT_EQ(0xD8, readData[2]);
  ASSERT_EQ(0x0A, readData[3]);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBits4Plus1Byte)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_uint8(0xAB);
  spISW->Close();
  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData[2];
  spISR->Read_uint8(readData, 2);
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(0x09, readData[0]);
  ASSERT_EQ(0xAB, readData[1]);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBits4Plus2Bytes)
{
  Format(128);

  uint8_t const someData[2] = { 0xAC, 0x6F };
  auto spISW = uut.Create("Section1", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_uint8(someData, 2);
  spISW->Close();
  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData[3];
  spISR->Read_uint8(readData, 3);
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(0x09, readData[0]);
  ASSERT_EQ(0xAC, readData[1]);
  ASSERT_EQ(0x6F, readData[2]);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBits4ThenClose)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Close();
  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData;
  *spISR >> readData;
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(0x09, readData);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBitsAllocationRequired)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  for (size_t i = 0; i < bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(0U, fakeStorage.readAccessCnt);

  spISW->Write_Bit(true);

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(2U, fakeStorage.readAccessCnt); // read-back of written block + read free block

  spISW->Close();
  spISW.reset();

  auto spISR = uut.Open("Section1");
  for (size_t i = 0; i < bytesPerBlock; i++)
  {
    uint8_t readData;
    *spISR >> readData;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), readData);
  }

  bool bit = spISR->Read_bit();
  ASSERT_TRUE(bit);
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBitsNextWriteWouldTriggerAllocation)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  for (size_t i = 0; i < bytesPerBlock-1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(false);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(0U, fakeStorage.readAccessCnt);

  spISW->Close();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(2U, fakeStorage.readAccessCnt);

  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData;
  for (size_t i = 0; i < bytesPerBlock-1U; i++)
  {
    *spISR >> readData;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), readData);
  }

  *spISR >> readData;
  ASSERT_EQ(0x2DU, readData);

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_Write2x6StoreAndAllocAfterWriting)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  for (size_t i = 0; i < bytesPerBlock-1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  spISW->Write_Bits(0x3B, 6);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(0U, fakeStorage.readAccessCnt);

  spISW->Write_Bits(0x26, 6);

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(2U, fakeStorage.readAccessCnt);
  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  spISW->Close();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(2U, fakeStorage.readAccessCnt);

  spISW.reset();

  auto spISR = uut.Open("Section1");
  uint8_t readData;
  for (size_t i = 0; i < bytesPerBlock-1U; i++)
  {
    *spISR >> readData;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), readData);
  }

  *spISR >> readData;
  ASSERT_EQ(0xBBU, readData);
  *spISR >> readData;
  ASSERT_EQ(0x09U, readData);

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteMultipleBytesAllocRequired)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  RandomData data(bytesPerBlock + 1U, bytesPerBlock + 1U);
  spISW->Write_uint8(data.GetData(), bytesPerBlock + 1U);

  ASSERT_EQ(1U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(2U, fakeStorage.readAccessCnt);
  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  spISW->Close();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(2U, fakeStorage.readAccessCnt);

  spISW.reset();

  auto spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock+1U; i++)
  {
    uint8_t readData;
    *spISR >> readData;
    ASSERT_EQ(data.GetData()[i], readData);
  }

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteMultipleBytesFullErrorOnAllocation)
{
  Format(128);

  RandomData fillUp(uut.GetFreeSpace() - 2U * bytesPerBlock - 8U, uut.GetFreeSpace() - 2U * bytesPerBlock - 8U);
  fillUp.Write("FillUp", false, uut);

  auto spISW = uut.Create("Section1", false);

  RandomData data(bytesPerBlock + 1U, bytesPerBlock + 1U);
  EXPECT_THROW(spISW->Write_uint8(data.GetData(), bytesPerBlock + 1U), gpcc::Stream::FullError);

  ASSERT_EQ(gpcc::Stream::IStreamWriter::States::error, spISW->GetState());

  spISW->Close();
  spISW.reset();

  ASSERT_EQ(bytesPerBlock, uut.GetFreeSpace());

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_WriteBitsFullErrorOnAllocation)
{
  Format(128);

  RandomData fillUp(uut.GetFreeSpace() - 2U * bytesPerBlock - 8U, uut.GetFreeSpace() - 2U * bytesPerBlock - 8U);
  fillUp.Write("FillUp", false, uut);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  RandomData data(bytesPerBlock, bytesPerBlock);
  spISW->Write_uint8(data.GetData(), bytesPerBlock);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(0U, fakeStorage.readAccessCnt);

  ASSERT_THROW(spISW->Write_Bits(0x1B, 6), gpcc::Stream::FullError);

  spISW->Close();
  spISW.reset();

  ASSERT_EQ(bytesPerBlock, uut.GetFreeSpace());

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_Write2x6StoreAndAllocAfterWritingWithFullError)
{
  Format(128);

  RandomData fillUp(uut.GetFreeSpace() - 2U * bytesPerBlock - 8U, uut.GetFreeSpace() - 2U * bytesPerBlock - 8U);
  fillUp.Write("FillUp", false, uut);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  spISW->Write_Bits(0x3B, 6);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);
  ASSERT_EQ(0U, fakeStorage.readAccessCnt);

  ASSERT_THROW(spISW->Write_Bits(0x26, 6), gpcc::Stream::FullError);

  spISW->Close();
  spISW.reset();

  ASSERT_EQ(bytesPerBlock, uut.GetFreeSpace());

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_ProperCleanupUponCloseInErrorState)
{
  Format(128);

  RandomData fillUp(uut.GetFreeSpace() - 2U * bytesPerBlock - 8U, uut.GetFreeSpace() - 2U * bytesPerBlock - 8U);
  fillUp.Write("FillUp", false, uut);

  RandomData dataSec1(bytesPerBlock - 8U + 1U, bytesPerBlock - 8U + 1U);
  ASSERT_THROW(dataSec1.Write("Section1", false, uut), gpcc::Stream::FullError);

  // check free space
  ASSERT_EQ(bytesPerBlock, uut.GetFreeSpace());

  // check section fillUp
  fillUp.Compare("FillUp", uut);

  // check: Section1 must not be existing
  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  // unmount/remount. Check: There must be no write access to storage
  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;
  uut.Unmount();
  uut.MountStep1();
  uut.MountStep2();
  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  // check free space
  ASSERT_EQ(bytesPerBlock, uut.GetFreeSpace());

  // check section fillUp
  fillUp.Compare("FillUp", uut);

  // check: Section1 must not be existing
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_StorageErrorUponWrite)
{
  Format(128);

  RandomData fillUp(512, 512);
  fillUp.Write("FillUp", false, uut);

  RandomData data(3U * bytesPerBlock, 3U * bytesPerBlock);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAndCheckAccessTillFailure = 2;

  ASSERT_THROW( for (size_t i = 0; i < 3U * bytesPerBlock; i++) { *spISW << static_cast<uint8_t>(i & 0xFFU); }, gpcc::Stream::IOError);

  ASSERT_EQ(gpcc::Stream::IStreamWriter::States::error, spISW->GetState());

  EXPECT_ANY_THROW(spISW->Close());
  spISW.reset();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  uut.MountStep1();

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  uut.MountStep2();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);

  fillUp.Compare("FillUp", uut);
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_StorageErrorUponClose1)
{
  Format(128);

  RandomData fillUp(512, 512);
  fillUp.Write("FillUp", false, uut);

  RandomData data(3U * bytesPerBlock, 3U * bytesPerBlock);

  auto spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < 3U * bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  fakeStorage.writeAndCheckAccessTillFailure = 1;
  EXPECT_THROW(spISW->Close(), gpcc::Stream::IOError);
  spISW.reset();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  uut.MountStep1();

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  uut.MountStep2();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);

  fillUp.Compare("FillUp", uut);
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_StorageErrorUponClose2)
{
  Format(128);

  RandomData fillUp(512, 512);
  fillUp.Write("FillUp", false, uut);

  RandomData data(3U * bytesPerBlock, 3U * bytesPerBlock);

  auto spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < 3U * bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  fakeStorage.writeAndCheckAccessTillFailure = 2;
  EXPECT_THROW(spISW->Close(), gpcc::Stream::IOError);
  spISW.reset();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  uut.MountStep1();

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  uut.MountStep2();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);

  fillUp.Compare("FillUp", uut);
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_StorageThrowsUponWrite)
{
  Format(128);

  RandomData fillUp(512, 512);
  fillUp.Write("FillUp", false, uut);

  RandomData data(3U * bytesPerBlock, 3U * bytesPerBlock);

  auto spISW = uut.Create("Section1", false);

  fakeStorage.writeAccessesTillThrow = 2;

  ASSERT_THROW( for (size_t i = 0; i < 3U * bytesPerBlock; i++) { *spISW << static_cast<uint8_t>(i & 0xFFU); }, gpcc::Stream::IOError);

  ASSERT_EQ(gpcc::Stream::IStreamWriter::States::error, spISW->GetState());

  EXPECT_ANY_THROW(spISW->Close());
  spISW.reset();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  uut.MountStep1();

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  uut.MountStep2();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);

  fillUp.Compare("FillUp", uut);
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_StorageThrowsUponClose1)
{
  Format(128);

  RandomData fillUp(512, 512);
  fillUp.Write("FillUp", false, uut);

  RandomData data(3U * bytesPerBlock, 3U * bytesPerBlock);

  auto spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < 3U * bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  fakeStorage.writeAccessesTillThrow = 1;
  EXPECT_THROW(spISW->Close(), gpcc::Stream::IOError);
  spISW.reset();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  uut.MountStep1();

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  uut.MountStep2();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);

  fillUp.Compare("FillUp", uut);
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_StorageThrowsUponClose2)
{
  Format(128);

  RandomData fillUp(512, 512);
  fillUp.Write("FillUp", false, uut);

  RandomData data(3U * bytesPerBlock, 3U * bytesPerBlock);

  auto spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < 3U * bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);

  fakeStorage.writeAccessesTillThrow = 2;
  EXPECT_THROW(spISW->Close(), gpcc::Stream::IOError);
  spISW.reset();

  ASSERT_EQ(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();

  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt = 0;

  uut.MountStep1();

  fillUp.Compare("FillUp", uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  uut.MountStep2();

  ASSERT_NE(0U, fakeStorage.writeAccessCnt);

  fillUp.Compare("FillUp", uut);
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_PowerFailUponWriteOrClose)
{
  Format(128);

  fakeStorage.SetEnableUndo(true);
  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt  = 0;

  RandomData data(2U * bytesPerBlock, 2U * bytesPerBlock);
  data.Write("Section1", false, uut);

  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);

  FakeEEPROM copyOfStorage(fakeStorage);

  uut.Unmount();

  for (uint_fast8_t i = 1U; i < 4U; i++)
  {
    fakeStorage = copyOfStorage;
    fakeStorage.Undo(i);

    uut.MountStep1();

    std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
    ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

    uut.MountStep2();

    ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

    BasicTest_WriteRead(&uut, blockSize, 1);

    uut.Unmount();
  }
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionWriter_GetNbOfCachedBits)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  ASSERT_EQ(gpcc::Stream::IStreamWriter::States::open, spISW->GetState());

  spISW->Write_uint8(0xAB);
  EXPECT_EQ(0U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(1U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(2U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(3U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(4U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(5U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(6U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(7U, spISW->GetNbOfCachedBits());
  spISW->Write_Bit(true);
  EXPECT_EQ(0U, spISW->GetNbOfCachedBits());

  spISW->Close();

  ASSERT_EQ(gpcc::Stream::IStreamWriter::States::closed, spISW->GetState());
  EXPECT_THROW(spISW->GetNbOfCachedBits(), gpcc::Stream::ClosedError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_BadNames)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  ASSERT_THROW(spISR = uut.Open(""), std::invalid_argument);
  ASSERT_THROW(spISR = uut.Open(" Sec1"), std::invalid_argument);
  ASSERT_THROW(spISR = uut.Open("Sec2 "), std::invalid_argument);
  ASSERT_THROW(spISR = uut.Open(" Sec3 "), std::invalid_argument);
  ASSERT_THROW(spISR = uut.Open(" "), std::invalid_argument);

  ASSERT_NO_THROW(spISW = uut.Create("A", false));
  ASSERT_NO_THROW(spISW = uut.Create("A B", false));
  spISW.reset();

  ASSERT_NO_THROW(spISR = uut.Open("A"));
  ASSERT_NO_THROW(spISR = uut.Open("A B"));
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionNotExisting)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Sec1"), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_WrongState)
{
  Format(128);
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  uut.Unmount();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;

  // state is not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  ASSERT_THROW(spISR = uut.Open("Section1"), InsufficientStateError);

  // bring uut into state "ro_mount"
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());

  ASSERT_NO_THROW(spISR = uut.Open("Section1"));
  spISR.reset();

  // bring uut into state "mounted"
  uut.MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  ASSERT_NO_THROW(spISR = uut.Open("Section1"));
  spISR.reset();

  // bring uut into state "defect"
  fakeStorage.Invalidate(blockSize * 3U, blockSize);
  ASSERT_THROW(data.Write("Section2", false, uut), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  ASSERT_THROW(spISR = uut.Open("Section1"), InsufficientStateError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionLockedByReader)
{
  Format(128);

  RandomData data(8,8);
  data.Write("Section1", false, uut);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR1;
  std::unique_ptr<gpcc::Stream::IStreamReader> spISR2;
  ASSERT_NO_THROW(spISR1 = uut.Open("Section1"));
  ASSERT_NO_THROW(spISR2 = uut.Open("Section1"));
  spISR1.reset();
  spISR2.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionLockedByWriter)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::FileAlreadyAccessedError);

  spISW.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_EmptySection)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_DestroyReaderWithoutClose)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint32(0x12345678U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");
  spISR.reset();

  spISR = uut.Open("Section1");
  uint8_t data[4];
  spISR->Read_uint8(data, 4);
  ASSERT_EQ(0x78U, data[0]);
  ASSERT_EQ(0x56U, data[1]);
  ASSERT_EQ(0x34U, data[2]);
  ASSERT_EQ(0x12U, data[3]);
  spISR.reset();

  ASSERT_NO_THROW(uut.Delete("Section1"));
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_diffName_sameVersion)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // Version of the section heads is the same.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 1;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), BlockLinkageError);

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_diffName_2ndOlder)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an lower version. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  data.Compare("Section1", uut);
  ASSERT_THROW(data.Compare("Section2", uut), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_diffName_2ndNewer)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an higher version. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 2;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_diffName_2ndOlder_WithWrapAround)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an lower version. With version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  data.Compare("Section1", uut);
  ASSERT_THROW(data.Compare("Section2", uut), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_diffName_2ndNewer_WithWrapAround)
{
  // Scenario in storage:
  // One section only: Head on block 1, data on block 2.
  // Block 3 contains another section head referencing to block 2, but with different name.
  // The second section head has an higher version. With version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data(8,8);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // create 2nd section head, uses head of 1st section as template
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '2';           // Section1 -> Section2
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]++;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0x00; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0x00; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_sameName_sameVersion)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Name and version of the section heads are the same.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '1';           // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 1;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), BlockLinkageError);

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_sameName_2ndOlder)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is lower. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '1';           // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  data1.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_sameName_2ndNewer)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is higher. No version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '1';           // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 2;   // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0;   // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  data2.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_sameName_2ndOlder_WithWrapAround)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is lower. With version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0x00;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0x00;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '1';           // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  data1.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Open_SectionWith2Heads_sameName_2ndNewer_WithWrapAround)
{
  // Scenario in storage:
  // Two sections: First: Head on block 1, data on block 2.
  //               Second: Head on block 3, data on block 4.
  // Names are the same, version of the second section is higher. With version wrap-around.

  Format(128);
  fakeStorage.writeAccessCnt = 0;
  RandomData data1(8,8);
  data1.Write("Section1", false, uut);
  RandomData data2(8,8);
  data2.Write("Section2", false, uut);
  ASSERT_EQ(4U, fakeStorage.writeAccessCnt);
  uut.Unmount();

  // set version of first section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  // update head of 2nd section
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);

  pBuffer[sizeof(SectionHeadBlock_t) + 7U] = '1';           // Section2 -> Section1
  pBuffer[offsetof(CommonBlockHead_t, sectionNameHash)]--;  // Update name hash

  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0x00; // set version LB
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0x00; // set version HB

  fakeStorage.Write(3U * blockSize, blockSize, pBuffer);
  UpdateCRC(3);

  // test
  uut.MountStep1();
  data2.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_RemainingBytes)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_FALSE(spISR->IsRemainingBytesSupported());

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), std::logic_error);

  spISR->Skip(2U * 8U);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), std::logic_error);

  ASSERT_THROW(spISR->Skip(1U), gpcc::Stream::EmptyError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), gpcc::Stream::ErrorStateError);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());
  EXPECT_THROW((void)spISR->RemainingBytes(), gpcc::Stream::ClosedError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_EmptySection)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  uint8_t data;
  ASSERT_THROW(*spISR >> data, gpcc::Stream::EmptyError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadAndSectionBecomesEmpty)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint8(55);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());
  uint8_t data;
  *spISR >> data;
  ASSERT_EQ(55, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadStrings)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  *spISW << std::string("Str1");
  *spISW << std::string("Str2");
  spISW->Write_line("Str3");
  spISW->Write_char("Str4\nStr5", 10);
  spISW->Write_char("Str6\nStr7\n", 10);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  *spISR >> s;
  ASSERT_TRUE(s == "Str2");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Str3");
  *spISR >> s;
  ASSERT_TRUE(s == "Str4\nStr5");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Str6");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Str7");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_NullTermIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << std::string("Str1");
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_NullTermIsLastByteInBlockAndNoNextBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << std::string("Str1");
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_StringSpansOverStorageBlockBoundary)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << std::string("Str1");
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_StringSpansOverStorageBlockBoundaryPlusData)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << std::string("Str1");
  *spISW << static_cast<uint8_t>(0x55U);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_StringSpansOverStorageBlockBoundaryNullTermIsFirstByteInNextBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << std::string("Str1");
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_StringSpansOverStorageBlockBoundaryNullTermIsFirstByteInNextBlockPlusData)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << std::string("Str1");
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55U);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  *spISR >> s;
  ASSERT_TRUE(s == "Str1");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_NoNullTerminator)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_char("Str1", 4);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  std::string s;
  ASSERT_THROW(*spISR >> s, gpcc::Stream::EmptyError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadString_NoNullTerminatorAtEndOfBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1", 4);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  ASSERT_THROW(*spISR >> s, gpcc::Stream::EmptyError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_DifferentLineEndings)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_string("Line1\nLine2\rLine3\r\nLine4");
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "Line1");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Line2");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Line3");
  s = spISR->Read_line();
  ASSERT_TRUE(s == "Line4");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_NUL_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_uint8(0x00);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_LF_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\n');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CR_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CRLF_TermCharsAreInBothBlocks)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW->Write_char('\n');
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CRLF_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_NUL_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_uint8(0x00);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_LF_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\n');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CR_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CRLF_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_NUL_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_uint8(0x00);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_LF_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\n');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CR_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_EmptyStr_CRLF_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char('\r');
  spISW->Write_char('\n');
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 2U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_NUL_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_string("ABC");
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_LF_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\n", 4U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CR_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r", 4U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CRLF_TermCharsAreInBothBlocks)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r\n", 5U);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CRLF_TermCharIsLastByteInBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r\n", 5U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55);
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_NUL_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_string("ABC");
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_LF_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\n", 4U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CR_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r", 4U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CRLF_EOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 6U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r\n", 5U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 6U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_NUL_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_string("ABC");
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_LF_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\n", 4U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CR_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r", 4U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_CRLF_EOFandBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("ABC\r\n", 5U);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 5U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "ABC");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_NoTermCharAtEOF)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_char("Str1", 4);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  std::string s;
  s = spISR->Read_line();
  EXPECT_TRUE(s == "Str1");
  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_NoTermCharAtEOF_AtBlockEnd)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1", 4);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s;
  s = spISR->Read_line();
  EXPECT_TRUE(s == "Str1");
  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_StringSpansOverStorageBlockBoundary)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1\n", 5);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "Str1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_StringSpansOverStorageBlockBoundary_ErrDuringReadNextBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1\n", 5);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  fakeStorage.readAccessesTillThrow = 1U;

  std::string s;
  EXPECT_THROW(s = spISR->Read_line(), std::runtime_error);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_StringSpansOverStorageBlockBoundaryPlusData)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1\n", 5);
  *spISW << static_cast<uint8_t>(0x55U);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 3U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "Str1");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_StringSpansOverStorageBlockBoundary_LFisFirstByteInNextBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1\n", 5);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    uint8_t data;
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "Str1");
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadLine_StringSpansOverStorageBlockBoundary_LFisFirstByteInNextBlockPlusData)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW->Write_char("Str1", 4);
  EXPECT_EQ(0U, fakeStorage.writeAccessCnt);
  spISW->Write_char('\n');
  EXPECT_EQ(1U, fakeStorage.writeAccessCnt);
  *spISW << static_cast<uint8_t>(0x55U);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 4U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  std::string s = spISR->Read_line();
  ASSERT_TRUE(s == "Str1");
  *spISR >> data;
  ASSERT_EQ(0x55U, data);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadByte_LastByteOfLastBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << static_cast<uint8_t>(0xEEU);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;

  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> data;
  ASSERT_EQ(0xEEU, data);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadByte_ErrorOnlyFewBitsLeft)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  *spISW << static_cast<uint8_t>(0xEEU);
  spISW.reset();

  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;

  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  spISR->Read_bits(&data, 4);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  ASSERT_THROW(*spISR >> data, gpcc::Stream::EmptyError);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadChunkOfBytes_LastBytesOfLastBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < 2U * bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  std::vector<uint8_t> data;
  data.resize(2U * bytesPerBlock);

  spISR->Read_uint8(data.data(), 2U * bytesPerBlock);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  for (size_t i = 0; i < 2U * bytesPerBlock; i++)
  {
    ASSERT_EQ(i & 0xFFU, data[i]);
  }

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadChunkOfBytes_ErrReadBeyondEndOfSection)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < 2U * bytesPerBlock; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  std::vector<uint8_t> data;
  data.resize((2U * bytesPerBlock) + 2U);

  ASSERT_THROW(spISR->Read_uint8(data.data(), (2U * bytesPerBlock) + 2U), gpcc::Stream::EmptyError);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadBits_MoreThanLeft_ButOneMoreByteAvailable)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x34U);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;

  spISR->Read_bits(&data, 4);
  ASSERT_EQ(0x02U, data);

  spISR->Read_bits(&data, 8);
  ASSERT_EQ(0x41U, data);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadBits_MoreThanLeft)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint8(0x12U);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;

  spISR->Read_bits(&data, 4);
  ASSERT_EQ(0x02U, data);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  ASSERT_THROW(spISR->Read_bits(&data, 8), gpcc::Stream::EmptyError);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  spISR.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadBits_LastBitsInSection)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x34U);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;

  spISR->Read_bits(&data, 4);
  ASSERT_EQ(0x02U, data);

  spISR->Read_bits(&data, 8);
  ASSERT_EQ(0x41U, data);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  bool bit;

  *spISR >> bit;
  ASSERT_TRUE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_TRUE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ReadBits_OneByOne)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint8(0x12U);
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  bool bit;

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_TRUE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_TRUE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  *spISR >> bit;
  ASSERT_FALSE(bit);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, ReadWriteBits_ProperInsertionOfGaps)
{
  Format(128);

  uint8_t data = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);

  spISW->Write_uint8(0x12U);

  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(false);

  spISW->Write_uint8(0xDEU);
  spISW->Write_uint8(0xADU);

  spISW->Write_Bit(false);
  spISW->Write_Bit(false);
  spISW->Write_Bit(true);
  spISW->Write_Bit(true);

  spISW->Write_uint8(&data, 0);

  spISW->Write_Bit(false);
  spISW->Write_Bit(true);

  spISW->Write_uint8(0xBEU);
  spISW->Write_uint8(0xEFU);

  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  bool bit;
  *spISR >> data;
  ASSERT_EQ(0x12U, data);

  *spISR >> bit;
  ASSERT_TRUE(bit);
  *spISR >> bit;
  ASSERT_FALSE(bit);
  *spISR >> bit;
  ASSERT_TRUE(bit);
  *spISR >> bit;
  ASSERT_FALSE(bit);

  *spISR >> data;
  ASSERT_EQ(0xDEU, data);
  *spISR >> data;
  ASSERT_EQ(0xADU, data);

  *spISR >> bit;
  ASSERT_FALSE(bit);
  *spISR >> bit;
  ASSERT_FALSE(bit);

  // Note that the read(0) and write(0) are placed at different positions within the read/written bits.
  spISR->Read_uint8(&data, 0);

  *spISR >> bit;
  ASSERT_TRUE(bit);
  *spISR >> bit;
  ASSERT_TRUE(bit);
  *spISR >> bit;
  ASSERT_FALSE(bit);
  *spISR >> bit;
  ASSERT_TRUE(bit);

  *spISR >> data;
  ASSERT_EQ(0xBEU, data);
  *spISR >> data;
  ASSERT_EQ(0xEFU, data);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_CRCErrorOnFirstDataBlock)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  *spISW << std::string("Test");
  spISW.reset();

  // invalid checksum of data block
  InvalidateCRC(2);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), DataIntegrityError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_CRCErrorOnSecondDataBlock)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock + 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  // invalid checksum of 2nd data block
  InvalidateCRC(3);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  ASSERT_THROW(*spISR >> data, gpcc::Stream::IOError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ThrowOnFirstDataBlock1)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  *spISW << std::string("Test");
  spISW.reset();

  fakeStorage.readAccessesTillThrow = 4; // Hash + Head(2) + Data(1 of 2)
  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), std::exception);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ThrowOnFirstDataBlock2)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  *spISW << std::string("Test");
  spISW.reset();

  fakeStorage.readAccessesTillThrow = 5; // Hash + Head(2) + Data(2 of 2)
  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), std::exception);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ThrowOnSecondDataBlock1)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock + 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  fakeStorage.readAccessesTillThrow = 1;
  ASSERT_THROW(*spISR >> data, gpcc::Stream::IOError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_ThrowOnSecondDataBlock2)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  for (size_t i = 0; i < bytesPerBlock + 5U; i++)
    *spISW << static_cast<uint8_t>(i & 0xFFU);
  spISW.reset();

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  uint8_t data;
  for (size_t i = 0; i < bytesPerBlock - 1U; i++)
  {
    *spISR >> data;
    ASSERT_EQ(static_cast<uint8_t>(i & 0xFFU), data);
  }

  fakeStorage.readAccessesTillThrow = 2;
  ASSERT_THROW(*spISR >> data, gpcc::Stream::IOError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());
  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_EnsureAllDataConsumed_OK_1)
{
  using namespace gpcc::Stream;

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Section1", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Write_uint8(0x13U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Section1");

  // (3 bytes left) -------------------------------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_uint16(); // (1 byte left) -------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (7 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (6 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (5 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (4 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (3 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (2 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (1 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (0 bit left) -----------------------------------------------------------------------
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::empty);

  spISR->Close();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_EnsureAllDataConsumed_OK_2)
{
  using namespace gpcc::Stream;

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // (2 bytes left) -------------------------------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  for (uint_fast8_t i = 0; i < 8; i++)
  {
    (void)spISR->Read_bit();
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
    EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), RemainingBitsError);
    EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven));
    EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

    EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);
  }

  (void)spISR->Read_bit(); // (7 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven));
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (6 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (5 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (4 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (3 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (2 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  (void)spISR->Read_bit(); // (1 bit left) -----------------------------------------------------------------------
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::open);

  (void)spISR->Read_bit(); // (0 bit left) -----------------------------------------------------------------------
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), RemainingBitsError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess));
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), RemainingBitsError);
  EXPECT_NO_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any));

  EXPECT_EQ(spISR->GetState(), IStreamReader::States::empty);

  spISR->Close();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_EnsureAllDataConsumed_ErrorState)
{
  using namespace gpcc::Stream;

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // create error condition
  ASSERT_THROW((void)spISR->Read_uint32(), EmptyError);

  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), ErrorStateError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any), ErrorStateError);

  spISR->Close();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, SectionReader_EnsureAllDataConsumed_ClosedState)
{
  using namespace gpcc::Stream;

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // create pre-condition
  spISR->Close();

  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::zero), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::one), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::two), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::three), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::four), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::five), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::six), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::seven), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::sevenOrLess), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::moreThanSeven), ClosedError);
  EXPECT_THROW(spISR->EnsureAllDataConsumed(IStreamReader::RemainingNbOfBits::any), ClosedError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, AlignToByteBoundary_OK)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  spISW->Write_Bit(true);
  spISW->Write_Bit(false);
  EXPECT_EQ(6U, spISW->AlignToByteBoundary(false));

  spISW->FillBits(12, false);
  EXPECT_EQ(4U, spISW->AlignToByteBoundary(true));

  spISW->Write_uint8(0xDEU);
  EXPECT_EQ(0U, spISW->AlignToByteBoundary(false));

  spISW->Close();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  EXPECT_EQ(0x01U, spISR->Read_uint8());
  EXPECT_EQ(0x00U, spISR->Read_uint8());
  EXPECT_EQ(0xF0U, spISR->Read_uint8());
  EXPECT_EQ(0xDEU, spISR->Read_uint8());

  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, AlignToByteBoundary_StateClosed)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  spISW->Write_uint32(0xDEADBEEFUL);

  spISW->Close();

  EXPECT_THROW((void)spISW->AlignToByteBoundary(false), gpcc::Stream::ClosedError);

  EXPECT_EQ(gpcc::Stream::IStreamWriter::States::closed, spISW->GetState());

  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  EXPECT_EQ(0xDEADBEEFUL, spISR->Read_uint32());

  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, FillBitsAndBytes_OK)
{
  using namespace gpcc::Stream;

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  spISW->FillBits(1, true);
  spISW->FillBits(1, false);
  spISW->Write_Bits(static_cast<uint8_t>(0x0FU), 4);
  spISW->FillBytes(1, 0xFFU);
  spISW->FillBytes(2, 0x55U);
  spISW->FillBits(16, false);

  spISW->FillBits(0, false);
  spISW->FillBytes(0, 0);

  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  EXPECT_EQ(0x3DU, spISR->Read_uint8());
  EXPECT_EQ(0xFFU, spISR->Read_uint8());
  EXPECT_EQ(0x55U, spISR->Read_uint8());
  EXPECT_EQ(0x55U, spISR->Read_uint8());
  EXPECT_EQ(0x00U, spISR->Read_uint8());
  EXPECT_EQ(0x00U, spISR->Read_uint8());

  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, FillBitsAndBytes_StateClosed)
{
  using namespace gpcc::Stream;

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);

  spISW->Write_uint32(0xDEADBEEFUL);

  spISW->Close();

  EXPECT_THROW(spISW->FillBits(1, true), gpcc::Stream::ClosedError);
  EXPECT_THROW(spISW->FillBytes(1, 0x55U), gpcc::Stream::ClosedError);

  EXPECT_EQ(gpcc::Stream::IStreamWriter::States::closed, spISW->GetState());

  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  EXPECT_EQ(0xDEADBEEFUL, spISR->Read_uint32());

  EXPECT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_ZeroBits)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x57U);
  spISW->Write_uint8(0xE9U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  uint8_t u8;

  spISR->Skip(0U);

  u8 = spISR->Read_uint8();
  ASSERT_EQ(0x57U, u8);

  spISR->Skip(0U);

  u8 = spISR->Read_uint8();
  ASSERT_EQ(0xE9U, u8);

  spISR->Skip(0U);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsLeft_SkipSomeBits)
{
  // There are 4 bits left that have not been read yet. We skip 3 of them.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(3U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bits(1U), 0x01U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsAndOneByteLeft_SkipAllBits)
{
  // There are 4 bits + 1 Byte left that have not been read yet. We skip 4 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(4U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0xDBU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsLeft_SkipAll)
{
  // There are 4 bits left that have not been read yet. We skip them all.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(4U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsLeft_SkipAllPlusOne)
{
  // There are 4 bits left that have not been read yet. We skip them all + 1.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(5U), gpcc::Stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndOneByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 12 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(12U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndTwoByte)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+8=20 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(20U), gpcc::Stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsAndOneByteLeft_SkipAllBitsAndOneByteAndOneBit)
{
  // There are 4 bits + 1 byte left that have not been read yet. We skip 4+8+1 = 13 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(13U), gpcc::Stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsAndTwoByteLeft_SkipAllBitsAndOneByte)
{
  // There are 4 bits + 2 byte left that have not been read yet. We skip 4+8=12 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Write_uint8(0x36U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(12U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x36U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_BitsAndTwoByteLeft_SkipAllBitsAndOneByteAndOneBit)
{
  // There are 4 bits + 2 byte left that have not been read yet. We skip 4+8+1=13 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Write_uint8(0x36U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(4U), 0x0AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(13U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bits(7U), 0x1BU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_OneByteLeft_Skip8Bits)
{
  // There is 1 byte left that has not been read yet. We skip 8 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0xDBU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(8U), 0x8AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(8U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_OneByteLeft_Skip7Bits)
{
  // There is 1 byte left that has not been read yet. We skip 7 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(8U), 0x8AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  spISR->Skip(7U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bit(), 0x01U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_OneByteLeft_Skip9Bits)
{
  // There is 1 byte left that has not been read yet. We skip 9 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_bits(8U), 0x8AU);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  // - precondition established -

  ASSERT_THROW(spISR->Skip(9U), gpcc::Stream::EmptyError);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::error);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_TwoByteLeft_Skip8Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 8 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // - precondition established -

  spISR->Skip(8U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint8(), 0x80U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_TwoByteLeft_Skip16Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 16 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // - precondition established -

  spISR->Skip(16U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_TwoByteLeft_Skip9Bits)
{
  // There are 2 bytes left that have not been read yet. We skip 9 bits.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0x8AU);
  spISW->Write_uint8(0x80U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // - precondition established -

  spISR->Skip(9U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_bits(7U), 0x40U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_LargerThanBlockSize)
{
  // This skips a number of bytes larger than the block size of the underlying storage

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  for (uint_fast16_t i = 0; i < 256U; i++)
    spISW->Write_uint16(i);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  // - precondition established -

  spISR->Skip(255UL * 2U * 8U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint16(), 255U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_LastBitInLastByteOfBlock_MoreBlocks)
{
  // This skips the first bit of the last byte in a block. There are more blocks.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  for (uint_fast16_t i = 0; i < bytesPerBlock; i++)
    spISW->Write_uint8(i & 0xFFU);
  spISW->Write_uint32(0xDEADBEEFUL);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  for (uint_fast16_t i = 0; i < bytesPerBlock-1U; i++)
  {
    ASSERT_EQ(spISR->Read_uint8(), i & 0xFFU);
  }

  // - precondition established -

  spISR->Skip(1U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  ASSERT_EQ(spISR->Read_uint32(), 0xDEADBEEFUL);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::empty);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_LastBitInLastByteOfBlock_NoMoreBlocks)
{
  // This skips the first bit of the last byte in a block. There are no more blocks.

  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  for (uint_fast16_t i = 0; i < bytesPerBlock; i++)
    spISW->Write_uint8(i & 0xFFU);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  for (uint_fast16_t i = 0; i < bytesPerBlock-1U; i++)
  {
    ASSERT_EQ(spISR->Read_uint8(), i & 0xFFU);
  }

  // - precondition established -

  spISR->Skip(1U);
  ASSERT_EQ(spISR->GetState(), gpcc::Stream::IStreamReader::States::open);

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_EmptyStream)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  ASSERT_EQ(spISR->Read_uint8(), 0xFAU);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::open, spISR->GetState());

  ASSERT_EQ(spISR->Read_uint8(), 0x12U);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::empty, spISR->GetState());

  ASSERT_THROW(spISR->Skip(1U), gpcc::Stream::EmptyError);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_ClosedStream)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");
  spISR->Close();

  ASSERT_THROW(spISR->Skip(1U), gpcc::Stream::ClosedError);
  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Skip_StreamInErrorState)
{
  Format(128);

  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  spISW = uut.Create("Test.dat", false);
  spISW->Write_uint8(0xFAU);
  spISW->Write_uint8(0x12U);
  spISW->Close();
  spISW.reset();

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  spISR = uut.Open("Test.dat");

  uint8_t au8[3];
  ASSERT_THROW(spISR->Read_uint8(au8, 3), gpcc::Stream::EmptyError);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());

  ASSERT_THROW(spISR->Skip(1U), gpcc::Stream::ErrorStateError);

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::error, spISR->GetState());

  spISR->Close();

  ASSERT_EQ(gpcc::Stream::IStreamReader::States::closed, spISR->GetState());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Unmount_OK_DifferentStates)
{
  Format(128);
  uut.Unmount();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  uut.MountStep1();
  uut.Unmount();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  uut.MountStep1();
  uut.MountStep2();

  fakeStorage.Invalidate(blockSize, blockSize);
  std::unique_ptr<gpcc::Stream::IStreamWriter> spISW;
  ASSERT_THROW(spISW = uut.Create("Section1", false), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  uut.Unmount();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Unmount_WrongState)
{
  Format(128);
  uut.Unmount();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  ASSERT_THROW(uut.Unmount(), InsufficientStateError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Unmount_LockedByReader)
{
  Format(128);

  RandomData data(8,8);
  data.Write("Section1", false, uut);

  auto spISR = uut.Open("Section1");

  ASSERT_THROW(uut.Unmount(), NotAllSectionsClosedError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  spISR.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Unmount_LockedByWriter)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  ASSERT_THROW(uut.Unmount(), NotAllSectionsClosedError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  spISW.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_BadNames)
{
  Format(128);

  ASSERT_THROW(uut.Delete(""), std::invalid_argument);
  ASSERT_THROW(uut.Delete(" Sec1"), std::invalid_argument);
  ASSERT_THROW(uut.Delete("Sec1 "), std::invalid_argument);
  ASSERT_THROW(uut.Delete(" Sec1 "), std::invalid_argument);
  ASSERT_THROW(uut.Delete(" "), std::invalid_argument);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_WrongState)
{
  Format(128);

  uut.Unmount();

  // state is not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  ASSERT_THROW(uut.Delete("Section1"), InsufficientStateError);

  // bring uut into state "ro_mount"
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());

  ASSERT_THROW(uut.Delete("Section1"), InsufficientStateError);

  // bring uut into state "mounted"
  uut.MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  // bring uut into state "defect"
  fakeStorage.Invalidate(blockSize, blockSize);
  RandomData data(8,8);
  ASSERT_THROW(data.Write("Section2", false, uut), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  ASSERT_THROW(uut.Delete("Section1"), InsufficientStateError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_LockedByReader)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  auto spISR = uut.Open("Section1");

  ASSERT_THROW(uut.Delete("Section1"), gpcc::file_systems::FileAlreadyAccessedError);

  spISR.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_LockedByWriter)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  ASSERT_THROW(uut.Delete("Section1"), gpcc::file_systems::FileAlreadyAccessedError);

  spISW.reset();

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_NoSuchSection)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;
  ASSERT_THROW(uut.Delete("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(0U, fakeStorage.writeAccessCnt);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_Powerfail)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  fakeStorage.SetEnableUndo(true);
  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt  = 0;

  uut.Delete("Section1");

  FakeEEPROM copyOfStorage(fakeStorage);

  size_t const nScenarios = fakeStorage.writeAccessCnt;
  for (size_t i = 1; i < nScenarios; i++)
  {
    uut.Unmount();

    fakeStorage = copyOfStorage;
    fakeStorage.Undo(i);
    fakeStorage.SetEnableUndo(false);

    uut.MountStep1();
    uut.MountStep2();

    std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
    ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);
    ASSERT_EQ(freeSpace, uut.GetFreeSpace());

    BasicTest_WriteRead(&uut, blockSize, 1);
  }

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_OK)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  ASSERT_NO_THROW(uut.Delete("Section1"));
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_FromFullSectionSystem)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  RandomData data1(8, 8);
  data1.Write("Section1", false, uut);
  RandomData data2(freeSpace - (2U * bytesPerBlock) - 8U, freeSpace - (2U * bytesPerBlock) - 8U);
  data2.Write("Section2", false, uut);

  ASSERT_NO_THROW(uut.Delete("Section1"));
  ASSERT_EQ(bytesPerBlock, uut.GetFreeSpace());

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_NO_THROW(uut.Delete("Section2"));
  ASSERT_THROW(spISR = uut.Open("Section2"), gpcc::file_systems::NoSuchFileError);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Delete_FromNonFullSectionSystem)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  RandomData data1(8, 8);
  data1.Write("Section1", false, uut);
  RandomData data2(freeSpace - (3U * bytesPerBlock) - 8U, freeSpace - (3U * bytesPerBlock) - 8U);
  data2.Write("Section2", false, uut);

  ASSERT_EQ(0U, uut.GetFreeSpace());

  ASSERT_NO_THROW(uut.Delete("Section1"));
  ASSERT_EQ(2U * bytesPerBlock, uut.GetFreeSpace());

  std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
  ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_NO_THROW(uut.Delete("Section2"));
  ASSERT_THROW(spISR = uut.Open("Section2"), gpcc::file_systems::NoSuchFileError);
  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_BadNames)
{
  Format(128);

  ASSERT_THROW(uut.Rename("", "Section2"), std::invalid_argument);
  ASSERT_THROW(uut.Rename(" Sec1", "Section2"), std::invalid_argument);
  ASSERT_THROW(uut.Rename("Sec1 ", "Section2"), std::invalid_argument);
  ASSERT_THROW(uut.Rename(" Sec1 ", "Section2"), std::invalid_argument);
  ASSERT_THROW(uut.Rename(" ", "Section2"), std::invalid_argument);

  ASSERT_THROW(uut.Rename("Section1", ""), std::invalid_argument);
  ASSERT_THROW(uut.Rename("Section1", " Sec2"), std::invalid_argument);
  ASSERT_THROW(uut.Rename("Section1", "Sec2 "), std::invalid_argument);
  ASSERT_THROW(uut.Rename("Section1", " Sec2 "), std::invalid_argument);
  ASSERT_THROW(uut.Rename("Section1", " "), std::invalid_argument);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_WrongState)
{
  Format(128);

  uut.Unmount();

  // state is not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  ASSERT_THROW(uut.Rename("Section1", "Section2"), InsufficientStateError);

  // bring uut into state "ro_mount"
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());

  ASSERT_THROW(uut.Rename("Section1", "Section2"), InsufficientStateError);

  // bring uut into state "mounted"
  uut.MountStep2();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  // bring uut into state "defect"
  fakeStorage.Invalidate(blockSize, blockSize);
  RandomData data(8,8);
  ASSERT_THROW(data.Write("Section2", false, uut), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  ASSERT_THROW(uut.Rename("Section1", "Section2"), InsufficientStateError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_SrcLockedByReader)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  auto spISR = uut.Open("Section1");

  ASSERT_THROW(uut.Rename("Section1", "Section2"), gpcc::file_systems::FileAlreadyAccessedError);

  spISR.reset();
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_DestLockedByReader)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  auto spISR = uut.Open("Section1");

  ASSERT_THROW(uut.Rename("Section2", "Section1"), gpcc::file_systems::FileAlreadyAccessedError);

  spISR.reset();
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_SrcLockedByWriter)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  ASSERT_THROW(uut.Rename("Section1", "Section2"), gpcc::file_systems::FileAlreadyAccessedError);

  spISW.reset();
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_DestLockedByWriter)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  auto spISW = uut.Create("Section2", false);

  ASSERT_THROW(uut.Rename("Section1", "Section2"), gpcc::file_systems::FileAlreadyAccessedError);

  spISW.reset();
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_SrcNotFound)
{
  Format(128);

  ASSERT_THROW(uut.Rename("Section1", "Section2"), gpcc::file_systems::NoSuchFileError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_DestAlreadyExisting)
{
  Format(128);

  RandomData data1(8, 8);
  data1.Write("Section1", false, uut);
  RandomData data2(8, 8);
  data2.Write("Section2", false, uut);

  ASSERT_THROW(uut.Rename("Section1", "Section2"), gpcc::file_systems::FileAlreadyExistingError);

  data1.Compare("Section1", uut);
  data2.Compare("Section2", uut);
  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_MaxNameLength)
{
  Format(128);

  size_t const maxNameLength = blockSize - (sizeof(SectionHeadBlock_t) + 1U + sizeof(uint16_t));

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  std::string newName("Section2");
  while (newName.length() < maxNameLength)
    newName.append("x");

  uut.Rename("Section1", newName);

  data.Compare(newName, uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_MaxNameLengthPlus1)
{
  Format(128);

  size_t const maxNameLength = blockSize - (sizeof(SectionHeadBlock_t) + 1U + sizeof(uint16_t));

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  std::string newName("Section2");
  while (newName.length() <= maxNameLength)
    newName.append("x");

  ASSERT_THROW(uut.Rename("Section1", newName), std::invalid_argument);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_NoFreeSpace)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  RandomData data(freeSpace - 8U, freeSpace - 8U);
  data.Write("Section1", false, uut);

  ASSERT_THROW(uut.Rename("Section1", "Section2"), gpcc::file_systems::InsufficientSpaceError);

  data.Compare("Section1", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_OneFreeBlock)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  RandomData data(freeSpace - 8U - bytesPerBlock, freeSpace - 8U - bytesPerBlock);
  data.Write("Section1", false, uut);

  ASSERT_EQ(0U, uut.GetFreeSpace());

  uut.Rename("Section1", "Section2");

  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_SameName_SectionNotExisting)
{
  Format(128);

  size_t const freeSpace = uut.GetFreeSpace();

  ASSERT_THROW(uut.Rename("Section1", "Section1"), gpcc::file_systems::NoSuchFileError);

  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_SameName_SectionExisting)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  size_t const freeSpace = uut.GetFreeSpace();

  ASSERT_THROW(uut.Rename("Section1", "Section1"), gpcc::file_systems::FileAlreadyExistingError);

  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_OK)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  size_t const freeSpace = uut.GetFreeSpace();

  uut.Rename("Section1", "Section2");

  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);

  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_Powerfail)
{
  Format(128);

  RandomData data(8, 8);
  data.Write("Section1", false, uut);

  size_t const freeSpace = uut.GetFreeSpace();

  fakeStorage.SetEnableUndo(true);
  fakeStorage.writeAccessCnt = 0;
  fakeStorage.readAccessCnt  = 0;

  uut.Rename("Section1", "Section2");

  ASSERT_EQ(3U, fakeStorage.writeAccessCnt);

  FakeEEPROM copyOfStorage(fakeStorage);

  size_t const nScenarios = fakeStorage.writeAccessCnt;
  for (size_t i = 1; i < nScenarios; i++)
  {
    uut.Unmount();

    fakeStorage = copyOfStorage;
    fakeStorage.Undo(i);
    fakeStorage.SetEnableUndo(false);

    uut.MountStep1();
    uut.MountStep2();

    std::unique_ptr<gpcc::Stream::IStreamReader> spISR;
    ASSERT_THROW(spISR = uut.Open("Section1"), gpcc::file_systems::NoSuchFileError);
    ASSERT_EQ(freeSpace, uut.GetFreeSpace());

    data.Compare("Section2", uut);

    BasicTest_WriteRead(&uut, blockSize, 1);
  }

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, Rename_VersionWrapAround)
{
  // Scenario in EEPROM:
  // Section: Head (1), Data (2)
  // New head: 3

  Format(128);

  // create section
  RandomData data(8,8);
  data.Write("Section1", false, uut);

  // set version of section
  fakeStorage.Read(1U * blockSize, blockSize, pBuffer);
  pBuffer[offsetof(SectionHeadBlock_t, version) + 0] = 0xFF;
  pBuffer[offsetof(SectionHeadBlock_t, version) + 1] = 0xFF;
  fakeStorage.Write(1U * blockSize, blockSize, pBuffer);
  UpdateCRC(1);

  size_t const freeSpace = uut.GetFreeSpace();

  // rename
  uut.Rename("Section1", "Section2");

  // check version of new section head
  fakeStorage.Read(3U * blockSize, blockSize, pBuffer);
  ASSERT_EQ(0x02, pBuffer[offsetof(CommonBlockHead_t, nextBlock) + 0]);
  ASSERT_EQ(0x00, pBuffer[offsetof(CommonBlockHead_t, nextBlock) + 1]);
  ASSERT_EQ(0x00, pBuffer[offsetof(SectionHeadBlock_t, version) + 0]);
  ASSERT_EQ(0x00, pBuffer[offsetof(SectionHeadBlock_t, version) + 1]);

  ASSERT_THROW(data.Compare("Section1", uut), gpcc::file_systems::NoSuchFileError);
  data.Compare("Section2", uut);

  ASSERT_EQ(freeSpace, uut.GetFreeSpace());

  BasicTest_WriteRead(&uut, blockSize, 1);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, EnumerateSections_WrongState)
{
  Format(128);

  uut.Unmount();

  std::list<std::string> sections;

  // state is not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());

  ASSERT_THROW(sections = uut.Enumerate(), InsufficientStateError);

  // bring uut into state "ro_mount"
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());

  ASSERT_THROW(sections = uut.Enumerate(), InsufficientStateError);

  // bring uut into state "mounted"
  uut.MountStep2();

  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());

  // bring uut into state "defect"
  fakeStorage.Invalidate(blockSize, blockSize);
  RandomData data(8,8);
  ASSERT_THROW(data.Write("Section2", false, uut), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());

  ASSERT_THROW(sections = uut.Enumerate(), InsufficientStateError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, EnumerateSections_Zero_One_n)
{
  Format(128);

  std::list<std::string> sections;

  // zero sections
  sections = uut.Enumerate();
  ASSERT_TRUE(sections.empty());

  // one section
  RandomData data1(10, 10);
  data1.Write("Section1", false, uut);

  sections = uut.Enumerate();
  ASSERT_EQ(1U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Section1") != sections.end());

  // two sections
  RandomData data2(10, 10);
  data1.Write("Section2", false, uut);

  sections = uut.Enumerate();
  ASSERT_EQ(2U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Section1") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Section2") != sections.end());

  // three sections
  RandomData data3(10, 10);
  data1.Write("Section3", false, uut);

  sections = uut.Enumerate();
  ASSERT_EQ(3U, sections.size());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Section1") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Section2") != sections.end());
  ASSERT_TRUE(std::find(sections.begin(), sections.end(), "Section3") != sections.end());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, EnumerateSections_Sort)
{
  Format(128);

  // create some sections
  RandomData data1(10, 10);
  data1.Write("B_Section", false, uut);

  RandomData data2(10, 10);
  data2.Write("A_Section", false, uut);

  RandomData data3(10, 10);
  data3.Write("Z_Section", false, uut);

  RandomData data4(10, 10);
  data4.Write("A_Section2", false, uut);

  RandomData data5(10, 10);
  data5.Write("C_Section", false, uut);

  auto const sections = uut.Enumerate();

  auto it = sections.begin();
  ASSERT_TRUE(*it == "A_Section");
  ++it;
  ASSERT_TRUE(*it == "A_Section2");
  ++it;
  ASSERT_TRUE(*it == "B_Section");
  ++it;
  ASSERT_TRUE(*it == "C_Section");
  ++it;
  ASSERT_TRUE(*it == "Z_Section");
  ++it;
  ASSERT_TRUE(it == sections.end());

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_WrongState)
{
  Format(128);

  fakeStorage.writeAccessCnt = 0;
  RandomData data(10,10);
  data.Write("Section1", false, uut);
  ASSERT_EQ(2U, fakeStorage.writeAccessCnt);

  uut.Unmount();

  size_t size;

  // state is not_mounted
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, uut.GetState());
  ASSERT_THROW(size = uut.DetermineSize("Section1", nullptr), InsufficientStateError);

  // bring uut into state "ro_mount"
  uut.MountStep1();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::ro_mount, uut.GetState());
  ASSERT_THROW(size = uut.DetermineSize("Section1", nullptr), InsufficientStateError);

  // bring uut into state "mounted"
  uut.MountStep2();
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::mounted, uut.GetState());
  ASSERT_NO_THROW(size = uut.DetermineSize("Section1", nullptr));
  ASSERT_EQ(18U, size);

  // bring uut into state "defect"
  fakeStorage.Invalidate(3 * blockSize, blockSize);
  RandomData data2(8,8);
  ASSERT_THROW(data2.Write("Section2", false, uut), DataIntegrityError);
  ASSERT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::defect, uut.GetState());
  ASSERT_THROW(size = uut.DetermineSize("Section1", nullptr), InsufficientStateError);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_BadNames)
{
  Format(128);

  size_t size;

  ASSERT_THROW(size = uut.DetermineSize("", nullptr), std::invalid_argument);
  ASSERT_THROW(size = uut.DetermineSize(" Sec1", nullptr), std::invalid_argument);
  ASSERT_THROW(size = uut.DetermineSize("Sec1 ", nullptr), std::invalid_argument);
  ASSERT_THROW(size = uut.DetermineSize(" Sec1 ", nullptr), std::invalid_argument);
  ASSERT_THROW(size = uut.DetermineSize(" ", nullptr), std::invalid_argument);

  (void)size;

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_LockedByWriter)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);

  size_t size;
  ASSERT_THROW(size = uut.DetermineSize("Section1", nullptr), gpcc::file_systems::FileAlreadyAccessedError);
  (void)size;

  spISW.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_LockedByReader)
{
  Format(128);

  RandomData data(8,8);
  data.Write("Section1", false, uut);

  auto spISR = uut.Open("Section1");

  size_t size;
  ASSERT_NO_THROW(size = uut.DetermineSize("Section1", nullptr));
  ASSERT_EQ(16U, size);

  spISR.reset();

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_SectionNotExisting)
{
  Format(128);

  size_t size;
  ASSERT_THROW(size = uut.DetermineSize("Section1", nullptr), gpcc::file_systems::NoSuchFileError);
  (void)size;

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_ZeroLength)
{
  Format(128);

  auto spISW = uut.Create("Section1", false);
  spISW.reset();

  size_t size;
  size_t totalSize;
  size = uut.DetermineSize("Section1", &totalSize);
  ASSERT_EQ(0U, size);
  ASSERT_EQ(2U * blockSize, totalSize);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, DetermineSectionSize_VariousLength)
{
  Format(128);

  RandomData data1(8, 8);
  data1.Write("Section1", false, uut);

  RandomData data2(3U * blockSize, 3U * blockSize);
  data2.Write("Section2", false, uut);

  size_t size;
  size_t totalSize;

  size = uut.DetermineSize("Section1", &totalSize);
  ASSERT_EQ(16U, size);
  ASSERT_EQ(2U * blockSize, totalSize);

  size = uut.DetermineSize("Section2", &totalSize);
  ASSERT_EQ(3U * blockSize + 8U, size);
  ASSERT_EQ(5U * blockSize, totalSize);

  size = uut.DetermineSize("Section1", nullptr);
  ASSERT_EQ(16U, size);

  size = uut.DetermineSize("Section2", nullptr);
  ASSERT_EQ(3U * blockSize + 8U, size);

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MaximumNumberOfSections)
{
  Format(128);

  std::list<RandomData> sections;

  size_t const nSections = ((storageSize / blockSize) - 1U) / 2U;
  for (size_t i = 0; i < nSections; i++)
  {
    RandomData section(0, bytesPerBlock - 8U);
    std::string const secName = "Section" + std::to_string(i);
    section.Write(secName, false, uut);
    sections.push_back(std::move(section));
  }

  ASSERT_EQ(0U, uut.GetFreeSpace());

  auto it = sections.begin();
  for (size_t i = 0; i < nSections; i++)
  {
    std::string const secName = "Section" + std::to_string(i);
    (*it).Compare(secName, uut);
    ++it;
  }

  uut.Unmount();
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_TestsF, MultipleReadersAndWritersAtTheSameTime)
{
  Format(128);

  RandomData data1(30, 30);
  data1.Write("Data1", false, uut);
  RandomData data2(30, 30);
  data2.Write("Data2", false, uut);
  RandomData data3(30, 30);
  data3.Write("Data3", false, uut);

  auto spISW1 = uut.Create("Section1", false);
  *spISW1 << std::string("ABC");
  auto spISW2 = uut.Create("Section2", false);
  auto spISW3 = uut.Create("Section3", false);

  *spISW2 << std::string("DEF");
  data1.Compare("Data1", uut);
  *spISW3 << std::string("GHI");
  spISW1->Write_uint8(12);
  spISW2->Write_uint8(13);
  data2.Compare("Data2", uut);
  spISW3->Write_uint8(14);

  spISW1->Close();
  spISW1.reset();
  data3.Compare("Data3", uut);
  spISW3->Close();
  spISW3.reset();
  spISW2->Close();
  spISW2.reset();

  uut.Unmount();
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests

