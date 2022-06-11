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

#ifndef SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROM_HPP_
#define SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROM_HPP_

#include "gpcc/src/StdIf/IRandomAccessStorage.hpp"
#include "FakeEEPROMUndo.hpp"
#include <list>
#include <memory>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

// Fake EEPROM, used as storage in EEPROMSectionSystem tests.
// Access via gpcc::StdIf::IRandomAccessStorage.
// There is an undo-functionality that can be enabled and disabled via SetEnableUndo(...).
// If enabled, each write is recorded, and the last X writes can be undone via Undo(X) later.
// If the undo-functionality is disabled, then the undo-history is cleared.
// To simulate EEPROM corruption, Invalidate(...) can be used to change bits by XORing data with 0xAA.
// "writeAccessCnt" and "readAccessCnt" can be used to check the number of read- and write accesses.
// "writeAccessesTillThrow", "writeAndCheckAccessTillFailure", and "readAccessesTillThrow" can be
// used to schedule failure of an specific read- or write-access.
class FakeEEPROM: public gpcc::StdIf::IRandomAccessStorage
{
  public:
    // Counter for write accesses. This counts each attempt to write, even if an exception is thrown.
    size_t writeAccessCnt;
    // Counter for read accesses. This counts each attempt to read, even if an exception is thrown.
    mutable size_t readAccessCnt;

    // Number of write access till Write() or WriteAndCheck() throw. 0 = no, 1 = next, ...
    size_t writeAccessesTillThrow;

    // Number of write-and-check access will WriteAndCheck() returns false. 0 = no, 1 = next, ...
    size_t writeAndCheckAccessTillFailure;

    /// Number of read accesses till Read() throws. 0 = no, 1 = next, ...
    mutable size_t readAccessesTillThrow;

    FakeEEPROM(size_t const _size, size_t const _pageSize);
    FakeEEPROM(FakeEEPROM const & other);
    FakeEEPROM(FakeEEPROM && other);

    FakeEEPROM& operator=(FakeEEPROM const & other);
    FakeEEPROM& operator=(FakeEEPROM && other);


    // --> gpcc::StdIf::IRandomAccessStorage
    size_t GetSize(void) const;
    size_t GetPageSize(void) const;

    void Read(uint32_t address, size_t n, void* pBuffer) const;
    void Write(uint32_t address, size_t n, void const * pBuffer);
    bool WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer);
    // <-- gpcc::StdIf::IRandomAccessStorage

    void SetEnableUndo(bool const onOff);
    void ClearUndo(void);
    void Undo(size_t n);

    void Invalidate(uint32_t const address, size_t n);

  private:
    size_t size;
    size_t pageSize;
    std::unique_ptr<uint8_t[]> spMem;

    bool enableUndo;
    std::list<FakeEEPROMUndo> undoList;

    void CheckAccessBounds(uint32_t const startAdress, size_t const n) const;
};

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests

#endif // SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROM_HPP_
