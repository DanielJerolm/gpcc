/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "MultiRODACLIClientBase.hpp"
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/Command.hpp>
#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/string/tools.hpp"
#include <functional>
#include <stdexcept>

namespace gpcc {
namespace cood {

uint16_t constexpr MultiRODACLIClientBase::rodaReadyTimeout_ms;

// <-- IMultiRODACLIClient

/// \copydoc gpcc::cood::IMultiRODACLIClient::Register
void MultiRODACLIClientBase::Register(IRemoteObjectDictionaryAccess & rodaItf, uint32_t const id)
{
  gpcc::osal::MutexLocker rodaItfMutexLocker(rodaItfMutex);

  if (registeredRODAItfs.find(id) != registeredRODAItfs.end())
    throw std::logic_error("MultiRODACLIClientBase::Register: Given 'id' is already in use.");

  registeredRODAItfs.insert(std::make_pair(id, &rodaItf));
}

/// \copydoc gpcc::cood::IMultiRODACLIClient::Unregister
void MultiRODACLIClientBase::Unregister(uint32_t const id)
{
  gpcc::osal::MutexLocker rodaItfMutexLocker(rodaItfMutex);

  auto it = registeredRODAItfs.find(id);

  if (it != registeredRODAItfs.end())
  {
    // disconnect, if we are currently connected to the RODA interface that shall be unregistered
    if (GetCurrentlyConnectedRODAItf() == it->second)
      Disconnect();

    registeredRODAItfs.erase(it);
  }
}

// --> IMultiRODACLIClient

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
 * \param _attributeStringMaxLength
 * Maximum number of bytes returned by the implementation of @ref RODACLIClientBase::AttributesToStringHook().
 */
MultiRODACLIClientBase::MultiRODACLIClientBase(gpcc::cli::CLI & _cli,
                                               std::string const & _cmdName,
                                               uint8_t const _attributeStringMaxLength)
: IMultiRODACLIClient()
, RODACLIClientBase(_cli, _attributeStringMaxLength)
, cmdName(_cmdName)
, rodaItfMutex()
, registeredRODAItfs()
{
  using gpcc::cli::Command;

  cli.AddCommand(Command::Create(cmdName.c_str(), " rodaID subcmd [args...]\n"
                                 "Accesses the remote object dictionary referenced by <rodaID>. The type of\n"
                                 "access is specified by <subcmd>:\n"
                                 "- enum [0xFROM-0xTO]\n"
                                 "  Enumerates objects contained in the object dictionary.\n"
                                 "  Options:\n"
                                 "    FROM   Index where enumeration shall start. Default: 0x0000\n"
                                 "    TO     Index where enumeration shall end. Default: 0xFFFF\n"
                                 "    FROM <= TO must be valid.\n"
                                 "\n"
                                 "- info 0xINDEX [asm]\n"
                                 "  Prints the meta data of an object and its subindices.\n"
                                 "  Options:\n"
                                 "    asm   Includes application-specific meta data in the output.\n"
                                 "\n"
                                 "- read 0xINDEX:Subindex\n"
                                 "  Reads the data of a subindex and prints it to CLI.\n"
                                 "  <subindex> shall be provided in decimal format.\n"
                                 "\n"
                                 "- write 0xINDEX:Subindex DATA\n"
                                 "  Writes <DATA> to a subindex. <Subindex> shall be provided in decimal format.\n"
                                 "\n"
                                 "  The format of <DATA> must meet the data type of the subindex:\n"
                                 "  For BOOLEAN: TRUE, FALSE, true, false\n"
                                 "  For REAL32/64: [+|-]digits[.][digits][(e|E)[+|-]digits]\n"
                                 "  For VISIBLE_STRING: \"Text...\"\n"
                                 "  For OCTET_STRING: 5B A3 ... (8bit hex values, separated by spaces)\n"
                                 "  For UNICODE_STRING: 5B33 A6CF (16bit hex values, separated by spaces)\n"
                                 "  For BIT1..BIT8: 0, 1, 3, 0x3, 0b11 (unused upper bits must be zero)\n"
                                 "\n"
                                 "- caread 0xINDEX [v]\n"
                                 "  Reads the whole object via complete access and prints the value of each\n"
                                 "  subindex to CLI.\n"
                                 "  Options:\n"
                                 "  v   Verbose output. Prints the data type and name of each subindex in addition\n"
                                 "      to the data.\n"
                                 "- cawrite 0xINDEX\n"
                                 "  Writes the whole object via complete access.\n"
                                 "  The data that shall be written is entered using an interactive dialog.",
                                 std::bind(&MultiRODACLIClientBase::CLICommandHandler, this,
                                           std::placeholders::_1, std::placeholders::_2)));
}

/**
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
MultiRODACLIClientBase::~MultiRODACLIClientBase(void)
{
  try
  {
    cli.RemoveCommand(cmdName.c_str());

    gpcc::osal::MutexLocker rodaItfMutexLocker(rodaItfMutex);
    if (!registeredRODAItfs.empty())
      gpcc::osal::Panic("MultiRODACLIClientBase::~MultiRODACLIClientBase: At least one interface still registered.");
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief CLI command handler.
 *
 * This is executed when the user enters the CLI command registered by this class.
 *
 * This will pick up the first argument (RODA ID) and the second argument (command) and delegate the call to a
 * specialized CLI handler provided by the base class.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This is intended to be executed in the context of @ref cli only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 * - Base class may have been connected to a different RODA interface.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 * - Base class may have been connected to a different RODA interface.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 *
 * \param cli
 * [CLI](@ref gpcc::cli::CLI) instance invoking this method.
 */
void MultiRODACLIClientBase::CLICommandHandler(std::string const & restOfLine, gpcc::cli::CLI & cli)
{
  (void)cli;

  if (restOfLine.empty())
    throw gpcc::cli::UserEnteredInvalidArgsError();

  // extract RODA ID from arguments and copy the remaining arguments into "rest"
  auto pos = restOfLine.find(' ');
  if (pos == std::string::npos)
    throw gpcc::cli::UserEnteredInvalidArgsError();

  uint32_t const rodaID = gpcc::string::AnyStringToU32(restOfLine.substr(0U, pos));

  auto rest = restOfLine.substr(pos + 1U);

  // extract the command from "rest" and copy the remaining arguments into "args"
  pos = rest.find(' ');
  auto const command = rest.substr(0U, pos);

  if (pos == std::string::npos)
    pos = rest.size() - 1U;
  auto const args = rest.substr(pos + 1U);


  gpcc::osal::MutexLocker rodaItfMutexLocker(rodaItfMutex);


  // lambda: checks "rodaID" and connects base class to the RODA interface referenced by "rodaID", if it is not yet
  // connected.
  auto EnsureConnected = [&]()
  {
    // query map entry
    auto it = registeredRODAItfs.find(rodaID);
    if (it == registeredRODAItfs.end())
      throw std::runtime_error("Given RODA interface ID is unknown.");

    // figure out current and desired RODA interface
    auto pDesired = it->second;
    auto pCurrent = GetCurrentlyConnectedRODAItf();

    // already connected to desired RODA interface?
    if (pDesired == pCurrent)
      return;

    // disconnected from RODA interface if connected to any
    if (pCurrent != nullptr)
      Disconnect();

    // connect to desired RODA interface and wait until RODA enters ready state
    Connect(*pDesired);

    if (!WaitForRODAItfReady(rodaReadyTimeout_ms))
      throw std::runtime_error("Timeout. RODA interface is not ready.");
  };

  if (command == "enum")
  {
    EnsureConnected();
    CLI_Enumerate(args);
  }
  else if (command == "info")
  {
    EnsureConnected();
    CLI_Info(args);
  }
  else if (command == "read")
  {
    EnsureConnected();
    CLI_Read(args);
  }
  else if (command == "write")
  {
    EnsureConnected();
    CLI_Write(args);
  }
  else if (command == "caread")
  {
    EnsureConnected();
    CLI_CARead(args);
  }
  else if (command == "cawrite")
  {
    EnsureConnected();
    CLI_CAWrite(args);
  }
  else
  {
    throw gpcc::cli::UserEnteredInvalidArgsError("Invalid/unknown sub command!");
  }
}

} // namespace cood
} // namespace gpcc
