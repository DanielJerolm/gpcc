/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019, 2022 Daniel Jerolm

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

#ifndef INTRUSIVEDLIST_HPP_201907050822
#define INTRUSIVEDLIST_HPP_201907050822

#include <iterator>
#include <cstddef>

namespace gpcc      {
namespace container {

/**
 * \ingroup GPCC_CONTAINER
 * \brief Intrusive double-linked list.
 *
 * # Functionality
 * This class implements an intrusive double-linked list.\n
 * It offers an alternative to `std::list` if a potential `std::bad_alloc` cannot be handled or if performance is
 * crucial.
 *
 * The list items contained in a `IntrusiveDList<T>` are raw pointers to objects (`T*`). A `IntrusiveDList<T>` cannot
 * contain objects and it therefore cannot overtake ownership of objects. See section "Ownership" below.
 *
 * The class' API is based on class `std::list` from the STL. In many cases, this class may be a direct replacement
 * for type `std::list` if `std::list` contains _pointers to objects_:
 * ~~~{.cpp}
 * // STL list containing pointers to MyClass objects.
 * std::list<MyClass*> list;
 *
 * // The same, but using GPCC's intrusive list:
 * gpcc::container::IntrusiveDList<MyClass> intrusiveList;
 * ~~~
 *
 * The naming of the API does not meet the GPCC coding style.\n
 * This is by intention to maximize compatibility to the API of `std::list`.
 *
 * # Comparison with std::list
 * Using class @ref IntrusiveDList instead of `std::list` may offer the following advantages:
 * - No extra list node objects are required.
 * - There are no heap allocations during runtime.
 * - There will be no `std::bad_alloc` at runtime.
 * - Higher performance.
 *
 * On the other hand, there may be some disadvantages:
 * - List items must not be enqueued in more than _one_ @ref IntrusiveDList instance at any time.
 * - List items must provide two class members:\n
 *   A `pPrevInIntrusiveDList` and a `pNextInIntrusiveDList` pointer (see below). This may waste some memory if only
 *   a small number of objects of a specific type is enqueued in a @ref IntrusiveDList, but all objects have the
 *   pointer members.
 *
 * Even though this class' API is based on the API of `std::list`, there are some differences:
 * - `operator*` of the iterator does not allow to modify the list item. Remember: The list items are `T*`, not `T`.
 * - `front()` and `back()` of the list also do not allow to modify the list item. Remember: The list items are `T*`, not `T`.
 * - @ref IntrusiveDList cannot contain objects `T`, only pointers `T*` to objects `T`. The class does not take over
 *   ownership of the objects referenced by the pointers `T*`. See section "Ownership" below.
 * - Currently not all methods offered by class `std::list` may be provided by this.
 *
 * # Requirements for list items
 * A class `T` must meet the following requirements to allow instances of that class `T` to be enqueued in a
 * `IntrusiveDList<T>` instance:
 * - Class `T` must provide two attributes:\n
 *   `T* pPrevInIntrusiveDList`\n
 *   `T* pNextInIntrusiveDList`
 * - The pointers must be accessible for class `IntrusiveDList<T>`. This can be achieved by two approaches:
 *   + make them public (not recommended).
 *   + make them private and add a friend class `IntrusiveDList<T>` (recommended).
 * - All constructors of class `T` (incl. copy- and move-constructors) shall initialize the `pPrevInIntrusiveDList`
 *   and `pNextInIntrusiveDList` pointers with `nullptr`.
 * - The destructor of class `T` shall check if `pPrevInIntrusiveDList` and `pNextInIntrusiveDList` are both `nullptr`
 *   and call @ref gpcc::osal::Panic() if any of the pointers is not `nullptr`.
 * - The copy- and move-assignment operators of class `T` shall not modify the `pPrevInIntrusiveDList` and
 *   `pNextInIntrusiveDList` pointers of the target instance.
 *
 * Example:
 * ~~~{.cpp}
 * class Item
 * {
 *   friend class IntrusiveDList<Item>;
 *
 *   public:
 *     uint32_t value;
 *
 *     inline Item(void) : value(0U), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
 *     inline explicit Item(uint32_t const _value) : value(_value), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
 *     inline Item(Item const & other) : value(other.value), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
 *     inline Item(Item const && other) noexcept : value(other.value), pPrevInIntrusiveDList(nullptr), pNextInIntrusiveDList(nullptr) {};
 *
 *     inline ~Item(void)
 *     {
 *       if ((pPrevInIntrusiveDList != nullptr) || (pNextInIntrusiveDList != nullptr))
 *         gpcc::osal::Panic("Item::~Item: Still referenced by IntrusiveDList!");
 *     }
 *
 *     inline Item& operator=(Item const & rhv) noexcept
 *     {
 *       value = rhv.value;
 *       return *this;
 *     }
 *
 *     inline Item& operator=(Item && rhv) noexcept
 *     {
 *       value = rhv.value;
 *       return *this;
 *     }
 *
 *   private:
 *     // Pointers used to enqueue instances of this class in IntrusiveDList<Item>
 *     Item* pPrevInIntrusiveDList;
 *     Item* pNextInIntrusiveDList;
 * };
 * ~~~
 *
 * # Ownership
 * The list items contained in a `IntrusiveDList<T>` are raw pointers to objects (`T*`). A `IntrusiveDList<T>` cannot
 * contain objects and it therefore cannot overtake ownership of objects.
 *
 * If the owner of the `IntrusiveDList<T>` uses the list to organize objects he or she _owns_, than the owner of the
 * `IntrusiveDList<T>` has to ensure that the items organized in the `IntrusiveDList<T>` are finally released. He or
 * she may use the method @ref ClearAndDestroyItems() provided by class `IntrusiveDList<T>` to clear the list and
 * release all items.
 *
 * - - -
 *
 * \tparam T
 * Data type of the items. Note that the type of the list items will be `T*`, __not__ `T`.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
template <class T>
class IntrusiveDList final
{
  public:
    /// Definition of type for number of elements.
    using size_type = size_t;

    class const_iterator;

    /// Iterator for the @ref IntrusiveDList container.
    /** Basically it is the same as @ref const_iterator, because class @ref IntrusiveDList does not
        allow to modify list items (T*). */
    class iterator final
    {
      friend class IntrusiveDList;
      friend class const_iterator;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T*;
        using difference_type   = void;
        using pointer           = void;
        using reference         = T* const &;

