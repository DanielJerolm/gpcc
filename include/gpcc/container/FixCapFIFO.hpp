/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#ifndef FIXCAPFIFO_HPP_202509081955
#define FIXCAPFIFO_HPP_202509081955

#include <cstddef>
#include <memory>

namespace gpcc      {
namespace container {

/**
 * \ingroup GPCC_CONTAINER
 * \brief FIFO with fixed capacity, usable from both thread and interrupt context.
 *
 * # Purpose
 * This class implements a light-weight FIFO with fixed capacity. It is designed for usage from both thread and
 * interrupt context by the following measures:
 * - Except for its constructor, it does not allocate memory at runtime.
 * - Methods do not throw by guarantee.
 * - Methods do not use any functions from the C/C++ runtime that are unavailable or incompatible with interrupt
 *   context. `memcpy()` is considered available and compatible with interrupt context.
 *
 * If the caller applies proper locking, then this FIFO can be used to transfer data between interrupt and thread
 * context. Please refer to the thread-safety notes of each method to determine if it can be used from interrupt
 * context.
 *
 * # Optimizations and limitations
 * Especially single push/pop operations are heavily optimized for ARM and x64 CPUs. The number of generated
 * instructions is minimal if @p SIZET is set to `size_t`. However, the RAM footprint of a @ref FixCapFIFO instance
 * can be reduced if @p SIZET is set to a smaller type at the cost of 1-2 additional machine instructions for push/pop
 * operations.
 *
 * The type @p T of the elements stored in the FIFO is limited to trivial types such as PODs (plain old data types).
 *
 * # Differentiation from STL
 * The STL offers `std::queue`. For use cases that do not involve interrupt context, you should prefer `std::queue` for
 * the following reasons:
 * - No limitation on the data type of the elements stored in the FIFO
 * - Rich API.
 *
 * However, if interrupt context is involved, `std::queue` must not be used and @ref FixCapFIFO is an alternative.
 *
 * - - -
 *
 * \tparam T
 * Data type of the items.
 *
 * \param SIZET
 * Data type used for the FIFO's capacity, size, and internal management. The default is `size_t` which gives best
 * performance. However, on size-constrained systems, a smaller datatype can be selected to reduce the footprint of the
 * FIFO object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread-safe, but non-modifying concurrent access is safe.\n
 * Selected member functions can be invoked from interrupt context, if the caller applies proper locking.
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

    void UnsafePush(T const value) noexcept;
    T UnsafePop(void) noexcept;

    size_t PushMultiple(T const * const pSrc, size_t const n) noexcept;
    size_t PopMultiple(T* const pDest, size_t const n) noexcept;

  private:
    /// Memory used by the FIFO to store elements of type T.
    std::unique_ptr<T[]> spMemory_;

    /// Capacity of the FIFO.
    /** This is a power of 2.\n
        This is not const by intention (simple impl. of move-assignment operator). */
    SIZET capacity_;

    /// Number of items in the FIFO.
    SIZET size_;

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
