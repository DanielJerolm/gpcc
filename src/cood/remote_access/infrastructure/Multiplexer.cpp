/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/infrastructure/Multiplexer.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/PingResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/RequestBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t constexpr Multiplexer::maxNbOfPorts;

/**
 * \brief Constructor.
 *
 * The created @ref Multiplexer instance has no exposed ports (RODA interfaces) yet. Use @ref CreatePort() to
 * create ports providing a RODA interfaces.
 *
 * The created @ref Multiplexer instance is not connected to a RODA interface yet. Use @ref Connect() to connect
 * the multiplexer to an existing RODA interface.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Multiplexer::Multiplexer(void)
: IRemoteObjectDictionaryAccessNotifiable()
, ownerID(0U)
, connectMutex()
, muxMutex()
, portMutex()
, state(States::notConnected)
, pRODA(nullptr)
, maxRequestSize(0U)
, maxResponseSize(0U)
, ports()
{
  auto const uipThis = reinterpret_cast<uintptr_t>(this);
  #if UINTPTR_WIDTH == UINT32_WIDTH
    ownerID = static_cast<uint32_t>(uipThis);
  #else
    ownerID = (static_cast<uint32_t>(uipThis)) ^ (static_cast<uint32_t>(uipThis >> 32U));
  #endif
}

/**
 * \brief Destructor.
 *
 * \pre   The @ref Multiplexer is not connected to any [RODA](@ref IRemoteObjectDictionaryAccess) interface any more.\n
 *        Use @ref Disconnect() to disconnect the multiplexer from a RODA interface if necessary.
 *
 * \pre   All shared_ptr instances acquired from @ref CreatePort() have been dropped.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Multiplexer::~Multiplexer(void)
{
  gpcc::osal::MutexLocker muxMutexLocker(muxMutex);

  if (state != States::notConnected)
    gpcc::osal::Panic("Multiplexer::~Multiplexer: Still connected to a RODA interface.");

  for (auto const & spPort : ports)
  {
    if (spPort.use_count() != 1U)
      gpcc::osal::Panic("Multiplexer::~Multiplexer: Port still referenced by someone.");
  }
}

/**
 * \brief Connects the @ref Multiplexer to a RODA interface.
 *
 * @ref Disconnect() is the counterpart of this method.
 *
 * \pre   The @ref Multiplexer is not connected to any RODA interface yet.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param roda
 * RODA interface the @ref Multiplexer shall be connected to.
 */
void Multiplexer::Connect(IRemoteObjectDictionaryAccess & roda)
{
  gpcc::osal::MutexLocker connectMutexLocker(connectMutex);

  {
    gpcc::osal::MutexLocker muxMutexLocker(muxMutex);

    if (state != States::notConnected)
      throw std::logic_error("Multiplexer::Connect: Already connected.");

    gpcc::osal::MutexLocker portMutexLocker(portMutex);
    state = States::notReady;
    pRODA = &roda;
  }

  ON_SCOPE_EXIT(undo)
  {
    gpcc::osal::MutexLocker muxMutexLocker(muxMutex);
    gpcc::osal::MutexLocker portMutexLocker(portMutex);
    pRODA = nullptr;
    state = States::notConnected;
  };

  pRODA->Register(this);

  ON_SCOPE_EXIT_DISMISS(undo);
}

