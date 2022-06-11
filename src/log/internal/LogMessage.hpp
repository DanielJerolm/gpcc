/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2021, 2022 Daniel Jerolm

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
#ifndef LOGMESSAGE_HPP_201701061501
#define LOGMESSAGE_HPP_201701061501

#include "gpcc/src/log/log_levels.hpp"
#include "gpcc/src/string/SharedString.hpp"
#include <string>
#include <cstdint>

namespace gpcc {
namespace log  {

class ThreadedLogFacility;

namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \brief Base class for log message containers.
 *
 * Classes derived from this implement containers for log messages.
 *
 * In GPCC's log system, log message encapsulation is kind of special. Classes derived from this implement containers
 * for log message _ingredients_. _Ready-build_ log message strings are usually _not_ encapsulated.
 *
 * Working with _ingredients_ instead of _ready-build log message strings_ moves the effort to create complex log
 * message strings (i.e. messages build from text fragments and numbers) from the source of the log message to the
 * log facility. The log facility is executed in a dedicated thread and thus removes most of the work from the log
 * message source. This approach minimizes the performance impact of logging that could otherwise be experienced
 * by log message sources.
 *
 * Sub-classes of this class shall encapsulate the ingredients for exactly one log message.\n
 * Depending on the sub-class implementation, the following ingredients are possible:
 * - name of log message source
 * - type of log message (@ref LogType)
 * - some kind of log message text
 * - optionally a time-stamp
 * - optionally parameters/values
 * - optional exception pointer
 *
 * The type of storage used for the ingredients, the type, format, and number of ingredients depends on the sub-class.
 * There are multiple sub-classes available, which encapsulate different ingredients for different log messages.
 *
 * To build the log message string from the ingredients, log facilities shall invoke @ref BuildText().
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class LogMessage
{
    friend class gpcc::log::ThreadedLogFacility;

  public:
    LogMessage(void) = delete;
    LogMessage(LogMessage const &) = delete;
    LogMessage(LogMessage &&) = delete;
    virtual ~LogMessage(void) = default;

    LogMessage& operator=(LogMessage const &) = delete;
    LogMessage& operator=(LogMessage &&) = delete;


    LogType GetLogType(void) const noexcept;
    virtual std::string BuildText(void) const = 0;

  protected:
    /// Name of the source of the log message.
    string::SharedString const srcName;

    /// Type of log message.
    /** This is a @ref LogType enumeration value represented by an `uint8_t` in order to save some memory if
        further `uint8_t` attributes are added behind this in the future. */
    uint8_t const type;


    LogMessage(string::SharedString const & _srcName, LogType const _type);

  private:
    /// Next-pointer used by log facilities to organize log messages in single linked lists.
    LogMessage* pNext;
};

/**
 * \brief Retrieves the @ref LogType of the log message.
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
 * @ref LogType of the log message.
 */
inline LogType LogMessage::GetLogType(void) const noexcept
{
  return static_cast<LogType>(type);
}

/**
 * \fn LogMessage::BuildText
 * \brief Creates an std::string containing the log message build from the ingredients stored in the
 *        @ref LogMessage sub-class.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * `std::string` object containing the log message.\n
 * Format:\n
 * [_type_] _source name_: (_optional time-stamp_) _log message_\n
 * Depending on the sub-class implementation, more fields might be possible.
 */

} // namespace internal
} // namespace log
} // namespace gpcc

#endif // LOGMESSAGE_HPP_201701061501
