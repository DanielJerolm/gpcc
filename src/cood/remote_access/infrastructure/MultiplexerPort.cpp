/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "MultiplexerPort.hpp"
#include "Multiplexer.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/PingRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/RequestBase.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/exceptions.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp"
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _owner
 * Reference to the @ref Multiplexer instance which is the owner of the new port instance.
 *
 * \param _index
 * Index of the new port instance in `_owner.ports`.
 */
MultiplexerPort::MultiplexerPort(Multiplexer & _owner, uint8_t const _index) noexcept
: IRemoteObjectDictionaryAccess()
, owner(_owner)
, index(_index)
, state(States::noClientRegistered)
, pRODAN(nullptr)
, sessionID(0U)
, oldestUsedSessionID(0U)
, sessionIDUsed(false)
, execContextRequested(false)
{
}

/**
 * \brief Destructor.
 *
 * \pre   There is not client registered at the provided RODA interface.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
MultiplexerPort::~MultiplexerPort(void)
{
  gpcc::osal::MutexLocker portMutexLocker(owner.portMutex);

  if (state != States::noClientRegistered)
    gpcc::osal::Panic("MultiplexerPort::~MultiplexerPort: Client still registered.");
}

/**
 * \brief Queries if a RODA-client is registered at the provided RODA interface.
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
 * \retval true   A RODA client is registered at the provided RODA inteface.
 * \retval false  No one if registered at the provided RODA interface.
 */
bool MultiplexerPort::IsClientRegistered(void) const noexcept
{
  gpcc::osal::MutexLocker portMutexLocker(owner.portMutex);
  return (state != States::noClientRegistered);
}

// <-- IRemoteObjectDictionaryAccess

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Register
void MultiplexerPort::Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable)
{
  if (pNotifiable == nullptr)
    throw std::invalid_argument("MultiplexerPort::Register: !pNotifiable");

  gpcc::osal::MutexLocker muxMutexLocker(owner.muxMutex);

  if (state != States::noClientRegistered)
    throw std::logic_error("MultiplexerPort::Register: A client is already registered.");

  // determine potential next session ID
  uint8_t const nextSessionID = sessionID + 1U;
  if (nextSessionID == oldestUsedSessionID)
    throw std::runtime_error("MultiplexerPort::Register: No unused session ID available.");

  gpcc::osal::MutexLocker portMutexLocker(owner.portMutex);

  if (owner.state == Multiplexer::States::ready)
  {
    if (owner.pRODA == nullptr)
      PANIC();

    try
    {
      // request execution context for invocation of client's OnReady()-method
      owner.pRODA->RequestExecutionContext();

      // send a ping if there are used session IDs
      if (sessionIDUsed)
      {
        std::unique_ptr<RequestBase> spPing = std::make_unique<PingRequest>(owner.maxResponseSize);
        spPing->Push(ReturnStackItem(owner.ownerID, (static_cast<uint32_t>(index) << 24U) |
                                                     0x00800000UL |
                                                     nextSessionID));
        owner.pRODA->Send(spPing);
      }
    }
    catch (RemoteAccessServerNotReadyError const &)
    {
      // Ignored by intention. The owner of this port will receive an OnDisconnected()-notification soon.
    }
  }

  // start using new session ID if necessary
  if (sessionIDUsed)
  {
    sessionID = nextSessionID;
    sessionIDUsed = false;
  }

  state = States::notReady;
  pRODAN = pNotifiable;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Unregister
void MultiplexerPort::Unregister(void) noexcept
{
  gpcc::osal::MutexLocker muxMutexLocker(owner.muxMutex);

  // no client registered -> no effect
  if (state == States::noClientRegistered)
    return;

  gpcc::osal::MutexLocker portMutexLocker(owner.portMutex);

  state = States::noClientRegistered;
  pRODAN = nullptr;
  execContextRequested = false;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Send
void MultiplexerPort::Send(std::unique_ptr<RequestBase> & spReq)
{
  if (!spReq)
    throw std::invalid_argument("MultiplexerPort::Send: !spReq");

  gpcc::osal::MutexLocker portMutexLocker(owner.portMutex);

  switch (state)
  {
    case States::noClientRegistered:
      throw std::logic_error("MultiplexerPort::Send: No client registered");

    case States::notReady:
      throw RemoteAccessServerNotReadyError();

    case States::ready:
      break;
  }

  if (owner.pRODA == nullptr)
    PANIC();

  spReq->Push(ReturnStackItem(owner.ownerID, (static_cast<uint32_t>(index) << 24U) | sessionID));
  ON_SCOPE_EXIT(recoverRSI)
  {
    // sanity check: Server shall not consume request in case of an error
    if (!spReq)
      PANIC();

    spReq->UndoPush();
  };

  owner.pRODA->Send(spReq);

  ON_SCOPE_EXIT_DISMISS(recoverRSI);
  sessionIDUsed = true;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext
void MultiplexerPort::RequestExecutionContext(void)
{
  gpcc::osal::MutexLocker portMutexLocker(owner.portMutex);

  switch (state)
  {
    case States::noClientRegistered:
      throw std::logic_error("MultiplexerPort::RequestExecutionContext: No client registered");

    case States::notReady:
      throw RemoteAccessServerNotReadyError();

    case States::ready:
      break;
  }

  if (owner.pRODA == nullptr)
    PANIC();

  owner.pRODA->RequestExecutionContext();
  execContextRequested = true;
}

// --> IRemoteObjectDictionaryAccess

} // namespace cood
} // namespace gpcc
