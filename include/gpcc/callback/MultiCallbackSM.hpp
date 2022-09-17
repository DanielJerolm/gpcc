/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef MULTICALLBACKSM_HPP_201702062029
#define MULTICALLBACKSM_HPP_201702062029

#include <gpcc/callback/ICallback.hpp>
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
