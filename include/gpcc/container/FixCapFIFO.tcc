/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include "FixCapFIFO.hpp"
#include <limits>
#include <stdexcept>
#include <cstring>

namespace gpcc      {
namespace container {

/**
 * \brief Constructor. Creates an empty FIFO with given capacity.
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
 * \param capacity
 * Desired capacity for the FIFO. The value must fit into the type specified by `SIZET`.\n
 * Zero is not allowed.
 */
template <typename T, typename SIZET>
FixCapFIFO<T, SIZET>::FixCapFIFO(size_t const capacity)
: spMemory_()
, capacity_(capacity)
, size_(0U)
, wrIndex_(0U)
, rdIndex_(0U)
{
  static_assert(   (std::is_trivially_default_constructible<T>::value == true)
                && (std::is_nothrow_copy_constructible<T>::value == true)
                && (std::is_nothrow_move_constructible<T>::value == true)
                && (std::is_trivially_copy_assignable<T>::value == true)
                && (std::is_trivially_copyable<T>::value == true)
                && (std::is_nothrow_move_assignable<T>::value == true),
                "T must be a simple data type that can be trivially copied and moved with noexcept guarantee.");

  static_assert(   (std::is_integral_v<SIZET> == true)
                && (std::is_signed_v<SIZET> == false)
                && (sizeof(SIZET) <= sizeof(size_t)),
                "SIZET must be an unsigned integral type. Its size must be equal to or less than 'size_t'.");

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if (   (capacity == 0U)
      || (capacity > std::numeric_limits<SIZET>::max()))
  {
    throw std::invalid_argument("Invalid args");
  }
  #pragma GCC diagnostic pop

  spMemory_.reset(new T[capacity]);
}

/**
 * \brief Copy constructor. Creates a copy of another FIFO instance.
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
 * \param other
 * The other FIFO instance that shall be copied.
 */
template <typename T, typename SIZET>
FixCapFIFO<T, SIZET>::FixCapFIFO(FixCapFIFO<T, SIZET> const & other)
: spMemory_(new T[other.capacity_])
, capacity_(other.capacity_)
, size_(other.size_)
, wrIndex_((size_ == capacity_) ? 0U : size_)
, rdIndex_(0U)
{
  CopyFromOther(other);
}

/**
 * \brief Copy-assigns the content and the capacity of another FIFO instance to this instance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee [if capacity of @p rhv is equal to or less than this FIFO's capacity]\n
 * Strong guarantee [otherwise]
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * The other FIFO instance.
 *
 * \return
 * Reference to self.
 */
template <typename T, typename SIZET>
FixCapFIFO<T, SIZET>& FixCapFIFO<T, SIZET>::operator=(FixCapFIFO<T, SIZET> const & rhv)
{
  if (&rhv != this)
  {
    // set this FIFO's capacity to the capacity of "rhv" by means of reallocation or by simply shrinking "capacity_"
    if (capacity_ < rhv.capacity_)
    {
      T* const pNewStorage = new T[rhv.capacity_];

      capacity_ = rhv.capacity_;
      spMemory_.reset(pNewStorage);
    }
    else if (capacity_ > rhv.capacity_)
    {
      capacity_ = rhv.capacity_;
    }

    size_    = rhv.size_;
    wrIndex_ = (size_ == capacity_) ? 0U : size_;
    rdIndex_ = 0U;
    CopyFromOther(rhv);
  }

  return *this;
}

/**
 * \brief Move-assigns the content of another FIFO instance to this instance.
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
 * The other FIFO instance whose content shall be move-assigned to this instance.\n
 * The other FIFO instance is left in empty state. Its capacity is valid, but undefined.
 *
 * \return
 * Reference to self.
 */
template <typename T, typename SIZET>
FixCapFIFO<T, SIZET>& FixCapFIFO<T, SIZET>::operator=(FixCapFIFO<T, SIZET> && rhv) noexcept
{
  if (&rhv != this)
  {
    std::swap(spMemory_, rhv.spMemory_);
    std::swap(capacity_, rhv.capacity_);

    size_    = rhv.size_;
    wrIndex_ = rhv.wrIndex_;
    rdIndex_ = rhv.rdIndex_;

    rhv.size_ = 0U;
    rhv.wrIndex_ = 0U;
    rhv.rdIndex_ = 0U;
  }

  return *this;
}

/**
 * \brief Queries the capacity of the FIFO.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Capacity of the FIFO.
 */
template <typename T, typename SIZET>
size_t FixCapFIFO<T, SIZET>::Capacity(void) const noexcept
{
  return capacity_;
}

/**
 * \brief Returns the number of elements currently stored in the FIFO.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Number of elements currently stored in the FIFO.
 */
template <typename T, typename SIZET>
size_t FixCapFIFO<T, SIZET>::Size(void) const noexcept
{
  return size_;
}

/**
 * \brief Queries if the FIFO is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   The FIFO is empty.
 * \retval false  The FIFO contains at least one element.
 */
template <typename T, typename SIZET>
bool FixCapFIFO<T, SIZET>::IsEmpty(void) const noexcept
{
  return (size_ == 0U);
}

/**
 * \brief Queries if the FIFO is full.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   The FIFO is full.
 * \retval false  The FIFO is not full. At least one element could be pushed onto the FIFO.
 */
template <typename T, typename SIZET>
bool FixCapFIFO<T, SIZET>::IsFull(void) const noexcept
{
  return (size_ == capacity_);
}

/**
 * \brief Flushes the FIFO.
 *
 * \post  The FIFO is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
template <typename T, typename SIZET>
void FixCapFIFO<T, SIZET>::Clear(void) noexcept
{
  size_    = 0U;
  rdIndex_ = 0U;
  wrIndex_ = 0U;
}

/**
 * \brief Pushes one element onto the FIFO.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * Element that shall be pushed onto the FIFO.
 *
 * \retval true   Success.
 * \retval false  FIFO is full. @p value has __not__ been pushed onto the FIFO.
 */
template <typename T, typename SIZET>
bool FixCapFIFO<T, SIZET>::Push(T const value) noexcept
{
  if (IsFull())
    return false;

  spMemory_[wrIndex_++] = value;

  if (wrIndex_ == capacity_)
    wrIndex_ = 0U;

  size_++;

  return true;
}

/**
 * \brief Pops one item from the FIFO.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * The popped item is written into the referenced value.
 *
 * \retval true   Success.
 * \retval false  FIFO is empty. The variable referenced by @p value has not been modified.
 */
template <typename T, typename SIZET>
bool FixCapFIFO<T, SIZET>::Pop(T & value) noexcept
{
  if (IsEmpty())
    return false;

  value = spMemory_[rdIndex_++];

  if (rdIndex_ == capacity_)
    rdIndex_ = 0U;

  size_--;

  return true;
}

/**
 * \brief Pushes one or more elements onto the FIFO.
 *
 * This method handles a full FIFO or insufficient free space gracefully.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pSrc
 * Pointer to a buffer containing the data that shall be pushed onto the FIFO.
 *
 * \param n
 * Number of elements, that shall be pushed onto the FIFO. Zero is allowed.
 *
 * \return
 * Number of elements pushed onto the FIFO. This may be less than @p n, if the number of free slots in the FIFO was
 * less than @p n.
 */
template <typename T, typename SIZET>
size_t FixCapFIFO<T, SIZET>::PushMultiple(T const * const pSrc, size_t const n) noexcept
{
  if (n == 0U)
    return 0U;

  // calculate number of free slots and number of elements to be pushed
  size_t const free = capacity_ - size_;
  size_t const tbp = std::min(free, n);

  if (wrIndex_ < rdIndex_)
  {
    memcpy(spMemory_.get() + wrIndex_, pSrc, tbp * sizeof(T));
    wrIndex_ += tbp;
    size_ += tbp;
  }
  else
  {
    // calculate number of elements that can be pushed until the write-index wraps around
    size_t const untilWrap = capacity_ - wrIndex_;

    if (untilWrap >= tbp)
    {
      memcpy(spMemory_.get() + wrIndex_, pSrc, tbp * sizeof(T));
      wrIndex_ += tbp;
      if (wrIndex_ == capacity_)
        wrIndex_ = 0U;
      size_ += tbp;
    }
    else
    {
      size_ += tbp;

      memcpy(spMemory_.get() + wrIndex_, pSrc, untilWrap * sizeof(T));

      size_t const rest = tbp - untilWrap;
      memcpy(spMemory_.get(), pSrc + untilWrap, rest * sizeof(T));
      wrIndex_ = rest;
    }
  }

  return tbp;
}

/**
 * \brief Pops one or more elements from the FIFO.
 *
 * This method handles an empty FIFO or an insufficient number of elements in the FIFO gracefully.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.\n
 * This can be invoked from interrupt context if the caller applies proper synchronization.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pDest
 * The elements popped from the FIFO are written into the buffer referenced by this.
 *
 * \param n
 * Number of elements, that shall be popped from the FIFO. Zero is allowed.
 *
 * \return
 * Number of elements popped from the FIFO. This may be less than @p n, if the number of elements in the FIFO was less
 * than @p n.
 */
template <typename T, typename SIZET>
size_t FixCapFIFO<T, SIZET>::PopMultiple(T* const pDest, size_t const n) noexcept
{
  if (n == 0U)
    return 0U;

  // calculate number of element that shall be popped from the FIFO
  size_t const tbp = std::min(static_cast<size_t>(size_), n);

  if (rdIndex_ < wrIndex_)
  {
    memcpy(pDest, spMemory_.get() + rdIndex_, tbp * sizeof(T));
    rdIndex_ += tbp;
    size_ -= tbp;
  }
  else
  {
    // calculate number of elements that can be popped until the read-index wraps around
    size_t const untilWrap = capacity_ - rdIndex_;

    if (untilWrap >= tbp)
    {
      memcpy(pDest, spMemory_.get() + rdIndex_, tbp * sizeof(T));
      rdIndex_ += tbp;
      if (rdIndex_ == capacity_)
        rdIndex_ = 0U;
      size_ -= tbp;
    }
    else
    {
      size_ -= tbp;

      memcpy(pDest, spMemory_.get() + rdIndex_, untilWrap * sizeof(T));

      size_t const rest = tbp - untilWrap;
      memcpy(pDest + untilWrap, spMemory_.get(), rest * sizeof(T));
      rdIndex_ = rest;
    }
  }

  return tbp;
}

/**
 * \brief Copies the data of another FIFO into this.
 *
 * This is used during copy-construction and copy-assignment.
 *
 * Regardles of the current value of `other.rdIndex`, other's data is copied into this FIFO starting at
 * `this->spMemory_[0]`.
 *
 * \pre   The caller must ensure the following:
 *        - `this->size_ == other.size_`
 *        - `this->rdIndex_ == 0`
 *        - `this->wrIndex_ == this->size_ % this->capacity_`
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
 * \param other
 * The other FIFO instance whose content shall be copied into this instance.
 */
template <typename T, typename SIZET>
void FixCapFIFO<T, SIZET>::CopyFromOther(FixCapFIFO const & other) noexcept
{
  if (size_ != 0U)
  {
    if (other.rdIndex_ < other.wrIndex_)
    {
      // not full, no wrap: |     xxxx |
      //                       -->    <-- size_
      memcpy(spMemory_.get(), other.spMemory_.get() + other.rdIndex_, size_ * sizeof(T));
    }
    else
    {
      // Three cases:
      // a) Not full and wrap:
      // |xxx    xxx|
      //  <b>    <a>
      //
      // b) Full and wrap, other.rdIndex != 0
      // |xxxxxxxxxx|
      //  <--b-><a->
      //
      // c) Full and wrap, other.rdIndex == 0
      // |xxxxxxxxxx|
      //  <--a----->  (b = 0)

      size_t const a = (other.capacity_ - other.rdIndex_);
      memcpy(spMemory_.get(), other.spMemory_.get() + other.rdIndex_, a * sizeof(T));

      size_t const b = size_ - a;
      if (b != 0U)
        memcpy(spMemory_.get() + a, other.spMemory_.get(), b * sizeof(T));
    }
  }
}

} // namespace container
} // namespace gpcc
