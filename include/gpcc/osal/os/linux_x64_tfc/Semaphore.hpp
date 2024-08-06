/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#ifndef SEMAPHORE_HPP_201702252311
#define SEMAPHORE_HPP_201702252311

#include <limits>
#include <memory>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace osal {

namespace internal {
class TFCCore;
class UnmanagedConditionVariable;
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
 * - Post/increment and wait/decrement
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
    std::unique_ptr<internal::UnmanagedConditionVariable> spFreeCV;

    void Signal_freeCV(void) noexcept;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef SEMAPHORE_HPP_201702252311
#endif // #ifdef OS_LINUX_X64_TFC
