/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef DEFERREDWORKQUEUE_HPP_201612221248
#define DEFERREDWORKQUEUE_HPP_201612221248

#include <gpcc/execution/async/IDeferredWorkQueue.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>

namespace gpcc {
namespace execution {
namespace async {

class DeferredWorkPackage;
class WorkPackage;

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Work queue for executing @ref WorkPackage and @ref DeferredWorkPackage instances.
 *
 * Features/characteristics:
 * - One thread.
 * - Normal and deferred work packages are stored in separate queues.
 * - Work packages from the normal queue are executed one-by-one in FIFO order.
 * - Work packages from the deferred queue are executed not before the point in time is reached,
 *   until when their execution is deferred.
 * - Deferred work packages whose point in time has been reached are executed one-by-one.\n
 *   The order of execution is:
 *   + sorted by point in time; most far in the past first.
 *   + FIFO-order if time-points of work packages are equal.
 * - Deferred work packages (if time point reached) have priority above normal work packages.
 *
 * For general information about work queues and work packages please refer to @ref GPCC_EXECUTION_ASYNC.
 *
 * \htmlonly <style>div.image img[src="execution/async/DeferredWorkQueue_Structure.png"]{width:80%;}</style> \endhtmlonly
 * \image html "execution/async/DeferredWorkQueue_Structure.png" "DeferredWorkQueue structure"
 */
class DeferredWorkQueue final: public IDeferredWorkQueue
{
  public:
    DeferredWorkQueue(void);
    DeferredWorkQueue(DeferredWorkQueue const &) = delete;
    DeferredWorkQueue(DeferredWorkQueue&&) = delete;
    virtual ~DeferredWorkQueue(void);

    DeferredWorkQueue& operator=(DeferredWorkQueue const &) = delete;
    DeferredWorkQueue& operator=(DeferredWorkQueue&&) = delete;

    // --> Interface required by IWorkQueue
    void Add(std::unique_ptr<WorkPackage> spWP) override;
    void Add(WorkPackage & wp) override;
    void InsertAtHeadOfList(std::unique_ptr<WorkPackage> spWP) override;
    void InsertAtHeadOfList(WorkPackage & wp) override;
    void Remove(WorkPackage & wp) override;
    void Remove(void const * const pOwnerObject) override;
    void Remove(void const * const pOwnerObject, uint32_t const ownerID) override;
    void WaitUntilCurrentWorkPackageHasBeenExecuted(void const * const pOwnerObject) const override;
    bool IsAnyInQueue(void const * const pOwnerObject) const override;
    void FlushNonDeferredWorkPackages(void) override;
    // <--

    // --> Interface required by IDeferredWorkQueue
    void Add(std::unique_ptr<DeferredWorkPackage> spDWP) override;
    void Add(DeferredWorkPackage & dwp) override;
    void Remove(DeferredWorkPackage & dwp) override;
    // <--

    void Work(void);
    void RequestTermination(void) noexcept;

  private:
    /// Mutex for queue-related stuff.
    osal::Mutex mutable queueMutex;

    /// Mutex for work queue flush.
    /** This is locked while a work package's functor is executed. */
    osal::Mutex flushMutex;

    /// Condition variable indicating that the queue is no longer empty, or that @ref terminate has been
    /// asserted, or that a new timeout must be setup (e.g. deferred work package added).
    /** This is to be used in conjunction with @ref queueMutex. \n
        This is also used to generate defined timeouts for the execution of deferred work packages. */
    osal::ConditionVariable queueConVar;

    /// First enqueued "normal" work package. This is the next "normal" work package to be executed.
    /** @ref queueMutex is required.\n
        The pPrev-pointers of the enqueued work packages point towards this. */
    WorkPackage* pQueueFirst;

    /// Last enqueued "normal" work package. New "normal" work packages are enqueued here.
    /** @ref queueMutex is required.\n
        The pNext-pointers of the enqueued work packages point towards this. */
    WorkPackage* pQueueLast;

    /// First enqueued "deferred" work package. This is the next "deferred" work package to be executed.
    /** @ref queueMutex is required.\n
        The pPrev-pointers of the enqueued work packages point towards this.\n
        The deferred work packages in this queue are sorted by the point in time until when their
        execution is deferred. The work package which reaches its point in time next is referenced
        by this pointer. */
    DeferredWorkPackage* pDeferredQueueFirst;

    /// Last enqueued "deferred" work package. This is the last "deferred" work package to be executed.
    /** @ref queueMutex is required.\n
        The pNext-pointers of the enqueued work packages point towards this.\n
        The deferred work packages in this queue are sorted by the point in time until when their
        execution is deferred. The work package which reaches its point in time last is referenced
        by this pointer. */
    DeferredWorkPackage *pDeferredQueueLast;

    /// Terminate flag.
    /** @ref queueMutex is required.\n
        true  = Work package execution shall stop after execution of the current work package.
                If no work package is currently executed, then work package execution shall stop
                immediately.\n
        false = No terminate request. The work queue shall wait for work packages and execute them. */
    bool terminate;

    /// Pointer to the owner object of the currently executed work package.
    /** @ref queueMutex is required.\n
        nullptr = work queue idle or the work package has no owner (anonymous owner). */
    void const * pOwnerOfCurrentExecutedWP;

    /// Condition variable indicating that @ref pOwnerOfCurrentExecutedWP has changed.
    /** This is to be used in conjunction with @ref queueMutex. */
    mutable osal::ConditionVariable ownerChangedConVar;

    /// Pointer to the currently executed work package / deferred work package. nullptr = none.
    /** @ref queueMutex is required.\n
        This is used to allow enqueueing of currently executed static (deferred) work packages. */
    void const * pCurrentExecutedWP;

    void CheckStateAndSetToInQ_static(WorkPackage& wp) const;
    void CheckStateAndSetToInQ_static(DeferredWorkPackage& dwp) const;
    void CheckStateAndSetToInQ_dynamic(WorkPackage& wp) const noexcept;
    void CheckStateAndSetToInQ_dynamic(DeferredWorkPackage& dwp) const noexcept;

    void Release(WorkPackage* const pWP) noexcept;
    void Release(DeferredWorkPackage* const pDWP) noexcept;
    void Finish(WorkPackage* const pWP) noexcept;
    void Finish(DeferredWorkPackage* const pDWP) noexcept;
};

} // namespace async
} // namespace execution
} // namespace gpcc

#endif /* DEFERREDWORKQUEUE_HPP_201612221248 */
