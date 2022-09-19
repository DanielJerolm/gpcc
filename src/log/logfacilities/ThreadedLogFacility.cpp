/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/log/logfacilities/ThreadedLogFacility.hpp>
#include <gpcc/log/backends/Backend.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "src/log/internal/LogMessage.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace gpcc {
namespace log  {

/**
 * \brief Constructor.
 *
 * After instantiation, consider using @ref SetDefaultSettings() to setup default log levels for
 * the @ref Logger instances that will be registered at this log facility.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param pThreadName
 * Pointer to a null-terminated c-string with the name that shall be assigned to the log facilities' thread.
 * The referenced string must not change during the lifetime of the @ref ThreadedLogFacility instance.\n
 * Usually you will use a string located in ROM or code memory.
 *
 * \param capacity
 * The maximum number of enqueued debug/OK/info/warn-messages is limited to this value.\n
 * If the contingent of debug/OK/info/warn-messages is exhausted, then new log messages of these types will
 * be dropped.\n
 * The limitation is not applied to error- and fatal-messages.\n
 * Minimum value: 8
 */
ThreadedLogFacility::ThreadedLogFacility(char const * const pThreadName, size_t const capacity)
: ILogFacility()
, ILogFacilityCtrl()
, mutex()
, msgListMutex()
, pLoggerList(nullptr)
, pBackendList(nullptr)
, defaultSettingsPresent(false)
, defaultSettings()
, notProperlyDeliveredMessages(0)
, messageCreationFailureCnt(0)
, droppedMessages(0)
, remainingCapacity(capacity)
, msgListNotEmptyCV()
, busy(false)
, notBusyAndEmptyCV()
, pMsgQueueHead(nullptr)
, pMsgQueueTail(nullptr)
, thread(pThreadName)
{
  if (capacity < 8U)
    throw std::invalid_argument("ThreadedLogFacility::ThreadedLogFacility: invalid capacity");
}

/**
 * \brief Destructor.
 *
 * \pre   There are no loggers registered.
 *
 * \pre   There are no backends registered.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ThreadedLogFacility::~ThreadedLogFacility(void)
{
  // ensure, that there is not any Logger or Backend still registered here
  gpcc::osal::MutexLocker mutexLocker(mutex);
  if ((pLoggerList != nullptr) || (pBackendList != nullptr))
    PANIC();

  // release any queued message
  gpcc::osal::MutexLocker msgListMutexLocker(msgListMutex);
  ReleaseMessages(pMsgQueueHead);
  pMsgQueueHead = nullptr;
  pMsgQueueTail = nullptr;
}

/**
 * \brief Starts the log facility.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param schedPolicy
 * Scheduling policy that shall be used for the log facilities' thread.\n
 * See @ref gpcc::osal::Thread::Start() for details.
 *
 * \param priority
 * Priority level (0 (low) .. 31 (high)) that shall be used for the log facilities' thread.\n
 * This is only relevant for the scheduling policies `SchedPolicy::Fifo` and `SchedPolicy::RR`.\n
 * _For the other scheduling policies this must be zero._\n
 * See @ref gpcc::osal::Thread::Start() for details.
 *
 * \param stackSize
 * Size of the stack in byte that shall be allocated for the log facilities' thread.\n
 * _This must be a multiple of_ @ref gpcc::osal::Thread::GetStackAlign(). \n
 * _This must be equal to or larger than_ @ref gpcc::osal::Thread::GetMinStackSize(). \n
 * Internally this may be round up to some quantity, e.g. the system's page size.\n
 * See @ref gpcc::osal::Thread::Start() for details.
 */
void ThreadedLogFacility::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                                gpcc::osal::Thread::priority_t const priority,
                                size_t const stackSize)
{
  thread.Start(std::bind(&ThreadedLogFacility::InternalThreadEntry, this), schedPolicy, priority, stackSize);
}

/**
 * \brief Stops log facility and blocks until the log facility has stopped.
 *
 * After this has returned, it is safe to restart the log facility via @ref Start().
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
 * Deferred cancellation is not allowed.
 */
