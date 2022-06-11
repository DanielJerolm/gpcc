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

#include "ObjectPtr.hpp"
#include "Object.hpp"
#include "ObjectDictionary.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"

namespace gpcc {
namespace cood {

/**
 * \brief Constructor. Creates a @ref ObjectPtr instance pointing to nothing (nullptr).
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ObjectPtr::ObjectPtr(void) noexcept
: it()
, pointsToNothing(true)
{
}

/**
 * \brief Copy constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param other
 * The new @ref ObjectPtr instance will be a copy of the referenced @ref ObjectPtr instance.
 */
ObjectPtr::ObjectPtr(ObjectPtr const & other)
: it(other.it)
, pointsToNothing(other.pointsToNothing)
{
  if (!pointsToNothing)
    it->second->pOD->IncReadLock();
}

/**
 * \brief Move constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * The new @ref ObjectPtr instance will be move-constructed from the referenced @ref ObjectPtr instance.\n
 * The referenced @ref ObjectPtr instance will point to nothing (nullptr) after the move operation.
 */
ObjectPtr::ObjectPtr(ObjectPtr && other) noexcept
: it(std::move(other.it))
, pointsToNothing(other.pointsToNothing)
{
  other.pointsToNothing = true;
}

/**
 * \brief Destructor.
 *
 * \post   If this was the last @ref ObjectPtr instance pointing to _any_ object from an @ref ObjectDictionary,
 *         then the @ref ObjectDictionary (currently locked for object access) will be unlocked.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ObjectPtr::~ObjectPtr(void)
{
  if (!pointsToNothing)
    it->second->pOD->DecReadLock();
}

/**
 * \brief Copy assignment operator.
 *
 * \post   If this was the last @ref ObjectPtr instance pointing to _any_ object from an @ref ObjectDictionary,
 *         then the @ref ObjectDictionary (currently locked for object access) will be unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rhv
 * Other @ref ObjectPtr instance whose content shall be copy-assigned to this @ref ObjectPtr.
 *
 * \return
 * Reference to self.
 */
ObjectPtr& ObjectPtr::operator=(ObjectPtr const & rhv)
{
  if (this != &rhv)
  {
    if (!rhv.pointsToNothing)
    {
      if (pointsToNothing)
      {
        it = rhv.it;
        it->second->pOD->IncReadLock();
        pointsToNothing = false;
      }
      else
      {
        // same object dictionary instance?
        if (it->second->pOD == rhv.it->second->pOD)
        {
          // same OD instances -> This ObjectPtr already holds a read-lock at the OD
          it = rhv.it;
        }
        else
        {
          // different OD instances -> This ObjectPtr must release its read-lock and acquire one at the other OD
          rhv.it->second->pOD->IncReadLock();
          ON_SCOPE_EXIT() { rhv.it->second->pOD->DecReadLock(); };

          auto pPrevOD = it->second->pOD;
          it = rhv.it;
          pPrevOD->DecReadLock();
          ON_SCOPE_EXIT_DISMISS();
        }
      }
    }
    else
    {
      if (!pointsToNothing)
      {
        it->second->pOD->DecReadLock();
        pointsToNothing = true;
      }
    }
  }

  return *this;
}

/**
 * \brief Move-assignment operator.
 *
 * \post   If this was the last @ref ObjectPtr instance pointing to _any_ object from an @ref ObjectDictionary,
 *         then the @ref ObjectDictionary (currently locked for object access) will be unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Other @ref ObjectPtr instance whose content shall be move-assigned to this @ref ObjectPtr instance.\n
 * The referenced @ref ObjectPtr instance will point to nothing (nullptr) after the move operation.
 *
 * \return
 * Reference to self.
 */
ObjectPtr& ObjectPtr::operator=(ObjectPtr && rhv) noexcept
{
  if (this != &rhv)
  {
    if (!rhv.pointsToNothing)
    {
      if (!pointsToNothing)
        it->second->pOD->DecReadLock();

      it = std::move(rhv.it);
      pointsToNothing = false;
      rhv.pointsToNothing = true;
    }
    else
    {
      if (!pointsToNothing)
      {
        it->second->pOD->DecReadLock();
        pointsToNothing = true;
      }
    }
  }

  return *this;
}

/**
 * \brief Increments the pointer (pre-increment).
 *
 * This can be used to iterate over the content of an @ref ObjectDictionary.
 *
 * \pre   The pointer must point to an object dictionary object (no nullptr).
 *
 * \post  The pointer will either point to the next object contained in the object dictionary or to nothing,
 *        if it pointed to the last object before incrementing.
 *
 * \post  If the pointer points to nothing after the increment, and if the pointer was the last @ref ObjectPtr
 *        instance pointing to _any_ object from an @ref ObjectDictionary, then the @ref ObjectDictionary
 *        (currently locked for object access) will be unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Reference to self.
 */
ObjectPtr& ObjectPtr::operator++()
{
  if (pointsToNothing)
    throw std::logic_error("ObjectPtr::operator++: Cannot increment pointer to nothing");

  auto pOD = it->second->pOD;

  ++it;
  if (it == pOD->container.end())
  {
    pOD->DecReadLock();
    pointsToNothing = true;
  }

  return *this;
}

/**
 * \brief Increments the pointer (post-increment).
 *
 * __For performance reasons, the pre-increment (e.g. ++ptr) should be preferred.__
 *
 * This can be used to iterate over the content of an @ref ObjectDictionary.
 *
 * \pre   The pointer must point to an object dictionary object (no nullptr).
 *
 * \post  The pointer will either point to the next object contained in the object dictionary or to nothing,
 *        if it pointed to the last object before incrementing.
 *
 * \post  If the pointer points to nothing after the increment, and if the pointer was the last @ref ObjectPtr
 *        instance pointing to _any_ object from an @ref ObjectDictionary, then the @ref ObjectDictionary
 *        (currently locked for object access) will be unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * A copy of this @ref ObjectPtr object created _before_ the increment takes place.
 */
ObjectPtr ObjectPtr::operator++(int)
{
  if (pointsToNothing)
    throw std::logic_error("ObjectPtr::operator++: Cannot increment pointer to nothing");

  ObjectPtr original(*this);
  ++(*this);
  return original;
}

/**
 * \brief Makes the pointer referencing to nothing (nullptr).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void ObjectPtr::Reset(void) noexcept
{
  if (!pointsToNothing)
  {
    it->second->pOD->DecReadLock();
    pointsToNothing = true;
  }
}

/**
 * \brief Constructor. Creates a @ref ObjectPtr pointing to an @ref Object.
 *
 * This is a private constructor intended to be invoked by friend class @ref ObjectDictionary only.
 *
 * \pre    The object dictionary must be locked for object access.
 *
 * \post   The number of locks for object access at the object dictionary has been incremented.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The object dictionary must be locked for object access.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _it
 * Iterator to the object that shall be referenced by the new @ref ObjectPtr instance.\n
 * The referenced object must be registered at an @ref ObjectDictionary.
 *
 * \param _pointsToNothing
 * Flag indicating if `_it` is valid or not:\n
 * true = `_it` is _invalid_\n
 * false = `_it` is _valid_
 */
ObjectPtr::ObjectPtr(std::map<uint16_t, Object*>::iterator const & _it, bool const _pointsToNothing)
: it(_it)
, pointsToNothing(_pointsToNothing)
{
  if (!pointsToNothing)
  {
    if (it->second->pOD == nullptr)
      throw std::invalid_argument("ObjectPtr::ObjectPtr: Referenced object's pOD is nullptr");

    it->second->pOD->IncReadLock();
  }
}

} // namespace cood
} // namespace gpcc
