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

#ifndef THREADREGISTRY_HPP_201701282133
#define THREADREGISTRY_HPP_201701282133

#include "IThreadRegistry.hpp"
#include "../Mutex.hpp"

namespace gpcc {
namespace osal {

class Thread;

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief Registry for @ref Thread instances.
 *
 * Each application including GPCC and using GPCC's threads will contain one instance of class
 * @ref ThreadRegistry.
 *
 * @ref gpcc::osal::Thread provides one global instance of class @ref ThreadRegistry per process.
 * The thread registry's public interface (@ref IThreadRegistry) can be retrieved by anybody
 * using the public static method @ref Thread::GetThreadRegistry().
 *
 * All instances of class @ref Thread will register and unregister themselves at the
 * global @ref ThreadRegistry instance upon creation and destruction.
 *
 * \sa @ref IThreadRegistry
 * \sa @ref Thread
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ThreadRegistry final: public IThreadRegistry
{
  public:
    ThreadRegistry(void);
    ThreadRegistry(ThreadRegistry const &) = delete;
    ThreadRegistry(ThreadRegistry &&) = delete;
    ~ThreadRegistry(void) = default;

    ThreadRegistry& operator=(ThreadRegistry const &) = delete;
    ThreadRegistry& operator=(ThreadRegistry &&) = delete;

    void RegisterThread(Thread const & thread);
    void UnregisterThread(Thread const & thread);

    // <-- IThreadRegistry
    MutexLocker Lock(void) override;
    size_t GetNbOfThreads(void) const override;
    iterator ThreadListBegin(void) const override;
    iterator ThreadListEnd(void) const override;
    // -->

  private:
    /// Mutex for making the API thread-safe.
    Mutex mutex;

    /// List of registered @ref Thread instances.
    /** @ref mutex is required. */
    std::list<Thread const *> threadList;
};

/// \copydoc IThreadRegistry::Lock()
inline MutexLocker ThreadRegistry::Lock(void)
{
  return MutexLocker(mutex);
}

/// \copydoc IThreadRegistry::GetNbOfThreads()
inline size_t ThreadRegistry::GetNbOfThreads(void) const
{
  return threadList.size();
}

/// \copydoc IThreadRegistry::ThreadListBegin()
inline ThreadRegistry::iterator ThreadRegistry::ThreadListBegin(void) const
{
  return threadList.begin();
}

/// \copydoc IThreadRegistry::ThreadListEnd()
inline ThreadRegistry::iterator ThreadRegistry::ThreadListEnd(void) const
{
  return threadList.end();
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef THREADREGISTRY_HPP_201701282133
#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
