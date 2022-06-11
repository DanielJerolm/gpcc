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

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#ifndef MUTEXLOCKER_HPP_201701271644
#define MUTEXLOCKER_HPP_201701271644

#include "../Mutex.hpp"

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING GPCC_RAII
 * \brief Automatic locker/unlocker for mutexes (see class @ref Mutex).
 *
 * The classes @ref MutexLocker and @ref AdvancedMutexLocker are both convenient classes, which allow to
 * automatically lock and unlock mutexes (see class @ref Mutex) based on the RAII pattern.\n
 * Both classes lock a given mutex when they are instantiated.\n
 * Both classes unlock the mutex again when they are released.
 *
 * The classes are intended to be instantiated on the stack. They allow methods to return at any time
 * without forgetting to unlock the mutex and they ensure that the managed mutex is unlocked in case of
 * exceptions or deferred thread cancellation.
 *
 * The classes are intended to be used in a multi-threaded environment, but each instance of the
 * classes shall be used within a single thread only. This means that the thread which instantiated an
 * (Advanced)MutexLocker also releases the (Advanced)MutexLocker. The thread which instantiated an
 * @ref AdvancedMutexLocker is also the only thread which is allowed to invoke `Unlock()` and `Relock()`
 * on that @ref AdvancedMutexLocker instance.
 *
 * Class @ref AdvancedMutexLocker provides the following additional features:
 * - The mutex can be explicitly unlocked and relocked during the lifetime of the @ref AdvancedMutexLocker.
 *
 * An @ref AdvancedMutexLocker can be move-constructed from either an @ref AdvancedMutexLocker or from a
 * @ref MutexLocker. \n
 * A @ref MutexLocker can be move-constructed from another @ref MutexLocker only.
 *
 * Using class @ref MutexLocker is theoretically for free. It can be completely optimized away by the compiler.\n
 * Using class @ref AdvancedMutexLocker needs to track the current lock-state of the mutex and comes with the
 * expense of at least one bool variable on the stack. However, modern compilers should be able to optimize
 * that variable away in most cases.
 *
 * The automatic mutex lockers @ref MutexLocker and @ref AdvancedMutexLocker can be used safely in
 * conjunction with class @ref ConditionVariable. Example:
 * ~~~{.cpp}
 * gpcc::osal::MutexLocker(myMutex);
 *
 * while (state != state_OK)
 *   stateCondVar.Wait(myMutex);
 * ~~~
 *
 * However, when using an @ref AdvancedMutexLocker, one must ensure that the managed mutex is locked
 * when invoking `stateCondVar.Wait(...)`.
 *
 * Using automatic mutex lockers in conjunction with the wait-methods of class @ref ConditionVariable
 * is safe, because all @ref ConditionVariable implementation guarantee that the mutex is always locked when
 * the method returns, regardless if an exception is thrown, or if the condition variable has been signaled,
 * or if an timeout has occurred.
 *
 * Mutex lockers can be used as function return values. @ref IThreadRegistry::Lock() provides an example.
 */
class MutexLocker final
{
    friend class AdvancedMutexLocker;

  public:
    MutexLocker(void) = delete;
    explicit MutexLocker(Mutex* const _pMutex);
    explicit MutexLocker(Mutex& mutex);
    MutexLocker(MutexLocker const &) = delete;
    MutexLocker(MutexLocker&& other) noexcept;
    ~MutexLocker(void);

    MutexLocker& operator=(MutexLocker const &) = delete;
    MutexLocker& operator=(MutexLocker&&) = delete;

  private:
    /// Pointer to the managed Mutex. nullptr = none (e.g. stolen by move constructor).
    Mutex* pMutex;
};

/**
 * \brief Constructor. Creates a @ref MutexLocker instance and locks the managed @ref Mutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _pMutex
 * Pointer to the @ref Mutex that shall be managed by this @ref MutexLocker. \n
 * The @ref Mutex is locked when the @ref MutexLocker is instantiated and the @ref Mutex is released
 * when the @ref MutexLocker is destroyed.\n
 * If this is nullptr, then the @ref MutexLocker will be passive.
 */
inline MutexLocker::MutexLocker(Mutex* const _pMutex)
: pMutex(_pMutex)
{
  if (pMutex != nullptr)
    pMutex->Lock();
}

/**
 * \brief Constructor. Creates a @ref MutexLocker instance and locks the managed @ref Mutex.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param mutex
 * Reference to the @ref Mutex instance that shall be managed by this @ref MutexLocker. \n
 * The @ref Mutex is locked when the @ref MutexLocker is instantiated and the @ref Mutex is released
 * when the @ref MutexLocker is destroyed.
 */
inline MutexLocker::MutexLocker(Mutex& mutex)
: pMutex(&mutex)
{
  pMutex->Lock();
}

/**
 * \brief Move constructor.
 *
 * The responsibility to unlock the @ref Mutex is moved from another @ref MutexLocker to the newly
 * constructed @ref MutexLocker.
 *
 * ---
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * The responsibility to release the @ref Mutex is moved from the referenced @ref MutexLocker `other` to the
 * new constructed @ref MutexLocker. After moving, the @ref MutexLocker referenced by `other` will behave passive.
 */
inline MutexLocker::MutexLocker(MutexLocker&& other) noexcept
: pMutex(other.pMutex)
{
  other.pMutex = nullptr;
}

inline MutexLocker::~MutexLocker(void)
/**
 * \brief Destructor. Unlocks the managed @ref Mutex.
 *
 * If the @ref MutexLocker is passive, then this does nothing.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the thread only that has created the @ref MutexLocker instance.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (pMutex != nullptr)
    pMutex->Unlock();
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef MUTEXLOCKER_HPP_201701271644
#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
