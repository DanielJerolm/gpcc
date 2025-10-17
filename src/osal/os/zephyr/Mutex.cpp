/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR

#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Panic.hpp>
#include <system_error>

namespace gpcc {
namespace osal {

/**
 * \brief Constructor. Creates a new (unlocked) @ref Mutex object.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
Mutex::Mutex(void)
{
  int const status = k_mutex_init(&mutex_);

  if (status != 0)
    throw std::system_error(status, std::generic_category(), "k_mutex_init() failed");
}

/**
 * \brief Destructor.
 *
 * \pre   The mutex must not be locked by any thread.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Mutex::~Mutex(void)
{
  // Lock for accessing 'owner' is not accessible.
  // Testing without the required lock will not create false positives, but there is a small chance that a usage error
  // is not detected. This is acceptable, since the altenative is to have no check at all.
  if (mutex_.owner != nullptr)
    Panic("Mutex::~Mutex: Mutex is locked");
}

/**
 * \brief Locks the mutex.
 *
 * If the mutex is already locked by another thread, then this method will block until the other thread unlocks the
 * mutex and this thread acquires the mutex.
 *
 * \pre   The mutex must not yet be acquired by the calling thread.
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
void Mutex::Lock(void)
{
  int const status = k_mutex_lock(&mutex_, K_FOREVER);

  if (status == 0)
  {
    if (mutex_.lock_count > 1U)
      Panic("Mutex::Lock: Recursion");
  }
  else
  {
    throw std::system_error(status, std::generic_category(), "k_mutex_lock() failed");
  }
}

/**
 * \brief Tries to lock the mutex.
 *
 * Same as @ref Lock(), but returns immediately if the mutex is already locked by the calling thread or by
 * another thread.
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
 *
 * - - -
 *
 * \retval true
 *    The mutex has been locked by the calling thread.
 * \retval false
 *    The mutex is already locked by the calling thread __or__ by another thread.
 */
bool Mutex::TryLock(void)
{
  int status = k_mutex_lock(&mutex_, K_NO_WAIT);

  switch (status)
  {
    case 0:
    {
      // First lock?
      if (mutex_.lock_count == 1U)
        return true;

      // ...otherwise the mutex is already locked by the calling thread.
      // Unlock once to undo the recursive lock and return false.
      status = k_mutex_unlock(&mutex_);
      if (status != 0U)
        Panic("Mutex::TryLock: Undo failed");

      return false;
    }

    case -EBUSY:
      return false;

    default:
      throw std::system_error(status, std::generic_category(), "k_mutex_lock() failed");
  }
}

/**
 * \brief Unlocks the mutex.
 *
 * \pre   The mutex must be the latest (most recent) mutex locked by the calling thread.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This must only be invoked by the thread who has locked the mutex.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void Mutex::Unlock(void) noexcept
{
  if (mutex_.owner != k_current_get())
    Panic("Mutex::Unlock");

  int const status = k_mutex_unlock(&mutex_);
  if (status != 0)
    Panic("Mutex::Unlock");
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_ZEPHYR
