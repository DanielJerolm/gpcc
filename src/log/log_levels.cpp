/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2021 Daniel Jerolm

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

#include "log_levels.hpp"
#include <stdexcept>

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG
 * \brief Retrieves a log message header for log messages with given @ref LogType.
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
 *
 * - - -
 *
 * \param type
 * @ref LogType value.
 *
 * \return
 * Log message header for log messages of type `type`. Examples:\n
 * [DEBUG], [INFO ], [WARN ], [ERROR], [FATAL]\n
 * This points to a null-terminated c-string located in ROM/program memory.\n
 * The string has always the same length: @ref logMsgHeaderLength plus null-terminator.
 */
char const * LogType2LogMsgHeader(LogType const type)
{
  switch (type)
  {
    case LogType::Debug  : return "[DEBUG]";
    case LogType::Info   : return "[INFO ]";
    case LogType::Warning: return "[WARN ]";
    case LogType::Error  : return "[ERROR]";
    case LogType::Fatal  : return "[FATAL]";
  }

  throw std::invalid_argument("LogType2LogMsgHeader: \"type\" invalid");
}

/**
 * \ingroup GPCC_LOG
 * \brief Retrieves a textual representation of a @ref LogLevel value.
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
 *
 * - - -
 *
 * \param level
 * @ref LogLevel value.
 *
 * \return
 * Textual description of `level`:\n
 * debug, info, warning, error, fatal, nothing\n
 * This points to a null-terminated c-string located in ROM/program memory.
 */
char const * LogLevel2String(LogLevel const level)
{
  switch (level)
  {
    case LogLevel::DebugOrAbove  : return "debug";
    case LogLevel::InfoOrAbove   : return "info";
    case LogLevel::WarningOrAbove: return "warning";
    case LogLevel::ErrorOrAbove  : return "error";
    case LogLevel::FatalOrAbove  : return "fatal";
    case LogLevel::Nothing       : return "nothing";
  }

  throw std::invalid_argument("LogLevel2String: \"level\" invalid");
}

/**
 * \ingroup GPCC_LOG
 * \brief Converts a string into a @ref LogLevel enum value.
 *
 * This function is the reverse of @ref LogLevel2String().
 *
 * This function is aware of all sort of erroneous user input. The recognized strings are:
 * - debug
 * - info
 * - warning
 * - error
 * - fatal
 * - nothing
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
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to an std::string containing the log level that shall be converted into an @ref LogLevel enum
 * value.
 *
 * \return
 * @ref LogLevel enum value referenced by `s`.
 */
LogLevel String2LogLevel(std::string const & s)
{
  if (s == "debug")
    return LogLevel::DebugOrAbove;
  else if (s == "info")
    return LogLevel::InfoOrAbove;
  else if (s == "warning")
    return LogLevel::WarningOrAbove;
  else if (s == "error")
    return LogLevel::ErrorOrAbove;
  else if (s == "fatal")
    return LogLevel::FatalOrAbove;
  else if (s == "nothing")
    return LogLevel::Nothing;
  else
    throw std::runtime_error("String2LogLevel: 's' is not a valid log level");
}

} // namespace log
} // namespace gpcc
