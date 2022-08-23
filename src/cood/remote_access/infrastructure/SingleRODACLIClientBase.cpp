/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "SingleRODACLIClientBase.hpp"
#include <gpcc/cli/CLI.hpp>
#include <gpcc/cli/Command.hpp>
#include <gpcc/cli/exceptions.hpp>
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <functional>
#include <stdexcept>

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
 * \param _attributeStringMaxLength
 * Maximum number of bytes returned by the implementation of @ref RODACLIClientBase::AttributesToStringHook().
 */
SingleRODACLIClientBase::SingleRODACLIClientBase(IRemoteObjectDictionaryAccess & _rodaItf,
                                                 gpcc::cli::CLI & _cli,
                                                 std::string const & _cmdName,
                                                 uint8_t const _attributeStringMaxLength)
: RODACLIClientBase(_cli, _attributeStringMaxLength)
, rodaItf(_rodaItf)
, cmdName(_cmdName)
{
  using gpcc::cli::Command;

  cli.AddCommand(Command::Create(cmdName.c_str(), " subcmd [args...]\n"
                                 "Accesses the remote object dictionary. The type of access is specified by"
                                 "<subcmd>:\n"
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
                                 std::bind(&SingleRODACLIClientBase::CLICommandHandler, this,
                                           std::placeholders::_1, std::placeholders::_2)));
  ON_SCOPE_EXIT(removeCliCommand) { cli.RemoveCommand(cmdName.c_str()); };

  Connect(rodaItf);

  ON_SCOPE_EXIT_DISMISS(removeCliCommand);
}

/**
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
SingleRODACLIClientBase::~SingleRODACLIClientBase(void)
{
  try
  {
    cli.RemoveCommand(cmdName.c_str());
    Disconnect();
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
 * This will pick up the first argument and delegate the call to a specialized CLI handler provided by the base
 * class.
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
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to CLI may be incomplete.
 *
 * - - -
 *
 * \param restOfLine
 * Arguments entered behind the command.
 *
 * \param cli
 * [CLI](@ref gpcc::cli::CLI) instance invoking this method.
 */
void SingleRODACLIClientBase::CLICommandHandler(std::string const & restOfLine, gpcc::cli::CLI & cli)
{
  (void)cli;

  if (restOfLine.empty())
    throw gpcc::cli::UserEnteredInvalidArgsError();

  // split restOfLine into "command" (one word) and potential further arguments (args)
  auto pos = restOfLine.find(' ');
  auto const command = restOfLine.substr(0U, pos);
  if (pos == std::string::npos)
    pos = restOfLine.size() - 1U;
  auto const args = restOfLine.substr(pos + 1U);

  if (command == "enum")
    CLI_Enumerate(args);
  else if (command == "info")
    CLI_Info(args);
  else if (command == "read")
    CLI_Read(args);
  else if (command == "write")
    CLI_Write(args);
  else if (command == "caread")
    CLI_CARead(args);
  else if (command == "cawrite")
    CLI_CAWrite(args);
  else
    throw gpcc::cli::UserEnteredInvalidArgsError("Invalid/unknown sub command!");
}

} // namespace cood
} // namespace gpcc
