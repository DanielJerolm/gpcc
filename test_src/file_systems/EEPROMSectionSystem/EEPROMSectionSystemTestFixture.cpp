/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm
    Copyright (C) 2017 Falk Werner

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

#include "EEPROMSectionSystemTestFixture.hpp"
#include "gpcc/src/crc/simple_crc.hpp"
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
