/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
