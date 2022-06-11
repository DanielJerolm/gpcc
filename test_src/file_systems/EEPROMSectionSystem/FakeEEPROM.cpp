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
#include <stdexcept>
#include <cstring>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

FakeEEPROM::FakeEEPROM(size_t const _size, size_t const _pageSize)
: gpcc::StdIf::IRandomAccessStorage()
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
: gpcc::StdIf::IRandomAccessStorage(other)
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
: gpcc::StdIf::IRandomAccessStorage(std::move(other))
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

    gpcc::StdIf::IRandomAccessStorage::operator =(other);
  }

  return *this;
}
FakeEEPROM& FakeEEPROM::operator=(FakeEEPROM && other)
{
  if (this != &other)
  {
    gpcc::StdIf::IRandomAccessStorage::operator =(std::move(other));

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

// --> gpcc::StdIf::IRandomAccessStorage
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
// <-- gpcc::StdIf::IRandomAccessStorage

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
