/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include "ObjectDictionary.hpp"
#include "Object.hpp"
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/RWLockWriteLocker.hpp>
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