void ThreadedLogFacility::Stop(void) noexcept
{
  {
    gpcc::osal::MutexLocker msgListMutexLocker(msgListMutex);
    thread.Cancel();
    msgListNotEmptyCV.Signal();
  }

  thread.Join();
}

/**
 * \brief Blocks the calling thread until all log messages are processed.
 *
 * This also blocks until log messages which logged while this method is already blocked are processed.
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
 * Strong guarantee.
 */
void ThreadedLogFacility::Flush(void)
{
  gpcc::osal::MutexLocker msgListMutexLocker(msgListMutex);
  while ((busy) || (pMsgQueueHead != nullptr) || (messageCreationFailureCnt != 0U) || (droppedMessages != 0U))
    notBusyAndEmptyCV.Wait(msgListMutex);
}

/// \copydoc ILogFacility::Register(Logger& logger)
void ThreadedLogFacility::Register(Logger& logger)
{
  bool complainNoDefaultLogLevel = false;

  {
    gpcc::osal::MutexLocker pLoggerMutexLocker(logger.mutex);

    if (logger.pLogFacility != nullptr)
      throw std::logic_error("ThreadedLogFacility::Register: Logger already registered");

    gpcc::osal::MutexLocker mutexLocker(mutex);

    // retrieve default log level for the new logger
    auto defaultSettingsIterator = defaultSettings.end();
    if (defaultSettingsPresent)
    {
      complainNoDefaultLogLevel = true;
      for (defaultSettingsIterator = defaultSettings.begin(); defaultSettingsIterator != defaultSettings.end(); ++defaultSettingsIterator)
      {
        if ((*defaultSettingsIterator).first == logger.srcName.GetStr())
        {
          complainNoDefaultLogLevel = false;
          break;
        }
      }
    }

    // logger-list empty?
    if (pLoggerList == nullptr)
    {
      logger.pNext        = nullptr;
      logger.pPrev        = nullptr;
      logger.pLogFacility = this;

      pLoggerList = &logger;
    } // if (pLoggerList == nullptr)
    else
    {
      Logger* p = pLoggerList;

      while (true)
      {
        auto const cmpResult = logger.srcName.GetStr().compare(p->srcName.GetStr());
        if (cmpResult < 0)
        {
          // insert the new logger in front of "p"
          logger.pPrev = p->pPrev;
          logger.pNext = p;
          p->pPrev = &logger;
          if (logger.pPrev != nullptr)
            logger.pPrev->pNext = &logger;
          else
            pLoggerList = &logger;
          break;
        }
        else if (cmpResult == 0)
          throw std::logic_error("ThreadedLogFacility::Register: There is already a Logger with the same name");

        // end of list not yet reached?
        if (p->pNext != nullptr)
        {
          p = p->pNext;
        }
        else
        {
          // end of list reached, insert the new logger after "p"
          logger.pPrev = p;
          logger.pNext = nullptr;
          p->pNext = &logger;
          break;
        }
      }

      logger.pLogFacility = this;
    } // if (pLoggerList == nullptr)... else...

    // consume and apply default settings if necessary
    if (defaultSettingsIterator != defaultSettings.end())
    {
      logger.level = (*defaultSettingsIterator).second;
      defaultSettings.erase(defaultSettingsIterator);
    }
  }

  if (complainNoDefaultLogLevel)
    logger.Log(LogType::Warning, "No default log level deposited.");
}

/// \copydoc ILogFacility::Unregister(Logger& logger)
void ThreadedLogFacility::Unregister(Logger& logger)
{
  gpcc::osal::MutexLocker pLoggerMutexLocker(logger.mutex);

  if (logger.pLogFacility != this)
    throw std::logic_error("ThreadedLogFacility::Unregister: Logger not registered here");

  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (logger.pPrev != nullptr)
   logger.pPrev->pNext = logger.pNext;
  if (logger.pNext != nullptr)
    logger.pNext->pPrev = logger.pPrev;

  if (&logger == pLoggerList)
    pLoggerList = logger.pNext;

  logger.pLogFacility = nullptr;
}

