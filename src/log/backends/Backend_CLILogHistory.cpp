/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2020, 2022 Daniel Jerolm

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

#include "Backend_CLILogHistory.hpp"
#include "gpcc/src/cli/CLI.hpp"
#include "gpcc/src/cli/CLIColors.hpp"
#include "gpcc/src/cli/Command.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include <stdexcept>
#include <vector>

namespace gpcc {
namespace log  {

/**
 * \brief Constructor.
 *
 * \post  The CLI command "LogHistory" is registered at the given CLI (if any given via _pCLI).
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
 * \param _pCLI
 * Pointer to a [CLI](@ref gpcc::cli::CLI) instance where the "LogHistory" CLI command shall be registered.\n
 * nullptr is allowed, if the "LogHistory" CLI command shall not be setup.
 *
 * \param _maxNbOfMessages
 * Maximum number of recorded log messages.\n
 * If a new message shall be recorded and this value is exceeded, then the oldest recorded log message will be removed
 * from the buffer.\n
 * Required minimum value: 1
 *
 * \param maxTotalSize
 * Maximum number of bytes of memory that shall be occupied by the text of all recorded log messages.\n
 * If a new message shall be recorded and the total number of bytes occupied by the text of all recorded log messages
 * would exceed this value, then old log messages will be removed from the buffer until the total number of occupied
 * memory bytes is equal to or below this value.\n
 * Required minimum value: 128
 */
Backend_CLILogHistory::Backend_CLILogHistory(gpcc::cli::CLI* const _pCLI, uint16_t const _maxNbOfMessages, size_t const maxTotalSize)
: Backend()
, mutex()
, pCLI(_pCLI)
, maxNbOfMessages(_maxNbOfMessages)
, nbOfDroppedMessages(0U)
, oldMessagesRemoved(false)
, remainingStorage(maxTotalSize)
, messages()
{
  if ((maxNbOfMessages == 0U) || (remainingStorage < 128U))
    throw std::invalid_argument("Backend_CLILogHistory::Backend_CLILogHistory: Invalid argument(s)");

  if (pCLI != nullptr)
  {
    pCLI->AddCommand(gpcc::cli::Command::Create("LogHistory",
                                                " [n] [clear]\n" \
                                                "Prints the latest 'n' log messages recorded in the log history to the CLI and\n" \
                                                "optionally clears the log history.\n" \
                                                "Options:\n" \
                                                "n      Number of log messages that shall be printed. If 'n' is not specified,\n" \
                                                "       then all recorded log messages will be printed.\n" \
                                                "       If 'n' is zero, then only the status of the log history buffer will\n" \
                                                "       be printed.\n" \
                                                "\n" \
                                                "clear  Clears all recorded log messages.",
                                                std::bind(&Backend_CLILogHistory::CLICMD_LogHistory,
                                                          this,
                                                          std::placeholders::_1,
                                                          std::placeholders::_2)));
  }
}

/**
 * \brief Destructor.
 *
 * \post  The "LogHistory" CLI command is unregistered (if it was registered).
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
Backend_CLILogHistory::~Backend_CLILogHistory(void)
{
  if (pCLI != nullptr)
    pCLI->RemoveCommand("LogHistory");
}

/**
 * \brief Clears the log message buffer and all additional status information.
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
 */
void Backend_CLILogHistory::Clear(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  UnprotectedClear();
}

/**
 * \brief Writes all recorded messages plus additional status information into the given IStreamWriter.
 *
 * The log messages and the additonal status information will be written using separate lines of text separated
 * by '\\n'.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Undefined or incomplete data may have been written to `output`.
 * - The log message buffer and the additional status information will not be cleared.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * However, if `output` contains a cancellation point, then the guarantee is:\n
 * Basic guarantee:
 * - Undefined or incomplete data may have been written to `output`.
 * - The log message buffer and the additional status information will not be cleared.
 *
 * - - -
 *
 * \param output
 * All recorded messages and additional status information will be written into this.\n
 * Multiple lines separated by '\\n' will be emitted.
 *
 * \param clearAfterExport
 * Controls, if the log message buffer and the additional status information shall be cleared after successful export.\n
 * Export and clear will be performed as an atomic operation.
 */
void Backend_CLILogHistory::Export(gpcc::Stream::IStreamWriter& output, bool const clearAfterExport)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (oldMessagesRemoved)
    output.Write_line("Note: At least one old log message has been removed from the buffer.");

  if (messages.empty())
  {
    output.Write_line("Log history empty.");
  }
  else
  {
    for (auto const & msg: messages)
      output.Write_line(msg.second);
  }


  if (nbOfDroppedMessages != 0U)
    output.Write_line(BuildWarningForDroppedMessages());

