/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_LARGEDYNAMICNAMEDRWLOCK_HPP_
#define SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_LARGEDYNAMICNAMEDRWLOCK_HPP_

#include <unordered_map>
#include <string>

namespace gpcc
{
namespace ResourceManagement
{
namespace Objects
{

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
 * Internally, this class uses a map to store the lock-state of each object. Each entry
 * in the map exists until the associated resource is unlocked.
 *
 * This class is intended to be used with a relatively large number of resources locked at
 * the same time.
 *
 * If the number of simultaneously locked resources is small, then @ref SmallDynamicNamedRWLock
 * might be a better choice. @ref SmallDynamicNamedRWLock uses a single-linked list to keep
 * the locks. Thus it has a smaller footprint and consumes less memory for small numbers
 * of simultaneously locked resources.
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
class LargeDynamicNamedRWLock
{
  public:
    LargeDynamicNamedRWLock(void) = default;
    LargeDynamicNamedRWLock(LargeDynamicNamedRWLock const &) = delete;
    LargeDynamicNamedRWLock(LargeDynamicNamedRWLock &&) = delete;
    ~LargeDynamicNamedRWLock(void);

    LargeDynamicNamedRWLock& operator=(LargeDynamicNamedRWLock const &) = delete;
    LargeDynamicNamedRWLock& operator=(LargeDynamicNamedRWLock &&) = delete;

    bool TestWriteLock(std::string const & resourceName) const noexcept;
    bool GetWriteLock(std::string const & resourceName);
    void ReleaseWriteLock(std::string const & resourceName);

    bool TestReadLock(std::string const & resourceName) const noexcept;
    bool GetReadLock(std::string const & resourceName);
    void ReleaseReadLock(std::string const & resourceName);

    bool IsLocked(std::string const & resourceName) const noexcept;
    bool AnyLocks(void) const noexcept;

  private:
    std::unordered_map<std::string, int> locks;
};

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc

#endif // SRC_GPCC_RESOURCEMANAGEMENT_OBJECTS_LARGEDYNAMICNAMEDRWLOCK_HPP_
