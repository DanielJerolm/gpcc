/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROMUNDO_HPP_
#define SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROMUNDO_HPP_

#include <memory>
#include <cstddef>
#include <cstdint>

namespace gpcc_tests
{
namespace file_systems
{
namespace eeprom_section_system
{

// Class encapsulating a chunk of data from an FakeEEPROM instance.
// Class FakeEEPROM creates instances of this to keep backup copies of memory areas
// before those memory areas are overwritten. The backups are used by class FakeEEPROM
// to implement an undo-functionality.
class FakeEEPROMUndo
{
  public:
    FakeEEPROMUndo(uint32_t const _startAddress, size_t const _size, void const * pData);
    FakeEEPROMUndo(FakeEEPROMUndo const & other);
    FakeEEPROMUndo(FakeEEPROMUndo && other);

    FakeEEPROMUndo& operator=(FakeEEPROMUndo const & other);
    FakeEEPROMUndo& operator=(FakeEEPROMUndo && other);

    void Revert(void* const pMem) const;

  private:
    uint32_t startAddress;
    size_t size;
    std::unique_ptr<uint8_t[]> spMem;
};

} // namespace file_systems
} // namespace eeprom_section_system
} // namespace gpcc_tests

#endif // SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROMUNDO_HPP_
