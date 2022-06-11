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

#ifdef OS_LINUX_ARM_TFC

#ifndef THREADBLOCKERBASE_HPP_201904071048
#define THREADBLOCKERBASE_HPP_201904071048

namespace gpcc {
namespace osal {
namespace internal {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Base class for all ThreadBlocker-implementations.
 *
 * This is the base class for class @ref ThreadBlocker and class @ref TimeLimitedThreadBlocker.
 *
 * Both classes allow to block threads until a wake-up condition is signaled. In addition to this,
 * @ref TimeLimitedThreadBlocker also wakes up the blocked thread if a timeout condition occurs.
 *
 * Thread blockers are used by the _TFC-managed_ condition variable implementation (class @ref ConditionVariable)
 * and by the _TFC-managed_ thread implementation (class @ref Thread) offered by GPCC's TFC feature.
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ThreadBlockerBase
{
  public:
    ThreadBlockerBase(ThreadBlockerBase const &) = delete;
    ThreadBlockerBase(ThreadBlockerBase &&) = delete;
    virtual ~ThreadBlockerBase(void) = default;

    ThreadBlockerBase& operator=(ThreadBlockerBase const &) = delete;
    ThreadBlockerBase& operator=(ThreadBlockerBase &&) = delete;

    virtual void Signal(void) = 0;

  protected:
    ThreadBlockerBase(void) = default;
};

/**
 * \fn virtual void ThreadBlockerBase::Signal(void) = 0
 * \brief Signals that the blocked thread (if any) is allowed to continue.
 *
 * After calling this, any subsequent call to `Block()` will not block the calling thread any more.\n
 * After calling this, any subsequent call to this will be treated as an error.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef THREADBLOCKERBASE_HPP_201904071048
#endif // #ifdef OS_LINUX_ARM_TFC
