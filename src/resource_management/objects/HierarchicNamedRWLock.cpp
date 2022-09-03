/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/resource_management/objects/HierarchicNamedRWLock.hpp>
#include <gpcc/resource_management/objects/exceptions.hpp>
#include "internal/HierarchicNamedRWLockNode.hpp"
#include <gpcc/osal/Panic.hpp>
#include <limits>

namespace gpcc                {
namespace resource_management {
namespace objects             {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
HierarchicNamedRWLock::HierarchicNamedRWLock(void) noexcept
: spRootNode()
{
}

/**
 * \brief Move constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * The locks acquired in `other` are moved to the new constructed instance.\n
 * `other` is left in a state as if it has been just been created using the standard constructor.
 */
HierarchicNamedRWLock::HierarchicNamedRWLock(HierarchicNamedRWLock && other) noexcept
: spRootNode(std::move(other.spRootNode))
{
}

/**
 * \brief Destructor.
 *
 * \pre   All locks must have been released before the object is destroyed.
 *        @ref Reset() could be used to accomplish that.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
HierarchicNamedRWLock::~HierarchicNamedRWLock(void)
{
  if ((spRootNode != nullptr) && (spRootNode->IsAnyLockInChilds()))
    PANIC();
}

/**
 * \brief Releases any previously acquired read- and write-locks.
 *
 * \post   Any read- and write-locks existing before the call to this will be released.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * std::bad_alloc will never be thrown by this method.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLock::Reset(void)
{
  if (spRootNode != nullptr)
    spRootNode->Reset();
}

/**
 * \brief Tries to acquire a write-lock on a group or resource with given name.
 *
 * \post   If all locking rules (see section "Policy" in @ref HierarchicNamedRWLock detailed documentation)
 *         are met, then the group or resource will be write-locked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param resourceName
 * Name of the group or resource that shall be write-locked.\n
 * An empty string is not allowed.
 *
 * \return
 * true  = write-lock has been acquired\n
 * false = write-lock has _not_ been acquired
 */
bool HierarchicNamedRWLock::GetWriteLock(std::string const & resourceName)
{
  if (spRootNode == nullptr)
    spRootNode = std::make_unique<internal::HierarchicNamedRWLockNode>();

  auto pNode = internal::HierarchicNamedRWLockNode::GetOrCreateNode(*spRootNode, resourceName);

  if ((pNode->IsLocked()) ||
      (pNode->IsAnyParentWriteLocked()) ||
      (pNode->IsAnyLockInChilds()))
  {
    return false;
  }

  if (spRootNode->GetNbOfLocksInChilds() == std::numeric_limits<uint32_t>::max())
    throw std::runtime_error("HierarchicNamedRWLock::GetWriteLock: Maximum number of locks reached");

  pNode->GetWriteLock();
  return true;
}

/**
 * \brief Releases a write-lock.
 *
 * \pre    The group or resource referenced by `resourceName` must be write-locked.
 *
 * \post   The group or resource referenced by `resourceName` will be unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws NotLockedError   The group or resource is not locked or it is locked by a reader
 *                          ([details](@ref NotLockedError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param resourceName
 * Name of the group or resource that shall be unlocked.\n
 * An empty string is not allowed.
 */
void HierarchicNamedRWLock::ReleaseWriteLock(std::string const & resourceName)
{
  if (spRootNode == nullptr)
    throw NotLockedError();

  auto pNode = internal::HierarchicNamedRWLockNode::GetExistingNode(*spRootNode, resourceName);

  if (pNode == nullptr)
    throw NotLockedError();

  pNode->ReleaseWriteLock();

  CleanupAfterUnlock(pNode);
}

/**
 * \brief Tries to acquire a read-lock on a group or resource with given name.
 *
 * \post   If all locking rules (see section "Policy" in @ref HierarchicNamedRWLock detailed documentation)
 *         are met, then the group or resource will be read-locked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param resourceName
 * Name of the group or resource that shall be read-locked.\n
 * An empty string is not allowed.
 *
 * \return
 * true  = read-lock has been acquired\n
 * false = read-lock has _not_ been acquired
 */
bool HierarchicNamedRWLock::GetReadLock(std::string const & resourceName)
{
  if (spRootNode == nullptr)
    spRootNode = std::make_unique<internal::HierarchicNamedRWLockNode>();

  auto pNode = internal::HierarchicNamedRWLockNode::GetOrCreateNode(*spRootNode, resourceName);

  if ((pNode->IsWriteLocked()) ||
      (pNode->IsAnyParentWriteLocked()))
  {
    return false;
  }

  if (spRootNode->GetNbOfLocksInChilds() == std::numeric_limits<uint32_t>::max())
    throw std::runtime_error("HierarchicNamedRWLock::GetReadLock: Maximum number of locks reached");

  pNode->GetReadLock();
  return true;
}

/**
 * \brief Releases a read-lock.
 *
 * \pre    The group or resource referenced by `resourceName` must be read-locked.
 *
 * \post   The number of read-locks on the group or resource referenced by `resourceName` will be decremented.
 *         If the last read-lock has been removed, then the group or resource will be unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws NotLockedError   The group or resource is not locked or it is locked by a writer
 *                          ([details](@ref NotLockedError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param resourceName
 * Name of the group or resource that shall be unlocked.\n
 * An empty string is not allowed.
 */
void HierarchicNamedRWLock::ReleaseReadLock(std::string const & resourceName)
{
  if (spRootNode == nullptr)
    throw NotLockedError();

  auto pNode = internal::HierarchicNamedRWLockNode::GetExistingNode(*spRootNode, resourceName);

  if (pNode == nullptr)
    throw NotLockedError();

  pNode->ReleaseReadLock();

  if (!pNode->IsLocked())
    CleanupAfterUnlock(pNode);
}

/**
 * \brief Retrieves if there are any locks.
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
 * \retval true  There is at least one read- or write-lock.
 * \retval false There are no read- and no write-locks.
 */
bool HierarchicNamedRWLock::IsAnyLock(void) const noexcept
{
  if (spRootNode == nullptr)
    return false;
  else
    return spRootNode->IsAnyLockInChilds();
}

/**
 * \brief Removes a node from the tree, if the node has only one child and if childs or grand-childs of
 *        the node are locked.
 *
 * This method is intended to be invoked by @ref CleanupAfterUnlock() only.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pNode
 * Pointer to the node that shall be removed, if the criteria above are met.\n
 * __Zero, one, or more nodes will be removed from the tree and/or will be manipulated!__\n
 * __Any pointers to nodes of the tree must be discarded and not dereferenced any more__
 * __after this method has been called and returned `true`.__\n
 * If the method returns `false`, then no node has been removed or manipulated.
 *
 * \return
 * true  = The node has been removed (or there was at least an attempt to remove it)\n
 * false = The node did not meet the criteria for this cleanup function and has not been removed.
 */
bool HierarchicNamedRWLock::RemoveNodeIfPossible(internal::HierarchicNamedRWLockNode* const pNode) noexcept
{
  if ((pNode->GetNbOfChilds() == 1U) && (pNode->IsAnyLockInChilds()))
  {
    try
    {
      pNode->RemoveSelf();
    }
    catch (std::bad_alloc const &)
    {
      // This error is intentionally ignored.
      // The node remains in the the tree. This is not harmful.
      // The node will be destroyed sooner or later when:
      // - ...the node is locked and unlocked (->RemoveSelf() gets another chance)
      // - ...the child or grand-child is unlocked
      // - ...Reset() is invoked
    }
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * \brief Removes unused nodes from the tree after a node has been unlocked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pNode
 * Pointer to the node that has been unlocked.\n
 * __Zero, one, or more nodes will be removed from the tree and/or will be manipulated!__\n
 * __Any pointers to nodes from the tree must be discarded and not dereferenced any more__
 * __after this method has returned.__
 */
void HierarchicNamedRWLock::CleanupAfterUnlock(internal::HierarchicNamedRWLockNode* const pNode) noexcept
{
  if (!RemoveNodeIfPossible(pNode))
  {
    auto pStartNode = pNode->GetStartPointForRemovalOfUnusedChilds();

    if (pStartNode != pNode)
    {
      pStartNode->RemoveUnusedChilds();
      RemoveNodeIfPossible(pStartNode);
    }
  }
}

} // namespace objects
} // namespace resource_management
} // namespace gpcc
