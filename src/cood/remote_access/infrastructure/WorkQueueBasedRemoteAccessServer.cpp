/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "WorkQueueBasedRemoteAccessServer.hpp"
#include <gpcc/log/Logger.hpp>
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <functional>
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
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
 * \param _dwq
 * Deferred work queue that shall be used as execution context.\n
 * __The work queue's thread must have deferred cancellation disabled!__
 *
 * \param _oomRetryDelay_ms
 * Delay (in ms) before a retry is performed after an out-of-memory condition.\n
 * Retries are only performed if there is no way to report the out-of-memory condition to the originator of a
 * remote access request.\n
 * Zero is not allowed.
 *
 * \param _od
 * Interface that shall be used to access the object dictionary.
 *
 * \param _pLogger
 * Pointer to a logger that shall be used to log messages.\n
 * nullptr, if logging is not required.\n
 * If a logger is provided, then it shall have a meaningful name and it should be assigned exclusively to the remote
 * access server to avoid confusing log messages.
 *
 * \param _maxRequestSize
 * Maximum size (in byte) of a serialized request object (incl. any @ref ReturnStackItem objects) that can be processed
 * by the server.\n
 * The value will be passed to the OnReady(...)-callback of any client registering at the provided
 * @ref IRemoteObjectDictionaryAccess interface.\n
 * Minimum value: @ref RequestBase::minimumUsefulRequestSize \n
 * Maximum value: @ref RequestBase::maxRequestSize
 *
 * \param _maxResponseSize
 * Maximum size (in byte) of a serialized response object (incl. any @ref ReturnStackItem objects) that can be created
 * by the server and send back to the client.\n
 * The value will be passed to the OnReady(...)-callback of any client registering at the provided
 * @ref IRemoteObjectDictionaryAccess interface.\n
 * Minimum value: @ref ResponseBase::minimumUsefulResponseSize \n
 * Maximum value: @ref ResponseBase::maxResponseSize
 */
WorkQueueBasedRemoteAccessServer::WorkQueueBasedRemoteAccessServer(gpcc::execution::async::IDeferredWorkQueue & _dwq,
                                                                   uint8_t const _oomRetryDelay_ms,
                                                                   IObjectAccess & _od,
                                                                   gpcc::log::Logger * const _pLogger,
                                                                   size_t const _maxRequestSize,
                                                                   size_t const _maxResponseSize)
: RemoteAccessServer(_od, _pLogger, _maxRequestSize, _maxResponseSize)
, dwq(_dwq)
, oomRetryDelay_ms(_oomRetryDelay_ms)
, startStopMutex()
, internalMutex()
, state(States::Off)
, wp(this, 0U, std::bind(&WorkQueueBasedRemoteAccessServer::WQentry, this))
, dwp(this, 0U, std::bind(&WorkQueueBasedRemoteAccessServer::WQentry, this))
{
  if (oomRetryDelay_ms == 0U)
    throw std::invalid_argument("WorkQueueBasedRemoteAccessServer::WorkQueueBasedRemoteAccessServer: oomRetryDelay_ms is invalid");
}

/**
 * \brief Destructor.
 *
 * \pre   The remote access server is not running.
 *
 * \pre   There is no client registered.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
WorkQueueBasedRemoteAccessServer::~WorkQueueBasedRemoteAccessServer(void)
{
  // Note: The 2nd precondition ("There is no client registered.") is checked by the base class' destructor.

  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::Off)
    gpcc::osal::Panic("WorkQueueBasedRemoteAccessServer::~WorkQueueBasedRemoteAccessServer: Still running");
}

/**
 * \brief Starts the remote access server.
 *
 * \pre   The server is not running.
 *
 * \post  The server is running.
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
 */
void WorkQueueBasedRemoteAccessServer::Start(void)
{
  gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::Off)
    throw std::logic_error("WorkQueueBasedRemoteAccessServer::Start: Already running");

  dwq.Add(wp);
  state = States::Starting;
}