/**
 * \brief Disconnects the @ref Multiplexer from a RODA interface.
 *
 * This method is the counterpart of @ref Connect().
 *
 * If the multiplexer is not connected to any RODA interface, then this method will have no effect.
 *
 * \post   The @ref Multiplexer is not connected to any RODA interface.
 *
 * \post   The RODA interfaces provided by the multiplexer's ports have been switched to "not ready" and all clients
 *         connected to the provided RODA interfaces have received the
 *         [OnDisconnected()](@ref IRemoteObjectDictionaryAccessNotifiable::OnDisconnected)-notification.
 *
 * \post   There is no client registered at the RODA interface this multiplexer was formerly connected to.
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
void Multiplexer::Disconnect(void) noexcept
{
  gpcc::osal::MutexLocker connectMutexLocker(connectMutex);

  gpcc::osal::AdvancedMutexLocker muxMutexLocker(muxMutex);

  // not connected -> no effect
  if (state == States::notConnected)
    return;

  // sanity check
  if (state == States::disconnecting)
    PANIC();

  // Start disconnection process.
  // From now on all notifications received via the multiplexer's RODAN interface will be ignored.
  // In particular response messages will not be delivered any more.
  {
    gpcc::osal::MutexLocker portMutexLocker(portMutex);
    state = States::disconnecting;
  }

  // switch ports to "not ready" if required and forget about old session IDs
  for (auto const & spPort : ports)
  {
    if (spPort->state == MultiplexerPort::States::ready)
    {
      {
        gpcc::osal::MutexLocker portMutexLocker(portMutex);
        spPort->state = MultiplexerPort::States::notReady;
        spPort->execContextRequested = false;
        spPort->oldestUsedSessionID = spPort->sessionID;
      }
      spPort->pRODAN->OnDisconnected();
    }
    else
    {
      gpcc::osal::MutexLocker portMutexLocker(portMutex);
      spPort->oldestUsedSessionID = spPort->sessionID;
    }
  }

  // muxMutex must be unlocked, because Unregister() blocks until potential ongoing calls to the RODAN interface
  // exposed by the multiplexer have completed.
  muxMutexLocker.Unlock();
  pRODA->Unregister();
  muxMutexLocker.Relock();

  // finish disconnection
  gpcc::osal::MutexLocker portMutexLocker(portMutex);
  pRODA = nullptr;
  state = States::notConnected;
}

/**
 * \brief Creates a new port providing a RODA interface.
 *
 * Under the hood, unused ports whose shared_ptr has been dropped by the user will be recycled before new ones are
 * created.
 *
 * \pre   There are less than @ref maxNbOfPorts in use.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc       Out of memory.
 *
 * \throws std::runtime_error   @ref maxNbOfPorts ports are already in use.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * @ref MultiplexerPort instance providing a RODA interface.\n
 * The client shall drop the shared_ptr when it doesn't need the port any more.\n
 * All references to the @ref MultiplexerPort instance must be dropped before this @ref Multiplexer instance is
 * destroyed.
 */
std::shared_ptr<MultiplexerPort> Multiplexer::CreatePort(void)
{
  gpcc::osal::MutexLocker muxMutexLocker(muxMutex);
  gpcc::osal::MutexLocker portMutexLocker(portMutex);

  // look for an unused port
  for (auto const & spPort : ports)
  {
    if (spPort.use_count() == 1U)
    {
      if (spPort->state != MultiplexerPort::States::noClientRegistered)
        gpcc::osal::Panic("Multiplexer::CreatePort: Dropped port has still a RODAN interface registered.");

      return spPort;
    }
  }

  if (ports.size() == maxNbOfPorts)
    throw std::runtime_error("Multiplexer::CreatePort: Maximum number of ports reached.");

  ports.emplace_back(std::make_shared<MultiplexerPort>(*this, ports.size()));
  return ports.back();
}