/// \copydoc ILogFacility::Register(Backend& backend)
void ThreadedLogFacility::Register(Backend& backend)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  bool expected = false;
  if (!backend.registered.compare_exchange_strong(expected, true))
    throw std::invalid_argument("ThreadedLogFacility::RegisterBackend: Backend already registered somewhere");

  backend.pNext = pBackendList;
  pBackendList = &backend;
}

/// \copydoc ILogFacility::Unregister(Backend& backend)
void ThreadedLogFacility::Unregister(Backend& backend)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  // look for backend in the list of registered back-ends
  Backend* pPrev = nullptr;
  Backend* pCurr = pBackendList;
  while ((pCurr != nullptr) && (pCurr != &backend))
  {
    pPrev = pCurr;
    pCurr = pCurr->pNext;
  }

  // not found?
  if (pCurr == nullptr)
    throw std::logic_error("ThreadedLogFacility::Unregister: Backend not registered here");

  // first entry in list?
  if (pPrev == nullptr)
    pBackendList = pCurr->pNext;
  else
    pPrev->pNext = pCurr->pNext;
  pCurr->pNext = nullptr;

  backend.registered = false;
}

/// \copydoc ILogFacility::Log
void ThreadedLogFacility::Log(std::unique_ptr<internal::LogMessage> spMsg)
{
  if (!spMsg)
    throw std::invalid_argument("ThreadedLogFacility::Log: !spMsg");

  if (spMsg->pNext != nullptr)
    throw std::logic_error("ThreadedLogFacility::Log: Bad spMsg->pNext");

  gpcc::osal::MutexLocker msgListMutexLocker(msgListMutex);

  if ((remainingCapacity != 0U) ||
      (static_cast<LogType>(spMsg->type) == LogType::Error) ||
      (static_cast<LogType>(spMsg->type) == LogType::Fatal))
  {
    if (pMsgQueueTail == nullptr)
    {
      msgListNotEmptyCV.Signal();

      pMsgQueueHead = spMsg.get();
      pMsgQueueTail = spMsg.release();
    }
    else
    {
      pMsgQueueTail->pNext = spMsg.get();
      pMsgQueueTail = spMsg.release();
    }

    if ((static_cast<LogType>(pMsgQueueTail->type) != LogType::Error) &&
        (static_cast<LogType>(pMsgQueueTail->type) != LogType::Fatal))
      --remainingCapacity;
  }
  else
  {
    if (droppedMessages != std::numeric_limits<decltype(droppedMessages)>::max())
      droppedMessages++;
  }
}

/// \copydoc ILogFacility::ReportLogMessageCreationFailed
void ThreadedLogFacility::ReportLogMessageCreationFailed(void) noexcept
{
  try
  {
    gpcc::osal::MutexLocker msgListMutexLocker(msgListMutex);

    if (messageCreationFailureCnt == 0U)
      msgListNotEmptyCV.Signal();

    if (messageCreationFailureCnt != std::numeric_limits<decltype(messageCreationFailureCnt)>::max())
      messageCreationFailureCnt++;
  }
  catch (std::exception const &)
  {
    // intentionally empty
  }
  catch (...)
  {
    PANIC();
  }
}

/// \copydoc ILogFacilityCtrl::EnumerateLogSources
std::vector<ILogFacilityCtrl::tLogSrcConfig> ThreadedLogFacility::EnumerateLogSources(void) const
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  // determine number of registered Logger instances
  size_t n = 0U;
  Logger* p = pLoggerList;
  while (p != nullptr)
  {
    n++;
    p = p->pNext;
  }

  // prepare vector
  std::vector<ILogFacilityCtrl::tLogSrcConfig> v;
  v.reserve(n);

  // fill vector
  p = pLoggerList;
  while (p != nullptr)
  {
    v.push_back(ILogFacilityCtrl::tLogSrcConfig(p->srcName.GetStr(), p->GetLogLevel()));
    p = p->pNext;
  }

  return v;
}

