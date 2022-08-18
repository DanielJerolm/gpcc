/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef WORKQUEUE_HPP_201612212309
#define WORKQUEUE_HPP_201612212309

#include "IWorkQueue.hpp"
#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/Mutex.hpp"

namespace gpcc {
namespace execution {
namespace async {

class WorkPackage;

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Work queue for executing @ref WorkPackage instances.
 *
 * Features/characteristics:
 * - One thread.
 * - Execution in FIFO order.
 *
 * For general information about work queues and work packages please refer to @ref GPCC_EXECUTION_ASYNC.
 *
 * \htmlonly <style>div.image img[src="execution/async/WorkQueue_Structure.png"]{width:90%;}</style> \endhtmlonly
 * \image html "execution/async/WorkQueue_Structure.png" "WorkQueue structure"
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class WorkQueue final: public IWorkQueue
{
  public:
    WorkQueue(void);
    WorkQueue(WorkQueue const &) = delete;
    WorkQueue(WorkQueue&&) = delete;
    virtual ~WorkQueue(void);

    WorkQueue& operator=(WorkQueue const &) = delete;
    WorkQueue& operator=(WorkQueue&&) = delete;

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

    void Work(void);
    void RequestTermination(void) noexcept;

  private:
    /// Mutex for queue-related stuff.
    osal::Mutex mutable queueMutex;

    /// Mutex for work queue flush.
    /** This is locked while a work package's functor is executed. */
    osal::Mutex flushMutex;

    /// Condition variable indicating that the queue is no longer empty or that @ref terminate has been asserted.
    /** This is to be used in conjunction with @ref queueMutex. */
    osal::ConditionVariable queueConVar;

    /// First enqueued work package. This is the next work package to be executed.
    /** @ref queueMutex is required.\n
        The pPrev-pointers of the enqueued work packages point towards this. */
    WorkPackage* pQueueFirst;

    /// Last enqueued work package. New work packages are enqueued here.
    /** @ref queueMutex is required.\n
        The pNext-pointers of the enqueued work packages point towards this. */
    WorkPackage* pQueueLast;

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

    /// Pointer to the currently executed work package. nullptr = none.
    /** @ref queueMutex is required.\n
        This is used to allow enqueueing of currently executed static work packages. */
    WorkPackage const * pCurrentExecutedWP;

    void CheckStateAndSetToInQ_static(WorkPackage& wp) const;
    void CheckStateAndSetToInQ_dynamic(WorkPackage& wp) const noexcept;

    void Release(WorkPackage* const pWP) noexcept;
    void Finish(WorkPackage* const pWP) noexcept;
};

} // namespace async
} // namespace execution
} // namespace gpcc

#endif /* WORKQUEUE_HPP_201612212309 */
