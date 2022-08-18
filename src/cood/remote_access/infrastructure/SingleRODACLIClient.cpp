/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "SingleRODACLIClient.hpp"

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
 *
 * \post  The client is connected to the RODA interface.
 *
 * \post  The client has registered the CLI command.
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
 * \param _rodaItf
 * Reference to the @ref IRemoteObjectDictionaryAccess interface this client shall connect to.
 *
 * \param _cli
 * [CLI](@ref gpcc::cli::CLI) instance where this client shall register the CLI command.
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
SingleRODACLIClient::SingleRODACLIClient(IRemoteObjectDictionaryAccess & _rodaItf,
                                         gpcc::cli::CLI & _cli,
                                         std::string const & _cmdName,
                                         bool const _ethercatStyleNotCanOpenStyle)
: SingleRODACLIClientBase(_rodaItf, _cli, _cmdName, _ethercatStyleNotCanOpenStyle ? 18U : 5U)
, ethercatStyleNotCanOpenStyle(_ethercatStyleNotCanOpenStyle)
{
}

/**
 * \fn SingleRODACLIClient::~SingleRODACLIClient
 * \brief Destructor.
 *
 * \post  The CLI command is unregistered.
 *
 * \post  The client is disconnected from the RODA interface.
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
std::string SingleRODACLIClient::AttributesToStringHook(Object::attr_t const attributes)
{
  if (ethercatStyleNotCanOpenStyle)
    return Object::AttributeToString(attributes, true);
  else
    return Object::AttributeToString(attributes, false);
}

} // namespace cood
} // namespace gpcc