/// \copydoc ILogFacilityCtrl::GetLogLevel
LogLevel ThreadedLogFacility::GetLogLevel(std::string const & srcName) const
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  Logger const * const p = FindLogger(srcName);
  if (p == nullptr)
    throw std::invalid_argument("GetLogLevel: No such log source");

  return p->GetLogLevel();
}

/// \copydoc ILogFacilityCtrl::SetLogLevel
bool ThreadedLogFacility::SetLogLevel(std::string const & srcName, LogLevel const level)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  Logger* const p = FindLogger(srcName);
  if (p == nullptr)
    return false;

  p->SetLogLevel(level);
  return true;
}

/// \copydoc ILogFacilityCtrl::LowerLogLevel
bool ThreadedLogFacility::LowerLogLevel(std::string const & srcName, LogLevel const level)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  Logger* const p = FindLogger(srcName);
  if (p == nullptr)
    return false;

  p->LowerLogLevel(level);
  return true;
}

/// \copydoc ILogFacilityCtrl::RaiseLogLevel
bool ThreadedLogFacility::RaiseLogLevel(std::string const & srcName, LogLevel const level)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  Logger* const p = FindLogger(srcName);
  if (p == nullptr)
    return false;

  p->RaiseLogLevel(level);
  return true;
}

/// \copydoc ILogFacilityCtrl::SetDefaultSettings
void ThreadedLogFacility::SetDefaultSettings(std::vector<tLogSrcConfig> _defaultSettings)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  defaultSettings = std::move(_defaultSettings);
  defaultSettingsPresent = true;
}

/// \copydoc ILogFacilityCtrl::RemoveDefaultSettings
std::vector<ILogFacilityCtrl::tLogSrcConfig> ThreadedLogFacility::RemoveDefaultSettings(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (!defaultSettingsPresent)
    defaultSettings.clear();

  defaultSettingsPresent = false;
  return std::move(defaultSettings);
}

/**
 * \brief Retrieves a logger from the list of registered loggers based on the log source name.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param srcName
 * Name of the log source.
 *
 * \return
 * Pointer to the logger with the source name `srcName`.\n
 * nullptr, if no matching logger is found.
 */
Logger* ThreadedLogFacility::FindLogger(std::string const & srcName) const noexcept
{
  Logger* p = pLoggerList;

  while ((p != nullptr) && (p->srcName.GetStr() != srcName))
    p = p->pNext;

  return p;
}

/**
 * \brief Entry function for the log facilities' thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not applicable. Program logic ensures that there can only be one thread at any time.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Disabled.
 *
 * - - -
 *
 * \return
 * Always `nullptr`.
 */
void* ThreadedLogFacility::InternalThreadEntry(void) noexcept
{
  try
  {
    thread.SetCancelabilityEnabled(false);

    gpcc::osal::AdvancedMutexLocker msgListMutexLocker(msgListMutex);
    while (!thread.IsCancellationPending())
    {
      // wait for something to log
      while ((pMsgQueueHead == nullptr) && (messageCreationFailureCnt == 0U) && (droppedMessages == 0U))
      {
        msgListNotEmptyCV.Wait(msgListMutex);

        if (thread.IsCancellationPending())
          return nullptr;
      }

      // fetch messages, messageCreationFailureCnt and droppedMessages into local variables
      auto pMessages = pMsgQueueHead;
      pMsgQueueHead = nullptr;
      pMsgQueueTail = nullptr;

      auto const local_messageCreationFailureCnt = messageCreationFailureCnt;
      messageCreationFailureCnt = 0U;

      auto const local_droppedMessages = droppedMessages;
      droppedMessages = 0U;

      // process
      busy = true;
      msgListMutexLocker.Unlock();

      DeliverMessages(pMessages, local_droppedMessages, local_messageCreationFailureCnt);

      msgListMutexLocker.Relock();
      busy = false;

      // wake up potential threads in Flush(), if there is nothing more to do
      if ((pMsgQueueHead == nullptr) && (messageCreationFailureCnt == 0U) && (droppedMessages == 0U))
        notBusyAndEmptyCV.Broadcast();
    }
  }
  catch (...)
  {
    // note: cancelability is disabled, so catch (...) will not interfere with deferred thread cancellation
    PANIC();
  }

  return nullptr;
}

