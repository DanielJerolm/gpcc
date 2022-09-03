/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/resource_management/objects/SmallDynamicNamedRWLock.hpp>
#include "internal/NamedRWLockEntry.hpp"
#include <gpcc/osal/Panic.hpp>
#include <stdexcept>

namespace gpcc
{
namespace resource_management
{
namespace objects
{

using namespace internal;

SmallDynamicNamedRWLock::~SmallDynamicNamedRWLock(void)
/**
 * \brief Destructor. There must be no locks left when the @ref SmallDynamicNamedRWLock instance is released.
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
  if (pList != nullptr)
    PANIC();
}

bool SmallDynamicNamedRWLock::TestWriteLock(std::string const & resourceName) const noexcept
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
  return !IsLocked(resourceName);
}
bool SmallDynamicNamedRWLock::GetWriteLock(std::string const & resourceName)
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
  if (FindEntry(resourceName, nullptr) != nullptr)
    return false;

  CreateEntry(resourceName, true);
  return true;
}
void SmallDynamicNamedRWLock::ReleaseWriteLock(std::string const & resourceName)
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
  NamedRWLockEntry* pPrevEntry;
  NamedRWLockEntry* const pEntry = FindEntry(resourceName, &pPrevEntry);

  if (pEntry == nullptr)
    throw std::logic_error("SmallDynamicNamedRWLock::ReleaseWriteLock: No such resource");

  pEntry->ReleaseWriteLock();
  ReleaseEntry(pEntry, pPrevEntry);
}

bool SmallDynamicNamedRWLock::TestReadLock(std::string const & resourceName) const noexcept
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
  NamedRWLockEntry const * const pEntry = FindEntry(resourceName, nullptr);

  // no locks yet?
  if (pEntry == nullptr)
    return true;

  // There is a lock. Is it a read lock?
  return (!pEntry->IsWriteLocked());
}
bool SmallDynamicNamedRWLock::GetReadLock(std::string const & resourceName)
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
  NamedRWLockEntry* const pEntry = FindEntry(resourceName, nullptr);

  if (pEntry != nullptr)
    return pEntry->GetReadLock();

  // no locks yet
  CreateEntry(resourceName, false);
  return true;
}
void SmallDynamicNamedRWLock::ReleaseReadLock(std::string const & resourceName)
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
  NamedRWLockEntry* pPrevEntry;
  NamedRWLockEntry* const pEntry = FindEntry(resourceName, &pPrevEntry);

  if (pEntry == nullptr)
    throw std::logic_error("SmallDynamicNamedRWLock::ReleaseReadLock: No such resource");

  pEntry->ReleaseReadLock();

  // no more locks?
  if (pEntry->GetNbOfReadLocks() == 0)
    ReleaseEntry(pEntry, pPrevEntry);
}

bool SmallDynamicNamedRWLock::IsLocked(std::string const & resourceName) const noexcept
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
  return (FindEntry(resourceName, nullptr) != nullptr);
}
bool SmallDynamicNamedRWLock::AnyLocks(void) const noexcept
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
  return (pList != nullptr);
}

NamedRWLockEntry* SmallDynamicNamedRWLock::FindEntry(std::string const & resourceName, NamedRWLockEntry** const ppPrev) const noexcept
/**
 * \brief Searches the single linked list @ref pList for a @ref internal::NamedRWLockEntry associated with a specific resource.
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
 * Name of the resource for which the associated @ref internal::NamedRWLockEntry shall be located.
 * \param ppPrev
 * In case of a match, a pointer to the @ref internal::NamedRWLockEntry instance that is located in front of the
 * matching entry in the single linked list @ref pList is written into the referenced pointer.\n
 * If the matching entry is the first one in the single linked list, then nullptr is written into
 * the referenced pointer.\n
 * The retrieved information is required, if the located @ref internal::NamedRWLockEntry instance shall be removed
 * via @ref ReleaseEntry().
 * This parameter may be nullptr, if the caller is not interested in this information.
 * \return
 * Pointer to the @ref internal::NamedRWLockEntry that is associated with the resource given by parameter `resourceName`.\n
 * If no entry is found, then nullptr is returned.
 */
{
  NamedRWLockEntry* pPrev = nullptr;
  NamedRWLockEntry* pEntry = pList;

  while ((pEntry != nullptr) && (pEntry->name != resourceName))
  {
    pPrev  = pEntry;
    pEntry = pEntry->pNext;
  }

  // found a resource lock entry and calling method wants to know who is sitting in front of it?
  if ((pEntry != nullptr) && (ppPrev != nullptr))
    *ppPrev = pPrev;

  // finished
  return pEntry;
}
void SmallDynamicNamedRWLock::ReleaseEntry(NamedRWLockEntry* const pEntry, NamedRWLockEntry* const pPrev) noexcept
/**
 * \brief Removes one @ref internal::NamedRWLockEntry from the single linked list @ref pList and releases the entry.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations may only fail due to serious errors that will result in program termination via Panic(...).
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pEntry
 * Pointer to the @ref internal::NamedRWLockEntry that shall be removed and released.
 * \param pPrev
 * Pointer to the @ref internal::NamedRWLockEntry that is located in front of `pEntry` in the single linked list
 * @ref pList. \n
 * If `pEntry` is the first entry in the list, then this must be nullptr.\n
 * This can be retrieved via the 2nd parameter of @ref FindEntry().
 */
{
  // first entry in pList?
  if (pPrev == nullptr)
  {
    if (pEntry != pList)
      PANIC();

    pList = pEntry->pNext;
    delete pEntry;
  }
  else
  {
    pPrev->pNext = pEntry->pNext;
    delete pEntry;
  }
}
void SmallDynamicNamedRWLock::CreateEntry(std::string const & resourceName, bool const writeLockNotReadLock)
/**
 * \brief Creates a new @ref internal::NamedRWLockEntry instance and enqueues it in the list @ref pList.
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
 * Name of the resource that shall be associated with the new entry.
 * \param writeLockNotReadLock
 * true  = New resource shall be locked by one writer\n
 * false = New resource shall be locked by a reader
 */
{
  pList = new NamedRWLockEntry(pList, resourceName, writeLockNotReadLock);
}

} // namespace objects
} // namespace resource_management
} // namespace gpcc
