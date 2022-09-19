/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/log/Logger.hpp>
#include <gpcc/log/logfacilities/ILogFacility.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include "internal/CStringLogMessage.hpp"
#include "internal/CStringLogMessageTS.hpp"
#include "internal/RomConstExceptionLogMessage.hpp"
#include "internal/RomConstExceptionLogMessageTS.hpp"
#include "internal/RomConstLogMessage.hpp"
#include "internal/RomConstLogMessageTS.hpp"
#include "internal/StringExceptionLogMessage.hpp"
#include "internal/StringExceptionLogMessageTS.hpp"
#include "internal/StringLogMessage.hpp"
#include "internal/StringLogMessageTS.hpp"
#include <memory>
#include <stdexcept>

namespace gpcc {
namespace log  {

using namespace internal;

/**
 * \brief Constructor.
 *
 * The initial log level is @ref LogLevel::InfoOrAbove.
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
 * Name of the log message originator.\n
 * - An empty string is not allowed
 * - There must be no white-spaces
 * - "all" is not allowed
 */
Logger::Logger(std::string const & _srcName)
: srcName(_srcName)
, level(LogLevel::InfoOrAbove)
, mutex()
, pLogFacility(nullptr)
, pNext(nullptr)
, pPrev(nullptr)
{
  if ((_srcName.length() == 0U) ||
      (_srcName.find_first_of(' ') != std::string::npos) ||
      (_srcName == "all"))
  {
    throw std::invalid_argument("Logger::Logger: Bad _srcName");
  }
}

/**
 * \brief Destructor.
 *
 * \pre   The @ref Logger must not be registered at any log facility.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Logger::~Logger(void)
{
  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
    osal::Panic("Logger::~Logger: Still registered");
}

/**
 * \brief Ensures that the log level is at or below a given level.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _level
 * Desired maximum value for the log level.\n
 * If the current log level is above this, then it will be lowered.\n
 * If the current log level is equal to this or below this, then it will _not_ be raised.\n
 * (Definition of "above" and "below": "debug" is below "error")
 */
void Logger::LowerLogLevel(LogLevel const _level) noexcept
{
  LogLevel currLogLevel = level;
  while ((currLogLevel > _level) && (!std::atomic_compare_exchange_weak(&level, &currLogLevel, _level)));
}

/**
 * \brief Ensures that the log level is at or above a given level.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _level
 * Desired minimum value for the log level.\n
 * If the current log level is equal to this or above this, then it will _not_ be lowered.\n
 * If the current log level is below this, then it will be raised.\n
 * (Definition of "above" and "below": "debug" is below "error")
 */
void Logger::RaiseLogLevel(LogLevel const _level) noexcept
{
  LogLevel currLogLevel = level;
  while ((currLogLevel < _level) && (!std::atomic_compare_exchange_weak(&level, &currLogLevel, _level)));
}

/**
 * \brief Retrieves a pointer to the log facility this @ref Logger instance is registered at.
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
 * \return
 * Pointer to the log facility this @ref Logger instance is registered at.\n
 * nullptr, if this @ref Logger instance is not registered at any log facility.
 */
ILogFacility* Logger::GetLogFacility(void) const noexcept
{
  osal::MutexLocker locker(mutex);
  return pLogFacility;
}

/**
 * \brief Logs a message. Message type: null-terminated c-string located in ROM/code memory.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param pMsg
 * Log message. A trailing '\\n' is not required.\n
 * This is intended to refer to a constant null-terminated c-string located in ROM/code memory.
 */
void Logger::Log(LogType const type, char const * const pMsg) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<RomConstLogMessage>(srcName, type, pMsg);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: null-terminated c-string located in ROM/code memory plus what()-description
 *        of an exception and potential nested exceptions.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   // ...
 * }
 * catch (std::exception const &)
 * {
 *   MyLogger->Log(LogType::Error, "Error while processing xyz:", std::current_exception());
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param pMsg
 * Log message. A trailing '\\n' is not required.\n
 * This is intended to refer to a constant null-terminated c-string located in ROM/code memory.\n
 * This will be the first part of the log message. The what()-description of the exception referenced by `ePtr` will
 * be appended to this on a new line. The what()-descriptions of potential nested exceptions will be appended too,
 * each starting on a new line.\n
 * Example output:\n
 * "msg"\n
 * 1: what() of exception\n
 * 2: what() of first nested exception\n
 * ...
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing to an exception object whose what()-description shall
 * be appended to `pMsg` in order to build the log message. The what()-description of potential nested exceptions will
 * also be appended to the log message. Each what()-description will start on a new line.\n
 * Note:
 * - A copy of the exception pointer will be created.
 * - The referenced exception object will continue living at least until the log message has been processed by the
 *   log facility.
 * - An empty exception pointer is allowed. In this case, no what()-description will be build into the log message.
 */
void Logger::Log(LogType const type, char const * const pMsg, std::exception_ptr const & ePtr) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<RomConstExceptionLogMessage>(srcName, type, pMsg, ePtr);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object. The message string will be copied.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be copied.
 */
void Logger::Log(LogType const type, std::string const & msg) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringLogMessage>(srcName, type, msg);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object. The message string will be moved.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be moved.
 */
void Logger::Log(LogType const type, std::string && msg) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringLogMessage>(srcName, type, std::move(msg));
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object plus what()-description of an exception and potential
 *        nested exceptions. The message string will be copied.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   // ...
 * }
 * catch (std::exception const &)
 * {
 *   try
 *   {
 *     std::string msg;
 *
 *     // Build message text into msg. Note that this may throw an std::bad_alloc
 *     // ...
 *
 *     MyLogger->Log(LogType::Error, msg, std::current_exception());
 *   }
 *   catch (std::exception const &)
 *   {
 *     MyLogger->LogFailed();
 *   }
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be copied.\n
 * This will be the first part of the log message. The what()-description of the exception referenced by `ePtr` will
 * be appended to this on a new line. The what()-descriptions of potential nested exceptions will be appended too,
 * each starting on a new line.\n
 * Example output:\n
 * "msg"\n
 * 1: what() of exception\n
 * 2: what() of first nested exception\n
 * ...
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing to an exception object whose what()-description shall
 * be appended to `msg` in order to build the log message. The what()-description of potential nested exceptions will
 * also be appended to the log message. Each what()-description will start on a new line.\n
 * Note:
 * - A copy of the exception pointer will be created.
 * - The referenced exception object will continue living at least until the log message has been processed by the
 *   log facility.
 * - An empty exception pointer is allowed. In this case, no what()-description will ne build into the log message.
 */
void Logger::Log(LogType const type, std::string const & msg, std::exception_ptr const & ePtr) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringExceptionLogMessage>(srcName, type, msg, ePtr);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object plus what()-description of an exception and potential
 *        nested exceptions. The message string will be moved.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   // ...
 * }
 * catch (std::exception const &)
 * {
 *   try
 *   {
 *     std::string msg;
 *
 *     // Build message text into msg. Note that this may throw an std::bad_alloc
 *     // ...
 *
 *     MyLogger->Log(LogType::Error, std::move(msg), std::current_exception());
 *   }
 *   catch (std::exception const &)
 *   {
 *     MyLogger->LogFailed();
 *   }
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be moved.\n
 * This will be the first part of the log message. The what()-description of the exception referenced by `ePtr` will
 * be appended to this on a new line. The what()-descriptions of potential nested exceptions will be appended too,
 * each starting on a new line.\n
 * Example output:\n
 * "msg"\n
 * 1: what() of exception\n
 * 2: what() of first nested exception\n
 * ...
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing to an exception object whose what()-description shall
 * be appended to `msg` in order to build the log message. The what()-description of potential nested exceptions will
 * also be appended to the log message. Each what()-description will start on a new line.\n
 * Note:
 * - A copy of the exception pointer will be created.
 * - The referenced exception object will continue living at least until the log message has been processed by the
 *   log facility.
 * - An empty exception pointer is allowed. In this case, no what()-description will ne build into the log message.
 */
void Logger::Log(LogType const type, std::string && msg, std::exception_ptr const & ePtr) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringExceptionLogMessage>(srcName, type, std::move(msg), ePtr);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. The message is created from a list of variable arguments and a printf-style format string.
 *
 * In contrast to the other Log()-methods, this method creates the log message text in the context of the calling
 * thread instead of off-loading log message creation to the thread of the log facility. This method therefore offers
 * high flexibility, but uses the calling thread to create the log message string.
 *
 * To prevent evaluation of arguments if the log type is below the log level threshold configured at the logger, one
 * should use @ref IsAboveLevel() to test before calling this.\n
 * One could also use the macro @ref LOGV().
 *
 * __Usage__:\n
 * ~~~{.cpp}
 * uint32_t someValue;
 * Logger logger;
 *
 * [...]
 *
 * // Option 1
 * if (logger.IsAboveLevel(LogType::Debug))
 *   logger.LogV(LogType::Debug, "'someValue' = %u", static_cast<unsigned int>(someValue));
 *
 * // Option 2
 * LOGV(logger, LogType::Debug, "'someValue' = %u", static_cast<unsigned int>(someValue));
 * ~~~
 *
 * __Format specifiers:__\n
 * Please refer to [this](@ref gpcc::string::VASPrintf).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param pFmt
 * Pointer to a null-terminated c-string containing the log message text and printf-style conversion specifications that
 * control how the variable arguments shall be converted and integrated into the log message text.\n
 * Details about format specifiers are [here](@ref gpcc::string::VASPrintf).
 *
 * \param ...
 * Variable number of arguments that shall be printed. The number and type of arguments must match the conversion
 * specifiers embedded in `pFmt`.
 */
void Logger::LogV(LogType const type, char const * const pFmt, ...) noexcept
{
  if (!IsAboveLevel(type))
    return;

  va_list args;
  va_start(args, pFmt);
  ON_SCOPE_EXIT(end_args) { va_end(args); };

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<CStringLogMessage>(srcName, type, gpcc::string::VASPrintf(pFmt, args));
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: null-terminated c-string located in ROM/code memory plus timestamp.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param pMsg
 * Log message. A trailing '\\n' is not required.\n
 * This is intended to refer to a constant null-terminated c-string located in ROM/code memory.
 */
void Logger::LogTS(LogType const type, char const * const pMsg) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<RomConstLogMessageTS>(srcName, type, pMsg);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: null-terminated c-string located in ROM/code memory plus what()-description
 *        of an exception and potential nested exceptions plus timestamp.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   // ...
 * }
 * catch (std::exception const &)
 * {
 *   MyLogger->LogTS(LogType::Error, "Error while processing xyz:", std::current_exception());
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param pMsg
 * Log message. A trailing '\\n' is not required.\n
 * This is intended to refer to a constant null-terminated c-string located in ROM/code memory.\n
 * This will be the first part of the log message. The what()-description of the exception referenced by `ePtr` will
 * be appended to this on a new line. The what()-descriptions of potential nested exceptions will be appended too,
 * each starting on a new line.\n
 * Example output:\n
 * "msg"\n
 * 1: what() of exception\n
 * 2: what() of first nested exception\n
 * ...
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing to an exception object whose what()-description shall
 * be appended to `pMsg` in order to build the log message. The what()-description of potential nested exceptions will
 * also be appended to the log message. Each what()-description will start on a new line.\n
 * Note:
 * - A copy of the exception pointer will be created.
 * - The referenced exception object will continue living at least until the log message has been processed by the
 *   log facility.
 * - An empty exception pointer is allowed. In this case, no what()-description will be build into the log message.
 */
void Logger::LogTS(LogType const type, char const * const pMsg, std::exception_ptr const & ePtr) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<RomConstExceptionLogMessageTS>(srcName, type, pMsg, ePtr);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object plus timestamp. The message string will be copied.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be copied.
 */
void Logger::LogTS(LogType const type, std::string const & msg) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringLogMessageTS>(srcName, type, msg);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object plus timestamp. The message string will be moved.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be moved.
 */
void Logger::LogTS(LogType const type, std::string && msg) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringLogMessageTS>(srcName, type, std::move(msg));
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object plus what()-description of an exception and potential
 *        nested exceptions plus timestamp. The message string will be copied.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   // ...
 * }
 * catch (std::exception const &)
 * {
 *   try
 *   {
 *     std::string msg;
 *
 *     // Build message text into msg. Note that this may throw an std::bad_alloc
 *     // ...
 *
 *     MyLogger->LogTS(LogType::Error, msg, std::current_exception());
 *   }
 *   catch (std::exception const &)
 *   {
 *     MyLogger->LogFailed();
 *   }
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be copied.\n
 * This will be the first part of the log message. The what()-description of the exception referenced by `ePtr` will
 * be appended to this on a new line. The what()-descriptions of potential nested exceptions will be appended too,
 * each starting on a new line.\n
 * Example output:\n
 * "msg"\n
 * 1: what() of exception\n
 * 2: what() of first nested exception\n
 * ...
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing to an exception object whose what()-description shall
 * be appended to `msg` in order to build the log message. The what()-description of potential nested exceptions will
 * also be appended to the log message. Each what()-description will start on a new line.\n
 * Note:
 * - A copy of the exception pointer will be created.
 * - The referenced exception object will continue living at least until the log message has been processed by the
 *   log facility.
 * - An empty exception pointer is allowed. In this case, no what()-description will ne build into the log message.
 */
void Logger::LogTS(LogType const type, std::string const & msg, std::exception_ptr const & ePtr) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringExceptionLogMessageTS>(srcName, type, msg, ePtr);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. Message type: std::string object plus what()-description of an exception and potential
 *        nested exceptions plus timestamp. The message string will be moved.
 *
 * Usage example:
 * ~~~{.cpp}
 * try
 * {
 *   // ...
 * }
 * catch (std::exception const &)
 * {
 *   try
 *   {
 *     std::string msg;
 *
 *     // Build message text into msg. Note that this may throw an std::bad_alloc
 *     // ...
 *
 *     MyLogger->Log(LogType::Error, std::move(msg), std::current_exception());
 *   }
 *   catch (std::exception const &)
 *   {
 *     MyLogger->LogFailed();
 *   }
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param msg
 * Log message. A trailing '\\n' is not required.\n
 * The referenced string will be moved.\n
 * This will be the first part of the log message. The what()-description of the exception referenced by `ePtr` will
 * be appended to this on a new line. The what()-descriptions of potential nested exceptions will be appended too,
 * each starting on a new line.\n
 * Example output:\n
 * "msg"\n
 * 1: what() of exception\n
 * 2: what() of first nested exception\n
 * ...
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing to an exception object whose what()-description shall
 * be appended to `msg` in order to build the log message. The what()-description of potential nested exceptions will
 * also be appended to the log message. Each what()-description will start on a new line.\n
 * Note:
 * - A copy of the exception pointer will be created.
 * - The referenced exception object will continue living at least until the log message has been processed by the
 *   log facility.
 * - An empty exception pointer is allowed. In this case, no what()-description will ne build into the log message.
 */
void Logger::LogTS(LogType const type, std::string && msg, std::exception_ptr const & ePtr) noexcept
{
  if (!IsAboveLevel(type))
    return;

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<StringExceptionLogMessageTS>(srcName, type, std::move(msg), ePtr);
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Logs a message. The message is created from a list of variable arguments and a printf-style format string
 *        plus timestamp.
 *
 * In contrast to the other Log()-methods, this method creates the log message text in the context of the calling
 * thread instead of off-loading log message creation to the thread of the log facility. This method therefore offers
 * high flexibility, but uses the calling thread to create the log message string.
 *
 * To prevent evaluation of arguments if the log type is below the log level threshold configured at the logger, one
 * should use @ref IsAboveLevel() to test before calling this.\n
 * One could also use the macro @ref LOGVTS().
 *
 * __Usage__:\n
 * ~~~{.cpp}
 * uint32_t someValue;
 * Logger logger;
 *
 * [...]
 *
 * // Option 1
 * if (logger.IsAboveLevel(LogType::Debug))
 *   logger.LogVTS(LogType::Debug, "'someValue' = %u", static_cast<unsigned int>(someValue));
 *
 * // Option 2
 * LOGVTS(logger, LogType::Debug, "'someValue' = %u", static_cast<unsigned int>(someValue));
 * ~~~
 *
 * __Format specifiers:__\n
 * Please refer to [this](@ref gpcc::string::VASPrintf).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type of log message. See @ref LogType for details.\n
 * If this is below the log level configured at this @ref Logger instance, then this method will do nothing.
 *
 * \param pFmt
 * Pointer to a null-terminated c-string containing the log message text and printf-style conversion specifications that
 * control how the variable arguments shall be converted and integrated into the log message text.\n
 * Details about format specifiers are [here](@ref gpcc::string::VASPrintf).
 *
 * \param ...
 * Variable number of arguments that shall be printed. The number and type of arguments must match the conversion
 * specifiers embedded in `pFmt`.
 */
void Logger::LogVTS(LogType const type, char const * const pFmt, ...) noexcept
{
  if (!IsAboveLevel(type))
    return;

  va_list args;
  va_start(args, pFmt);
  ON_SCOPE_EXIT(end_args) { va_end(args); };

  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
  {
    try
    {
      auto spLM = std::make_unique<CStringLogMessageTS>(srcName, type, gpcc::string::VASPrintf(pFmt, args));
      pLogFacility->Log(std::move(spLM));
    }
    catch (std::exception const &)
    {
      pLogFacility->ReportLogMessageCreationFailed();
    }
  }
}

/**
 * \brief Reports that the user failed to create a log message and pass it to any of the Log()-methods.
 *
 * The incident will be reported to the log facility. The log facility guarantees, that a special log message
 * indicating the incident will be logged.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
void Logger::LogFailed(void) noexcept
{
  osal::MutexLocker locker(mutex);
  if (pLogFacility != nullptr)
    pLogFacility->ReportLogMessageCreationFailed();
}

} // namespace log
} // namespace gpcc
