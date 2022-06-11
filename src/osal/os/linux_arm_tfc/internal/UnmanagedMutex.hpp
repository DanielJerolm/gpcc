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

#ifndef UNMANAGEDMUTEX_HPP_201904071050
#define UNMANAGEDMUTEX_HPP_201904071050

#include <pthread.h>

namespace gpcc     {
namespace osal     {
namespace internal {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief A native, unmanaged mutex.
 *
 * This class provides a trivial mutex with the following properties:
 * - no recursive locking
 * - presence of priority inheritance protocol depends on underlying OS
 * - methods: lock, unlock, and non-blocking try-lock
 *
 * __This mutex is completely based on the underlying OS and it is not managed by GPCC's TFC feature.__\n
 * __This mutex implementation is intended to be used by the internals of TFC only.__
 *
 * It is recommended to use this class in conjunction with class @ref UnmanagedMutexLocker and
 * @ref AdvancedUnmanagedMutexLocker.
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class UnmanagedMutex final
{
  friend class UnmanagedConditionVariable;

  public:
    UnmanagedMutex(void);
    UnmanagedMutex(UnmanagedMutex const &) = delete;
    UnmanagedMutex(UnmanagedMutex &&) = delete;
    ~UnmanagedMutex(void);

    UnmanagedMutex& operator=(UnmanagedMutex const &) = delete;
    UnmanagedMutex& operator=(UnmanagedMutex &&) = delete;

    void Lock(void);
    bool TryLock(void);
    void Unlock(void) noexcept;

  private:
    /// The encapsulated pthread-mutex.
    pthread_mutex_t mutex;
};

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef UNMANAGEDMUTEX_HPP_201904071050
#endif // #ifdef OS_LINUX_ARM_TFC
