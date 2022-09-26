/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef LOGMESSAGE_HPP_201701061501
#define LOGMESSAGE_HPP_201701061501

#include <gpcc/log/log_levels.hpp>
#include <gpcc/string/SharedString.hpp>
#include <string>
#include <cstdint>

namespace gpcc {
namespace log  {

class ThreadedLogFacility;

namespace internal {

/**
 * \ingroup GPCC_LOG_INTERNAL
 * \class LogMessage LogMessage.hpp "src/log/internal/LogMessage.hpp"
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
 * \fn std::string LogMessage::BuildText(void) const
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
