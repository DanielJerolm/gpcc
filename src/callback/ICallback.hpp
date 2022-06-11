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
