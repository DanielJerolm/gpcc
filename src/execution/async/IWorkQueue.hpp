/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IWORKQUEUE_HPP_201612212242
#define IWORKQUEUE_HPP_201612212242

#include <memory>
#include <cstdint>

namespace gpcc {
namespace execution {
namespace async {

class WorkPackage;

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Interface for work queues.
 *
 * Unless otherwise noted, the methods offered by this interface (except for Add... and Insert...)
 * apply to both work packages and _deferred_ work packages.
 *
 * For details, please refer to the work queue implementation and the deferred work queue implementation,
 * classes @ref DeferredWorkQueue and @ref WorkQueue.
 *
 * Regarding exception safety, all methods adding work packages offer the strong guarantee.
 * However, these methods do only fail in case of serious errors related to mutex locking
 * or passing invalid parameters.
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IWorkQueue
{
  public:
    IWorkQueue(void) = default;
    IWorkQueue(IWorkQueue const &) = delete;
    IWorkQueue(IWorkQueue&&) = delete;

    IWorkQueue& operator=(IWorkQueue const &) = delete;
    IWorkQueue& operator=(IWorkQueue&&) = delete;

    virtual void Add(std::unique_ptr<WorkPackage> spWP) = 0;
    virtual void Add(WorkPackage & wp) = 0;
    virtual void InsertAtHeadOfList(std::unique_ptr<WorkPackage> spWP) = 0;
    virtual void InsertAtHeadOfList(WorkPackage & wp) = 0;
    virtual void Remove(WorkPackage & wp) = 0;
    virtual void Remove(void const * const pOwnerObject) = 0;
    virtual void Remove(void const * const pOwnerObject, uint32_t const ownerID) = 0;
    virtual void WaitUntilCurrentWorkPackageHasBeenExecuted(void const * const pOwnerObject) const = 0;
    virtual bool IsAnyInQueue(void const * const pOwnerObject) const = 0;
    virtual void FlushNonDeferredWorkPackages(void) = 0;

  protected:
    virtual ~IWorkQueue(void) = default;
};

/**
 * \fn virtual void IWorkQueue::Add(std::unique_ptr<WorkPackage> spWP)
 *
 * \brief Adds an _dynamic_ work package to the work queue.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * No memory/resource allocation related errors possible.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param spWP
 * Pointer to the work package that shall be added to the work queue.\n
 * _The work package must be a dynamic work package._\n
 * _This means that ownership moves from the caller to the work queue,_
 * _and the work queue will finally release the work package._
 */

/**
 * \fn virtual void IWorkQueue::Add(WorkPackage & wp)
 *
 * \brief Adds an _static_ work package to the work queue.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * No memory/resource allocation related errors possible.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param wp
 * Reference to the work package that shall be added to the work queue.\n
 * _The work package must be a static work package._\n
 * _This means that ownership remains at the caller,_
 * _and the caller will finally release the work package._
 */

/**
 * \fn virtual void IWorkQueue::InsertAtHeadOfList(std::unique_ptr<WorkPackage> spWP)
 *
 * \brief Inserts an _dynamic_ work package at the head of the work queue.
 *
 * The work package is added to the head of the queue. This means, that the work package will
 * be executed next, regardless if the work queue is empty or not.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * No memory/resource allocation related errors possible.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 * \param spWP
 * Pointer to the work package that shall be inserted at the head of the work queue.\n
 * _The work package must be a dynamic work package._\n
 * _This means that ownership moves from the caller to the work queue,_
 * _and the work queue will finally release the work package._
 */

/**
 * \fn virtual void IWorkQueue::InsertAtHeadOfList(WorkPackage & wp)
 *
 * \brief Inserts an _static_ work package at the head of the work queue.
 *
 * The work package is added to the head of the queue. This means, that the work package will
 * be executed next, regardless if the work queue is empty or not.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * No memory/resource allocation related errors possible.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 * \param wp
 * Reference to the work package that shall be inserted at the head of the work queue.\n
 * _The work package must be a static work package._\n
 * _This means that ownership remains at the caller,_
 * _and the caller will finally release the work package._
 */

/**
 * \fn virtual void IWorkQueue::Remove(WorkPackage & wp)
 *
 * \brief Removes an _static_ work package from the work queue.
 *
 * _Note: The currently executed work package is not affected by this method!_\n
 * If this __is__ executed (via a work package) in the context of __this__ work queue instance,
 * then it is guaranteed, that the work package `wp` is not left in the work queue when this method
 * returns.\n
 * If this __is not__ executed in the context of __this__ work queue instance, then the work package
 * `wp` may currently be executed by the work queue when this method returns.\n
 * _Therefore it is recommended to invoke this method from within the context_
 * _of this work queue only._
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param wp
 * Reference to the work package that shall be removed from the work queue.\n
 * _The work package must be a static work package._\n
 * _Static_ work packages must be finally released by their owner.
 */

/**
 * \fn virtual void IWorkQueue::Remove(void const * const pOwnerObject)
 *
 * \brief Removes all work packages of an specific owner from the work queue.
 *
 * This method does not care about the work package's ownerID. There is an overloaded
 * version of this method available that additionally considers the ownerID.
 *
 * _Note: The currently executed work package is not affected by this method!_\n
 * If this __is__ executed (via a work package) in the context of __this__ work queue instance,
 * then it is guaranteed, that no work package of the specified owner will be left in the work queue
 * when this method returns.\n
 * If this __is not__ executed in the context of __this__ work queue instance, then a work package of
 * the specified owner may currently be executed by the work queue when this method returns. After
 * calling this, you can invoke @ref WaitUntilCurrentWorkPackageHasBeenExecuted() in order
 * to wait until the last work package of the specified owner has been executed.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pOwnerObject
 * Pointer to the owner object. All work packages created by the specified owner are removed
 * from the work queue.\n
 * Removed _dynamic_ work packages will be released.\n
 * Removed _static_ work packages must be finally released by their owner.
 */

/**
 * \fn virtual void IWorkQueue::Remove(void const * const pOwnerObject, uint32_t const ownerID)
 *
 * \brief Removes all work packages of an specific owner and with an specific ownerID from the work queue.
 *
 * This method does care about the work package's ownerID. There is an overloaded
 * version of this method available that considers the owner object only.
 *
 * _Note: The currently executed work package is not affected by this method!_\n
 * If this __is__ executed (via a work package) in the context of __this__ work queue instance,
 * then it is guaranteed, that no work package of the specified owner and with the specified ownerID will
 * be left in the work queue when this method returns.\n
 * If this __is not__ executed in the context of __this__ work queue instance, then a work package of
 * the specified owner and with the specified ownerID may currently be executed by the work queue when this
 * method returns.\n
 * Note that @ref WaitUntilCurrentWorkPackageHasBeenExecuted() cannot be used in a reasonable way with this
 * version of this method.\n
 * _Therefore it is recommended to invoke this version of the method from within the context_
 * _of this work queue only._
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pOwnerObject
 * Pointer to the owner object. All work packages created by the given owner and
 * whose ownerID matches parameter `ownerID` are removed from the work queue.\n
 * Removed _dynamic_ work packages will be released.\n
 * Removed _static_ work packages must be finally released by their owner.
 * \param ownerID
 * OwnerID filter. All work packages created by the owner given by parameter `pOwnerObject` and
 * whose ownerID matches this value are removed from the work queue.\n
 * Removed _dynamic_ work packages will be released.\n
 * Removed _static_ work packages must be finally released by their owner.
 */

/**
 * \fn virtual void IWorkQueue::WaitUntilCurrentWorkPackageHasBeenExecuted(void const * const pOwnerObject)
 *
 * \brief Blocks the calling thread until the current work package has been executed. The current thread is only
 * blocked if there is a work package currently executed and if the work package belongs to an specific owner.
 *
 * This is intended to be invoked after @ref Remove(void const * const pOwnerObject) to ensure
 * that there is no work package of an specific owner currently being executed. After this method has returned, it
 * is e.g. safe to destroy the owner.
 *
 * If there are still work packages of the given owner in the work queue, then the point in time when this
 * method returns is not defined.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * Invoking this from the context of this work queue instance will result in a dead-lock if the currently
 * executed work package belongs to the specified owner (parameter `pOwnerObject`).\n
 * If the owner does not match, then it is safe to invoke this from within the context of this work queue,
 * though doing so makes no sense.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pOwnerObject
 * Pointer to the owner object.\n
 * _nullptr (anonymous owner) is not allowed._\n
 * The method will only block and wait for execution of the currently executed work package if...
 * - there is a work package currently executed
 * - this parameter matches the owner object of the currently executed work package.
 */

/**
 * \fn virtual bool IWorkQueue::IsAnyInQueue(void const * const pOwnerObject) const
 *
 * \brief Checks if any work package enqueued by an specific owner is still in the work queue.
 *
 * _The currently executed work package (if any) is NOT included in the check._
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param pOwnerObject
 * Pointer to the owner object.
 * \return
 * true  = There is at least one work package owned by `pOwnerObject` enqueued in the work queue.\n
 * false = There are no work packages owned by `pOwnerObject` enqueued in the work queue.
 */

/**
 * \fn virtual void IWorkQueue::FlushNonDeferredWorkPackages(void)
 *
 * \brief Blocks the calling thread until all work packages (_non deferred only!_) currently enqueued in
 * the work queue have been executed.
 *
 * Deferred work packages (if supported by the underlying work queue) and work packages added
 * while the thread is blocked are _not_ considered.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This must not be invoked in the context of this work queue instance.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */

} // namespace async
} // namespace execution
} // namespace gpcc

#endif /* IWORKQUEUE_HPP_201612212242 */
