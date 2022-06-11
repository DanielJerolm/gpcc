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

#include "FakeEEPROMUndo.hpp"
#include <cstring>
#
namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

FakeEEPROMUndo::FakeEEPROMUndo(uint32_t const _startAddress, size_t const _size, void const * pData)
: startAddress(_startAddress)
, size(_size)
, spMem(new uint8_t[_size])
{
  memcpy(spMem.get(), pData, size);
}
FakeEEPROMUndo::FakeEEPROMUndo(FakeEEPROMUndo const & other)
: startAddress(other.startAddress)
, size(other.size)
, spMem(new uint8_t[other.size])
{
  memcpy(spMem.get(), other.spMem.get(), size);
}
FakeEEPROMUndo::FakeEEPROMUndo(FakeEEPROMUndo && other)
: startAddress(other.startAddress)
, size(other.size)
, spMem(std::move(other.spMem))
{
  other.size = 0;
}

FakeEEPROMUndo& FakeEEPROMUndo::operator=(FakeEEPROMUndo const & other)
{
  if (this != &other)
  {
    if (size != other.size)
    {
      spMem.reset(new uint8_t[other.size]);
      size = other.size;
    }
    startAddress = other.startAddress;
    memcpy(spMem.get(), other.spMem.get(), size);
  }

  return *this;
}
FakeEEPROMUndo& FakeEEPROMUndo::operator=(FakeEEPROMUndo && other)
{
  if (this != &other)
  {
    startAddress = other.startAddress;
    size = other.size;
    spMem = std::move(other.spMem);
    other.size = 0;
  }

  return *this;
}

void FakeEEPROMUndo::Revert(void* const pMem) const
{
  memcpy(static_cast<uint8_t*>(pMem) + startAddress, spMem.get(), size);
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
