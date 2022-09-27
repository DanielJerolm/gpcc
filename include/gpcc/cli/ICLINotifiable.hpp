/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ICLINOTIFIABLE_HPP_201712012244
#define ICLINOTIFIABLE_HPP_201712012244

namespace gpcc {
namespace cli  {

class CLI;

/**
 * \ingroup GPCC_CLI
 * \brief Interface which must be implemented by the owner of an @ref CLI instance in order to receive notifications
 * upon certain @ref CLI related events.
 *
 * Implementation of this interface and registration at @ref CLI is optional, not mandatory.
 *
 * - - -
 *
 * __Thread safety:__\n
 * - All methods in this interface are invoked in the context of the @ref CLI component.
 * - CLI guarantees, that no more than one method is invoked at any time.
 */
class ICLINotifiable
{
  public:
    virtual void OnBeforePasswordPrompt(CLI & cli) = 0;
    virtual void OnWrongPasswordEntered(CLI & cli) = 0;
    virtual void OnLogin(CLI & cli) = 0;
    virtual void OnLogout(CLI & cli) = 0;
    virtual void OnCTRL_C(CLI & cli) = 0;

  protected:
    ICLINotifiable(void) = default;
    virtual ~ICLINotifiable(void) = default;
};

/**
 * \fn void ICLINotifiable::OnBeforePasswordPrompt(gpcc::cli::CLI & cli)
 * \brief This is invoked directly before [CLI](@ref gpcc::cli::CLI) prints "Type 'login' or password>" to the terminal.
 *
 * This method could be used to display legal information:
 * ~~~{.cpp}
 * void MyApplication::OnBeforePasswordPrompt(CLI & cli)
 * {
 *   cli.WriteLine("Debug interface of PRODUCT-NAME.\n"\
 *                 "Usage is restricted to authorized personnel only!");
 * }
 * ~~~
 *
 * If there is no use for this method, than it can be left empty.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__\n
 * This will be invoked in the context of the [CLI](@ref gpcc::cli::CLI) component.\n
 * All non-blocking methods offered by [CLI](@ref gpcc::cli::CLI) may be safely invoked from this.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the basic guarantee:
 * - Incomplete/undefined output printed to the terminal is anticipated
 *
 * Any exception will result in a retry by [CLI](@ref gpcc::cli::CLI) to display the password prompt
 * after a short delay.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param cli
 * Reference to the [CLI](@ref gpcc::cli::CLI) component.
 */

/**
 * \fn void ICLINotifiable::OnWrongPasswordEntered(gpcc::cli::CLI & cli)
 * \brief This is invoked after a wrong password has been entered by the user and after
 * [CLI](@ref gpcc::cli::CLI) has printed "Wrong password." to the terminal.
 *
 * In case of a failed login, [CLI](@ref gpcc::cli::CLI) implements a delay of one second before
 * next password prompt.
 *
 * An application could use this method to:
 * - Add extra delay (via @ref gpcc::osal::Thread::Sleep_ms()) before next password prompt.
 * - Record the event to some log facility.
 * - Shutdown the product/application after multiple failed attempts to login.
 *
 * If there is no use for this method, than it can be left empty.
 *
 * This will not be invoked, if no password is setup and the user fails to enter "login" into
 * the password/login prompt.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__\n
 * This will be invoked in the context of the [CLI](@ref gpcc::cli::CLI) component.\n
 * All non-blocking methods offered by [CLI](@ref gpcc::cli::CLI) may be safely invoked from this.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the basic guarantee:
 * - Incomplete/undefined output printed to the terminal is anticipated
 *
 * Any exception will result in a retry by [CLI](@ref gpcc::cli::CLI) to display the password prompt
 * after a short delay.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param cli
 * Reference to the [CLI](@ref gpcc::cli::CLI) component.
 */

/**
 * \fn void ICLINotifiable::OnLogin(gpcc::cli::CLI & cli)
 * \brief This is invoked directly after successful login before the command prompt is displayed.
 *
 * This method could be used to display a welcome screen and legal information:
 *
 * ~~~{.cpp}
 * void MyApplication::OnLogin(CLI & cli)
 * {
 *   cli.WriteLine("Welcome to debug interface of PRODUCT-NAME.\n"\
 *                 "Usage of this interface is restricted to authorized personnel only!\n"\
 *                 "Type 'help' for assistance.\n");
 * }
 * ~~~
 *
 * If the [ICLINotifiable](@ref gpcc::cli::ICLINotifiable) interface is not implemented and not connected
 * to [CLI](@ref gpcc::cli::CLI), then [CLI](@ref gpcc::cli::CLI) will print the following default welcome
 * message after successful login:\n
 * "Welcome. Type 'help' for assistance."
 *
 * If the [ICLINotifiable](@ref gpcc::cli::ICLINotifiable) interface is implemented and connected to
 * [CLI](@ref gpcc::cli::CLI), then [CLI](@ref gpcc::cli::CLI) __will not__ print a default welcome message
 * after successful login. This means:
 * - If this method does not print a welcome message, then there will be no welcome message.
 * - If this method prints a welcome message, then it should mention the 'help' command as a starting
 *   point for the user. The code snippet above gives an example.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__\n
 * This will be invoked in the context of the [CLI](@ref gpcc::cli::CLI) component.\n
 * All non-blocking methods offered by [CLI](@ref gpcc::cli::CLI) may be safely invoked from this.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the basic guarantee:
 * - Incomplete/undefined output printed to the terminal is anticipated
 *
 * Any exception will result in logout of the user and in retry by [CLI](@ref gpcc::cli::CLI) to
 * display the password prompt after a short delay.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param cli
 * Reference to the [CLI](@ref gpcc::cli::CLI) component.
 */

/**
 * \fn void ICLINotifiable::OnLogout(gpcc::cli::CLI & cli)
 * \brief This is invoked after logout.
 *
 * If there is no use for this method, than it can be left empty.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__\n
 * This will be invoked in the context of the [CLI](@ref gpcc::cli::CLI) component.\n
 * All non-blocking methods offered by [CLI](@ref gpcc::cli::CLI) may be safely invoked from this.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the basic guarantee:
 * - Incomplete/undefined output printed to the terminal is anticipated
 *
 * [CLI](@ref gpcc::cli::CLI) will print the description of any caught exception and the description
 * of any nested exception to the terminal.\n
 * An exception has no influence on logout. Logout will always take place.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param cli
 * Reference to the [CLI](@ref gpcc::cli::CLI) component.
 */

/**
 * \fn void ICLINotifiable::OnCTRL_C(gpcc::cli::CLI & cli)
 * \brief This is invoked if the user has pressed CTRL+C directly from the command prompt.
 *
 * This method can be used to trigger actions like application shutdown etc.\n
 * This method shall print at least "CTRL+C ignored" to the terminal. Further actions are optional.
 *
 * ~~~{.cpp}
 * void MyClass::OnCTRL_C(gpcc::cli::CLI & cli)
 * {
 *   // minimum implementation
 *
 *   cli.WriteLine("CTRL+C ignored");
 * }
 * ~~~
 *
 * Note:\n
 * - Any CTRL+C issued from within a command's callback will not result in invocation of this method.
 * - Printing "CTRL+C ignored" or similar is strongly recommended, because the user's input will be
 *   discarded if CTRL+C is pressed. Without any message, this might be considered rude by users.
 * - After returning from this method, the command prompt (empty) will be displayed to the user.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__\n
 * This will be invoked in the context of the [CLI](@ref gpcc::cli::CLI) component.\n
 * All non-blocking methods offered by [CLI](@ref gpcc::cli::CLI) may be safely invoked from this.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the basic guarantee:
 * - Incomplete/undefined output printed to the terminal is anticipated
 *
 * [CLI](@ref gpcc::cli::CLI) will print the description of any caught exception and the description
 * of any nested exception to the terminal. After a small delay, [CLI](@ref gpcc::cli::CLI) will
 * display the command prompt an continue normally.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param cli
 * Reference to the [CLI](@ref gpcc::cli::CLI) component.
 */

} // namespace cli
} // namespace gpcc

#endif // ICLINOTIFIABLE_HPP_201712012244
