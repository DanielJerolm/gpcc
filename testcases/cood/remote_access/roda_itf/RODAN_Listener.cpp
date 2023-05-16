/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "RODAN_Listener.hpp"
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include <stdexcept>

namespace gpcc_tests {
namespace cood       {

using gpcc::log::LogType;

uint8_t const RODAN_Listener::loanExecContextDuration_ms;

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
 * \param _logger
 * Reference to a logger instance that shall be used by the listener to log messages.
 */
RODAN_Listener::RODAN_Listener(gpcc::log::Logger & _logger)
: IRemoteObjectDictionaryAccessNotifiable()
, logger(_logger)
, regUnregMutex()
, apiMutex()
, state(States::unregistered)
, anyError(false)
, nbOfCallsOnReady(0U)
, latestMaxRequestSize(0U)
, latestMaxResponseSize(0U)
, nbOfCallsOnDisconnected(0U)
, nbOfCallsOnRequestProcessed(0U)
, nbOfCallsLoanExecutionContext(0U)
, stateReadyCV()
, respAvailCV()
, responses()
, onLoanExecutionContext()
{
}

/**
 * \brief Destructor.
 *
 * Any responses that are still in the receive queue will be released.
 *
 * \pre   The listener is not registered at any RODA
 *        ([IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)) interface.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
RODAN_Listener::~RODAN_Listener(void)
{
  try
  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    if (state != States::unregistered)
      throw std::logic_error("'state' is not 'unregistered'");

    responses.ClearAndDestroyItems();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("RODAN_Listener::~RODAN_Listener: Failed: ", e);
  }
  catch (...)
  {
    gpcc::osal::Panic("RODAN_Listener::~RODAN_Listener: Caught an unknown exception");
  }
}

/**
 * \brief Registers the listener at a RODA ([IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess))
 *        interface.
 *
 * \pre   The listener is not registered at any RODA yet.
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
 * \param roda
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface where the listener shall
 * be registered at.
 */
void RODAN_Listener::Register(gpcc::cood::IRemoteObjectDictionaryAccess & roda)
{
  gpcc::osal::MutexLocker regUnregMutexLocker(regUnregMutex);

  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    if (state != States::unregistered)
      throw std::logic_error("RODAN_Listener::Register: Already registered.");

    state = States::notReady;
  }

  ON_SCOPE_EXIT(recoverState)
  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
    state = States::unregistered;
  };

  roda.Register(this);

  ON_SCOPE_EXIT_DISMISS(recoverState);

  logger.LogTS(LogType::Info, "RODAN_Listener::Register: Registered");
}

/**
 * \brief Unregisters the listener from a RODA
 *        ([IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)) interface.
 *
 * This is the counterpart of @ref Register().
 *
 * \pre    The listener is registered at the [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)
 *         interface referenced by `roda`.
 *
 * \post   The listener is not registered at `roda` any more.
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
 * \param roda
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface where the listener is
 * currently registered at.
 */
void RODAN_Listener::Unregister(gpcc::cood::IRemoteObjectDictionaryAccess & roda)
{
  logger.LogTS(LogType::Info, "RODAN_Listener::Unregister: Unregistering...");

  gpcc::osal::MutexLocker regUnregMutexLocker(regUnregMutex);

  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    if (state == States::unregistered)
      throw std::logic_error("RODAN_Listener::Unregister: Already unregistered");
  }

  roda.Unregister();

  try
  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
    state = States::unregistered;

    logger.LogTS(LogType::Info, "RODAN_Listener::Unregister: Unregistered");
  }
  catch (...)
  {
    gpcc::osal::Panic("RODAN_Listener::Unregister");
  }
}

/**
 * \brief Sets a callback that shall be invoked if the listener's `LoanExecutionContext()` method is invoked.
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
 * \param func
 * Pointer to a function that shall be invoked in the context of the listener's `LoanExecutionContext()` method.\n
 * Pass nullptr to unregister.
 */
void RODAN_Listener::SetOnLoanExecutionContext(std::function<void(void)> const & func)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  onLoanExecutionContext = func;
}

/**
 * \brief Queries if the listener has detected any error since creation of the listener.
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
 * \retval true   At least one error has been detected yet.
 * \retval false  No error has been detected yet.
 */
bool RODAN_Listener::AnyError(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return anyError;
}

/**
 * \brief Retrieves the current state of the listener.
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
 * Current state of the listener.
 */
