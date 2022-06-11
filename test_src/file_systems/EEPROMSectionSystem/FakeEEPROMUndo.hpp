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

#ifndef SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROMUNDO_HPP_
#define SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROMUNDO_HPP_

#include <memory>
#include <cstdint>
#include <cstddef>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
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
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests

#endif // SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROMUNDO_HPP_
