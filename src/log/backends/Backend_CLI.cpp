/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019 Daniel Jerolm

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

#include "Backend_CLI.hpp"
#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/cli/CLIColors.hpp"

namespace gpcc {
namespace log  {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _cli
 * @ref gpcc::cli::CLI instance to which log messages shall be printed.
 */
Backend_CLI::Backend_CLI(gpcc::cli::CLI & _cli)
: Backend()
, cli(_cli)
{
}

/// \copydoc Backend::Process
void Backend_CLI::Process(std::string const & msg, LogType const type)
{
  // We use CLI::WriteLineComposed(...) to prepend CLI font/color/style control information instead of
  // concatenating std::string instances. This safes some heap allocations and CPU time.
  char const * fragments[3];

  switch (type)
  {
    case LogType::Warning:
      fragments[0] = CLI_BOLD_YELLOW;
      fragments[1] = msg.c_str();
      fragments[2] = nullptr;
      break;

    case LogType::Error:
      fragments[0] = CLI_RED;
      fragments[1] = msg.c_str();
      fragments[2] = nullptr;
      break;

    case LogType::Fatal:
      fragments[0] = CLI_BOLD_LIGHT_RED;
      fragments[1] = msg.c_str();
      fragments[2] = nullptr;
      break;

    default:
      fragments[0] = msg.c_str();
      fragments[1] = nullptr;
  }

  cli.WriteLineComposed(fragments);
}

} // namespace log
} // namespace gpcc
