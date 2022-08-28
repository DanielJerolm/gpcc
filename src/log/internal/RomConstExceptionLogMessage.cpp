/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "RomConstExceptionLogMessage.hpp"
#include <gpcc/string/tools.hpp>
#include <cstring>
#include <stdexcept>

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
 * Pointer to a null-terminated c-string which contains the first part of the log message.\n
 * The referenced c-string must be located in ROM/code memory and must not change.\n
 * nullptr is not allowed.\n
 * The error description of the exception referenced by parameter `_ePtr` and the error description of potential
 * nested exceptions will be appended to this on a new line. There is no need for a trailing \\n.
 *
 * \param _ePtr
 * Exception pointer referencing to the exception whose what()-method's output shall be build into the log message.\n
 * The what()-method's output of potential nested exceptions will also be build into the log message.\n
 * If the exception pointer is a nullptr, then no what()-method's output will be build into the log message.
 */
RomConstExceptionLogMessage::RomConstExceptionLogMessage(string::SharedString const & _srcName,
                                                         LogType const _type,
                                                         char const * const _pMsg,
                                                         std::exception_ptr const & _ePtr)
: LogMessage(_srcName, _type)
, pMsg(_pMsg)
, ePtr(_ePtr)
{
  if (pMsg == nullptr)
    throw std::invalid_argument("RomConstExceptionLogMessage::RomConstExceptionLogMessage: !_pMsg");
}

/// \copydoc LogMessage::BuildText
std::string RomConstExceptionLogMessage::BuildText(void) const
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

  s.reserve(logMsgHeaderLength + 1U + srcName.GetStr().size() + 2U + strlen(pMsg) + whatLength);

  s = LogType2LogMsgHeader(static_cast<LogType>(type));
  s += ' ';
  s += srcName.GetStr();
  s += ": ";
  s += pMsg;

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
