/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "HierarchicNamedRWLockNode.hpp"
#include <gpcc/resource_management/objects/exceptions.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/string/tools.hpp>
#include <algorithm>
#include <cstring>

namespace gpcc                {
namespace ResourceManagement  {
namespace Objects             {
namespace internal            {

/**
 * \brief Constructor. Creates a root-node containing no child-nodes.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
HierarchicNamedRWLockNode::HierarchicNamedRWLockNode(void)
: pParentNode(nullptr)
, nameFragment()
, locks(0)
, locksInChilds(0U)
, childNodes()
{
}

/**
 * \brief Constructor. Creates a node with no lock, no child-nodes, a given name fragment, and a given parent-node.
 *
 * - - -
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
 * \param _pParentNode
 * Pointer to the parent node.\n
 * nullptr is not allowed.\n
 * The referenced node is not modified by this constructor; e.g. the new node is not added to the list of
 * child-nodes of the parent node.
 *
 * \param _nameFragment
 * Name fragment for this node.\n
 * The complete name of the node can be retrieved by concatenating the name fragments of all parent nodes
 * starting at the root node until this node.\n
 * An empty string will not be accepted.
 */
HierarchicNamedRWLockNode::HierarchicNamedRWLockNode(HierarchicNamedRWLockNode* _pParentNode,
                                                     std::string const & _nameFragment)
: pParentNode(_pParentNode)
, nameFragment(_nameFragment)
, locks(0)
, locksInChilds(0U)
, childNodes()
{
  if ((_pParentNode == nullptr) || (_nameFragment.empty()))
    throw std::invalid_argument("HierarchicNamedRWLockNode::HierarchicNamedRWLockNode");
}

/**
 * \brief Move-constructor.
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
 * The content of the node referenced by this parameter will be move into the new constructed node.\n
 * The `pParentNode` attribute of all childs will be updated.\n
 * This node will be left with no locks and no childs.\n
 * The child-list of the parent node will not be updated by this.
 */
HierarchicNamedRWLockNode::HierarchicNamedRWLockNode(HierarchicNamedRWLockNode && other) noexcept
: pParentNode(other.pParentNode)
, nameFragment(std::move(other.nameFragment))
, locks(other.locks)
, locksInChilds(other.locksInChilds)
, childNodes(std::move(other.childNodes))
{
  for (auto & child: childNodes)
    child.pParentNode = this;

  other.pParentNode = nullptr;
  other.nameFragment.clear();
  other.locks = 0;
  other.locksInChilds = 0;
  other.childNodes.clear();
}

/**
 * \brief Move-assignment operator.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * The content of the node referenced by this will be move-assigned to this node.\n
 * The `pParentNode` attribute of all moved childs will be updated.\n
 * All childs owned by this node before the move-assignment operation will be released.\n
 * The node referenced by `rhv` will be left with no locks and no childs.\n
 * The child-list of the parent node will not be updated by this.
 */
HierarchicNamedRWLockNode& HierarchicNamedRWLockNode::operator=(HierarchicNamedRWLockNode && rhv) noexcept
{
  if (this != &rhv)
  {
    pParentNode   = rhv.pParentNode;
    nameFragment  = std::move(rhv.nameFragment);
    locks         = rhv.locks;
    locksInChilds = rhv.locksInChilds;
    childNodes    = std::move(rhv.childNodes);

    for (auto & child: childNodes)
      child.pParentNode = this;

    rhv.pParentNode = nullptr;
    rhv.nameFragment.clear();
    rhv.locks = 0;
    rhv.locksInChilds = 0;
    rhv.childNodes.clear();
  }

  return *this;
}

/**
 * \brief Retrieves an existing node with specific name from the tree or creates a new node and inserts it
 *        into the tree.
 *
 * If a node with the given name is not existing, then a new node will be created. Depending on the position of
 * the new node in the tree, one more node may be created.
 *
 * Use @ref GetExistingNode() if a new node shall not be created and a nullptr shall be returned instead.
 *
 * \htmlonly <style>div.image img[src="resource_management/objects/HNRWL_creation_of_new_node1.png"]{width:40%;}</style> \endhtmlonly
 * \image html "resource_management/objects/HNRWL_creation_of_new_node1.png" "Creation of new leaf-node \"A/foo/\""
 * \n\n
 * \htmlonly <style>div.image img[src="resource_management/objects/HNRWL_creation_of_new_node2.png"]{width:40%;}</style> \endhtmlonly
 * \image html "resource_management/objects/HNRWL_creation_of_new_node2.png" "Retrieval of existing node \"A/B/C\""
 * \n\n
 * \htmlonly <style>div.image img[src="resource_management/objects/HNRWL_creation_of_new_node3.png"]{width:40%;}</style> \endhtmlonly
 * \image html "resource_management/objects/HNRWL_creation_of_new_node3.png" "Creation of intermediate node \"A/B/\" (scenario 'a' in code)"
 * \n\n
 * \htmlonly <style>div.image img[src="resource_management/objects/HNRWL_creation_of_new_node4.png"]{width:40%;}</style> \endhtmlonly
 * \image html "resource_management/objects/HNRWL_creation_of_new_node4.png" "Creation of new leaf-node \"A/B2/\" (scenario 'b' in code)"
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
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
 * \param rootNode
 * Root node of the tree.
 *
 * \param name
 * Name of the node that shall be retrieved.\n
 * This must not be an empty string.
 *
 * \return
 * Pointer to a node from the tree.\n
 * Ownership remains at the tree. The object must not be destroyed by the caller.
 */
HierarchicNamedRWLockNode* HierarchicNamedRWLockNode::GetOrCreateNode(HierarchicNamedRWLockNode& rootNode, std::string const & name)
{
  if (name.empty())
    throw std::invalid_argument("HierarchicNamedRWLockNode::GetOrCreateNode");

  size_t nameOffset = 0;

  // ------------------------------------------------------------------------------
  // Look for the parent node that either contains a child node with matching name
  // or where we have to create 1..2 new child nodes.
  // ------------------------------------------------------------------------------
  size_t nbOfSameChars;
  HierarchicNamedRWLockNode* pParent = &rootNode;
  HierarchicNamedRWLockNode* pChild;
  do
  {
    pChild = pParent->FindChildNode(name[nameOffset]);

    // case: No matching child -> create a new child node below pParent
    if (pChild == nullptr)
    {
      pParent->childNodes.emplace_back(pParent, name.substr(nameOffset));
      return &(pParent->childNodes.back());
    }

    // Still here: There is a child pChild with exactly or partially matching name.
    // Let's determine the number of equal characters in fragment name.
    size_t maxNbOfSameChars = std::min(pChild->nameFragment.length(), name.length() - nameOffset);

    if (maxNbOfSameChars == 0U)
      throw std::runtime_error("HierarchicNamedRWLockNode::GetOrCreateNode: Invalid child name fragment length or undefined error");

    char const *pA = pChild->nameFragment.c_str();
    char const * const _pA = pA;
    char const *pB = name.c_str() + nameOffset;

    while (maxNbOfSameChars-- != 0U)
    {
      if (*(++pA) != *(++pB))
        break;
    }

    nbOfSameChars = pA - _pA;

    // Case: First 'nbOfSameChars' characters of fragment names are equal.
    // We will need to create 1..2 new child-node below pParent. Let's leave the loop to
    // do further examination and create the nodes.
    if (nbOfSameChars != pChild->nameFragment.length())
      break;

    // Still here: pChild's name is either a match, or:
    // a) A child of pChild matches
    // b) A grand-child of pChild matches
    // c) 1..2 new nodes will be created below pChild or any of its children

    nameOffset += nbOfSameChars;

    // case: Exact match -> pChild is the node we are looking for
    if (nameOffset == name.length())
      return pChild;

    // Its a), b), or c). Continue in child node...
    pParent = pChild;
  }
  while (true);

  // -----------------------------------------------------------------------------------------------------
  // pParent is the node where we have to create 1..2 new child nodes.
  // There are two scenarios. In both of them a new node is created in between pParent and pChild.
  // The scenarios are:
  // a) The new node created in between pParent and pChild is the one we are looking for.
  // b) We need a second new node, that will be a child of the node created in between pParent and pChild.
  //    The second new node is the one we are looking for.
  // -----------------------------------------------------------------------------------------------------
  if ((nameOffset + nbOfSameChars) == name.length())
  {
    // -----------
    // scenario a)
    // -----------

    // prepare updated nameFragment for pChild
    std::string newNameFragment = pChild->nameFragment.substr(nbOfSameChars);

    // create the new node on the stack
    HierarchicNamedRWLockNode newNode(pParent, name.substr(nameOffset));

    // add one child, which is move-constructed from *pChild
    newNode.locksInChilds = pChild->locksInChilds + pChild->GetNbOfLocks();
    newNode.childNodes.emplace_back(std::move(*pChild));
    newNode.childNodes.back().pParentNode = &newNode;
    newNode.childNodes.back().nameFragment = std::move(newNameFragment);

    // now move-assign the new node to the storage of *pChild (which has been moved away)
    *pChild = std::move(newNode);
    return pChild;
  }
  else
  {
    // -----------
    // scenario b)
    // -----------

    // prepare updated nameFragment for pChild
    std::string newNameFragment = pChild->nameFragment.substr(nbOfSameChars);

    // create the new node which will be in between pParent and pChild
    HierarchicNamedRWLockNode newNode(pParent, name.substr(nameOffset, nbOfSameChars));

    // Add one child, which is the node we are looking for.
    // This is the node that will be returned by this function.
    newNode.childNodes.emplace_back(&newNode, name.substr(nameOffset + nbOfSameChars));

    // add another child, which is move-constructed from *pChild
    newNode.locksInChilds = pChild->locksInChilds + pChild->GetNbOfLocks();
    newNode.childNodes.emplace_front(std::move(*pChild));
    newNode.childNodes.front().pParentNode = &newNode;
    newNode.childNodes.front().nameFragment = std::move(newNameFragment);

    // now move-assign the new node to the storage of *pChild (which has been moved away)
    *pChild = std::move(newNode);
    return &(pChild->childNodes.back());
  }
}

/**
 * \brief Retrieves an existing node with specific name from the tree.
 *
 * If a node with the given name is not existing, then `nullptr` will be returned.
 *
 * Use @ref GetOrCreateNode() if a new node shall be created if there is no existing node.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * std::bad_alloc will never be thrown.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rootNode
 * Root node of the tree.
 *
 * \param name
 * Name of the node that shall be retrieved.\n
 * This must not be an empty string.
 *
 * \return
 * Pointer to a node from the tree, or `nullptr` if there is no node with given name.\n
 * Ownership remains at the tree. The object must not be destroyed by the caller.
 */
HierarchicNamedRWLockNode* HierarchicNamedRWLockNode::GetExistingNode(HierarchicNamedRWLockNode& rootNode, std::string const & name)
{
  if (name.empty())
    throw std::invalid_argument("HierarchicNamedRWLockNode::GetExistingNode");

  size_t nameOffset = 0;

  // ------------------------------------------------------------------------------
  // Look for the parent node that either contains a child node with matching name
  // or where we can give up.
  // ------------------------------------------------------------------------------
  size_t nbOfSameChars;
  HierarchicNamedRWLockNode* pParent = &rootNode;
  HierarchicNamedRWLockNode* pChild;
  while (true)
  {
    pChild = pParent->FindChildNode(name[nameOffset]);

    // case: No matching child
    if (pChild == nullptr)
      return nullptr;

    // Still here: There is a child pChild with exactly or partially matching name.
    // Let's determine the number of equal characters in fragment name.
    size_t maxNbOfSameChars = std::min(pChild->nameFragment.length(), name.length() - nameOffset);

    if (maxNbOfSameChars == 0U)
      throw std::runtime_error("HierarchicNamedRWLockNode::GetExistingNode: Invalid child name fragment length or undefined error");

    char const *pA = pChild->nameFragment.c_str();
    char const * const _pA = pA;
    char const *pB = name.c_str() + nameOffset;

    while (maxNbOfSameChars-- != 0U)
    {
      if (*(++pA) != *(++pB))
        break;
    }

    nbOfSameChars = pA - _pA;

    // Case: First 'nbOfSameChars' characters of fragment names are equal, but that is
    // less than the name fragment of pChild. There is no matching node.
    if (nbOfSameChars != pChild->nameFragment.length())
      return nullptr;

    // Still here: pChild's name is either a match, or:
    // a) A child of pChild matches
    // b) A grand-child of pChild matches
    // c) There is no match

    nameOffset += nbOfSameChars;

    // case: Exact match -> pChild is the node we are looking for
    if (nameOffset == name.length())
      return pChild;

    // Its a), b), or c). Continue in child node...
    pParent = pChild;
  }
}

/**
 * \brief Retrieves, if any parent node is write-locked.
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
 * true = At least one parent node is write-locked\n
 * false = No parent node is write-locked
 */
bool HierarchicNamedRWLockNode::IsAnyParentWriteLocked(void) const noexcept
{
  HierarchicNamedRWLockNode const * pNode = pParentNode;

  while (pNode != nullptr)
  {
    if (pNode->IsWriteLocked())
      return true;

    pNode = pNode->pParentNode;
  }

  return false;
}

/**
 * \brief Removes all read- and write-locks from the tree.
 *
 * This method is only applicable to root-nodes.
 *
 * \post   All read- and write-locks are removed from the tree.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * std::bad_alloc will never be thrown.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::Reset(void)
{
  if (pParentNode != nullptr)
    throw std::logic_error("HierarchicNamedRWLockNode::Reset: This method is only applicable to root-nodes");

  locks = 0;
  locksInChilds = 0U;
  childNodes.clear();
}

/**
 * \brief Read-locks the node.
 *
 * \pre    The node must not be locked by a writer. Locks by other readers are allowed.
 *         The caller must ensure this precondition, e.g. via @ref IsWriteLocked().
 *
 * \pre    The calling function must have checked, if a read-lock is allowed in the scope of the whole tree.
 *         This method does not check the lock-state of any parent or child node.
 *
 * \pre    The calling function must have checked the `locksInChilds` attribute of the root node for a potential
 *         overflow. This method does not check for any overflow.
 *
 * \post   The node will be read-locked. The read-lock count will be incremented.
 *
 * \post   The `locksInChilds` attribute of all parent nodes up to the root node will be updated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::GetReadLock(void)
{
  if (locks < 0)
    throw std::logic_error("HierarchicNamedRWLockNode::GetReadLock: There is a write-lock");

  // Note: No need to check overflow of "locks". Caller has checked "locksInChilds" of root node for overflow.
  locks++;

  if (pParentNode != nullptr)
    pParentNode->IncLocksInChilds();
}

/**
 * \brief Releases a read-lock.
 *
 * \pre    The node must be locked by at least one reader.
 *
 * \post   The read-lock count will be decremented. The node will be unlocked if the last read-lock is released.
 *
 * \post   The `locksInChilds` attribute of all parent nodes up to the root node will be updated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws NotLockedError   Node is either unlocked or locked by a writer ([details](@ref NotLockedError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::ReleaseReadLock(void)
{
  if (locks < 1)
    throw NotLockedError();

  locks--;

  if (pParentNode != nullptr)
    pParentNode->DecLocksInChilds();
}

/**
 * \brief Write-locks the node.
 *
 * \pre    The node must not be locked by a writer or reader.
 *         The caller must ensure this precondition, e.g. via @ref IsLocked().
 *
 * \pre    The calling function must have checked, if a write-lock is allowed in the scope of the whole tree.
 *         This method does not check the lock-state of any parent or child node.
 *
 * \pre    The calling function must have checked the `locksInChilds` attribute of the root node for a potential
 *         overflow. This method does not check for any overflow.
 *
 * \post   The node will be write-locked.
 *
 * \post   The `locksInChilds` attribute of all parent nodes up to the root node will be updated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::GetWriteLock(void)
{
  if (locks != 0)
    throw std::logic_error("HierarchicNamedRWLockNode::GetWriteLock: There is a lock");

  locks = -1;

  if (pParentNode != nullptr)
    pParentNode->IncLocksInChilds();
}

/**
 * \brief Releases a write-lock.
 *
 * \pre    The node must be locked by a writer.
 *
 * \post   The node will be unlocked.
 *
 * \post   The `locksInChilds` attribute of all parent nodes up to the root node will be updated.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws NotLockedError   Node is either unlocked or locked by one or more readers ([details](@ref NotLockedError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::ReleaseWriteLock(void)
{
  if (locks != -1)
    throw NotLockedError();

  locks = 0;

  if (pParentNode != nullptr)
    pParentNode->DecLocksInChilds();
}

/**
 * \brief Determines a parent node of this node where it is worth to invoke @ref RemoveUnusedChilds().
 *
 * This method walks the tree up from this node to the root node until either:
 * - the root node is reached
 * - a locked node is reached
 * - a node with locks in child-nodes is reached
 *
 * The node where the tree-walk stops is returned by this method.
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
 * Node of tree where it is worth to invoke @ref RemoveUnusedChilds(). \n
 * The returned node might be _this_ node.
 */
HierarchicNamedRWLockNode* HierarchicNamedRWLockNode::GetStartPointForRemovalOfUnusedChilds(void) noexcept
{
  HierarchicNamedRWLockNode* startPoint = this;

  while ((startPoint->pParentNode != nullptr) &&
         (startPoint->locks == 0) &&
         (startPoint->locksInChilds == 0U))
  {
    startPoint = startPoint->pParentNode;
  }

  return startPoint;
}

/**
 * \brief Removes all child nodes which are not locked and whose childs and grand-childs are also all not locked.
 *
 * \post   Any child nodes of this node, which are not locked and whose child-nodes and grand-child-nodes
 *         are also not locked will be removed from the tree and will be destroyed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::RemoveUnusedChilds(void) noexcept
{
  auto it = childNodes.begin();

  while (it != childNodes.end())
  {
    if (((*it).IsLocked()) || ((*it).IsAnyLockInChilds()))
      ++it;
    else
      it = childNodes.erase(it);
  }
}

/**
 * \brief Removes this node from the tree.
 *
 * \pre   The node is not the root node of the tree.
 *
 * \pre   The node is not locked.
 *
 * \pre   The node has no more than one child node.
 *
 * \post  The node has been either destroyed, or its child-node (if any) has been move-assigned to it,
 *        or the node and the tree have not been modified due to an out-of-memory condition.
 *
 * \htmlonly <style>div.image img[src="resource_management/objects/HNRWL_remove_self1.png"]{width:50%;}</style> \endhtmlonly
 * \image html "resource_management/objects/HNRWL_remove_self1.png" "Candidates (green) for RemoveSelf()"
 *
 * __After this method has returned, any pointer/reference to any node from the tree__
 * __(except the root node) must be dropped.__
 *
 * This may fail with an `std::bad_alloc` exception if there is a child node and creation of the new
 * name fragment for the child node failed. In this case, the node remains in the tree.
 * This is not harmful. There may be a retry to remove the node later.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory. If there is a child-node, then a new name fragment string
 *                          must be created.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::RemoveSelf(void)
{
  if (pParentNode == nullptr)
    throw std::logic_error("HierarchicNamedRWLockNode::RemoveSelf: Not applicable to root node");

  if (locks != 0)
    throw std::logic_error("HierarchicNamedRWLockNode::RemoveSelf: Node is locked");

  // case: no child nodes
  if (childNodes.empty())
  {
    auto matcher = [&](HierarchicNamedRWLockNode const & e) { return (this == &e); };
    auto it = std::find_if(pParentNode->childNodes.begin(), pParentNode->childNodes.end(), matcher);

    if (it == pParentNode->childNodes.end())
      PANIC(); // Invalid parent/child relationship

    pParentNode->childNodes.erase(it);

    return;
  }

  if (childNodes.size() > 1U)
    throw std::logic_error("HierarchicNamedRWLockNode::RemoveSelf: Not applicable to nodes with two or more child nodes");

  auto & child = childNodes.front();
  std::string newNameFragment = nameFragment + child.nameFragment;

  HierarchicNamedRWLockNode temp(std::move(child));
  temp.pParentNode = pParentNode;
  *this = std::move(temp);
  nameFragment = std::move(newNameFragment);
}

/**
 * \brief Searches for a child node whose name fragment starts with a specific character.\n
 *        Grand-child-nodes are not included.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param firstCharOfNameFragment
 * The first character of the child node's name fragment must match this.
 *
 * \return
 * Pointer to the child node whose name fragment starts with `firstCharOfNameFragment`.\n
 * nullptr, if no matching child node was found.
 */
HierarchicNamedRWLockNode* HierarchicNamedRWLockNode::FindChildNode(char const firstCharOfNameFragment)
{
  for (auto & child: childNodes)
  {
    if (child.nameFragment.empty())
      throw std::logic_error("HierarchicNamedRWLockNode::FindChildNode: Child node has empty name");

    if (child.nameFragment.front() == firstCharOfNameFragment)
      return &child;
  }

  return nullptr;
}

/**
 * \brief Increments the `locksInChilds` attribute of this node and of all parent nodes up to the root node.
 *
 * \pre   The caller must have checked, that the `locksInChilds` attribute will not overflow in the root node.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::IncLocksInChilds(void) noexcept
{
  locksInChilds++;

  HierarchicNamedRWLockNode* pNode = pParentNode;
  while (pNode != nullptr)
  {
    pNode->locksInChilds++;
    pNode = pNode->pParentNode;
  }
}

/**
 * \brief Decrements the `locksInChilds` attribute of the node and of all parent nodes up to the root node.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object/object-tree is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void HierarchicNamedRWLockNode::DecLocksInChilds(void) noexcept
{
  if (locksInChilds == 0U)
    PANIC();

  locksInChilds--;

  HierarchicNamedRWLockNode* pNode = pParentNode;
  while (pNode != nullptr)
  {
    if (pNode->locksInChilds == 0U)
      PANIC();

    pNode->locksInChilds--;
    pNode = pNode->pParentNode;
  }
}

} // namespace internal
} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc
