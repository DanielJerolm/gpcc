/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "NamedRWLockEntry.hpp"
#include <gpcc/osal/Panic.hpp>
#include <limits>
#include <stdexcept>

namespace gpcc
{
namespace resource_management
{
namespace objects
{
namespace internal
{

NamedRWLockEntry::NamedRWLockEntry(NamedRWLockEntry* const _pNext, std::string const & _name)
: pNext(_pNext)
, name(_name)
, locks(0)
/**
 * \brief Constructor. Creates an @ref NamedRWLockEntry instance with the associated resource being not locked.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
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
 * \param _pNext
 * Initial value for @ref pNext.
 * \param _name
 * Name of the resource associated with this @ref NamedRWLockEntry instance.\n
 * This is the initial value for @ref name.
 */
{
}
NamedRWLockEntry::NamedRWLockEntry(NamedRWLockEntry* const _pNext, std::string const & _name, bool const writeLockNotReadLock)
: pNext(_pNext)
, name(_name)
, locks(writeLockNotReadLock?-1:1)
/**
 * \brief Constructor. Creates an @ref NamedRWLockEntry instance with the associated resource being locked.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
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
 * \param _pNext
 * Initial value for @ref pNext.
 * \param _name
 * Name of the resource associated with this @ref NamedRWLockEntry instance.\n
 * This is the initial value for @ref name.
 * \param writeLockNotReadLock
 * Flag indicating the desired type of lock:\n
 * true  = Resource shall be locked by one writer\n
 * false = Resource shall be locked by one reader\n
 * After object creation, more readers may lock via @ref GetReadLock().
 */
{
}
NamedRWLockEntry::~NamedRWLockEntry(void)
/**
 * \brief Destructor. The associated resource must be unlocked.
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
  if (locks != 0)
    PANIC();
}

bool NamedRWLockEntry::GetReadLock(void)
/**
 * \brief Tries to acquire a read-lock.
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
 * \return
 * true  = read-lock acquired\n
 * false = read-lock not acquired
 */
{
  // locked by writer?
  if (locks == -1)
    return false;

  // too many read locks?
  if (locks == std::numeric_limits<int>::max())
    throw std::logic_error("NamedRWLockEntry::GetReadLock: No more read-locks possible");

  // acquire read-lock
  locks++;
  return true;
}
void NamedRWLockEntry::ReleaseReadLock(void)
/**
 * \brief Releases one read-lock.
 *
 * _This is to be invoked by readers only, who have successfully acquired a read-lock before._
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
 */
{
  // not locked by reader?
  if (locks < 1)
    throw std::logic_error("NamedRWLockEntry::ReleaseReadLock: Not locked by reader");

  locks--;
}
int NamedRWLockEntry::GetNbOfReadLocks(void) const noexcept
/**
 * \brief Retrieves the number of readers who have locked the resource.
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
 * Number of readers who have locked.\n
 * If zero is returned, then no reader has locked, but still a writer could have locked.
 */
{
  if (locks == -1)
    return 0;
  else
    return locks;
}

bool NamedRWLockEntry::GetWriteLock(void) noexcept
/**
 * \brief Tries to acquire a write-lock.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
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
 * true  = write-lock acquired\n
 * false = write-lock not acquired
 */
{
  // locked by anybody?
  if (locks != 0)
    return false;

  // acquire write-lock
  locks = -1;
  return true;
}
void NamedRWLockEntry::ReleaseWriteLock(void)
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
 */
{
  // not locked by writer?
  if (locks != -1)
    throw std::logic_error("NamedRWLockEntry::ReleaseWriteLock: Not locked by writer");

  locks = 0;
}
bool NamedRWLockEntry::IsWriteLocked(void) const noexcept
/**
 * \brief Retrieves if any writer has locked the resource.
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
 * true  = Locked by a writer\n
 * false = Not locked by any writer (but maybe by at least one reader)
 */
{
  return (locks == -1);
}

bool NamedRWLockEntry::IsLocked(void) const noexcept
/**
 * \brief Retrieves if the resource is locked.
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
 * true  = Locked\n
 * false = Not locked
 */
{
  return (locks != 0);
}

} // namespace internal
} // namespace objects
} // namespace resource_management
} // namespace gpcc
