/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "CLI.hpp"
#include "CLIColors.hpp"
#include "ICLINotifiable.hpp"
#include "ITerminal.hpp"
#include "exceptions.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/levenshtein_distance.hpp"
#include "gpcc/src/string/tools.hpp"
#include <exception>
#include <cstdio>
#include <cstring>

namespace gpcc {
namespace cli  {

using namespace gpcc::cli::internal;

#ifndef __DOXYGEN__
uint8_t const CLI::minimumTerminalWidth;
uint8_t const CLI::maximumTerminalWidth;
uint8_t const CLI::minimumHistoryDepth;
uint8_t const CLI::maximumHistoryDepth;
uint8_t const CLI::maximumSuggestions;

uint16_t const CLI::terminalReadTimeout_ms;
#endif

/**
 * \brief Constructor, initial line head for command prompt is ">", no password set.
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
 * \param _terminal
 * Reference to an @ref ITerminal interface that shall be used to access the terminal.
 * \param _terminalWidth
 * Width of the terminal's screen in characters.\n
 * The maximum number of characters that can be entered by the user is:\n
 * terminalWidth - width of the line head (e.g. '>') - 1\n
 * This must be in the range @ref minimumTerminalWidth to @ref maximumTerminalWidth.
 * \param _historyDepth
 * Depth/capacity of the command history.\n
 * This must be in the range @ref minimumHistoryDepth to @ref maximumHistoryDepth.
 * \param pThreadName
 * Pointer to a null-terminated c-string with the name that shall be assigned to the CLI's thread.\n
 * The referenced string must not be released or modified during life-time of this CLI instance.
 * \param _pICLINotifiable
 * @ref ICLINotifiable interface provided by the owner of the CLI instance in order to receive
 * notifications about special CLI related events.\n
 * nullptr is allowed.
 */
CLI::CLI(ITerminal& _terminal,
         uint8_t const _terminalWidth,
         uint8_t const _historyDepth,
         char const * const pThreadName,
         ICLINotifiable * const _pICLINotifiable)
: terminal(_terminal)
, pICLINotifiable(_pICLINotifiable)
, terminalWidth(_terminalWidth)
, historyDepth(_historyDepth)
, thread(pThreadName)
, rxParser()
, returnKeyFilter()
, history()
, cmdMutex()
, pCurrExecCmd(nullptr)
, cv_pCurrExecCmd_IsNull()
, pCMDListHead(nullptr)
, suggestionsValid(false)
, suggestions()
, suggestionIterator()
, terminalMutex()
, commandLineHead(">")
, currentLineHead()
, inputBuffer()
, cursorX(0)
, recoveryRequired(false)
, password()
, loggedIn(false)
{
  if ((terminalWidth < minimumTerminalWidth) ||
      (terminalWidth > maximumTerminalWidth))
    throw std::invalid_argument("CLI::CLI: _terminalWidth out of bounds");

  if ((historyDepth < minimumHistoryDepth) ||
      (historyDepth > maximumHistoryDepth))
    throw std::invalid_argument("CLI::CLI: _historyWidth out of bounds");

  ON_SCOPE_EXIT() { DeleteRegisteredCommands(); };

  AddCommand(Command::Create("help",
                             "\n" \
                             "Prints help.",
                             std::bind(&CLI::CCMD_help, this, std::placeholders::_1, std::placeholders::_2)));
  AddCommand(Command::Create("logout",
                             "\n" \
                             "Locks the console. Invoke this command when you are finished.",
                             std::bind(&CLI::CCMD_logout, this, std::placeholders::_1, std::placeholders::_2)));

  ON_SCOPE_EXIT_DISMISS();
}

/**
 * \brief Destructor.
 *
 * The component must not be running. If necessary, use @ref Stop() to stop the component before destruction.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
CLI::~CLI(void)
{
  DeleteRegisteredCommands();
}

/**
 * \brief Starts the CLI component.
 *
 * If a password shall be required for users to log into the command prompt, then
 * @ref SetPassword() should be invoked __before__ calling this method.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param schedPolicy
 * Scheduling policy that shall be used for the CLI's thread.\n
 * See @ref gpcc::osal::Thread::Start() for details.
 * \param priority
 * Priority level (0 (low) .. 31 (high)) that shall be used for the CLI's thread.\n
 * This is only relevant for the scheduling policies `SchedPolicy::Fifo` and `SchedPolicy::RR`.\n
 * _For the other scheduling policies this must be zero._\n
 * See @ref gpcc::osal::Thread::Start() for details.
 * \param stackSize
 * Size of the stack in byte that shall be allocated for the CLI's thread.\n
 * _This must be a multiple of_ @ref gpcc::osal::Thread::GetStackAlign(). \n
 * _This must be equal to or larger than_ @ref gpcc::osal::Thread::GetMinStackSize(). \n
 * Internally this may be round up to some quantity, e.g. the system's page size.\n
 * See @ref gpcc::osal::Thread::Start() for details.
 */
void CLI::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                gpcc::osal::Thread::priority_t const priority,
                size_t const stackSize)
{
  thread.Start(std::bind(&CLI::InternalThreadEntry, this), schedPolicy, priority, stackSize);
}

/**
 * \brief Requests the CLI component to stop and blocks until the component has stopped.
 *
 * This will block until the CLI's thread has terminated. Remember that the CLI's thread
 * could currently be executing the callback of a command entered into the terminal,
 * so calling this might consume some time.
 *
 * After this has returned, it is safe to either restart the CLI component via @ref Start()
 * or to destroy the CLI instance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void CLI::Stop(void) noexcept
{
  try
  {
    thread.Cancel();
    thread.Join();
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Adds a @ref Command instance to this @ref CLI instance.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param spNewCmd
 * @ref Command instance that shall be registered.\n
 * If there is already a @ref Command instance with the same command string registered,
 * then an exception will be thrown. Note: The check is case insensitive.\n
 */
void CLI::AddCommand(std::unique_ptr<Command> spNewCmd)
{
  if (!spNewCmd)
    throw std::invalid_argument("CLI::AddCommand: !spNewCmd");

  if (spNewCmd->pNext != nullptr)
    throw std::logic_error("CLI::AddCommand: Command has pNext set");

  osal::MutexLocker mutexLocker(cmdMutex);

  if (pCMDListHead == nullptr)
  {
    // (list empty)
    pCMDListHead = spNewCmd.release();
  }
  else
  {
    // (list not empty)

    // find Command object where to insert the new Command object
    Command * pPrev = nullptr;
    Command * pCurr = pCMDListHead;
    while (pCurr != nullptr)
    {
      int const result = strcasecmp(spNewCmd->GetCommand(), pCurr->GetCommand());
      if (result == 0)
      {
        throw std::logic_error("CLI::AddCommand: Command already registered");
      }
      else if (result < 0)
      {
        // (found the appropriate place to insert)
        spNewCmd->pNext = pCurr;

        // insert within list or insert at the head of the list
        if (pPrev != nullptr)
          pPrev->pNext = spNewCmd.release();
        else
          pCMDListHead = spNewCmd.release();

        // that's it
        return;
      }

      // move to next list element for next loop cycle
      pPrev = pCurr;
      pCurr = pCurr->pNext;
    }

    // end of list reached => append at end of list
    pPrev->pNext = spNewCmd.release();
  }
}

/**
 * \brief Removes an @ref Command instance from this @ref CLI instance.
 *
 * If the command's callback is currently executed, then this blocks until execution has finished.
 *
 * Note:\n
 * - The command is removed from the list of suggested commands (-> TAB-key).
 * - The command is not removed from the command history.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This may be called from a @ref Command object's callback,
 * __BUT__ a @ref Command __SHALL NEVER__ unregister itself!
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param pCmdStr
 * Null-terminated c-string containing the command that shall be removed.\n
 * The @ref Command object whose command-string matches this string will be removed.\n
 * If there is no match, then this method will do nothing.\n
 * Build-in commands "help" and "logout" cannot be removed.
 */
void CLI::RemoveCommand(char const * const pCmdStr)
{
  if (pCmdStr == nullptr)
    throw std::invalid_argument("CLI::RemoveCommand: !pCmdStr");

  if ((strcmp(pCmdStr, "help") == 0) ||
      (strcmp(pCmdStr, "logout") == 0))
    throw std::logic_error("CLI::RemoveCommand: Attempt to remove built-in command");

  osal::MutexLocker mutexLocker(cmdMutex);

  // locate command
  Command * pPrev = nullptr;
  Command * pCurr = pCMDListHead;
  while (pCurr != nullptr)
  {
    // match?
    if (strcmp(pCurr->GetCommand(), pCmdStr) == 0)
    {
      // wait until execution of the command has finished
      if (pCurr == pCurrExecCmd)
      {
        if (thread.IsItMe())
          throw std::runtime_error("CLI::RemoveCommand: Command attempted to remove itself");

        do
        {
          cv_pCurrExecCmd_IsNull.Wait(cmdMutex);
        }
        while (pCurrExecCmd == pCurr);
      }

      RemoveFromSuggestionList(pCurr);

      // remove from list of registered commands
      if (pPrev == nullptr)
        pCMDListHead = pCurr->pNext;
      else
        pPrev->pNext = pCurr->pNext;

      // that's it
      delete pCurr;
      return;
    }

    // move to next list element for next loop cycle
    pPrev = pCurr;
    pCurr = pCurr->pNext;
  }

  // if we reach this, then there was no match
}

/**
 * \brief Configures the password required for users to login into the command prompt.
 *
 * __Note:__\n
 * If a password is required right from the beginning, then the password __must__ be setup
 * __before__ the CLI component is started for the first time. Otherwise it might be possible
 * to login and execute commands before the password is setup.
 *
 * If the password shall be changed, removed, or setup while the CLI component is running,
 * then there is nothing special to consider.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newPassword
 * New password required for login into the command prompt.\n
 * If the string is empty, then no password will be required for login.\n
 * There must be no leading or trailing whitespaces.
 */
void CLI::SetPassword(std::string const & newPassword)
{
  // check new password
  if (newPassword.length() != 0)
  {
    if ((newPassword.front() == ' ') || (newPassword.back() == ' '))
      throw std::invalid_argument("CLI::SetPassword: newPassword invalid");
  }

  // set net password
  osal::MutexLocker mutexLocker(terminalMutex);
  password = newPassword;
}

/**
 * \brief Retrieves the currently configured password required to login into the command prompt.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * std::string containing the currently configured password required for login into the command prompt.\n
 * If the string is empty, then no password is required for login.
 */
std::string CLI::GetPassword(void) const
{
  osal::MutexLocker mutexLocker(terminalMutex);
  return password;
}

/**
 * \brief Updates the line head (e.g. '>') used for the command prompt.
 *
 * The command prompt is displayed on the terminal when no command callback is currently executed
 * and when the user is logged in.
 *
 * This method has no effect on the line head used when @ref ReadLine() is invoked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newConsoleLineHead
 * New line head for the command prompt, e.g. '>'.\n
 * __Note:__\n
 * The change does not immediately become visible on the terminal's screen!\n
 * Instead the new line head becomes visible either __after__ ENTER has been pressed by the user
 * or after some text has been printed via @ref WriteLine().
 *
 * Constraints:
 * - The line head must be shorter than `CLI::terminalWidth` (has been passed to the constructor) minus one.
 * - The line head must be comprised of at least one character.
 * - At least one character of the line head must not be a white space.
 */
void CLI::SetLineHead(std::string const & newConsoleLineHead)
{
  if ((newConsoleLineHead.length() == 0) ||
      (newConsoleLineHead.length() > terminalWidth - 2U))
  {
    throw std::invalid_argument("CLI::SetLineHead: Zero length or too long");
  }

  if (string::CountChar(newConsoleLineHead, ' ') == newConsoleLineHead.length())
    throw std::invalid_argument("CLI::SetLineHead: Only white spaces");

  osal::MutexLocker mutexLocker(terminalMutex);
  commandLineHead = newConsoleLineHead;
}

/**
 * \brief Writes a line (composed of multiple null-terminated c-strings) synchronously to the terminal WITHOUT
 * disturbing potential user input.
 *
 * If an input prompt is currently displayed to the user, then the line will be printed above
 * the bottom line containing the cursor and the input prompt (and potential user input).
 *
 * This method can be used as an alternative if otherwise multiple `std::string` objects had to be concatenated:
 * ~~~{.cpp}
 * #include "gpcc/src/cli/CLIColors.hpp"
 * #include <string>
 *
 * // [...]
 *
 * std::string const str = FunctionProducingLotOfText();
 *
 * char const * fragments[5];
 * fragments[0] = CLI_RED;
 * fragments[1] = "Part1 ";
 * fragments[2] = "Part2 ";
 * fragments[3] = str.c_str();
 * fragments[4] = nullptr;
 *
 * myCLI.WriteLineComposed(fragments);
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param fragments
 * Array of pointers to null-terminated c-strings that shall be printed to the terminal's screen.\n
 * The single c-strings are printed seamlessly to the terminal.\n
 * The last entry in the array must be a nullptr to indicate the end of the array.\n
 * Note: A '\\n' is automatically appended by this method behind the last string.\n
 * The c-strings may contain any font/color/style control information (see @ref GPC_CLI_FCS).\n
 * Class @ref CLI guarantees, that the terminal has received an @ref CLI_STD before the first string is printed.
 */
void CLI::WriteLineComposed(char const * const fragments[])
{
  char const * const * p = fragments;
  if (p == nullptr)
    throw std::invalid_argument("CLI::WriteLineComposed: 'fragments' is nullptr");

  osal::MutexLocker mutexLocker(terminalMutex);

  if (recoveryRequired)
  {
    // remove line
    Terminal_MoveCursor(-(currentLineHead.length() + cursorX));
    Terminal_DeleteChars(currentLineHead.length() + inputBuffer.length());

    // print
    while (*p != nullptr)
    {
      Terminal_Write(*p++);
    }
    Terminal_Write(CLI_STD "\n");

    // recover line and cursor position
    Terminal_Write(currentLineHead.c_str());
    Terminal_Write(inputBuffer.c_str());
    Terminal_MoveCursor(-(inputBuffer.length() - cursorX));
  }
  else
  {
    // no recovery needed, just print s
    while (*p != nullptr)
    {
      Terminal_Write(*p++);
    }
    Terminal_Write(CLI_STD "\n");
  }
}

/**
 * \brief Writes a line (given as null-terminated c-string) synchronously to the terminal WITHOUT
 * disturbing potential user input.
 *
 * If an input prompt is currently displayed to the user, then the line will be printed above
 * the bottom line containing the cursor and the input prompt (and potential user input).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param s
 * Pointer to a null-terminated c-string that shall be printed to the terminal's screen.\n
 * nullptr is not allowed.\n
 * Note: A '\\n' is automatically appended by this method.\n
 * The c-string may contain any font/color/style control information (see @ref GPC_CLI_FCS).\n
 * Class @ref CLI guarantees, that the terminal has received an @ref CLI_STD before the text is printed.
 */
void CLI::WriteLine(char const * const s)
{
  if (s == nullptr)
    throw std::invalid_argument("CLI::WriteLine: 's' is nullptr");

  char const * lines[2];
  lines[0] = s;
  lines[1] = nullptr;
  WriteLineComposed(lines);
}

/**
 * \brief Writes a line (given as std::string) synchronously to the terminal WITHOUT
 * disturbing potential user input.
 *
 * If an input prompt is currently displayed to the user, then the line will be printed above
 * the bottom line containing the cursor and the input prompt (and potential user input).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to an std::string containing the text that shall be printed to the
 * terminal's screen.\n
 * Note: A '\\n' is automatically appended by this method.\n
 * The string may contain any font/color/style control information (see @ref GPC_CLI_FCS).\n
 * Class @ref CLI guarantees, that the terminal has received an @ref CLI_STD before the text is printed.
 */
void CLI::WriteLine(std::string const & s)
{
  WriteLine(s.c_str());
}

/**
 * \brief Displays a prompt to the user and reads the line of text entered by the user from the terminal.
 *
 * This is intended to be invoked from the callback of an CLI command (class @ref Command) only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This must be executed in the context of the CLI's thread only, e.g.
 * from the callback of an CLI command (class @ref Command).
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * Internal runtime errors and errors that occur during communication with the terminal
 * are handled internally by this method (-> error message printed to terminal and retry).
 *
 * _This method only throws if it is invoked by a non-CLI thread or if an error_
 * _occurs during copy/move of the returned std::string object, or if any of_
 * _the special exceptions pointed out below occur._
 *
 * \throws CLIStopError  Stop of CLI component has been requested ([details](@ref CLIStopError)).
 * \throws CtrlCError    User has hit CTRL+C ([details](@ref CtrlCError)).
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * This is guaranteed, because only the CLI's own thread is allowed to invoke this
 * method and because the identity of the calling thread is checked by this method.
 *
 * - - -
 *
 * \param lineHead
 * Unmodifiable reference to an std::string containing the line head that shall be printed
 * in front of the cursor, e.g. "Enter name: ". The length of the referenced string must be
 * less than the width of the terminal (passed to the constructor) minus one.
 * \return
 * std::string containing the input entered by the user.
 */
std::string CLI::ReadLine(std::string const & lineHead)
{
  // make sure that this is executed in the context of the CLI's thread
  // (note: CLI's thread runs with deferred cancellation disabled)
  if (!thread.IsItMe())
    throw std::runtime_error("CLI::ReadLine: Executed by non-CLI thread");

  RobustEnterLine(false, lineHead);

  // Only the CLI's thread can modify "inputBuffer", so for read-access the mutex is not
  // required here because this is always executed in the context of the CLI's thread.
  return std::move(inputBuffer);
}

/**
 * \brief Checks if stop of the CLI component has been requested or if the user has pressed CTRL+C.
 *
 * This method is intended to be invoked by the callback of time-consuming CLI commands (class @ref Command)
 * to check if stop of the CLI component has been requested, or if the user has pressed CTRL+C.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This must be executed in the context of the CLI's thread only, e.g.
 * from the callback of an CLI command (class @ref Command).
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws CLIStopError  Stop of CLI component has been requested ([details](@ref CLIStopError)).
 * \throws CtrlCError    User has hit CTRL+C ([details](@ref CtrlCError)).
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * This is guaranteed, because only the CLI's own thread is allowed to invoke this
 * method and because the identity of the calling thread is checked by this method.
 */
void CLI::TestTermination(void)
{
  // make sure that this is executed in the context of the CLI's thread
  // (note: CLI's thread runs with deferred cancellation disabled)
  if (!thread.IsItMe())
    throw std::runtime_error("CLI::TestTermination: Executed by non-CLI thread");

  // CLI::Stop() called and waiting for thread to terminate?
  if (thread.IsCancellationPending())
    throw CLIStopError();

  // flush any user input and check if an ETX (CTRL+C) is there...
  char c = 0;
  while (terminal.Read(&c, 1, 0) != 0)
  {
    if (c == 0x03U)
      throw CtrlCError();
  }
}

/**
 * \brief Writes a null-terminated c-string synchronously to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param s
 * Null-terminated c-string that shall be send to the terminal.
 */
void CLI::Terminal_Write(char const * const s)
{
  try
  {
    terminal.Write(s, strlen(s));
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(TerminalOutputError());
  }
}

/**
 * \brief Writes an std::string synchronously to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to an std::string that shall be send to the terminal.
 */
void CLI::Terminal_Write(std::string const & s)
{
  try
  {
    terminal.Write(s.c_str(), s.length());
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(TerminalOutputError());
  }
}

/**
 * \brief Writes a single character synchronously to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param c
 * Character that shall be send to the terminal.
 */
void CLI::Terminal_Write(char const c)
{
  try
  {
    terminal.Write(&c, 1U);
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(TerminalOutputError());
  }
}

/**
 * \brief Moves the cursor on the terminal's screen to the left or right by `deltaX` characters.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param deltaX
 * Number of characters the cursor shall be moved. Allowed range: -9999..+9999.\n
 * Negative values move the cursor to the left, positive values move the cursor to the right.\n
 * If this is zero, then this method does nothing.
 */
void CLI::Terminal_MoveCursor(int16_t deltaX)
{
  if ((deltaX < -9999) || (deltaX > 9999))
    throw std::invalid_argument("CLI::Terminal_MoveCursor: deltaX invalid");

  if (deltaX != 0)
  {
    // determine the last character of the ESC sequence and make deltaX absolute
    char lastChar;
    if (deltaX < 0)
    {
      lastChar = 'D';
      deltaX = -deltaX;
    }
    else
      lastChar = 'C';

    // build and transmit command
    char str[8];
    int const result = snprintf(str, sizeof(str), "\x1B[%d%c", static_cast<int>(deltaX), lastChar);
    if ((result < 0) || (static_cast<size_t>(result) >= sizeof(str)))
      PANIC();
    Terminal_Write(str);
  } // if (deltaX != 0)
}

/**
 * \brief Deletes `n` characters on the terminal's screen starting at the current cursor position.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param n
 * Number of characters that shall be deleted.\n
 * If this is zero, then this method does nothing.
 */
void CLI::Terminal_DeleteChars(uint8_t const n)
{
  if (n != 0)
  {
    char str[8];
    int const status = snprintf(str, sizeof(str), "\x1B[%uP", static_cast<unsigned int>(n));
    if ((status < 0) || (static_cast<size_t>(status) >= sizeof(str)))
      PANIC();
    Terminal_Write(str);
  }
}

/**
 * \brief Clears @ref inputBuffer, sets @ref cursorX to zero, and updates the terminal's screen.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 */
void CLI::ClearInputBuffer(void)
{
  auto const prevCursorX = cursorX;
  auto const prevInputBufferLength = inputBuffer.length();

  inputBuffer.resize(0);
  cursorX = 0;

  Terminal_MoveCursor(-prevCursorX);
  Terminal_DeleteChars(prevInputBufferLength);
}

/**
 * \brief Replaces the content of @ref inputBuffer with a given string, updates @ref cursorX to
 * the end of the given string, and updates the terminal's screen.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param newInputBuffer
 * Unmodifiable reference to an string that shall be assigned to @ref inputBuffer.
 * \param maxLength
 * Maximum number of characters that are allowed to be loaded into @ref inputBuffer.
 * If `newInputBuffer` is larger than this, then @ref inputBuffer will be truncated.
 */
void CLI::ReplaceInputBufferWithString(std::string const & newInputBuffer, size_t const maxLength)
{
  auto const prevCursorX = cursorX;
  auto const prevInputBufferLength = inputBuffer.length();

  if (newInputBuffer.length() <= maxLength)
    inputBuffer = newInputBuffer;
  else
    inputBuffer = newInputBuffer.substr(0, maxLength);
  cursorX = inputBuffer.length();

  Terminal_MoveCursor(-prevCursorX);
  Terminal_DeleteChars(prevInputBufferLength);
  Terminal_Write(inputBuffer);
}

/**
 * \brief Replaces the content of @ref inputBuffer with an entry from the command history,
 * updates @ref cursorX to the end of the line, and updates the terminal's screen.
 *
 * - - -
 *
 * __Thread safety:__
 * - @ref terminalMutex must be locked.
 * - This must be executed in the context of the CLI's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * \throws TerminalOutputError   Terminal's interface has thrown an exception. It is nested to this exception.
 *                               ([details](@ref TerminalOutputError))
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param idx
 * Index of the command history entry.
 * \param maxLength
 * Maximum number of characters that are allowed to be loaded into @ref inputBuffer. \n
 * If the content of the command history entry is larger, then it will be truncated.
 */
void CLI::ReplaceInputBufferWithHistory(uint8_t const idx, size_t const maxLength)
{
  if (idx >= history.size())
    throw std::invalid_argument("CLI::ReplaceInputBufferWithHistory: idx");

  ReplaceInputBufferWithString(history[idx], maxLength);
}

/**
 * \brief Adds the content of @ref inputBuffer to the back of @ref history.
 *
 * If there is already an entry in @ref history which matches @ref inputBuffer, then the
 * existing entry will be moved to the back of @ref history. \n
 * (Remember: The latest entered command is located at the back of @ref history).
 *
 * - - -
 *
 * __Thread safety:__
 * - @ref terminalMutex must be locked.
 * - This must be executed in the context of the CLI's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The command history is in a valid and defined state, but one entry might be missing
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void CLI::AddInputBufferToHistory(void)
{
  // Check if the content of "inputBuffer" is already in the command history.
  // If so, then move it to the end where the latest executed command resides.
  for (size_t i = 0; i < history.size(); i++)
  {
    if (history[i] == inputBuffer)
    {
      // not the last entry?
      if (i != history.size() - 1U)
      {
        std::string temp(history[i]);
        history.erase(history.begin() + i);
        history.push_back(std::move(temp));
      }
      return;
    }
  }

  // The content of "inputBuffer" is not yet in the command history.
  // Add it. If the history is full, then the oldest entry will be removed.
  if (history.size() == historyDepth)
    history.erase(history.begin());
  history.push_back(inputBuffer);
}

/**
 * \brief Calculates the Levenshtein-distances between @ref inputBuffer and all registered
 * commands and builds the list of suggested commands (@ref suggestions).
 *
 * - - -
 *
 * __Thread safety:__
 * - @ref terminalMutex must be locked
 * - @ref cmdMutex must be locked
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @ref suggestionsValid is guaranteed to be "false"
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:\n
 * - @ref suggestionsValid is guaranteed to be "false"
 */
void CLI::BuildListOfSuggestions(void)
{
  suggestionsValid = false;
  suggestions.clear();

  // Loop through all registered commands and calculate the Levenshtein-distance to the user's input and
  // insert the commands ordered by the Levenshtein-distance in ascending order into the list of suggestions.
  Command * pCommand = pCMDListHead;
  while (pCommand != nullptr)
  {
    auto distance = string::LevenshteinDistance(inputBuffer, pCommand->GetCommand(), false);
    if (distance > 255U)
      distance = 255U;
    pCommand->levenshteinDistance = distance;

    InsertIntoSuggestionList(pCommand);

    pCommand = pCommand->pNext;
  }

  suggestionIterator = suggestions.begin();
  suggestionsValid = true;
}

/**
 * \brief Helper method used to fill the list of suggested commands (@ref suggestions).
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref cmdMutex must be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pCommand
 * Pointer to the @ref Command object that shall be inserted into the list of suggested commands.\n
 * The list is sorted by the Levenshtein-distance in ascending order.\n
 * To achieve an additional alphabetical sorting for entries with the same Levenshtein-
 * distance, this method should be called with the commands in alphabetical order.
 */
void CLI::InsertIntoSuggestionList(Command const * const pCommand)
{
  for (auto it = suggestions.begin(); it != suggestions.end(); ++it)
  {
    if ((*it)->levenshteinDistance > pCommand->levenshteinDistance)
    {
      suggestions.insert(it, pCommand);
      if (suggestions.size() > maximumSuggestions)
        suggestions.pop_back();
      return;
    }
  }

  if (suggestions.size() < maximumSuggestions)
    suggestions.push_back(pCommand);
}

/**
 * \brief Removes an command from the list of suggestions (@ref suggestions) and updates
 * @ref suggestionIterator if necessary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref cmdMutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pCommand
 * Pointer to the @ref Command object that shall be removed from the list of suggestions.
 */
void CLI::RemoveFromSuggestionList(Command const * const pCommand) noexcept
{
  if (!suggestionsValid)
    return;

  auto itPrev = suggestions.begin();
  for (auto it = suggestions.begin(); it != suggestions.end(); ++it)
  {
    // match?
    if ((*it) == pCommand)
    {
      // do we need to update 'suggestionIterator'?
      if (it == suggestionIterator)
      {
        // is 'it' referencing to the first entry in the list?
        if (itPrev == it)
          suggestionIterator = suggestions.end();
        else
          suggestionIterator = itPrev;
      }

      // remove element from list
      suggestions.erase(it);

      // is the list empty now?
      if (suggestions.empty())
        suggestionsValid = false;

      return;
    } // if ((*it) == pCommand)

    itPrev = it;
  } // (for ...)
}

/**
 * \brief Requests the user to enter one line into the terminal (without runtime error handling).
 *
 * This returns after the user has pressed the ENTER key. After returning, the user's
 * input is located in @ref inputBuffer.
 *
 * The only alternate way to leave this method is via an exception (see below).
 *
 * This method is not intended to be used directly. Instead @ref RobustEnterLine() shall
 * be used which implements a retry mechanism for handling runtime errors like @ref TerminalOutputError
 * and `std::bad_alloc`.
 *
 * - - -
 *
 * __Thread safety:__
 * - This must be executed in the context of the CLI's thread only.
 * - @ref terminalMutex must NOT be locked.
 * - @ref cmdMutex must NOT be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen may be undefined (except for @ref CLIStopError and @ref CtrlCError)
 * - content of @ref inputBuffer may be undefined (except for @ref CLIStopError and @ref CtrlCError)
 *
 * \throws CLIStopError     Stop of CLI component has been requested ([details](@ref CLIStopError)).
 * \throws CtrlCError       User has hit CTRL+C ([details](@ref CtrlCError)).
 * \throws std::bad_alloc   Out of memory.
 *
 * After any exception, it is safe to invoke this method again. However, a "\n" should
 * be printed to the terminal in order to start on a new clean line.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param useHistory
 * true  = command history (arrow up/down keys) and word completion (tab key) are enabled\n
 * false = command history (arrow up/down keys) and word completion (tab key) are disabled
 * \param lineHead
 * Line head displayed in front of the cursor, e.g. "Enter name: " or ">".\n
 * The length of this must be less than @ref terminalWidth minus one.
 */
void CLI::EnterLine(bool const useHistory, std::string const & lineHead)
{
  if (lineHead.length() > terminalWidth - 2U)
    throw std::invalid_argument("CLI::EnterLine: lineHead too long");

  // -------
  // Prepare
  // -------
  osal::AdvancedMutexLocker terminalMutexLocker(terminalMutex);

  // setup line head
  currentLineHead = lineHead;
  Terminal_Write(currentLineHead);

  // setup input buffer and related stuff
  uint8_t const maxInputLength = (terminalWidth - 1U) - currentLineHead.length();

  inputBuffer.resize(0);
  inputBuffer.reserve(maxInputLength);
  cursorX = 0;
  recoveryRequired = true;

  terminalMutexLocker.Unlock();

  ON_SCOPE_EXIT(clrRecoveryRequired)
  {
    if (!terminalMutexLocker.IsLocked())
      terminalMutexLocker.Relock();
    recoveryRequired = false;
  };

  // Storage for backup of input buffer used to recover user input after iterating the command history or
  // the list of suggestions
  std::string inputBufferBackup;

  // setup command history stuff
  bool iteratingHistory = false;
  size_t historyIdx = 0;

  // setup suggestion-related stuff
  osal::AdvancedMutexLocker cmdMutexLocker(cmdMutex);
  suggestionsValid = false;
  cmdMutexLocker.Unlock();

  // ------------------------------------------------------------------------------------------
  // Loop until any of the following events occurs:
  // - ENTER is pressed by user
  // - CTRL+C is pressed by user
  // - Stop of CLI is requested
  // ------------------------------------------------------------------------------------------
  terminal.Flush();
  rxParser.Clear();
  char c = 0;
  while (true)
  {
    // no re-insertion into rx parser? Then read from terminal
    if (c == 0)
    {
      // loop until a byte (that is not 0x00) is received or thread cancellation is requested
      do
      {
        // leave if thread cancellation is requested
        if (thread.IsCancellationPending())
        {
          terminalMutexLocker.Relock();
          Terminal_Write('\n');
          throw CLIStopError();
        }
      }
      while ((terminal.Read(&c, 1U, terminalReadTimeout_ms) == 0) || (c == 0));
    }

    // pass received or re-inserted byte to the VT100 parser
    auto const result = rxParser.Input(c);
    c = 0;

    // check if RETURN-key has been pressed
    bool const returnKey = returnKeyFilter.Filter(result);

    terminalMutexLocker.Relock();
    cmdMutexLocker.Relock();

    // If the current content of "inputBuffer" is a copy from an entry of the command history and if "inputBuffer"
    // is about to be changed (valid data has been received from the terminal but it is not a RETURN, CURSOR-UP,
    // or CURSOR-DOWN), then we must reserve sufficient space in "inputBuffer" to prevent frequent reallocations if
    // characters are appended to "inputBuffer".
    // At the same time we are no longer iterating through the command history.
    if ((iteratingHistory) &&
        (!returnKey) &&
        (result != TerminalRxParser::Result::NeedMoreData) &&
        (result != TerminalRxParser::Result::ArrowUp) &&
        (result != TerminalRxParser::Result::ArrowDown))
    {
      iteratingHistory = false;
      inputBufferBackup.clear();
      inputBuffer.reserve(maxInputLength);
    }

    // If the current content of "inputBuffer" is a copy from an entry of the list of suggested commands and if
    // "inputBuffer" is about to be changed (valid data has been received from the terminal but it is not a RETURN
    // or TAB), then we must reserve sufficient space in "inputBuffer" to prevent frequent reallocations if characters
    // are appended to "inputBuffer".
    // At the same time we are no longer iterating through the list of command suggestions.
    if ((suggestionsValid) &&
        (!returnKey) &&
        (result != TerminalRxParser::Result::NeedMoreData) &&
        (result != TerminalRxParser::Result::Tab))
    {
      suggestionsValid = false;
      inputBufferBackup.clear();
      inputBuffer.reserve(maxInputLength);
    }

    cmdMutexLocker.Unlock();

    // examine the stuff received from the terminal
    switch (result)
    {
      case TerminalRxParser::Result::Backspace:
      {
        // any characters available that could be deleted?
        if ((inputBuffer.length() > 0U) && (cursorX > 0U))
        {
          if (cursorX == inputBuffer.length())
          {
            // (delete at end of line)

            inputBuffer.pop_back();
            cursorX--;
            Terminal_MoveCursor(-1);
            Terminal_DeleteChars(1U);
          }
          else
          {
            // (delete somewhere inside the line)

            cursorX--;
            inputBuffer.erase(cursorX, 1U);
            Terminal_MoveCursor(-1);
            Terminal_Write(inputBuffer.c_str() + cursorX);
            Terminal_DeleteChars(1U);
            Terminal_MoveCursor(-(inputBuffer.length() - cursorX));
          }
        } // if ((inputBuffer.length() > 0U) && (cursorX > 0U))
        break;
      }

      case TerminalRxParser::Result::Tab:
      {
        if (useHistory)
        {
          cmdMutexLocker.Relock();
          if (pCMDListHead != nullptr)
          {
            if (!suggestionsValid)
            {
              inputBufferBackup = inputBuffer;
              BuildListOfSuggestions();
            }
            else
            {
              if (suggestionIterator == suggestions.end())
                suggestionIterator = suggestions.begin();
              else
                ++suggestionIterator;
            }

            if (suggestionIterator == suggestions.end())
              ReplaceInputBufferWithString(inputBufferBackup, maxInputLength);
            else
              ReplaceInputBufferWithString((*suggestionIterator)->GetCommand(), maxInputLength);
          } // if (pCMDListHead != nullptr)
          cmdMutexLocker.Unlock();
        } // if (useHistory)
        break;
      }

      case TerminalRxParser::Result::ArrowLeft:
      {
        if (cursorX > 0U)
        {
          cursorX--;
          Terminal_MoveCursor(-1);
        }
        break;
      }

      case TerminalRxParser::Result::ArrowRight:
      {
        if (cursorX < inputBuffer.length())
        {
          cursorX++;
          Terminal_MoveCursor(1U);
        }
        break;
      }

      case TerminalRxParser::Result::ArrowUp: // (toward older commands)
      {
        if ((useHistory) && (!history.empty()))
        {
          if (!iteratingHistory)
          {
            // start at latest entered command from command history
            inputBufferBackup = inputBuffer;
            historyIdx = history.size() - 1U;

            ReplaceInputBufferWithHistory(historyIdx, maxInputLength);
            iteratingHistory = true;
          }
          else
          {
            // move to next older command in command history
            if (historyIdx == 0)
              historyIdx = history.size() - 1U;
            else
              historyIdx--;

            ReplaceInputBufferWithHistory(historyIdx, maxInputLength);
          }
        }
        break;
      }

      case TerminalRxParser::Result::ArrowDown: // (toward latest entered command)
      {
        if (iteratingHistory)
        {
          historyIdx++;
          if (historyIdx >= history.size())
          {
            // restore user entry and finish command list iteration
            ReplaceInputBufferWithString(inputBufferBackup, maxInputLength);
            iteratingHistory = false;
          }
          else
          {
            ReplaceInputBufferWithHistory(historyIdx, maxInputLength);
          }
        }
        break;
      }

      case TerminalRxParser::Result::Pos1:
      {
        Terminal_MoveCursor(-cursorX);
        cursorX = 0;
        break;
      }

      case TerminalRxParser::Result::END:
      {
        Terminal_MoveCursor(inputBuffer.length() - cursorX);
        cursorX = inputBuffer.length();
        break;
      }

      case TerminalRxParser::Result::DEL:
      {
        if (cursorX < inputBuffer.length())
        {
          inputBuffer.erase(cursorX, 1U);
          Terminal_Write(inputBuffer.c_str() + cursorX);
          Terminal_DeleteChars(1U);
          Terminal_MoveCursor(-(inputBuffer.length() - cursorX));
        }
        break;
      }

      case TerminalRxParser::Result::ETX:
      {
        Terminal_Write('\n');
        throw CtrlCError();
      }

      case TerminalRxParser::Result::NoCommand:
      {
        // fetch stuff from rx parser
        rxParser.RemoveNonPrintableCharacters();
        char const * const newChars = rxParser.Output(maxInputLength - inputBuffer.length());
        size_t nbOfNewChars = strlen(newChars);

        if (nbOfNewChars != 0)
        {
          // If there is more than one character, then assign the last character to "c" for
          // re-insertion into the rx parser. The last character will not be printed to the
          // terminal now.
          if (nbOfNewChars > 1U)
          {
            c = newChars[nbOfNewChars - 1U];
            nbOfNewChars--;
          }

          auto const prevCursorX = cursorX;
          if (cursorX == inputBuffer.length())
          {
            // (append)
            inputBuffer.append(newChars, nbOfNewChars);
            cursorX += nbOfNewChars;
            Terminal_Write(inputBuffer.c_str() + prevCursorX);
          }
          else
          {
            // (insert)
            inputBuffer.insert(cursorX, newChars, nbOfNewChars);
            cursorX += nbOfNewChars;
            Terminal_Write(inputBuffer.c_str() + prevCursorX);
            Terminal_MoveCursor(-(inputBuffer.length() - cursorX));
          }
        } // if (nbOfNewChars != 0)
        break;
      }

      default:
        // all others are intentionally ignored
        break;
    } // switch (Result)

    if (result != TerminalRxParser::Result::NeedMoreData)
      rxParser.Clear();

    if (returnKey)
    {
      Terminal_Write('\n');
      break;
    }

    terminalMutexLocker.Unlock();
  } // while (true)
}

/**
 * \brief Requests the user to enter one line into the terminal (incl. runtime error handling).
 *
 * This returns after the user has pressed the ENTER key. After returning, the user's
 * input is located in @ref inputBuffer.
 *
 * The only alternate way to leave this method is via an exception (see below).
 *
 * This method shall be preferred above @ref EnterLine(), because it implements a retry mechanism
 * for handling runtime errors like @ref TerminalOutputError and `std::bad_alloc`.
 *
 * - - -
 *
 * __Thread safety:__
 * - This must be executed in the context of the CLI's thread only.
 * - @ref terminalMutex must NOT be locked.
 * - @ref cmdMutex must NOT be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - @ref inputBuffer contains the current user input from the point in time when the exception occurred
 *
 * \throws CLIStopError  Stop of CLI component has been requested ([details](@ref CLIStopError)).
 * \throws CtrlCError    User has hit CTRL+C ([details](@ref CtrlCError)).
 *
 * After any exception, it is safe to invoke this method again.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param useHistory
 * true  = command history (arrow up/down keys) and word completion (tab key) are enabled\n
 * false = command history (arrow up/down keys) and word completion (tab key) are disabled
 * \param lineHead
 * Line head displayed in front of the cursor, e.g. "Enter name: " or ">".\n
 * The length of this must be less than @ref terminalWidth minus one.
 */
void CLI::RobustEnterLine(bool const useHistory, std::string const & lineHead)
{
  while (true)
  {
    try
    {
      EnterLine(useHistory, lineHead);
      return;
    }
    catch (CLIStopError const &)
    {
      throw;
    }
    catch (CtrlCError const &)
    {
      throw;
    }
    catch (std::exception const & e)
    {
      try
      {
        osal::MutexLocker terminalMutexLocker(terminalMutex);
        Terminal_Write(CLI_STD "\nERROR IN CLI:\n");
        PrintException(e);
        Terminal_Write("\nRETRY\n");
      }
      catch (std::exception const &)
      {
        // ignore
      }

      // sleep before retry to prevent heavy CPU load in case of permanent error
      thread.Sleep_ms(1000);
    }
  } // while (true)
}

/**
 * \brief Processes the content of @ref inputBuffer after ENTER has been pressed.
 *
 * This is a helper function for @ref InternalThreadEntry().
 *
 * - - -
 *
 * __Thread safety:__
 * - This must be executed in the context of the CLI's thread only.
 * - @ref terminalMutex must NOT be locked.
 * - @ref cmdMutex must NOT be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen may be undefined
 * - @ref inputBuffer may contain undefined data
 *
 * _Any exception thrown by a potentially executed callback of a @ref Command object will be caught_
 * _here and will not be re-thrown._
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void CLI::ProcessLineEnteredIntoCLI(void)
{
  // Verify pCurrExecCmd. cmdMutex is not required, because this is a read-access and any
  // modifications done to pCurrExecCmd are only done by the CLI's thread, which is the calling thread.
  if (pCurrExecCmd != nullptr)
    PANIC();

  // ----------------------------------------------------------------------------------------------
  // Step 1:
  // - Look for a Command object (in cmdList) which matches inputBuffer
  // - set pCurrExecCmd
  // - setup restOfLine
  // ----------------------------------------------------------------------------------------------
  std::string restOfLine;

  {
    osal::MutexLocker terminalMutexLocker(terminalMutex);

    inputBuffer = string::Trim(inputBuffer);
    inputBuffer.shrink_to_fit();

    // nothing entered, or only white spaces entered? => leave, nothing to do
    if (inputBuffer.length() == 0)
      return;

    AddInputBufferToHistory();

    osal::AdvancedMutexLocker cmdMutexLocker(cmdMutex);
    Command* pCommand = pCMDListHead;
    while (pCommand != nullptr)
    {
      char const * const command = pCommand->GetCommand();

      if (string::StartsWith(inputBuffer, command))
      {
        size_t const len = strlen(command);

        // exact match of length or a white space behind the command inside "inputBuffer"?
        if ((inputBuffer.length() == len) || (inputBuffer[len] == ' '))
        {
          // match!
          if (inputBuffer.length() > len)
            restOfLine = inputBuffer.substr(len + 1U);
          pCurrExecCmd = pCommand;
          break;
        }
      }

      pCommand = pCommand->pNext;
    }
    cmdMutexLocker.Unlock();

    // no registered command found?
    // (cmdMutex is not required, because only the CLI's thread (this one here) modifies pCurrExecCmd)
    if (pCurrExecCmd == nullptr)
    {
      Terminal_Write("Unknown command! Enter 'help'!\n");
      return;
    }
  }

  // ----------------------------------------------------------------------------------------------
  // Step 2:
  // Execute command
  // ----------------------------------------------------------------------------------------------
  // (cmdMutex is not required for read-access to pCurrExecCmd, because only the CLI's thread (this one here)
  // modifies pCurrExecCmd)

  ON_SCOPE_EXIT(clear_pCurrExecCmd)
  {
    osal::MutexLocker cmdMutexLocker(cmdMutex);
    pCurrExecCmd = nullptr;
    cv_pCurrExecCmd_IsNull.Signal();
  };

  // help for entered command requested?
  if ((restOfLine == "help") ||
      (restOfLine == "-help") ||
      (restOfLine == "--help"))
  {
    // print help
    osal::MutexLocker terminalMutexLocker(terminalMutex);
    Terminal_Write(pCurrExecCmd->GetCommand());
    Terminal_Write(pCurrExecCmd->GetHelpText());
    Terminal_Write('\n');
  }
  else
  {
    // execute command
    try
    {
      pCurrExecCmd->Entered(restOfLine, *this);
    }
    catch (CLIStopError const &)
    {
      throw;
    }
    catch (CtrlCError const &)
    {
      WriteLine("Aborted by CTRL+C");
    }
    catch (UserEnteredInvalidArgsError const & e)
    {
      osal::MutexLocker terminalMutexLocker(terminalMutex);
      Terminal_Write(CLI_STD "\nInvalid arguments. Try '");
      Terminal_Write(pCurrExecCmd->GetCommand());
      Terminal_Write(" help'.\n");
      if (!e.details.GetStr().empty())
      {
        Terminal_Write(e.details.GetStr());
        Terminal_Write('\n');
      }
      else
      {
        Terminal_Write("Details:\n");
        PrintException(e);
      }
    }
    catch (std::exception const &e)
    {
      osal::MutexLocker terminalMutexLocker(terminalMutex);
      Terminal_Write(CLI_STD "\nError! Caught an exception:\n");
      PrintException(e);
    }
  } // if (restOfLine == "help")... else...
}

/**
 * \brief Presents the login prompt on the terminal. This does not return until either the user has
 * entered the correct password/login or until stop of the CLI component has been requested.
 *
 * - - -
 *
 * __Thread safety:__
 * - This must be executed in the context of the CLI's thread only.
 * - @ref terminalMutex must NOT be locked.
 * - @ref cmdMutex must NOT be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen may be undefined
 *
 * \throws CLIStopError   Stop of CLI component has been requested ([details](@ref CLIStopError)).
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void CLI::LogIn(void)
{
  bool exit;
  do
  {
    try
    {
      exit = false;

      // show password prompt
      if (pICLINotifiable != nullptr)
        pICLINotifiable->OnBeforePasswordPrompt(*this);

      RobustEnterLine(false, "Type 'login' or password>");

      // examine password and user's input
      bool passwordSetup;
      {
        osal::MutexLocker terminalMutexLocker(terminalMutex);

        passwordSetup = (password.length() != 0);

        if (passwordSetup)
          exit = (inputBuffer == password);
        else
          exit = (inputBuffer == "login");
      }

      // process result of examination
      if (exit)
      {
        if (pICLINotifiable == nullptr)
          WriteLine("Welcome. Type 'help' for assistance.");
        else
          pICLINotifiable->OnLogin(*this);
      }
      else
      {
        if (passwordSetup)
        {
          WriteLine("Wrong password.");

          if (pICLINotifiable != nullptr)
            pICLINotifiable->OnWrongPasswordEntered(*this);
        }

        gpcc::osal::Thread::Sleep_ms(1000);
      }
    }
    catch (CLIStopError const &)
    {
      throw;
    }
    catch (std::exception const &)
    {
      // (note: std::exception includes CtrlCError)
      exit = false;
      gpcc::osal::Thread::Sleep_ms(1000);
    }
  }
  while (!exit);
}

/**
 * \brief Entry function for the CLI's thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be executed by the CLI's thread only.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation will be disabled by this method.
 *
 * - - -
 *
 * \return
 * Always nullptr.
 */
void* CLI::InternalThreadEntry(void)
{
  thread.SetCancelabilityEnabled(false);

  loggedIn = false;

  while (!thread.IsCancellationPending())
  {
    try
    {
      try
      {
        if (!loggedIn)
        {
          LogIn();
          loggedIn = true;
        }

        osal::AdvancedMutexLocker terminalMutexLocker(terminalMutex);
        std::string const copyOfConsoleLineHead = commandLineHead;
        terminalMutexLocker.Unlock();

        RobustEnterLine(true, copyOfConsoleLineHead);
        ProcessLineEnteredIntoCLI();
      }
      catch (CLIStopError const &)
      {
        return nullptr;
      }
      catch (CtrlCError const &)
      {
        if (pICLINotifiable != nullptr)
          pICLINotifiable->OnCTRL_C(*this);
        else
          WriteLine("CTRL+C ignored");
      }
    }
    catch (std::exception const & e)
    {
      try
      {
        osal::MutexLocker terminalMutexLocker(terminalMutex);
        Terminal_Write(CLI_STD "\nERROR IN CLI:\n");
        PrintException(e);
        Terminal_Write("\nRETRY\n");
      }
      catch (std::exception const &)
      {
        // ignore
      }

      // sleep before retry to prevent heavy CPU load in case of permanent error
      thread.Sleep_ms(1000);
    }
  } // while (!thread.IsCancellationPending())

   return nullptr;
}

/**
 * \brief Callback for build-in CLI command "help".
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 *
 * - - -
 *
 * \param restOfLine
 * Parameters entered behind the command. Ignored.
 * \param cli
 * Reference to the @ref CLI instance into which the command has been entered. Ignored.
 */
void CLI::CCMD_help(std::string const & restOfLine, CLI & cli)
{
  (void)restOfLine;
  (void)cli;

  uint16_t x = 0;
  bool first = true;

  osal::MutexLocker terminalMutexLocker(terminalMutex);

  Terminal_Write("Help:\n"\
                 "========================================================================\n"\
                 "This is a command line interface compatible with most terminal programs.\n"\
                 "For PuTTY, choose the following settings:\n"\
                 "- Terminal->Keyboard->Function Keys: \"ESC[n~\" or \"Linux\"\n"\
                 "- Terminal: Set checkbox \"Implicit CR in every LF\"\n"\
                 "\n"\
                 "Implemented commands:\n"\
                 "=====================\n");

  osal::MutexLocker cmdMutexLocker(cmdMutex);

  Command* pCommand = pCMDListHead;
  while (pCommand != nullptr)
  {
    // get a pointer to the command string and determine the length of the string
    char const * const pCommandStr = pCommand->GetCommand();
    size_t const len = strlen(pCommandStr);

    // if this is not the first displayed command, then print a ", " first.
    if (first)
      first = false;
    else
    {
      Terminal_Write(", ");
      x += 2U;
    }

    // if the command plus ", " and an "\n" does not fit onto the line, then start a new line
    if (x + len > terminalWidth - 3U)
    {
      if (x == 0)
        throw std::runtime_error("CLI::CCMD_help: Command string too large");

      Terminal_Write('\n');
      x = 0;
    }

    Terminal_Write(pCommandStr);
    x += len;

    pCommand = pCommand->pNext;
  }

  Terminal_Write("\n\n"\
                 "Some commands require parameters. For details about a command, enter the\n"\
                 "command plus \"help\". Example: \"HeapStat help\".\n");
}

/**
 * \brief Callback for built-in CLI command "logout".
 *
 * - - -
 *
 * __Thread safety:__\n
 * This must be executed by CLI's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Incomplete/undefined output may be printed to the terminal
 * - Logout will always take place, even in case of an exception
 *
 * __Thread cancellation safety:__\n
 * This method will be invoked with deferred thread cancellation disabled.
 *
 * - - -
 *
 * \param restOfLine
 * Parameters entered behind the command. Ignored.
 * \param cli
 * Reference to the @ref CLI instance into which the command has been entered. Ignored.
 */
void CLI::CCMD_logout(std::string const & restOfLine, CLI & cli)
{
  (void)restOfLine;
  (void)cli;

  loggedIn = false;

  if (pICLINotifiable != nullptr)
    pICLINotifiable->OnLogout(*this);
}

/**
 * \brief Prints the what()-text of an exception and of all nested exceptions to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref terminalMutex must be locked.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 * - text output may be incomplete
 *
 * Any exception thrown by the output function used to write to the terminal
 * will be translated to a nested @ref TerminalOutputError exception.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - content of terminal's screen is undefined
 * - text output may be incomplete
 *
 * - - -
 *
 * \param e
 * Exception to be printed.
 * \param level
 * Current nesting level.
 */
void CLI::PrintException(std::exception const & e, size_t const level)
{
  // scope used to safe stack during recursive calls
  {
    char str[6];
    int const result = snprintf(str, sizeof(str), "%u", static_cast<unsigned int>(level)) >= static_cast<int>(sizeof(str));
    if ((result < 0) || (static_cast<size_t>(result) >= sizeof(str)))
      PANIC();

    Terminal_Write(str);
    Terminal_Write(": ");
    Terminal_Write(e.what());
    Terminal_Write('\n');
  }

  try
  {
    std::rethrow_if_nested(e);
  }
  catch (std::exception const & e)
  {
    PrintException(e, level + 1U);
  }
  catch (...)
  {
    Terminal_Write("Unknown exception\n");
  }
}

/**
 * \brief Deletes all registered @ref Command objects.
 *
 * - - -
 *
 * __Thread safety:__
 * - @ref cmdMutex is required
 * - Constructor/destructor may invoke this without @ref cmdMutex
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void CLI::DeleteRegisteredCommands(void) noexcept
{
  while (pCMDListHead != nullptr)
  {
    Command* const pVictim = pCMDListHead;
    pCMDListHead = pVictim->pNext;
    delete pVictim;
  }
}

} // namespace cli
} // namespace gpcc
