/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef THREADEDLOGFACILITY_HPP_201701061541
#define THREADEDLOGFACILITY_HPP_201701061541

#include "ILogFacility.hpp"
#include "ILogFacilityCtrl.hpp"
#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <atomic>
#include <cstdint>
#include <cstddef>

namespace gpcc {
namespace log  {

/**
 * \ingroup GPCC_LOG_LOGFACILITIES
 * \brief Thread-based log facility.
 *
 * One or more @ref Logger instances can be registered at this log facility and emit log messages
 * (instances of sub-classes of class [LogMessage](@ref internal::LogMessage)) to the log facility.\n
 * The log facility passes all incoming log messages to all the back-ends (instances of sub-classes
 * of class @ref Backend), which are registered at the log facility.
 *
 * This log facility implementation has an own thread for building log message strings and for
 * passing the log message strings to the registered back-ends.
 *
 * # Log message limitation
 * Log message delivery is decoupled from log message creation. This means, that log messages are
 * enqueued in the @ref ThreadedLogFacility before they are finally processed.
 *
 * The number of enqueued log messages is limited. The limit is setup via parameter `capacity` passed
 * to this class' constructor.
 *
 * If the limit is exceeded, then new log messages will be dropped. If any log message is dropped, then a
 * special error message will be generated and send to all back-ends registered at the log facility.
 * This ensures, that users are informed if any log messages are dropped.
 *
 * Messages of type @ref LogType::Error and @ref LogType::Fatal are not affected by the limitation.
 * The number of enqueued @ref LogType::Error and @ref LogType::Fatal messages is only limited by the
 * resources of the system.
 *
 * # Errors during log message creation
 * Errors may occur during log message text creation at the user, and during log message creation inside the
 * @ref Logger instance. These errors are mostly `std::bad::alloc`.
 *
 * User may report these errors to the @ref Logger instance. The @ref Logger instance reports all errors
 * to the log facility. The log facility will count these errors and the log facility will generate a special
 * log message which indicates the number of reported errors.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ThreadedLogFacility final: public ILogFacility, public ILogFacilityCtrl
{
  public:
    ThreadedLogFacility(char const * const pThreadName, size_t const capacity);
    ThreadedLogFacility(ThreadedLogFacility const &) = delete;
    ThreadedLogFacility(ThreadedLogFacility &&) = delete;
    ~ThreadedLogFacility(void);

    ThreadedLogFacility& operator=(ThreadedLogFacility const &) = delete;
    ThreadedLogFacility& operator=(ThreadedLogFacility &&) = delete;


    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

    void Flush(void);


    // --> ILogFacility
    void Register(Logger& logger) override;
    void Unregister(Logger& logger) override;

    void Register(Backend& backend) override;
    void Unregister(Backend& backend) override;

    void Log(std::unique_ptr<internal::LogMessage> spMsg) override;
    void ReportLogMessageCreationFailed(void) noexcept override;
    // <-- ILogFacility

    // --> ILogFacilityCtrl
    std::vector<tLogSrcConfig> EnumerateLogSources(void) const override;
    LogLevel GetLogLevel(std::string const & srcName) const override;
    bool SetLogLevel(std::string const & srcName, LogLevel const level) override;
    bool LowerLogLevel(std::string const & srcName, LogLevel const level) override;
    bool RaiseLogLevel(std::string const & srcName, LogLevel const level) override;

    void SetDefaultSettings(std::vector<tLogSrcConfig> _defaultSettings) override;
    std::vector<tLogSrcConfig> RemoveDefaultSettings(void) override;
    // <-- ILogFacilityCtrl

  private:
    /// Mutex protecting access to logger- and backend-lists.
    /** Locking order: @ref Logger::mutex -> @ref mutex -> @ref msgListMutex */
    mutable gpcc::osal::Mutex mutex;

    /// Mutex protecting access to the log-message queue.
    /** Locking order: @ref Logger::mutex -> @ref mutex -> @ref msgListMutex */
    gpcc::osal::Mutex msgListMutex;


    /// List containing registered loggers.
    /** @ref mutex is required.\n
        The loggers in this list are sorted alphabetically and upper-case before lower-case.\n
        The pPrev-pointers of the loggers point toward this. */
    Logger* pLoggerList;

    /// List containing registered backends.
    /** @ref mutex is required.\n
        The pNext-pointers of the backends point away from this. */
    Backend* pBackendList;

    /// Flag indicating if default settings for new registered @ref Logger instances are present.
    /** @ref mutex is required. */
    bool defaultSettingsPresent;

    /// List of default log levels for new registered @ref Logger instances.
    /** @ref mutex is required. */
    std::vector<tLogSrcConfig> defaultSettings;

    /// Number of undelivered messages.
    /** @ref mutex is required.\n
        This variable contains the number of completely or partially undelivered log messages.\n
        The number includes:
        - errors during message text creation from message ingredients
        - errors during message processing by back-ends
        - dropped messages due to limited capacity */
    uint8_t notProperlyDeliveredMessages;


    /// Number of times a @ref Logger or user of a @ref Logger failed to create a log message,
    /// e.g. due to out-of-memory.
    /** @ref msgListMutex is required. */
    uint8_t messageCreationFailureCnt;

    /// Number of log messages dropped due to log message queue limitation.
    /** @ref msgListMutex is required. */
    uint8_t droppedMessages;

    /// Remaining contingent of log messages which are not @ref LogType::Error or @ref LogType::Fatal.
    /** For decrementing, @ref msgListMutex is required. */
    std::atomic<size_t> remainingCapacity;

    /// Condition variable for signaling that either the message queue is no longer empty, that
    /// @ref messageCreationFailureCnt is no longer zero, or @ref droppedMessages is no longer zero.
    /** This is to be used in conjunction with @ref msgListMutex. */
    gpcc::osal::ConditionVariable msgListNotEmptyCV;

    /// Flag indicating if the log facility is currently busy with delivery of log messages.
    /** @ref msgListMutex is required. */
    bool busy;

    /// Condition variable for signaling that the log facility is idle.
    /** This is to be used in conjunction with @ref msgListMutex. \n
        This signals the following condition: The message queue is empty, @ref busy is false,
        @ref messageCreationFailureCnt is zero, and @ref droppedMessages is zero. */
    gpcc::osal::ConditionVariable notBusyAndEmptyCV;

    /// Message queue head. Messages are removed here.
    /** @ref msgListMutex is required. */
    internal::LogMessage* pMsgQueueHead;

    /// Message queue tail. New messages are added here.
    /** @ref msgListMutex is required.\n
        The pNext-pointer of the log messages points toward this. */
    internal::LogMessage* pMsgQueueTail;

    /// Thread used to process log messages.
    gpcc::osal::Thread thread;


    Logger* FindLogger(std::string const & srcName) const noexcept;

    void* InternalThreadEntry(void) noexcept;

    void ReleaseMessages(internal::LogMessage* pMessages) noexcept;
    void DeliverMessages(internal::LogMessage* pMessages, uint8_t const dropped, uint8_t const creationFailed);
    void Deliver(std::string const & msg, LogType const type);

    void IncNotProperlyDeliveredMessages(void) noexcept;
};

} // namespace log
} // namespace gpcc

#endif // THREADEDLOGFACILITY_HPP_201701061541
