/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "DeferredWorkQueue.hpp"
#include "DeferredWorkPackage.hpp"
#include "WorkPackage.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Semaphore.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <stdexcept>

namespace gpcc {
namespace execution {
namespace async {

using namespace gpcc::osal;
using namespace gpcc::time;

/**
 * \brief Constructor.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 */
DeferredWorkQueue::DeferredWorkQueue(void)
: queueMutex()
, flushMutex()
, queueConVar()
, pQueueFirst(nullptr)
, pQueueLast(nullptr)
, pDeferredQueueFirst(nullptr)
, pDeferredQueueLast(nullptr)
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
DeferredWorkQueue::~DeferredWorkQueue(void)
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

    auto pDWP = pDeferredQueueFirst;
    while (pDWP != nullptr)
    {
      auto toBeReleased = pDWP;
      pDWP = pDWP->pNext;
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
void DeferredWorkQueue::Add(std::unique_ptr<WorkPackage> spWP)
{
  if (!spWP)
    throw std::invalid_argument("DeferredWorkQueue::Add: !spWP");

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
void DeferredWorkQueue::Add(WorkPackage & wp)
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
void DeferredWorkQueue::InsertAtHeadOfList(std::unique_ptr<WorkPackage> spWP)
{
  if (!spWP)
    throw std::invalid_argument("DeferredWorkQueue::InsertAtHeadOfList: !spWP");

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
void DeferredWorkQueue::InsertAtHeadOfList(WorkPackage & wp)
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
void DeferredWorkQueue::Remove(WorkPackage & wp)
{
  // ensure that wp is a static work package
  {
    WorkPackage::States const currState = wp.state;
    if ((currState != WorkPackage::States::staticNotInQ) &&
        (currState != WorkPackage::States::staticInQ) &&
        (currState != WorkPackage::States::staticExec) &&
        (currState != WorkPackage::States::staticExecInQ))
      throw std::invalid_argument("DeferredWorkQueue::Remove: &wp is dynamic");
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
void DeferredWorkQueue::Remove(void const * const pOwnerObject)
{
  MutexLocker queueMutexLocker(queueMutex);

  // normal queue
  {
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

  // deferred queue
  {
    auto pDWP = pDeferredQueueFirst;
    bool timeoutUpdateRequired = false;
    while (pDWP != nullptr)
    {
      if (pDWP->pOwnerObject == pOwnerObject)
      {
        if (pDWP->pPrev != nullptr)
          pDWP->pPrev->pNext = pDWP->pNext;
        else
        {
          pDeferredQueueFirst = pDWP->pNext;
          timeoutUpdateRequired = true;
        }

        if (pDWP->pNext != nullptr)
          pDWP->pNext->pPrev = pDWP->pPrev;
        else
          pDeferredQueueLast = pDWP->pPrev;

        auto toBeReleased = pDWP;
        pDWP = pDWP->pNext;
        Release(toBeReleased);
      }
      else
        pDWP = pDWP->pNext;
    }

    if ((timeoutUpdateRequired) && (pDeferredQueueFirst != nullptr))
      queueConVar.Signal();
  }
}

/// \copydoc IWorkQueue::Remove(void const * const pOwnerObject, uint32_t const ownerID)
void DeferredWorkQueue::Remove(void const * const pOwnerObject, uint32_t const ownerID)
{
  MutexLocker queueMutexLocker(queueMutex);

  // normal queue
  {
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

  // deferred queue
  {
    auto pDWP = pDeferredQueueFirst;
    bool timeoutUpdateRequired = false;
    while (pDWP != nullptr)
    {
      if ((pDWP->pOwnerObject == pOwnerObject) && (pDWP->ownerID == ownerID))
      {
        if (pDWP->pPrev != nullptr)
          pDWP->pPrev->pNext = pDWP->pNext;
        else
        {
          pDeferredQueueFirst = pDWP->pNext;
          timeoutUpdateRequired = true;
        }

        if (pDWP->pNext != nullptr)
          pDWP->pNext->pPrev = pDWP->pPrev;
        else
          pDeferredQueueLast = pDWP->pPrev;

        auto toBeReleased = pDWP;
        pDWP = pDWP->pNext;
        Release(toBeReleased);
      }
      else
        pDWP = pDWP->pNext;
    }

    if ((timeoutUpdateRequired) && (pDeferredQueueFirst != nullptr))
      queueConVar.Signal();
  }
}

/// \copydoc IWorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted
void DeferredWorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted(void const * const pOwnerObject) const
{
  if (pOwnerObject == nullptr)
    throw std::invalid_argument("DeferredWorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted: !pOwnerObject");

  MutexLocker queueMutexLocker(queueMutex);

  while (pOwnerOfCurrentExecutedWP == pOwnerObject)
    ownerChangedConVar.Wait(queueMutex);
}

/// \copydoc IWorkQueue::IsAnyInQueue
bool DeferredWorkQueue::IsAnyInQueue(void const * const pOwnerObject) const
{
  MutexLocker queueMutexLocker(queueMutex);

  // normal queue
  {
    auto pWP = pQueueFirst;
    while (pWP != nullptr)
    {
      if (pWP->pOwnerObject == pOwnerObject)
        return true;
      pWP = pWP->pNext;
    }
  }

  // deferred queue
  {
    auto pDWP = pDeferredQueueFirst;
    while (pDWP != nullptr)
    {
      if (pDWP->pOwnerObject == pOwnerObject)
        return true;
      pDWP = pDWP->pNext;
    }
  }

  return false;
}

/// \copydoc IWorkQueue::FlushNonDeferredWorkPackages
void DeferredWorkQueue::FlushNonDeferredWorkPackages(void)
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

// --> Interface required by IDeferredWorkQueue
/// \copydoc IDeferredWorkQueue::Add(std::unique_ptr<DeferredWorkPackage> spDWP)
void DeferredWorkQueue::Add(std::unique_ptr<DeferredWorkPackage> spDWP)
{
  if (!spDWP)
    throw std::invalid_argument("DeferredWorkQueue::Add: !spDWP");

  MutexLocker queueMutexLocker(queueMutex);

  // Note: The queue for deferred work packages (pDeferredQueueFirst and pDeferredQueueLast)
  // is sorted by blocking time in ascending order.
  // spDWP must be inserted at the proper position.

  if (pDeferredQueueLast == nullptr)
  {
    // (queue empty)

    queueConVar.Signal();

    CheckStateAndSetToInQ_dynamic(*spDWP);

    spDWP->pNext = nullptr;
    spDWP->pPrev = nullptr;

    pDeferredQueueFirst = spDWP.get();
    pDeferredQueueLast  = spDWP.release();
  }
  else
  {
    // (queue not empty)

    // iterate to the deferred work package behind which spDWP must be inserted
    auto pDWP = pDeferredQueueLast;
    while ((pDWP != nullptr) && (pDWP->tp > spDWP->tp))
      pDWP = pDWP->pPrev;

    // insert in front of the first element?
    if (pDWP == nullptr)
    {
      // Insert spDWP in front of the first element and signal queueConVar in order to
      // setup a new timeout. This is necessary because the current timeout is larger
      // than the one required by spDWP.

      queueConVar.Signal();

      CheckStateAndSetToInQ_dynamic(*spDWP);

      spDWP->pNext = pDeferredQueueFirst;
      spDWP->pPrev = nullptr;

      pDeferredQueueFirst->pPrev = spDWP.get();
      pDeferredQueueFirst = spDWP.release();
    }
    // ...insert at end of list?
    else if (pDWP == pDeferredQueueLast)
    {
      CheckStateAndSetToInQ_dynamic(*spDWP);

      spDWP->pNext = nullptr;
      spDWP->pPrev = pDeferredQueueLast;

      pDeferredQueueLast->pNext = spDWP.get();
      pDeferredQueueLast = spDWP.release();
    }
    // ...insert somewhere in the list
    else
    {
      CheckStateAndSetToInQ_dynamic(*spDWP);

      spDWP->pNext = pDWP->pNext;
      spDWP->pPrev = pDWP;

      spDWP->pNext->pPrev = spDWP.get();
      pDWP->pNext = spDWP.release();
    }
  }
}

/// \copydoc IDeferredWorkQueue::Add(DeferredWorkPackage & dwp)
void DeferredWorkQueue::Add(DeferredWorkPackage & dwp)
{
  MutexLocker queueMutexLocker(queueMutex);

  // Note: The queue for deferred work packages (pDeferredQueueFirst and pDeferredQueueLast)
  // is sorted by blocking time in ascending order.
  // spDWP must be inserted at the proper position.

  if (pDeferredQueueLast == nullptr)
  {
    // (queue empty)

    queueConVar.Signal();

    CheckStateAndSetToInQ_static(dwp);

    dwp.pNext = nullptr;
    dwp.pPrev = nullptr;

    pDeferredQueueFirst = &dwp;
    pDeferredQueueLast  = &dwp;
  }
  else
  {
    // (queue not empty)

    // iterate to the deferred work package behind which dwp must be inserted
    auto pDWP = pDeferredQueueLast;
    while ((pDWP != nullptr) && (pDWP->tp > dwp.tp))
      pDWP = pDWP->pPrev;

    // insert in front of the first element?
    if (pDWP == nullptr)
    {
      // Insert dwp in front of the first element and signal queueConVar in order to
      // setup a new timeout. This is necessary because the current timeout is larger
      // than the one required by dwp.

      queueConVar.Signal();

      CheckStateAndSetToInQ_static(dwp);

      dwp.pNext = pDeferredQueueFirst;
      dwp.pPrev = nullptr;

      pDeferredQueueFirst->pPrev = &dwp;
      pDeferredQueueFirst = &dwp;
    }
    // ...insert at end of list?
    else if (pDWP == pDeferredQueueLast)
    {
      CheckStateAndSetToInQ_static(dwp);

      dwp.pNext = nullptr;
      dwp.pPrev = pDeferredQueueLast;

      pDeferredQueueLast->pNext = &dwp;
      pDeferredQueueLast = &dwp;
    }
    // ...insert somewhere in the list
    else
    {
      CheckStateAndSetToInQ_static(dwp);

      dwp.pNext = pDWP->pNext;
      dwp.pPrev = pDWP;

      dwp.pNext->pPrev = &dwp;
      pDWP->pNext = &dwp;
    }
  }
}

/// \copydoc IDeferredWorkQueue::Remove(DeferredWorkPackage & dwp)
void DeferredWorkQueue::Remove(DeferredWorkPackage & dwp)
{
  // ensure that dwp is a static work package
  {
    DeferredWorkPackage::States const currState = dwp.state;
    if ((currState != DeferredWorkPackage::States::staticNotInQ) &&
        (currState != DeferredWorkPackage::States::staticInQ) &&
        (currState != DeferredWorkPackage::States::staticExec) &&
        (currState != DeferredWorkPackage::States::staticExecInQ))
      throw std::invalid_argument("DeferredWorkQueue::Remove: &dwp is dynamic");
  }

  MutexLocker queueMutexLocker(queueMutex);

  if (dwp.state == DeferredWorkPackage::States::staticExec)
    return;

  auto pDWP = pDeferredQueueFirst;
  bool timeoutUpdateRequired = false;
  while (pDWP != nullptr)
  {
    if (pDWP == &dwp)
    {
      if (pDWP->pPrev != nullptr)
        pDWP->pPrev->pNext = pDWP->pNext;
      else
      {
        pDeferredQueueFirst = pDWP->pNext;
        timeoutUpdateRequired = true;
      }

      if (pDWP->pNext != nullptr)
        pDWP->pNext->pPrev = pDWP->pPrev;
      else
        pDeferredQueueLast = pDWP->pPrev;

      Release(pDWP);
      if ((timeoutUpdateRequired) && (pDeferredQueueFirst != nullptr))
        queueConVar.Signal();
      return;
    }
    else
      pDWP = pDWP->pNext;
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
void DeferredWorkQueue::Work(void)
{
  AdvancedMutexLocker queueMutexLocker(queueMutex);

  while (true)
  {
    bool timeout = false;

    // deferred queue empty?
    if (pDeferredQueueFirst == nullptr)
    {
      // clear pOwnerOfCurrentExecutedWP if non-deferred queue is empty
      if ((pOwnerOfCurrentExecutedWP != nullptr) && (pQueueFirst == nullptr))
      {
        ownerChangedConVar.Broadcast();
        pOwnerOfCurrentExecutedWP = nullptr;
      }

      // wait for a work package (deferred and normal) or a termination request
      while ((pQueueFirst == nullptr) && (pDeferredQueueFirst == nullptr) && (!terminate))
        queueConVar.Wait(queueMutex);
    }
    else
    {
      // deferred work packages ready for execution shall have priority above normal work packages
      if ((pQueueFirst != nullptr) &&
          (pDeferredQueueFirst->tp <= TimePoint::FromSystemClock(Clocks::monotonic)))
        timeout = true;

      // Clear pOwnerOfCurrentExecutedWP if non-deferred queue is empty and if deferred work package
      // has not yet reached its timeout.
      if ((pOwnerOfCurrentExecutedWP != nullptr) && (pQueueFirst == nullptr) && (!timeout))
      {
        ownerChangedConVar.Broadcast();
        pOwnerOfCurrentExecutedWP = nullptr;
      }

      // wait for a normal work package, or timeout of deferred work package or a termination request
      while ((!timeout) && (pQueueFirst == nullptr) && (pDeferredQueueFirst != nullptr) && (!terminate))
      {
        timeout = queueConVar.TimeLimitedWait(queueMutex, pDeferredQueueFirst->tp);

        // "timeout" is part of the convar's predicate we are waiting for. Double check is required
        // because pDeferredQueueFirst may have changed while waiting.
        if ((timeout) &&
            (pDeferredQueueFirst != nullptr) &&
            (pDeferredQueueFirst->tp > TimePoint::FromSystemClock(Clocks::monotonic)))
          timeout = false;
      }
    }

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

    // fetch a work package from queue, but do not remove it yet
    DeferredWorkPackage* pDWP = nullptr;
    WorkPackage* pWP = nullptr;
    if ((timeout) && (pDeferredQueueFirst != nullptr))
    {
      pDWP = pDeferredQueueFirst;
    }
    else
    {
      if (pQueueFirst != nullptr)
        pWP = pQueueFirst;
      else
      {
        // We have been woken up to setup a new timeout or there was only one deferred work package
        // and it has been removed from the queue.
        continue;
      }
    }

    // work package is about to be executed, so update pOwnerOfCurrentExecutedWP
    if (pWP != nullptr)
      pOwnerOfCurrentExecutedWP = pWP->pOwnerObject;
    else
      pOwnerOfCurrentExecutedWP = pDWP->pOwnerObject;
    ON_SCOPE_EXIT(owner) { pOwnerOfCurrentExecutedWP = nullptr; };
    ownerChangedConVar.Broadcast();

    if (pWP != nullptr)
    {
      // remove work package from normal queue
      pQueueFirst = pWP->pNext;
      if (pQueueFirst != nullptr)
        pQueueFirst->pPrev = nullptr;
      else
        pQueueLast = nullptr;

      // update work package's state and prepare for execution
      if (pWP->state == WorkPackage::States::staticInQ)
        pWP->state = WorkPackage::States::staticExec;
      pCurrentExecutedWP = pWP;
    }
    else
    {
      // remove work package from deferred queue
      pDeferredQueueFirst = pDWP->pNext;
      if (pDeferredQueueFirst != nullptr)
        pDeferredQueueFirst->pPrev = nullptr;
      else
        pDeferredQueueLast = nullptr;

      // update work package's state and prepare for execution
      if (pDWP->state == DeferredWorkPackage::States::staticInQ)
        pDWP->state = DeferredWorkPackage::States::staticExec;
      pCurrentExecutedWP = pDWP;
    }

    queueMutexLocker.Unlock();

    ON_SCOPE_EXIT(execStateAndBackInQ)
    {
      queueMutexLocker.Relock();

      if (pWP != nullptr)
      {
        // restore work package's state and undo preparation
        pCurrentExecutedWP = nullptr;
        if (pWP->state == WorkPackage::States::staticExec)
          pWP->state = WorkPackage::States::staticInQ;

        // put the work package back into the normal queue
        if (pQueueFirst != nullptr)
          pQueueFirst->pPrev = pWP;
        pQueueFirst = pWP;
        if (pQueueLast == nullptr)
          pQueueLast = pWP;
      }
      else
      {
        // restore work package's state and undo preparation
        pCurrentExecutedWP = nullptr;
        if (pDWP->state == DeferredWorkPackage::States::staticExec)
          pDWP->state = DeferredWorkPackage::States::staticInQ;

        // put the work package back into the deferred queue
        if (pDeferredQueueFirst != nullptr)
          pDeferredQueueFirst->pPrev = pDWP;
        pDeferredQueueFirst = pDWP;
        if (pDeferredQueueLast == nullptr)
          pDeferredQueueLast = pDWP;
      }
    };

    flushMutex.Lock();

    ON_SCOPE_EXIT_DISMISS(execStateAndBackInQ);
    ON_SCOPE_EXIT_DISMISS(owner);

    // finally execute the work package
    if (pWP != nullptr)
    {
      ON_SCOPE_EXIT(afterExecWP)
      {
        flushMutex.Unlock();
        queueMutexLocker.Relock();
        Finish(pWP);
        pCurrentExecutedWP = nullptr;
      };

      pWP->functor();
    }
    else
    {
      ON_SCOPE_EXIT(afterExecDWP)
      {
        flushMutex.Unlock();
        queueMutexLocker.Relock();
        Finish(pDWP);
        pCurrentExecutedWP = nullptr;
      };

      pDWP->functor();
    }
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
void DeferredWorkQueue::RequestTermination(void) noexcept
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
void DeferredWorkQueue::CheckStateAndSetToInQ_static(WorkPackage& wp) const
{
  if (pCurrentExecutedWP != &wp)
  {
    auto expected = WorkPackage::States::staticNotInQ;
    if (!wp.state.compare_exchange_strong(expected, WorkPackage::States::staticInQ))
      throw std::logic_error("DeferredWorkQueue::CheckStateAndSetToInQ_static: Bad WP state");
  }
  else
  {
    auto expected = WorkPackage::States::staticExec;
    if (!wp.state.compare_exchange_strong(expected, WorkPackage::States::staticExecInQ))
      throw std::logic_error("DeferredWorkQueue::CheckStateAndSetToInQ_static: Bad WP state");
  }
}

/**
 * \brief Checks the state of an @ref DeferredWorkPackage (static), which shall be enqueued into the
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
 * \param dwp
 * Reference to the deferred work package.
 */
void DeferredWorkQueue::CheckStateAndSetToInQ_static(DeferredWorkPackage& dwp) const
{
  if (pCurrentExecutedWP != &dwp)
  {
    auto expected = DeferredWorkPackage::States::staticNotInQ;
    if (!dwp.state.compare_exchange_strong(expected, DeferredWorkPackage::States::staticInQ))
      throw std::logic_error("DeferredWorkQueue::CheckStateAndSetToInQ_static: Bad DWP state");
  }
  else
  {
    auto expected = DeferredWorkPackage::States::staticExec;
    if (!dwp.state.compare_exchange_strong(expected, DeferredWorkPackage::States::staticExecInQ))
      throw std::logic_error("DeferredWorkQueue::CheckStateAndSetToInQ_static: Bad DWP state");
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
void DeferredWorkQueue::CheckStateAndSetToInQ_dynamic(WorkPackage& wp) const noexcept
{
  auto expected = WorkPackage::States::dynamicNotInQ;
  if (!wp.state.compare_exchange_strong(expected, WorkPackage::States::dynamicInQ))
    Panic("DeferredWorkQueue::CheckStateAndSetToInQ_dynamic: Bad WP state");
}

/**
 * \brief Checks the state of an @ref DeferredWorkPackage (dynamic), which shall be enqueued into the
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
 * \param dwp
 * Reference to the deferred work package.
 */
void DeferredWorkQueue::CheckStateAndSetToInQ_dynamic(DeferredWorkPackage& dwp) const noexcept
{
  auto expected = DeferredWorkPackage::States::dynamicNotInQ;
  if (!dwp.state.compare_exchange_strong(expected, DeferredWorkPackage::States::dynamicInQ))
    Panic("DeferredWorkQueue::CheckStateAndSetToInQ_dynamic: Bad DWP state");
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
void DeferredWorkQueue::Release(WorkPackage* const pWP) noexcept
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
      Panic("DeferredWorkQueue::Release: Bad WP state");
  };
}

/**
 * \brief Releases a @ref DeferredWorkPackage instance which is enqueued in the work queue.
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
 * \param pDWP
 * Pointer to the @ref DeferredWorkPackage that shall be released.
 */
void DeferredWorkQueue::Release(DeferredWorkPackage* const pDWP) noexcept
{
  switch (pDWP->state)
  {
    case DeferredWorkPackage::States::staticInQ:
      pDWP->state = DeferredWorkPackage::States::staticNotInQ;
      break;

    case DeferredWorkPackage::States::staticExecInQ:
      pDWP->state = DeferredWorkPackage::States::staticExec;
      break;

    case DeferredWorkPackage::States::dynamicInQ:
      pDWP->state = DeferredWorkPackage::States::dynamicNotInQ;
      delete pDWP;
      break;

    // case DeferredWorkPackage::States::staticNotInQ:
    // case DeferredWorkPackage::States::staticExec:
    // case DeferredWorkPackage::States::dynamicNotInQ:
    default:
      Panic("DeferredWorkQueue::Release: Bad DWP state");
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
void DeferredWorkQueue::Finish(WorkPackage* const pWP) noexcept
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
      Panic("DeferredWorkQueue::Finish: Bad WP state");
  };
}

/**
 * \brief Releases a @ref DeferredWorkPackage instance after execution.
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
 * \param pDWP
 * Pointer to the @ref DeferredWorkPackage that shall be released.
 */
void DeferredWorkQueue::Finish(DeferredWorkPackage* const pDWP) noexcept
{
  switch (pDWP->state)
  {
    case DeferredWorkPackage::States::staticExec:
      pDWP->state = DeferredWorkPackage::States::staticNotInQ;
      break;

    case DeferredWorkPackage::States::staticExecInQ:
      pDWP->state = DeferredWorkPackage::States::staticInQ;
      break;

    case DeferredWorkPackage::States::dynamicInQ:
      pDWP->state = DeferredWorkPackage::States::dynamicNotInQ;
      delete pDWP;
      break;

    // case DeferredWorkPackage::States::staticNotInQ:
    // case DeferredWorkPackage::States::staticInQ:
    // case DeferredWorkPackage::States::dynamicNotInQ:
    default:
      Panic("DeferredWorkQueue::Finish: Bad DWP state");
  };
}

} // namespace async
} // namespace execution
} // namespace gpcc
