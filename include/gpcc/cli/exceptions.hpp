/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef EXCEPTIONS_HPP_201710232109
#define EXCEPTIONS_HPP_201710232109

#include <gpcc/string/SharedString.hpp>
#include <stdexcept>
#include <string>

namespace gpcc     {
namespace cli      {

/**
 * \ingroup GPCC_CLI_EXCEPTIONS
 * \brief Exception thrown if sending output to the terminal has failed.
 *
 * The original exception thrown by the terminal output function is nested to this.
 */
class TerminalOutputError : public std::runtime_error
{
  public:
    inline TerminalOutputError(void) : std::runtime_error("Terminal Output Error.") {};
    virtual ~TerminalOutputError(void) = default;
};

/**
 * \ingroup GPCC_CLI_EXCEPTIONS
 * \brief Exception thrown if the @ref CLI component has been requested to stop.
 *
 * This exception is thrown by @ref CLI::ReadLine() and @ref CLI::TestTermination(), if the
 * @ref CLI component has been requested to stop. The exception is relevant for callbacks of
 * @ref Command objects.
 *
 * The CLI command's callback may catch this exception, but it __shall__ re-throw it
 * in order to terminate quickly and in order to let the exception propagate back into the
 * @ref CLI component, which is well aware of the exception.
 */
class CLIStopError : public std::runtime_error
{
  public:
    inline CLIStopError(void) : std::runtime_error("CLI component requested to stop.") {};
    virtual ~CLIStopError(void) = default;
};

/**
 * \ingroup GPCC_CLI_EXCEPTIONS
 * \brief Exception thrown if the user has pressed CTRL+C.
 *
 * This exception is thrown by @ref CLI::ReadLine() and @ref CLI::TestTermination(), if the
 * user has pressed CTRL+C. The exception is relevant for callbacks of @ref Command objects.
 *
 * The CLI command's callback may catch this exception, but it _should_ re-throw it
 * in order to terminate and in order to let the exception propagate back into the @ref CLI
 * component. _The callback is allowed to not re-throw the exception_. In this case, the CTRL+C
 * keystroke of the user will be ignored. The latter case should be rare.
 */
class CtrlCError : public std::runtime_error
{
  public:
    inline CtrlCError(void) : std::runtime_error("User hit CTRL+C.") {};
    virtual ~CtrlCError(void) = default;
};

/**
 * \ingroup GPCC_CLI_EXCEPTIONS
 * \brief Exception thrown by a CLI command handler if the user has entered invalid arguments ("restOfLine") or if
 *        arguments are missing or unexpected.
 *
 * # CLI output
 * The @ref CLI component will print the following generic message if it catches this exception:\n
 * "Invalid arguments. Try 'CMD help'." (CMD will be replaced with the command's name)
 *
 * The @ref CLI component will append the details contained in the exception to the generic message, if the details are
 * not empty. If the details are empty, then the what-text of the exception and the what-texts of all nested exceptions
 * will be printed to CLI.
 *
 * # Usage
 * A @ref UserEnteredInvalidArgsError exception shall be thrown if the parameters/arguments passed to a CLI command
 * are invalid.
 *
 * Use `throw UserEnteredInvalidArgsError()` with no details if the error is obvious if the user reads the CLI
 * command's help text.
 *
 * Use `throw UserEnteredInvalidArgsError("details...")` to provide a concrete hint to the user what's wrong with the
 * arguments.
 *
 * Other exceptions can be rethrown as nested exceptions attached to a `UserEnteredInvalidArgsError` exception if that
 * makes sense. `std::bad_alloc` should not be rethrown as nested exception. See example below.
 *
 * ~~~{.cpp}
 * void ExampleCommandHandler(std::string const & restOfLine, CLI & cli)
 * {
 *   // split restOfLine into multiple std::string objects
 *   auto params = gpcc::string::Split(restOfLine, ' ', true);
 *
 *   // analyse parameters and extract information
 *   uint32_t p1 = 0U;
 *   uint8_t p2 = 0U;
 *   try
 *   {
 *     if (params.size() != 2U)
 *       throw UserEnteredInvalidArgsError();
 *
 *     p1 = gpcc::string::AnyStringToU32(params[0]);
 *     p2 = gpcc::string::DecimalToU8(params[1]);
 *
 *     // [...] More code that could also throw std::bad_alloc
 *
 *     // string objects are no longer needed
 *     params.clear();
 *
 *     // check values
 *     // (check for an fictious constraint)
 *     if (p1 == p2)
 *       throw UserEnteredInvalidArgsError("param1 must not equal param2");
 *   }
 *   catch (std::bad_alloc const &)
 *   {
 *     // Just rethrow std::bad_alloc. We do not want to have it wrapped into a UserEnteredInvalidArgsError.
 *     throw;
 *   }
 *   catch (UserEnteredInvalidArgsError const &)
 *   {
 *     // Just rethrow UserEnteredInvalidArgsError. We do not want to wrap it into
 *     // another UserEnteredInvalidArgsError.
 *     throw;
 *   }
 *   catch (std::exception const &)
 *   {
 *     // Everything else going wrong shall be wrapped into a UserEnteredInvalidArgsError.
 *     std::throw_with_nested(UserEnteredInvalidArgsError());
 *   }
 *
 *   // ...
 * }
 * ~~~
 */
class UserEnteredInvalidArgsError : public std::runtime_error
{
  public:
    /// Description what was wrong with the arguments entered behind the command. This may be an empty string.
    gpcc::string::SharedString const details;

    UserEnteredInvalidArgsError(void);
    UserEnteredInvalidArgsError(std::string const & _details);
    UserEnteredInvalidArgsError(std::string && _details);
    virtual ~UserEnteredInvalidArgsError(void) = default;
};

} // namespace cli
} // namespace gpcc

#endif // EXCEPTIONS_HPP_201710232109
