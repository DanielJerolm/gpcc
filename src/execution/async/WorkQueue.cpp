/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "WorkQueue.hpp"
#include "WorkPackage.hpp"
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Semaphore.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <stdexcept>

namespace gpcc {
namespace execution {
namespace async {

using namespace gpcc::osal;

/**
 * \brief Constructor.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 */
WorkQueue::WorkQueue(void)
: queueMutex()
, flushMutex()
, queueConVar()
, pQueueFirst(nullptr)
, pQueueLast(nullptr)
, terminate(false)
, pOwnerOfCurrentExecutedWP(nullptr)
, ownerChangedConVar()
, pCurrentExecutedWP(nullptr)
{
}

/**
 * \brief Destructor.
 *
 * Any dynamic work packages that are still enqueued will be released.\n
 * Any static work packages that are still enqueued will be removed from the work queue.
 *
 * - - -
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
WorkQueue::~WorkQueue(void)
{
  try
  {
    MutexLocker queueMutexLocker(queueMutex);

    auto pWP = pQueueFirst;
    while (pWP != nullptr)
    {
      auto toBeReleased = pWP;
      pWP = pWP->pNext;
      Release(toBeReleased);
    }
  }
  catch (...)
  {
    PANIC();
  }
}

// --> Interface required by IWorkQueue
/// \copydoc IWorkQueue::Add(std::unique_ptr<WorkPackage> spWP)
void WorkQueue::Add(std::unique_ptr<WorkPackage> spWP)
{
  if (!spWP)
    throw std::invalid_argument("WorkQueue::Add: !spWP");

  MutexLocker queueMutexLocker(queueMutex);

  if (pQueueLast == nullptr)
  {
    // (queue empty)

    queueConVar.Signal();

    CheckStateAndSetToInQ_dynamic(*spWP);

    spWP->pNext = nullptr;
    spWP->pPrev = nullptr;

    pQueueFirst = spWP.get();
    pQueueLast  = spWP.release();
  }
  else
  {
    // (queue not empty)

    CheckStateAndSetToInQ_dynamic(*spWP);

    spWP->pPrev = pQueueLast;
    spWP->pNext = nullptr;

    pQueueLast->pNext = spWP.get();
    pQueueLast = spWP.release();
  }
}

/// \copydoc IWorkQueue::Add(WorkPackage & wp)
void WorkQueue::Add(WorkPackage & wp)
{
  MutexLocker queueMutexLocker(queueMutex);

  if (pQueueLast == nullptr)
  {
    // (queue empty)

    queueConVar.Signal();

    CheckStateAndSetToInQ_static(wp);

    wp.pNext = nullptr;
    wp.pPrev = nullptr;

    pQueueFirst = &wp;
    pQueueLast  = &wp;
  }
  else
  {
    // (queue not empty)

    CheckStateAndSetToInQ_static(wp);

    wp.pPrev = pQueueLast;
    wp.pNext = nullptr;

    pQueueLast->pNext = &wp;
    pQueueLast = &wp;
  }
}

/// \copydoc IWorkQueue::InsertAtHeadOfList(std::unique_ptr<WorkPackage> spWP)
void WorkQueue::InsertAtHeadOfList(std::unique_ptr<WorkPackage> spWP)
{
  if (!spWP)
    throw std::invalid_argument("WorkQueue::InsertAtHeadOfList: !spWP");

  MutexLocker queueMutexLocker(queueMutex);

  if (pQueueLast == nullptr)
  {
    // (queue empty)

    queueConVar.Signal();

    CheckStateAndSetToInQ_dynamic(*spWP);

    spWP->pNext = nullptr;
    spWP->pPrev = nullptr;

    pQueueFirst = spWP.get();
    pQueueLast  = spWP.release();
  }
  else
  {
    // (queue not empty)

    CheckStateAndSetToInQ_dynamic(*spWP);

    spWP->pPrev = nullptr;
    spWP->pNext = pQueueFirst;

    pQueueFirst->pPrev = spWP.get();
    pQueueFirst = spWP.release();
  }
}

/// \copydoc IWorkQueue::InsertAtHeadOfList(WorkPackage & wp)
void WorkQueue::InsertAtHeadOfList(WorkPackage & wp)
{
  MutexLocker queueMutexLocker(queueMutex);

  if (pQueueLast == nullptr)
  {
    // (queue empty)

    queueConVar.Signal();

    CheckStateAndSetToInQ_static(wp);

    wp.pNext = nullptr;
    wp.pPrev = nullptr;

    pQueueFirst = &wp;
    pQueueLast  = &wp;
  }
  else
  {
    // (queue not empty)

    CheckStateAndSetToInQ_static(wp);

    wp.pPrev = nullptr;
    wp.pNext = pQueueFirst;

    pQueueFirst->pPrev = &wp;
    pQueueFirst = &wp;
  }
}

/// \copydoc IWorkQueue::Remove(WorkPackage & wp)
void WorkQueue::Remove(WorkPackage & wp)
{
  // ensure that wp is a static work package
  {
    WorkPackage::States const currState = wp.state;
    if ((currState != WorkPackage::States::staticNotInQ) &&
        (currState != WorkPackage::States::staticInQ) &&
        (currState != WorkPackage::States::staticExec) &&
        (currState != WorkPackage::States::staticExecInQ))
      throw std::invalid_argument("WorkQueue::Remove: &wp is dynamic");
  }

  MutexLocker queueMutexLocker(queueMutex);

  if (wp.state == WorkPackage::States::staticExec)
    return;

  auto pWP = pQueueFirst;
  while (pWP != nullptr)
  {
    if (pWP == &wp)
    {
      if (pWP->pPrev != nullptr)
        pWP->pPrev->pNext = pWP->pNext;
      else
        pQueueFirst = pWP->pNext;

      if (pWP->pNext != nullptr)
        pWP->pNext->pPrev = pWP->pPrev;
      else
        pQueueLast = pWP->pPrev;

      Release(pWP);
      return;
    }
    else
      pWP = pWP->pNext;
  }
}

/// \copydoc IWorkQueue::Remove(void const * const pOwnerObject)
void WorkQueue::Remove(void const * const pOwnerObject)
{
  MutexLocker queueMutexLocker(queueMutex);

  auto pWP = pQueueFirst;
  while (pWP != nullptr)
  {
    if (pWP->pOwnerObject == pOwnerObject)
    {
      if (pWP->pPrev != nullptr)
        pWP->pPrev->pNext = pWP->pNext;
      else
        pQueueFirst = pWP->pNext;

      if (pWP->pNext != nullptr)
        pWP->pNext->pPrev = pWP->pPrev;
      else
        pQueueLast = pWP->pPrev;

      auto toBeReleased = pWP;
      pWP = pWP->pNext;
      Release(toBeReleased);
    }
    else
      pWP = pWP->pNext;
  }
}

/// \copydoc IWorkQueue::Remove(void const * const pOwnerObject, uint32_t const ownerID)
void WorkQueue::Remove(void const * const pOwnerObject, uint32_t const ownerID)
{
  MutexLocker queueMutexLocker(queueMutex);

  auto pWP = pQueueFirst;
  while (pWP != nullptr)
  {
    if ((pWP->pOwnerObject == pOwnerObject) && (pWP->ownerID == ownerID))
    {
      if (pWP->pPrev != nullptr)
        pWP->pPrev->pNext = pWP->pNext;
      else
        pQueueFirst = pWP->pNext;

      if (pWP->pNext != nullptr)
        pWP->pNext->pPrev = pWP->pPrev;
      else
        pQueueLast = pWP->pPrev;

      auto toBeReleased = pWP;
      pWP = pWP->pNext;
      Release(toBeReleased);
    }
    else
      pWP = pWP->pNext;
  }
}

/// \copydoc IWorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted
void WorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted(void const * const pOwnerObject) const
{
  if (pOwnerObject == nullptr)
    throw std::invalid_argument("WorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted: !pOwnerObject");

  MutexLocker queueMutexLocker(queueMutex);

  while (pOwnerOfCurrentExecutedWP == pOwnerObject)
    ownerChangedConVar.Wait(queueMutex);
}

/// \copydoc IWorkQueue::IsAnyInQueue
bool WorkQueue::IsAnyInQueue(void const * const pOwnerObject) const
{
  MutexLocker queueMutexLocker(queueMutex);

  auto pWP = pQueueFirst;
  while (pWP != nullptr)
  {
    if (pWP->pOwnerObject == pOwnerObject)
      return true;
    pWP = pWP->pNext;
  }

  return false;
}

/// \copydoc IWorkQueue::FlushNonDeferredWorkPackages
void WorkQueue::FlushNonDeferredWorkPackages(void)
{
  Semaphore s(0);
  Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Semaphore::Post, &s)));

  try
  {
    s.Wait();

    // lock flushMutex to ensure that the invocation of the work package's functor is complete
    flushMutex.Lock();
    flushMutex.Unlock();
  }
  catch (...)
  {
    // If s.Wait() or the mutex lock/unlock fails, then we would leave and "s" would be released.
    // This is bad, because the work package referencing s.Post() is either still in the work queue or
    // the execution of the work package's functor might not have been completed yet.
    PANIC();

    // never returns, but makes compiler happy
    throw;
  }
}
// <--

/**
 * \brief Executes work packages until termination is requested.
 *
 * If there is a pending request for termination when this is invoked (see @ref RequestTermination()), then the request
 * will be consumed and this will return immediately.
 *
 * If termination is requested while a thread is inside this method, then the request will be consumed and this method
 * will return either immediately or after execution of the current work package has finished.
 *
 * - - -
 *
 * __Thread safety:__\n
 * There must be no more than one thread executing this method at any time.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * _Note: Exceptions thrown by executed work packages are not caught._
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void WorkQueue::Work(void)
{
  AdvancedMutexLocker queueMutexLocker(queueMutex);

  while (true)
  {
    // clear pOwnerOfCurrentExecutedWP if queue is empty
    if ((pOwnerOfCurrentExecutedWP != nullptr) && (pQueueFirst == nullptr))
    {
      ownerChangedConVar.Broadcast();
      pOwnerOfCurrentExecutedWP = nullptr;
    }

    // wait for a work package or a termination request
    while ((pQueueFirst == nullptr) && (!terminate))
      queueConVar.Wait(queueMutex);

    // terminate?
    if (terminate)
    {
      if (pOwnerOfCurrentExecutedWP != nullptr)
      {
        ownerChangedConVar.Broadcast();
        pOwnerOfCurrentExecutedWP = nullptr;
      }

      terminate = false;
      return;
    }

    // fetch work package from queue, but do not remove it yet
    auto pWP = pQueueFirst;

    // work package is about to be executed, so update pOwnerOfCurrentExecutedWP
    pOwnerOfCurrentExecutedWP = pWP->pOwnerObject;
    ON_SCOPE_EXIT(owner) { pOwnerOfCurrentExecutedWP = nullptr; };
    ownerChangedConVar.Broadcast();

    // remove work package from queue
    pQueueFirst = pWP->pNext;
    if (pQueueFirst != nullptr)
      pQueueFirst->pPrev = nullptr;
    else
      pQueueLast = nullptr;

    // update work package's state and prepare for execution
    if (pWP->state == WorkPackage::States::staticInQ)
      pWP->state = WorkPackage::States::staticExec;
    pCurrentExecutedWP = pWP;

    queueMutexLocker.Unlock();

    ON_SCOPE_EXIT(execStateAndBackInQ)
    {
      queueMutexLocker.Relock();

      // restore work package's state and undo preparation
      pCurrentExecutedWP = nullptr;
      if (pWP->state == WorkPackage::States::staticExec)
        pWP->state = WorkPackage::States::staticInQ;

      // put the work package back into the queue
      if (pQueueFirst != nullptr)
        pQueueFirst->pPrev = pWP;
      pQueueFirst = pWP;
      if (pQueueLast == nullptr)
        pQueueLast = pWP;
    };

    flushMutex.Lock();

    ON_SCOPE_EXIT_DISMISS(execStateAndBackInQ);
    ON_SCOPE_EXIT_DISMISS(owner);

    // finally execute the work package
    ON_SCOPE_EXIT(afterExecWP)
    {
      flushMutex.Unlock();
      queueMutexLocker.Relock();
      Finish(pWP);
      pCurrentExecutedWP = nullptr;
    };

    pWP->functor();
  } // while (true)
}

/**
 * \brief Requests abort of work package execution.
 *
 * If the queue is empty, then the thread inside @ref Work() will consume the request and return immediately. Otherwise
 * @ref Work() will consume the request and return after execution of the current work package has finished.
 *
 * If there is currently no thread in @ref Work(), then the request will be consumed when a thread enters @ref Work().
 * The thread will then return from @ref Work() immediately.
 *
 * If there is a pending abort request, then this method has no effect.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void WorkQueue::RequestTermination(void) noexcept
{
  try
  {
    MutexLocker queueMutexLocker(queueMutex);

    queueConVar.Signal();
    terminate = true;
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Checks the state of an @ref WorkPackage (static), which shall be enqueued into the
 * work queue and sets the work package's state to the proper "in-Q" state.
 *
 * __Thread-safety:__\n
 * @ref queueMutex must be locked.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param wp
 * Reference to the work package.
 */
void WorkQueue::CheckStateAndSetToInQ_static(WorkPackage& wp) const
{
  if (pCurrentExecutedWP != &wp)
  {
    auto expected = WorkPackage::States::staticNotInQ;
    if (!wp.state.compare_exchange_strong(expected, WorkPackage::States::staticInQ))
      throw std::logic_error("WorkQueue::CheckStateAndSetToInQ_static: Bad WP state");
  }
  else
  {
    auto expected = WorkPackage::States::staticExec;
    if (!wp.state.compare_exchange_strong(expected, WorkPackage::States::staticExecInQ))
      throw std::logic_error("WorkQueue::CheckStateAndSetToInQ_static: Bad WP state");
  }
}

/**
 * \brief Checks the state of an @ref WorkPackage (dynamic), which shall be enqueued into the
 * work queue and sets the work package's state to the proper "in-Q" state.
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
 * \param wp
 * Reference to the work package.
 */
void WorkQueue::CheckStateAndSetToInQ_dynamic(WorkPackage& wp) const noexcept
{
  auto expected = WorkPackage::States::dynamicNotInQ;
  if (!wp.state.compare_exchange_strong(expected, WorkPackage::States::dynamicInQ))
    Panic("WorkQueue::CheckStateAndSetToInQ_dynamic: Bad WP state");
}

/**
 * \brief Releases a @ref WorkPackage instance which is enqueued in the work queue.
 *
 * __Thread-safety:__\n
 * @ref queueMutex must be locked.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pWP
 * Pointer to the @ref WorkPackage that shall be released.
 */
void WorkQueue::Release(WorkPackage* const pWP) noexcept
{
  switch (pWP->state)
  {
    case WorkPackage::States::staticInQ:
      pWP->state = WorkPackage::States::staticNotInQ;
      break;

    case WorkPackage::States::staticExecInQ:
      pWP->state = WorkPackage::States::staticExec;
      break;

    case WorkPackage::States::dynamicInQ:
      pWP->state = WorkPackage::States::dynamicNotInQ;
      delete pWP;
      break;

    // case WorkPackage::States::staticNotInQ:
    // case WorkPackage::States::staticExec:
    // case WorkPackage::States::dynamicNotInQ:
    default:
      Panic("WorkQueue::Release: Bad WP state");
  };
}

/**
 * \brief Releases a @ref WorkPackage instance after execution.
 *
 * __Thread-safety:__\n
 * @ref queueMutex must be locked.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pWP
 * Pointer to the @ref WorkPackage that shall be released.
 */
void WorkQueue::Finish(WorkPackage* const pWP) noexcept
{
  switch (pWP->state)
  {
    case WorkPackage::States::staticExec:
      pWP->state = WorkPackage::States::staticNotInQ;
      break;

    case WorkPackage::States::staticExecInQ:
      pWP->state = WorkPackage::States::staticInQ;
      break;

    case WorkPackage::States::dynamicInQ:
      pWP->state = WorkPackage::States::dynamicNotInQ;
      delete pWP;
      break;

    // case WorkPackage::States::staticNotInQ:
    // case WorkPackage::States::staticInQ:
    // case WorkPackage::States::dynamicNotInQ:
    default:
      Panic("WorkQueue::Finish: Bad WP state");
  };
}

} // namespace async
} // namespace execution
} // namespace gpcc
