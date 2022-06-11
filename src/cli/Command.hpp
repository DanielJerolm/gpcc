/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2021, 2022 Daniel Jerolm

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

#ifndef COMMAND_HPP_201710042140
#define COMMAND_HPP_201710042140

#include <functional>
#include <memory>
#include <string>
#include <cstdint>

namespace gpcc {
namespace cli  {

class CLI;

/**
 * \ingroup GPCC_CLI
 * \brief A command that can be registered at a @ref CLI instance.
 *
 * This class encapsulates everything required to fully specify a command that can be recognized
 * by a @ref CLI instance's command prompt:
 * - the command string that must be entered by the user to trigger execution of the command
 * - a help text describing the command
 * - a functor to a callback that shall be executed if the command string has been entered by the user
 *
 * # Usage
 * ## Creation and registration at a CLI instance
 * @ref Command objects are created via @ref Command::Create(). All constructors are private
 * and thus not accessible. Using a factory method ensures that @ref Command objects can only
 * be created into smart pointers (`std::unique_ptr<Command>`).
 *
 * When a @ref Command object is registered at a @ref CLI instance, then ownership moves from the
 * caller to the @ref CLI instance. A @ref Command instance can be registered at exactly one
 * @ref CLI instance only. See @ref CLI::AddCommand() for details.
 *
 * Example:
 * ~~~{.cpp}
 * auto spCMD = Command::Create("doSomething", " param1 param2\n"
 *                              "Example command, that does something with param1 and param2",
 *                              std::bind(&MyClass::doSomethingCmdHandler,
 *                                        this,
 *                                        std::placeholders::_1,
 *                                        std::placeholders::_2));
 * MyCLI.AddCommand(std::move(spCMD));
 * ~~~
 *
 * In the example above, the help text will be shown if the user enters "doSomething help" into the
 * command prompt.
 *
 * ## Unregistration
 * @ref Command objects registered at a @ref CLI instance can be manually unregistered again.
 * After unregistration, the command will be no longer recognized by @ref CLI if it is entered by the user.
 * See @ref CLI::RemoveCommand() for details.
 *
 * Example:
 * ~~~{.cpp}
 * // remove the command which has been registered in the previous example
 * MyCLI.RemoveCommand("doSomething");
 * ~~~
 *
 * If a @ref Command object is not manually unregistered, then it will be automatically unregistered and
 * released when the @ref CLI instance is finally destroyed.
 *
 * ## Writing robust, efficient and graceful callbacks
 * ### Processing parameters
 * The following example shows how parameters could be fetched by a command handler and converted
 * to useful types:
 *
 * ~~~{.cpp}
 * void ExampleCommandHandler(std::string const & restOfLine, CLI & cli)
 * {
 *   // step 1: Split restOfLine into multiple std::string objects
 *   auto params = gpcc::string::Split(restOfLine, ' ', true);
 *
 *   if (params.size() != 2U)
 *     throw UserEnteredInvalidArgsError("2 parameters expected!");
 *
 *   // step 2: Convert strings to numbers
 *   uint32_t const p1 = 0U;
 *   uint8_t const p2 = 0U;
 *
 *   try
 *   {
 *     p1 = gpcc::string::AnyStringToU32(params[0]);
 *     p2 = gpcc::string::DecimalToU8(params[1]);
 *   }
 *   catch (std::exception const &)
 *   {
 *     std::throw_with_nested(UserEnteredInvalidArgsError());
 *   }
 *
 *   // string objects are no longer needed
 *   params.clear();
 *
 *   // step 3: Check values
 *   // (check for an fictious constraint)
 *   if (p1 == p2)
 *     throw UserEnteredInvalidArgsError("param1 must not equal param2");
 *
 *   // ...
 * }
 * ~~~
 *
 * Note that step 2 contains full error checking of the user's input behind the scenes.\n
 * Please refer to @ref gpcc::string for more useful string conversion functions.
 *
 * In case of any error encountered with `restOfLine`, a command handler shall throw @ref UserEnteredInvalidArgsError.
 * The @ref CLI will then print a message containing a hint to enter the command's name plus "help", which will show
 * an explanation of the CLI command and its proper use. If available, then the message will also contain details from
 * the @ref UserEnteredInvalidArgsError exception indicating what was wrong with the arguments.
 *
 * ### Error handling
 * Class @ref CLI supports error handling for command callbacks based on exceptions. If a command
 * callback throws an exception, then class @ref CLI will print an error message to the user. The
 * error message contains the error description retrieved from the exception's `what()` method and
 * also the error description of all nested exceptions (if any). After printing the error message(s),
 * @ref CLI will continue normally and display the command prompt to the user.
 *
 * __Summarized, command callbacks shall just follow the rules of exception safe programming.__\n
 * __Command callbacks shall provide at least the basic guarantee.__
 *
 * ~~~{.cpp}
 * void ExampleCommandHandler(std::string const & restOfLine, CLI & cli)
 * {
 *   if (!restOfLine.empty())
 *     throw UserEnteredInvalidArgsError("No parameters expected!");
 *
 *   // ...
 *
 *   if (condition == bad)
 *     throw std::runtime_error("ExampleCommandHandler: Error XYZ has happened");
 *
 *   // ...
 * }
 * ~~~
 *
 * ### Handling CLI stop requests and CTRL+C keystrokes
 * Most command callback implementations will finish quickly, but there may be also implementations executing
 * actions which require seconds or even minutes to complete.
 *
 * However, any command callback implementation regardless of its execution time __shall__ be aware of the
 * following two events:
 * - The @ref CLI component could be requested to stop (its owner invokes @ref CLI::Stop()) while the
 *   command's callback is executed.
 * - The user could press CTRL+C while the command's callback is executed.
 *
 * In the first case, @ref CLI::Stop() will block until execution of the callback has finished and until the
 * CLI's thread has returned from the callback back into the @ref CLI component where it will terminate.
 *
 * In the second case, the CTRL+C keystroke will usually be recorded among other keystrokes in some kind of
 * buffer (e.g. inside the software component providing the @ref ITerminal interface to @ref CLI) until the
 * keystrokes are fetched by @ref CLI.
 *
 * There are two methods which could be invoked by a command's callback, which will check if a stop request
 * is pending and which will read keystrokes from the @ref ITerminal interface and look for CTRL+C:
 * - @ref CLI::ReadLine()
 * - @ref CLI::TestTermination()
 *
 * All other API methods offered by @ref CLI _will not_ check for a pending stop request and _will not_
 * fetch keystrokes from the @ref ITerminal interface.
 *
 * If necessary, @ref CLI::ReadLine() and @ref CLI::TestTermination() will throw one of the
 * following exceptions:
 * - @ref CLIStopError
 * - @ref CtrlCError
 *
 * The command's callback shall follow the rules of exception safe programming (see chapter
 * "Error handling" above), so occurrence of any of the two exceptions will result in well-controlled
 * abort of execution of the command's callback.
 *
 * The example below shows proper implementation of a time-consuming command callback:
 * ~~~{.cpp}
 * void ExampleCommandHandler(std::string const & restOfLine, CLI & cli)
 * {
 *   // ...
 *
 *   // process items (may take seconds or minutes)
 *   for (auto & item: itemsToBeProcessed)
 *   {
 *     ON_SCOPE_EXIT()
 *     {
 *       // If necessary, one could store the progress here in order to continue seamless
 *       // during a second invocation of the command.
 *       // It is also possible to place clean-up code here...
 *     };
 *
 *     // this will throw, if CLI shall stop or if the user pressed CTRL+C
 *     cli.TestTermination();
 *
 *     // this actually does the productive work in this example
 *     item.Process();
 *
 *     ON_SCOPE_EXIT_DISSMISS();
 *   }
 *
 *   // ...
 * }
 * ~~~
 *
 * The example below shows an implementation of a callback that asks for confirmation before
 * abort:
 * ~~~{.cpp}
 * void ExampleCommandHandler(std::string const & restOfLine, CLI & cli)
 * {
 *   // ...
 *
 *   try
 *   {
 *     // this will throw, if CLI shall stop or if the user pressed CTRL+C
 *     cli.TestTermination();
 *   }
 *   catch (CLIStopError const &)
 *   {
 *     // we can catch CLIStopError, but we have to rethrow it...
 *     throw;
 *
 *     // usually catching CLIStopError makes no sense, so this catch-block
 *     // should be omitted.
 *   }
 *   catch (CtrlCError const &)
 *   {
 *     // we can catch CtrlCError, and we are allowed to drop it in order to
 *     // ignore the CTRL+C if we like to do so...
 *
 *     if (cli.ReadLine("Abort? Sure? Type 'yes'>") == "yes")
 *       throw;
 *   }
 *
 *   // ...
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class Command final
{
  public:
    /// Type of functor to the callback that shall be executed if the command has been entered
    /// into CLI's command prompt.
    /** The referenced function/method will be executed in the context of the thread of the @ref CLI
        instance at which this @ref Command has been registered.

        - - -

        Parameters:\n
        1st: Unmodifiable reference to an `std::string` containing any characters entered behind
        the command, i.e. parameters\n
        2nd: Reference to the @ref CLI instance into which the command has been entered.

        - - -

        __Thread safety requirements/hints:__
        - The referenced function/method will be executed in the context of the thread of the
          @ref CLI instance given by the second parameter.
        - The referenced function/method may safely invoke any non-blocking method offered by @ref CLI.

        __Exception safety requirements/hints:__\n
        The referenced function/method shall provide at least the basic guarantee and
        fail gracefully.\n
        Class @ref CLI is aware of exceptions and will print the description of any caught exception
        and the description of any nested exception to the terminal.

        __Thread cancellation safety requirements/hints:__\n
        The referenced function/method will be invoked with deferred thread cancellation disabled. */
    typedef std::function<void(std::string const &, CLI&)> tCommandFunc;


    /// Next-pointer used by @ref CLI to create an intrusive single linked list containing
    /// all registered @ref Command objects.
    Command* pNext;

    /// Levenshtein-distance of this command to the user's entry.
    /** This is used by class @ref CLI to suggest commands that best fit the user's entry. */
    uint8_t levenshteinDistance;


    ~Command(void) = default;

    static std::unique_ptr<Command> Create(char const * const _command,
                                           char const * const _helpText,
                                           tCommandFunc const & _onCmdEntered);


    char const * GetCommand(void) const noexcept;
    char const * GetHelpText(void) const noexcept;

    void Entered(std::string const & restOfLine, CLI & cli) const;

  private:
    /// Command string.
    /** If this string is entered into the terminal, then the function/method referenced by
        @ref onCmdEntered will be executed.\n
        This refers to a null-terminated string, usually located in code memory / ROM. */
    char const * const command;

    /// Help text.
    /** This is concatenated to @ref command and displayed in the terminal,
        if the command plus "help" is entered into the terminal.\n
        This refers to a null-terminated string, usually located in code memory / ROM. */
    char const * const helpText;

    /// Functor to the function/method that shall be executed if the command has been entered into the terminal.
    tCommandFunc onCmdEntered;


    Command(void) = delete;
    Command(char const * const _command,
            char const * const _helpText,
            tCommandFunc const & _onCmdEntered);
    Command(Command const &) = delete;
    Command(Command &&) = delete;

    Command& operator=(Command const &) = delete;
    Command& operator=(Command &&) = delete;
};

/**
 * \brief Retrieves the command string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Pointer to a null-terminated c-string containing the command string.\n
 * The referenced string is valid during the life-time of this object.
 */
inline char const * Command::GetCommand(void) const noexcept
{
  return command;
}

/**
 * \brief Retrieves the help text string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Pointer to a null-terminated c-string containing the help string.\n
 * Please refer to the details of @ref Command for an description of the format and
 * purpose of the help string.\n
 * The referenced string is valid during the life-time of this object.
 */
inline char const * Command::GetHelpText(void) const noexcept
{
  return helpText;
}

} // namespace cli
} // namespace gpcc

#endif // COMMAND_HPP_201710042140
