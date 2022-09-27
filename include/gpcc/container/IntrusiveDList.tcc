/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#include "IntrusiveDList.hpp"
#include <stdexcept>

namespace gpcc      {
namespace container {


// <== class IntrusiveDList::iterator

/**
 * \brief Constructor. Creates an iterator referencing to nothing.
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
template <class T>
IntrusiveDList<T>::iterator::iterator(void) noexcept
: pItem(nullptr)
{
};

/**
 * \brief Compares two iterators for equality.
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
 * \param rhv   Value on the right side of the ==-operator
 *
 * \retval true    Both iterators reference to the same list element or to nothing.
 * \retval false   Both iterators reference to different list elements.
 */
template <class T>
bool IntrusiveDList<T>::iterator::operator==(IntrusiveDList::iterator const & rhv) const noexcept
{
  return (pItem == rhv.pItem);
}

/**
 * \brief Compares two iterators for not equal.
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
 * \param rhv   Value on the right side of the !=-operator
 *
 * \retval true    Both iterators reference to different list elements.
 * \retval false   Both iterators reference to the same list element or to nothing.
 */
template <class T>
bool IntrusiveDList<T>::iterator::operator!=(IntrusiveDList::iterator const & rhv) const noexcept
{
  return (pItem != rhv.pItem);
}

/**
 * \brief Compares two iterators for equality.
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
 * \param rhv   Value on the right side of the ==-operator
 *
 * \retval true    Both iterators reference to the same list element or to nothing.
 * \retval false   Both iterators reference to different list elements.
 */
template <class T>
bool IntrusiveDList<T>::iterator::operator==(IntrusiveDList::const_iterator const & rhv) const noexcept
{
  return (pItem == rhv.pItem);
}

/**
 * \brief Compares two iterators for not equal.
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
 * \param rhv   Value on the right side of the !=-operator
 *
 * \retval true    Both iterators reference to different list elements.
 * \retval false   Both iterators reference to the same list element or to nothing.
 */
template <class T>
bool IntrusiveDList<T>::iterator::operator!=(IntrusiveDList::const_iterator const & rhv) const noexcept
{
  return (pItem != rhv.pItem);
}

/**
 * \brief Pre-increments the iterator.
 *
 * \pre   The iterator refers to a list element. It does not refer to past-the-end of the list.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Reference to self, _after_ incrementing.
 */
template <class T>
typename IntrusiveDList<T>::iterator& IntrusiveDList<T>::iterator::operator++(void)
{
  if (pItem == nullptr)
    throw std::logic_error("Attempt to increment past-the-end iterator!");

  pItem = pItem->pNextInIntrusiveDList;
  return *this;
}

/**
 * \brief Post-increments the iterator.
 *
 * \pre   The iterator refers to a list element. It does not refer to past-the-end of the list.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Copy of the iterator, created _before_ incrementing.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::iterator::operator++(int)
{
  if (pItem == nullptr)
    throw std::logic_error("Attempt to increment past-the-end iterator!");

  iterator copy(*this);
  pItem = pItem->pNextInIntrusiveDList;
  return copy;
}

/**
 * \brief Dereference operator.
 *
 * \pre   The iterator refers to a list element. It does not refer to past-the-end of the list.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Reference to the list item.
 */
template <class T>
typename IntrusiveDList<T>::iterator::reference IntrusiveDList<T>::iterator::operator*()
{
  if (pItem == nullptr)
    throw std::logic_error("Attempt to dereference past-the-end iterator!");

  return pItem;
}

/**
 * \brief Constructor. Creates an iterator referencing to a given list item or past-the-end of the list.
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
 * \param _pItem
 * Referenced list item.\n
 * nullptr = no element referenced (past-the-end iterator).
 */
template <class T>
IntrusiveDList<T>::iterator::iterator(T* const _pItem) noexcept
: pItem(_pItem)
{
};

// ==> class IntrusiveDList::iterator

// <== class IntrusiveDList::const_iterator

/**
 * \brief Constructor. Creates an const_iterator referencing to nothing.
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
template <class T>
IntrusiveDList<T>::const_iterator::const_iterator(void) noexcept
: pItem(nullptr)
{
};

/**
 * \brief Copy-constructor. Creates a const_iterator via copy-construction from an iterator.
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
 * Reference to the `iterator` from which a `const_iterator` shall be copy-constructed.
 */
template <class T>
IntrusiveDList<T>::const_iterator::const_iterator(iterator const & other) noexcept
: pItem(other.pItem)
{
};

/**
 * \brief Move-constructor. Creates a const_iterator via move-construction from an iterator.
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
 * Reference to the `iterator` from which a `const_iterator` shall be move-constructed.\n
 * Though move-construction takes place, the other iterator is not modified.
 */
template <class T>
IntrusiveDList<T>::const_iterator::const_iterator(iterator && other) noexcept
: pItem(other.pItem)
{
};

/**
 * \brief Copy-assignment operator. Copy-assigns an iterator to this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
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
 * Right hand value of copy-assignment operator.
 *
 * \return
 * Reference to self.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator& IntrusiveDList<T>::const_iterator::operator=(iterator const & rhv) noexcept
{
  pItem = rhv.pItem;
  return *this;
}

/**
 * \brief Move-assignment operator. Move-assigns an iterator to this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
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
 * Right hand value of copy-assignment operator.\n
 * Though move-assignment takes place, the referenced iterator will not be modified.
 *
 * \return
 * Reference to self.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator& IntrusiveDList<T>::const_iterator::operator=(iterator && rhv) noexcept
{
  pItem = rhv.pItem;
  return *this;
}

/**
 * \brief Compares two iterators for equality.
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
 * \param rhv   Value on the right side of the ==-operator
 *
 * \retval true    Both iterators reference to the same list element or to nothing.
 * \retval false   Both iterators reference to different list elements.
 */
template <class T>
bool IntrusiveDList<T>::const_iterator::operator==(IntrusiveDList<T>::const_iterator const & rhv) const noexcept
{
  return (pItem == rhv.pItem);
}

/**
 * \brief Compares two iterators for not equal.
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
 * \param rhv   Value on the right side of the !=-operator
 *
 * \retval true    Both iterators reference to different list elements.
 * \retval false   Both iterators reference to the same list element or to nothing.
 */
template <class T>
bool IntrusiveDList<T>::const_iterator::operator!=(IntrusiveDList<T>::const_iterator const & rhv) const noexcept
{
  return (pItem != rhv.pItem);
}

/**
 * \brief Compares two iterators for equality.
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
 * \param rhv   Value on the right side of the ==-operator
 *
 * \retval true    Both iterators reference to the same list element or to nothing.
 * \retval false   Both iterators reference to different list elements.
 */
template <class T>
bool IntrusiveDList<T>::const_iterator::operator==(IntrusiveDList<T>::iterator const & rhv) const noexcept
{
  return (pItem == rhv.pItem);
}

/**
 * \brief Compares two iterators for not equal.
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
 * \param rhv   Value on the right side of the !=-operator
 *
 * \retval true    Both iterators reference to different list elements.
 * \retval false   Both iterators reference to the same list element or to nothing.
 */
template <class T>
bool IntrusiveDList<T>::const_iterator::operator!=(IntrusiveDList<T>::iterator const & rhv) const noexcept
{
  return (pItem != rhv.pItem);
}

/**
 * \brief Pre-increments the iterator.
 *
 * \pre   The iterator refers to a list element. It does not refer to past-the-end of the list.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Reference to self, _after_ incrementing.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator& IntrusiveDList<T>::const_iterator::operator++(void)
{
  if (pItem == nullptr)
    throw std::logic_error("Attempt to increment past-the-end iterator!");

  pItem = pItem->pNextInIntrusiveDList;
  return *this;
}

/**
 * \brief Post-increments the iterator.
 *
 * \pre   The iterator refers to a list element. It does not refer to past-the-end of the list.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Copy of the iterator, created _before_ incrementing.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator IntrusiveDList<T>::const_iterator::operator++(int)
{
  if (pItem == nullptr)
    throw std::logic_error("Attempt to increment past-the-end iterator!");

  const_iterator copy(*this);
  pItem = pItem->pNextInIntrusiveDList;
  return copy;
}

/**
 * \brief Dereference operator.
 *
 * \pre   The iterator refers to a list element. It does not refer to past-the-end of the list.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Unmodifiable reference to the list item.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator::reference IntrusiveDList<T>::const_iterator::operator*()
{
  if (pItem == nullptr)
    throw std::logic_error("Attempt to dereference past-the-end iterator!");

  return pItem;
}

/**
 * \brief Constructor. Creates an const_iterator referencing to a given list item or past-the-end of the list.
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
 * \param _pItem
 * Referenced list item.\n
 * nullptr = no element referenced (past-the-end iterator).
 */
template <class T>
IntrusiveDList<T>::const_iterator::const_iterator(T* const _pItem) noexcept
: pItem(_pItem)
{
};

// ==> class IntrusiveDList::const_iterator

// <== class IntrusiveDList

/**
 * \brief Constructor. Creates an empty list object.
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
template <class T>
IntrusiveDList<T>::IntrusiveDList(void) noexcept
: pFirst(nullptr)
, pLast(nullptr)
, nbOfItems(0U)
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
 * Another @ref IntrusiveDList instance, whose content shall be moved into the new constructed instance.\n
 * The other instance is left in empty state.
 */
template <class T>
IntrusiveDList<T>::IntrusiveDList(IntrusiveDList<T> && other) noexcept
: pFirst(other.pFirst)
, pLast(other.pLast)
, nbOfItems(other.nbOfItems)
{
  // leave the other list in empty state
  other.pFirst    = nullptr;
  other.pLast     = nullptr;
  other.nbOfItems = 0U;
}

/**
 * \brief Destructor.
 *
 * Any items still in the list will be removed from the list before destruction.
 *
 * \post   Any iterator retrieved from this container becomes invalid and must not be used any more.
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
template <class T>
IntrusiveDList<T>::~IntrusiveDList(void)
{
  clear();
}

/**
 * \brief Move-assignement operator.
 *
 * The content of the @ref IntrusiveDList instance on the right side of the assignment operator will be move-
 * assigned to this @ref IntrusiveDList instance.
 *
 * The previous content of this list instance will be removed from the list before move-assignment takes place.
 *
 * The other list instance is left in empty state after move-assignment has finished.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
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
 * Reference to the @ref IntrusiveDList instance on the right side of the assignement operator.
 *
 * \return
 * Reference to self.
 */
template <class T>
IntrusiveDList<T>& IntrusiveDList<T>::operator=(IntrusiveDList<T> && rhv) noexcept
{
  if (&rhv != this)
  {
    clear();

    pFirst    = rhv.pFirst;
    pLast     = rhv.pLast;
    nbOfItems = rhv.nbOfItems;

    rhv.pFirst    = nullptr;
    rhv.pLast     = nullptr;
    rhv.nbOfItems = 0U;
  }

  return *this;
}

/**
 * \brief Retrieves an iterator referencing the first (front) element in the list.
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
 * \return
 * Iterator referencing to the first (front) element in the list.\n
 * If the list is empty, then the end-iterator will be returned.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::begin(void) noexcept
{
  return iterator(pFirst);
}

/**
 * \brief Retrieves a const-iterator referencing the first (front) element in the list.
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
 * \return
 * Const-iterator referencing to the first (front) element in the list.\n
 * If the list is empty, then the end-iterator will be returned.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator IntrusiveDList<T>::begin(void) const noexcept
{
  return const_iterator(pFirst);
}

/**
 * \brief Retrieves a const-iterator referencing the first (front) element in the list.
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
 * \return
 * Const-iterator referencing to the first (front) element in the list.\n
 * If the list is empty, then the end-iterator will be returned.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator IntrusiveDList<T>::cbegin(void) const noexcept
{
  return const_iterator(pFirst);
}

/**
 * \brief Retrieves an iterator referencing to past-the-end of the list.
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
 * \return
 * Iterator referencing to past-the-end of the list.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::end(void) noexcept
{
  return iterator(nullptr);
}

/**
 * \brief Retrieves a const-iterator referencing to past-the-end of the list.
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
 * \return
 * Const-iterator referencing to past-the-end of the list.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator IntrusiveDList<T>::end(void) const noexcept
{
  return const_iterator(nullptr);
}

/**
 * \brief Retrieves a const-iterator referencing to past-the-end of the list.
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
 * \return
 * Const-iterator referencing to past-the-end of the list.
 */
template <class T>
typename IntrusiveDList<T>::const_iterator IntrusiveDList<T>::cend(void) const noexcept
{
  return const_iterator(nullptr);
}

/**
 * \brief Removes all items from the list.
 *
 * \post   The list is empty.
 *
 * \post   Any iterator retrieved from this container becomes invalid and must not be used any more.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
template <class T>
void IntrusiveDList<T>::clear(void) noexcept
{
  while (pFirst != nullptr)
  {
    auto const pItem = pFirst;
    pFirst = pFirst->pNextInIntrusiveDList;

    pItem->pPrevInIntrusiveDList = nullptr;
    pItem->pNextInIntrusiveDList = nullptr;
  }

  pLast = nullptr;
  nbOfItems = 0U;
}

/**
 * \brief Inserts an item into the list.
 *
 * \pre    The item is not enqueued in any other @ref IntrusiveDList yet.
 *
 * \post   All iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pos
 * Iterator in front of which the item shall be inserted.\n
 * If this is the post-the-end iterator, then the item will be appended to the end (back) of the list.
 *
 * \param pItem
 * Pointer to the item that shall be inserted into the list in front of the list item referenced by `pos`.\n
 * nullptr is not allowed.
 *
 * \return
 * Iterator referencing to the inserted item.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::insert(IntrusiveDList<T>::iterator pos, T* const pItem)
{
  return insert(const_iterator(pos), pItem);
}

/**
 * \brief Inserts an item into the list.
 *
 * \pre    The item is not enqueued in any other @ref IntrusiveDList yet.
 *
 * \post   All iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pos
 * Iterator in front of which the item shall be inserted.\n
 * If this is the post-the-end iterator, then the item will be appended to the end (back) of the list.
 *
 * \param pItem
 * Pointer to the item that shall be inserted into the list in front of the list item referenced by `pos`.\n
 * nullptr is not allowed.
 *
 * \return
 * Iterator referencing to the inserted item.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::insert(IntrusiveDList<T>::const_iterator pos, T* const pItem)
{
  if (pItem == nullptr)
    throw std::invalid_argument("IntrusiveDList<T>::insert: pItem is nullptr!");

  if ((pItem->pPrevInIntrusiveDList != nullptr) || (pItem->pNextInIntrusiveDList != nullptr))
    throw std::logic_error("IntrusiveDList<T>::insert: Item is already enqueued in a list!");

  if (pos == cend())
  {
    // special case: append to end
    push_back(pItem);
    return iterator(pLast);
  }
  else if (pos == cbegin())
  {
    // special case: insert at front
    push_front(pItem);
    return iterator(pFirst);
  }
  else
  {
    // insert

    pItem->pPrevInIntrusiveDList = (*pos)->pPrevInIntrusiveDList;
    pItem->pNextInIntrusiveDList = *pos;

    if (pItem->pPrevInIntrusiveDList != nullptr)
      pItem->pPrevInIntrusiveDList->pNextInIntrusiveDList = pItem;

    (*pos)->pPrevInIntrusiveDList = pItem;

    ++nbOfItems;

    return iterator(pItem);
  }
}

/**
 * \brief Removes an element from the list.
 *
 * \post   Any iterator referencing the removed element becomes invalid and must not be used any more.
 *         All other iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pos
 * Iterator referencing to the item that shall be removed.\n
 * The iterator must refer to a list item. The past-the-end iterator is not allowed.
 *
 * \return
 * Iterator to the element behind the erased element.\n
 * This will be the past-the-end iterator, if the last element is removed.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::erase(IntrusiveDList<T>::iterator pos)
{
  return erase(const_iterator(pos));
}

/**
 * \brief Removes an element from the list.
 *
 * \post   Any iterator referencing the removed element becomes invalid and must not be used any more.
 *         All other iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pos
 * Iterator referencing to the item that shall be removed.\n
 * The iterator must refer to a list item. The past-the-end iterator is not allowed.
 *
 * \return
 * Iterator to the element behind the erased element.\n
 * This will be the past-the-end iterator, if the last element is removed.
 */
template <class T>
typename IntrusiveDList<T>::iterator IntrusiveDList<T>::erase(IntrusiveDList<T>::const_iterator pos)
{
  if (pos == end())
    throw std::invalid_argument("IntrusiveDList<T>::erase: 'pos' is invalid'");

  if (nbOfItems == 0U)
    throw std::logic_error("IntrusiveDList<T>::erase: List is empty");

  if ((*pos)->pPrevInIntrusiveDList != nullptr)
    (*pos)->pPrevInIntrusiveDList->pNextInIntrusiveDList = (*pos)->pNextInIntrusiveDList;

  if ((*pos)->pNextInIntrusiveDList != nullptr)
    (*pos)->pNextInIntrusiveDList->pPrevInIntrusiveDList = (*pos)->pPrevInIntrusiveDList;

  T* const pItemBehindPos = (*pos)->pNextInIntrusiveDList;

  if (pFirst == *pos)
    pFirst = (*pos)->pNextInIntrusiveDList;

  if (pLast == *pos)
    pLast = (*pos)->pPrevInIntrusiveDList;

  (*pos)->pPrevInIntrusiveDList = nullptr;
  (*pos)->pNextInIntrusiveDList = nullptr;
  --nbOfItems;

  return iterator(pItemBehindPos);
}

/**
 * \brief Retrieves a pointer to the first (front) item in the list.
 *
 * \pre   The list is not empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Pointer to the first (front) item in the list.
 */
template <class T>
T* IntrusiveDList<T>::front(void) const
{
  if (pFirst == nullptr)
    throw std::logic_error("IntrusiveDList<T>::front: Container empty");

  return pFirst;
}

/**
 * \brief Retrieves a pointer to the last (back) item in the list.
 *
 * \pre   The list is not empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Pointer to the last (back) item in the list.
 */
template <class T>
T* IntrusiveDList<T>::back(void) const
{
  if (pFirst == nullptr)
    throw std::logic_error("IntrusiveDList<T>::back: Container empty");

  return pLast;
}

/**
 * \brief Adds an item to the end (back) of the list.
 *
 * \pre    The item is not enqueued in this or any other @ref IntrusiveDList yet.
 *
 * \post   All iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * This method will not throw an `std::bad_alloc`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pItem
 * Pointer to the item that shall be added to the end (back) of the list.\n
 * nullptr is not allowed.
 */
template <class T>
void IntrusiveDList<T>::push_back(T * const pItem)
{
  if (pItem == nullptr)
    throw std::invalid_argument("IntrusiveDList<T>::push_back: 'pItem' is nullptr!");

  if ((pItem->pPrevInIntrusiveDList != nullptr) || (pItem->pNextInIntrusiveDList != nullptr))
    throw std::logic_error("IntrusiveDList<T>::push_back: Item is already enqueued in a list!");

  if (pFirst == nullptr)
  {
    pFirst    = pItem;
    pLast     = pItem;
    nbOfItems = 1U;
  }
  else
  {
    pLast->pNextInIntrusiveDList = pItem;
    pItem->pPrevInIntrusiveDList = pLast;
    pLast = pItem;
    ++nbOfItems;
  }
}

/**
 * \brief Adds an item to the begin (front) of the list.
 *
 * \pre    The item is not enqueued in this or any other @ref IntrusiveDList yet.
 *
 * \post   All iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * This method will not throw an `std::bad_alloc`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pItem
 * Pointer to the item that shall be added to the begin (front) of the list.\n
 * nullptr is not allowed.
 */
template <class T>
void IntrusiveDList<T>::push_front(T * const pItem)
{
  if (pItem == nullptr)
    throw std::invalid_argument("IntrusiveDList<T>::push_front: 'pItem' is nullptr!");

  if ((pItem->pPrevInIntrusiveDList != nullptr) || (pItem->pNextInIntrusiveDList != nullptr))
    throw std::logic_error("IntrusiveDList<T>::push_front: Item is already enqueued in a list!");

  if (pFirst == nullptr)
  {
    pFirst    = pItem;
    pLast     = pItem;
    nbOfItems = 1U;
  }
  else
  {
    pItem->pNextInIntrusiveDList  = pFirst;
    pFirst->pPrevInIntrusiveDList = pItem;
    pFirst = pItem;
    ++nbOfItems;
  }
}

/**
 * \brief Removes an element from the end (back) of the list.
 *
 * \pre    The list is not empty.
 *
 * \post   The item is removed from the list. The item can be added to this or a different @ref IntrusiveDList
 *         instance.
 *
 * \post   Any iterator referencing the removed element becomes invalid and must not be used any more.
 *         All other iterators retrieved from this container remain valid.
 *
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
template <class T>
void IntrusiveDList<T>::pop_back(void)
{
  if (pLast == nullptr)
    throw std::logic_error("IntrusiveDList<T>::pop_back: List is empty!");

  if (pFirst == pLast)
  {
    pFirst    = nullptr;
    pLast     = nullptr;
    nbOfItems = 0U;
  }
  else
  {
    auto const poppedItem = pLast;
    pLast = pLast->pPrevInIntrusiveDList;
    pLast->pNextInIntrusiveDList = nullptr;

    poppedItem->pPrevInIntrusiveDList = nullptr;
    --nbOfItems;
  }
}

/**
 * \brief Removes an element from the begin (front) of the list.
 *
 * \pre    The list is not empty.
 *
 * \post   The item is removed from the list. The item can be added to this or a different @ref IntrusiveDList
 *         instance.
 *
 * \post   Any iterator referencing the removed element becomes invalid and must not be used any more.
 *         All other iterators retrieved from this container remain valid.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
template <class T>
void IntrusiveDList<T>::pop_front(void)
{
  if (pFirst == nullptr)
    throw std::logic_error("IntrusiveDList<T>::pop_front: List is empty!");

  if (pFirst == pLast)
  {
    pFirst    = nullptr;
    pLast     = nullptr;
    nbOfItems = 0U;
  }
  else
  {
    auto const poppedItem = pFirst;
    pFirst = pFirst->pNextInIntrusiveDList;
    pFirst->pPrevInIntrusiveDList = nullptr;

    poppedItem->pNextInIntrusiveDList = nullptr;
    --nbOfItems;
  }
}

/**
 * \brief Retrieves the number of items in the list.
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
 * \return
 * Number of items in the list.
 */
template <class T>
typename IntrusiveDList<T>::size_type IntrusiveDList<T>::size(void) const noexcept
{
  return nbOfItems;
}

/**
 * \brief Retrieves, if the list is empty or not.
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
 * \retval true   List is empty.
 * \retval false  List is not empty.
 */
template <class T>
bool IntrusiveDList<T>::empty(void) const noexcept
{
  return (nbOfItems == 0U);
}

/**
 * \brief Clears the list and destroys all items in the list.
 *
 * The list items are destroyed from back to front of the list.
 *
 * \post   The list is empty.
 *
 * \post   Any iterator retrieved from this container becomes invalid and must not be used any more.
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
 */
template <class T>
void IntrusiveDList<T>::ClearAndDestroyItems(void) noexcept
{
  while (!empty())
  {
    T* const pItem = back();
    pop_back(); // (only throws if list is empty)
    delete pItem;
  }
}

} // namespace container
} // namespace gpcc
