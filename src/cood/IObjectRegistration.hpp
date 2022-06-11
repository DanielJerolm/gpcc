/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018 Daniel Jerolm

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

#ifndef IOBJECTREGISTRATION_HPP_201810051942
#define IOBJECTREGISTRATION_HPP_201810051942

#include <memory>
#include <cstdint>

namespace gpcc {
namespace cood {

class Object;

/**
 * \ingroup GPCC_COOD
 * \brief Interface for registration and removal of objects at/from an CAN open object dictionary (class @ref ObjectDictionary).
 *
 * This interface allows to:
 * - add objects to the object dictionary
 * - remove selected objects from the object dictionary based on their index
 * - remove all objects from the object dictionary
 *
 * Please refer to chapter "Object Lifecycle" in the documentation of class @ref ObjectDictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * All methods offered by this interface will lock the object dictionary for modification.\n
 * See class @ref ObjectDictionary, chapter "Locking" for details.
 */
class IObjectRegistration
{
  public:
    virtual void Clear(void) = 0;
    virtual void Add(std::unique_ptr<Object> & spObj, uint16_t const index) = 0;
    virtual void Remove(uint16_t const index) = 0;

  protected:
    IObjectRegistration(void) = default;

    IObjectRegistration(IObjectRegistration const &) = default;
    IObjectRegistration(IObjectRegistration &&) = default;

    virtual ~IObjectRegistration(void) = default;

    IObjectRegistration& operator=(IObjectRegistration const &) = default;
    IObjectRegistration& operator=(IObjectRegistration &&) = default;
};

/**
 * \fn IObjectRegistration::Clear
 * \brief Removes all objects from the object dictionary and releases them.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object dictionary modification).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */

/**
 * \fn IObjectRegistration::Add
 * \brief Adds an object to the object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object dictionary modification).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param spObj
 * Reference to an `std::unique_ptr` pointing to the object that shall be added to the object dictionary.\n
 * Ownership will move to the object dictionary, if - __and only if__ - the call succeeds (no exception thrown).\n
 * In case of any error (exception), ownership will remain at the referenced `std::unique_ptr`.
 *
 * \param index
 * Desired index for the object.\n
 * An exception will be thrown, if there is already an object registered with this index.
 */

/**
 * \fn IObjectRegistration::Remove
 * \brief Removes an object from the object dictionary and releases it.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object dictionary modification).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param index
 * Index of the object that shall be removed from the object dictionary and released.\n
 * If there is no object registered with the given index, then this method will have no effect.
 */

} // namespace cood
} // namespace gpcc

#endif // IOBJECTREGISTRATION_HPP_201810051942
