/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019, 2022 Daniel Jerolm

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

#include "CLIAdapterForCANOpen.hpp"

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param _od
 * @ref IObjectAccess interface of the object dictionary that shall be accessed by the CLI commands.
 *
 * \param _cli
 * Reference to the CLI instance where the CLI commands shall be registered.
 *
 * \param _cmdName
 * Name for the CLI command.\n
 * Sub-commands will be realized via arguments passed to the command.\n
 * The string must meet the requirements of [cli::Command::Create(...)](@ref gpcc::cli::Command::Create).
 */
CLIAdapterForCANOpen::CLIAdapterForCANOpen(IObjectAccess & _od,
                                           gpcc::cli::CLI & _cli,
                                           std::string const & _cmdName)
: CLIAdapterBase(_od, _cli, _cmdName, 5U)
{
  RegisterCLICommand();
}

/**
 * \brief Destructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
CLIAdapterForCANOpen::~CLIAdapterForCANOpen(void)
{
  UnregisterCLICommand();
}

/// \copydoc CLIAdapterBase::BeginAccessHook
Object::attr_t CLIAdapterForCANOpen::BeginAccessHook(void)
{
  return Object::attr_ACCESS_RW;
}

/// \copydoc CLIAdapterBase::EndAccessHook
void CLIAdapterForCANOpen::EndAccessHook(void) noexcept
{
  // intentionally empty
}

/// \copydoc CLIAdapterBase::AttributesToStringHook
std::string CLIAdapterForCANOpen::AttributesToStringHook(Object::attr_t const attributes)
{
  std::string s = Object::AttributeToString(attributes, false);

  while (s.size() < 5U)
  {
    s += ' ';
  }

  return s;
}

} // namespace cood
} // namespace gpcc
