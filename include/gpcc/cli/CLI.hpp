/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef CLI_HPP_201710051347
#define CLI_HPP_201710051347

#include <gpcc/cli/Command.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Thread.hpp>
#include <memory>
#include <stdexcept>
#include <vector>
#include <cstddef>

namespace gpcc     {
namespace cli      {
namespace internal {
  class ReturnKeyFilter;
  class TerminalRxParser;
}}}

namespace gpcc {
namespace cli  {

class ICLINotifiable;
class ITerminal;

/**
 * \ingroup GPCC_CLI
 * \brief VT100-compatible command line interface (CLI).
 *
 * An instance of this class is intended to be connected to a terminal comprised of a screen and
 * a keyboard. Via the terminal, CLI can display text to the user and CLI can receive input (keystrokes)
 * from the user.
 *
 * CLI establishes a command prompt accessible via the terminal. The CLI component allows the
 * user to enter commands and CLI will execute command handlers associated with the entered commands.
 * In the opposite direction, CLI allows other software components to display text to the user.
 *
 * CLI offers the following features:
 * - Registration and unregistration of commands (see class @ref Command).
 * - Execution of a command's callback when the command is entered.
 * - Parameters entered by the user are passed to the command's callback.
 * - Access to the command prompt can be protected by a password.
 * - The user can modify his/her input using HOME, END, Backspace, DEL, and cursor keys.
 * - Command history to recall previously entered commands.
 * - Word completion.
 * - Other software components (e.g. gpcc's log facility -> see @ref GPCC_LOG) can print text to the user via
 *   CLI _without disturbing the current user input_.
 * - Command's callbacks can request input from the user (@ref ReadLine()).
 * - CLI can deliver notifications to its owner via the @ref ICLINotifiable interface.
 *
 * # Use Cases
 * The most common use cases for CLI are:
 * - Implementation of a debug interface in embedded systems (for developers only).
 * - Implementation of a service interface in embedded systems (for service personell and developers).
 * - Implementation of a debug interface for linux processes
 *
 * # Functionality (from programmer's point of view)
 * ## Connection to terminal
 * Class CLI needs a connection to a terminal to provide it services. Class CLI can be connected to any
 * VT100-compatible terminal or terminal-emulation like PUTTY.
 *
 * To establish a connection to a terminal, a reference to an @ref ITerminal interface must be provided
 * to class CLI's constructor. You can either create your own implementation or use one of the adapters
 * provided by GPCC:
 * - @ref ISyncSerialIO_to_ITerminal
 * - @ref StdIO_to_ITerminal
 *
 * ## Commands
 * Commands that shall be recognized by CLI must be modeled using instances of class @ref Command.
 *
 * One or more @ref Command objects can be registered at one CLI instance. If a registered @ref Command
 * is entered into the terminal, then the callback encapsulated by the @ref Command object will be
 * executed in the context of the CLI's thread.
 *
 * Commands can be registered and removed via @ref AddCommand() and @ref RemoveCommand().
 *
 * The following example shows addition and removal of commands in an exception safe manner:
 * ~~~{.cpp}
 * void MyClass::RegisterCommands(void)
 * {
 *   ON_SCOPE_EXIT() { UnregisterCommands(); };
 *
 *   cli.AddCommand(Command::Create("Command1",
 *                                  " param1 param2\n"\
 *                                  "Example command, that does something with param1 and param2",
 *                                  std::bind(&MyClass::Command1Callback,
 *                                            this, std::placeholders::_1, std::placeholders::_2));
 *
 *   cli.AddCommand(Command::Create("Command2",
 *                                  "\n"\
 *                                  "Another example command",
 *                                  std::bind(&MyClass::Command2Callback,
 *                                            this, std::placeholders::_1, std::placeholders::_2));
 *
 *   ON_SCOPE_EXIT_DISMISS();
 * }
 *
 * void MyClass::UnregisterCommands(void)
 * {
 *   // note: it is not harmful to attempt to unregister commands which have not been registered before
 *   cli.RemoveCommand("Command1");
 *   cli.RemoveCommand("Command2");
 * }
 * ~~~
 *
 * ## Printing to the terminal
 * ### Printing lines
 * CLI offers the following methods to print lines to the terminal:
 * - @ref WriteLine(char const * const)
 * - @ref WriteLine(std::string const &)
 * - @ref WriteLineComposed(char const * const [])
 *
 * Lines can be printed at any time by any thread _without disturbing each other_ and _without disturbing_
 * _the current user input_.\n
 * Multiple lines can be printed with one call to `WriteLine()` using '\\n'. This allows to print
 * multiple associated lines without mixing them with the text printed by other threads at the same time.
 *
 * ~~~{.cpp}
 * myCli.WriteLine("Line 1");
 * myCli.WriteLine("Line 2\n"\
 *                 "Line 3");
 * ~~~
 *
 * Text color, font, and style can be controlled using the definitions in group @ref GPC_CLI_FCS.
 *
 * ### Rewrite lines
 * CLI offers the following method to print a line that can be rewritten by a subsequent call to the same method:
 * - @ref RewriteLine()
 *
 * The method can be used by CLI command handlers only. It is intended to display a status indicator on the terminal
 * and update it cyclically without printing on a new line during each update.
 *
 * ~~~{.cpp}
 * myCli.WriteLine("Process starts...");
 *
 * myCli.RewriteLine("Step 1 of 3");
 * gpcc::osal::Thread::Sleep_ms(100U);
 *
 * myCli.RewriteLine("Step 2 of 3");
 * gpcc::osal::Thread::Sleep_ms(100U);
 *
 * myCli.RewriteLine("Step 3 of 3");
 * gpcc::osal::Thread::Sleep_ms(100U);
 *
 * myCli.RewriteLine("...process complete!");
 * ~~~
 *
 * ## Line Head
 * The command prompt is preceded by a text string, usually ">". This "text string" is called the
 * "line head".
 *
 * The line head used for the command prompt can be changed via @ref SetLineHead().
 *
 * ## Reading user input from the terminal
 * If a command has been entered into the terminal, then the callback encapsulated by the associated
 * @ref Command object will be invoked.
 *
 * The function referenced by the callback can request the user to enter a line into the terminal by
 * invoking @ref ReadLine(). @ref ReadLine() allows to prepend a custom line head to the prompt, e.g.
 * "Enter name:". @ref ReadLine() allows to implement a dialog with the user:
 *
 * ~~~{.cpp}
 * void NewPhoneBookEntryCommandHandler(std::string const & restOfLine, CLI & cli)
 * {
 *   std::string name = cli.ReadLine("Enter name: ");
 *   std::string number = cli.ReadLine("Enter phone number: ");
 *
 *   // ...
 * }
 * ~~~
 *
 * ## Abort of command execution
 * CLI can abort execution of callbacks of @ref Command objects in two cases:
 * - The user pressed CTRL+C
 * - The CLI component shall be stopped (@ref Stop() has been invoked)
 *
 * CLI does not use deferred thread cancellation. Instead the exceptions @ref CLIStopError and
 * @ref CtrlCError will be thrown by @ref ReadLine() and by @ref TestTermination() if execution
 * of a command shall be aborted for some reason.
 *
 * @ref TestTermination() is intended to be invoked cyclically by commands which have a long
 * execution time.
 *
 * Please refer to the documentation of class @ref Command for more information about how to write
 * robust, efficient and graceful callbacks for @ref Command objects.
 *
 * ## Password protection (login/logout)
 * Class CLI allows to protect access to the command prompt by requiring a password for login into
 * the command prompt.
 *
 * There are two states: The user can be either _logged in_ or _logged out_:
 * - In state _logged in_, the command prompt is displayed to the user and the user can enter commands.\n
 *   The user remains "logged in" until he/she enters the command "logout".\n
 *   "logout" is a @ref Command build-in into the CLI.
 * - In state _logged out_, a password prompt is displayed to the user. The user cannot enter commands,
 *   but text can be printed to the user.\n
 *   To log in, the user must either enter the correct password, or enter "login", if no password
 *   is setup.
 *
 * A password can be setup and cleared via @ref SetPassword(). The currently configured password can be
 * retrieved via @ref GetPassword().
 *
 * __Note:__
 * - If a password shall be setup, then it shall be setup _before_ the CLI component is started.\n
 *   See @ref SetPassword() for details.
 * - The password only affects entering commands. Any text printed via `WriteLine()` (e.g.
 *   log messages) is always displayed to the user, regardless if he/she is logged in or not.
 *
 * __Note that building a secure application is not a trivial task. CLI's password feature alone__
 * __may not be sufficient to achieve a reasonable level of protection against unauthorized access.__
 *
 * When using CLI, consider the following: (this list is likely incomplete)
 * - How is CLI connected to the terminal? (UART / Ethernet)
 * - Is the connection to the terminal encrypted?
 * - Is the connection to the terminal physically accessible?
 * - Does a _release version_ of your software really need a CLI or could it be completely disabled
 *   in a _release version_?
 * - Where is the password stored?
 * - Is the password stored encrypted?
 * - How can the password be changed?
 * - Review your commands: What is the goal of an intruder and how could an intruder use your commands
 *   to achieve it?
 *
 * If security is a concern, then utilizing professional assistance might be a good idea.
 *
 * ## Line length
 * The whole line of the terminal's screen except for the last character can be written with user
 * input. The last character is reserved for the cursor. This simplifies the implementation of class
 * CLI, because different terminals and terminal emulations behave different regarding the cursor
 * position and wrap around behavior if the last character of a line is written.
 *
 * ## Start and stop
 * Class CLI is an active component. It uses a own thread to execute code.\n
 * The CLI component can be started and stopped via @ref Start() and @ref Stop().
 *
 * In stopped-state all API methods are functional, but user input is not processed and thus
 * commands are not executed.
 *
 * # Functionality (from user's point of view)
 * ## Cursor control
 * The user can edit his/her entry in a comfortable way. POS1, END, BACKSPACE, DEL, and the left/right
 * arrow keys are supported and can be used to move the cursor and to delete and insert characters
 * at any place.
 *
 * ## Command history
 * Class CLI provides a history that allows to recall commands that have been entered before. The depth of
 * the command history is limited. The arrow-up and arrow-down keys can be used to cycle through the command
 * history.
 *
 * __The arrow-up key:__
 * - The first keystroke recalls the _latest_ entered command into the command line.
 * - Subsequent keystrokes will recall older commands.
 * - At the _oldest_ command, a keystroke will recall the _latest_ entered command again (wrap-around in
 *   command list).
 *
 * __The arrow-down key:__
 * - Has no effect if no command from the command history is currently recalled in the command prompt.
 * - Recalls the next (more recent) command.
 * - At the _most recent_ (latest) command, a keystroke will recover the user's input. There is no wrap-
 *   around in the command list.
 *
 * ## Help
 * Class CLI is equipped with a build-in command "help". If the user enters "help" on the command prompt,
 * then a list of all registered commands will be printed to the terminal.
 *
 * If the user enters a valid command plus "help" on the command prompt, then a help text dedicated
 * to the entered command will be displayed to the user. The help text is encapsulated in the @ref Command
 * object associated with the command entered by the user.
 *
 * ## Word completion
 * The user can enter a command including spelling errors or typos.
 *
 * By pressing the TAB key, class CLI builds a list of @ref Command objects which fit best the user's
 * erroneous or incomplete input. By pressing the TAB key multiple times, the user can cycle through
 * the list of suggested commands.
 *
 * The list of suggestions is sorted by the Levenshtein-distance of the suggested command to the user's
 * incomplete or erroneous input.
 *
 * Example:\n
 * Assume there are three registered commands: "test", "ShowLastErrors", and "ShowOldestError".
 * 1. The user enters "showlstrrors"
 * 2. The user presses TAB. "ShowLastErrors" will be displayed on the command prompt.
 * 3. The user presses TAB. "ShowOldestError" will be displayed on the command prompt.
 * 4. The user presses TAB. "test" will be displayed on the command prompt.
 * 5. The user pressed TAB. The user's original input "showlstrrors" will be displayed on the command prompt.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class CLI final
{
  public:
    /// Minimum value for the width of the terminal in characters.
    static uint8_t const minimumTerminalWidth = 80U;

    /// Maximum value for the width of the terminal in characters.
    static uint8_t const maximumTerminalWidth = 240U;


    /// Minimum capacity of the command history.
    static uint8_t const minimumHistoryDepth = 4U;

    /// Maximum capacity of the command history.
    static uint8_t const maximumHistoryDepth = 32U;


    /// Maximum number of suggestions (TAB-key)
    static uint8_t const maximumSuggestions = 6U;


    CLI(ITerminal& _terminal,
        uint8_t const _terminalWidth,
        uint8_t const _historyDepth,
        char const * const pThreadName,
        ICLINotifiable * const _pICLINotifiable);
    CLI(CLI const &) = delete;
    CLI(CLI &&) = delete;
    ~CLI(void);

    CLI& operator=(CLI const &) = delete;
    CLI& operator=(CLI &&) = delete;


    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

    void AddCommand(std::unique_ptr<Command> spNewCmd);
    void RemoveCommand(char const * const pCmdStr);

    void SetPassword(std::string const & newPassword);
    std::string GetPassword(void) const;

    void SetLineHead(std::string const & newConsoleLineHead);

    void WriteLineComposed(char const * const fragments[]);
    void WriteLine(char const * const s);
    void WriteLine(std::string const & s);

    void RewriteLine(std::string const & s);

    std::string ReadLine(std::string const & lineHead);
    void TestTermination(void);

  private:
    /// Timeout (in ms) applied when waiting for user-input via @ref ITerminal::Read().
    /** Each time when the timeout expires the CLI's thread will return from @ref ITerminal::Read()
        and CLI will check if a stop request (call to @ref Stop()) is pending. If no stop request
        is pending, then CLI will continue to wait for user-input from the terminal via
        @ref ITerminal::Read() (with timeout). */
    static uint16_t const terminalReadTimeout_ms = 1000U;