  if (clearAfterExport)
    UnprotectedClear();
}

// <-- Backend

/// \copydoc gpcc::log::Backend::Process
void Backend_CLILogHistory::Process(std::string const & msg, LogType const type)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  // increment counter for dropped messages if anything goes wrong
  ON_SCOPE_EXIT(incErrorCnt)
  {
    if (nbOfDroppedMessages != 255U)
      nbOfDroppedMessages++;
  };

  // remove oldest message if maximum allowed number of messages would be exceeded by recording the new message
  if (messages.size() == maxNbOfMessages)
    RemoveMessage();

  // remove old messages until "remainingStorage" is sufficient to record the new message
  RemoveMessages(msg.length());

  // enough storage available?
  if (remainingStorage >= msg.length())
  {
    // yes, record the message
    messages.emplace_back(type, msg);
    remainingStorage -= msg.length();
  }
  else
  {
    // No. The size of the message exceeds the maximum storage size configured at the constructor.
    // We trim the message and append "...".
    std::string trimmedMsg = msg.substr(0, remainingStorage - 3U) + "...";

    // quick health check
    if (remainingStorage != trimmedMsg.length())
      PANIC();

    messages.emplace_back(type, std::move(trimmedMsg));
    remainingStorage = 0U;
  }

  // success
  ON_SCOPE_EXIT_DISMISS(incErrorCnt);
}

// --> Backend

/**
 * \brief Clears the log message buffer and all additional status information.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Backend_CLILogHistory::UnprotectedClear(void) noexcept
{
  while (!messages.empty())
    RemoveMessage();

  nbOfDroppedMessages = 0U;
  oldMessagesRemoved = false;
}

/**
 * \brief Removes the oldest log message from the buffer.
 *
 * This has no effect, if there is no message in the buffer.
 *
 * \post  @ref oldMessagesRemoved will be set if a message was removed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Backend_CLILogHistory::RemoveMessage(void) noexcept
{
  if (!messages.empty())
  {
    remainingStorage += messages.front().second.size();
    messages.pop_front();
    oldMessagesRemoved = true;
  }
}

/**
 * \brief Removes old log messages from the log message buffer, until @ref remainingStorage is equal to or larger than
 *        the given parameter requiredRemainingStorage, or until the log message buffer is empty.
 *
 * This has no effect, if there is no message in the buffer.
 *
 * If parameter `requiredRemainingStorage` exceeds the maximum size passed to the constructor (parameter
 * `maxTotalSize`), then all log messages will be removed from the buffer, but @ref remainingStorage will still be less
 * than parameter `requiredRemainingStorage`.
 *
 * \post  @ref oldMessagesRemoved will be set if at least one log message has been removed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param requiredRemainingStorage
 * Required minumum value for @ref remainingStorage.
 */
void Backend_CLILogHistory::RemoveMessages(size_t const requiredRemainingStorage) noexcept
{
  while ((!messages.empty()) && (remainingStorage < requiredRemainingStorage))
    RemoveMessage();
}

/**
 * \brief Creates a std::string containing a warning about dropped messages.
 *
 * Note: The created string makes no sense if @ref nbOfDroppedMessages is zero.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
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
 * String containing a warning about dropped messages.
 */
std::string Backend_CLILogHistory::BuildWarningForDroppedMessages(void) const
{
  std::string s("Warning: ");
  if (nbOfDroppedMessages == 255U)
    s += "At least 255";
  else
    s += std::to_string(static_cast<uint32_t>(nbOfDroppedMessages));

  s += " message(s) were not recorded due to errors! (e.g. out of memory)";

  return s;
}

/**
 * \brief Prints a @ref tBufferItem to a CLI.
 *
 * Output format (example):\n
 * History -n: [ERROR] SomeObj: Got an error
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - incomplete or undefined data may have been written to the CLI.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - incomplete or undefined data may have been written to the CLI.
 *
 * - - -
 *
 * \param n
 * Number of the history buffer item.\n
 * This is build into the output (see example above).
 *
 * \param item
 * Reference to the @ref tBufferItem that shall be printed.
 *
 * \param cli
 * Reference to the CLI to which the @ref tBufferItem shall be printed.
 */
void Backend_CLILogHistory::PrintBufferItem(uint_fast16_t const n, tBufferItem const & item, gpcc::cli::CLI & cli)
{
  std::string const lineHead(CLI_BOLD_LIGHT_CYAN "History -" + std::to_string(n) + ": " CLI_STD);

  char const * fragments[4];
  switch (item.first)
  {
    case LogType::Warning:
      fragments[0] = lineHead.c_str();
      fragments[1] = CLI_BOLD_YELLOW;
      fragments[2] = item.second.c_str();
      fragments[3] = nullptr;
      break;

    case LogType::Error:
      fragments[0] = lineHead.c_str();
      fragments[1] = CLI_RED;
      fragments[2] = item.second.c_str();
      fragments[3] = nullptr;
      break;

    case LogType::Fatal:
      fragments[0] = lineHead.c_str();
      fragments[1] = CLI_BOLD_LIGHT_RED;
      fragments[2] = item.second.c_str();
      fragments[3] = nullptr;
      break;

    default:
      fragments[0] = lineHead.c_str();
      fragments[1] = item.second.c_str();
      fragments[2] = nullptr;
      fragments[3] = nullptr; // (unused but makes compiler and code analyzers happy)
      break;
  }

  cli.WriteLineComposed(fragments);
}

