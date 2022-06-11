/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef MULTICALLBACK_HPP_201702062036
#define MULTICALLBACK_HPP_201702062036

#include "MultiCallbackSM.hpp"
#include "gpcc/src/osal/Mutex.hpp"

namespace gpcc {
namespace callback {

/**
 * \ingroup GPCC_CALLBACK
 * \brief Delivery of callbacks to one or more registered clients (own mutex).
 *
 * The base class @ref ICallback can be offered to clients for registration and
 * unregistration of callbacks.\n
 * By invocation of @ref Notify() (offered by base class @ref MultiCallbackSM), the owner of
 * this class' instance can invoke all registered callbacks.
 *
 * This class contains its own mutex to provide thread-safety between interface @ref ICallback
 * and @ref Notify(). \n
 * Use class @ref MultiCallbackSM instead of this, if you want to provide your own mutex.
 * Multiple instances of class @ref MultiCallback can share the same mutex.
 *
 * \tparam ARGS
 * Zero, one, or more data types indicating the number and type of parameters which shall be passed
 * to the registered callback function(s).
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
template<typename... ARGS>
class MultiCallback: public MultiCallbackSM<ARGS...>
{
  public:
    MultiCallback(void);
    MultiCallback(MultiCallback const &) = delete;
    MultiCallback(MultiCallback &&) = delete;
    ~MultiCallback(void) = default;

    MultiCallback& operator=(MultiCallback const &) = delete;
    MultiCallback& operator=(MultiCallback &&) = delete;

    void NotifyMutexAlreadyLocked(ARGS... args) override;

  private:
    /// Mutex used to make the API thread safe.
    gpcc::osal::Mutex mutex;
};

} // namespace callback
} // namespace gpcc

#include "MultiCallback.tcc"

#endif // MULTICALLBACK_HPP_201702062036