    /// Interface used to access the terminal.
    ITerminal& terminal;

    /// @ref ICLINotifiable interface that shall be informed about special CLI related events.
    /** This may be nullptr. */
    ICLINotifiable* const pICLINotifiable;

    /// Width of the terminal's screen in characters.
    /** This minus one minus the length of the current input line head (@ref currentLineHead) determines the
        maximum number of characters that can be entered into the terminal by the user before further
        user input will be ignored by CLI. */
    uint8_t const terminalWidth;

    /// Depth/capacity of the command history.
    uint8_t const historyDepth;


    /// Thread used by the @ref CLI component.
    gpcc::osal::Thread thread;

    /// Parser for data received from the terminal.
    /** No mutex is required, this is only accessed by the CLI's thread. */
    std::unique_ptr<internal::TerminalRxParser> spRxParser;

    /// Filter for recognizing RETURN-keypress in CR/LF sequences received from the terminal.
    /** No mutex is required, this is only accessed by the CLI's thread. */
    std::unique_ptr<internal::ReturnKeyFilter> spReturnKeyFilter;

    /// Command history containing commands that have been entered into the terminal.
    /** No mutex is required, this is only accessed by the CLI's thread.\n
        The last element is the _latest_ entered command, the first element is the _oldest_ entered command. */
    std::vector<std::string> history;


