/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
    Copyright (C) 2017 Falk Werner
*/

#include "EEPROMSectionSystemTestFixture.hpp"
#include <gpcc/crc/simple_crc.hpp>
#include "src/file_systems/eeprom_section_system/internal/EEPROMSectionSystemInternals.hpp"
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstdlib>

namespace gpcc_tests
{
namespace file_systems
{
namespace eeprom_section_system
{

using namespace testing;
using namespace gpcc::file_systems::eeprom_section_system;
using namespace gpcc::file_systems::eeprom_section_system::internal;

EEPROMSectionSystemTestFixture::EEPROMSectionSystemTestFixture(void)
: Test()
, blockSize_(0)
, bytesPerBlock_(0)
, fakeStorage_(storageSize_, storagePageSize_)
, uut_(fakeStorage_, 0, storageSize_)
, pBuffer_(nullptr)
{
}
EEPROMSectionSystemTestFixture::~EEPROMSectionSystemTestFixture(void)
{
  if (pBuffer_ != nullptr)
  {
    delete [] pBuffer_;
    pBuffer_ = nullptr;
  }
}

void EEPROMSectionSystemTestFixture::SetUp(void)
{
}
void EEPROMSectionSystemTestFixture::TearDown(void)
{
  // unmount, if uut's state is not not_mounted
  eeprom_section_system::EEPROMSectionSystem::States const state = uut_.GetState();
  EXPECT_EQ(eeprom_section_system::EEPROMSectionSystem::States::not_mounted, state);
  if (state != eeprom_section_system::EEPROMSectionSystem::States::not_mounted)
    uut_.Unmount();
}

void EEPROMSectionSystemTestFixture::Format(uint16_t const blockSize)
{
  // Formats the UUT and allocates/reallocates pBuffer_.

  std::unique_ptr<uint8_t[]> spNewBuffer(new uint8_t[blockSize]);

  uut_.Format(blockSize);
  blockSize_ = blockSize;
  bytesPerBlock_ = blockSize_ - (sizeof(DataBlock_t) + sizeof(uint16_t));

  if (pBuffer_ != nullptr)
  {
    delete [] pBuffer_;
    pBuffer_ = nullptr;
  }
  pBuffer_ = spNewBuffer.release();
}
void EEPROMSectionSystemTestFixture::InvalidateCRC(uint16_t const blockIdx)
{
  // Invalidates the CRC of an storage block inside "fakeStorage_".

  // calculate block start address
  size_t const bsa = blockIdx * blockSize_;

  // retrieve field "nBytes"
  fakeStorage_.Read(bsa + offsetof(CommonBlockHead_t, nBytes), 2U, pBuffer_);
  uint16_t const nBytes = pBuffer_[0] + (static_cast<uint16_t>(pBuffer_[1]) << 8U);
  if ((nBytes < 2U) || (nBytes > blockSize_ - 2U))
    throw std::runtime_error("EEPROMSectionSystemTestFixture::InvalidateCRC: Bad \"nBytes\"");

  // load CRC, negate it, and write it back
  fakeStorage_.Read(bsa + (nBytes - 2U), 2U, pBuffer_);
  pBuffer_[0] = ~pBuffer_[0];
  pBuffer_[1] = ~pBuffer_[1];
  fakeStorage_.Write(bsa + (nBytes - 2U), 2U, pBuffer_);
}
void EEPROMSectionSystemTestFixture::UpdateCRC(uint16_t const blockIdx)
{
  // Updates the CRC of an storage block inside "fakeStorage_".

  // calculate block start address
  size_t const bsa = blockIdx * blockSize_;

  // retrieve field "nBytes"
  fakeStorage_.Read(bsa + offsetof(CommonBlockHead_t, nBytes), 2U, pBuffer_);
  uint16_t const nBytes = pBuffer_[0] + (static_cast<uint16_t>(pBuffer_[1]) << 8U);
  if ((nBytes < 2U) || (nBytes > blockSize_ - 2U))
    throw std::runtime_error("EEPROMSectionSystemTestFixture::UpdateCRC: Bad \"nBytes\"");

  // load block
  fakeStorage_.Read(bsa, nBytes - 2U, pBuffer_);

  // calculate new CRC
  uint16_t crc = 0xFFFFU;
  gpcc::crc::CalcCRC16_normal_noInputReverse(crc, pBuffer_, nBytes - 2U, gpcc::crc::crc16_ccitt_table_normal);

  // update CRC
  pBuffer_[0] = crc & 0xFFU;
  pBuffer_[1] = crc >> 8U;
  fakeStorage_.Write(bsa + (nBytes - 2U), 2U, pBuffer_);
}
void EEPROMSectionSystemTestFixture::UpdateNextBlock(uint16_t const blockIdx, uint16_t const newNextBlock)
{
  // Updates the nextBlock-attribute of an storage block and updates the block's CRC

  fakeStorage_.Read(blockSize_ * blockIdx + offsetof(CommonBlockHead_t, nextBlock), 2, pBuffer_);
  pBuffer_[0] = newNextBlock & 0xFFU;
  pBuffer_[1] = newNextBlock >> 8U;
  fakeStorage_.Write(blockSize_ * blockIdx + offsetof(CommonBlockHead_t, nextBlock), 2, pBuffer_);
  UpdateCRC(blockIdx);
}

} // namespace file_systems
} // namespace eeprom_section_system
} // namespace gpcc_tests
