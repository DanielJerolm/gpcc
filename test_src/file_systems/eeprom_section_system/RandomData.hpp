/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_RANDOMDATA_HPP_
#define SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_RANDOMDATA_HPP_

#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc
{
  namespace file_systems
  {
    namespace EEPROMSectionSystem
    {
      class EEPROMSectionSystem;
    }
  }
}

namespace gpcc_tests
{
namespace file_systems
{
namespace EEPROMSectionSystem
{

// Class encapsulating a chunk of random data that can be written to an EEPROMSectionSystem instance
// during unit-tests and read-back during unit tests.
class RandomData
{
  public:
    RandomData(size_t const minSize, size_t const maxSize);

    bool operator=(RandomData const & other) const;

    void Write(std::string const & name, bool overwriteIfExisting, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem & uut) const;
    void Compare(std::string const & name, gpcc::file_systems::EEPROMSectionSystem::EEPROMSectionSystem & uut) const;

    size_t GetSize(void) const;
    uint8_t const * GetData(void) const;

  private:
    std::vector<uint8_t> data;
};

} // namespace file_systems
} // namespace EEPROMSectionSystem
} // namespace gpcc_tests

#endif // SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_RANDOMDATA_HPP_
