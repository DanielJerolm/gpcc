/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))

#include <gpcc/osal/ThreadRegistry.hpp>
#include <gpcc/osal/Thread.hpp>
#include <cstring>

namespace gpcc {
namespace osal {

/**
 * \brief Constructor. Creates an empty thread registry.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 */
ThreadRegistry::ThreadRegistry(void)
: mutex()
, threadList()
{
}

/**
 * \brief Registers a thread at the thread registry.
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
 * \param thread
 * Unmodifiable reference to the @ref Thread instance that shall be registered.\n
 * The @ref Thread instance must not be registered twice.
 */
void ThreadRegistry::RegisterThread(Thread const & thread)
{
  MutexLocker ml(mutex);

  // List is sorted alphabetically. Insert at the correct position.
  auto const threadsName = thread.GetName();
  auto it = threadList.begin();
  while (true)
  {
    if (it == threadList.end())
    {
      // (end of list reached or list is empty)
      threadList.push_back(&thread);
      break;
    }
    if (threadsName < (*it)->GetName())
    {
      // (found the appropriate place to insert)
      threadList.insert(it, &thread);
      break;
    }
    ++it;
  }
}

/**
 * \brief Removes a thread from the thread registry.
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
 * \param thread
 * Unmodifiable reference to the @ref Thread instance that shall be removed.\n
 * If the referenced @ref Thread instance is not found in the registry, then this method does nothing.
 */
void ThreadRegistry::UnregisterThread(Thread const & thread)
{
  MutexLocker ml(mutex);

  for (auto it = threadList.begin(); it != threadList.end(); ++it)
  {
    if (&thread == *it)
    {
      threadList.erase(it);
      break;
    }
  }
}

} // namespace osal
} // namespace gpcc

#endif // #if (defined(OS_CHIBIOS_ARM) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC))
