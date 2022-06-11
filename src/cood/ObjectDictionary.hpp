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

#ifndef OBJECTDICTIONARY_HPP_201809301431
#define OBJECTDICTIONARY_HPP_201809301431

#include "IObjectAccess.hpp"
#include "IObjectRegistration.hpp"
#include "gpcc/src/osal/RWLock.hpp"
#include <map>

namespace gpcc {
namespace cood {

class Object;

/**
 * \ingroup GPCC_COOD
 * \brief CANopen Object Dictionary.
 *
 * This class implements a CANopen object dictionary. Group @ref GPCC_COOD contains an explanation what a CANopen
 * dictionary is and what it is good for.
 *
 * Basically this class is a container for CANopen object dictionary objects (subclasses of class @ref Object).
 * Objects stored in the container are associated with a 16 bit index. The index is unique. This means up to one
 * object can be stored per index value. The index can be used to refer to an object stored in the object dictionary.
 *
 * Class @ref ObjectDictionary offers two interfaces:
 * - @ref IObjectRegistration allows to add and remove objects to/from the object dictionary.
 * - @ref IObjectAccess allows to access objects stored in the object dictionary using the object's index.
 *
 * # Locking
 * There are two locks which must be properly used:
 * - Object Dictionary Lock
 * - Locks associated with the data represented by objects
 *
 * ## Object Dictionary Locking
 * Class @ref ObjectDictionary contains an [RWLock](@ref gpcc::osal::RWLock). The [RWLock](@ref gpcc::osal::RWLock)
 * is used to make the object dictionary thread-safe:
 * - A _write-lock_ is required to add and remove objects\n
 *   (_write-lock_ = lock for object dictionary modification).
 * - A _read-lock_ is required to access objects, the meta-data of objects, and the data represented by objects\n
 *   (_read-lock_ = lock for object access).
 *
 * The [RWLock](@ref gpcc::osal::RWLock) allows the acquisition of multiple read-locks, but only one write-lock
 * can be acquired at any time.
 *
 * Any call to the @ref IObjectRegistration interface will automatically acquire a _write-lock_.\n
 * Any call to the @ref IObjectAccess interface will automatically acquire a _read-lock_.\n
 * The two interfaces are therefore locked against each-other.
 *
 * If a @ref Object is retrieved through the @ref IObjectAccess interface, then the @ref Object will be referenced
 * by a @ref ObjectPtr. Class @ref ObjectPtr is a smart-pointer with two functionalities: First it points to an
 * @ref Object and can be used like a raw-pointer; second it holds a read-lock at the object dictionary which
 * contains the @ref Object currently referenced by the @ref ObjectPtr.
 *
 * Calls to @ref IObjectRegistration will block until all @ref ObjectPtr objects have been destroyed or assigned nullptr.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectDictionaryLocking.png"]{width:50%;}</style> \endhtmlonly
 * \image html "cood/ObjectDictionaryLocking.png" "Object Dictionary Locking"
 *
 * ## Data Locking
 * When reading or writing from/to an CANopen object, two locks are required:
 * 1. The object dictionary must be locked for _object access_. This is automatically accomplished by the
 *    @ref ObjectPtr referencing to the CANopen object (see previous chapter).
 * 2. The mutex associated with the application data represented by the object must be locked. This can be
 *    accomplished via @ref Object::LockData().
 *
 * # Object Lifecycle
 * CANopen objects are created by application software that wants to offer access to selected pieces of
 * application data. The objects are created by instantiating a sub-class of class @ref Object, e.g.
 * @ref ObjectVAR, @ref ObjectARRAY, or @ref ObjectRECORD. After object creation, the application data
 * represented by the object can be read and written through the methods offered by base class @ref Object.
 *
 * The application data is accessible via the object, until the @ref Object instance is destroyed.
 * The blue brace in the figure below shows this time-span.\n
 * __It is strictly required, that the life-line of the application data exceeds the life-line of the__
 * __object as shown in the figure below.__
 *
 * When the object is added to an @ref ObjectDictionary instance, ownership will be moved to the
 * object dictionary. Transfer of ownership is enforced by usage of `unique_ptr`. At the same time, the
 * object (and the data represented by it) becomes accessible to clients of the @ref IObjectAccess interface
 * offered by class @ref ObjectDictionary. The object is accessible, until the application removes it from
 * the object dictionary. The red brace in the figure below shows this time-span.
 *
 * Objects are destroyed by the object dictionary when they are removed from the object dictionary. Ownership
 * is not returned to the application.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectLifeCycle.png"]{width:55%;}</style> \endhtmlonly
 * \image html "cood/ObjectLifeCycle.png" "Object Lifecycle"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ObjectDictionary final : public IObjectAccess, public IObjectRegistration
{
    friend class ObjectPtr;

  public:
    ObjectDictionary(void);
    ObjectDictionary(ObjectDictionary const &) = delete;
    ObjectDictionary(ObjectDictionary &&) = delete;
    ~ObjectDictionary(void);

    ObjectDictionary& operator=(ObjectDictionary const &) = delete;
    ObjectDictionary& operator=(ObjectDictionary &&) = delete;

    // <-- Interface IObjectRegistration
    void Clear(void) override;
    void Add(std::unique_ptr<Object> & spObj, uint16_t const index) override;
    void Remove(uint16_t const index) override;
    // --> Interface IObjectRegistration

    // <-- Interface IObjectAccess
    gpcc::osal::RWLockReadLocker LockForObjAccess(void) override;

    size_t GetNumberOfObjects(void) const override;
    std::vector<uint16_t> GetIndices(void) const override;
    ObjectPtr GetFirstObject(void) override;
    ObjectPtr GetObject(uint16_t const index) override;
    ObjectPtr GetNextNearestObject(uint16_t const index) override;
    // --> Interface IObjectAccess

  private:
    /// Lock used to make access to the object dictionary thread-safe.
    gpcc::osal::RWLock mutable lock;

    /// Map containing the objects registered in the object dictionary.
    std::map<uint16_t, Object*> container;


    void IncReadLock(void);
    void DecReadLock(void) noexcept;
};

} // namespace cood
} // namespace gpcc

#endif
