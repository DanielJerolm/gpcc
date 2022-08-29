/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "ThreadBasedRemoteAccessServer.hpp"
#include <gpcc/log/Logger.hpp>
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
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
 * \param threadName
 * Name that shall be assigned to the server's thread.
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
ThreadBasedRemoteAccessServer::ThreadBasedRemoteAccessServer(std::string const & threadName,
                                                             uint8_t const _oomRetryDelay_ms,
                                                             IObjectAccess & _od,
                                                             gpcc::log::Logger * const _pLogger,
                                                             size_t const _maxRequestSize,
                                                             size_t const _maxResponseSize)
: RemoteAccessServer(_od, _pLogger, _maxRequestSize, _maxResponseSize)
, thread(threadName)
, oomRetryDelay_ms(_oomRetryDelay_ms)
, startStopMutex()
, internalMutex()
, running(false)
, invokeWorkRequestPending(false)
, cvInvokeWorkRequestPending()
{
  if (oomRetryDelay_ms == 0U)
    throw std::invalid_argument("ThreadBasedRemoteAccessServer::ThreadBasedRemoteAccessServer: oomRetryDelay_ms is invalid");
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
ThreadBasedRemoteAccessServer::~ThreadBasedRemoteAccessServer(void)
{
  // Note: The 2nd precondition ("There is no client registered.") is checked by the base class' destructor.

  gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

  if (running)
    gpcc::osal::Panic("ThreadBasedRemoteAccessServer::~ThreadBasedRemoteAccessServer: Still running");
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param schedPolicy
 * Desired scheduling policy for the server's thread.
 *
 * \param priority
 * Desired priority for the server's thread.
 *
 * \param stackSize
 * Desired stack size for the server's thread.\n
 * On ChibiOS/RT, 6..8kB are a good starting point.
 */
void ThreadBasedRemoteAccessServer::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                                          gpcc::osal::Thread::priority_t  const priority,
                                          size_t const stackSize)
{
  gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

  if (running)
    throw std::logic_error("ThreadBasedRemoteAccessServer::Start: Already running");

  thread.Start(std::bind(&ThreadBasedRemoteAccessServer::ThreadEntry, this), schedPolicy, priority, stackSize);
  running = true;
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
void ThreadBasedRemoteAccessServer::Stop(void) noexcept
{
  gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

  if (!running)
    gpcc::osal::Panic("ThreadBasedRemoteAccessServer::Stop: Not running");

  {
    gpcc::osal::MutexLocker internalMutexLocker(internalMutex);
    thread.Cancel();
    cvInvokeWorkRequestPending.Signal();
  }

  thread.Join();

  running = false;
}

/**
 * \brief Entry function for the server's thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures, that no more than one thread can execute this at any time.
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
 * Value that can be retrieved via Thread::Join(). Here: Always nullptr.
 */
void* ThreadBasedRemoteAccessServer::ThreadEntry(void) noexcept
{
  try
  {
    // Disable deferred thread cancellation. We cancel gracefully using our own logic.
    thread.SetCancelabilityEnabled(false);

    OnStart();
    ServeRequests();
    OnStop();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }

  return nullptr;
}

/**
 * \brief Waits for requests to invoke @ref RemoteAccessServer::Work() and serves the requests.
 *
 * This returns if deferred thread cancellation is requested. But instead of using thread cancellation, this method
 * will finish gracefully using our own logic.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures, that no more than one thread can execute this at any time.
 *
 * __Exception safety:__\n
 * Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void ThreadBasedRemoteAccessServer::ServeRequests(void)
{
  while (true)
  {
    // wait for request for work or thread cancellation
    {
      gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

      while ((!invokeWorkRequestPending) && (!thread.IsCancellationPending()))
        cvInvokeWorkRequestPending.Wait(internalMutex);

      if (thread.IsCancellationPending())
        return;

      invokeWorkRequestPending = false;
    }

    // loop used to implement a retry-mechanism in case of std::bad_alloc
    while (true)
    {
      try
      {
        Work();
      }
      catch (std::bad_alloc const &)
      {
        if (pLogger != nullptr)
          pLogger->Log(gpcc::log::LogType::Warning, "Out of memory during processing request(s). Retry...");

        gpcc::osal::Thread::Sleep_ms(oomRetryDelay_ms);

        if (thread.IsCancellationPending())
        {
          invokeWorkRequestPending = true;
          return;
        }

        continue;
      }

      break;
    } // retry-loop
  } // outer work-loop
}

/// \copydoc gpcc::cood::RemoteAccessServer::RequestWorkInvocationHook
void ThreadBasedRemoteAccessServer::RequestWorkInvocationHook(void)
{
  gpcc::osal::MutexLocker internalMutexLocker(internalMutex);

  if (!invokeWorkRequestPending)
  {
    cvInvokeWorkRequestPending.Signal();
    invokeWorkRequestPending = true;
  }
}

} // namespace cood
} // namespace gpcc
