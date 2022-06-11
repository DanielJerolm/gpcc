/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018, 2022 Daniel Jerolm

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

#ifndef HIERARCHICNAMEDRWLOCK_HPP_201808192204
#define HIERARCHICNAMEDRWLOCK_HPP_201808192204

#include "internal/HierarchicNamedRWLockNode.hpp"
#include <string>

namespace gpcc                {
namespace ResourceManagement  {
namespace Objects             {

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_OBJECTS
 * \brief Provides read- and write-locking of resources and groups of resources based
 *        on the resource's name.
 *
 * # Features
 * - Arbitration of access to arbitrary resources based on the resource's name.
 * - Arbitration of access to groups of resources. Group names are just prepended to the resource's name
 *   like directories to a filename. See example below.
 * - The resources may be of any type (e.g. files, hardware resources, etc.).
 * - Non-intrusive. The managed resources do not need to be modified and are even not aware of the arbitration.
 * - Differentiation between readers (non-modifying access) and writers (modifying access).\n
 *   See section "Policy" for details.
 * - No static registration of resources and groups necessary. Any resource name and group name can be
 *   used dynamically.
 * - @ref Reset() method allow to release all locks.
 *
 * # Restrictions
 * This class does not block threads if a lock on a resource cannot be acquired.
 *
 * # Definition of resources and groups of resources
 * _Resources_ are identified by their name.\n
 * _Groups of resources_ are identified by common prefixes of resources names. Example:
 * - A5
 * - A6
 * - B8
 *
 * Here we have:\n
 * Groups: "A"\n
 * Resources: "5", "6" in group "A"; "B8" in no group.
 *
 * If we add "B9" to our example we get:\n
 * Groups: "A" and "B"\n
 * Resources: "5", "6" in group "A"; "8" and "9" in group "B".
 *
 * At this point it seems a bit weird that resource "B8" has turned into group "B" containing the
 * resources "8" and "9". To simplify grouping and to avoid spurious creation of groups, client code
 * can select and use a character that acts as a separator between groups and resources. The character
 * must not be part of resource and group names. Example:
 * - A/5/
 * - A/6/
 * - B8/
 *
 * Here we have:\n
 * Groups: "A"\n
 * Resources: "5", "6" in group "A"; "B8" in no group.
 *
 * If we add "B9/" to our example we get:\n
 * Groups: "A"\n
 * Resources: "5", "6" in group "A"; "B8" and "B9" in no group.
 *
 * Using a separating character (e.g. '/') avoids unwanted grouping.\n
 * If we add "A/55/" to our example, then we get another resource "55" in group "A".
 *
 * # Policy
 * A _resource_ or a _group_ can be locked by either one writer or by one or more readers, but never
 * by a writer and reader at the same time.
 *
 * There are rules which allow that resources within a group can be locked even if the enclosing group
 * is already locked. The other way round the enclosing group may be locked even if resources contained
 * in that group are already locked. This also applies across multiple groups, e.g. a group can be locked
 * even if a group or resource in a sub-group or sub-sub-group is already locked. The rules for this are:
 *
 * __Rules for acquisition of a read-lock:__\n
 * - The resource/group that shall be locked must not yet be write-locked.
 * - The enclosing group (and any grandparent group) must not yet be write-locked.
 *
 * __Rules for acquisition of a write-lock:__\n
 * - The resource/group that shall be locked must not yet be read- or write-locked.
 * - The enclosing group (and any grandparent group) must not yet be write-locked.
 * - If a group shall be write-locked, then no child group/resource must be read- or write-locked yet.
 *
 * # Scalability
 * The maximum number of locks is 2^32-1.\n
 * The maximum number of different names for resources and groups is 2^32-1.
 *
 * Internally a tree is used to organize tree nodes and leafs which represent groups and resources.
 * The maximum number of cascaded groups should be considered, because tree node destructors may be called recursively.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class HierarchicNamedRWLock final
{
  public:
    HierarchicNamedRWLock(void);
    HierarchicNamedRWLock(HierarchicNamedRWLock const &) = delete;
    HierarchicNamedRWLock(HierarchicNamedRWLock && other) noexcept;
    ~HierarchicNamedRWLock(void);

    HierarchicNamedRWLock& operator=(HierarchicNamedRWLock const &) = delete;
    HierarchicNamedRWLock& operator=(HierarchicNamedRWLock && rhv) = delete;

    void Reset(void);

    bool GetWriteLock(std::string const & resourceName);
    void ReleaseWriteLock(std::string const & resourceName);

    bool GetReadLock(std::string const & resourceName);
    void ReleaseReadLock(std::string const & resourceName);

    bool IsAnyLock(void) const noexcept;

  private:
    /// Root node.
    internal::HierarchicNamedRWLockNode mutable rootNode;

    bool RemoveNodeIfPossible(internal::HierarchicNamedRWLockNode* const pNode) noexcept;
    void CleanupAfterUnlock(internal::HierarchicNamedRWLockNode* const pNode) noexcept;
};

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc

#endif // HIERARCHICNAMEDRWLOCK_HPP_201808192204
