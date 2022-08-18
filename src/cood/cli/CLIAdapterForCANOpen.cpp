/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
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