/**
 * \brief Releases all @ref internal::LogMessage objects in a chain of messages.
 *
 * @ref remainingCapacity will be incremented by this depending on the type of each released message.
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
 * \param pMessages
 * Pointer to the first log message in a chain of log messages.\n
 * The chain of log messages is made up by `LogMessage::pNext`.\n
 * `nullptr` is allowed.
 */
void ThreadedLogFacility::ReleaseMessages(internal::LogMessage* pMessages) noexcept
{
  while (pMessages != nullptr)
  {
    auto pNext = pMessages->pNext;

    if ((static_cast<LogType>(pMessages->type) != LogType::Error) &&
        (static_cast<LogType>(pMessages->type) != LogType::Fatal))
    {
      ++remainingCapacity;
    }

    delete pMessages;

    pMessages = pNext;
  }
}

/**
 * \brief Delivers all @ref internal::LogMessage objects in a chain of messages to the registered back-ends.
 *
 * Delivery encompasses:
 * - building the log message text string
 * - passing the log message text string to each registered back-end
 *
 * If an error occurs during building the log message text string, then the counter for not properly delivered
 * log messages (@ref notProperlyDeliveredMessages) will be incremented.
 *
 * If an error occurs during passing a log message text to a back-end, then the counter for not properly delivered
 * log messages (@ref notProperlyDeliveredMessages) will be incremented, too. The message text will be passed to all
 * back-ends, even in case of an error ocurred at one or more back-ends. The error counter will be incremented once
 * per message, not once per back-end.
 *
 * If the counter for not properly delivered messages (@ref notProperlyDeliveredMessages) indicates that at least
 * one log message was not properly delivered, then a special log message indicating the error condition and the
 * number of incidents will be created and passed to all back-ends via @ref Deliver(). The counter will be cleared
 * before invoking @ref Deliver(). If an error occurs during delivery of the special log message, then
 * @ref notProperlyDeliveredMessages will be incremented by @ref Deliver() and there will be another attempt to
 * create and deliver a error log message at a later point in time.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - All messages may be dropped (very unlikely)
 * - In any case, there is no memory leak or undefined state
 *
 * Any exception thrown by a back-end or thrown during building a log message text is properly handled here and
 * will __not__ be forwarded to the caller.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Not all log messages may be delivered (= log messages may be dropped).
 * - The currently processed log message may not be delivered to all registered @ref Backend instances.
 * - The currently processed log message may have been processed only partially by a back-end.
 * - An error log message indicating that log messages have been dropped may be dropped, too. The information about
 *   this incident is lost then.
 * - An error log message indicating that log message creation has failed may be dropped. The information about this
 *   incident is lost then.
 * - In any case, there is no memory leak or undefined state
 *
 * - - -
 *
 * \param pMessages
 * Pointer to the first log message in a chain of log messages.\n
 * The chain of log messages is made up by `LogMessage::pNext`.\n
 * `nullptr` is allowed.\n
 * Note that ownership moves to this method. The log messages will be released even in case of an exception.
 *
 * \param dropped
 * Number of log messages dropped due to limitation of the capacity of the log message queue.
 *
 * \param creationFailed
 * Number of times the creation of a log message failed.
 */
