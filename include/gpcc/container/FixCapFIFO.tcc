/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include "FixCapFIFO.hpp"
#include <gpcc/math/checks.hpp>
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
 * Desired capacity for the FIFO.\n
 * This must be a power of 2. Zero is not allowed.
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

  if (   (capacity == 0U)
      || (capacity > std::numeric_limits<SIZET>::max())
      || (!gpcc::math::IsPowerOf2(capacity)))
  {
    throw std::invalid_argument("Invalid args");
  }

  spMemory_.reset(new T[capacity]);
}

/**
 * \brief Copy constructor. Creates a deep copy of another FIFO instance.
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
, wrIndex_(other.wrIndex_)
, rdIndex_(other.rdIndex_)
{
  CopyFromOther(other.spMemory_.get());
}

/**
 * \brief Copy-assigns the content of another FIFO instance to this instance.
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
 * \param rhv
 * The other FIFO instance.\n
 * Both FIFOs must have the same capacity.
 *
 * \return
 * Reference to self.
 */
template <typename T, typename SIZET>
FixCapFIFO<T, SIZET>& FixCapFIFO<T, SIZET>::operator=(FixCapFIFO<T, SIZET> const & rhv)
{
  if (&rhv != this)
  {
    if (capacity_ != rhv.capacity_)
      throw std::logic_error("Different capacity");

    size_ = rhv.size_;
    wrIndex_ = rhv.wrIndex_;
    rdIndex_ = rhv.rdIndex_;
    CopyFromOther(rhv.spMemory_.get());
  }

  return *this;
}

/**
 * \brief Move-assigns the content of another FIFO instance to this instance.
 *
 * \note  The capacity of the FIFO may be reduced or enlarged.
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
  size_ = 0U;
  rdIndex_ = 0U;
  wrIndex_ = 0U;
}

/**
 * \brief Pushes one element onto the FIFO.
 *
 * \pre   The FIFO is not full.\n
 *        The precondition is not checked. If it is violated, then undefined behaviour will occur.\n
 *        Use @ref IsFull() or @ref Size() in conjunction with this method.
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
 */
template <typename T, typename SIZET>
void FixCapFIFO<T, SIZET>::UnsafePush(T const value) noexcept
{
  spMemory_[wrIndex_] = value;
  wrIndex_ = (wrIndex_ + 1U) & (~capacity_);
  size_++;
}

/**
 * \brief Pops one item from the FIFO.
 *
 * \pre   The FIFO is not empty.\n
 *        The precondition is not checked. If it is violated, then undefined behaviour will occur.\n
 *        Use @ref IsEmpty() or @ref Size() in conjunction with this method.
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
 * \return
 * Element popped from the FIFO.
 */
template <typename T, typename SIZET>
T FixCapFIFO<T, SIZET>::UnsafePop(void) noexcept
{
  size_--;
  auto const prevRdIdx = rdIndex_;
  rdIndex_ = (rdIndex_ + 1U) & (~capacity_);
  return spMemory_[prevRdIdx];
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
  SIZET const free = capacity_ - size_;
  size_t const tbp = std::min(static_cast<size_t>(free), n);

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
      wrIndex_ = (wrIndex_ + tbp) & (capacity_ - 1U);
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
      rdIndex_ = (rdIndex_ + tbp) & (capacity_ - 1U);
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
 * This is used during copy-construction and copy-assignment. The caller must ensure, that all members of the two FIFOs
 * (of course except for @ref spMemory_ and its content) are the same.
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
 * \param pOtherData
 * Pointer to the memory of the other FIFO. Data is copied from that location into this FIFO's storage according to
 * @ref size_, @ref rdIndex_ and @ref wrIndex_.
 */
template <typename T, typename SIZET>
void FixCapFIFO<T, SIZET>::CopyFromOther(T const * const pOtherData) noexcept
{
  if (size_ != 0U)
  {
    if (rdIndex_ == wrIndex_)
    {
      // full: |xxxxxxxxxx|
      //        <-size_-->
      memcpy(spMemory_.get(), pOtherData, size_ * sizeof(T));
    }
    else if (rdIndex_ < wrIndex_)
    {
      // not full, no wrap: |     xxxx |
      //                       -->    <-- size_t
      memcpy(spMemory_.get() + rdIndex_, pOtherData + rdIndex_, size_ * sizeof(T));
    }
    else
    {
      // not full, wrap: |xxx    xxx|
      //                  <b>    <a>
      SIZET const a = (capacity_ - rdIndex_);
      memcpy(spMemory_.get() + rdIndex_, pOtherData + rdIndex_, a * sizeof(T));

      SIZET const b = size_ - a;
      if (b != 0U)
        memcpy(spMemory_.get(), pOtherData, b * sizeof(T));
    }
  }
}

} // namespace container
} // namespace gpcc
