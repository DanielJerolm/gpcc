/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef TFCCORE_HPP_201702252127
#define TFCCORE_HPP_201702252127

#include "UnmanagedMutex.hpp"
#include <vector>
#include <cstddef>
#include <cstdint>
#include <ctime>

namespace gpcc {

namespace time {
class TimePoint;
}

namespace osal {
namespace internal {

class TimeLimitedThreadBlocker;

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Time-Flow-Control Core
 *
 * This class implements the core of GPCC's Time-Flow-Control feature (see @ref GPCC_TIME_FLOW_CONTROL). It is part
 * of GPCC's OSAL variant "linux_x64_tfc".
 *
 * This class' main responsibility is:
 * - keeping a set of emulated system clocks
 * - provide the emulated system clock's values to the process upon request
 * - keeping the "TFC Big Lock", a mutex used by the managed OSAL primitives (threads, semaphores, mutexes and
 *   condition variables) when they interact with class @ref TFCCore or TFC core helper classes.
 *
 * This class is intended to be used as singleton. Any process can have only one instance of it and all OSAL
 * primitives have to use the same instance. Class @ref TFCCore therefore cannot be instantiated directly. Instead
 * a global instance of it can be accessed via the static method @ref TFCCore::Get().
 *
 * All threads in the process which are created using GPCC's OSAL variant (linux_x64_tfc) are managed by GPCC's TFC
 * feature. Management by TFC requires that all threads which are going to block (either permanently or until some
 * point in time) have to report their blocking- and wake-up-activities to the global instance of this class.
 *
 * Whenever all managed threads are blocked and no thread is expected to wake-up due to a pending signal, this class
 * will increment the emulated system clocks to the point in time at which the next thread will continue due to some
 * kind of timeout condition. If there is no such thread, then a dead-lock has occurred. Dead-locks will be detected
 * by this class.
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TFCCore final
{
  public:
    TFCCore(TFCCore const &) = delete;
    TFCCore(TFCCore &&) = delete;
    ~TFCCore(void) = default;

    TFCCore& operator=(TFCCore const &) = delete;
    TFCCore& operator=(TFCCore &&) = delete;


    static TFCCore* Get(void);


    UnmanagedMutex& GetBigLock(void) const noexcept;


    void ReportNewThread(void) noexcept;
    void ReportThreadTermination(void) noexcept;

    void ReportThreadPermanentlyBlockedBegin(void) noexcept;
    void ReportThreadPermanentlyBlockedBegin(TimeLimitedThreadBlocker & blocker) noexcept;

    void ReportThreadAboutToWakeUp(void) noexcept;
    void UndoReportThreadAboutToWakeUp(void) noexcept;

    void ReportThreadCancellationRequested(void) noexcept;
    void ReportThreadCancellationDone(void) noexcept;

    void ReportThreadPermanentlyBlockedEnd(void) noexcept;
    void ReportThreadPermanentlyBlockedEnd(TimeLimitedThreadBlocker & blocker) noexcept;


    void GetEmulatedRealtime(struct ::timespec & ts) const noexcept;
    void GetEmulatedMonotonicTime(struct ::timespec & ts) const noexcept;

  private:
    /// Mutex protecting access to the emulated system time.
    UnmanagedMutex mutable timeMutex;

    /// Current time of the emulated realtime clock.
    /** @ref timeMutex is required. */
    struct ::timespec timeRealtime;

    /// Current time of the emulated monotonic clock.
    /** @ref timeMutex is required. */
    struct ::timespec timeMonotonic;


    /// TFC's "Big Lock".
    UnmanagedMutex mutable bigLock;


    /// Number of threads.
    /** @ref bigLock required. */
    size_t nbOfThreads;

    /// Number of currently blocked threads.
    /** @ref bigLock required.\n
        This includes threads which are about to wake up (-> @ref nbOfThreadsAboutToWakeUp). */
    size_t nbOfBlockedThreads;

    /// Number of currently blocked threads which are about to wake up.
    /** @ref bigLock required. */
    size_t nbOfThreadsAboutToWakeUp;

    /// Number of currently pending cancellation requests.
    /** @ref bigLock required. */
    size_t nbOfCancellationRequests;


    /// Vector containing @ref TimeLimitedThreadBlocker instances which have a thread blocked.
    /** @ref bigLock required.\n
        The blockers are sorted by the point in time until when their configured timeout expires.\n
        The blocker with the next timeout that will expire is located at index 0.*/
    std::vector<TimeLimitedThreadBlocker*> threadsBlockedByTimeout;


    TFCCore(void);


    void AllThreadsBlocked(void) noexcept;
    void IncrementEmulatedClocks(uint64_t const delta_ns);
    void NormalizeTimespec(struct ::timespec & ts);
};

/**
 * \brief Retrieves a reference to TFC's big lock.
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * Reference to TFC's big lock.
 */
inline UnmanagedMutex& TFCCore::GetBigLock(void) const noexcept
{
  return bigLock;
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef TFCCORE_HPP_201702252127
#endif // #ifdef OS_LINUX_X64_TFC