/**
 * \brief Command handler for the "LogHistory" CLI command.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - incomplete or undefined data may have been written to the CLI.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - incomplete or undefined data may have been written to the CLI.
 *
 * - - -
 *
 * \param restOfLine
 * Anything entered behind the command.
 *
 * \param cli
 * Reference to the CLI instance which invokes this.
 */
void Backend_CLILogHistory::CLICMD_LogHistory(std::string const & restOfLine, gpcc::cli::CLI & cli)
{
  std::vector<std::string> params = gpcc::string::Split(restOfLine, ' ', true);

  gpcc::osal::MutexLocker mutexLocker(mutex);

#if 0
  // Use this to generate a "dropped message" output message
  if (restOfLine == "Test")
  {
    nbOfDroppedMessages++;
    return;
  }
#endif

  // examine arguments and overwrite defaults for "n" and "clear" if any args are given
  uint32_t n = messages.size();
  bool nEntered = false;
  bool nZeroEntered = false;
  bool clear = false;

  auto it = params.begin();

  // first argument could be "n"
  if (it != params.end())
  {
    if (gpcc::string::IsDecimalDigitsOnly(*it))
    {
      n = gpcc::string::DecimalToU32(*it);
      nEntered = true;
      if (n == 0U)
        nZeroEntered = true;

      if (n > messages.size())
        n = messages.size();

      ++it;
    }
  }

  // next argument could be "clear"
  if (it != params.end())
  {
    if (*it == "clear")
    {
      clear = true;
      ++it;
    }
  }

  // there must be no more args
  if (it != params.end())
  {
    cli.WriteLine("Error: Invalid parameters!\nTry 'LogHistory help'!");
    return;
  }

  params.clear();

  // ask user for confirmation if the user did not provide "n" and if a large number of messages shall be printed
  if ((!nEntered) && (n >= askBeforePrintThreshold))
  {
    cli.WriteLine("The log history contains a large number of entries. Proceed?\n"\
                  "You can use CTRL+C to abort during printing.");

    if (cli.ReadLine("Continue? (y/n) >") != "y")
      return;
  }

  // print recorded log messages
  if (n != 0U)
  {
    // Guarantee:
    // n is equal to or less than messages.size() and messages,size() is equal to or less than maxNbOfMessages

    uint_fast16_t const skippedRecords = messages.size() - n;
    if (skippedRecords == 0U)
    {
      if (oldMessagesRemoved)
        cli.WriteLine(CLI_BOLD_LIGHT_CYAN "History: " CLI_STD "At least one old message has been discarded.");
    }
    else
    {
      cli.WriteLine(CLI_BOLD_LIGHT_CYAN "History: " CLI_STD "Skipping " + std::to_string(skippedRecords) + " record(s).");
    }

    // We use a reverse iterator to iterate over the latest n recorded log messages.
    // Note that it is safe and allowed to decrement the end/rend-iterator of std::list containters.
    auto rev_it = messages.crbegin();
    std::advance(rev_it, n);

    do
    {
      cli.TestTermination();

      --rev_it;
      PrintBufferItem(static_cast<uint_fast16_t>(n), *rev_it, cli);
      n--;
    }
    while (rev_it != messages.crbegin());

    if (clear && (nbOfDroppedMessages != 0U))
      cli.WriteLine(CLI_BOLD_YELLOW + BuildWarningForDroppedMessages());
  }
  else if ((!nEntered) || ((nEntered) && (!nZeroEntered)))
  {
    // case: Buffer is empty and user did not enter "n"
    //    OR Buffer is empty and user entered "n" and user did not enter zero

    cli.WriteLine("Log history empty.");

    if (clear && (nbOfDroppedMessages != 0U))
      cli.WriteLine(CLI_BOLD_YELLOW + BuildWarningForDroppedMessages());
  }
  else
  {
    // case: User provided zero for "n". The buffer may or may not be empty.
  }

  // clear log message buffer if requested, otherwise print status
  if (clear)
  {
    UnprotectedClear();
    cli.WriteLine("Log history cleared.");
  }
  else
  {
    if (nbOfDroppedMessages != 0U)
      cli.WriteLine(CLI_BOLD_YELLOW + BuildWarningForDroppedMessages());

    cli.WriteLine("Remaining capacity: " + std::to_string(maxNbOfMessages - messages.size()) + " entries or " +
                   std::to_string(remainingStorage) + " bytes.");
  }
}

} // namespace log
} // namespace gpcc
