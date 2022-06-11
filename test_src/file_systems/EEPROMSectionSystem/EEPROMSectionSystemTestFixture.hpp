/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "FakeEEPROM.hpp"
#include "gpcc/src/file_systems/EEPROMSectionSystem/EEPROMSectionSystem.hpp"
#include "gtest/gtest.h"
#include <cstdint>
#include <cstddef>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

// Test fixture for EEPROMSectionSystem related tests.
// Provides storage, the an EEPROMSectionSystem instance, and a buffer that can be used by the test cases.
class EEPROMSectionSystemTestFixture: public testing::Test
{
  public:
    static const size_t storageSize     = 16*1024;
    static const size_t storagePageSize = 128;
    uint16_t blockSize;
    uint16_t bytesPerBlock;

    EEPROMSectionSystemTestFixture(void);
    virtual ~EEPROMSectionSystemTestFixture(void);

  protected:
    FakeEEPROM fakeStorage;
    gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem uut;
    uint8_t* pBuffer;

    void SetUp(void) override;
    void TearDown(void) override;

    void Format(uint16_t const _blockSize);
    void InvalidateCRC(uint16_t const blockIdx);
    void UpdateCRC(uint16_t const blockIdx);
    void UpdateNextBlock(uint16_t const blockIdx, uint16_t const newNextBlock);
};

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests

