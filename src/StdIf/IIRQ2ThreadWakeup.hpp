/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_STDIF_IIRQ2THREADWAKEUP_HPP_
#define SRC_GPCC_STDIF_IIRQ2THREADWAKEUP_HPP_

namespace gpcc
{

namespace time
{
  class TimeSpan;
}

namespace StdIf
{

/**
 * \ingroup GPCC_STDIF
 * \brief Common interface base class for classes delivering triggers from IRQ to thread context.
 *
 * Drivers can subclass this class in order to wake-up an _(application) thread_ by an event handler that is
 * executed in _managed interrupt context_.
 *
 * The class works similar to a binary semaphore:\n
 * Via method @ref SignalFromISR() or @ref SignalFromThread() the wake-up flag is set. If any thread is blocked
 * in one of the methods @ref Wait() or @ref WaitWithTimeout(), then the flag is cleared and the thread is woken-up.
 * If no thread is blocked, then the next thread calling one of the wait-methods will not block. Instead the flag is
 * cleared and the thread immediately returns.
 */
class IIRQ2ThreadWakeup
{
  public:
    /// Enumeration with return values for the wait-methods.
    enum class Result
    {
      OK,                ///<OK, thread was blocked and then woken up.
      Timeout,           ///<Thread was blocked and woken up due to timeout expiration.
      AlreadySignalled   ///<Thread was not blocked because wake-up flag was already set.
    };

    IIRQ2ThreadWakeup(void) = default;
    IIRQ2ThreadWakeup(IIRQ2ThreadWakeup const &) = delete;
    IIRQ2ThreadWakeup(IIRQ2ThreadWakeup &&) = delete;

    IIRQ2ThreadWakeup& operator=(IIRQ2ThreadWakeup const &) = delete;
    IIRQ2ThreadWakeup& operator=(IIRQ2ThreadWakeup &&) = delete;

    virtual void SignalFromISR(void) noexcept = 0;
    virtual void SignalFromThread(void) = 0;

    virtual Result Wait(void) = 0;
    virtual Result WaitWithTimeout(time::TimeSpan const & timeout) = 0;

  protected:
    virtual ~IIRQ2ThreadWakeup(void) = default;
};

/**
 * \fn virtual void IIRQ2ThreadWakeup::SignalFromISR(void) = 0
 * \brief Sets the wake-up flag and wakes up a blocked thread (if any).
 *
 * If the wake-up flag is already set, then nothing happens.
 *
 * ---
 *
 * __Thread safety:__\n
 * This must be executed in managed interrupt context only.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.\n
 * Operations may also fail due to serious errors that will result in program termination via Panic(...).
 *
 * __Thread cancellation safety:__\n
 * Not applicable, interrupt context.
 */

/**
 * \fn virtual void IIRQ2ThreadWakeup::SignalFromThread(void) = 0
 * \brief Sets the wake-up flag and wakes up a blocked thread (if any).
 *
 * If the wake-up flag is already set, then nothing happens.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 */

/**
 * \fn virtual IIRQ2ThreadWakeup::Result IIRQ2ThreadWakeup::Wait(void) = 0
 * \brief Blocks the calling thread until the wake-up flag is set.
 *
 * It is recommended to have only one thread invoking this method and @ref WaitWithTimeout().
 * If multiple-threads are blocked in this method, then it depends on the underlying OS
 * which thread is woken up.
 *
 * If the wake-up flag is already set, then the thread immediately returns. Otherwise the thread is blocked until
 * any of the signal-methods is called.
 *
 * The wake-up flag is always cleared.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * @ref Result::OK = OK, thread was blocked and woken up by assertion of wake-up flag.\n
 * @ref Result::AlreadySignalled = Thread was not blocked because wake-up flag was already set.
 */

/**
 * \fn virtual IIRQ2ThreadWakeup::Result IIRQ2ThreadWakeup::WaitWithTimeout(time::TimeSpan const & timeout) = 0
 * \brief Blocks the calling thread until the wake-up flag is set or a timeout occurs.
 *
 * It is recommended to have only one thread invoking this method and @ref Wait().
 * If multiple-threads are blocked in this method, then it depends on the underlying OS
 * which thread is woken up.
 *
 * If the wake-up flag is already set, then the thread immediately returns. Otherwise the thread is blocked until
 * any of the signal-methods is called or a timeout occurs.
 *
 * The wake-up flag is always cleared.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param timeout
 * Minimum time-span after which the thread is woken up if the wake-up flag is not set before.
 * \return
 * @ref Result::OK = OK, thread was blocked and then woken up.\n
 * @ref Result::Timeout = Thread was blocked and then woken up due to timeout expiration.\n
 * @ref Result::AlreadySignalled = Thread was not blocked because wake-up flag was already set.
 */

} // namespace StdIf
} // namespace gpcc

#endif // SRC_GPCC_STDIF_IIRQ2THREADWAKEUP_HPP_
