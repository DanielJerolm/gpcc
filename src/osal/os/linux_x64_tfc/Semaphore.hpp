/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2019, 2021 Daniel Jerolm

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

#ifdef OS_LINUX_X64_TFC

#ifndef SEMAPHORE_HPP_201702252311
#define SEMAPHORE_HPP_201702252311

#include "internal/UnmanagedConditionVariable.hpp"
#include <limits>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace osal {

namespace internal {
class TFCCore;
}

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A counting semaphore.
 *
 * __Note:__\n
 * __This semaphore is managed by GPCC's TFC feature.__
 *
 * # Features
 * - Initial value configurable during instantiation
 * - Post/inkrement and wait/decrement
 *
 * # Constraints/Restrictions
 * - _All threads using instances of class Semaphore must live in the same process._
 *
 * # Alternatives
 * There are further semaphore implementations available in @ref GPCC_RESOURCEMANAGEMENT ->
 * @ref GPCC_RESOURCEMANAGEMENT_SEMAPHORES.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Semaphore final
{
  public:
    /// Maximum value of the semaphore.
    static size_t const MAX = std::numeric_limits<int32_t>::max();


    Semaphore(void) = delete;
    explicit Semaphore(size_t const initialValue);
    Semaphore(Semaphore const &) = delete;
    Semaphore(Semaphore &&) = delete;
    ~Semaphore(void);

    Semaphore& operator=(Semaphore const &) = delete;
    Semaphore& operator=(Semaphore &&) = delete;

    void Post(void);
    void Wait(void);

  private:
    /// Pointer to the @ref internal::TFCCore instance.
    /** This is setup by the constructor and not changed afterwards. */
    internal::TFCCore* const pTFCCore;

    /// Value of the semaphore.
    /** TFCCore's big lock is required.\n
        < 0: Number of blocked threads excl. those threads which are about to wake up.\n
        > 0: Number of threads that could call @ref Wait without being blocked. */
    int32_t v;

    /// Number of blocked threads. This includes the number of threads which are about to wake up.
    /** TFCCore's big lock is required. */
    int32_t blockedThreads;

    /// Number of threads to be released.
    /** TFCCore's big lock is required.\n
        This is incremented each time when @ref v is negative and incremented.\n
        TFCCore::ReportThreadAboutToWakeUp() is invoked each time this is incremented. */
    int32_t threadsToBeReleased;

    /// Condition variable used to signal when @ref threadsToBeReleased becomes larger than zero.
    /** This must be used in conjunction with TFCCore's big lock. */
    internal::UnmanagedConditionVariable freeCV;

    void Signal_freeCV(void) noexcept;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef SEMAPHORE_HPP_201702252311
#endif // #ifdef OS_LINUX_X64_TFC
