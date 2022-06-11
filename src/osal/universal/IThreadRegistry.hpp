/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#ifndef ITHREADREGISTRY_HPP_201701271713
#define ITHREADREGISTRY_HPP_201701271713

#include "../MutexLocker.hpp"
#include <list>
#include <cstddef>

namespace gpcc {
namespace osal {

class Thread;

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief Interface for gathering information about threads from GPCC's @ref ThreadRegistry.
 *
 * Each application including GPCC will contain one instance of class @ref ThreadRegistry.
 * It can be retrieved via @ref gpcc::osal::Thread::GetThreadRegistry(void). All instances of class
 * @ref Thread will register and unregister themselves at the @ref ThreadRegistry upon creation
 * and destruction.
 *
 * This interface is intended to iterate over @ref Thread instances registered at GPCC's @ref ThreadRegistry
 * in order to e.g. dump information about the threads to a console. Each @ref Thread instance allows
 * to retrieve detailed information about the managed thread by invoking @ref Thread::GetInfo().
 *
 * __Before__ gathering information from this interface, @ref Lock() must be invoked to lock the
 * thread registry. @ref Lock() will return a @ref MutexLocker instance which is used to lock
 * and unlock an internal mutex of the thread registry.
 *
 * __After__ all required information has been gathered from the thread registry, the @ref MutexLocker
 * instance returned by @ref Lock() must be discarded in order to unlock the thread registry. The
 * documentation of @ref Lock() provides examples.
 *
 * _You should minimize the time the thread registry is locked, because creation and destruction_
 * _of @ref Thread instances is blocked while the thread registry is locked._
 *
 * Any @ref iterator acquired via @ref ThreadListBegin() or @ref ThreadListEnd() becomes invalid
 * after the thread registry has been unlocked.
 *
 * Example:
 * ~~~{.cpp}
 * {
 *   gpcc::MutexLocker locker(pThreadRegistry->Lock());
 *
 *   size_t const n = pThreadRegistry->GetNbOfThreads();
 *   for (auto it = pThreadRegistry->ThreadListBegin(); it != pThreadRegistry->ThreadListEnd(); ++it)
 *   {
 *     // ...
 *   }
 * } // "locker" is discarded and thread registry is unlocked when leaving this scope
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IThreadRegistry
{
  public:
    /// Iterator used to iterate over @ref Thread instances registered in the thread registry.
    /** Refer to @ref ThreadListBegin() and @ref ThreadListEnd() for details. */
    typedef std::list<Thread const *>::const_iterator iterator;

    virtual MutexLocker Lock(void) = 0;
    virtual size_t GetNbOfThreads(void) const = 0;
    virtual iterator ThreadListBegin(void) const = 0;
    virtual iterator ThreadListEnd(void) const = 0;

  protected:
    IThreadRegistry(void) = default;
    IThreadRegistry(IThreadRegistry const &) = delete;
    IThreadRegistry(IThreadRegistry&&) = delete;
    virtual ~IThreadRegistry(void) = default;

    IThreadRegistry& operator=(IThreadRegistry const &) = delete;
    IThreadRegistry& operator=(IThreadRegistry&&) = delete;
};

/**
 * \fn IThreadRegistry::Lock(void)
 * \brief Locks the thread registry's internal mutex using a @ref MutexLocker. This blocks until the lock is acquired.
 *
 * The thread registry's internal mutex must be locked when invoking any of the public methods
 * offered by this interface:
 * - GetNbOfThreads(void)
 * - ThreadListBegin(void)
 * - ThreadListEnd(void)
 *
 * After all information has been gathered from the methods listed above, the @ref MutexLocker
 * must be released in order to unlock the mutex. When the @ref MutexLocker is instantiated on
 * the stack, then this automatically happens when the scope in which the @ref MutexLocker is
 * instantiated is left:\n
 * ~~~{.cpp}
 * {
 *   gpcc::MutexLocker locker(pThreadRegistry->Lock);
 *
 *   // Thread registry's internal mutex is locked. It is safe to invoke GetNbOfThreads(),
 *   // ThreadListBegin(), and ThreadListEnd() now.
 *
 * } // "locker" is discarded and the thread registry is unlocked when leaving this scope
 * ~~~
 *
 * Alternatively, you could also convert the @ref MutexLocker into an @ref AdvancedMutexLocker
 * that could be unlocked manually before the scope is left:\n
 * ~~~{.cpp}
 * gpcc::AdvancedMutexLocker locker(pThreadRegistry->Lock);
 *
 * // Thread registry's internal mutex is locked. It is safe to invoke GetNbOfThreads(),
 * // ThreadListBegin(), and ThreadListEnd() now.
 *
 * // we unlock manually here because we do not need the lock any more and the
 * // current scope is not yet left
 * locker.Unlock();
 * ~~~
 *
 * _You should minimize the time the thread registry is locked, because creation and destruction_
 * _of @ref Thread instances is blocked while the thread registry is locked._
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
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * A @ref MutexLocker instance used to lock the thread registry's internal mutex.\n
 * The @ref MutexLocker must be released when the thread registry shall be unlocked.\n
 * For details, please refer to the detailed documentation of @ref IThreadRegistry.
 */

/**
 * \fn IThreadRegistry::GetNbOfThreads(void) const
 * \brief Retrieves the number of registered @ref Thread instances.
 *
 * __Thread safety:__\n
 * The thread registry must be locked using @ref Lock().
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Number of @ref Thread instances currently registered at the thread registry.
 */

/**
 * \fn IThreadRegistry::ThreadListBegin(void) const
 * \brief Returns an iterator referencing to the first registered @ref Thread instance.
 *
 * __Thread safety:__\n
 * The thread registry must be locked using @ref Lock().
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * An iterator referencing to the first registered @ref Thread instance.\n
 * _The retrieved iterator is valid until the thread registry is unlocked._\n
 * Please also have a look at @ref ThreadListEnd().
 */

/**
 * \fn IThreadRegistry::ThreadListEnd(void) const
 * \brief Returns an iterator referencing beyond the end of the list of registered @ref Thread instances.
 *
 * __Thread safety:__\n
 * The thread registry must be locked using @ref Lock().
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * An iterator referencing beyond the end of the list of registered @ref Thread instances.\n
 * _The retrieved iterator is valid until the thread registry is unlocked._\n
 * In conjunction with @ref ThreadListBegin() one can iterate over the registered threads:
 * ~~~{.cpp}
 * {
 *   gpcc::MutexLocker locker(pThreadRegistry->Lock());
 *
 *   size_t const n = pThreadRegistry->GetNbOfThreads();
 *   for (auto it = pThreadRegistry->ThreadListBegin(); it != pThreadRegistry->ThreadListEnd(); ++it)
 *   {
 *     // ...
 *   }
 * } // "locker" is discarded and thread registry is unlocked when leaving this scope
 * ~~~
 */

} // namespace osal
} // namespace gpcc

#endif // #ifndef ITHREADREGISTRY_HPP_201701271713
#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