/**
 * \brief Stops the remote access server.
 *
 * The server can be stopped at any time. From the client's point of view stopping the server is treated like a broken
 * connection to the server:\n
 * The server will invoke @ref IRemoteObjectDictionaryAccessNotifiable::OnDisconnected() at the client's notifiable
 * interface to indicate that the offered @ref IRemoteObjectDictionaryAccess interface is no longer in ready-state. All
 * jobs still enqueued in the server will be dropped according to the rules of the
 * [RODA/RODAN interfaces](@ref GPCC_COOD_REMOTEACCESS_ITF).
 *
 * \pre   The server is running.
 *
 * \post  The server is not running.
 *
 * \post  The provided RODA/RODAN interface is in "not-ready" state. Pending requests and responses will be treated
 *        according to the rules of the [RODA/RODAN interfaces](@ref GPCC_COOD_REMOTEACCESS_ITF).
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
void WorkQueueBasedRemoteAccessServer::Stop(void) noexcept
{
  try
  {
    gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

    {
      gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

      switch (state)
      {
        case States::Off:
          throw std::logic_error("Not running");

        case States::Starting:
          state = States::Off;
          break;

        case States::On:
          state = States::Stopping;
          dwq.Add(wp);
          break;

        case States::InvocationRequested:
          state = States::Stopping;
          break;

        case States::RetryInvocation:
          state = States::Stopping;
          // WQEntry() may be executed twice, but it will ignore the second call when state is "Off".
          dwq.Remove(dwp);
          dwq.Add(wp);
          break;

        case States::Stopping:
          throw std::logic_error("'state' is invalid");
      }
    }

    dwq.FlushNonDeferredWorkPackages();

    // sanity check
    {
      gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

      if (state != States::Off)
        throw std::logic_error("Failed to stop!");
    }
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Entry function for the server's work packages.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by @ref wp and @ref dwp in context of @ref dwq.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void WorkQueueBasedRemoteAccessServer::WQentry(void) noexcept
{
  try
  {
    // check current state, safe it in "prevState" and switch to new state
    States prevState;
    {
      gpcc::osal::MutexLocker internalMutexLocker(internalMutex);
      prevState = state;

      switch (state)
      {
        case States::Off:
          // spurious call, will be ignored
          break;

        case States::Starting:
          state = States::On;
          break;

        case States::On:
          throw std::logic_error("Unexpected call");

        case States::InvocationRequested:
          state = States::On;
          break;

        case States::RetryInvocation:
          state = States::On;
          break;

        case States::Stopping:
          state = States::Off;
          break;
      }
    }

    // depending on previous state, take action with "internalMutex" being unlocked
    switch (prevState)
    {
      case States::Starting:
        OnStart();
        break;

      case States::InvocationRequested:
      case States::RetryInvocation:
        ServeRequest();
        break;

      case States::Stopping:
        OnStop();
        break;

      default:
        // don't care by intention
        break;
    }
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Invokes @ref RemoteAccessServer::Work() and handles a potential std::bad_alloc exception.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked in context of @ref dwq. \n
 * @ref internalMutex shall not be locked when calling this.
 *
 * __Exception safety:__\n
 * Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void WorkQueueBasedRemoteAccessServer::ServeRequest(void)
{
  try
  {
    Work();
  }
  catch (std::bad_alloc const &)
  {
    gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

    // Stop requested in the mean-time?
    if (state == States::Stopping)
      return;

    // New request for invocation of Work() in the mean-time?
    if (state == States::InvocationRequested)
      return;

    if (state != States::On)
      throw std::logic_error("WorkQueueBasedRemoteAccessServer::ServeRequest: 'state' invalid");

    if (pLogger != nullptr)
      pLogger->Log(gpcc::log::LogType::Warning, "Out of memory during processing request(s). Retry...");

    dwp.SetTimeSpan(gpcc::time::TimeSpan::ms(oomRetryDelay_ms));
    dwq.Add(dwp);

    state = States::RetryInvocation;
  }
}

/// \copydoc gpcc::cood::RemoteAccessServer::RequestWorkInvocationHook
void WorkQueueBasedRemoteAccessServer::RequestWorkInvocationHook(void)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (state != States::On)
    return;

  dwq.Add(wp);
  state = States::InvocationRequested;
}

} // namespace cood
} // namespace gpcc
