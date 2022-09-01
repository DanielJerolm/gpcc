/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/infrastructure/RemoteAccessServer.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/PingRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/PingResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequestResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/RequestBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp>
#include <gpcc/cood/remote_access/roda_itf/exceptions.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
#include <gpcc/cood/IObjectAccess.hpp>
#include <gpcc/cood/Object.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/Stream/MemStreamReader.hpp"
#include "gpcc/src/Stream/MemStreamWriter.hpp"
#include <exception>
#include <stdexcept>

using gpcc::log::LogType;

namespace gpcc {
namespace cood {

// <-- IRemoteObjectDictionaryAccess

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Register
void RemoteAccessServer::Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable)
{
  if (pNotifiable == nullptr)
    throw std::invalid_argument("RemoteAccessServer::Register: !pNotifiable.");

  gpcc::osal::MutexLocker clientMutexLocker(clientMutex);

  if (pClient != nullptr)
    throw std::logic_error("RemoteAccessServer::Register: Already registered.");

  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  switch (state)
  {
    case States::UnregisteredAndOff:
      pClient = pNotifiable;
      state = States::off;
      break;

    case States::UnregisteredAndIdle:
      RequestWorkInvocationHook();
      pClient = pNotifiable;
      state = States::justRegistered;
      break;

    default:
      // In all other states a client is registered. This should never be executed because pClient has been
      // checked for nullptr before. Otherwise we have a broken invariant.
      PANIC();
  }

  if (pLogger != nullptr)
    pLogger->Log(LogType::Info, "Client registered.");
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Unregister
void RemoteAccessServer::Unregister(void) noexcept
{
  // set unregisterPending-flag
  {
    bool expected = false;
    if (!unregisterPending.compare_exchange_strong(expected, true))
      gpcc::osal::Panic("RemoteAccessServer::Unregister: More than one thread.");

    // note: No roll-back due to noexcept
  }

  gpcc::osal::MutexLocker clientMutexLocker(clientMutex);
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  // no client registered? -> no effect
  if (pClient == nullptr)
  {
    unregisterPending = false;
    return;
  }

  switch (state)
  {
    case States::off:
      state = States::UnregisteredAndOff;
      break;
    case States::justRegistered:
      state = States::UnregisteredAndIdle;
      break;
    case States::idle:
      state = States::UnregisteredAndIdle;
      break;
    case States::processing:
      state = States::UnregisteredAndIdle;
      break;
    default:
      // In all other states no client is registered. This should never be executed because pClient has been
      // checked for nullptr before. Otherwise we have a broken invariant.
      PANIC();
  }

  // drop any pending requests
  bool const droppedRequests = !queue.empty();
  queue.ClearAndDestroyItems();

  loanExecContextRequested = false;

  // unregister
  pClient = nullptr;

  if (pLogger != nullptr)
  {
    if (droppedRequests)
      pLogger->Log(LogType::Info, "Client unregistered. Dropped at least one request from queue.");
    else
      pLogger->Log(LogType::Info, "Client unregistered.");
  }

  unregisterPending = false;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Send
void RemoteAccessServer::Send(std::unique_ptr<RequestBase> & spReq)
{
  if (!spReq)
    throw std::invalid_argument("RemoteAccessServer::Send: !spReq.");

  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  switch (state)
  {
    case States::UnregisteredAndOff:
    case States::UnregisteredAndIdle:
      throw std::logic_error("RemoteAccessServer::Send: No client registered.");

    case States::off:
    case States::justRegistered:
      throw RemoteAccessServerNotReadyError();

    case States::idle:
      SanityCheck(*spReq);

      RequestWorkInvocationHook();

      queue.push_back(spReq.get());
      spReq.release();

      state = States::processing;
      break;

    case States::processing:
      SanityCheck(*spReq);

      queue.push_back(spReq.get());
      spReq.release();
      break;
  }

  if (pLogger != nullptr)
    pLogger->Log(LogType::Debug, "Send() invoked.");
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext
void RemoteAccessServer::RequestExecutionContext(void)
{
  gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

  switch (state)
  {
    case States::UnregisteredAndOff:
    case States::UnregisteredAndIdle:
      throw std::logic_error("RemoteAccessServer::RequestExecutionContext: No client registered.");

    case States::off:
    case States::justRegistered:
      throw RemoteAccessServerNotReadyError();

    case States::idle:
      RequestWorkInvocationHook();
      break;

    case States::processing:
      break;
  }

  loanExecContextRequested = true;

  if (pLogger != nullptr)
    pLogger->Log(LogType::Debug, "RequestExecutionContext() invoked.");
}

// --> IRemoteObjectDictionaryAccess

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
 * \param _od
 * Interface that shall be used to access the object dictionary.
 *
 * \param _pLogger
 * Pointer to a logger that shall be used by the @ref RemoteAccessServer to log messages.\n
 * nullptr, if logging is not required.\n
 * If a logger is provided, then it shall have a meaningful name and it should be assigned exclusively to the
 * remote access server to avoid confusing log messages.
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
RemoteAccessServer::RemoteAccessServer(IObjectAccess & _od,
                                       gpcc::log::Logger * const _pLogger,
                                       size_t const _maxRequestSize,
                                       size_t const _maxResponseSize)
: IRemoteObjectDictionaryAccess()
, pLogger(_pLogger)
, maxRequestSize(_maxRequestSize)
, maxResponseSize(_maxResponseSize)
, od(_od)
, clientMutex()
, apiMutex()
, unregisterPending(false)
, state(States::UnregisteredAndOff)
, pClient(nullptr)
, queue()
, loanExecContextRequested(false)
, temporaryReturnStack()
{
  if (   (maxRequestSize < RequestBase::minimumUsefulRequestSize)
      || (maxRequestSize > RequestBase::maxRequestSize))
  {
    throw std::invalid_argument("RemoteAccessServer::RemoteAccessServer: _maxRequestSize invalid");
  }

  if (   (maxResponseSize < ResponseBase::minimumUsefulResponseSize)
      || (maxResponseSize > ResponseBase::maxResponseSize))
  {
    throw std::invalid_argument("RemoteAccessServer::RemoteAccessServer: _maxResponseSize invalid");
  }

  if (pLogger != nullptr)
  {
    if (pLogger->IsAboveLevel(LogType::Info))
    {
      try
      {
        pLogger->Log(LogType::Info, "Instantiated.\n" \
                                    "Max. request size : " + std::to_string(maxRequestSize) + "\n" \
                                    "Max. response size: " + std::to_string(maxResponseSize));
      }
      catch (std::exception const &)
      {
        pLogger->LogFailed();
      }
    }
  }
}

/**
 * \brief Destructor.
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
RemoteAccessServer::~RemoteAccessServer(void)
{
  gpcc::osal::MutexLocker clientMutexLocker(clientMutex);

  if (pClient != nullptr)
    gpcc::osal::Panic("RemoteAccessServer::~RemoteAccessServer: Client still registered.");
}

/**
 * \brief OnStart()-method invoked by derived classes when the server is started.
 *
 * \pre   The server is not running.
 *
 * \post  The server is running and the provided @ref IRemoteObjectDictionaryAccess interface is in ready-state.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a derived class.
 *
 * __Exception safety:__\n
 * No guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void RemoteAccessServer::OnStart(void)
{
  gpcc::osal::MutexLocker clientMutexLocker(clientMutex);

  bool invokeOnReady = false;
  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    switch (state)
    {
      case States::UnregisteredAndOff:
        if (pLogger != nullptr)
          pLogger->Log(LogType::Info, "Started.");

        state = States::UnregisteredAndIdle;
        break;

      case States::UnregisteredAndIdle:
        throw std::logic_error("RemoteAccessServer::OnStart: Already running");

      case States::off:
        if (pLogger != nullptr)
        {
          // 2 log messages by intention
          pLogger->Log(LogType::Info, "Started.");
          pLogger->Log(LogType::Info, "RODA/RODAN: ready");
        }

        state = States::idle;
        invokeOnReady = true;
        break;

      case States::justRegistered:
      case States::idle:
      case States::processing:
        throw std::logic_error("RemoteAccessServer::OnStart: Already running");
    }
  }

  if (invokeOnReady)
    pClient->OnReady(maxRequestSize, maxResponseSize);
}

/**
 * \brief Work-method used to process one request.
 *
 * This shall be invoked by a derived class after invocation has been requested via
 * [RequestWorkInvocationHook()](@ref gpcc::cood::RemoteAccessServer::RequestWorkInvocationHook).
 *
 * This is well aware of spurious invocations.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a derived class.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The derived class shall not drop the request issued via
 *                          [RequestWorkInvocationHook()](@ref gpcc::cood::RemoteAccessServer::RequestWorkInvocationHook)
 *                          and invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void RemoteAccessServer::Work(void)
{
  gpcc::osal::MutexLocker clientMutexLocker(clientMutex);

  if (unregisterPending)
  {
    if (pLogger != nullptr)
      pLogger->Log(LogType::Debug, "Work() aborted due to pending unregister request.");

    return;
  }

  bool callOnReady = false;
  bool callLoanExecutionContext = false;
  RequestBase* pRequestToBeProcessed = nullptr;

  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    // no client registered? (-> spurious call)
    if (pClient == nullptr)
    {
      // check invariant
      if (   (state != States::UnregisteredAndOff)
          && (state != States::UnregisteredAndIdle))
      {
        PANIC();
      }

      if (pLogger != nullptr)
        pLogger->Log(LogType::Debug, "Spurious call to Work()");

      return;
    }

    // determine what to do and update state
    if (state == States::justRegistered)
    {
      // check invariant
      if ((!queue.empty()) || (loanExecContextRequested))
        PANIC();

      if (pLogger != nullptr)
        pLogger->Log(LogType::Info, "RODA/RODAN: ready");

      state = States::idle;
      callOnReady = true;
    }
    else if (loanExecContextRequested)
    {
      // hijacked the call to this? -> request another invocation
      if (state == States::processing)
        RequestWorkInvocationHook();

      if (pLogger != nullptr)
        pLogger->Log(LogType::Debug, "Loan execution context to client.");

      callLoanExecutionContext = true;
      loanExecContextRequested = false;
    }
    else if (state == States::processing)
    {
      // check invariant
      if (queue.empty())
        PANIC();

      if (pLogger != nullptr)
        pLogger->Log(LogType::Debug, "Processing request.");

      // need another call to this?
      if (queue.size() > 1U)
        RequestWorkInvocationHook();

      pRequestToBeProcessed = queue.front();
      queue.pop_front();

      if (queue.empty())
        state = States::idle;
    }
    else
    {
      if (pLogger != nullptr)
        pLogger->Log(LogType::Debug, "Spurious call to Work()");

      return;
    }
  }

  // do what has to be done
  if (callOnReady)
    pClient->OnReady(maxRequestSize, maxResponseSize);

  if (callLoanExecutionContext)
    pClient->LoanExecutionContext();

  if (pRequestToBeProcessed != nullptr)
  {
    ON_SCOPE_EXIT(undoQueuePop)
    {
      gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

      if (queue.empty())
      {
        state = States::processing;
        RequestWorkInvocationHook();
      };

      queue.push_front(pRequestToBeProcessed);
    };

    ServeRequest(*pRequestToBeProcessed);

    // success
    delete pRequestToBeProcessed;
    ON_SCOPE_EXIT_DISMISS(undoQueuePop);
  }
}

/**
 * \brief OnStop()-method invoked by derived classes when the server is about to stop.
 *
 * \pre   The server is running.
 *
 * \post  The server has switched the provided @ref IRemoteObjectDictionaryAccess interface into "not-ready" state.
 *        Any enqueued requests that have not yet been served are dropped.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a derived class.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void RemoteAccessServer::OnStop(void)
{
  gpcc::osal::MutexLocker clientMutexLocker(clientMutex);

  bool invokeOnDisconnected = false;
  {
    gpcc::osal::MutexLocker apiMutexLocker(apiMutex);

    switch (state)
    {
      case States::UnregisteredAndOff:
        throw std::logic_error("RemoteAccessServer::OnStop: Already stopped");

      case States::UnregisteredAndIdle:
        state = States::UnregisteredAndOff;
        break;

      case States::off:
        throw std::logic_error("RemoteAccessServer::OnStop: Already stopped");

      case States::justRegistered:
        state = States::off;
        break;

      case States::idle:
        // check invariant
        if (!queue.empty())
          PANIC();

        loanExecContextRequested = false;
        state = States::off;

        invokeOnDisconnected = true;
        break;

      case States::processing:
        queue.ClearAndDestroyItems();
        loanExecContextRequested = false;
        state = States::off;

        invokeOnDisconnected = true;
    }
  }

  if (invokeOnDisconnected)
  {
    pClient->OnDisconnected();

    if (pLogger != nullptr)
      pLogger->Log(LogType::Info, "RODA/RODAN: not-ready");
  }

  if (pLogger != nullptr)
    pLogger->Log(LogType::Info, "Stopped");
}

/**
 * \brief Runs essential tests on incoming requests.
 *
 * The following checks are done:
 * - Size of request does not exceed @ref maxRequestSize
 * - Maximum response size does not exceed @ref maxResponseSize
 * - The maximum response size (without stack of return stack items) is equal to or larger than the minimum useful size.
 *
 * In case of any error this method will take the following actions:
 * 1. Log an error message
 * 2. Throw an appropriate exception
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param request
 * Unmodifiable reference to the request that shall be checked.
 */
void RemoteAccessServer::SanityCheck(RequestBase const & request) const
{
  if (request.GetBinarySize() > maxRequestSize)
  {
    if (pLogger != nullptr)
      pLogger->Log(LogType::Error, "Request rejected: Request too large");

    throw RequestTooLargeError();
  }

  auto const mrs = request.GetMaxResponseSize();
  if (mrs > maxResponseSize)
  {
    if (pLogger != nullptr)
      pLogger->Log(LogType::Error, "Request rejected: maxResponseSize too large");

    throw ResponseTooLargeError();
  }

  auto const stackSize = request.GetReturnStackSize();

  if (   (stackSize >= mrs)
      || ((mrs - stackSize) < ResponseBase::minimumUsefulResponseSize))
  {
    if (pLogger != nullptr)
      pLogger->Log(LogType::Error, "Request rejected: Minimum useful response size not met");

    throw MinimumResponseSizeNotMetError();
  }
}

/**
 * \brief Serves a request.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a subclass of this.\n
 * `clientMutex` shall be locked by caller.\n
 * `apiMutex` shall not be locked yet.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The caller shall invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param request
 * Request that shall be served.\n
 * In case of an exception, the referenced request is not modified (-> strong guarantee).\n
 * In case of success, the referenced request or parts of it may have been consumed or moved somewhere.
 */
void RemoteAccessServer::ServeRequest(RequestBase & request)
{
  switch (request.GetType())
  {
    case RequestBase::RequestTypes::objectEnumRequest:
      ServeObjectEnumRequest(dynamic_cast<ObjectEnumRequest&>(request));
      return;

    case RequestBase::RequestTypes::objectInfoRequest:
      ServeObjectInfoRequest(dynamic_cast<ObjectInfoRequest&>(request));
      return;

    case RequestBase::RequestTypes::pingRequest:
      ServePingRequest(dynamic_cast<PingRequest&>(request));
      return;

    case RequestBase::RequestTypes::readRequest:
      ServeReadRequest(dynamic_cast<ReadRequest&>(request));
      return;

    case RequestBase::RequestTypes::writeRequest:
      ServeWriteRequest(dynamic_cast<WriteRequest&>(request));
      return;
  }

  // This line should never be reached. If the request type is not supported, then RequestBase::FromBinary() should
  // have rejected to deserialize the request, so the request object should not exist.
  throw std::logic_error("RemoteAccessServer::ServeRequest: Request type not supported");
}

/**
 * \brief Serves an @ref ObjectEnumRequest.
 *
 * \pre   A client is registered.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a subclass of this.\n
 * `clientMutex` shall be locked by caller.\n
 * `apiMutex` shall not be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The caller shall invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param request
 * Request that shall be served.\n
 * In case of an exception, the referenced request is not modified (-> strong guarantee).\n
 * In case of success, the referenced request or parts of it may have been consumed or moved somewhere.
 */
void RemoteAccessServer::ServeObjectEnumRequest(ObjectEnumRequest & request)
{
  // allocate response and calculate maximum payload that can be attached to it
  auto spResponse = std::make_unique<ObjectEnumResponse>(SDOAbortCode::GeneralError);
  size_t maxPayload = spResponse->CalcMaxNbOfIndices(request.GetMaxResponseSize(), request.GetReturnStackSize());

  if (maxPayload != 0U)
  {
    // serve request
    try
    {
      std::vector<uint16_t> indices;

      auto const attributes = request.GetAttributeFilter();
      auto spObject = od.GetNextNearestObject(request.GetStartIndex());
      auto const lastIndex = request.GetLastIndex();

      while ((spObject) && (spObject->GetIndex() <= lastIndex) && (maxPayload != 0U))
      {
        bool enumerate = false;

        if (attributes == 0xFFFFU)
        {
          // Each object has at least one attribute bit set, we do not need to check subindices if attributes
          // is 0xFFFF.
          enumerate = true;
        }
        else
        {
          uint_fast16_t nbOfSIs = spObject->GetMaxNbOfSubindices();
          for (uint_fast16_t si = 0U; si < nbOfSIs; ++si)
          {
            if ((spObject->GetSubIdxAttributes(si) & attributes) != 0U)
            {
              enumerate = true;
              break;
            }
          }
        }

        if (enumerate)
        {
          indices.push_back(spObject->GetIndex());
          --maxPayload;
        }

        ++spObject;
      }

      spResponse->SetData(std::move(indices), (!spObject) || (spObject->GetIndex() > lastIndex));
    }
    catch (std::bad_alloc const &)
    {
      spResponse->SetError(SDOAbortCode::OutOfMemory);
    }
    catch (std::exception const & e)
    {
      LogErrorWhileServingRequest(request);
      spResponse->SetError(SDOAbortCode::GeneralError);
    }
  }
  else
  {
    spResponse->SetError(SDOAbortCode::ObjectLengthExceedsMbxSize);
  }

  MoveReturnStack(request, *spResponse);
  pClient->OnRequestProcessed(std::move(spResponse));
}

/**
 * \brief Serves an @ref ObjectInfoRequest.
 *
 * \pre   A client is registered.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a subclass of this.\n
 * `clientMutex` shall be locked by caller.\n
 * `apiMutex` shall not be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The caller shall invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param request
 * Request that shall be served.\n
 * In case of an exception, the referenced request is not modified (-> strong guarantee).\n
 * In case of success, the referenced request or parts of it may have been consumed or moved somewhere.
 */
void RemoteAccessServer::ServeObjectInfoRequest(ObjectInfoRequest & request)
{
  std::unique_ptr<ResponseBase> spResponse;

  try
  {
    auto spObject = od.GetObject(request.GetIndex());

    if (spObject)
    {
      spResponse = std::make_unique<ObjectInfoResponse>(*spObject,
                                                        request.GetFirstSubIndex(),
                                                        request.GetLastSubIndex(),
                                                        request.IsInclusiveNames(),
                                                        request.IsInclusiveAppSpecificMetaData(),
                                                        request.GetMaxResponseSize(),
                                                        request.GetReturnStackSize());
    }
    else
    {
      spResponse = std::make_unique<ObjectInfoResponse>(SDOAbortCode::ObjectDoesNotExist);
    }
  }
  catch (std::bad_alloc const &)
  {
    spResponse = std::make_unique<ObjectInfoResponse>(SDOAbortCode::OutOfMemory);
  }
  catch (std::exception const & e)
  {
    LogErrorWhileServingRequest(request);
    spResponse = std::make_unique<ObjectInfoResponse>(SDOAbortCode::GeneralError);
  }

  MoveReturnStack(request, *spResponse);
  pClient->OnRequestProcessed(std::move(spResponse));
}

/**
 * \brief Serves a @ref PingRequest.
 *
 * \pre   A client is registered.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a subclass of this.\n
 * `clientMutex` shall be locked by caller.\n
 * `apiMutex` shall not be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The caller shall invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param request
 * Request that shall be served.\n
 * In case of an exception, the referenced request is not modified (-> strong guarantee).\n
 * In case of success, the referenced request or parts of it may have been consumed or moved somewhere.
 */
void RemoteAccessServer::ServePingRequest(PingRequest & request)
{
  auto spResponse = std::make_unique<PingResponse>();
  MoveReturnStack(request, *spResponse);
  pClient->OnRequestProcessed(std::move(spResponse));
}

/**
 * \brief Serves a @ref ReadRequest.
 *
 * \pre   A client is registered.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a subclass of this.\n
 * `clientMutex` shall be locked by caller.\n
 * `apiMutex` shall not be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The caller shall invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param request
 * Request that shall be served.\n
 * In case of an exception, the referenced request is not modified (-> strong guarantee).\n
 * In case of success, the referenced request or parts of it may have been consumed or moved somewhere.
 */
void RemoteAccessServer::ServeReadRequest(ReadRequest & request)
{
  // allocate response and calculate maximum payload that can be attached to it
  auto spResponse = std::make_unique<ReadRequestResponse>(SDOAbortCode::GeneralError);
  auto const maxPayload = ReadRequestResponse::CalcMaxDataPayload(request.GetMaxResponseSize(), request.GetReturnStackSize());

  // serve request
  try
  {
    auto spObject = od.GetObject(request.GetIndex());

    // object existing?
    if (spObject)
    {
      uint8_t const subindex = request.GetSubIndex();

      // lock object's mutex
      auto objMutexLocker = spObject->LockData();

      // subindex existing?
      if (   (subindex < spObject->GetNbOfSubIndices())
          && (!spObject->IsSubIndexEmpty(subindex)))
      {
        bool const completeAccess = (request.GetAccessType() != ReadRequest::AccessType::singleSubindex);
        bool const si0_16bit = (request.GetAccessType() == ReadRequest::AccessType::completeAccess_SI0_16bit);

        // determine size of data that shall be read
        size_t sizeInBit;
        if (!completeAccess)
        {
          sizeInBit = spObject->GetSubIdxActualSize(subindex);
        }
        else
        {
          sizeInBit = spObject->GetObjectStreamSize(si0_16bit);
          if (subindex != 0U)
            sizeInBit -= si0_16bit ? 16U : 8U;
        }
        size_t sizeInByte = (sizeInBit + 7U) / 8U;

        // does the data fit into the response object?
        if (sizeInByte <= maxPayload)
        {
          // create a container for the data and a MemStreamWriter
          std::vector<uint8_t> data(sizeInByte);
          gpcc::Stream::MemStreamWriter msw(data.data(),
                                            data.size(),
                                            gpcc::Stream::MemStreamWriter::Endian::Little);

          // do the actual read
          SDOAbortCode result;
          if (!completeAccess)
            result = spObject->Read(subindex, request.GetPermissions(), msw);
          else
            result = spObject->CompleteRead((subindex == 0U), si0_16bit, request.GetPermissions(), msw);

          if (result == SDOAbortCode::OK)
            spResponse->SetData(std::move(data), sizeInBit);
          else
            spResponse->SetError(result);
        }
        else
        {
          // Size of data that shall be read exceeds maximum amounth of data that can be attached to the
          // remote access response.
          spResponse->SetError(SDOAbortCode::ObjectLengthExceedsMbxSize);
        }
      }
      else
      {
        // subindex is not existing
        spResponse->SetError(SDOAbortCode::SubindexDoesNotExist);
      }
    }
    else
    {
      // object is not existing
      spResponse->SetError(SDOAbortCode::ObjectDoesNotExist);
    }
  }
  catch (std::bad_alloc const &)
  {
    spResponse->SetError(SDOAbortCode::OutOfMemory);
  }
  catch (std::exception const & e)
  {
    LogErrorWhileServingRequest(request);
    // result is already SDOAbortCode::GeneralError
  }

  MoveReturnStack(request, *spResponse);
  pClient->OnRequestProcessed(std::move(spResponse));
}

/**
 * \brief Serves a @ref WriteRequest.
 *
 * \pre   A client is registered.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be invoked in the execution context of the server only, provided by a subclass of this.\n
 * `clientMutex` shall be locked by caller.\n
 * `apiMutex` shall not be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. The caller shall invoke this again after a small delay.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param request
 * Request that shall be served.\n
 * In case of an exception, the referenced request is not modified (-> strong guarantee).\n
 * In case of success, the referenced request or parts of it may have been consumed or moved somewhere.
 */
void RemoteAccessServer::ServeWriteRequest(WriteRequest & request)
{
  // Allocate response.
  auto spResponse = std::make_unique<WriteRequestResponse>(SDOAbortCode::GeneralError);

  // serve request
  try
  {
    auto spObject = od.GetObject(request.GetIndex());
    if (spObject)
    {
      gpcc::Stream::MemStreamReader msr(request.GetData().data(),
                                        request.GetData().size(),
                                        gpcc::Stream::MemStreamReader::Endian::Little);

      bool const completeAccess = (request.GetAccessType() != WriteRequest::AccessType::singleSubindex);

      // lock object's mutex
      auto objMutexLocker = spObject->LockData();

      SDOAbortCode result;
      if (!completeAccess)
      {
        result = spObject->Write(request.GetSubIndex(), request.GetPermissions(), msr);
      }
      else
      {
        bool const inclSI0   = (request.GetSubIndex() == 0U);
        bool const si0_16bit = (request.GetAccessType() == WriteRequest::AccessType::completeAccess_SI0_16bit);

        result = spObject->CompleteWrite(inclSI0, si0_16bit, request.GetPermissions(),
                                         msr, gpcc::Stream::IStreamReader::RemainingNbOfBits::sevenOrLess);
      }

      spResponse->SetResult(result);
    }
    else
    {
      spResponse->SetResult(SDOAbortCode::ObjectDoesNotExist);
    }
  }
  catch (std::bad_alloc const &)
  {
    spResponse->SetResult(SDOAbortCode::OutOfMemory);
  }
  catch (std::exception const & e)
  {
    LogErrorWhileServingRequest(request);
    // result is already SDOAbortCode::GeneralError
  }

  MoveReturnStack(request, *spResponse);
  pClient->OnRequestProcessed(std::move(spResponse));
}

/**
 * \brief Logs an error caught during serving a request.
 *
 * This is intended to be called from a catch-block. The current exception will be attached to the log message.
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
 * \param request
 * Unmodifiable reference to the request object.\n
 * The request's ToString()-method will be invoked to gather details about the failed request.
 */
void RemoteAccessServer::LogErrorWhileServingRequest(RequestBase const & request) noexcept
{
  if ((pLogger != nullptr) && (pLogger->IsAboveLevel(LogType::Error)))
  {
    try
    {
      pLogger->Log(LogType::Error, "Error while serving request:\n" + request.ToString(), std::current_exception());
    }
    catch (std::exception const &)
    {
      pLogger->LogFailed();
    }
  }
}

/**
 * \brief Moves the stack of @ref ReturnStackItem objects from a request object to a response object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param src
 * Reference to request object.
 *
 * \param dest
 * Reference to response object.
 */
void RemoteAccessServer::MoveReturnStack(RequestBase & src, ResponseBase & dest) noexcept
{
  try
  {
    src.ExtractReturnStack(temporaryReturnStack);
    dest.SetReturnStack(std::move(temporaryReturnStack));
  }
  catch (...)
  {
    PANIC();
  }
}

} // namespace cood
} // namespace gpcc
