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

#include "ThreadRegistry.hpp"
#include "../Thread.hpp"
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
