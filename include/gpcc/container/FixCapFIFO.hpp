/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#ifndef FIXCAPFIFO_HPP_202509081955
#define FIXCAPFIFO_HPP_202509081955

#include <atomic>
#include <memory>
#include <cstddef>

namespace gpcc      {
namespace container {

/**
 * \ingroup GPCC_CONTAINER
 * \brief FIFO with fixed capacity, usable from both thread and interrupt context.
 *
 * # Purpose
 * This class implements a light-weight FIFO with fixed capacity. It is designed for usage from both thread and
 * interrupt context. The capacity is fixed in terms that push-operations will not increase the capacity when the FIFO
 * becomes full. However, copy- and move-assignment may change the FIFO's capacity.
 *
 * # Build-in Synchronization
 * The push- and pop-methods internally use atomics. The FIFO can be used to transfer data from a producer in interrupt
 * or thread context to an consumer in a different thread or interrupt context without the need for external
 * synchronization measures.
 *
 * However, the user __must__ apply proper external locking/synchronization if:
 * - Methods other than @ref Push(), @ref Pop(), @ref PushMultiple(), or @ref PopMultiple() are used from different
 *   contexts.
 * - More than one producer uses @ref Push() and/or @ref PushMultiple()
 * - More than one consumer uses @ref Pop() and/or @ref PopMultiple()
 *
 * If only thread context is used, then locking measures may incorporate an @ref gpcc::osal::Mutex for instance. If
 * interrupt context is involved, then the user has to apply locking mechanisms for synchronization of thread and
 * interrupt context provided by the specific operating system.
 *
 * Please refer to the thread-safety notes of each method to determine which methods of the FIFO can be used from
 * interrupt context and if external synchronization is required.
 *
 * # Use in interrupt context
 * The following measures were applied to methods that can be invoked from interrupt context:
 * - No dynamic memory allocation.
 * - No exceptions are thrown, not even under the hood.
 * - FIFO overflow/underflow awareness and report by return value.
 * - No use of functions from the C/C++ runtime that are unavailable or incompatible with interrupt context.\n
 *   `memcpy()` is considered available and compatible with interrupt context.
 *
 * # Differentiation from STL
 * The STL offers `std::queue`. For use cases that do not involve interrupt context, you should prefer `std::queue` for
 * the following reasons:
 * - No limitation on the data type of the elements stored in the FIFO.
 * - Rich API.
 *
 * However, if interrupt context is involved, `std::queue` must not be used and @ref FixCapFIFO is an alternative.
 *
 * - - -
 *
 * \tparam T
 * Data type of the items stored in the FIFO.\n
 * The type is limited to trivial types such as PODs (plain old data types).
 *
 * \param SIZET
 * Data type used _internally_ for the FIFO's capacity, size, and indexing. The default is `size_t`, which gives best
 * performance. However, on size-constrained systems, a smaller datatype (e.g. `uint8_t`) can be selected to reduce the
 * RAM footprint of an FIFO object.\n
 * Regardless of `SIZET`, the _public API_ of the FIFO always uses `size_t` to express capacity and size values.
 *
 * - - -
 *
 * __Thread safety:__\n
 * - Not thread-safe. If used from different contexts, then the user has to apply proper locking.\n
 *   As an exception, push- and pop-operations use atomics and can be invoked from different contexts.\n
 * - Selected member functions can be invoked from interrupt context.\n
 * - Pay attention to method's thread-safety notes.
 */
template <typename T, typename SIZET = size_t>
class FixCapFIFO final
{
  public:
    FixCapFIFO(void) = delete;
    FixCapFIFO(size_t const capacity);
    FixCapFIFO(FixCapFIFO const & other);
    FixCapFIFO(FixCapFIFO && other) = delete;
    ~FixCapFIFO(void) = default;

    FixCapFIFO& operator=(FixCapFIFO const & rhv);
    FixCapFIFO& operator=(FixCapFIFO && rhv) noexcept;

    size_t Capacity(void) const noexcept;
    size_t Size(void) const noexcept;

    bool IsEmpty(void) const noexcept;
    bool IsFull(void) const noexcept;

    void Clear(void) noexcept;

    bool Push(T const value) noexcept;
    bool Pop(T & value) noexcept;

    size_t PushMultiple(T const * const pSrc, size_t const n) noexcept;
    size_t PopMultiple(T* const pDest, size_t const n) noexcept;

  private:
    /// Memory used by the FIFO to store elements of type T.
    std::unique_ptr<T[]> spMemory_;

    /// Capacity of the FIFO.
    SIZET capacity_;

    /// Number of items currently stored in the FIFO.
    std::atomic<SIZET> size_;

    /// Index of next element to be written in @ref spMemory_.
    SIZET wrIndex_;

    /// Index of next element to be read in @ref spMemory_.
    SIZET rdIndex_;


    void CopyFromOther(FixCapFIFO const & other) noexcept;
};

} // namespace container
} // namespace gpcc

#include "FixCapFIFO.tcc"

#endif // FIXCAPFIFO_HPP_202509081955
