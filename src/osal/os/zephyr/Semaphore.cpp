/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR

#include <gpcc/osal/Semaphore.hpp>
#include <gpcc/osal/Panic.hpp>
#include <stdexcept>
#include <system_error>

namespace gpcc {
namespace osal {

size_t const Semaphore::MAX;

/**
 * \brief Constructor. Creates a semaphore with a configurable initial value.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param initialValue
 * Initial value for the semaphore's counter.
 */
Semaphore::Semaphore(size_t const initialValue)
{
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if (initialValue > MAX)
    throw std::invalid_argument("Invalid args");
  #pragma GCC diagnostic pop

  int const status = k_sem_init(&sem_, initialValue, K_SEM_MAX_LIMIT);

  if (status != 0)
    throw std::system_error(status, std::generic_category(), "k_sem_init() failed");
}

/**
 * \brief Destructor.
 *
 * \pre   No thread must be blocked on the semaphore.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Semaphore::~Semaphore(void)
{
}

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
void Semaphore::Post(void)
{
  // k_sem_give() doesn't report if the semaphore's counter exceeds its maximum value, so we have to check here.
  // We cannot perform the check atomically with k_sem_give(). This means that there will be no false positives, but
  // there is a small chance that the semaphore's counter exceeds MAX. However, the probability that a call to Post() is
  // dropped, is close to zero, because the technical limit for the semaphore's counter is 2x MAX.
  if (k_sem_count_get(&sem_) >= MAX)
    throw std::logic_error("Semaphore::Post: Count at MAX");

  k_sem_give(&sem_);
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
void Semaphore::Wait(void)
{
  int const status = k_sem_take(&sem_, K_FOREVER);

  if (status != 0)
    throw std::system_error(status, std::generic_category(), "k_sem_take() failed");
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_ZEPHYR