    /// Mutex used to protect access to the list of registered commands and to the list of suggestions.
    /** Locking order: @ref terminalMutex -> @ref cmdMutex */
    gpcc::osal::Mutex cmdMutex;

    /// Pointer to the currently executed command. `nullptr` if no command is currently executed.
    /** @ref cmdMutex is required.\n
        The only thread modifying this is the CLI's thread. Locking @ref cmdMutex for read access
        is therfore not mandatory for code executed by the CLI's thread. */
    Command const * pCurrExecCmd;

    /// Condition variable used to signal when @ref pCurrExecCmd has been set to `nullptr`.
    /** This is to be used in conjunction with @ref cmdMutex. */
    gpcc::osal::ConditionVariable cv_pCurrExecCmd_IsNull;

    /// Head of single-linked-list of registered commands.
    /** @ref cmdMutex is required.\n
        The list's content is sorted alphabetically and upper-case-first by command string. */
    Command* pCMDListHead;


    /// Flag indicating if the list of suggestions (-> TAB-key) is valid.
    /** @ref cmdMutex is required.\n
        true = list is valid\n
        false = list is not valid */
    bool suggestionsValid;

    /// List of suggested commands, sorted by Levenshtein-distance, alphabetically, and upper-case-first.
    /** @ref cmdMutex is required.\n
        The content is only valid if @ref suggestionsValid is true. */
    std::vector<Command const *> suggestions;

