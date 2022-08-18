/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_SMALLDYNAMICNAMEDRWLOCK_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_SMALLDYNAMICNAMEDRWLOCK_HPP_

#include <string>

namespace gpcc
{
namespace ResourceManagement
{
namespace Objects
{

namespace internal
{
  class NamedRWLockEntry;
}

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_OBJECTS
 * \brief Provides read- and write-locking of resources based on the resource's name.
 *
 * Features:
 * - Arbitration of access to arbitrary resources based on the resource's name.
 * - The resources may be of any type (e.g. files, hardware resources, etc.).
 * - Non-intrusive. The managed resources do not need to be modified and are even
 *   not aware of the arbitration.
 * - Differentiation between readers (non-modifying access) and writers (modifying access).
 *   See section "Policy" for details.
 * - No static registration of resources necessary. Any resource name can be used dynamically.
 *
 * # Footprint
 * Internally, this class keeps a single linked list of objects each representing one
 * resource and encapsulating the resource's lock state. Each object exists until the
 * associated resource is unlocked.
 *
 * This class is intended to be used with a relatively small number of resources locked at
 * the same time to keep the number of entries in the single linked list small.
 *
 * If large numbers of locks are required, then @ref LargeDynamicNamedRWLock might be a
 * better choice. @ref LargeDynamicNamedRWLock uses a map to keep the locks. Thus it has a
 * larger footprint, but scales better for large numbers of locks.
 *
 * # Policy
 * A resource can be locked by either one writer or by one or more readers, but never
 * by a writer and reader at the same time.
 *
 * # Multithreading
 * This class has no build-in thread-safety. If necessary, then a @ref osal::Mutex
 * shall be used to protect access to the class.
 *
 * This class does not block when resources are not available. If blocking is
 * required, then @ref osal::RWLock might be a better choice.
 */
class SmallDynamicNamedRWLock
{
  public:
    SmallDynamicNamedRWLock(void) = default;
    SmallDynamicNamedRWLock(SmallDynamicNamedRWLock const &) = delete;
    SmallDynamicNamedRWLock(SmallDynamicNamedRWLock &&) = delete;
    ~SmallDynamicNamedRWLock(void);

    SmallDynamicNamedRWLock& operator=(SmallDynamicNamedRWLock const &) = delete;
    SmallDynamicNamedRWLock& operator=(SmallDynamicNamedRWLock &&) = delete;

    bool TestWriteLock(std::string const & resourceName) const noexcept;
    bool GetWriteLock(std::string const & resourceName);
    void ReleaseWriteLock(std::string const & resourceName);

    bool TestReadLock(std::string const & resourceName) const noexcept;
    bool GetReadLock(std::string const & resourceName);
    void ReleaseReadLock(std::string const & resourceName);

    bool IsLocked(std::string const & resourceName) const noexcept;
    bool AnyLocks(void) const noexcept;

  private:
    /// Pointer to the first @ref internal::NamedRWLockEntry in a single linked list of @ref internal::NamedRWLockEntry instances.
    /** This is `nullptr` if the single linked list is empty.\n
        The list's elements are linked via @ref internal::NamedRWLockEntry::pNext. \n
        The @ref internal::NamedRWLockEntry::pNext attribute of the last @ref internal::NamedRWLockEntry instance
        is nullptr. */
    internal::NamedRWLockEntry* pList = nullptr;

    internal::NamedRWLockEntry* FindEntry(std::string const & resourceName, internal::NamedRWLockEntry** const ppPrev) const noexcept;
    void ReleaseEntry(internal::NamedRWLockEntry* const pEntry, internal::NamedRWLockEntry* const pPrev) noexcept;
    void CreateEntry(std::string const & resourceName, bool const writeLockNotReadLock);
};

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_SMALLDYNAMICNAMEDRWLOCK_HPP_
