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

#include "gpcc/src/file_systems/EEPROMSectionSystem/EEPROMSectionSystem.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include "RandomData.hpp"
#include <stdexcept>
#include <cstdlib>

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

using gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem;

RandomData::RandomData(size_t const minSize, size_t const maxSize)
: data()
{
  if (maxSize < minSize)
    throw std::invalid_argument("RandomData::RandomData: minSize/maxSize invalid");

  size_t size = minSize + rand() % (maxSize - minSize + 1U);

  data.resize(size);

  uint8_t* pData = data.data();
  while (size != 0)
  {
    *pData++ = rand() % 256U;
    size--;
  }
}

bool RandomData::operator=(RandomData const & other) const
{
  return (data == other.data);
}

void RandomData::Write(std::string const & name, bool overwriteIfExisting, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem & uut) const
{
  auto writer(uut.Create(name, overwriteIfExisting));
  ON_SCOPE_EXIT()
  {
    try
    {
      writer->Close();
    }
    catch (std::exception const &)
    {
    };
  };

  *writer << static_cast<uint64_t>(data.size());
  writer->Write_uint8(data.data(), data.size());

  ON_SCOPE_EXIT_DISMISS();
  writer->Close();
}
void RandomData::Compare(std::string const & name, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem & uut) const
{
  std::vector<uint8_t> readData;

  auto reader(uut.Open(name));
  ON_SCOPE_EXIT()
  {
    try
    {
      reader->Close();
    }
    catch (std::exception const &)
    {
    };
  };

  uint64_t size;
  *reader >> size;
  readData.resize(static_cast<size_t>(size));

  reader->Read_uint8(readData.data(), readData.size());

  if (reader->GetState() != gpcc::Stream::IStreamReader::States::empty)
    throw std::runtime_error("RandomData::Compare: File should be empty, but it is not");

  ON_SCOPE_EXIT_DISMISS();
  reader->Close();

  if (readData != data)
    throw std::runtime_error("RandomData::Compare: Comparison shows mismatch");
}
size_t RandomData::GetSize(void) const
{
  return data.size();
}
uint8_t const * RandomData::GetData(void) const
{
  return data.data();
}

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests
