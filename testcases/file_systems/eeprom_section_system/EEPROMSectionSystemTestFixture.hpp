/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/file_systems/eeprom_section_system/EEPROMSectionSystem.hpp>
#include "FakeEEPROM.hpp"
#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>

namespace gpcc_tests
{
namespace file_systems
{
namespace eeprom_section_system
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
    gpcc::file_systems::eeprom_section_system::EEPROMSectionSystem uut;
    uint8_t* pBuffer;

    void SetUp(void) override;
    void TearDown(void) override;

    void Format(uint16_t const _blockSize);
    void InvalidateCRC(uint16_t const blockIdx);
    void UpdateCRC(uint16_t const blockIdx);
    void UpdateNextBlock(uint16_t const blockIdx, uint16_t const newNextBlock);
};

} // namespace file_systems
} // namespace eeprom_section_system
} // namespace gpcc_tests

