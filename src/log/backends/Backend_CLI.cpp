/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "Backend_CLI.hpp"
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/CLIColors.hpp>

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
