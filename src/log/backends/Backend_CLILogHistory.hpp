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

#ifndef BACKEND_CLILOGHISTORY_HPP_202001041425
#define BACKEND_CLILOGHISTORY_HPP_202001041425

#include "Backend.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include <list>
#include <string>
#include <utility>
#include <cstddef>
#include <cstdint>

namespace gpcc {

namespace cli
{
  class CLI;
}

namespace Stream
{
  class IStreamWriter;
}

namespace log {

/**
 * \ingroup GPCC_LOG_BACKENDS
 * \brief Log facility back-end which records log messages in a ring buffer and offers access via @ref gpcc::cli::CLI.
 *
 * Recorded log messages can be...
 * - ...printed to a [CLI](@ref gpcc::cli::CLI) on demand via a CLI command registered by this class.
 * - ...written into an [IStreamWriter](@ref gpcc::Stream::IStreamWriter) interface via @ref Export().
 * - ...printed and exported multiple times.
 * - ...discarded upon request (@ref Clear() or via CLI command)
 *
 * # CLI integration
 * This class is intended to be used in conjunction with a [CLI](@ref gpcc::cli::CLI) instance. If a pointer to a
 * [CLI](@ref gpcc::cli::CLI) instance is passed to the constructor, then this class will register a CLI command:
 * "LogHistory". If nullptr is passed, then no CLI command will be registered.
 *
 * The "LogHistory" CLI command allows the user to print recorded messages and to clear the log message buffer.
 *
 * # Log message buffer capacity
 * The recorded log messages are stored in a ring buffer. If the buffer is full, then old messages will be discarded to
 * make room for new messages. The buffer's capacity is dynamic and composed of _two_ limitations:
 * - Maximum number of recorded log messages.
 * - Maximum number of bytes occupied by the text of the recorded log messages.
 *
 * Values for both limitations must be passed to the constructor upon object creation. Using two limitations
 * allows the user to limit the _number of recorded messages_ and the _memory_ occupied by them.
 *
 * # Additional status information
 * In addition to the log message buffer, this class implements additional attributes that provide additional
 * status information:
 * - Flag `oldMessagesRemoved`:\n
 *   This is set when a old log message is removed from the log message buffer in order to make room for a new log
 *   message. This indicates, that there have been more log messages than currently stored in the log message buffer.
 * - Counter `nbOfDroppedMessages`:\n
 *   This is incremented each time a log message could not be recorded due to an error (most likely an out-of-memory
 *   condition).
 *
 * Both the flag and the counter are reset each time the log message buffer is cleared either via CLI command
 * "LogHistory" or via @ref Clear().
 *
 * The additional status information is printed to CLI or exported together with the recorded log messages each time
 * the recorded log messages are exported or displayed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Backend_CLILogHistory final : public Backend
{
  public:
    Backend_CLILogHistory(void) = delete;
    Backend_CLILogHistory(gpcc::cli::CLI* const _pCLI, uint16_t const _maxNbOfMessages, size_t const maxTotalSize);
    Backend_CLILogHistory(Backend_CLILogHistory const &) = delete;
    Backend_CLILogHistory(Backend_CLILogHistory &&) = delete;
    ~Backend_CLILogHistory(void);

    Backend_CLILogHistory& operator=(Backend_CLILogHistory const &) = delete;
    Backend_CLILogHistory& operator=(Backend_CLILogHistory &&) = delete;

    void Clear(void);
    void Export(gpcc::Stream::IStreamWriter& output, bool const clearAfterExport);

    // <-- Backend
    void Process(std::string const & msg, LogType const type) override;
    // --> Backend

  private:
    /// Threshold at which the user will be asked if he really wants to print the recorded messages if the user did not
    /// specify the number of messages to be printed.
    static uint16_t const askBeforePrintThreshold = 128U;

    /// Type definition for an item in the message buffer.
    typedef std::pair<LogType,std::string> tBufferItem;


    /// Mutex used to make this class thread-safe.
    gpcc::osal::Mutex mutable mutex;


    /// Pointer to CLI. May be nullptr.
    gpcc::cli::CLI* const pCLI;

    /// Maximum number of messages in the buffer.
    uint16_t const maxNbOfMessages;


    /// Number of messages that could not be recorded due to an error (e.g. std::bad_alloc) since last buffer clear.
    /** @ref mutex is required. */
    uint8_t nbOfDroppedMessages;

    /// Flag indicating that at least one old message has been removed from the buffer since last buffer clear.
    /** @ref mutex is required. */
    bool oldMessagesRemoved;

    /// Number of bytes remaining for log message text until the maximum size passed to the constructor is reached.
    /** @ref mutex is required. */
    size_t remainingStorage;

    /// The recorded log messages.
    /** @ref mutex is required.\n
        New messages are inserted at the back, old messages are removed at the front. */
    std::list<tBufferItem> messages;


    void UnprotectedClear(void) noexcept;
    void RemoveMessage(void) noexcept;
    void RemoveMessages(size_t const requiredRemainingStorage) noexcept;

    std::string BuildWarningForDroppedMessages(void) const;

    static void PrintBufferItem(uint_fast16_t const n, tBufferItem const & item, gpcc::cli::CLI & cli);

    void CLICMD_LogHistory(std::string const & restOfLine, gpcc::cli::CLI & cli);
};

} // namespace log
} // namespace gpcc

#endif // BACKEND_CLILOGHISTORY_HPP_202001041425
