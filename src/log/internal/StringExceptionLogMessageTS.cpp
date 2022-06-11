/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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
#include "StringExceptionLogMessageTS.hpp"
#include "gpcc/src/string/tools.hpp"

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \brief Constructor. Copies the message string.
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
 * \param _srcName
 * Name of the source of the log message.
 *
 * \param _type
 * Type of log message.
 *
 * \param _msg
 * `std::string` containing the first part of the log message.\n
 * The referenced string will be copied into the created message object.\n
 * The error description of the exception referenced by parameter `_ePtr` and the error description of potential
 * nested exceptions will be appended to this on a new line. There is no need for a trailing \\n.
 *
 * \param _ePtr
 * Exception pointer referencing to the exception whose what()-method's output shall be build into the log message.\n
 * The what()-method's output of potential nested exceptions will also be build into the log message.\n
 * If the exception pointer is a nullptr, then no what()-method's output will be build into the log message.
 */
StringExceptionLogMessageTS::StringExceptionLogMessageTS(string::SharedString const & _srcName,
                                                         LogType const _type,
                                                         std::string const & _msg,
                                                         std::exception_ptr const & _ePtr)
: LogMessage(_srcName, _type)
, ePtr(_ePtr)
, timestamp(gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::realtime))
, msg(_msg)
{
}

/**
 * \brief Constructor. Moves the message string.
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
 * \param _srcName
 * Name of the source of the log message.
 *
 * \param _type
 * Type of log message.
 *
 * \param _msg
 * `std::string` containing the first part of the log message.\n
 * The referenced string will be moved into the created message object.\n
 * The error description of the exception referenced by parameter `_ePtr` and the error description of potential
 * nested exceptions will be appended to this on a new line. There is no need for a trailing \\n.
 *
 * \param _ePtr
 * Exception pointer referencing to the exception whose what()-method's output shall be build into the log message.\n
 * The what()-method's output of potential nested exceptions will also be build into the log message.\n
 * If the exception pointer is a nullptr, then no what()-method's output will be build into the log message.
 */
StringExceptionLogMessageTS::StringExceptionLogMessageTS(string::SharedString const & _srcName,
                                                         LogType const _type,
                                                         std::string && _msg,
                                                         std::exception_ptr const & _ePtr)
: LogMessage(_srcName, _type)
, ePtr(_ePtr)
, timestamp(gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::realtime))
, msg(std::move(_msg))
{
}

/// \copydoc LogMessage::BuildText
std::string StringExceptionLogMessageTS::BuildText(void) const
{
  std::string s;

  std::string what;
  size_t whatLength = 0U;
  if (ePtr)
  {
    what = string::ExceptionDescriptionToString(ePtr);
    whatLength = 1U + (logMsgHeaderLength + 1U) + what.length();
    // (whatLength includes lenth of '\n' and length of indention for first new line)
  }

  s.reserve(logMsgHeaderLength + 1U + srcName.GetStr().size() + 3U + gpcc::time::TimePoint::stringLength + 2U + msg.size() + whatLength);

  s = LogType2LogMsgHeader(static_cast<LogType>(type));
  s += ' ';
  s += srcName.GetStr();
  s += ": (";
  s += timestamp.ToString();
  s += ") ";
  s += msg;

  if (whatLength != 0U)
  {
    s += '\n';
    s += what;
  }

  string::InsertIndention(s, logMsgHeaderLength + 1U);

  return s;
}

} // namespace internal
} // namespace log
} // namespace gpcc
