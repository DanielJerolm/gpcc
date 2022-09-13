/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IIRQ2THREADWAKEUP_HPP_202209101537
#define IIRQ2THREADWAKEUP_HPP_202209101537

namespace gpcc {
namespace time {
  class TimeSpan;
}}

namespace gpcc  {
namespace stdif {

/**
 * \ingroup GPCC_STDIF_NOTIFY
 * \brief Interface for classes implementing a mechanism for unblocking a thread from managed interrupt context.
 *
 * # Functionality
 * This interface offers a functionality similar to a binary semaphore:\n
 * A wake-up flag is set via @ref SignalFromISR() or @ref SignalFromThread(). If any thread is blocked in one of the
 * methods @ref Wait() or @ref WaitWithTimeout(), then the flag will be cleared and the thread will be woken-up
 * immediately.
 *
 * If no thread is blocked in any of the wait-methods when any of the signal-methods is invoked, then the flag remains
 * set. The next thread calling one of the wait-methods will then consume the flag and return immediately without being
 * blocked.
 *
 * # Missed signal detection
 * The intended use of this interface is, that there is a blocked thread when any of the signal-methods are invoked.
 *
 * This interface allows to detect deviations from this scenario:
 * - The signal-methods provide a return value indicating if there was a blocked thread or not.
 * - The wait-methods provide a return value indicating if the wake-up flag was already set when the method was invoked,
 *   or if the thread was blocked and woken up by the signal-method.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IIRQ2ThreadWakeup
{
  public:
    /// Enumeration with return values for the wait-methods.
    enum class Result
    {
      OK,                ///<OK, thread was blocked and then woken up by assertion of the wake-up flag.
      Timeout,           ///<Thread was blocked and woken up due to timeout expiration. The wake-up flag is not set.
      AlreadySignalled   ///<Thread was not blocked, because the wake-up flag was already set.
    };

    virtual bool SignalFromISR(void) noexcept = 0;
    virtual bool SignalFromThread(void) = 0;

    virtual Result Wait(void) = 0;
    virtual Result WaitWithTimeout(time::TimeSpan const & timeout) = 0;

  protected:
    IIRQ2ThreadWakeup(void) noexcept = default;
    IIRQ2ThreadWakeup(IIRQ2ThreadWakeup const &) = delete;
    IIRQ2ThreadWakeup(IIRQ2ThreadWakeup &&) noexcept = default;
    virtual ~IIRQ2ThreadWakeup(void) = default;

    IIRQ2ThreadWakeup& operator=(IIRQ2ThreadWakeup const &) = delete;
    IIRQ2ThreadWakeup& operator=(IIRQ2ThreadWakeup &&) noexcept = default;
};


/**
 * \fn virtual bool IIRQ2ThreadWakeup::SignalFromISR(void) = 0
 * \brief Sets the wake-up flag and wakes up a blocked thread (if any).
 *
 * If the wake-up flag is already set, then this method has no effect.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This must be executed in managed interrupt context only.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   The wake-up flag was already set.
 * \retval false  The wake-up flag was not yet set.
 */

/**
 * \fn virtual bool IIRQ2ThreadWakeup::SignalFromThread(void) = 0
 * \brief Sets the wake-up flag and wakes up a blocked thread (if any).
 *
 * If the wake-up flag is already set, then this method has no effect.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \retval true   The wake-up flag was already set.
 * \retval false  The wake-up flag was not yet set.
 */

/**
 * \fn virtual IIRQ2ThreadWakeup::Result IIRQ2ThreadWakeup::Wait(void) = 0
 * \brief Blocks the calling thread until the wake-up flag is set and clears the flag.
 *
 * It is recommended to have only one thread invoking this method or @ref WaitWithTimeout() at any time. If multiple
 * threads are blocked in these methods, then it depends on the underlying implementation and on the operating system
 * which thread is woken up.
 *
 * If the wake-up flag is already set, then the thread returns immediately. Otherwise the thread is blocked until any
 * of the signal-methods is called.
 *
 * The wake-up flag is always cleared.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \retval Result::OK                Thread was blocked and woken up by assertion of the wake-up flag.
 * \retval Result::AlreadySignalled  Thread was not blocked because wake-up flag was already set.
 */

/**
 * \fn virtual IIRQ2ThreadWakeup::Result IIRQ2ThreadWakeup::WaitWithTimeout(time::TimeSpan const & timeout) = 0
 * \brief Blocks the calling thread (with timeout) until the wake-up flag is set and clears the flag.
 *
 * It is recommended to have only one thread invoking this method or @ref Wait() at any time. If multiple threads are
 * blocked in these methods, then it depends on the underlying implementation and on the operating system which thread
 * is woken up.
 *
 * If the wake-up flag is already set, then the thread returns immediately. Otherwise the thread is blocked until any
 * of the signal-methods is called or a timeout occurs.
 *
 * The wake-up flag is always cleared.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param timeout
 * Timeout after which the thread is woken up if the wake-up flag is not set.
 *
 * \retval Result::OK                Thread was blocked and woken up by assertion of the wake-up flag.
 * \retval Result::Timeout           Thread was blocked and the timeout expired. The wake-up flag is not set.
 * \retval Result::AlreadySignalled  Thread was not blocked because wake-up flag was already set.
 */

} // namespace stdif
} // namespace gpcc

#endif // IIRQ2THREADWAKEUP_HPP_202209101537
