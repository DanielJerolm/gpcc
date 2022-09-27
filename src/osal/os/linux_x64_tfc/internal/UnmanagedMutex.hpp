/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#ifndef UNMANAGEDMUTEX_HPP_201702252110
#define UNMANAGEDMUTEX_HPP_201702252110

#include <pthread.h>

namespace gpcc     {
namespace osal     {
namespace internal {

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \class UnmanagedMutex UnmanagedMutex.hpp "src/osal/os/linux_x64_tfc/internal/UnmanagedMutex.hpp"
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

#endif // #ifndef UNMANAGEDMUTEX_HPP_201702252110
#endif // #ifdef OS_LINUX_X64_TFC
