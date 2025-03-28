/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021, 2025 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/infrastructure/MultiplexerPort.hpp>
#include <gpcc/cood/remote_access/infrastructure/Multiplexer.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/PingRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/RequestBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/cood/remote_access/roda_itf/exceptions.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
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
 * \param owner
 * Reference to the @ref Multiplexer instance which is the owner of the new port instance.
 *
 * \param index
 * Index of the new port instance in `owner.ports`.
 */
MultiplexerPort::MultiplexerPort(Multiplexer & owner, uint8_t const index) noexcept
: IRemoteObjectDictionaryAccess()
, owner_(owner)
, index_(index)
, state_(States::noClientRegistered)
, pRODAN_(nullptr)
, sessionID_(0U)
, oldestUsedSessionID_(0U)
, sessionIDUsed_(false)
, execContextRequested_(false)
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
  gpcc::osal::MutexLocker portMutexLocker(owner_.portMutex_);

  if (state_ != States::noClientRegistered)
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
  gpcc::osal::MutexLocker portMutexLocker(owner_.portMutex_);
  return (state_ != States::noClientRegistered);
}

// <-- IRemoteObjectDictionaryAccess

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Register
void MultiplexerPort::Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable)
{
  if (pNotifiable == nullptr)
    throw std::invalid_argument("MultiplexerPort::Register: !pNotifiable");

  gpcc::osal::MutexLocker muxMutexLocker(owner_.muxMutex_);

  if (state_ != States::noClientRegistered)
    throw std::logic_error("MultiplexerPort::Register: A client is already registered.");

  // determine potential next session ID
  uint8_t const nextSessionID = sessionID_ + 1U;
  if (nextSessionID == oldestUsedSessionID_)
    throw std::runtime_error("MultiplexerPort::Register: No unused session ID available.");

  gpcc::osal::MutexLocker portMutexLocker(owner_.portMutex_);

  if (owner_.state_ == Multiplexer::States::ready)
  {
    if (owner_.pRODA_ == nullptr)
      PANIC();

    try
    {
      // request execution context for invocation of client's OnReady()-method
      owner_.pRODA_->RequestExecutionContext();

      // send a ping if there are used session IDs
      if (sessionIDUsed_)
      {
        std::unique_ptr<RequestBase> spPing = std::make_unique<PingRequest>(owner_.maxResponseSize_);
        spPing->Push(ReturnStackItem(owner_.ownerID_, (static_cast<uint32_t>(index_) << 24U) |
                                                     0x00800000UL |
                                                     nextSessionID));
        owner_.pRODA_->Send(spPing);
      }
    }
    catch (RemoteAccessServerNotReadyError const &)
    {
      // Ignored by intention. The owner_ of this port will receive an OnDisconnected()-notification soon.
    }
  }

  // start using new session ID if necessary
  if (sessionIDUsed_)
  {
    sessionID_ = nextSessionID;
    sessionIDUsed_ = false;
  }

  state_ = States::notReady;
  pRODAN_ = pNotifiable;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Unregister
void MultiplexerPort::Unregister(void) noexcept
{
  gpcc::osal::MutexLocker muxMutexLocker(owner_.muxMutex_);

  // no client registered -> no effect
  if (state_ == States::noClientRegistered)
    return;

  gpcc::osal::MutexLocker portMutexLocker(owner_.portMutex_);

  state_ = States::noClientRegistered;
  pRODAN_ = nullptr;
  execContextRequested_ = false;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::Send
void MultiplexerPort::Send(std::unique_ptr<RequestBase> & spReq)
{
  if (!spReq)
    throw std::invalid_argument("MultiplexerPort::Send: !spReq");

  gpcc::osal::MutexLocker portMutexLocker(owner_.portMutex_);

  switch (state_)
  {
    case States::noClientRegistered:
      throw std::logic_error("MultiplexerPort::Send: No client registered");

    case States::notReady:
      throw RemoteAccessServerNotReadyError();

    case States::ready:
      break;
  }

  if (owner_.pRODA_ == nullptr)
    PANIC();

  spReq->Push(ReturnStackItem(owner_.ownerID_, (static_cast<uint32_t>(index_) << 24U) | sessionID_));
  ON_SCOPE_EXIT(recoverRSI)
  {
    // sanity check: Server shall not consume request in case of an error
    if (!spReq)
      PANIC();

    spReq->UndoPush();
  };

  owner_.pRODA_->Send(spReq);

  ON_SCOPE_EXIT_DISMISS(recoverRSI);
  sessionIDUsed_ = true;
}

/// \copydoc gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext
void MultiplexerPort::RequestExecutionContext(void)
{
  gpcc::osal::MutexLocker portMutexLocker(owner_.portMutex_);

  switch (state_)
  {
    case States::noClientRegistered:
      throw std::logic_error("MultiplexerPort::RequestExecutionContext: No client registered");

    case States::notReady:
      throw RemoteAccessServerNotReadyError();

    case States::ready:
      break;
  }

  if (owner_.pRODA_ == nullptr)
    PANIC();

  owner_.pRODA_->RequestExecutionContext();
  execContextRequested_ = true;
}

// --> IRemoteObjectDictionaryAccess

} // namespace cood
} // namespace gpcc
