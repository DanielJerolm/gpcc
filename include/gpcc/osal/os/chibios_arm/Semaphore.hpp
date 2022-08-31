/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_CHIBIOS_ARM

#ifndef SEMAPHORE_HPP_201701290858
#define SEMAPHORE_HPP_201701290858

#include <ch.h>
#include <limits>

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A counting semaphore.
 *
 * # Features
 * - Initial value configurable during instantiation
 * - Post/inkrement and wait/decrement
 *
 * # Constraints/Restrictions
 * - _All threads using instances of class Semaphore must live in the same process._
 *
 * # Alternatives
 * There are further semaphore implementations available in @ref GPCC_RESOURCEMANAGEMENT ->
 * @ref GPCC_RESOURCEMANAGEMENT_SEMAPHORES.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Semaphore final
{
  public:
    /// Maximum value of the semaphore.
    static size_t const MAX = std::numeric_limits<cnt_t>::max();


    Semaphore(void) = delete;
    explicit Semaphore(size_t const initialValue);
    Semaphore(Semaphore const &) = delete;
    Semaphore(Semaphore &&) = delete;
    ~Semaphore(void);

    Semaphore& operator=(Semaphore const &) = delete;
    Semaphore& operator=(Semaphore &&) = delete;

    void Post(void);
    void Wait(void);

  private:
    /// ChibiOS semaphore structure.
    semaphore_t sem;
};

/**
 * \brief Increments (posts) the semaphore.
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
 * No cancellation point included.
 */
inline void Semaphore::Post(void)
{
  chSemSignal(&sem);
}

/**
 * \brief Decrements the semaphore's counter if it is greater than zero or waits if the counter is zero.
 *
 * The semaphore's counter cannot become negative. If the counter is already zero, then it can't be decremented any
 * more and the thread will be blocked until another thread invokes @ref Post().
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
 * Strong guarantee.\n
 * On some systems, this method contains a cancellation point.
 */
inline void Semaphore::Wait(void)
{
  chSemWait(&sem);
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef SEMAPHORE_HPP_201701290858
#endif // #ifdef OS_CHIBIOS_ARM
