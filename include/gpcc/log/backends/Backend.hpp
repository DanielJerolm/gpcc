/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef BACKEND_HPP_201701052121
#define BACKEND_HPP_201701052121

#include <gpcc/log/log_levels.hpp>
#include <atomic>
#include <string>

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG_BACKENDS
 * \brief Base class for log facility back-ends.
 *
 * Log facility back-ends can be registered at log facilities. Log facilities offer all incoming log messages
 * to their registered back-ends.
 *
 * Multiple back-ends can be registered at one log facility, but an instance of a back-end can be registered
 * at one log facility only.
 *
 * Log messages are offered to back-ends as `std::string` objects containing a text of the log message. Back-ends
 * then either print the message to a console, write it to a file, or send it via the internet, or whatever the
 * particular back-end does. Back-ends could also filter log messages, e.g. a back-end could only write
 * error-messages to a file.
 *
 * - - -
 *
 *  __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.\n
 * This is sufficient, because a back-end can be registered at one log facility only and because log facilities
 * do not use multiple threads to invoke one and the same back-end instance.
 */
class Backend
{
    friend class ThreadedLogFacility;

  public:
    Backend(Backend const &) = delete;
    Backend(Backend &&) = delete;

    Backend& operator=(Backend const &) = delete;
    Backend& operator=(Backend &&) = delete;

    virtual void Process(std::string const & msg, LogType const type) = 0;

  protected:
    Backend(void) noexcept;
    virtual ~Backend(void) = default;

  private:
    /// Flag indicating if the back-end is registered at a log facility.
    std::atomic<bool> registered;

    /// Next-pointer used by log-facilities to organize back-ends in single-linked lists.
    Backend* pNext;
};

/**
 * \fn Backend::Process
 * \brief Processes a log message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Log message processing may be incomplete (e.g. only parts of the message could be printed to console
 *   or written to a file).
 *
 * - - -
 *
 * \param msg
 * String containing the log message.\n
 * Format:\n
 * [_type_] _source name_: _log message_\n
 * There may be more fields than shown in the example above.
 *
 * \param type
 * Log message type. Allows for filtering, if the back-end supports filtering.
 */

} // namespace log
} // namespace gpcc

#endif // BACKEND_HPP_201701052121
