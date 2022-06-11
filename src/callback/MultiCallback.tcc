/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
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
