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

#ifdef OS_LINUX_ARM_TFC

#include "UnmanagedMutex.hpp"
#include "../Panic.hpp"
#include <system_error>
#include <cerrno>

namespace gpcc {
namespace osal {
namespace internal {

// Helper class for class UnmanagedMutex. Provides an initialized pthread_mutexattr_t structure
// to the constructor of class UnmanagedMutex.
class MutexAttr final
{
    friend class UnmanagedMutex;

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
 * \brief Constructor. Creates a new (unlocked) @ref UnmanagedMutex object.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
UnmanagedMutex::UnmanagedMutex(void)
{
  static MutexAttr mutexAttr;

  int const status = pthread_mutex_init(&mutex, &mutexAttr.mutexAttr);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_mutex_init(...) failed");
}

/**
 * \brief Destructor.
 *
 * _The mutex must not be locked by any thread._
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
UnmanagedMutex::~UnmanagedMutex(void)
{
  if (pthread_mutex_destroy(&mutex) != 0)
    PANIC();
}

/**
 * \brief Locks the mutex.
 *
 * If the mutex is already locked by another thread, then this method blocks until
 * the other thread unlocks the mutex and this thread acquires the mutex.
 *
 * _The mutex must not yet be acquired by the calling thread._
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
 * Safe, no cancellation point included.
 */
void UnmanagedMutex::Lock(void)
{
  int const status = pthread_mutex_lock(&mutex);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "pthread_mutex_lock(...) failed");
}

/**
 * \brief Tries to lock the mutex.
 *
 * Same as @ref Lock(), but this returns immediately if the mutex is already locked by the calling thread or
 * by another thread.
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
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * true  = mutex has been locked\n
 * false = mutex is already locked by the calling thread __or__ by another thread
 */
bool UnmanagedMutex::TryLock(void)
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
 * _The mutex must be locked by the calling thread._
 *
 * ---
 *
 * __Thread safety:__\n
 * This must only be invoked by the thread which has locked the mutex.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
void UnmanagedMutex::Unlock(void) noexcept
{
  if (pthread_mutex_unlock(&mutex) != 0)
    PANIC();
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
