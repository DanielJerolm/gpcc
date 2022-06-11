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
#ifndef LOG_LEVELS_HPP_201701061530
#define LOG_LEVELS_HPP_201701061530

#include <string>
#include <cstddef>

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG
 * \brief Enumeration with log types for log messages.
 */
enum class LogType
{
  Debug     = 0,  ///<Debug message.
                  /**<Messages by developers for developers.\n
                      Example output:\n
                      [DEBUG] State machine: Entered State 5\n
                      [DEBUG] Message dispatcher: initialized */
  Info      = 1,  ///<Info message.
                  /**<This type of message shall be used for information that is useful for
                      running and management of the system.\n
                      Example output:\n
                      [INFO ] USB Host: Device XY attached. */
  Warning   = 2,  ///<Warning message.
                  /**<This type of message shall be used for handled exceptions.\n
                      Example output:\n
                      [WARN ] Power supply: Battery low. Approx. 5 minutes remaining.\n
                      [WARN ] Configuration file not found. Using defaults. */
  Error     = 3,  ///<Error message.
                  /**<This type of message shall be used for unhandled exceptions.\n
                      Example output:\n
                      [ERROR] Loader: CRC error in EEPROM data block. */
  Fatal     = 4   ///<Fatal error message.
                  /**<Critical error which leads to program termination.\n
                      It is not useful to log any fatal error which will result in program
                      termination, because after program termination the logger will not
                      process the fatal error message.\n
                      Instead this type of message shall be used to log post-mortem messages
                      collected after program (re)start. Example:\n
                      A program should terminate via @ref gpcc::osal::Panic() in case of a fatal
                      error. The panic handler could be configured to create a post-mortem log
                      before terminating the application.\n
                      After restart of the application, the post-mortem log could be located
                      by the application and it could be logged as a fatal error message. */
};

/**
 * \ingroup GPCC_LOG
 * \brief Enumeration with thresholds for filtering log messages by log types.
 */
enum class LogLevel
{
  DebugOrAbove          = 0,  ///<Logs everything.
  InfoOrAbove           = 1,  ///<Logs info, warning, error, and fatal error messages.
  WarningOrAbove        = 2,  ///<Logs warning, error, and fatal error messages.
  ErrorOrAbove          = 3,  ///<Logs error, and fatal error messages.
  FatalOrAbove          = 4,  ///<Logs fatal error messages only.
  Nothing               = 5,  ///<Logs __nothing__.
};

/**
 * \ingroup GPCC_LOG
 * \brief Length of any log message header string returned by @ref LogType2LogMsgHeader().
 */
size_t constexpr logMsgHeaderLength = 7U;


char const * LogType2LogMsgHeader(LogType const type);
char const * LogLevel2String(LogLevel const level);
LogLevel String2LogLevel(std::string const & s);

} // namespace log
} // namespace gpcc

#endif // LOG_LEVELS_HPP_201701061530