// <-- IRemoteObjectDictionaryAccessNotifiable

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady
void Multiplexer::OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept
{
  gpcc::osal::MutexLocker muxMutexLocker(muxMutex);

  switch (state)
  {
    case States::notConnected:
      gpcc::osal::Panic("Multiplexer::OnReady: Not connected to any RODA interface.");

    case States::notReady:
      break;

    case States::ready:
      gpcc::osal::Panic("Multiplexer::OnReady: Already ready.");

    case States::disconnecting:
      // ignore call, disconnection from RODA interface is in process
      return;
  }

  // switch mutiplexer to "ready"
  {
    gpcc::osal::MutexLocker portMutexLocker(portMutex);

    if (maxRequestSize < (RequestBase::minimumUsefulRequestSize + ReturnStackItem::binarySize))
      this->maxRequestSize = 0U;
    else
      this->maxRequestSize = maxRequestSize - ReturnStackItem::binarySize;

    if (maxResponseSize < (ResponseBase::minimumUsefulResponseSize + ReturnStackItem::binarySize))
      this->maxResponseSize = 0U;
    else
      this->maxResponseSize = maxResponseSize - ReturnStackItem::binarySize;

    state = States::ready;
  }

  // switch ports to "ready"
  for (auto const & spPort : ports)
  {
    if (spPort->state == MultiplexerPort::States::notReady)
    {
      {
        gpcc::osal::MutexLocker portMutexLocker(portMutex);
        spPort->state = MultiplexerPort::States::ready;
      }

      spPort->pRODAN->OnReady(this->maxRequestSize, this->maxResponseSize);
    }
  }
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnDisconnected
void Multiplexer::OnDisconnected(void) noexcept
{
  gpcc::osal::MutexLocker muxMutexLocker(muxMutex);

  switch (state)
  {
    case States::notConnected:
      gpcc::osal::Panic("Multiplexer::OnDisconnected: Not connected to any RODA interface.");

    case States::notReady:
      gpcc::osal::Panic("Multiplexer::OnDisconnected: Already disconnected / not ready.");

    case States::ready:
      break;

    case States::disconnecting:
      // ignore call, disconnection from RODA interface is in process
      return;
  }

  // switch multiplexer to "not ready"
  {
    gpcc::osal::MutexLocker portMutexLocker(portMutex);
    state = States::notReady;
  }

  // switch ports to "not ready" and reset sessionID
  for (auto const & spPort : ports)
  {
    if (spPort->state == MultiplexerPort::States::ready)
    {
      {
        gpcc::osal::MutexLocker portMutexLocker(portMutex);
        spPort->state = MultiplexerPort::States::notReady;
        spPort->execContextRequested = false;
        spPort->oldestUsedSessionID = spPort->sessionID;
      }

      spPort->pRODAN->OnDisconnected();
    }
    else
    {
      gpcc::osal::MutexLocker portMutexLocker(portMutex);
      spPort->oldestUsedSessionID = spPort->sessionID;
    }
  }
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed
void Multiplexer::OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept
{
  gpcc::osal::MutexLocker muxMutexLocker(muxMutex);

  switch (state)
  {
    case States::notConnected:
      gpcc::osal::Panic("Multiplexer::OnRequestProcessed: Not connected to any RODA interface.");

    case States::notReady:
      gpcc::osal::Panic("Multiplexer::OnRequestProcessed: Unexpected call, RODA interface is 'not ready'.");

    case States::ready:
      break;

    case States::disconnecting:
      // ignore call, disconnection from RODA interface is in process
      return;
  }

  // Extract the top return stack item from the response message and check:
  // - Are we the originator of the request?
  // - Which port is the originator of the request?
  // - Does the response belong to the current session of the port?
  // In case of any mismatch we just discard the response.
  //
  // If the message is a ping response for a ping request sent by the port, then we will reset the port's sessionID.
  // Otherwise the message is forwarded to the client connected to the port.

  if (spResponse->IsReturnStackEmpty())
    return;

  auto const rsi = spResponse->PopReturnStack();

  if (rsi.GetID() != ownerID)
    return;

  // extract our information from the return stack item
  auto const info = rsi.GetInfo();
  uint_fast8_t  const index     = (info >> 24U) & 0xFFU;
  bool          const myPing    = ((info & 0x00800000UL) != 0U);
  uint_fast16_t const gap       = (info >> 8U) & 0x7FFFU;
  uint_fast8_t  const sessionID = info & 0xFFU;

  if (gap != 0U)
    return;

  if (index >= ports.size())
    return;

  auto & spPort = ports[index];

  if (!myPing)
  {
    if ((spPort->state == MultiplexerPort::States::ready) && (spPort->sessionID == sessionID))
      spPort->pRODAN->OnRequestProcessed(std::move(spResponse));
  }
  else
  {
    if (typeid(*spResponse) != typeid(PingResponse))
      return;

    // the ping was sent by us, so its return stack should be empty now
    if (!spResponse->IsReturnStackEmpty())
      return;

    if (spPort->sessionID != sessionID)
      return;

    gpcc::osal::MutexLocker portMutexLocker(portMutex);
    spPort->oldestUsedSessionID = spPort->sessionID;
  }
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext
void Multiplexer::LoanExecutionContext(void) noexcept
{
  gpcc::osal::MutexLocker muxMutexLocker(muxMutex);

  switch (state)
  {
    case States::notConnected:
      gpcc::osal::Panic("Multiplexer::LoanExecutionContext: Not connected to any RODA interface.");

    case States::notReady:
      gpcc::osal::Panic("Multiplexer::LoanExecutionContext: Unexpected call, RODA interface is 'not ready'.");

    case States::ready:
      break;

    case States::disconnecting:
      // ignore call, disconnection from RODA interface is in process
      return;
  }

  // The multiplexer is in state "ready". We will check the state of all ports:
  // - If any port is in state "not ready", then we will switch it to "ready".
  // - If a port is "ready", then we check if it has a pending request for a call to client's LoanExecutionContext()
  //   method, and -if so- we will serve the request.
  for (auto const & spPort : ports)
  {
    if (spPort->state == MultiplexerPort::States::notReady)
    {
      {
        gpcc::osal::MutexLocker portMutexLocker(portMutex);
        spPort->state = MultiplexerPort::States::ready;
      }

      spPort->pRODAN->OnReady(this->maxRequestSize, this->maxResponseSize);
    }
    else if (spPort->state == MultiplexerPort::States::ready)
    {
      {
        gpcc::osal::MutexLocker portMutexLocker(portMutex);
        if (!spPort->execContextRequested)
          continue;

        spPort->execContextRequested = false;
      }

      spPort->pRODAN->LoanExecutionContext();
    }
    else
    {
      // no client at port registered
    }
  }
}

// --> IRemoteObjectDictionaryAccessNotifiable

} // namespace cood
} // namespace gpcc
