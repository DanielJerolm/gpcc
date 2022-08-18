/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
