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

#ifndef OBJECTPTR_HPP_201809301437
#define OBJECTPTR_HPP_201809301437

#include <map>
#include <stdexcept>
#include <cstdint>

namespace gpcc {
namespace cood {

class Object;

/**
 * \ingroup GPCC_COOD
 * \brief Smart pointer referencing to an CANopen object dictionary object and keeping a _lock for object access_
 *        at the object dictionary.
 *
 * CANopen object dictionary objects contained in a @ref ObjectDictionary are referenced via @ref ObjectPtr
 * instances only. The @ref IObjectAccess interface implemented by class @ref ObjectDictionary will only hand over
 * @ref ObjectPtr instances instead of raw-pointers.
 *
 * From the user's point of view, a @ref ObjectPtr instance behaves like a raw-pointer to class @ref Object.
 *
 * In addition to referencing an @ref Object instance, class @ref ObjectPtr also holds a
 * _lock for object access_ at the object dictionary containing the referenced object. The lock is released if...
 * - the @ref ObjectPtr is destroyed
 * - `nullptr` is assigned to the @ref ObjectPtr
 * - @ref Reset() is invoked on the @ref ObjectPtr
 * - an @ref Object from a different object dictionary instance is assigned to the @ref ObjectPtr
 *
 * Remember that multiple locks for object access can be acquired at an @ref ObjectDictionary at the same time.
 * Each @ref ObjectPtr referencing to an @ref Object will hold one lock. For details, please refer to
 * class @ref ObjectDictionary, chapter "Locking".
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ObjectPtr final
{
    friend class ObjectDictionary;

  public:
    ObjectPtr(void) noexcept;
    ObjectPtr(ObjectPtr const & other);
    ObjectPtr(ObjectPtr && other) noexcept;
    ~ObjectPtr(void);

    ObjectPtr& operator=(ObjectPtr const & rhv);
    ObjectPtr& operator=(ObjectPtr && rhv) noexcept;

    ObjectPtr& operator++();
    ObjectPtr operator++(int);

    Object& operator*() const;
    Object* operator->() const;

    bool operator!() const noexcept;
    explicit operator bool() const noexcept;

    bool operator==(ObjectPtr const & rhv) const noexcept;
    bool operator!=(ObjectPtr const & rhv) const noexcept;

    void Reset(void) noexcept;

  private:
    /// Iterator referencing to the object.
    /** This is only valid, if @ref pointsToNothing is false. */
    std::map<uint16_t, Object*>::iterator it;

    /// Flag indicating if this @ref ObjectPtr instance points to nothing (nullptr).
    bool pointsToNothing;


    explicit ObjectPtr(std::map<uint16_t, Object*>::iterator const & _it, bool const _pointsToNothing);
};

/**
 * \brief Dereferencing operator.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   Pointer points to nothing (nullptr).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Reference to the @ref Object referenced by this @ref ObjectPtr.
 */
inline Object& ObjectPtr::operator*() const
{
  if (pointsToNothing)
    throw std::runtime_error("ObjectPtr::operator*: Null pointer");

  return *(it->second);
}

/**
 * \brief Pointer member access operator.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   Pointer points to nothing (nullptr).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Pointer to the @ref Object referenced by this @ref ObjectPtr.
 */
inline Object* ObjectPtr::operator->() const
{
  if (pointsToNothing)
    throw std::runtime_error("ObjectPtr::operator->: Null pointer");

  return it->second;
}

/**
 * \brief Logical NOT operator. Returns true, if the pointer points to nothing (nullptr).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * `true`, if the @ref ObjectPtr points to nothing (nullptr), otherwise `false`.
 */
inline bool ObjectPtr::operator!() const noexcept
{
  return pointsToNothing;
}

/**
 * \brief Operator bool. Returns true, if the pointer points to an object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * `true`, if the @ref ObjectPtr points to an object, otherwise `false`.
 */
inline ObjectPtr::operator bool() const noexcept
{
  return !pointsToNothing;
}

/**
 * \brief Compare to equal operator.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * Other @ref ObjectPtr instance that shall be compared to this instance.
 *
 * \return
 * true  = Both @ref ObjectPtr instances point to the same @ref Object, or both are nullptr.\n
 * false = Both @ref ObjectPtr instances do not point to the same @ref Object.
 */
inline bool ObjectPtr::operator==(ObjectPtr const & rhv) const noexcept
{
  if (pointsToNothing)
    return rhv.pointsToNothing;

  if (rhv.pointsToNothing)
    return false;

  return (it->second == rhv.it->second);
}

/**
 * \brief Compare to unequal operator.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * Other @ref ObjectPtr instance that shall be compared to this instance.
 *
 * \return
 * true  = Both @ref ObjectPtr instances do not point to the same @ref Object. \n
 * false = Both @ref ObjectPtr instances point to the same @ref Object, or both are nullptr.
 */
inline bool ObjectPtr::operator!=(ObjectPtr const & rhv) const noexcept
{
  return !(operator==(rhv));
}

} // namespace cood
} // namespace gpcc

#endif // OBJECTPTR_HPP_201809301437
