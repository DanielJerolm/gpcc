/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "FakeEEPROM.hpp"
#include <stdexcept>
#include <cstring>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

FakeEEPROM::FakeEEPROM(size_t const _size, size_t const _pageSize)
: gpcc::stdif::IRandomAccessStorage()
, writeAccessCnt(0)
, readAccessCnt(0)
, writeAccessesTillThrow(0)
, writeAndCheckAccessTillFailure(0)
, readAccessesTillThrow(0)
, size(_size)
, pageSize(_pageSize)
, spMem()
, enableUndo(false)
, undoList()
{
  if ((pageSize != 0) && (size % pageSize != 0))
    throw std::invalid_argument("FakeEEPROM::FakeEEPROM: _pageSize does not divide _size without remainder");

  spMem.reset(new uint8_t[size]);
  memset(spMem.get(), 0x00, size);
}
FakeEEPROM::FakeEEPROM(FakeEEPROM const & other)
: gpcc::stdif::IRandomAccessStorage(other)
, writeAccessCnt(other.writeAccessCnt)
, readAccessCnt(other.readAccessCnt)
, writeAccessesTillThrow(other.writeAccessesTillThrow)
, writeAndCheckAccessTillFailure(other.writeAndCheckAccessTillFailure)
, readAccessesTillThrow(other.readAccessesTillThrow)
, size(other.size)
, pageSize(other.pageSize)
, spMem(new uint8_t[other.size])
, enableUndo(other.enableUndo)
, undoList(other.undoList)
{
  memcpy(spMem.get(), other.spMem.get(), size);
}
FakeEEPROM::FakeEEPROM(FakeEEPROM && other)
: gpcc::stdif::IRandomAccessStorage(std::move(other))
, writeAccessCnt(other.writeAccessCnt)
, readAccessCnt(other.readAccessCnt)
, writeAccessesTillThrow(other.writeAccessesTillThrow)
, writeAndCheckAccessTillFailure(other.writeAndCheckAccessTillFailure)
, readAccessesTillThrow(other.readAccessesTillThrow)
, size(other.size)
, pageSize(other.pageSize)
, spMem(std::move(other.spMem))
, enableUndo(other.enableUndo)
, undoList(std::move(other.undoList))
{
  other.size = 0;
}

FakeEEPROM& FakeEEPROM::operator=(FakeEEPROM const & other)
{
  if (this != &other)
  {
    if (size != other.size)
    {
      spMem.reset(new uint8_t[other.size]);
      size = other.size;
    }
    writeAccessCnt = other.writeAccessCnt;
    readAccessCnt = other.readAccessCnt;
    writeAccessesTillThrow = other.writeAccessesTillThrow;
    writeAndCheckAccessTillFailure = other.writeAndCheckAccessTillFailure;
    readAccessesTillThrow = other.readAccessesTillThrow;
    pageSize = other.pageSize;
    memcpy(spMem.get(), other.spMem.get(), size);
    enableUndo = other.enableUndo;
    undoList = other.undoList;

    gpcc::stdif::IRandomAccessStorage::operator =(other);
  }

  return *this;
}
FakeEEPROM& FakeEEPROM::operator=(FakeEEPROM && other)
{
  if (this != &other)
  {
    gpcc::stdif::IRandomAccessStorage::operator =(std::move(other));

    writeAccessCnt = other.writeAccessCnt;
    readAccessCnt = other.readAccessCnt;
    writeAccessesTillThrow = other.writeAccessesTillThrow;
    writeAndCheckAccessTillFailure = other.writeAndCheckAccessTillFailure;
    readAccessesTillThrow = other.readAccessesTillThrow;
    size = other.size;
    pageSize = other.pageSize;
    spMem = std::move(other.spMem);
    enableUndo = other.enableUndo;
    undoList = std::move(other.undoList);

    other.size = 0;
  }

  return *this;
}

// --> gpcc::stdif::IRandomAccessStorage
size_t FakeEEPROM::GetSize(void) const
{
  return size;
}
size_t FakeEEPROM::GetPageSize(void) const
{
  return pageSize;
}

void FakeEEPROM::Read(uint32_t address, size_t n, void* pBuffer) const
{
  readAccessCnt++;

  if (readAccessesTillThrow != 0)
  {
    readAccessesTillThrow--;
    if (readAccessesTillThrow == 0)
      throw std::runtime_error("FakeEEPROM::Read: Stimulated exception");
  }

  CheckAccessBounds(address, n);
  memcpy(pBuffer, spMem.get() + address, n);
}
void FakeEEPROM::Write(uint32_t address, size_t n, void const * pBuffer)
{
  writeAccessCnt++;

  if (writeAccessesTillThrow != 0)
  {
    writeAccessesTillThrow--;
    if (writeAccessesTillThrow == 0)
      throw std::runtime_error("FakeEEPROM::Write: Stimulated exception");
  }

  CheckAccessBounds(address, n);

  void * const pDest = spMem.get() + address;
  if (enableUndo)
    undoList.push_back(FakeEEPROMUndo(address, n, pDest));
  memcpy(pDest, pBuffer, n);
}
bool FakeEEPROM::WriteAndCheck(uint32_t address, size_t n, void const * pBuffer, void* pAuxBuffer)
{
  (void)pAuxBuffer;

  if (writeAccessesTillThrow != 1)
  {
    if (writeAndCheckAccessTillFailure != 0)
    {
      writeAndCheckAccessTillFailure--;
      if (writeAndCheckAccessTillFailure == 0)
      {
        writeAccessCnt++;
        return false;
      }
    }
  }

  Write(address, n, pBuffer);

  return true;
}
// <-- gpcc::stdif::IRandomAccessStorage

void FakeEEPROM::SetEnableUndo(bool const onOff)
{
  enableUndo = onOff;
  if (!enableUndo)
    undoList.clear();
}
void FakeEEPROM::ClearUndo(void)
{
  undoList.clear();
}
void FakeEEPROM::Undo(size_t n)
{
  if (!enableUndo)
    throw std::logic_error("FakeEEPROM::Undo: Undo disabled");

  if (n > undoList.size())
    throw std::invalid_argument("FakeEEPROM::Undo: n too large");

  void* const pMem = spMem.get();
  while (n-- != 0)
  {
    undoList.back().Revert(pMem);
    undoList.pop_back();
  }
}

void FakeEEPROM::Invalidate(uint32_t const address, size_t n)
{
  CheckAccessBounds(address, n);

  uint8_t* pData = spMem.get() + address;

  while (n-- != 0)
  {
    *pData ^= 0xAA;
    pData++;
  }
}

void FakeEEPROM::CheckAccessBounds(uint32_t const startAdress, size_t const n) const
{
  if ((n > size) ||
      (startAdress > size - n))
    throw std::invalid_argument("FakeEEPROM::CheckAccessBounds");
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
