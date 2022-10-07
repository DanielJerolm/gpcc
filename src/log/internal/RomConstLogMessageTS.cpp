/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "RomConstLogMessageTS.hpp"
#include <gpcc/string/tools.hpp>
#include <stdexcept>
#include <cstring>

namespace gpcc     {
namespace log      {
namespace internal {

/**
 * \brief Constructor.
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
 * \param _pMsg
 * Pointer to a null-terminated c-string which contains the log message text.\n
 * The referenced c-string must be located in ROM/code memory and must not change.\n
 * nullptr is not allowed.
 */
RomConstLogMessageTS::RomConstLogMessageTS(string::SharedString const & _srcName,
                                           LogType const _type,
                                           char const * const _pMsg)
: LogMessage(_srcName, _type)
, pMsg(_pMsg)
, timestamp(gpcc::time::TimePoint::FromSystemClock(gpcc::time::Clocks::realtimeCoarse))
{
  if (pMsg == nullptr)
    throw std::invalid_argument("RomConstLogMessageTS::RomConstLogMessageTS: !_pMsg");
}

/// \copydoc LogMessage::BuildText
std::string RomConstLogMessageTS::BuildText(void) const
{
  std::string s;
  s.reserve(logMsgHeaderLength + 1U + srcName.GetStr().size() + 3U + gpcc::time::TimePoint::stringLength + 2U + strlen(pMsg));

  s = LogType2LogMsgHeader(static_cast<LogType>(type));
  s += ' ';
  s += srcName.GetStr();
  s += ": (";
  s += timestamp.ToString();
  s += ") ";
  s += pMsg;

  string::InsertIndention(s, logMsgHeaderLength + 1U);

  return s;
}

} // namespace internal
} // namespace log
} // namespace gpcc
