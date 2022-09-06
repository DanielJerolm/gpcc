/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/crc/simple_crc.hpp>
#include "gpcc/src/file_systems/eeprom_section_system/internal/BlockAccessor.hpp"
#include "gpcc/src/file_systems/eeprom_section_system/internal/EEPROMSectionSystemInternals.hpp"
#include <gpcc/file_systems/eeprom_section_system/EEPROMSectionSystem.hpp>
#include <gpcc/file_systems/eeprom_section_system/exceptions.hpp>
#include "../FakeEEPROM.hpp"
#include "gtest/gtest.h"
#include <limits>
#include <stdexcept>
#include <cstring>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{
namespace internal
{

using namespace testing;
using namespace gpcc::file_systems::EEPROMSectionSystem;
using namespace gpcc::file_systems::EEPROMSectionSystem::internal;

// ================================================================================================
// Some valid dummy section system blocks, can be used in tests
// ================================================================================================
static const uint8_t testBlock_SectionSystemInfo[] =
{
 0x00, // type
 0x00, // sectionNameHash
 0x12, // nBytes LB
 0x00, // nBytes HB
 0xB1, // totalNbOfWrites LB
 0xB2, // totalNbOfWrites
 0xB3, // totalNbOfWrites
 0xB4, // totalNbOfWrites HB
 0xFF, // nextBlock LB
 0xFF, // nextBlock HB
 0x11, // sectionSystemVersion LB
 0x22, // sectionSystemVersion HB
 0x40, // blockSize LB
 0x00, // blockSize HB
 0x04, // nBlocks LB
 0x00, // nBlocks HB
 0x00, // CRC LB
 0x00  // CRC HB
};

static const uint8_t testBlock_FreeBlock[] =
{
 0x01, // type
 0x00, // sectionNameHash
 0x0C, // nBytes LB
 0x00, // nBytes HB
 0xC1, // totalNbOfWrites LB
 0xC2, // totalNbOfWrites
 0xC3, // totalNbOfWrites
 0xC4, // totalNbOfWrites HB
 0xFF, // nextBlock LB
 0xFF, // nextBlock HB
 0x00, // CRC LB
 0x00  // CRC HB
};

static const uint8_t testBlock_SectionHead[] =
{
  0x02, // type
  0xA0, // sectionNameHash
  0x13, // nBytes LB
  0x00, // nBytes HB
  0xA1, // totalNbOfWrites LB
  0xA2, // totalNbOfWrites
  0xA3, // totalNbOfWrites
  0xA4, // totalNbOfWrites HB
  0x03, // nextBlock LB
  0x00, // nextBlock HB
  0x67, // version LB
  0x89, // version HB
  'T',  // name[0]
  'e',  // name[1]
  's',  // name[2]
  't',  // name[3]
   0x00,// name[4]
   0x00,// CRC LB
   0x00 // CRC HB
};

static const uint8_t testBlock_SectionData[] =
{
 0x03, // type
 0x00, // sectionNameHash
 0x12, // nBytes LB
 0x00, // nBytes HB
 0xD1, // totalNbOfWrites LB
 0xD2, // totalNbOfWrites
 0xD3, // totalNbOfWrites
 0xD4, // totalNbOfWrites HB
 0xFF, // nextBlock LB
 0xFF, // nextBlock HB
 0x01, // seqNb LB
 0x00, // seqNb HB
 0x01, // Data[0]
 0x02, // Data[1]
 0x03, // Data[2]
 0x04, // Data[3]
 0x00, // CRC LB
 0x00  // CRC HB
};

static const uint8_t testBlock_InvalidType[] =
{
 0x04, // type
 0x00, // sectionNameHash
 0x0C, // nBytes LB
 0x00, // nBytes HB
 0xC1, // totalNbOfWrites LB
 0xC2, // totalNbOfWrites
 0xC3, // totalNbOfWrites
 0xC4, // totalNbOfWrites HB
 0xFF, // nextBlock LB
 0xFF, // nextBlock HB
 0x00, // CRC LB
 0x00  // CRC HB
};

// ================================================================================================
// ================================================================================================

// Test fixture for BlockAccessor. Provides an UUT and an FakeEEPROM with 1kB RAM and some test sections.
// Block 0: copy of testBlock_SectionSystemInfo
// Block 1: copy of testBlock_FreeBlock
// Block 2: copy of testBlock_SectionHead
// Block 3: copy of testBlock_SectionData
class GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture: public Test
{
  public:
    GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture(void);