void ThreadedLogFacility::DeliverMessages(internal::LogMessage* pMessages, uint8_t const dropped, uint8_t const creationFailed)
{
  ON_SCOPE_EXIT(releaseMessages) { ReleaseMessages(pMessages); };

  gpcc::osal::MutexLocker mutexLocker(mutex);

  // add "dropped" to "notProperlyDeliveredMessages" and limit properly
  notProperlyDeliveredMessages = std::min(static_cast<size_t>(dropped) + notProperlyDeliveredMessages,
                                          static_cast<size_t>(std::numeric_limits<decltype(notProperlyDeliveredMessages)>::max()));

  while (pMessages != nullptr)
  {
    // fetch one message from list
    std::unique_ptr<internal::LogMessage> spMsg(pMessages);
    pMessages = pMessages->pNext;

    if ((static_cast<LogType>(spMsg->type) != LogType::Error) &&
        (static_cast<LogType>(spMsg->type) != LogType::Fatal))
    {
      ++remainingCapacity;
    }

    try
    {
      // get message type and build message text
      LogType type;
      std::string message;

      type    = spMsg->GetLogType();
      message = spMsg->BuildText();
      spMsg.reset();

      // deliver to back-ends
      Deliver(message, type);
    }
    catch (std::exception const &)
    {
      IncNotProperlyDeliveredMessages();
    }
  }

  ON_SCOPE_EXIT_DISMISS(releaseMessages);

  // create an additional error message if any message has been dropped or not properly processed
  if (notProperlyDeliveredMessages != 0U)
  {
    try
    {
      std::string message("[ERROR] *** Logger: ");
      if (notProperlyDeliveredMessages < std::numeric_limits<decltype(notProperlyDeliveredMessages)>::max())
        message += std::to_string(notProperlyDeliveredMessages);
      else
        message += "At least 255";
      static_assert(std::numeric_limits<decltype(notProperlyDeliveredMessages)>::max() == 255U, "Check number of messages in text string above");
      message += " not (properly) delivered message(s)! ***";

      // If we reach this, then the error message was properly build. The error counter can be cleared then.
      // In case of an error when passing the message to the back-ends, the counter will be incremented by Deliver().
      // This will result in another attempt to create another error message later.
      notProperlyDeliveredMessages = 0U;

      Deliver(message, LogType::Error);
    }
    catch (std::exception const &)
    {
      // ignore
    }
  } // if (notProperlyDeliveredMessages != 0U)

  // create an additional error message if there was any error during creation of a log message
  if (creationFailed != 0U)
  {
    try
    {
      std::string message("[ERROR] *** Logger: ");
      if (creationFailed < std::numeric_limits<decltype(creationFailed)>::max())
        message += std::to_string(creationFailed);
      else
        message += "At least 255";
      static_assert(std::numeric_limits<decltype(creationFailed)>::max() == 255U, "Check number of messages in text string above");
      message += " error(s) during log message creation (e.g. out-of-memory) ***";

      Deliver(message, LogType::Error);
    }
    catch (std::exception const &)
    {
      IncNotProperlyDeliveredMessages();
    }
  } // if (creationFailed != 0U)
}

/**
 * \brief Delivers a message to all registered back-ends.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * Exceptions thrown by back-ends will be properly caught and handled by this.\n
 * In case of any error, @ref notProperlyDeliveredMessages will be incremented.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the log message may not have been delivered to all registered back-ends
 * - a back-end may have processed only parts of the log message
 *
 * - - -
 *
 * \param msg
 * Message to be send to all registered back-ends.
 *
 * \param type
 * Type of message.
 */
void ThreadedLogFacility::Deliver(std::string const & msg, LogType const type)
{
  Backend* pBackend = pBackendList;
  bool error = false;
  while (pBackend != nullptr)
  {
    try
    {
      pBackend->Process(msg, type);
    }
    catch (std::exception const &)
    {
      error = true;
    }

    pBackend = pBackend->pNext;
  }

  if (error)
    IncNotProperlyDeliveredMessages();
}

/**
 * \brief Increments @ref notProperlyDeliveredMessages and stops at maximum value to prevent overflow.
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref mutex must be locked.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void ThreadedLogFacility::IncNotProperlyDeliveredMessages(void) noexcept
{
  if (notProperlyDeliveredMessages != std::numeric_limits<decltype(notProperlyDeliveredMessages)>::max())
    notProperlyDeliveredMessages++;
}

} // namespace log
} // namespace gpcc
