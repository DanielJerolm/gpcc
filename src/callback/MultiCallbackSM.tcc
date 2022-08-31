/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/MutexLocker.hpp>
#include <stdexcept>

namespace gpcc {
namespace callback {

/**
 * \brief Constructor.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _pMutex
 * Mutex to be used by this @ref MultiCallbackSM instance to provide thread-safety.\n
 * The same mutex may be shared among multiple @ref MultiCallbackSM instances.\n
 * _The mutex must not be released before this @ref MultiCallbackSM instance._\n
 * _nullptr is not allowed._
 */
template<typename... ARGS>
MultiCallbackSM<ARGS...>::MultiCallbackSM(gpcc::osal::Mutex* const _pMutex)
: ICallback<ARGS...>()
, pMutex(_pMutex)
, callbacks()
{
  if (pMutex == nullptr)
    throw std::invalid_argument("MultiCallbackSM::MultiCallbackSM: !_pMutex");
}

/// \copydoc ICallback::Register()
template<typename... ARGS>
void MultiCallbackSM<ARGS...>::Register(void const * const pClient, std::function<void(ARGS...)> const & callback)
{
  if ((pClient == nullptr) || (!callback))
    throw std::invalid_argument("MultiCallbackTS::Register: Invalid parameter(s)");

  gpcc::osal::MutexLocker mutexLocker(pMutex);

  for (auto const & e: callbacks)
  {
    if (e.first == pClient)
      throw std::logic_error("MultiCallbackTS::Register: Client already registered");
  }

  callbacks.emplace_front(pClient, callback);
}

/// \copydoc ICallback::Unregister()
template<typename... ARGS>
void MultiCallbackSM<ARGS...>::Unregister(void const * const pClient) noexcept
{
  if (pClient == nullptr)
    return;

  gpcc::osal::MutexLocker mutexLocker(pMutex);

  auto prev = callbacks.before_begin();
  auto it = callbacks.begin();
  while (it != callbacks.end())
  {
    if ((*it).first == pClient)
    {
      callbacks.erase_after(prev);
      return;
    }
    prev = it;
    ++it;
  }
}

/**
 * \brief Invokes all registered callbacks. The mutex passed to this class' constructor
 * __must not__ be locked by the caller.
 *
 * __Note__:
 * - The registered callbacks are invoked in the context of this thread.
 * - The order in which the callbacks are invoked is random.
 * - This method blocks until all callbacks have been invoked.
 * - If any callback throws or cancels the thread, then the remaining callbacks will not be invoked.\n
 *   This method does not contain any code to catch potential exceptions thrown by callbacks.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * The mutex passed to this class' constructor __must not__ be locked by the caller.\n
 * See @ref NotifyMutexAlreadyLocked() for an alternative which expects the mutex being already
 * locked by the caller.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Not all registered callbacks may be invoked
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included, but registered callbacks might include cancellation points.\n
 * In case of deferred cancellation, not all registered callbacks may be invoked.
 *
 * ---
 *
 * \param args
 * Arguments to be passed to each callback.
 */
template<typename... ARGS>
void MultiCallbackSM<ARGS...>::Notify(ARGS... args)
{
  gpcc::osal::MutexLocker mutexLocker(pMutex);
  for (auto & e: callbacks)
    e.second(args...);
}

/**
 * \brief Invokes all registered callbacks. The mutex passed to this class' constructor
 * __must__ be locked by the caller.
 *
 * __Note__:
 * - The registered callbacks are invoked in the context of this thread.
 * - The order in which the callbacks are invoked is random.
 * - This method blocks until all callbacks have been invoked.
 * - If any callback throws or cancels the thread, then the remaining callbacks will not be invoked.\n
 *   This method does not contain any code to catch potential exceptions thrown by callbacks.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * The mutex passed to this class' constructor __must__ be locked by the caller.\n
 * See @ref Notify() for an alternative which locks the mutex itself.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Not all registered callbacks may be invoked
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included, but registered callbacks might include cancellation points.\n
 * In case of deferred cancellation, not all registered callbacks may be invoked.
 *
 * ---
 *
 * \param args
 * Arguments to be passed to each callback.
 */
template<typename... ARGS>
void MultiCallbackSM<ARGS...>::NotifyMutexAlreadyLocked(ARGS... args)
{
  for (auto & e: callbacks)
    e.second(args...);
}

} // namespace callback
} // namespace gpcc
