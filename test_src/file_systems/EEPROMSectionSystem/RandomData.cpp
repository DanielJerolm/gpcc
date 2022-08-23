/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/file_systems/EEPROMSectionSystem/EEPROMSectionSystem.hpp"
#include <gpcc/raii/scope_guard.hpp>
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
