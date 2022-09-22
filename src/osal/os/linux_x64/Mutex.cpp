/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64

#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Panic.hpp>
#include <system_error>
#include <cerrno>

namespace gpcc {
namespace osal {

// Helper class for class Mutex. Provides an initialized pthread_mutexattr_t structure
// to the constructor of class Mutex.
class MutexAttr final
{
    friend class Mutex;

  public:
    MutexAttr(void)
    {
      int status;

      status = pthread_mutexattr_init(&mutexAttr);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_mutexattr_init(...) failed");

      status = pthread_mutexattr_setprotocol(&mutexAttr, PTHREAD_PRIO_INHERIT);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_mutexattr_setprotocol(...) failed");

      status = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_NORMAL);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_mutexattr_settype(...) failed");
    }

    MutexAttr(MutexAttr const &) = delete;
    MutexAttr(MutexAttr &&) = delete;

    ~MutexAttr(void)
    {
      if (pthread_mutexattr_destroy(&mutexAttr) != 0)
        PANIC();
    }

    MutexAttr& operator=(MutexAttr const &) = delete;
    MutexAttr& operator=(MutexAttr &&) = delete;

  private:
    pthread_mutexattr_t mutexAttr;
};


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
  static MutexAttr mutexAttr;

  int const status = pthread_mutex_init(&mutex, &mutexAttr.mutexAttr);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_mutex_init(...) failed");
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
  if (pthread_mutex_destroy(&mutex) != 0)
    PANIC();
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
  int const status = pthread_mutex_lock(&mutex);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_mutex_lock(...) failed");
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
  int const status = pthread_mutex_trylock(&mutex);

  if (status == EBUSY)
    return false;
  else if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_mutex_trylock(...) failed");
  else
    return true;
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
  if (pthread_mutex_unlock(&mutex) != 0)
    PANIC();
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_X64
