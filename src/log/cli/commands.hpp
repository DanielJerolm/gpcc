/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2020 Daniel Jerolm

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