        iterator(void) noexcept;
        iterator(iterator const &) noexcept = default;
        iterator(iterator &&) noexcept = default;
        ~iterator(void) = default;

        iterator& operator=(iterator const &) noexcept = default;
        iterator& operator=(iterator &&) noexcept = default;

        bool operator==(iterator const & rhv) const noexcept;
        bool operator!=(iterator const & rhv) const noexcept;
        bool operator==(const_iterator const & rhv) const noexcept;
        bool operator!=(const_iterator const & rhv) const noexcept;

        iterator& operator++(void);
        iterator operator++(int);

        reference operator*(void);

      private:
        /// Currently referenced list item. nullptr = none (past-the-end iterator).
        T* pItem;

        explicit iterator(T* const _pItem) noexcept;
    };

    /// Const-iterator for the @ref IntrusiveDList container.
    class const_iterator final
    {
      friend class IntrusiveDList;
      friend class iterator;

      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T*;
        using difference_type   = void;
        using pointer           = void;
        using reference         = T* const &;

        const_iterator(void) noexcept;
        explicit const_iterator(iterator const & other) noexcept;
        explicit const_iterator(iterator && other) noexcept;
        const_iterator(const_iterator const &) noexcept = default;
        const_iterator(const_iterator &&) noexcept = default;
        ~const_iterator(void) = default;

        const_iterator& operator=(const_iterator const &) noexcept = default;
        const_iterator& operator=(const_iterator &&) noexcept = default;
        const_iterator& operator=(iterator const & rhv) noexcept;
        const_iterator& operator=(iterator && rhv) noexcept;

        bool operator==(const_iterator const & rhv) const noexcept;
        bool operator!=(const_iterator const & rhv) const noexcept;
        bool operator==(iterator const & rhv) const noexcept;
        bool operator!=(iterator const & rhv) const noexcept;

        const_iterator& operator++(void);
        const_iterator operator++(int);

        reference operator*(void);

      private:
        /// Currently referenced list item. nullptr = none (past-the-end iterator).
        T* pItem;

        explicit const_iterator(T* const _pItem) noexcept;
      };


    IntrusiveDList(void) noexcept;
    IntrusiveDList(IntrusiveDList const &) = delete;
    IntrusiveDList(IntrusiveDList && other) noexcept;
    ~IntrusiveDList(void);

    IntrusiveDList& operator=(IntrusiveDList const &) = delete;
    IntrusiveDList& operator=(IntrusiveDList && rhv) noexcept;

    iterator begin(void) noexcept;
    const_iterator begin(void) const noexcept;
    const_iterator cbegin(void) const noexcept;
    iterator end(void) noexcept;
    const_iterator end(void) const noexcept;
    const_iterator cend(void) const noexcept;

    void clear(void) noexcept;

    iterator insert(iterator pos, T* const pItem);
    iterator insert(const_iterator pos, T* const pItem);

    iterator erase(iterator pos);
    iterator erase(const_iterator pos);

    T* front(void) const;
    T* back(void) const;

    void push_back(T * const pItem);
    void push_front(T * const pItem);
    void pop_back(void);
    void pop_front(void);

    size_type size(void) const noexcept;
    bool empty(void) const noexcept;

    // <== Additional functionality, which is NOT compatible to std::list
    void ClearAndDestroyItems(void) noexcept;
    // ==>

  private:
    /// Pointer to the list item at the front. nullptr = empty list.
    T* pFirst;

    /// Pointer to the list item at the back. nullptr = empty list.
    T* pLast;

    /// Number of items in the list.
    size_t nbOfItems;
};

} // namespace container
} // namespace gpcc

#include "IntrusiveDList.tcc"

#endif
