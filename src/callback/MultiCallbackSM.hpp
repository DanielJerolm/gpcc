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

#ifndef MULTICALLBACKSM_HPP_201702062029
#define MULTICALLBACKSM_HPP_201702062029

#include "ICallback.hpp"
#include <forward_list>
#include <utility>

namespace gpcc {

namespace osal {
  class Mutex;
}

namespace callback {

/**
 * \ingroup GPCC_CALLBACK
 * \brief Delivery of callbacks to one or more registered clients (foreign/shared mutex).
 *
 * The base class @ref ICallback can be offered to clients for registration and
 * unregistration of callbacks.\n
 * By invocation of @ref Notify() or @ref NotifyMutexAlreadyLocked(), the owner of
 * this class' instance can invoke all registered callbacks.
 *
 * This class does not contain an own mutex to provide thread-safety between interface
 * @ref ICallback and @ref Notify() / @ref NotifyMutexAlreadyLocked(). Instead, a pointer
 * to an mutex must be provided to the constructor.\n
 * Use class @ref MultiCallback instead of this, if you do not want to provide a mutex.
 * However, multiple instances of class @ref MultiCallback could share the same mutex.
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
class MultiCallbackSM: public ICallback<ARGS...>
{
  public:
    MultiCallbackSM(gpcc::osal::Mutex* const _pMutex);
    MultiCallbackSM(MultiCallbackSM const &) = delete;
    MultiCallbackSM(MultiCallbackSM &&) = delete;
    virtual ~MultiCallbackSM(void) = default;

    MultiCallbackSM& operator=(MultiCallbackSM const &) = delete;
    MultiCallbackSM& operator=(MultiCallbackSM &&) = delete;

    // --> ICallback
    void Register(void const * const pClient, std::function<void(ARGS...)> const & callback) override;
    void Unregister(void const * const pClient) noexcept override;
    // <-- ICallback

    void Notify(ARGS... args);
    virtual void NotifyMutexAlreadyLocked(ARGS... args);

  protected:
    /// Mutex used to make stuff thread-safe.
    gpcc::osal::Mutex* pMutex = nullptr;

    MultiCallbackSM(void) = default;

  private:
    /// Type definition of an std::pair combining the client and his callback into one list item.
    using tListItem = std::pair<void const*, std::function<void(ARGS...)>>;

    /// List with registered callbacks.
    /** @ref pMutex is required.\n
        An forward list has been choosen because it's emplace-method provides the strong guarantee
        in conjunction with std::function.*/
    std::forward_list<tListItem> callbacks;
};

} // namespace callback
} // namespace gpcc

#include "MultiCallbackSM.tcc"

#endif // MULTICALLBACKSM_HPP_201702062029