    /// Iterator for @ref suggestions.
    /** @ref cmdMutex is required.\n
        This is only valid if @ref suggestionsValid is true. */
    std::vector<Command const *>::iterator suggestionIterator;


    /// Mutex used to protect access to the terminal.
    /** Locking order: @ref terminalMutex -> @ref cmdMutex */
    mutable gpcc::osal::Mutex terminalMutex;

    /// Line head used for the command prompt, e.g. '>'.
    /** @ref terminalMutex is required. */
    std::string commandLineHead;

    /// Current line head.
    /** @ref terminalMutex is required.\n
        This is the currently used line head. If commands are expected to be entered,
        then this is equal to @ref commandLineHead. If user input (via @ref ReadLine()) is
        expected, then this will be a user-defined string. */
    std::string currentLineHead;

    /// Input buffer. This is kept synchronous with the user-input visible on the terminal.
    /** @ref terminalMutex is required. */
    std::string inputBuffer;

    /// X-offset of the cursor inside @ref inputBuffer.
    /** @ref terminalMutex is required.\n
        On the terminal's screen, zero corresponds to the position directly behind @ref currentLineHead. \n
        This is only valid, if @ref recoveryRequired is true. */
    uint8_t cursorX;

    /// Flag indicating if the latest line on the terminal's screen needs to be recovered when printing
    /// something via @ref WriteLine().
    /** @ref terminalMutex is required.\n
        true  = recovery required\n
        false = recovery not required */
    bool recoveryRequired;

