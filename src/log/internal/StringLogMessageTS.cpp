/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "StringLogMessageTS.hpp"
#include <gpcc/string/tools.hpp>

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
 * `std::string` containing the log message.\n
 * The referenced string will be copied into the created message object.
 */
StringLogMessageTS::StringLogMessageTS(string::SharedString const & _srcName,
                                       LogType const _type,
                                       std::string const & _msg)
: LogMessage(_srcName, _type)
, timestamp(gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::realtimeCoarse))
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
 * `std::string` containing the log message.\n
 * The referenced string will be moved into the created message object.
 */
StringLogMessageTS::StringLogMessageTS(string::SharedString const & _srcName,
                                       LogType const _type,
                                       std::string && _msg)
: LogMessage(_srcName, _type)
, timestamp(gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::realtimeCoarse))
, msg(std::move(_msg))
{
}

/// \copydoc LogMessage::BuildText
std::string StringLogMessageTS::BuildText(void) const
{
  std::string s;
  s.reserve(logMsgHeaderLength + 1U + srcName.GetStr().size() + 3U + gpcc::time::TimePoint::stringLength + 2U + msg.size());

  s = LogType2LogMsgHeader(static_cast<LogType>(type));
  s += ' ';
  s += srcName.GetStr();
  s += ": (";
  s += timestamp.ToString();
  s += ") ";
  s += msg;

  string::InsertIndention(s, logMsgHeaderLength + 1U);

  return s;
}

} // namespace internal
} // namespace log
} // namespace gpcc
