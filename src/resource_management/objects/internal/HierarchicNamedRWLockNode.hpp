/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef HIERARCHICNAMEDRWLOCKNODE_HPP_2018708202227
#define HIERARCHICNAMEDRWLOCKNODE_HPP_2018708202227

#include <list>
#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc                {
namespace resource_management {
namespace objects             {
namespace internal            {

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_OBJECTS_INTERNALS
 * \brief Class representing a node in the tree used by class @ref HierarchicNamedRWLock to manage read- and write-locks.
 *
 * # Tree structure
 * Each tree has a root node. The root node cannot be read- or write-locked and it has no parent node.
 *
 * The root node and any other node can have child nodes. Each child node has a pointer to its parent node.
 *
 * Each node has a name. The name of each node is the name of the parent node plus a string fragment called "name fragment".
 *
 * Each node may represent a lock and stores the following information:
 * - the type and number of locks
 * - the number of locks in child nodes
 *
 * In addition to nodes representing locks, there may be additional nodes required to resolve the names of child nodes.\n
 * __Example:__\n
 * We have three locks:
 * - "A/"
 * - "A/B/C1/"
 * - "A/B/C2/"
 *
 * The resulting tree looks like this:
 * \htmlonly <style>div.image img[src="resource_management/objects/HNRWL_simple_tree_example.png"]{width:70%;}</style> \endhtmlonly
 * \image html "resource_management/objects/HNRWL_simple_tree_example.png" "Tree example"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.\n
 * Any thread-safety specification from methods of this class must be applied to the whole tree of
 * @ref HierarchicNamedRWLockNode objects linked with each other.
 */
class HierarchicNamedRWLockNode final
{
  public:
    HierarchicNamedRWLockNode(void);
    HierarchicNamedRWLockNode(HierarchicNamedRWLockNode* _pParentNode, std::string const & _nameFragment);
    HierarchicNamedRWLockNode(HierarchicNamedRWLockNode const &) = delete;
    HierarchicNamedRWLockNode(HierarchicNamedRWLockNode && other) noexcept;
    ~HierarchicNamedRWLockNode(void) = default;

    HierarchicNamedRWLockNode& operator=(HierarchicNamedRWLockNode const &) = delete;
    HierarchicNamedRWLockNode& operator=(HierarchicNamedRWLockNode && rhv) noexcept;

    static HierarchicNamedRWLockNode* GetOrCreateNode(HierarchicNamedRWLockNode& rootNode, std::string const & name);
    static HierarchicNamedRWLockNode* GetExistingNode(HierarchicNamedRWLockNode& rootNode, std::string const & name);

    bool IsLocked(void) const noexcept;
    bool IsWriteLocked(void) const noexcept;
    bool IsReadLocked(void) const noexcept;
    uint32_t GetNbOfLocks(void) const noexcept;

    bool IsAnyParentWriteLocked(void) const noexcept;
    bool IsAnyLockInChilds(void) const noexcept;

    size_t GetNbOfChilds(void) const noexcept;
    uint32_t GetNbOfLocksInChilds(void) const noexcept;

    void Reset(void);

    void GetReadLock(void);
    void ReleaseReadLock(void);
    void GetWriteLock(void);
    void ReleaseWriteLock(void);

    HierarchicNamedRWLockNode* GetStartPointForRemovalOfUnusedChilds(void) noexcept;
    void RemoveUnusedChilds(void) noexcept;
    void RemoveSelf(void);

  private:
    /// Pointer to the parent node. nullptr, if this is the root node.
    HierarchicNamedRWLockNode* pParentNode;

    /// Fragment of the node name.
    /** The name of the node can be retrieved by concatenating the fragments of all parent nodes starting
        at the root node until this node.\n
        If this is the root node, then this is an empty string. */
    std::string nameFragment;

    /// Lock state of this node.
    /** -1 = locked by one writer\n
        0  = unlocked\n
        >0 = number of readers who have locked\n
        Note: The root-node cannot be locked. */
    int32_t locks;

    /// Total number of read- and write-locks in child nodes.
    uint32_t locksInChilds;

    /// Child nodes.
    /** Note:\n
        The first character of the @ref nameFragment attribute of all child nodes is always different.\n
        There are no child nodes whose @ref nameFragment starts with the same character. */
    std::list<HierarchicNamedRWLockNode> childNodes;


    HierarchicNamedRWLockNode* FindChildNode(char const firstCharOfNameFragment);

    void IncLocksInChilds(void) noexcept;
    void DecLocksInChilds(void) noexcept;
};

/**
 * \brief Retrieves if the node is locked (read-lock or write-lock), or if there is no lock.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * true  = The node is locked by a writer or by at least one reader\n
 * false = The node is not locked
 */
inline bool HierarchicNamedRWLockNode::IsLocked(void) const noexcept
{
  return (locks != 0);
}

/**
 * \brief Retrieves if the node is write-locked or not.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * true  = The node is locked by a writer\n
 * false = The node is not locked by a writer
 */
inline bool HierarchicNamedRWLockNode::IsWriteLocked(void) const noexcept
{
  return (locks < 0);
}

/**
 * \brief Retrieves if the node is read-locked or not.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * true  = The node is locked by a reader\n
 * false = The node is not locked by a reader
 */
inline bool HierarchicNamedRWLockNode::IsReadLocked(void) const noexcept
{
  return (locks > 0);
}

/**
 * \brief Retrieves the number of readers or writers who have locked this node.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * Number of readers or writers who have locked this node.\n
 * 0 = no lock
 */
inline uint32_t HierarchicNamedRWLockNode::GetNbOfLocks(void) const noexcept
{
  if (locks >= 0)
    return locks;
  else
    return 1U;
}

/**
 * \brief Retrieves if any child-node is locked (read-lock or write-lock), or if there is no lock in any child-node.
 *        Grand-childs are included in the check.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * true  = At least one child-node (or grand-child node) is locked by a writer or by at least one reader\n
 * false = No child-node is locked by any reader or writer
 */
inline bool HierarchicNamedRWLockNode::IsAnyLockInChilds(void) const noexcept
{
  return (locksInChilds != 0U);
}

/**
 * \brief Retrieves the number of direct child nodes. Grand-children are not included.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * Number of child nodes.\n
 * Grand-children are not included.
 */
inline size_t HierarchicNamedRWLockNode::GetNbOfChilds(void) const noexcept
{
  return childNodes.size();
}

/**
 * \brief Retrieves the total number of read- and write-locks in child-nodes and grand-child-nodes.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
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
 * Number of read- and write-locks in child-nodes and grand-child-nodes.
 */
inline uint32_t HierarchicNamedRWLockNode::GetNbOfLocksInChilds(void) const noexcept
{
  return locksInChilds;
}

} // namespace internal
} // namespace objects
} // namespace resource_management
} // namespace gpcc

#endif // HIERARCHICNAMEDRWLOCKNODE_HPP_2018708202227
