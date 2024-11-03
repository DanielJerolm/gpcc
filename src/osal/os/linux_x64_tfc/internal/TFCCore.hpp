/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
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
 * \class TFCCore TFCCore.hpp "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
 * \brief Core of TFC.
 *
 * This class implements the core of GPCC's TFC feature (@ref GPCC_TIME_FLOW_CONTROL).
 *
 * This class' main responsibility is:
 * - keeping the emulated system clock
 * - keeping the "TFC Big Lock", a mutex used by the managed OSAL primitives (threads, semaphores, mutexes and
 *   condition variables) when they interact with TFC related classes from @ref gpcc::osal::internal.
 * - wakeup of threads whose timeout for blocking or sleeping has expired
 * - watch for special scenarios
 *   - dead lock
 *   - unreproducible behaviour (@ref GPCC_TIME_FLOW_CONTROL_REPRODUCIBILITY)
 *
 * This class is intended to be used as a singleton. Any process can have only one instance of it and all OSAL
 * primitives have to use the same instance. Class @ref TFCCore therefore cannot be instantiated directly. Instead
 * a global instance can be accessed via static method @ref TFCCore::Get().
 *
 * All threads in the process which are created using GPCC's OSAL variant with TFC are managed by GPCC's TFC feature.
 * Management by TFC requires that all threads which are going to block (either permanently or until some point in time)
 * have to report their blocking- and wake-up-activities to the global @ref TFCCore instance. Threads can only block in
 * OSAL primitives and reporting to @ref TFCCore is done by the OSAL primitives. User code does not directly interact
 * with this class.
 *
 * Whenever all managed threads are blocked and no thread is expected to wake-up (e.g. due to a condition variable being
 * signaled, or a semaphore being incremented), then this class will increment the emulated system time to the point in
 * time at which the next thread will continue due to some kind of timeout condition. If there is no such thread, then a
 * dead-lock has occured. Dead-locks will be detected by this class and result in a panic.
 *
 * - - -
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

    void EnableWatchForAlreadyExpiredTimeout(void);
    bool QueryAndResetWatchForAlreadyExpiredTimeout(void);
    bool DisableWatchForAlreadyExpiredTimeout(void);

    void EnableWatchForBlockWithSameTimeout(void);
    bool QueryAndResetWatchForBlockWithSameTimeout(void);
    bool DisableWatchForBlockWithSameTimeout(void);

    void EnableWatchForSimultaneousResumeOfMultipleThreads(void);
    bool QueryAndResetWatchForSimultaneousResumeOfMultipleThreads(void);
    bool DisableWatchForSimultaneousResumeOfMultipleThreads(void);

  private:
    /// Mutex protecting access to the emulated system time.
    /** Locking order: @ref bigLock -> @ref timeMutex */
    UnmanagedMutex mutable timeMutex;

    /// Current time of the emulated realtime clock.
    /** Read access: @ref timeMutex or @ref bigLock is required.\n
        Write access: @ref timeMutex and @ref bigLock are both required. */
    struct ::timespec timeRealtime;

    /// Current time of the emulated monotonic clock.
    /** Read access: @ref timeMutex or @ref bigLock is required.\n
        Write access: @ref timeMutex and @ref bigLock are both required. */
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


    /// Controls, if watching for threads that want to block with an already expired timeout is enabled.
    /** @ref bigLock required. */
    bool watchForAlreadyExpiredTimeout;

    /// Indicates, if a case of blocking with already expired timeout has been detected.
    /** @ref bigLock required. */
    bool alreadyExpiredTimeoutDetected;


    /// Controls, if watching for threads that block until the same point in time is enabled.
    /** @ref bigLock required. */
    bool watchForBlockWithSameTimeout;

    /// Indicates, if a case of at least two threads blocking until the same point in time has been detected.
    /** @ref bigLock required. */
    bool blockWithSameTimeoutDetected;


    /// Controls, if watching for simultaneous resume of multiple threads after increment of the system time is enabled.
    /** @ref bigLock required. */
    bool watchForSimultaneousResumeOfMultipleThreads;

    /// Indicates, if a case of resuming more than one thread after increment of the system time has been detected.
    /** @ref bigLock required. */
    bool simultaneousResumeOfMultipleThreadsDetected;


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
