/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
