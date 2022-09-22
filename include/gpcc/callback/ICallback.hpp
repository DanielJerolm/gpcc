/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef ICALLBACK_HPP_201702051725
#define ICALLBACK_HPP_201702051725

#include <functional>

namespace gpcc {
namespace callback {

/**
 * \ingroup GPCC_CALLBACK
 * \brief Interface for registering and unregistering callbacks at an @ref MultiCallback or @ref MultiCallbackSM.
 *
 * \tparam ARGS
 * Zero, one, or more data types indicating the number and type of parameters which shall be passed
 * to the registered callback function(s).
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * Pay attention to the notes on thread-safety supplied with the object which offers this interface.
 */
template<typename... ARGS>
class ICallback
{
  public:
    /// Type definition of callback functor.
    using tFunctor = std::function<void(ARGS...)>;


    ICallback(ICallback const &) = delete;
    ICallback(ICallback &&) = delete;


    ICallback& operator=(ICallback const &) = delete;
    ICallback& operator=(ICallback &&) = delete;


    virtual void Register(void const * const pClient, tFunctor const & callback) = 0;
    virtual void Unregister(void const * const pClient) noexcept = 0;

  protected:
    ICallback(void) = default;
    virtual ~ICallback(void) = default;
};

/**
 * \fn ICallback::Register
 * \brief Registers a client's callback.
 *
 * Note: The registered callback may be invoked before this method returns.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pClient
 * Pointer to the client who wants to register the callback.\n
 * _nullptr is not allowed._\n
 * _Any client must not register twice at the same @ref ICallback interface._
 * \param callback
 * Callback that shall be registered.\n
 * _A copy is generated._\n
 * _This must not be an empty functor._\n
 * The callback will be invoked in the context of the thread invoking @ref MultiCallback::Notify(). This may
 * be any thread or a work queue. Watch out the documentation provided by the object which offers this interface.
 */

/**
 * \fn ICallback::Unregister
 * \brief Unregisters a client's callback.
 *
 * After the call to this method has returned, the client's callback will not be invoked any more.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pClient
 * Pointer to the client who wants to unregister its callback.\n
 * If this is `nullptr`, then this method does nothing.\n
 * If the client is not registered, then this method does nothing.
 */

} // namespace callback
} // namespace gpcc

#endif // #ifndef ICALLBACK_HPP_201702051725
