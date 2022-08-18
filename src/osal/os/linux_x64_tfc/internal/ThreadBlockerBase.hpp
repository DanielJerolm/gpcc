/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#ifndef THREADBLOCKERBASE_HPP_201703031421
#define THREADBLOCKERBASE_HPP_201703031421

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

#endif // #ifndef THREADBLOCKERBASE_HPP_201703031421
#endif // #ifdef OS_LINUX_X64_TFC
