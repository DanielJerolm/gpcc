/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef COMMANDS_HPP_201601052139
#define COMMANDS_HPP_201601052139

#include <string>

namespace gpcc {

namespace cli
{
  class CLI;
}

namespace file_systems
{
  class IFileStorage;
}

namespace log {

class ILogFacilityCtrl;

void CLI_Cmd_LogCtrl(std::string const & restOfLine,
                     gpcc::cli::CLI & cli,
                     ILogFacilityCtrl* const pLogFacilityCtrl);

void CLI_Cmd_WriteConfigToFile(std::string const & restOfLine,
                               gpcc::cli::CLI & cli,
                               ILogFacilityCtrl* const pLogFacilityCtrl,
                               gpcc::file_systems::IFileStorage* const pFileStorage);
void CLI_Cmd_ReadConfigFromFile(std::string const & restOfLine,
                                gpcc::cli::CLI & cli,
                                ILogFacilityCtrl* const pLogFacilityCtrl,
                                gpcc::file_systems::IFileStorage* const pFileStorage);

void CLI_Cmd_WriteConfigToTextFile(std::string const & restOfLine,
                                   gpcc::cli::CLI & cli,
                                   ILogFacilityCtrl* const pLogFacilityCtrl,
                                   gpcc::file_systems::IFileStorage* const pFileStorage,
                                   char const * pHeadline);
void CLI_Cmd_ReadConfigFromTextFile(std::string const & restOfLine,
                                    gpcc::cli::CLI & cli,
                                    ILogFacilityCtrl* const pLogFacilityCtrl,
                                    gpcc::file_systems::IFileStorage* const pFileStorage);

} // namespace log
} // namespace gpcc

#endif // COMMANDS_HPP_201601052139
