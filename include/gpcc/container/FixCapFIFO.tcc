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

  static_assert(std::atomic<SIZET>::is_always_lock_free,
                "std::atomic must be lock-free for SIZET, but it is not");

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
, size_(other.size_.load(std::memory_order_relaxed))
, wrIndex_((size_.load(std::memory_order_relaxed) == capacity_) ? 0U : size_.load(std::memory_order_relaxed))
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

    size_.store(rhv.size_.load(std::memory_order_relaxed), std::memory_order_relaxed);
    wrIndex_ = (size_.load(std::memory_order_relaxed) == capacity_) ? 0U : size_.load(std::memory_order_relaxed);
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

    size_.store(rhv.size_.load(std::memory_order_relaxed), std::memory_order_relaxed);
    wrIndex_ = rhv.wrIndex_;
    rdIndex_ = rhv.rdIndex_;

    rhv.size_.store(0U, std::memory_order_relaxed);
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
 * This can be invoked from thread and interrupt context.
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
 * This can be invoked from thread and interrupt context.\n
 * This method can be used in conjunction with @ref Push(), @ref PushMultiple(), @ref Pop(), and @ref PopMultiple()
 * without external synchronization. The return value will be valid, but may be outdated immediately. Note that an
 * outdated value is not harmful if this is invoked by the producer (consumer) before pushing (popping) elements.
 * If an outdated value is not acceptable, then the caller must apply external synchronization.
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
  return size_.load(std::memory_order_relaxed);
}

/**
 * \brief Queries if the FIFO is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This can be invoked from thread and interrupt context if the caller applies proper synchronization.\n
 * As an exception, a consumer may omit external synchronization measures, if false positives are acceptable (method
 * returns `true`, although the FIFO is not empty).
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
  return (size_.load(std::memory_order_relaxed) == 0U);
}

/**
 * \brief Queries if the FIFO is full.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * This can be invoked from thread and interrupt context if the caller applies proper synchronization.\n
 * As an exception, a producer may omit external synchronization measures, if false positives are acceptable (method
 * returns `true`, although the FIFO is not full).
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
  return (size_.load(std::memory_order_relaxed) == capacity_);
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
 * This can be invoked from thread and interrupt context.
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
  size_.store(0U, std::memory_order_relaxed);
  rdIndex_ = 0U;
  wrIndex_ = 0U;
}

/**
 * \brief Pushes one element onto the FIFO.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe, except for the following methods:\n
 * - @ref Pop()
 * - @ref PopMultiple()
 *
 * This can be invoked from thread and interrupt context.
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
  if (size_.load(std::memory_order_acquire) == capacity_)
    return false;

  spMemory_[wrIndex_++] = value;

  if (wrIndex_ == capacity_)
    wrIndex_ = 0U;

  (void)size_.fetch_add(1U, std::memory_order_release);

  return true;
}

/**
 * \brief Pops one item from the FIFO.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe, except for the following methods:\n
 * - @ref Push()
 * - @ref PushMultiple()
 *
 * This can be invoked from thread and interrupt context.
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
  if (size_.load(std::memory_order_acquire) == 0U)
    return false;

  value = spMemory_[rdIndex_++];

  if (rdIndex_ == capacity_)
    rdIndex_ = 0U;

  (void)size_.fetch_sub(1U, std::memory_order_release);

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
 * The state of the object is modified. Any concurrent accesses are not safe, except for the following methods:\n
 * - @ref Pop()
 * - @ref PopMultiple()
 *
 * This can be invoked from thread and interrupt context.
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

  // Fetch current number of items in the FIFO. "size_" might be decreased by a pop-operation during this function,
  // but that's not harmful because it increases the number of free slots.
  size_t const used = size_.load(std::memory_order_acquire);

  // calculate number of free slots and number of elements to be pushed
  size_t const free = capacity_ - used;
  size_t const tbp = std::min(free, n);

  // calculate number of elements that can be pushed until the write-index wraps around
  size_t const untilWrap = capacity_ - wrIndex_;

  if (tbp <= untilWrap)
  {
    memcpy(spMemory_.get() + wrIndex_, pSrc, tbp * sizeof(T));
    wrIndex_ += tbp;
    if (wrIndex_ == capacity_)
      wrIndex_ = 0U;
  }
  else
  {
    memcpy(spMemory_.get() + wrIndex_, pSrc, untilWrap * sizeof(T));

    size_t const rest = tbp - untilWrap;
    memcpy(spMemory_.get(), pSrc + untilWrap, rest * sizeof(T));
    wrIndex_ = rest;
  }

  (void)size_.fetch_add(tbp, std::memory_order_release);
  return tbp;
}

/**
 * \brief Pops one or more elements from the FIFO.
 *
 * This method handles an empty FIFO or a too small number of elements in the FIFO gracefully.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe, except for the following methods:\n
 * - @ref Push()
 * - @ref PushMultiple()
 *
 * This can be invoked from thread and interrupt context.
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

  // Fetch current number of items in the FIFO. "size_" might be increased by a push-operation during this function,
  // but that's not harmful, because it increases the number of available items.
  size_t const used = size_.load(std::memory_order_acquire);

  // calculate number of element that shall be popped from the FIFO
  size_t const tbp = std::min(used, n);

  // calculate number of elements that can be popped until the read-index wraps around
  size_t const untilWrap = capacity_ - rdIndex_;

  if (tbp <= untilWrap)
  {
    memcpy(pDest, spMemory_.get() + rdIndex_, tbp * sizeof(T));
    rdIndex_ += tbp;
    if (rdIndex_ == capacity_)
      rdIndex_ = 0U;
  }
  else
  {
    memcpy(pDest, spMemory_.get() + rdIndex_, untilWrap * sizeof(T));

    size_t const rest = tbp - untilWrap;
    memcpy(pDest + untilWrap, spMemory_.get(), rest * sizeof(T));
    rdIndex_ = rest;
  }

  (void)size_.fetch_sub(tbp, std::memory_order_release);
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
  if (size_.load(std::memory_order_relaxed) != 0U)
  {
    if (other.rdIndex_ < other.wrIndex_)
    {
      // not full, no wrap: |     xxxx |
      //                       -->    <-- size_
      memcpy(spMemory_.get(), other.spMemory_.get() + other.rdIndex_, size_.load(std::memory_order_relaxed) * sizeof(T));
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

      size_t const b = size_.load(std::memory_order_relaxed) - a;
      if (b != 0U)
        memcpy(spMemory_.get() + a, other.spMemory_.get(), b * sizeof(T));
    }
  }
}

} // namespace container
} // namespace gpcc
