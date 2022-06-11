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

#ifndef SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_RANDOMDATA_HPP_
#define SRC_TESTS_FILESYSTEMS_EEPROMSECTIONSYSTEM_RANDOMDATA_HPP_

#include <vector>
#include <cstdint>
#include <cstddef>

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
