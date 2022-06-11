/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#include "MultiRODACLIClient.hpp"

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
 *
 * After construction, use interface @ref IMultiRODACLIClient to register RODA interfaces.
 *
 * \post  The CLI command has been registered.
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
 * [CLI](@ref gpcc::cli::CLI) instance where the CLI command shall be registered.
 *
 * \param _cmdName
 * Name for the CLI command.\n
 * Sub-commands will be realized via arguments passed to the command.\n
 * An empty string is not allowed.
 *
 * \param _ethercatStyleNotCanOpenStyle
 * Desired style for displaying object attributes:\n
 * true = EtherCAT\n
 * false = CANopen
 */
MultiRODACLIClient::MultiRODACLIClient(gpcc::cli::CLI & _cli,
                                       std::string const & _cmdName,
                                       bool const _ethercatStyleNotCanOpenStyle)
: MultiRODACLIClientBase(_cli, _cmdName, _ethercatStyleNotCanOpenStyle ? 18U : 5U)
, ethercatStyleNotCanOpenStyle(_ethercatStyleNotCanOpenStyle)
{
}

/**
 * \fn MultiRODACLIClient::~MultiRODACLIClient
 * \brief Destructor.
 *
 * \pre   There is no RODA interface registered.
 *
 * \post  The CLI command is unregistered.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */

/// \copydoc gpcc::cood::RODACLIClientBase::AttributesToStringHook
std::string MultiRODACLIClient::AttributesToStringHook(Object::attr_t const attributes)
{
  if (ethercatStyleNotCanOpenStyle)
    return Object::AttributeToString(attributes, true);
  else
    return Object::AttributeToString(attributes, false);
}

} // namespace cood
} // namespace gpcc
