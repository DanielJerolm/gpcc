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


#include "LargeDynamicNamedRWLock.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <limits>
#include <stdexcept>

namespace gpcc
{
namespace ResourceManagement
{
namespace Objects
{

LargeDynamicNamedRWLock::~LargeDynamicNamedRWLock(void)
/**
 * \brief Destructor. There must be no locks left when the @ref LargeDynamicNamedRWLock instance is released.
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object after invocation of destructor.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations may only fail due to serious errors that will result in program termination via Panic(...).
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (!locks.empty())
    PANIC();
}

bool LargeDynamicNamedRWLock::TestWriteLock(std::string const & resourceName) const noexcept
/**
 * \brief Checks if a write-lock could be acquired for a specific resource.
 *
 * __Note:__\n
 * This method does _not_ acquire any lock!\n
 * It only checks if a write-lock could be acquired via @ref GetWriteLock().
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource.
 * \return
 * true  = Write-lock could be acquired\n
 * false = Cannot acquire write-lock
 */
{
  return (locks.find(resourceName) == locks.end());
}
bool LargeDynamicNamedRWLock::GetWriteLock(std::string const & resourceName)
/**
 * \brief Tries to acquire a write-lock for a resource.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource for which a write-lock shall be acquired.
 * \return
 * true  = Write lock acquired\n
 * false = Write lock not acquired, resource is already locked by a writer or reader
 */
{
  // any locks yet?
  if (locks.find(resourceName) != locks.end())
    return false;

  locks[resourceName] = -1;
  return true;
}
void LargeDynamicNamedRWLock::ReleaseWriteLock(std::string const & resourceName)
/**
 * \brief Releases a write-lock.
 *
 * _This is to be invoked by writers only, who have successfully acquired a write-lock before._
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource for which a write-lock shall be released.\n
 * An exception is thrown if there is no write-lock registered for the given resource.
 */
{
  auto it = locks.find(resourceName);
  if (it == locks.end())
    throw std::logic_error("LargeDynamicNamedRWLock::ReleaseWriteLock: No such resource");

  if (it->second != -1)
    throw std::logic_error("LargeDynamicNamedRWLock::ReleaseWriteLock: No write-lock");

  locks.erase(it);
}

bool LargeDynamicNamedRWLock::TestReadLock(std::string const & resourceName) const noexcept
/**
 * \brief Checks if a read-lock could be acquired for a specific resource.
 *
 * __Note:__\n
 * This method does _not_ acquire any lock!\n
 * It only checks if a read-lock could be acquired via @ref GetReadLock().
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource.
 * \return
 * true  = Read-lock could be acquired\n
 * false = Cannot acquire read-lock
 */
{
  auto it = locks.find(resourceName);

  // no locks yet?
  if (it == locks.end())
    return true;

  // There is a lock. Is it a read lock?
  return (it->second > 0);
}
bool LargeDynamicNamedRWLock::GetReadLock(std::string const & resourceName)
/**
 * \brief Tries to acquire a read-lock for a resource.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource for which a read-lock shall be acquired.
 * \return
 * true  = Read-lock acquired\n
 * false = Read-lock not acquired, resource is already locked by a writer
 */
{
  auto it = locks.find(resourceName);

  if (it != locks.end())
  {
    // alread write-locked?
    if (it->second == -1)
      return false;

    // maximum number of read-locks reached?
    if (it->second == std::numeric_limits<int>::max())
      throw std::logic_error("LargeDynamicNamedRWLock::GetReadLock: No more read-locks possible");

    it->second++;
    return true;
  }
  else
  {
    locks[resourceName] = 1;
    return true;
  }
}
void LargeDynamicNamedRWLock::ReleaseReadLock(std::string const & resourceName)
/**
 * \brief Releases a read-lock.
 *
 * _This is to be invoked by readers only, who have successfully acquired a read-lock before._
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource for which a read-lock shall be released.\n
 * An exception is thrown if there is no read-lock registered for the given resource.
 */
{
  auto it = locks.find(resourceName);

  if (it == locks.end())
    throw std::logic_error("LargeDynamicNamedRWLock::ReleaseReadLock: No such resource");

  if (it->second == -1)
    throw std::logic_error("LargeDynamicNamedRWLock::ReleaseReadLock: Not locked by reader");

  if (it->second == 1)
    locks.erase(it);
  else
    it->second--;
}

bool LargeDynamicNamedRWLock::IsLocked(std::string const & resourceName) const noexcept
/**
 * \brief Determines whether there is any lock (read/write) on a specific resource.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param resourceName
 * Name of the resource.
 * \return
 * true  = There is a read- or write-lock on the resource\n
 * false = No lock on the resource
 */
{
  return (locks.find(resourceName) != locks.end());
}
bool LargeDynamicNamedRWLock::AnyLocks(void) const noexcept
/**
 * \brief Determines whether any resources are locked or not.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * true  = at least one resource is locked by a reader or writer\n
 * false = no locks
 */
{
  return (!locks.empty());
}

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc
