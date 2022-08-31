/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef LOGGER_HPP_201701141240
#define LOGGER_HPP_201701141240

#include "log_levels.hpp"
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/string/SharedString.hpp>
#include <atomic>
#include <exception>
#include <string>
#include <cstdarg>
#include <cstdint>

namespace gpcc {
namespace log  {

class ILogFacility;

/**
 * \ingroup GPCC_LOG
 * \brief Logger (front-end for a log-facility).
 *
 * Class @ref Logger allows to emit log messages to a log facility. Class @ref Logger encapsulates the name of the
 * log source and a configurable log-level for suppression of uninteresting log messages (e.g. debug log messages).
 *
 * Any functionality/object/sub-system that wants to create log messages and emit them to a log facility has to
 * create an instance of this class and register it at the log facility. An instance of this class can be registered
 * at one log facility only.
 *
 * If a system is comprised of several sub-systems, then each sub-system should create an own instance of this class
 * in order to emit log messages. If each sub-system has its own @ref Logger instance, then the source of a
 * log message can be easily identified and a log level can be easily setup and managed for each sub-system.
 *
 * # Creating log messages
 * Log messages are created by invoking one of the `void Log()` or `void LogTS()` methods offered by this
 * class. There are multiple overloads of the `void Log()` and `void LogTS()` method, each designed for a
 * different type of log message.
 * - @ref Log(LogType const type, char const * const pMsg)
 * - @ref Log(LogType const type, char const * const pMsg, std::exception_ptr const & ePtr)
 * - @ref Log(LogType const type, std::string const & msg)
 * - @ref Log(LogType const type, std::string && msg)
 * - @ref Log(LogType const type, std::string const & msg, std::exception_ptr const & ePtr)
 * - @ref Log(LogType const type, std::string && msg, std::exception_ptr const & ePtr)
 * - @ref LogV(LogType const type, char const * const pFmt, ...)
 * - @ref LogTS(LogType const type, char const * const pMsg)
 * - @ref LogTS(LogType const type, char const * const pMsg, std::exception_ptr const & ePtr)
 * - @ref LogTS(LogType const type, std::string const & msg)
 * - @ref LogTS(LogType const type, std::string && msg)
 * - @ref LogTS(LogType const type, std::string const & msg, std::exception_ptr const & ePtr)
 * - @ref LogTS(LogType const type, std::string && msg, std::exception_ptr const & ePtr)
 * - @ref LogVTS(LogType const type, char const * const pFmt, ...)
 *
 * All Log()-methods will not emit a log message, if the type of the log message (see @ref LogType) is below the
 * log level (see @ref LogLevel) configured at the @ref Logger instance. To prevent building a log message for nothing
 * (especially when using the versions of `Log()` / `LogTS()` which accept an `std::string`), @ref IsAboveLevel()
 * should be invoked first before creating a log message:
 *
 * ~~~{.cpp}
 * if (myLogger.IsAboveLevel(LogType::Info))
 * {
 *   try
 *   {
 *     std::string msg = "Usb device \"" + usbDevice.name + "\" has been attached";
 *     myLogger.Log(LogType::Info, std::move(msg));
 *   }
 *   catch (std::exception const &)
 *   {
 *     // something went wrong during creation of the string "msg"
 *     myLogger.LogFailed();
 *   }
 * }
 * ~~~
 *
 * If there is almost zero overhead for the call to the Log()-method, as it is for the overloads accepting a pointer
 * to a null-terminated c-string, then invocation of @ref IsAboveLevel() can be omitted:
 *
 * ~~~
 * myLogger.Log(LogType::Info, "Detected a new USB device");
 * // or
 * myLogger.LogTS(LogType::Info, "Detected a new USB device");
 * ~~~
 *
 * # Setting the log level
 * Class @ref Logger contains a class member which holds the log level (@ref LogLevel) configured for the log source.
 *
 * The log level can be set using the following methods:
 * - @ref SetLogLevel()
 * - @ref LowerLogLevel()
 * - @ref RaiseLogLevel()
 *
 * The log level can be retrieved via @ref GetLogLevel().
 *
 * @ref IsAboveLevel() should be used to test a @ref LogType value before creating a complex log message text.
 *
 * The interface @ref ILogFacilityCtrl, which is implemented by any log facility also offers some methods for
 * settings log levels. See @ref ILogFacilityCtrl for details.
 *
 * # Error handling
 * Errors may occur during any phase of logging:
 * - During preparation of a log message before invocation of a Log()-method
 * - Inside a Log()-method
 * - In the log facility during delivery of the log message to the back-end(s)
 *
 * The two most likely errors are:
 * - occurrence of an std::bad_alloc
 * - Log facilities' log message FIFO is full
 *
 * In case of an error, the log message will be dropped, but the incident will be recognized and GPCC's log system
 * will create a special log message informing about the error that will be send to the back-end(s).\n
 * In other words:\n
 * GPCC tries hard to not drop any log message silently.\n
 * Even the special log message will be repeated until it is properly delivered.
 *
 * ## Errors before invoking a Log()-method
 * Some overloads of the `void Log()` and `void LogTS()` methods accept an `std::string` parameter. Typically
 * the string is created before invoking the Log()-method. During creation of the string, a `std::bad_alloc` or
 * other errors may ocurr. Users should catch any error that could occur during creation of a log message string and
 * invoke @ref LogFailed() to report the incident:
 *
 * ~~~{.cpp}
 * if (myLogger.IsAboveLevel(LogType::Info))
 * {
 *   try
 *   {
 *     std::string msg;
 *
 *     // Build the message text into msg. This may fail at least with an std::bad_alloc.
 *     // ...
 *
 *     myLogger.Log(LogType::Info, std::move(msg));
 *   }
 *   catch (std::exception const &)
 *   {
 *     // something went wrong during creation of the string "msg"
 *     myLogger.LogFailed();
 *   }
 * }
 * ~~~
 *
 * Invocation of @ref LogFailed() will report the error to the log facility.\n
 * The log facility will create a special log message indicating that at least one log message has been dropped due to
 * an error. Emission of the special log message is guaranteed by the log facility.
 *
 * ## Errors after invoking a Log()-method
 * All overloads of the `void Log()` and `void LogTS()` methods provide the no-throw guarantee and do not
 * contain any cancellation points. However, log messages and some ingredients are allocated on the heap, so a
 * `std::bad_alloc` could occur inside a Log()-method. In these cases, the information passed to the Log()-
 * method is lost, but the incident will be recognized and the log facility will create a special log message
 * indicating that at least one log message has been dropped due to an error. Emission of the special log message
 * is guaranteed by the log facility.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Logger final
{
    friend class ThreadedLogFacility;

  public:
    Logger(void) = delete;
    Logger(std::string const & _srcName);
    Logger(Logger const &) = delete;
    Logger(Logger &&) = delete;
    ~Logger(void);

    Logger& operator=(Logger const &) = delete;
    Logger& operator=(Logger &&) = delete;

    std::string const & GetName(void) const noexcept;

    bool IsAboveLevel(LogType const type) const noexcept;
    LogLevel GetLogLevel(void) const noexcept;
    void SetLogLevel(LogLevel const _level) noexcept;
    void LowerLogLevel(LogLevel const _level) noexcept;
    void RaiseLogLevel(LogLevel const _level) noexcept;

    ILogFacility* GetLogFacility(void) const noexcept;

    void Log(LogType const type, char const * const pMsg) noexcept;
    void Log(LogType const type, char const * const pMsg, std::exception_ptr const & ePtr) noexcept;
    void Log(LogType const type, std::string const & msg) noexcept;
    void Log(LogType const type, std::string && msg) noexcept;
    void Log(LogType const type, std::string const & msg, std::exception_ptr const & ePtr) noexcept;
    void Log(LogType const type, std::string && msg, std::exception_ptr const & ePtr) noexcept;
    void LogV(LogType const type, char const * const pFmt, ...) noexcept;
    void LogTS(LogType const type, char const * const pMsg) noexcept;
    void LogTS(LogType const type, char const * const pMsg, std::exception_ptr const & ePtr) noexcept;
    void LogTS(LogType const type, std::string const & msg) noexcept;
    void LogTS(LogType const type, std::string && msg) noexcept;
    void LogTS(LogType const type, std::string const & msg, std::exception_ptr const & ePtr) noexcept;
    void LogTS(LogType const type, std::string && msg, std::exception_ptr const & ePtr) noexcept;
    void LogVTS(LogType const type, char const * const pFmt, ...) noexcept;
    void LogFailed(void) noexcept;

  private:
    /// Name of the log message source.
    string::SharedString const srcName;

    /// Log level for log message suppression.
    /** Logging messages with a log type below this level will be suppressed. */
    std::atomic<LogLevel> level;

