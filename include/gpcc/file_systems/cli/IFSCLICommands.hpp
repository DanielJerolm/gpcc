/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_FILESYSTEMS_CLI_IFSCLICOMMANDS_HPP_
#define SRC_GPCC_FILESYSTEMS_CLI_IFSCLICOMMANDS_HPP_

#include <string>

namespace gpcc
{
namespace cli
{
  class CLI;
}
namespace file_systems
{

class IFileStorage;

void CLICMDDelete(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS);
void CLICMDRename(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS);
void CLICMDEnumerate(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS);
void CLICMDFreeSpace(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS);
void CLICMDDump(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS);
void CLICMDCopy(std::string const & restOfLine, gpcc::cli::CLI & cli, IFileStorage* pIFS);

} // namespace file_systems
} // namespace gpcc

#endif // SRC_GPCC_FILESYSTEMS_CLI_IFSCLICOMMANDS_HPP_
