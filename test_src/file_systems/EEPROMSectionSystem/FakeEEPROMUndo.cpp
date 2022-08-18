/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "FakeEEPROMUndo.hpp"
#include <cstring>

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
