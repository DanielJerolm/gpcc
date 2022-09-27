/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef IDEFERREDWORKQUEUE_HPP_201612212259
#define IDEFERREDWORKQUEUE_HPP_201612212259

#include <gpcc/execution/async/IWorkQueue.hpp>

namespace gpcc {
namespace execution {
namespace async {

class DeferredWorkPackage;

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Interface for deferred work queues.
 *
 * For details, please refer to the deferred work queue implementation, class @ref DeferredWorkQueue.
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
class IDeferredWorkQueue: public IWorkQueue
{
  public:
    IDeferredWorkQueue(void) = default;
    IDeferredWorkQueue(IDeferredWorkQueue const &) = delete;
    IDeferredWorkQueue(IDeferredWorkQueue&&) = delete;

    IDeferredWorkQueue& operator=(IDeferredWorkQueue const &) = delete;
    IDeferredWorkQueue& operator=(IDeferredWorkQueue&&) = delete;

    using IWorkQueue::Add;
    virtual void Add(std::unique_ptr<DeferredWorkPackage> spDWP) = 0;
    virtual void Add(DeferredWorkPackage & dwp) = 0;

    using IWorkQueue::Remove;
    virtual void Remove(DeferredWorkPackage & dwp) = 0;

  protected:
    virtual ~IDeferredWorkQueue(void) = default;
};

/**
 * \fn void IDeferredWorkQueue::Add(std::unique_ptr<gpcc::execution::async::DeferredWorkPackage> spDWP)
 *
 * \brief Adds an _dynamic_ deferred work package to the work queue.
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
 * \param spDWP
 * Pointer to the deferred work package that shall be added to the work queue.\n
 * _The work package must be a dynamic work package._\n
 * _This means that ownership moves from the caller to the work queue,_
 * _and the work queue will finally release the work package._
 */

/**
 * \fn void IDeferredWorkQueue::Add(gpcc::execution::async::DeferredWorkPackage & dwp)
 *
 * \brief Adds an _static_ deferred work package to the work queue.
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
 * \param dwp
 * Reference to the deferred work package that shall be added to the work queue.\n
 * _The work package must be a static work package._\n
 * _This means that ownership remains at the caller,_
 * _and the caller will finally release the work package._
 */

/**
 * \fn void IDeferredWorkQueue::Remove(gpcc::execution::async::DeferredWorkPackage & dwp)
 *
 * \brief Removes an _static_ deferred work package from the work queue.
 *
 * _Note: The currently executed work package is not affected by this method!_\n
 * If this __is__ executed (via a work package) in the context of __this__ work queue instance,
 * then it is guaranteed, that the work package `dwp` is not left in the work queue when this method
 * returns.\n
 * If this __is not__ executed in the context of __this__ work queue instance, then the work package
 * `dwp` may currently be executed by the work queue when this method returns.\n
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
 * \param dwp
 * Reference to the deferred work package that shall be removed from the work queue.\n
 * _The work package must be a static work package._\n
 * _Static_ work packages must be finally released by their owner.
 */

} // namespace async
} // namespace execution
} // namespace gpcc

#endif /* IDEFERREDWORKQUEUE_HPP_201612212259 */