    /// Mutex used to make the API thread-safe.
    /** Locking order: @ref mutex -> @ref ThreadedLogFacility::mutex -> @ref ThreadedLogFacility::msgListMutex */
    osal::Mutex mutable mutex;

    /// Pointer to the log facility this class is connected to.
    /** @ref mutex is required.\n
        nullptr = none. */
    ILogFacility* pLogFacility;


    /// Next-pointer for building lists of @ref Logger instances inside the log facility.
    Logger* pNext;

    /// Prev-pointer for building lists of @ref Logger instances inside the log facility.
    Logger* pPrev;
};

/**
 * \brief Retrieves the name of the log source.
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
 * \return
 * Unmodifiable reference to a string containing the name of the log source.\n
 * The reference is valid until this @ref Logger instance is destroyed.
 */
inline std::string const & Logger::GetName(void) const noexcept
{
  return srcName.GetStr();
}

/**
 * \brief Tests if a given log type is at or above the log level configured at this log source.
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
 * @ref LogType that shall be tested against the log level configured at this log source.
 *
 * \retval true
 * @ref LogType `type` is at or above the log level configured at the log source.
 *
 * \retval false
 * @ref LogType `type` is below the log level configured at the log source. `Log()` and `LogTS()`
 * will drop any log message with this @ref LogType value.
 */
inline bool Logger::IsAboveLevel(LogType const type) const noexcept
{
  LogLevel const _level = level;
  return (static_cast<uint8_t>(type) >= static_cast<uint8_t>(_level));
}

/**
 * \brief Retrieves the currently configured log level.
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
 * \return
 * Currently configured log level.
 */
inline LogLevel Logger::GetLogLevel(void) const noexcept
{
  return level;
}

/**
 * \brief Sets the log level.
 *
 * Due to an explicitly accepted race-condition in the log facility, a few log messages with a log type below the new
 * log level could still be logged after raising the log level.\n
 * This behavior is accepted, because the effort (incl. performance penalty) to remove the race-condition is not worth
 * compared to just accepting occurrence of a few log messages with insufficient log level after raising the log level.
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
 * New log level for log messages.
 */
inline void Logger::SetLogLevel(LogLevel const _level) noexcept
{
  level = _level;
}

/**
 * \ingroup GPCC_LOG
 * \brief Macro for invocation of [Logger::LogV()](@ref gpcc::log::Logger::LogV).\n
 *        The arguments will only be evaluated and LogV() will only be invoked if the log type is equal to or above
 *        the log level threshold configured at the logger.
 *
 * For details on usage, please refer to [Logger::LogV()](@ref gpcc::log::Logger::LogV).
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
 * \param logger
 * Reference to logger instance that shall be used to log the message.
 *
 * \param type
 * Type of log message. See [LogType](@ref gpcc::log::LogType) for details.\n
 * If this is below the log level configured at `logger`, then this method will do nothing.
 *
 * \param pFmt
 * Pointer to a null-terminated c-string containing the log message text and printf-style conversion specifications that
 * control how the variable arguments shall be converted and integrated into the log message text.\n
 * The life-time of the referenced string must extend at least until when the call to this method returns.
 *
 * \param ...
 * Variable number of arguments that shall be printed. The number and type of arguments must match the conversion
 * specifiers embedded in `pFmt`.
 */
#define LOGV(logger, type, pFmt, ...) \
do { \
  if ((logger).IsAboveLevel(type)) { \
    (logger).LogV((type), pFmt, __VA_ARGS__); \
} } while(false)

/**
 * \ingroup GPCC_LOG
 * \brief Macro for invocation of [Logger::LogVTS()](@ref gpcc::log::Logger::LogVTS).\n
 *        The arguments will only be evaluated and LogVTS() will only be invoked if the log type is equal to or
 *        above the log level threshold configured at the logger.
 *
 * For details on usage, please refer to [Logger::LogVTS()](@ref gpcc::log::Logger::LogVTS).
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
 * \param logger
 * Reference to logger instance that shall be used to log the message.
 *
 * \param type
 * Type of log message. See [LogType](@ref gpcc::log::LogType) for details.\n
 * If this is below the log level configured at `logger`, then this method will do nothing.
 *
 * \param pFmt
 * Pointer to a null-terminated c-string containing the log message text and printf-style conversion specifications that
 * control how the variable arguments shall be converted and integrated into the log message text.\n
 * The life-time of the referenced string must extend at least until when the call to this method returns.
 *
 * \param ...
 * Variable number of arguments that shall be printed. The number and type of arguments must match the conversion
 * specifiers embedded in `pFmt`.
 */
#define LOGVTS(logger, type, pFmt, ...) \
do { \
  if ((logger).IsAboveLevel(type)) { \
    (logger).LogVTS((type), pFmt, __VA_ARGS__); \
} } while(false)

} // namespace log
} // namespace gpcc

#endif // LOGGER_HPP_201701141240
