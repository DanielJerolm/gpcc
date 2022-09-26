/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
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
 * \fn void IObjectRegistration::Clear(void)
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
 * \fn void IObjectRegistration::Add(std::unique_ptr<gpcc::cood::Object> & spObj, uint16_t const index)
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
 * \fn void IObjectRegistration::Remove(uint16_t const index)
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
