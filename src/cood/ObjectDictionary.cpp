/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018, 2022 Daniel Jerolm

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

#include "ObjectDictionary.hpp"
#include "Object.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/RWLockWriteLocker.hpp"
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \brief Constructor. Creates an empty object dictionary.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ObjectDictionary::ObjectDictionary(void)
: IObjectAccess()
, IObjectRegistration()
, lock()
, container()
{
}

/**
 * \brief Destructor.
 *
 * Any objects contained in the object dictionary will be released too.
 *
 * \pre   The object dictionary must not be locked, neither for object access, nor for
 *        adding/removing objects. Any violation of this precondition will result in panic.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
ObjectDictionary::~ObjectDictionary(void)
{
  if (!lock.TryWriteLock())
    gpcc::osal::Panic("ObjectDictionary::~ObjectDictionary: In use");

  for (auto const & e: container)
  {
    Object * const pObj = e.second;
    pObj->pOD = nullptr;
    delete pObj;
  }

  lock.ReleaseWriteLock();
}

// <-- Interface IObjectRegistration
/// \copydoc IObjectRegistration::Clear
void ObjectDictionary::Clear(void)
{
  gpcc::osal::RWLockWriteLocker write_lock(lock);

  try
  {
    for (auto const & e: container)
    {
      e.second->pOD = nullptr;
      delete e.second;
    }

    container.clear();
  }
  catch (...)
  {
    PANIC();
  }
}

/// \copydoc IObjectRegistration::Add
void ObjectDictionary::Add(std::unique_ptr<Object> & spObj, uint16_t const index)
{
  gpcc::osal::RWLockWriteLocker write_lock(lock);

  auto & e = container[index];

  if (e != nullptr)
    throw std::logic_error("ObjectDictionary::Add: Index is already in use");

  e = spObj.release();
  e->index = index;
  e->pOD = this;
}

/// \copydoc IObjectRegistration::Remove
void ObjectDictionary::Remove(uint16_t const index)
{
  gpcc::osal::RWLockWriteLocker write_lock(lock);

  // look-up index
  auto it = container.find(index);
  if (it == container.end())
    return;

  // fetch pointer to object
  auto pObj = it->second;

  // remove from object dictionary
  container.erase(it);

  // destroy object
  pObj->pOD = nullptr;
  delete pObj;
}
// --> Interface IObjectRegistration


// <-- Interface IObjectAccess
/// \copydoc IObjectAccess::LockForObjAccess
gpcc::osal::RWLockReadLocker ObjectDictionary::LockForObjAccess(void)
{
  return gpcc::osal::RWLockReadLocker(lock);
}

/// \copydoc IObjectAccess::GetNumberOfObjects
size_t ObjectDictionary::GetNumberOfObjects(void) const
{
  gpcc::osal::RWLockReadLocker read_lock(lock);
  return container.size();
}

/// \copydoc IObjectAccess::GetIndices
std::vector<uint16_t> ObjectDictionary::GetIndices(void) const
{
  std::vector<uint16_t> v;

  gpcc::osal::RWLockReadLocker read_lock(lock);

  v.reserve(container.size());
  for (auto const & e: container)
    v.push_back(e.first);

  return v;
}

/// \copydoc IObjectAccess::GetFirstObject
ObjectPtr ObjectDictionary::GetFirstObject(void)
{
  gpcc::osal::RWLockReadLocker read_lock(lock);

  auto it = container.begin();
  return ObjectPtr(it, (it == container.end()));
}

/// \copydoc IObjectAccess::GetObject
ObjectPtr ObjectDictionary::GetObject(uint16_t const index)
{
  gpcc::osal::RWLockReadLocker read_lock(lock);

  auto it = container.find(index);
  return ObjectPtr(it, (it == container.end()));
}

/// \copydoc IObjectAccess::GetNextNearestObject
ObjectPtr ObjectDictionary::GetNextNearestObject(uint16_t const index)
{
  gpcc::osal::RWLockReadLocker read_lock(lock);

  auto it = container.find(index);

  if (it == container.end())
  {
    uint16_t const index_minus_1 = (index == 0U) ? 0U : (index - 1U);
    it = container.upper_bound(index_minus_1);
  }

  return ObjectPtr(it, (it == container.end()));
}

// --> Interface IObjectAccess

/**
 * \brief Read-locks the object dictionary for object access, or increments an already existing read-lock.
 *
 * This will not block if a read-lock cannot be acquired. Instead an `std::logic_error` will be thrown.
 *
 * This is intended to be invoked by class @ref ObjectPtr only while the object dictionary is already
 * locked for object access.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void ObjectDictionary::IncReadLock(void)
{
  if (!lock.TryReadLock())
    throw std::logic_error("ObjectDictionary::IncReadLock: Cannot lock.");
}

/**
 * \brief Decrements the number of read-locks for object access and finally unlocks the object
 *        dictionary when the number of read-locks reaches zero.
 *
 * This is intended to be invoked by class @ref ObjectPtr only.
 *
 * \pre   There must be at least one read-lock.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void ObjectDictionary::DecReadLock(void) noexcept
{
  try
  {
    lock.ReleaseReadLock();
  }
  catch (...)
  {
    PANIC();
  }
}

} // namespace cood
} // namespace gpcc