    /// Controls if @ref RewriteLine() is allowed to rewrite the prev. line or if it shall behave like @ref WriteLine().
    /** @ref terminalMutex is required.\n
        true = rewrite of previous line is allowed\n
        false = write a new line */
    bool allowRewriteLine;

    /// Password required for login to the command prompt.
    /** @ref terminalMutex is required.\n
        If this is an empty string, then no password is required to access the command prompt.\n
        However, the user has to login by typing "login". */
    std::string password;

    /// Flag indicating if a user is currently logged in or not.
    /** No mutex is required, this is only accessed by the CLI's thread.\n
        true  = user is logged in\n
        false = user is not logged in */
    bool loggedIn;


    void Terminal_Write(char const * const s);
    void Terminal_Write(std::string const & s);
    void Terminal_Write(char const c);
    void Terminal_MoveCursorX(int16_t deltaX);
    void Terminal_MoveCursorY_OneUp(void);
    void Terminal_DeleteChars(uint8_t const n);
    void Terminal_EraseFromCursorToEOL(void);

    void ClearInputBuffer(void);
    void ReplaceInputBufferWithString(std::string const & newInputBuffer, size_t const maxLength);
    void ReplaceInputBufferWithHistory(uint8_t const idx, size_t const maxLength);

    void AddInputBufferToHistory(void);

    void BuildListOfSuggestions(void);
    void InsertIntoSuggestionList(Command const * const pCommand);
    void RemoveFromSuggestionList(Command const * const pCommand) noexcept;

    void EnterLine(bool const useHistory, std::string const & lineHead);
    void RobustEnterLine(bool const useHistory, std::string const & lineHead);

    void ProcessLineEnteredIntoCLI(void);
    void LogIn(void);
    void* InternalThreadEntry(void);

    void CCMD_help(std::string const & restOfLine, CLI & cli);
    void CCMD_logout(std::string const & restOfLine, CLI & cli);

    void PrintException(std::exception const & e, size_t const level = 0);

    void DeleteRegisteredCommands(void) noexcept;
};

} // namespace cli
} // namespace gpcc

#endif // CLI_HPP_201710051347
