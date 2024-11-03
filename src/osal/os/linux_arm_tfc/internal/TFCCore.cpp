/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include "TFCCore.hpp"
#include <gpcc/compiler/builtins.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "TimeLimitedThreadBlocker.hpp"
#include "UnmanagedMutexLocker.hpp"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <cerrno>

#define NSEC_PER_SEC 1000000000L

namespace gpcc {
namespace osal {
namespace internal {

/**
 * \brief Retrieves a pointer to the one-and-only instance of class @ref TFCCore.
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * Pointer to the one-and-only instance of class @ref TFCCore.
 */
TFCCore* TFCCore::Get(void)
{
  static TFCCore globalInst;
  return &globalInst;
}

/**
 * \brief Reports creation of a new thread.
 *
 * This must be invoked just before the new thread is started.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportNewThread(void) noexcept
{
  nbOfThreads++;
  if (nbOfThreads == 0)
    PANIC(); // Too many threads
}

/**
 * \brief Reports termination of an thread.
 *
 * This is the counterpart to @ref ReportNewThread(). It must be invoked
 * after the terminated thread has been joined or after the attempt to create
 * a new thread has failed.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadTermination(void) noexcept
{
  if (nbOfThreads == 1U)
    PANIC();

  nbOfThreads--;
}

/**
 * \brief Announces that an thread is going to block permanently using an unmanaged POSIX primitive.
 *
 * \note  This may increment the emulated system time!
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadPermanentlyBlockedBegin(void) noexcept
{
  nbOfBlockedThreads++;
  if (nbOfBlockedThreads > nbOfThreads)
    PANIC();

  if ((nbOfBlockedThreads == nbOfThreads) && (nbOfThreadsAboutToWakeUp == 0))
    AllThreadsBlocked();
}

/**
 * \brief Announces that a thread is going to block permanently using an unmanaged POSIX primitive and
 *        an @ref TimeLimitedThreadBlocker to realize a timeout.
 *
 * \note  This may increment the emulated system time!
 *
 * - - -
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param blocker
 * @ref TimeLimitedThreadBlocker instance that will be notified if the timeout expires.
 */
void TFCCore::ReportThreadPermanentlyBlockedBegin(TimeLimitedThreadBlocker & blocker) noexcept
{
  try
  {
    // timeout already reached?
    bool const timeout = (blocker.absTimeout <= gpcc::time::TimePoint(timeMonotonic));

    if (!timeout)
    {
      // add to list of blocked threads
      auto insertHereCheck = [&](TimeLimitedThreadBlocker const * e) { return (blocker.absTimeout <= e->absTimeout); };
      auto it = std::find_if(threadsBlockedByTimeout.begin(), threadsBlockedByTimeout.end(), insertHereCheck);

      if (   (watchForBlockWithSameTimeout)
          && (it != threadsBlockedByTimeout.end())
          && ((*it)->absTimeout == blocker.absTimeout))
      {
        std::cout << "TFC: Unreproducible behaviour may occur in the future." << std::endl
                  << "     (At least two threads blocked until same point in time)" << std::endl;

        blockWithSameTimeoutDetected = true;
      }

      threadsBlockedByTimeout.insert(it, &blocker);
    }
    else
    {
      if (watchForAlreadyExpiredTimeout)
      {
        std::cout << "TFC: A thread wants to block with an already expired timeout value!" << std::endl;
        alreadyExpiredTimeoutDetected = true;
      }
    }

    nbOfBlockedThreads++;
    if (nbOfBlockedThreads > nbOfThreads)
      PANIC();

    if (timeout)
      blocker.SignalTimeout();

    if ((nbOfBlockedThreads == nbOfThreads) && (nbOfThreadsAboutToWakeUp == 0))
      AllThreadsBlocked();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Announces that an thread which is currently blocked using an unmanaged POSIX primitive is
 * about to wake up.
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadAboutToWakeUp(void) noexcept
{
  nbOfThreadsAboutToWakeUp++;
  if (nbOfThreadsAboutToWakeUp > nbOfBlockedThreads)
    PANIC();
}

/**
 * \brief Neutralizes a previous call to @ref ReportThreadAboutToWakeUp().
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::UndoReportThreadAboutToWakeUp(void) noexcept
{
  if (nbOfThreadsAboutToWakeUp == 0)
    PANIC();

  nbOfThreadsAboutToWakeUp--;
}

/**
 * \brief Reports that cancellation of an thread has been requested.
 *
 * Note: TFC's dead-lock detection is disabled while any thread cancellation request is pending.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadCancellationRequested(void) noexcept
{
  nbOfCancellationRequests++;
  if (nbOfCancellationRequests > nbOfThreads)
    PANIC();
}

/**
 * \brief Reports that an thread-cancellation request is being processed.
 *
 * Note: TFC's dead-lock detection is disabled while any thread cancellation request is pending.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadCancellationDone(void) noexcept
{
  if (nbOfCancellationRequests == 0)
    PANIC();

  nbOfCancellationRequests--;
}

/**
 * \brief Reports that an thread no longer blocks on an unmanaged POSIX primitive.
 *
 * This is the counterpart to @ref ReportThreadPermanentlyBlockedBegin(void).
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadPermanentlyBlockedEnd(void) noexcept
{
   if ((nbOfBlockedThreads == 0) || (nbOfThreadsAboutToWakeUp == 0))
     PANIC();

   if (nbOfBlockedThreads < nbOfThreadsAboutToWakeUp)
     PANIC();

  nbOfBlockedThreads--;
  nbOfThreadsAboutToWakeUp--;
}

/**
 * \brief Reports that an thread no longer blocks on an unmanaged POSIX primitive.
 *
 * This is the counterpart to @ref ReportThreadPermanentlyBlockedBegin(TimeLimitedThreadBlocker & blocker).
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
void TFCCore::ReportThreadPermanentlyBlockedEnd(TimeLimitedThreadBlocker & blocker) noexcept
{
   if ((nbOfBlockedThreads == 0) || (nbOfThreadsAboutToWakeUp == 0))
     PANIC();

   if (nbOfBlockedThreads < nbOfThreadsAboutToWakeUp)
     PANIC();

  nbOfBlockedThreads--;
  nbOfThreadsAboutToWakeUp--;

  auto it = std::find(threadsBlockedByTimeout.begin(), threadsBlockedByTimeout.end(), &blocker);
  if (it != threadsBlockedByTimeout.end())
    threadsBlockedByTimeout.erase(it);
}

/**
 * \brief Retrieves the current value of the emulated realtime clock.
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
 * \param ts
 * The current value of the emulated realtime clock will be written into the referenced `timespec`.
 */
void TFCCore::GetEmulatedRealtime(struct ::timespec & ts) const noexcept
{
  UnmanagedMutexLocker timeMutexLocker(timeMutex);
  ts = timeRealtime;
}

/**
 * \brief Retrieves the current value of the emulated monotonic clock.
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
 * \param ts
 * The current value of the emulated monotonic clock will be written into the referenced `timespec`.
 */
void TFCCore::GetEmulatedMonotonicTime(struct ::timespec & ts) const noexcept
{
  UnmanagedMutexLocker timeMutexLocker(timeMutex);
  ts = timeMonotonic;
}

/**
 * \brief Enables watching for threads that attempt to block with an already expired timeout.
 *
 * \pre   Watching for threads that attempt to block with an already expired timeout is disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void TFCCore::EnableWatchForAlreadyExpiredTimeout(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (watchForAlreadyExpiredTimeout)
    throw std::logic_error("TFCCore::EnableWatchForAlreadyExpiredTimeout: Already enabled");

  watchForAlreadyExpiredTimeout = true;
  alreadyExpiredTimeoutDetected = false;
}

/**
 * \brief Queries, if a thread attempted to block with already expired timeout and forgets the attempt.
 *
 * The query and forgetting a potential attempt are one atomic operation.
 *
 * \pre   Watching for threads that attempt to block with an already expired timeout is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   There was at least one attempt to block with already expired timeout.
 * \retval false  There was no attempt to block with already expired timeout.
 */
bool TFCCore::QueryAndResetWatchForAlreadyExpiredTimeout(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (!watchForAlreadyExpiredTimeout)
    throw std::logic_error("TFCCore::QueryAndResetWatchForAlreadyExpiredTimeout: Not enabled");

  bool const retVal = alreadyExpiredTimeoutDetected;
  alreadyExpiredTimeoutDetected = false;
  return retVal;
}

/**
 * \brief Disables watching for threads that attempt to block with an already expired timeout, and returns if any such
 *        attempt has occurred since watching has been enabled.
 *
 * \pre   Watching for threads that attempt to block with an already expired timeout is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   There was at least one attempt by a thread to block with an already expired timeout.
 * \retval false  There was no attempt by any thread to block with an already expired timeout.
 */
bool TFCCore::DisableWatchForAlreadyExpiredTimeout(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (!watchForAlreadyExpiredTimeout)
    throw std::logic_error("TFCCore::DisableWatchForAlreadyExpiredTimeout: Not enabled");

  watchForAlreadyExpiredTimeout = false;
  return alreadyExpiredTimeoutDetected;
}

/**
 * \brief Enables watching for threads that block with same timeout.
 *
 * \pre   Watching for threads that block with same timeout is disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void TFCCore::EnableWatchForBlockWithSameTimeout(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (watchForBlockWithSameTimeout)
    throw std::logic_error("TFCCore::EnableWatchForBlockWithSameTimeout: Already enabled");

  watchForBlockWithSameTimeout = true;
  blockWithSameTimeoutDetected = false;
}

/**
 * \brief Queries, if any two threads attempted to block with same timeout and forgets the attempt.
 *
 * The query and forgetting a potential attempt are one atomic operation.
 *
 * \pre   Watching for threads that block with same timeout is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   There was at least one attempt by at least two threads to block with same timeout.
 * \retval false  There was no attempt by any threads to block with same timeout.
 */
bool TFCCore::QueryAndResetWatchForBlockWithSameTimeout(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (!watchForBlockWithSameTimeout)
    throw std::logic_error("TFCCore::QueryAndResetWatchForBlockWithSameTimeout: Not enabled");

  bool const retVal = blockWithSameTimeoutDetected;
  blockWithSameTimeoutDetected = false;
  return retVal;
}

/**
 * \brief Disables watching for threads that block with same timeout, and returns if any such situation has occurred
 *        since watching has been enabled.
 *
 * \pre   Watching for threads that block with same timeout is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   There was at least one situation in which multiple threads blocked until the same timeout.
 * \retval false  All threads blocked with unique timeouts.
 */
bool TFCCore::DisableWatchForBlockWithSameTimeout(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (!watchForBlockWithSameTimeout)
    throw std::logic_error("TFCCore::DisableWatchForBlockWithSameTimeout: Not enabled");

  watchForBlockWithSameTimeout = false;
  return blockWithSameTimeoutDetected;
}

/**
 * \brief Enables watching for simultaneous resume of multiple threads after increment of the system time.
 *
 * \pre   Watching for simultaneous resume of multiple threads after increment of the system time is disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void TFCCore::EnableWatchForSimultaneousResumeOfMultipleThreads(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (watchForSimultaneousResumeOfMultipleThreads)
    throw std::logic_error("TFCCore::EnableWatchForSimultaneousResumeOfMultipleThreads: Already enabled");

  watchForSimultaneousResumeOfMultipleThreads = true;
  simultaneousResumeOfMultipleThreadsDetected = false;
}

/**
 * \brief Queries, if more than one thread was resumed simultaneously after an increment of the system time and forgets
 *        the incident.
 *
 * The query and forgetting a potential incident are one atomic operation.
 *
 * \pre   Watching for simultaneous resume of multiple threads after increment of the system time is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   More than one thread has been resumed after increment of the system time at least once.
 * \retval false  No thread, or only single threads have been resumed after any increment of the system time.
 */
bool TFCCore::QueryAndResetWatchForSimultaneousResumeOfMultipleThreads(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (!watchForSimultaneousResumeOfMultipleThreads)
    throw std::logic_error("TFCCore::QueryAndResetWatchForSimultaneousResumeOfMultipleThreads: Not enabled");

  bool const retVal = simultaneousResumeOfMultipleThreadsDetected;
  simultaneousResumeOfMultipleThreadsDetected = false;
  return retVal;
}

/**
 * \brief Disables watching for simultaneous resume of multiple threads after increment of the system time, and returns
 *        if any such situation has occurred since watching has been enabled.
 *
 * \pre   Watching for simultaneous resume of multiple threads after increment of the system time is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   More than one thread has been resumed after increment of the system time at least once.
 * \retval false  No thread, or only single threads have been resumed after any increment of the system time.
 */
bool TFCCore::DisableWatchForSimultaneousResumeOfMultipleThreads(void)
{
  UnmanagedMutexLocker bigLockLocker(bigLock);

  if (!watchForSimultaneousResumeOfMultipleThreads)
    throw std::logic_error("TFCCore::DisableWatchForSimultaneousResumeOfMultipleThreads: Not enabled");

  watchForSimultaneousResumeOfMultipleThreads = false;
  return simultaneousResumeOfMultipleThreadsDetected;
}

/**
 * \brief Constructor.
 *
 * - The emulated clocks are initialized with the system's native clocks.
 * - Monitoring for special situations is disabled:
 *   - attempt to block with already expired timeout value
 *   - multiple threads block until same point in time
 *   - resume of multiple threads after increment of the emulated system time
 *
 * - - -
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Deferred cancellation is safe.
 */
TFCCore::TFCCore(void)
: timeMutex()
, timeRealtime()
, timeMonotonic()
, bigLock()
, nbOfThreads(1)
, nbOfBlockedThreads(0)
, nbOfThreadsAboutToWakeUp(0)
, nbOfCancellationRequests(0)
, threadsBlockedByTimeout()
, watchForAlreadyExpiredTimeout(false)
, alreadyExpiredTimeoutDetected(false)
, watchForBlockWithSameTimeout(false)
, blockWithSameTimeoutDetected(false)
, watchForSimultaneousResumeOfMultipleThreads(false)
, simultaneousResumeOfMultipleThreadsDetected(false)
{
  auto ret = clock_gettime(CLOCK_REALTIME, &timeRealtime);
  if (ret != 0)
    throw std::system_error(errno, std::generic_category(), "TFCCore::TFCCore: clock_gettime failed (CLOCK_REALTIME)");

  ret = clock_gettime(CLOCK_MONOTONIC, &timeMonotonic);
  if (ret != 0)
    throw std::system_error(errno, std::generic_category(), "TFCCore::TFCCore: clock_gettime failed (CLOCK_MONOTONIC)");
}

/**
 * \brief Performs actions if all threads are blocked and no wake-up of any thread is pending.
 *
 * The following actions are performed:
 * 1. System time is advanced to the timeout of the next blocked thread.
 * 2. The next blocked thread and all further threads with the same timeout are woken up.
 *
 * \pre   All threads are blocked.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 */
void TFCCore::AllThreadsBlocked(void) noexcept
{
  try
  {
    if (nbOfThreadsAboutToWakeUp != 0)
      PANIC(); // Precondition violated: Not all threads blocked

    if (threadsBlockedByTimeout.empty())
    {
      if (nbOfCancellationRequests == 0)
      {
        Panic("TFCCore::AllThreadsBlocked: Dead-Lock detected. All threads permanently blocked.");
      }
      else
      {
        // dead-lock detection is disabled while at least one thread has a cancellation request pending
        return;
      }
    }

    gpcc::time::TimePoint now(timeMonotonic);

    auto it = threadsBlockedByTimeout.begin();
    TimeLimitedThreadBlocker* tb = *it;

    gpcc::time::TimeSpan const delta = tb->absTimeout - now;
    if (delta.ns() <= 0)
    {
      // There should be no thread with expired timeout in threadsBlockedByTimeout
      PANIC();
    }

    // increment system clock
    IncrementEmulatedClocks(delta.ns());
    now = timeMonotonic;

    // wake up thread
    tb->SignalTimeout();

    // wake up all other threads whose timeout has also expired
    it = threadsBlockedByTimeout.erase(it);
    while (it != threadsBlockedByTimeout.end())
    {
      tb = *it;

      if (tb->absTimeout == now)
      {
        if (watchForSimultaneousResumeOfMultipleThreads)
        {
          std::cout << "TFC: Unreproducible behaviour." << std::endl
                    << "     (resumed more than one thread after increase of emulated system time)" << std::endl;
          simultaneousResumeOfMultipleThreadsDetected = true;
        }

        tb->SignalTimeout();
        it = threadsBlockedByTimeout.erase(it);
      }
      else if (tb->absTimeout > now)
      {
        break;
      }
      else
      {
        // threads in threadsBlockedByTimeout are not properly sorted
        PANIC();
      }
    } // while (it != threadsBlockedByTimeout.end())
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Increments all emulated clocks.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread-safety:__\n
 * TFC's big lock must be acquired.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param delta_ns
 * Timespan in ns that shall be added to the emulated clocks.
 */
void TFCCore::IncrementEmulatedClocks(uint64_t const delta_ns)
{
  // split delta into seconds and ns
  uint64_t const sec = delta_ns / NSEC_PER_SEC;
  uint32_t const ns  = delta_ns % NSEC_PER_SEC;

  UnmanagedMutexLocker timeMutexLocker(timeMutex);

  struct ::timespec ts_realtime;
  if (compiler::OverflowAwareAdd(timeRealtime.tv_sec, sec, &ts_realtime.tv_sec))
    throw std::overflow_error("TFCCore::IncrementEmulatedClocks: Overflow adding seconds to timeRealtime");

  struct ::timespec ts_monotonic;
  if (compiler::OverflowAwareAdd(timeMonotonic.tv_sec, sec, &ts_monotonic.tv_sec))
    throw std::overflow_error("TFCCore::IncrementEmulatedClocks: Overflow adding seconds to timeMonotonic");

  ts_realtime.tv_nsec = timeRealtime.tv_nsec + ns;
  ts_monotonic.tv_nsec = timeMonotonic.tv_nsec + ns;

  // tv_nsec may be out of bounds and required inc/dec of tv_sec
  NormalizeTimespec(ts_realtime);
  NormalizeTimespec(ts_monotonic);

  // assign result
  timeRealtime = ts_realtime;
  timeMonotonic = ts_monotonic;
}

/**
 * \brief Helper function:\n
 * Normalizes the ns-portion of a `timespec` struct to [0..1E9-1] by inc/dec of the second portion.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - `ts` will contain random data
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param ts
 * `timespec` structure, which shall be normalized.
 */
void TFCCore::NormalizeTimespec(struct ::timespec & ts)
{
  if (ts.tv_nsec < 0)
  {
    do
    {
      ts.tv_nsec += NSEC_PER_SEC;

      if (ts.tv_sec == std::numeric_limits<time_t>::min())
      {
        ts.tv_nsec = 0; // ensure basic exception safety
        throw std::overflow_error("TFCCore::NormalizeNanoseconds: Overflow decrementing seconds");
      }
      ts.tv_sec--;
    }
    while (ts.tv_nsec < 0);
  } // if (ts.tv_nsec < 0)
  else
  {
    while (ts.tv_nsec >= NSEC_PER_SEC)
    {
      ts.tv_nsec -= NSEC_PER_SEC;

      if (ts.tv_sec == std::numeric_limits<time_t>::max())
      {
        ts.tv_nsec = 0; // ensure basic exception safety
        throw std::overflow_error("TFCCore::NormalizeNanoseconds: Overflow incrementing seconds");
      }
      ts.tv_sec++;
    }
  } // if (ts.tv_nsec < 0)... else...
}

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
