/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
    Copyright (C) 2017 Falk Werner
*/

#include "EEPROMSectionSystemTestFixture.hpp"
#include "gpcc/src/file_systems/eeprom_section_system/internal/EEPROMSectionSystemInternals.hpp"
#include <gpcc/crc/simple_crc.hpp>
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

EEPROMSectionSystemTestFixture::EEPROMSectionSystemTestFixture(void)
: Test()
, blockSize(0)
, bytesPerBlock(0)
, fakeStorage(storageSize, storagePageSize)
, uut(fakeStorage, 0, storageSize)
, pBuffer(nullptr)
{
}
EEPROMSectionSystemTestFixture::~EEPROMSectionSystemTestFixture(void)
{
  if (pBuffer != nullptr)
  {
    delete [] pBuffer;
    pBuffer = nullptr;
  }
}

void EEPROMSectionSystemTestFixture::SetUp(void)
{
}
void EEPROMSectionSystemTestFixture::TearDown(void)
{
  // unmount, if uut's state is not not_mounted
  EEPROMSectionSystem::EEPROMSectionSystem::States const state = uut.GetState();
  EXPECT_EQ(EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted, state);
  if (state != EEPROMSectionSystem::EEPROMSectionSystem::States::not_mounted)
    uut.Unmount();
}

void EEPROMSectionSystemTestFixture::Format(uint16_t const _blockSize)
{
  // Formats the UUT and allocated/reallocates pBuffer.

  std::unique_ptr<uint8_t[]> spNewBuffer(new uint8_t[_blockSize]);

  uut.Format(_blockSize);
  blockSize = _blockSize;
  bytesPerBlock = blockSize - (sizeof(DataBlock_t) + sizeof(uint16_t));

  if (pBuffer != nullptr)
  {
    delete [] pBuffer;
    pBuffer = nullptr;
  }
  pBuffer = spNewBuffer.release();
}
void EEPROMSectionSystemTestFixture::InvalidateCRC(uint16_t const blockIdx)
{
  // Invalidates the CRC of an storage block inside "fakeStorage".

  // calculate block start address
  size_t const bsa = blockIdx * blockSize;

  // retrieve field "nBytes"
  fakeStorage.Read(bsa + offsetof(CommonBlockHead_t, nBytes), 2U, pBuffer);
  uint16_t const nBytes = pBuffer[0] + (static_cast<uint16_t>(pBuffer[1]) << 8U);
  if ((nBytes < 2U) || (nBytes > blockSize - 2U))
    throw std::runtime_error("EEPROMSectionSystemTestFixture::InvalidateCRC: Bad \"nBytes\"");

  // load CRC, negate it, and write it back
  fakeStorage.Read(bsa + (nBytes - 2U), 2U, pBuffer);
  pBuffer[0] = ~pBuffer[0];
  pBuffer[1] = ~pBuffer[1];
  fakeStorage.Write(bsa + (nBytes - 2U), 2U, pBuffer);
}
void EEPROMSectionSystemTestFixture::UpdateCRC(uint16_t const blockIdx)
{
  // Updates the CRC of an storage block inside "fakeStorage".

  // calculate block start address
  size_t const bsa = blockIdx * blockSize;

  // retrieve field "nBytes"
  fakeStorage.Read(bsa + offsetof(CommonBlockHead_t, nBytes), 2U, pBuffer);
  uint16_t const nBytes = pBuffer[0] + (static_cast<uint16_t>(pBuffer[1]) << 8U);
  if ((nBytes < 2U) || (nBytes > blockSize - 2U))
    throw std::runtime_error("EEPROMSectionSystemTestFixture::UpdateCRC: Bad \"nBytes\"");

  // load block
  fakeStorage.Read(bsa, nBytes - 2U, pBuffer);

  // calculate new CRC
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, pBuffer, nBytes - 2U, gpcc::crc::crc16_ccitt_table_normal);

  // update CRC
  pBuffer[0] = crc & 0xFFU;
  pBuffer[1] = crc >> 8U;
  fakeStorage.Write(bsa + (nBytes - 2U), 2U, pBuffer);
}
void EEPROMSectionSystemTestFixture::UpdateNextBlock(uint16_t const blockIdx, uint16_t const newNextBlock)
{
  // Updates the nextBlock-attribute of an storage block and updates the block's CRC

  fakeStorage.Read(blockSize * blockIdx + offsetof(CommonBlockHead_t, nextBlock), 2, pBuffer);
  pBuffer[0] = newNextBlock & 0xFFU;
  pBuffer[1] = newNextBlock >> 8U;
  fakeStorage.Write(blockSize * blockIdx + offsetof(CommonBlockHead_t, nextBlock), 2, pBuffer);
  UpdateCRC(blockIdx);
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
