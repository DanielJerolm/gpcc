/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI_ESSCLICOMMANDS_HPP_
#define SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI_ESSCLICOMMANDS_HPP_

#include <string>

namespace gpcc
{
namespace cli
{
  class CLI;
}
namespace file_systems
{
namespace EEPROMSectionSystem
{
class EEPROMSectionSystem;

void CLICMDGetState(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS);
void CLICMDFormat(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS, uint16_t blockSize);
void CLICMDUnmount(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS);
void CLICMDMount(std::string const & restOfLine, gpcc::cli::CLI & cli, EEPROMSectionSystem* pESS);

} // namespace EEPROMSectionSystem
} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_EEPROMSECTIONSYSTEM_CLI_ESSCLICOMMANDS_HPP_
