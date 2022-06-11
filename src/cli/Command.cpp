/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2021 Daniel Jerolm

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

#include "Command.hpp"
#include <stdexcept>
#include <cstddef>
#include <cstring>

namespace gpcc {
namespace cli  {

/**
 * \brief Creates a @ref Command instance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _command
 * Pointer to a null-terminated c-string containing the text that must be entered into the terminal
 * to trigger execution of this command.\n
 * _The referenced c-string must not change during the lifetime of this object._\n
 * _The referenced string is intended to be located in code memory or ROM._\n
 * \n
 * The command must meet the following requirements:
 * - minimum length: 1 char
 * - no white spaces
 * - no leading digits
 * \param _helpText
 * Pointer to a null-terminated c-string containing a description of the command and its parameters.\n
 * The text will be displayed if the command plus "help" is entered into the terminal.\n
 * This text will be concatenated to `_command`.\n
 * _The referenced c-string must not change during the lifetime of this object._\n
 * _The referenced string is intended to be located in code memory._\n
 * \n
 * Example:\n
 * ~~~{.cpp}
 * auto spCMD = Command::Create("doSomething", " param1 param2\n"
 *                              "Example command, that does something with param1 and param2",
 *                              std::bind(&MyClass::doSomethingCmdHandler,
 *                                        this,
 *                                        std::placeholders::_1,
 *                                        std::placeholders::_2));
 * MyCLI.AddCommand(std::move(spCMD));
 * ~~~
 * \param _onCmdEntered
 * Functor to a callback that shall be executed if the command has been entered into the terminal.\n
 * See @ref tCommandFunc for details, especially exception safety requirements and thread cancellation
 * safety requirements.\n
 * _A copy is generated._
 */
std::unique_ptr<Command> Command::Create(char const * const _command,
                                         char const * const _helpText,
                                         tCommandFunc const & _onCmdEntered)
{
  return std::unique_ptr<Command>(new Command(_command, _helpText, _onCmdEntered));
}

/**
 * \brief This will be invoked by class @ref CLI if the command has been entered into the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This will be executed in the context of the thread of the @ref CLI instance.
 *
 * __Exception safety:__\n
 * Basic guarantee or better:\n
 * The exact behavior depends on the command handler method that has been passed
 * to the constructor or to the @ref Create() method. See @ref tCommandFunc for details.
 *
 * __Thread cancellation safety:__\n
 * @ref CLI will invoke this with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param restOfLine
 * Unmodifiable reference to an std::string containing the rest of the line entered into the terminal
 * _behind_ the command.\n
 * This will contain parameters passed to the command. If no parameters have been entered by the user,
 * then this will be an empty string.
 * \param cli
 * Reference to the @ref CLI instance into which the command has been entered.
 */
void Command::Entered(std::string const & restOfLine, CLI & cli) const
{
  onCmdEntered(restOfLine, cli);
}

/**
 * \brief Constructor.
 *
 * This constructor is private. @ref Create() shall be used to create instances of class @ref Command.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _command
 * Pointer to a null-terminated c-string containing the text that must be entered into the terminal
 * to trigger execution of this command.\n
 * _The referenced c-string must not change during the lifetime of this object._\n
 * _The referenced string is intended to be located in code memory or ROM._\n
 * \n
 * The command must meet the following requirements:
 * - minimum length: 1 char
 * - no white spaces
 * - no leading digits
 * \param _helpText
 * Pointer to a null-terminated c-string containing a description of the command and its parameters.\n
 * The text will be displayed if the command plus "help" is entered into the terminal.\n
 * This text will be concatenated to `_command`.\n
 * _The referenced c-string must not change during the lifetime of this object._\n
 * _The referenced string is intended to be located in code memory._\n
 * \n
 * Example:\n
 * ~~~{.cpp}
 * auto spCMD = Command::Create("doSomething",
 *                              " param1 param2\n"\
 *                              "Example command, that does something with param1 and param2",
 *                              std::bind(&MyClass::doSomethingCmdHandler,
 *                                        this,
 *                                        std::placeholders::_1,
 *                                        std::placeholders::_2));
 * MyCLI.AddCommand(std::move(spCMD));
 * ~~~
 * \param _onCmdEntered
 * Functor to a callback that shall be executed if the command has been entered into the terminal.\n
 * See @ref tCommandFunc for details, especially exception safety requirements and thread cancellation
 * safety requirements.\n
 * _A copy is generated._
 */
Command::Command(char const * const _command,
                 char const * const _helpText,
                 tCommandFunc const & _onCmdEntered)
: pNext(nullptr)
, levenshteinDistance(0)
, command(_command)
, helpText(_helpText)
, onCmdEntered(_onCmdEntered)
{
  if ((_command == nullptr) || (_helpText == nullptr))
    throw std::invalid_argument("Command::Command: nullptr");

  // check: length
  size_t const n = strlen(_command);
  if (n == 0)
    throw std::invalid_argument("Command::Command: _command is invalid");

  // check: no leading digit
  char const c = *_command;
  if ((c >= '0') && (c <= '9'))
    throw std::invalid_argument("Command::Command: _command is invalid");

  // check: no white spaces
  if (strchr(_command, ' ') != nullptr)
    throw std::invalid_argument("Command::Command: _command is invalid");

  if (!_onCmdEntered)
    throw std::invalid_argument("Command::Command: _onCmdEntered is invalid");
}

} // namespace cli
} // namespace gpcc
