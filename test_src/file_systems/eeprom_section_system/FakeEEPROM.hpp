/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROM_HPP_
#define SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_FAKEEEPROM_HPP_

#include <gpcc/stdif/storage/IRandomAccessStorage.hpp>
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