  protected:
    FakeEEPROM eeprom;
    BlockAccessor uut;

    uint8_t buffer[64];
    uint8_t copyOfBuffer[64];

    void SetUp(void) override;
    void TearDown(void) override;

    void UpdateCRC(uint16_t const index);

    void CopyBuffer(size_t const n = 64);
    bool VerifyNoChangeInBuffer(size_t const n = 64);
    bool VerifyNoChangeInBufferExceptCRCAndTNOW(void);
    bool VerifyNoChangeInBufferExceptCRC(void);
};

GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture()
: Test()
, eeprom(1024, 64)
, uut(eeprom, 0, 1024)
, buffer()
{
}
void GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::SetUp(void)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 64);
  uut.SetBlockSize(64);

  eeprom.Write(0,   sizeof(testBlock_SectionSystemInfo), testBlock_SectionSystemInfo);
  UpdateCRC(0);
  eeprom.Write(64,  sizeof(testBlock_FreeBlock), testBlock_FreeBlock);
  UpdateCRC(1);
  eeprom.Write(128, sizeof(testBlock_SectionHead), testBlock_SectionHead);
  UpdateCRC(2);
  eeprom.Write(192, sizeof(testBlock_SectionData), testBlock_SectionData);
  UpdateCRC(3);
  eeprom.Write(256, sizeof(testBlock_InvalidType), testBlock_InvalidType);
  UpdateCRC(4);

  eeprom.readAccessCnt = 0;
  eeprom.writeAccessCnt = 0;
}
void GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::TearDown(void)
{
}

void GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::UpdateCRC(uint16_t const index)
{
  eeprom.Read(64 * index, 64, buffer);

  uint16_t const nBytes = buffer[offsetof(CommonBlockHead_t, nBytes)] |
                          (static_cast<uint16_t>(buffer[offsetof(CommonBlockHead_t, nBytes) + 1]) << 8U);

  if ((nBytes < sizeof(CommonBlockHead_t) + sizeof(uint16_t)) ||
      (nBytes >= sizeof(buffer)))
    throw std::runtime_error("Bad nBytes in fake EEPROM block");

  uint16_t crc = 0xFFFF;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, nBytes - 2, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;

  eeprom.Write(64 * index + (nBytes - 2), 2, buffer);
}

void GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::CopyBuffer(size_t const n)
{
  if (n > sizeof(buffer))
    throw std::invalid_argument("CopyBuffer: n bad");

  memcpy(copyOfBuffer, buffer, n);
}
bool GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::VerifyNoChangeInBuffer(size_t const n)
{
  if (n > sizeof(buffer))
    throw std::invalid_argument("VerifyNoChangeInBuffer: n bad");

  return (memcmp(buffer, copyOfBuffer, n) == 0);
}
bool GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::VerifyNoChangeInBufferExceptCRCAndTNOW(void)
{
  size_t const n = copyOfBuffer[offsetof(CommonBlockHead_t, nBytes)] |
                   static_cast<uint16_t>(copyOfBuffer[offsetof(CommonBlockHead_t, nBytes) + 1U]) << 8U;

  if ((n < 11) ||
      (n > 64))
    throw std::runtime_error("VerifyNoChangeInBufferExceptCRCAndTNOW: Bad \"nBytes\" in 'copyOfBuffer'");

  return ((memcmp(buffer, copyOfBuffer, 4) == 0) && (memcmp(buffer + 8, copyOfBuffer + 8, n - 10) == 0));
}
bool GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture::VerifyNoChangeInBufferExceptCRC(void)
{
  size_t const n = copyOfBuffer[offsetof(CommonBlockHead_t, nBytes)] |
                   static_cast<uint16_t>(copyOfBuffer[offsetof(CommonBlockHead_t, nBytes) + 1U]) << 8U;

  if ((n < 3) ||
      (n > 64))
    throw std::runtime_error("VerifyNoChangeInBufferExceptCRCAndTNOW: Bad \"nBytes\" in 'copyOfBuffer'");

  return (memcmp(buffer, copyOfBuffer, n - 2) == 0);
}

// ================================================================================================
// ================================================================================================

TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation1)
{
  // test creation with standard parameters
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024, 64);
  BlockAccessor uut1(eeprom, 0, 512);
  BlockAccessor uut2(eeprom, 0, 1024);
  BlockAccessor uut3(eeprom, 64, 512);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation2)
{
  // test creation with no page size specified for underlying storage
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut1(eeprom, 0, 512);
  BlockAccessor uut2(eeprom, 0, 1024);
  BlockAccessor uut3(eeprom, 64, 600);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation_BadPageAlignment1)
{
  // check bad page alignment of start address in storage

  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024, 64);

  EXPECT_THROW(BlockAccessor uut(eeprom, 1, 512), std::invalid_argument);
  EXPECT_NO_THROW(BlockAccessor uut(eeprom, 64, 512));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation_BadPageAlignment2)
{
  // check bad page alignment of size in storage

  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024, 64);

  EXPECT_THROW(BlockAccessor uut(eeprom, 0, 511), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation_PageAlignmentDontCare)
{
  // check behaviour if no page size specified for underlying storage
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024, 0);

  EXPECT_NO_THROW(BlockAccessor uut(eeprom, 1, 512));
  EXPECT_NO_THROW(BlockAccessor uut(eeprom, 1, 600));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation_MinimumSize)
{
  // check creation with minimum size in storage

  FakeEEPROM eeprom(1024, 0);

  EXPECT_THROW(BlockAccessor uut(eeprom, 0, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks - 1U), std::invalid_argument);
  EXPECT_NO_THROW(BlockAccessor uut(eeprom, 0, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation_MemRangeOutOf32Bit)
{
  // check behaviour if memory range occupied in storage exceeds 32bit address space
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024, 64);

  EXPECT_THROW(BlockAccessor uut(eeprom, std::numeric_limits<uint32_t>::max() - gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks + 1, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, Creation_MemRangeOutOfStorage)
{
  // check behaviour if memory range occupied in storage exceeds end of storage

  FakeEEPROM eeprom(1024, 0);

  EXPECT_THROW(BlockAccessor uut(eeprom, 1024 - gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks + 1, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks), std::invalid_argument);
  EXPECT_NO_THROW(BlockAccessor uut(eeprom, 1024 - gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks));
}

TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetSizeInStorage)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024,64);
  BlockAccessor uut(eeprom, 0, 512);

  ASSERT_EQ(512U, uut.GetSizeInStorage());
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetPageSize)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom(1024,64);
  BlockAccessor uut(eeprom, 0, 512);

  ASSERT_EQ(64U, uut.GetPageSize());
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_Min)
{
  // check: set minimum block size
  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  ASSERT_THROW(uut.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize - 1U), std::invalid_argument);
  ASSERT_NO_THROW(uut.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_BetweenMinMax)
{
  // check: set block size between minimum and maximum
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize < 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize > 64);

  FakeEEPROM eeprom(1024, 128);
  BlockAccessor uut(eeprom, 0, 1024);

  ASSERT_NO_THROW(uut.SetBlockSize(64));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_GreaterThanPageSize)
{
  // check: block size larger than page size of the underlying storage
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize > 64);

  FakeEEPROM eeprom(1024, 64);
  BlockAccessor uut(eeprom, 0, 1024);

  ASSERT_THROW(uut.SetBlockSize(128), std::invalid_argument);
  ASSERT_THROW(uut.SetBlockSize(65), std::invalid_argument);
  ASSERT_NO_THROW(uut.SetBlockSize(64));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_Max)
{
  // check: block size larger than page size of the underlying storage
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize > 64);

  FakeEEPROM eeprom(2 * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks, 0);
  BlockAccessor uut(eeprom, 0, 2 * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks);

  ASSERT_THROW(uut.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize + 1), std::invalid_argument);
  ASSERT_NO_THROW(uut.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_Alignment)
{
  // check: block size violates page size of underlying storage
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 128);
  BlockAccessor uut(eeprom, 0, 1024);

  ASSERT_THROW(uut.SetBlockSize(120), std::invalid_argument);
  ASSERT_NO_THROW(uut.SetBlockSize(128));
  ASSERT_NO_THROW(uut.SetBlockSize(64));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_NoAlignmentRequired)
{
  // check: block size set and no page size specified by underlying storage
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 70);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  ASSERT_NO_THROW(uut.SetBlockSize(70));
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_MinNbOfBlocks)
{
  // check: resulting number of blocks too small
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(2*gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize);

  FakeEEPROM eeprom(2 * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks, 0);

  BlockAccessor uut1(eeprom, 0, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks);
  ASSERT_NO_THROW(uut1.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize));

  BlockAccessor uut2(eeprom, 0, (2*gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize) * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumNbOfBlocks - 1);
  ASSERT_THROW(uut2.SetBlockSize(2*gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_MaxNbOfBlocks)
{
  // check: resulting number of blocks too large
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);

  FakeEEPROM eeprom((gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumNbOfBlocks + 1) * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize, 0);

  BlockAccessor uut1(eeprom, 0, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumNbOfBlocks * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize);
  ASSERT_NO_THROW(uut1.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize));

  BlockAccessor uut2(eeprom, 0, (gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumNbOfBlocks + 1) * gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize);
  ASSERT_THROW(uut2.SetBlockSize(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize), std::invalid_argument);
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, SetBlockSize_NoUpdateInCaseOfError)
{
  // check: block size is not altered in case of an error
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  uut.SetBlockSize(64);
  ASSERT_EQ(64U, uut.GetBlockSize());

  ASSERT_THROW(uut.SetBlockSize(1024), std::invalid_argument);
  ASSERT_EQ(64U, uut.GetBlockSize());

  uut.SetBlockSize(128);
  ASSERT_EQ(128U, uut.GetBlockSize());
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetBlockSize_NotConfigured)
{
  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  uint16_t dummy = 0;
  ASSERT_THROW(dummy = uut.GetBlockSize(), std::logic_error);
  (void)dummy;
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetBlockSize)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  uut.SetBlockSize(64);
  ASSERT_EQ(64U, uut.GetBlockSize());
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetnBlocks_NotConfigured)
{
  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  uint16_t dummy = 0;
  ASSERT_THROW(dummy = uut.GetnBlocks(), std::logic_error);
  (void)dummy;
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetnBlocks)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  uut.SetBlockSize(64);
  ASSERT_EQ(1024/64, uut.GetnBlocks());
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetMaxSectionNameLength_NotConfigured)
{
  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  size_t dummy = 0;
  ASSERT_THROW(dummy = uut.GetMaxSectionNameLength(), std::logic_error);
  (void)dummy;
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, GetMaxSectionNameLength)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);

  uut.SetBlockSize(64);
  ASSERT_EQ(64 - (sizeof(SectionHeadBlock_t) + sizeof(uint16_t) + 1U), uut.GetMaxSectionNameLength());
}

TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, LoadFieldFuncs_NotConfigured)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);
  eeprom.Write(64, sizeof(testBlock_SectionHead), testBlock_SectionHead);

  uint32_t dummy;
  ASSERT_THROW(dummy = uut.LoadFields_type_sectionNameHash(1), std::logic_error);
  ASSERT_THROW(dummy = uut.LoadField_type(1), std::logic_error);
  ASSERT_THROW(dummy = uut.LoadField_totalNbOfWrites(1), std::logic_error);
  ASSERT_THROW(dummy = uut.LoadField_nextBlock(1), std::logic_error);
  (void)dummy;
}
TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, LoadFieldFuncs)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);
  uut.SetBlockSize(64);
  eeprom.Write(64, sizeof(testBlock_SectionHead), testBlock_SectionHead);

  ASSERT_EQ(0xA002U, uut.LoadFields_type_sectionNameHash(1));
  ASSERT_EQ(0x02U, uut.LoadField_type(1));
  ASSERT_EQ(0xA4A3A2A1U, uut.LoadField_totalNbOfWrites(1));
  ASSERT_EQ(0x0003U, uut.LoadField_nextBlock(1));
}

TEST(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_Tests, LoadBlock_NotConfigured)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM eeprom(1024, 0);
  BlockAccessor uut(eeprom, 0, 1024);
  eeprom.Write(64, sizeof(testBlock_SectionSystemInfo), testBlock_SectionSystemInfo);

  uint8_t buffer[64];

  ASSERT_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)), std::logic_error);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_InvalidIndex)
{
  eeprom.Read(0, 64, buffer);
  eeprom.Write((uut.GetnBlocks() - 1U) * 64U, 64, buffer);

  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));
  ASSERT_NO_THROW(uut.LoadBlock(uut.GetnBlocks() - 1U, buffer, sizeof(buffer)));
  ASSERT_THROW(uut.LoadBlock(uut.GetnBlocks(), buffer, sizeof(buffer)), std::logic_error);
  ASSERT_THROW(uut.LoadBlock(NOBLOCK, buffer, sizeof(buffer)), std::logic_error);
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_Bad_nBytes)
{
  ASSERT_EQ(64U, uut.GetBlockSize());

  // too small nBytes
  buffer[0] = (sizeof(CommonBlockHead_t) + sizeof(uint16_t)) - 1;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);
  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);

  // too large nBytes
  buffer[0] = 65;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);
  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_tooLarge)
{
  ASSERT_THROW(uut.LoadBlock(0, buffer, (sizeof(SectionSystemInfoBlock_t) + sizeof(uint16_t)) - 1U), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_CRCError)
{
  uint32_t addr = sizeof(testBlock_SectionSystemInfo) - 2U;
  uint16_t crc;
  eeprom.Read(addr, 2, &crc);
  crc = ~crc;
  eeprom.Write(addr, 2, &crc);

  ASSERT_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)), CRCError);
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionSystemInfo_OK)
{
  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionSystemInfo_nBytesInvalid)
{
  // set nBytes from 18 to 19
  buffer[0] = 19;
  buffer[1] = 0;
  eeprom.Write(offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // clear old CRC
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(sizeof(testBlock_SectionSystemInfo) - 2U , 2, buffer);

  // read block
  eeprom.Read(0, 19, buffer);

  // calculate new CRC and update eeprom
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 17, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(sizeof(testBlock_SectionSystemInfo) - 1U , 2, buffer);

  ASSERT_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)), InvalidHeaderError);

  // set nBytes from 18 to 17
  buffer[0] = 17;
  buffer[1] = 0;
  eeprom.Write(offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // clear old CRC
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(sizeof(testBlock_SectionSystemInfo) - 2U , 2, buffer);

  // read block
  eeprom.Read(0, 17, buffer);

  // calculate new CRC and update eeprom
  crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 15, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(sizeof(testBlock_SectionSystemInfo) - 3U , 2, buffer);

  ASSERT_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionSystemInfo_SectionNameHashInvalid)
{
  buffer[0] = 1;
  eeprom.Write(offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);
  UpdateCRC(0);

  ASSERT_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionSystemInfo_NextBlockInvalid)
{
  buffer[0] = 1;
  buffer[1] = 0;
  eeprom.Write(offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(0);

  ASSERT_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_FreeBlock_OK)
{
  ASSERT_NO_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_FreeBlock_nBytesInvalid)
{
  // set nBytes from 12 to 13
  buffer[0] = 13;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // clear old CRC
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(64 + sizeof(testBlock_FreeBlock) - 2U , 2, buffer);

  // read block
  eeprom.Read(64, 13, buffer);

  // calculate new CRC and update eeprom
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 11, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(64 + sizeof(testBlock_FreeBlock) - 1U , 2, buffer);

  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);

  // set nBytes from 12 to 11
  buffer[0] = 11;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // clear old CRC
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(64 + sizeof(testBlock_FreeBlock) - 2U , 2, buffer);

  // read block
  eeprom.Read(64, 11, buffer);

  // calculate new CRC and update eeprom
  crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 9, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(64 + sizeof(testBlock_FreeBlock) - 3U , 2, buffer);

  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_FreeBlock_SectionNameHashInvalid)
{
  buffer[0] = 1;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);
  UpdateCRC(1);

  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_FreeBlock_NextBlock)
{
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(1);

  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = 1;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(1);

  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = uut.GetnBlocks() - 1;
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(1);

  ASSERT_NO_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)));

  buffer[0] = uut.GetnBlocks();
  buffer[1] = 0;
  eeprom.Write(64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(1);

  ASSERT_THROW(uut.LoadBlock(1, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionHead_OK)
{
  ASSERT_NO_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionHead_nBytes)
{
  // set nBytes to 15
  buffer[0] = 15;
  buffer[1] = 0;
  eeprom.Write(2 * 64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // read block
  eeprom.Read(2 * 64, 15, buffer);

  // calculate new CRC and update eeprom
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 13, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(2 * 64 + sizeof(SectionHeadBlock_t) + 1U , 2, buffer);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);

  // set nBytes to 16
  buffer[0] = 16;
  buffer[1] = 0;
  eeprom.Write(2 * 64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // set SectionNameHash
  buffer[0] = 'T';
  eeprom.Write(2 * 64 + offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);

  // set text
  buffer[0] = 'T';
  buffer[1] = 0;
  eeprom.Write(2 * 64 + sizeof(SectionHeadBlock_t), 2, buffer);

  // read block
  eeprom.Read(2 * 64, 16, buffer);

  // calculate new CRC and update eeprom
  crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 14, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(2 * 64 + sizeof(SectionHeadBlock_t) + 2U , 2, buffer);

  ASSERT_NO_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionHead_NoNullTerminator)
{
  buffer[0] = 'X';
  eeprom.Write(2 * 64 + sizeof(testBlock_SectionHead) - 3, 1, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionHead_MultipleNullTerminator)
{
  buffer[0] = 0;
  eeprom.Write(2 * 64 + sizeof(testBlock_SectionHead) - 6, 1, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionHead_BadSectionNameHash)
{
  eeprom.Read(2* 64 + offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);
  buffer[0] = ~buffer[0];
  eeprom.Write(2* 64 + offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionHead_NextBlock)
{
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(2* 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = 0x2;
  buffer[1] = 0x0;
  eeprom.Write(2* 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = uut.GetnBlocks() - 1;
  buffer[1] = 0x0;
  eeprom.Write(2 * 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(2);

  ASSERT_NO_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)));

  buffer[0] = uut.GetnBlocks();
  buffer[1] = 0x0;
  eeprom.Write(2* 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = 0xFF;
  buffer[1] = 0xFF;
  eeprom.Write(2* 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(2);

  ASSERT_THROW(uut.LoadBlock(2, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionData_OK)
{
  ASSERT_NO_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)));
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionData_nBytes)
{
  // set nBytes to 14
  buffer[0] = 14;
  buffer[1] = 0;
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // read block
  eeprom.Read(3 * 64, 14, buffer);

  // calculate new CRC and update eeprom
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 12, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(3 * 64 + 12 , 2, buffer);

  ASSERT_NO_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)));

  // set nBytes to 13
  buffer[0] = 13;
  buffer[1] = 0;
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, nBytes), 2, buffer);

  // read block
  eeprom.Read(3 * 64, 13, buffer);

  // calculate new CRC and update eeprom
  crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, buffer, 11, gpcc::crc::crc16_ccitt_table_normal);
  buffer[0] = crc & 0xFF;
  buffer[1] = crc >> 8U;
  eeprom.Write(3 * 64 + 11 , 2, buffer);

  ASSERT_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionData_BadSectionNameHash)
{
  eeprom.Read(3 * 64 + offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);
  buffer[0] = ~buffer[0];
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, sectionNameHash), 1, buffer);
  UpdateCRC(3);

  ASSERT_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionData_SeqNb)
{
  buffer[0] = uut.GetnBlocks() - 2;
  eeprom.Write(3 * 64 + offsetof(DataBlock_t, seqNb), 1, buffer);
  UpdateCRC(3);

  ASSERT_NO_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)));

  buffer[0] = uut.GetnBlocks() - 1;
  eeprom.Write(3 * 64 + offsetof(DataBlock_t, seqNb), 1, buffer);
  UpdateCRC(3);

  ASSERT_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_SectionData_NextBlock)
{
  buffer[0] = 0;
  buffer[1] = 0;
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(3);

  ASSERT_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = 0x3;
  buffer[1] = 0x0;
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(3);

  ASSERT_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)), InvalidHeaderError);

  buffer[0] = uut.GetnBlocks() - 1;
  buffer[1] = 0x0;
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(3);

  ASSERT_NO_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)));

  buffer[0] = uut.GetnBlocks();
  buffer[1] = 0x0;
  eeprom.Write(3 * 64 + offsetof(CommonBlockHead_t, nextBlock), 2, buffer);
  UpdateCRC(3);

  ASSERT_THROW(uut.LoadBlock(3, buffer, sizeof(buffer)), InvalidHeaderError);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, LoadBlock_InvalidTypeField)
{
  ASSERT_THROW(uut.LoadBlock(4, buffer, sizeof(buffer)), InvalidHeaderError);
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_NotConfigured)
{
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MinimumBlockSize <= 64);
  ASSERT_TRUE(gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem::MaximumBlockSize >= 128);

  FakeEEPROM _eeprom(1024, 0);
  BlockAccessor _uut(_eeprom, 0, 1024);

  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));

  CopyBuffer();
  ASSERT_THROW(_uut.StoreBlock(0, buffer, nullptr, false), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_InvalidIndex)
{
  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));

  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(uut.GetnBlocks() - 1U, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(uut.GetnBlocks(), buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(NOBLOCK, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());
}

TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_Bad_nBytes)
{
  eeprom.writeAccessCnt = 0;

  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));
  CommonBlockHead_t * pHead = reinterpret_cast<CommonBlockHead_t *>(buffer);
  pHead->nBytes = 11;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  uint8_t buffer2[128];
  memcpy(buffer2, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));
  pHead = reinterpret_cast<CommonBlockHead_t *>(buffer2);
  pHead->nBytes = 65;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_Bad_nextBlock)
{
  eeprom.writeAccessCnt = 0;

  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));
  CommonBlockHead_t * const pHead = reinterpret_cast<CommonBlockHead_t *>(buffer);
  pHead->nextBlock = 0;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(1, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  pHead->nextBlock = 1; // self
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(1, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  pHead->nextBlock = uut.GetnBlocks();
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(1, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());
  ASSERT_EQ(0U, eeprom.writeAccessCnt);

  pHead->nextBlock = uut.GetnBlocks() - 1U;
  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(1, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());
  ASSERT_EQ(1U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_IncTotalNbOfWrites)
{
  uut.LoadBlock(0, buffer, sizeof(buffer));
  uut.StoreBlock(0, buffer, nullptr, true);
  uut.LoadBlock(0, buffer, sizeof(buffer));

  CommonBlockHead_t const * const pHead = reinterpret_cast<CommonBlockHead_t *>(buffer);
  ASSERT_EQ(0xB4B3B2B2U, pHead->totalNbOfWrites);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_CRCUpdate)
{
  uut.LoadBlock(0, buffer, sizeof(buffer));
  CopyBuffer();
  eeprom.Invalidate(0, 64);

  uut.StoreBlock(0, buffer, nullptr, true);
  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));

  uut.LoadBlock(0, buffer, sizeof(buffer));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionSystemInfo_OK)
{
  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));

  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  memset(buffer, 0, sizeof(buffer));
  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionSystemInfo_nBytesInvalid)
{
  eeprom.writeAccessCnt = 0;

  SectionSystemInfoBlock_t * const pHead = reinterpret_cast<SectionSystemInfoBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));
  pHead->head.nBytes = 17;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));
  pHead->head.nBytes = 19;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionSystemInfo_SectionNameHashInvalid)
{
  eeprom.writeAccessCnt = 0;

  SectionSystemInfoBlock_t * const pHead = reinterpret_cast<SectionSystemInfoBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));
  pHead->head.sectionNameHash = 1;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionSystemInfo_NextBlockInvalid)
{
  eeprom.writeAccessCnt = 0;

  SectionSystemInfoBlock_t * const pHead = reinterpret_cast<SectionSystemInfoBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionSystemInfo, sizeof(testBlock_SectionSystemInfo));
  pHead->head.nextBlock = 1;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_FreeBlock_OK)
{
  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));

  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  memset(buffer, 0, sizeof(buffer));
  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_FreeBlock_nBytesInvalid)
{
  eeprom.writeAccessCnt = 0;

  CommonBlockHead_t * const pHead = reinterpret_cast<CommonBlockHead_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));
  pHead->nBytes = 11;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));
  pHead->nBytes = 13;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_FreeBlock_SectionNameHashInvalid)
{
  eeprom.writeAccessCnt = 0;

  CommonBlockHead_t * const pHead = reinterpret_cast<CommonBlockHead_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));
  pHead->sectionNameHash = 1;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionHead_OK)
{
  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));

  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  memset(buffer, 0, sizeof(buffer));
  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionHead_nBytes)
{
  eeprom.writeAccessCnt = 0;

  SectionHeadBlock_t * const pHead = reinterpret_cast<SectionHeadBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  pHead->head.nBytes = 15;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  pHead->head.nBytes = 16;
  pHead->head.sectionNameHash = 'A';
  buffer[sizeof(SectionHeadBlock_t) + 0] = 'A';
  buffer[sizeof(SectionHeadBlock_t) + 1] = 0x00;
  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(1U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  pHead->head.nBytes = 64;
  pHead->head.sectionNameHash = static_cast<uint8_t>('A' * 49U);
  for (size_t i = 0; i < 49; i++)
    buffer[sizeof(SectionHeadBlock_t) + i] = 'A';
  buffer[sizeof(SectionHeadBlock_t) + 49] = 0x00;
  CopyBuffer();

  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(2U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  pHead->head.nBytes = 65;
  pHead->head.sectionNameHash = static_cast<uint8_t>('A' * 50U);
  for (size_t i = 0; i < 50; i++)
    buffer[sizeof(SectionHeadBlock_t) + i] = 'A';
  buffer[sizeof(SectionHeadBlock_t) + 50] = 0x00;
  CopyBuffer();

  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  ASSERT_EQ(2U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionHead_NextBlockInvalid)
{
  eeprom.writeAccessCnt = 0;

  SectionHeadBlock_t * const pHead = reinterpret_cast<SectionHeadBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  pHead->head.nextBlock = NOBLOCK;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionHead_NoNullTerminator)
{
  eeprom.writeAccessCnt = 0;

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  buffer[sizeof(testBlock_SectionHead) - 3U] = 'X';
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionHead_MultipleNullTerminator)
{
  eeprom.writeAccessCnt = 0;

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionHead, sizeof(testBlock_SectionHead));
  buffer[sizeof(testBlock_SectionHead) - 4U] = 0;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionData_OK)
{
  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));

  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  memset(buffer, 0, sizeof(buffer));
  ASSERT_NO_THROW(uut.LoadBlock(0, buffer, sizeof(buffer)));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionData_nBytes)
{
  eeprom.writeAccessCnt = 0;

  DataBlock_t * const pHead = reinterpret_cast<DataBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->head.nBytes = 13;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->head.nBytes = 14;
  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(1U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->head.nBytes = 64;
  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(2U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->head.nBytes = 65;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBuffer());

  ASSERT_EQ(2U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionData_SectionNameHashInvalid)
{
  eeprom.writeAccessCnt = 0;

  DataBlock_t * const pHead = reinterpret_cast<DataBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->head.sectionNameHash = 1;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_SectionData_SeqNb)
{
  eeprom.writeAccessCnt = 0;

  DataBlock_t * const pHead = reinterpret_cast<DataBlock_t *>(buffer);

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->seqNb = uut.GetnBlocks() - 1U;
  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_SectionData, sizeof(testBlock_SectionData));
  pHead->seqNb = uut.GetnBlocks() - 2U;
  CopyBuffer();
  ASSERT_NO_THROW(uut.StoreBlock(0, buffer, nullptr, true));
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(1U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_InvalidType)
{
  eeprom.writeAccessCnt = 0;

  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_InvalidType, sizeof(testBlock_InvalidType));

  CopyBuffer();
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::logic_error);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRC());

  ASSERT_EQ(0U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_WriteAndCheckThrows)
{
  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));

  CopyBuffer();
  eeprom.writeAccessesTillThrow = 1;
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), std::exception);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(1U, eeprom.writeAccessCnt);
}
TEST_F(GPCC_FileSystems_EEPROMSectionSystem_internal_BlockAccessor_TestFixture, StoreBlock_WriteAndCheckFails)
{
  eeprom.Invalidate(0, eeprom.GetSize());

  memset(buffer, 0, sizeof(buffer));
  memcpy(buffer, testBlock_FreeBlock, sizeof(testBlock_FreeBlock));

  CopyBuffer();
  eeprom.writeAndCheckAccessTillFailure = 1;
  ASSERT_THROW(uut.StoreBlock(0, buffer, nullptr, true), VolatileStorageError);
  ASSERT_TRUE(VerifyNoChangeInBufferExceptCRCAndTNOW());

  ASSERT_EQ(1U, eeprom.writeAccessCnt);
}

} // namespace internal
} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
