/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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
