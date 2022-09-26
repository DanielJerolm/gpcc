/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#ifndef IOBJECTACCESS_HPP_201810042131
#define IOBJECTACCESS_HPP_201810042131

#include <gpcc/cood/ObjectPtr.hpp>
#include <gpcc/osal/RWLockReadLocker.hpp>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD
 * \brief Interface offering access to the objects contained in an CANopen object dictionary (class @ref ObjectDictionary).
 *
 * This interface allows to:
 * - determine the number of objects contained in the object dictionary
 * - retrieve a list of the indices of the objects contained in the object dictionary
 * - access an object contained in the object dictionary using an index
 * - access an object or a subsequent object contained in the object dictionary using an index
 * - access the first object contained in the object dictionary (the one with the smallest index value) and use the
 *   retrieved @ref ObjectPtr instance to iterate over all objects contained in the object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * All methods offered by this interface will lock the object dictionary for object access.\n
 * See class @ref ObjectDictionary, chapter "Locking" for details.\n
 *
 * Note:
 * - Multiple threads are allowed to acquire and hold locks for object access simultaneously.
 * - A thread is allowed to hold multiple locks for object access.
 * - If consistency among multiple calls to the methods offered by this interface is required, then
 *   @ref LockForObjAccess() allows to acquire an additional read-lock manually. This prevents other threads from
 *   modifying the object dictionary (e.g. via @ref IObjectRegistration) between calls to this interface.
 * - @ref ObjectPtr objects retrieved from this interface hold a lock for object access while they refer
 *   to an @ref Object. \n
 *   Note that any call to @ref IObjectRegistration will be blocked until all locks for object access have been
 *   released.
 */
class IObjectAccess
{
  public:
    virtual gpcc::osal::RWLockReadLocker LockForObjAccess(void) = 0;

    virtual size_t GetNumberOfObjects(void) const = 0;
    virtual std::vector<uint16_t> GetIndices(void) const = 0;
    virtual ObjectPtr GetFirstObject(void) = 0;
    virtual ObjectPtr GetObject(uint16_t const index) = 0;
    virtual ObjectPtr GetNextNearestObject(uint16_t const index) = 0;

  protected:
    IObjectAccess(void) = default;

    IObjectAccess(IObjectAccess const &) = default;
    IObjectAccess(IObjectAccess &&) = default;

    virtual ~IObjectAccess(void) = default;

    IObjectAccess& operator=(IObjectAccess const &) = default;
    IObjectAccess& operator=(IObjectAccess &&) = default;
};

/**
 * \fn gpcc::osal::RWLockReadLocker IObjectAccess::LockForObjAccess(void)
 * \brief Locks the object dictionary for object access.
 *
 * While locked for object access, objects cannot be added to or removed from the object dictionary.
 *
 * A lock for object access is not mandatory for invocation of any method offered by the
 * [IObjectAccess](@ref gpcc::cood::IObjectAccess) interface, because all methods internally acquire a lock for object
 * access before executing and release it before returning.
 *
 * __But__ if consistency _among multiple calls_ to the methods offered by the [IObjectAccess](@ref gpcc::cood::IObjectAccess)
 * interface is required, then an additional lock for object access should be acquired via this method. The additional
 * lock will not be released between calls to the methods offered by the [IObjectAccess](@ref gpcc::cood::IObjectAccess)
 * interface and thus the lock will prevent other threads from modifying the object dictionary (e.g. add/remove objects)
 * between calls to the methods offered by the [IObjectAccess](@ref gpcc::cood::IObjectAccess) interface.
 *
 * Note:
 * - Multiple threads are allowed to acquire and hold locks for object access simultaneously.
 * - A thread is allowed to hold multiple locks for object access.
 * - @ref ObjectPtr objects retrieved from this interface hold a lock for object access while they refer
 *   to an @ref Object. \n
 *   Note that any call to @ref IObjectRegistration will be blocked until all locks for object access have been
 *   released.
 *
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
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
 *
 * - - -
 *
 * \return
 * @ref gpcc::osal::RWLockReadLocker object managing the acquired lock for object access.
 */

/**
 * \fn size_t IObjectAccess::GetNumberOfObjects(void) const
 * \brief Retrieves the number of objects currently contained in the object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object access).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.\n
 * Consider acquisition of an extra lock for object access. See [LockForObjAccess()](@ref gpcc::cood::IObjectAccess::LockForObjAccess)
 * for details.
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
 * Number of objects currently contained in the object dictionary.
 */

