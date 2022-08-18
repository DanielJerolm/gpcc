/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "MultiCallback.hpp"
#include <stdexcept>

namespace {
  // used to get rid of warnings about unused parameters in a variadic template
  struct sink { template<typename ...Args> sink(Args const & ... ) {} };
}

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
 */
template<typename... ARGS>
MultiCallback<ARGS...>::MultiCallback(void)
: MultiCallbackSM<ARGS...>()
, mutex()
{
  this->pMutex = &mutex;
}

/**
 * \brief Replacement for the base-class' implementation of this method.
 *
 * This is not allowed to be invoked on an @ref MultiCallback.\n
 * This will always throw.
 *
 * ---
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param args
 * All ignored.
 */
template<typename... ARGS>
void MultiCallback<ARGS...>::NotifyMutexAlreadyLocked(ARGS... args)
{
  sink(args...);
  throw std::logic_error("MultiCallback<ARGS...>::NotifyMutexAlreadyLocked: Forbidden");
}

} // namespace callback
} // namespace gpcc