RODAN_Listener::States RODAN_Listener::GetState(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return state;
}

/**
 * \brief Queries if the listener is currently registered at a RODA.
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
 * \retval true   The listener is registered at a RODA.
 * \retval false  The listener is not registered at any RODA.
 */
bool RODAN_Listener::IsRegistered(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return (state != States::unregistered);
}

/**
 * \brief Blocks the calling thread (with timeout) until the listener's state is @ref States::ready.
 *
 * This is intended to be invoked by one thread only. If multiple threads are blocked, then only one thread may be
 * woken up when the listener's state becomes @ref States::ready. Which thread is woken up is random.
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
 * \param timeout_ms
 * Timeout in ms until when the state must be @ref States::ready.
 *
 * \retval true   The listener's state is @ref States::ready.
 * \retval false  Timeout. The listener's state is not @ref States::ready.
 */
bool RODAN_Listener::WaitForStateReady(uint32_t const timeout_ms)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  if (state == States::ready)
    return true;

  auto timeout = gpcc::time::TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID);
  timeout += gpcc::time::TimeSpan::ms(timeout_ms);

  while (state != States::ready)
  {
    if (stateReadyCV.TimeLimitedWait(apiMutex, timeout))
    {
      // timeout, but check if States::Ready was reached
      return (state == States::ready);
    }
  }

  return true;
}

/**
 * \brief Retrieves the number of calls to `OnReady()` since creation of the listener.
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
 * Number of calls to `OnReady()` since creation of the listener.
 */
uint32_t RODAN_Listener::GetNbOfCallsOnReady(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return nbOfCallsOnReady;
}

/**
 * \brief Retrieves the value of parameter "maxRequestSize" of `OnReady()` from the latest call to `OnReady()`.
 *
 * \pre   There was at least one call to `OnReady()`.
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
 * Value of parameter "maxRequestSize" of `OnReady()` from latest call to `OnReady()`.
 */
size_t RODAN_Listener::GetMaxRequestSize(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  if (nbOfCallsOnReady == 0U)
    throw std::logic_error("RODAN_Listener::GetMaxRequestSize: There was no call to OnReady() yet.");

  return latestMaxRequestSize;
}

/**
 * \brief Retrieves the value of parameter "maxResponseSize" of `OnReady()` from the latest call to `OnReady()`.
 *
 * \pre   There was at least one call to `OnReady()`.
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
 * Value of parameter "maxResponseSize" of `OnReady()` from latest call to `OnReady()`.
 */
size_t RODAN_Listener::GetMaxResponseSize(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  if (nbOfCallsOnReady == 0U)
    throw std::logic_error("RODAN_Listener::GetMaxResponseSize: There was no call to OnReady() yet.");

  return latestMaxResponseSize;
}

/**
 * \brief Retrieves the number of calls to `OnDisconnected()` since creation of the listener.
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
 * Number of calls to `OnDisconnected()` since creation of the listener.
 */
uint32_t RODAN_Listener::GetNbOfCallsOnDisconnected(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return nbOfCallsOnDisconnected;
}

/**
 * \brief Retrieves the number of calls to `OnRequestProcessed()` since creation of the listener.
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
 * Number of calls to `OnRequestProcessed()` since creation of the listener.
 */
uint32_t RODAN_Listener::GetNbOfCallsOnRequestProcessed(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return nbOfCallsOnRequestProcessed;
}

/**
 * \brief Retrieves the number of calls to `LoanExecutionContext()` since creation of the listener.
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
 * Number of calls to `LoanExecutionContext()` since creation of the listener.
 */
uint32_t RODAN_Listener::GetNbOfCallsLoanExecutionContext(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return nbOfCallsLoanExecutionContext;
}

/**
 * \brief Blocks the calling thread (with timeout) until at least one response is available.
 *
 * This is intended to be invoked by one thread only. If multiple threads are blocked, then only one thread may be
 * woken up when a response becomes available. Which thread is woken up is random.
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
 * \param timeout_ms
 * Timeout in ms until when a response must be available.
 *
 * \retval true   A response is available.
 * \retval false  Timeout. No response available.
 */
