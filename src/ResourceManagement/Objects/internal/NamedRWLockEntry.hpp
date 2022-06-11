/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_INTERNAL_NAMEDRWLOCKENTRY_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_INTERNAL_NAMEDRWLOCKENTRY_HPP_

#include <string>

namespace gpcc
{
namespace ResourceManagement
{
namespace Objects
{
namespace internal
{

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_OBJECTS_INTERNALS
 * \brief Simple single-writer-/multiple-reader-lock for one instance of a named resource.
 *
 * Instances of this class offer arbitration of access to single resources (objects, files, or similar).
 * Each single instance of an arbitrary resource can be associated with one @ref NamedRWLockEntry instance
 * based on the name of the resource, e.g. a file name.
 *
 * # Access arbitration
 * Class @ref NamedRWLockEntry distinguishes between readers and writers.\n
 * _Readers_ are allowed to perform non-modifying access to the resource only.\n
 * _Writers_ are allowed to perform both non-modifying and modifying access to the resource.
 *
 * The associated resource can be either...
 * - unlocked
 * - locked by exactly one writer
 * - locked by one or more readers
 *
 * Both readers and writers cannot lock the resource at the same time.
 *
 * # Public attributes
 * In addition to the lock state (which is private), class @ref NamedRWLockEntry encapsulates the
 * resource's name (@ref name) and a next-pointer (@ref pNext) for building a single linked list of
 * @ref NamedRWLockEntry instances. @ref name and @ref pNext are public attributes that can be used by
 * the owner of the @ref NamedRWLockEntry (usually some kind of container like @ref SmallDynamicNamedRWLock).
 *
 * # Usage
 * Instances of class @ref NamedRWLockEntry are typically not used directly. Usually instances of
 * class @ref NamedRWLockEntry are used inside containers like @ref SmallDynamicNamedRWLock and hidden by the
 * container's API.
 *
 * # Multithreading
 * This class is not intended for multi-threading. It does not offer any functionality to block
 * any thread until some kind of lock can be acquired. If blocking is required, then @ref gpcc::osal::RWLock
 * might be a better choice.
 */
class NamedRWLockEntry
{
  public:
    /// Next-pointer that can be used by containers to build single linked lists of @ref NamedRWLockEntry instances.
    NamedRWLockEntry* pNext;

    /// Name of the resource associated with this @ref NamedRWLockEntry instance.
    std::string const name;


    NamedRWLockEntry(NamedRWLockEntry* const _pNext, std::string const & _name);
    NamedRWLockEntry(NamedRWLockEntry* const _pNext, std::string const & _name, bool const writeLockNotReadLock);
    NamedRWLockEntry(NamedRWLockEntry const &) = delete;
    NamedRWLockEntry(NamedRWLockEntry &&) = delete;
    ~NamedRWLockEntry(void);

    NamedRWLockEntry& operator=(NamedRWLockEntry const &) = delete;
    NamedRWLockEntry& operator=(NamedRWLockEntry &&) = delete;

    bool GetReadLock(void);
    void ReleaseReadLock(void);
    int GetNbOfReadLocks(void) const noexcept;

    bool GetWriteLock(void) noexcept;
    void ReleaseWriteLock(void);
    bool IsWriteLocked(void) const noexcept;

    bool IsLocked(void) const noexcept;
  private:
    /// Lock state.
    /** -1 = locked by one writer\n
        0  = unlocked\n
        >0 = number of readers who have locked*/
    int locks;
};

} // namespace internal
} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_INTERNAL_NAMEDRWLOCKENTRY_HPP_