/**
 * \fn std::vector<uint16_t> IObjectAccess::GetIndices(void) const
 * \brief Retrieves a list of the indices of all objects currently contained in the object dictionary.
 *
 * As an alternative to this method, [GetFirstObject()](@ref gpcc::cood::IObjectAccess::GetFirstObject) could be
 * used to retrieve an [ObjectPtr](@ref gpcc::cood::ObjectPtr) pointing to the first object and then use the ObjectPtr
 * to iterate over all objects contained in the object dictionary. See
 * [GetFirstObject()](@ref gpcc::cood::IObjectAccess::GetFirstObject) for an example.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object access).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.\n
 * Consider acquisition of an extra lock for object access. See [LockForObjAccess()](@ref gpcc::cood::IObjectAccess::LockForObjAccess)
 * for details.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * `std::vector` containing the indices of all objects currently contained in the object dictionary.
 */

/**
 * \fn gpcc::cood::ObjectPtr IObjectAccess::GetFirstObject(void)
 * \brief Returns a pointer to the first object in the object dictionary (the one with the smallest index value).
 *
 * This is intended to be used to iterate over all objects in an object dictionary.\n
 * The following example iterates over all objects in an object dictionary and prints the name/description of each
 * object to stdout.
 * ~~~{.cpp}
 * // Get pointer to first object.
 * // Note that "p" keeps the object dictionary locked for object access as long as "p" does not point to nullptr (nothing)
 * ObjectPtr p = objectDictionary.GetFirstObj();
 * while (p)
 * {
 *   std::cout << p->GetObjectName() << std::endl;
 *   ++p;
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object access).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.\n
 * Consider acquisition of an extra lock for object access. See [LockForObjAccess()](@ref gpcc::cood::IObjectAccess::LockForObjAccess)
 * for details.
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
 * [ObjectPtr](@ref gpcc::cood::ObjectPtr) object referencing to the first object contained in the object dictionary.
 * The first object is the one with the smallest index.
 * \n
 * If the object dictionary is empty, then the returned ObjectPtr will point to nothing (nullptr).\n
 * \n
 * __Note:__\n
 * If the returned ObjectPtr points to an object, then the ObjectPtr will also hold a lock for object access.\n
 * The lock hold by the ObjectPtr will be released when the ObjectPtr is destroyed or when nullptr is assigned to the
 * ObjectPtr.\n
 * \n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
 */

/**
 * \fn gpcc::cood::ObjectPtr IObjectAccess::GetObject(uint16_t const index)
 * \brief Performs a look-up in the object dictionary for an object with a given index.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object access).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.\n
 * Consider acquisition of an extra lock for object access. See [LockForObjAccess()](@ref gpcc::cood::IObjectAccess::LockForObjAccess)
 * for details.
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
 * Index that shall be looked-up.
 *
 * \return
 * [ObjectPtr](@ref gpcc::cood::ObjectPtr) referencing to the object associated with the index given by
 * parameter `index`.\n
 * \n
 * If there is no object associated with the given index contained in the object dictionary, then the returned
 * ObjectPtr will point to nothing (nullptr).\n
 * \n
 * Alternatively: If you want the next nearest subsequent object, please refer to
 * [GetNextNearestObject(...)](@ref gpcc::cood::IObjectAccess::GetNextNearestObject).\n
 * \n
 * __Note:__\n
 * If the returned ObjectPtr points to an object, then the ObjectPtr will also hold a lock for object access.\n
 * The lock hold by the ObjectPtr will be released when the ObjectPtr is destroyed or when nullptr is assigned to the
 * ObjectPtr.\n
 * \n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
 */

/**
 * \fn gpcc::cood::ObjectPtr IObjectAccess::GetNextNearestObject(uint16_t const index)
 * \brief Retrieves the object registered at a given index or a subsequent object at the next nearest index.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe (this acquires and releases a lock for object access).\n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.\n
 * Consider acquisition of an extra lock for object access. See [LockForObjAccess()](@ref gpcc::cood::IObjectAccess::LockForObjAccess)
 * for details.
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
 * Index that shall be looked-up.
 *
 * \return
 * [ObjectPtr](@ref gpcc::cood::ObjectPtr) referencing to the object associated with the index given by
 * parameter `index`.\n
 * \n
 * If there is no object associated with the given index contained in the object dictionary, then this method will
 * return the next nearest subsequent object. If there if not any subsequent object, then the returned ObjectPtr will
 * point to nothing (nullptr).\n
 * \n
 * __Note:__\n
 * If the returned ObjectPtr points to an object, then the ObjectPtr will also hold a lock for object access.\n
 * The lock hold by the ObjectPtr will be released when the ObjectPtr is destroyed or when nullptr is assigned to the
 * ObjectPtr.\n
 * \n
 * See class [ObjectDictionary](@ref gpcc::cood::ObjectDictionary), chapter "Locking" for details.
 */

} // namespace cood
} // namespace gpcc

#endif // IOBJECTACCESS_HPP_201810042131