bool RODAN_Listener::WaitForResponseAvailable(uint32_t const timeout_ms)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  if (!responses.empty())
    return true;

  auto timeout = gpcc::time::TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID);
  timeout += gpcc::time::TimeSpan::ms(timeout_ms);

  while (responses.empty())
  {
    if (respAvailCV.TimeLimitedWait(apiMutex, timeout))
    {
      // timeout, but check if a message is available
      return !responses.empty();
    }
  }

  return true;
}

/**
 * \brief Retrieves the number of available response messages.
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
 * Number of available response messages.
 */
size_t RODAN_Listener::GetNbOfAvailableResponses(void) const
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  return responses.size();
}

/**
 * \brief Pops a response from the queue.
 *
 * \pre   There is at least one response available.
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
 * Popped response message. Ownership moves to the caller.\n
 * Note that the queue works in FIFO order.
 */
std::unique_ptr<gpcc::cood::ResponseBase> RODAN_Listener::PopResponse(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  if (responses.empty())
    throw std::logic_error("RODAN_Listener::PopResponse: Empty");

  std::unique_ptr<gpcc::cood::ResponseBase> spResponse(responses.front());
  responses.pop_front();

  return spResponse;
}

// <-- IRemoteObjectDictionaryAccessNotifiable

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady()
void RODAN_Listener::OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  nbOfCallsOnReady++;
  latestMaxRequestSize = maxRequestSize;
  latestMaxResponseSize = maxResponseSize;

  switch (state)
  {
    case States::unregistered:
      logger.LogTS(LogType::Error, "RODAN_Listener::OnReady: Called, but state is 'unregistered'");
      anyError = true;
      break;

    case States::notReady:
      logger.LogTS(LogType::Info, "RODAN_Listener::OnReady: READY (maxRequestSize = " + std::to_string(maxRequestSize) + ", maxResponseSize = " + std::to_string(maxResponseSize) + ")");
      state = States::ready;
      break;

   case States::ready:
      logger.LogTS(LogType::Error, "RODAN_Listener::OnReady: Called, but state is already 'ready'");
      anyError = true;
      break;
  }
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnDisconnected()
void RODAN_Listener::OnDisconnected(void) noexcept
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  nbOfCallsOnDisconnected++;

  switch (state)
  {
    case States::unregistered:
      logger.LogTS(LogType::Error, "RODAN_Listener::OnDisconnected: Called, but state is 'unregistered'");
      anyError = true;
      break;

    case States::notReady:
      logger.LogTS(LogType::Error, "RODAN_Listener::OnDisconnected: Called, but state is already 'notReady'");
      anyError = true;
      break;

    case States::ready:
      logger.LogTS(LogType::Info, "RODAN_Listener::OnDisconnected: DISCONNECTED");
      state = States::notReady;
      break;
  }
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed()
void RODAN_Listener::OnRequestProcessed(std::unique_ptr<gpcc::cood::ResponseBase> spResponse) noexcept
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  nbOfCallsOnRequestProcessed++;

  switch (state)
  {
    case States::unregistered:
      logger.LogTS(LogType::Error, "RODAN_Listener::OnRequestProcessed: Called, but state is 'unregistered'");
      anyError = true;
      break;

    case States::notReady:
      logger.LogTS(LogType::Error, "RODAN_Listener::OnRequestProcessed: Called, but state is 'notReady'");
      anyError = true;
      break;

    case States::ready:
      logger.LogTS(LogType::Debug, "RODAN_Listener::OnRequestProcessed: Received response");
      break;
  }

  respAvailCV.Signal();
  responses.push_back(spResponse.get());
  spResponse.release();
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext()
void RODAN_Listener::LoanExecutionContext(void) noexcept
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);
  nbOfCallsLoanExecutionContext++;

  switch (state)
  {
    case States::unregistered:
      logger.LogTS(LogType::Error, "RODAN_Listener::LoanExecutionContext: Called, but state is 'unregistered'");
      anyError = true;
      break;

    case States::notReady:
      logger.LogTS(LogType::Error, "RODAN_Listener::LoanExecutionContext: Called, but state is 'notReady'");
      anyError = true;
      break;

    case States::ready:
      logger.LogTS(LogType::Debug, "RODAN_Listener::LoanExecutionContext: Called");
      break;
  }

  gpcc::osal::Thread::Sleep_ms(loanExecContextDuration_ms);

  if (onLoanExecutionContext != nullptr)
    onLoanExecutionContext();
}

// --> IRemoteObjectDictionaryAccessNotifiable

} // namespace cood
} // namespace gpcc_tests
