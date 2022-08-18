/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef DEFERREDWORKPACKAGE_HPP_201612212229
#define DEFERREDWORKPACKAGE_HPP_201612212229

#include "gpcc/src/time/TimePoint.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <cstdint>

namespace gpcc {

namespace time
{
  class TimeSpan;
}

namespace execution {
namespace async {

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Deferred work package which can be processed by class @ref DeferredWorkQueue.
 *
 * In contrast to class @ref WorkPackage, the execution of this type of work package will be deferred until
 * the monotonic system clock (@ref gpcc::time::Clocks::monotonic) reaches a specific point in time.
 *
 * # Content
 * The deferred work package encapsulates the following:
 * - A functor to a function or method that shall be executed.
 * - A pointer to the owner (originator) of the work package (nullptr = anonymous).
 * - An ID for further identification of @ref DeferredWorkPackage instances on a per-owner basis.
 * - A timestamp specifying the point in time until when execution of the work package shall be deferred.\n
 *   The timestamp refers to the clock @ref gpcc::time::Clocks::monotonic.
 *
 * The owner and the ID are only used for selective removal of work packages from a work queue.
 *
 * # Creation and Ownership
 * Use any of the constructors to create a _static_ work package.\n
 * Use any of the @ref CreateDynamic() methods to create a _dynamic_ work package.
 *
 * Ownership of _dynamic_ work packages moves to the work queue and the work queue will finally release
 * the work package. In contrast to this, ownership of _static_ work packages always remains at the
 * creator of the work package.
 *
 * _Static_ work packages can be recycled and do not use any memory allocation during runtime.\n
 * See @ref GPCC_EXECUTION_ASYNC for details.
 *
 * ---
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class DeferredWorkPackage final
{
    friend class DeferredWorkQueue;

  public:
    /// Type definition of the functor encapsulated by the deferred work package.
    typedef std::function<void(void)> tFunctor;

    DeferredWorkPackage(void) = delete;
    DeferredWorkPackage(void const * const _pOwnerObject,
                        uint32_t const _ownerID,
                        tFunctor const & _functor,
                        time::TimePoint const & _tp);
    DeferredWorkPackage(void const * const _pOwnerObject,
                        uint32_t const _ownerID,
                        tFunctor && _functor,
                        time::TimePoint const & _tp);
    DeferredWorkPackage(void const * const _pOwnerObject,
                        uint32_t const _ownerID,
                        tFunctor const & _functor,
                        time::TimeSpan const & delay);
    DeferredWorkPackage(void const * const _pOwnerObject,
                        uint32_t const _ownerID,
                        tFunctor && _functor,
                        time::TimeSpan const & delay);
    DeferredWorkPackage(void const * const _pOwnerObject,
                        uint32_t const _ownerID,
                        tFunctor const & _functor);
    DeferredWorkPackage(void const * const _pOwnerObject,
                        uint32_t const _ownerID,
                        tFunctor && _functor);
    DeferredWorkPackage(DeferredWorkPackage const &) = delete;
    DeferredWorkPackage(DeferredWorkPackage &&) = delete;
    ~DeferredWorkPackage(void);

    static std::unique_ptr<DeferredWorkPackage> CreateDynamic(void const * const _pOwnerObject,
                                                              uint32_t const _ownerID,
                                                              tFunctor const & _functor,
                                                              time::TimePoint const & _tp);
    static std::unique_ptr<DeferredWorkPackage> CreateDynamic(void const * const _pOwnerObject,
                                                              uint32_t const _ownerID,
                                                              tFunctor && _functor,
                                                              time::TimePoint const & _tp);
    static std::unique_ptr<DeferredWorkPackage> CreateDynamic(void const * const _pOwnerObject,
                                                              uint32_t const _ownerID,
                                                              tFunctor const & _functor,
                                                              time::TimeSpan const & delay);
    static std::unique_ptr<DeferredWorkPackage> CreateDynamic(void const * const _pOwnerObject,
                                                              uint32_t const _ownerID,
                                                              tFunctor && _functor,
                                                              time::TimeSpan const & delay);

    DeferredWorkPackage& operator=(DeferredWorkPackage const &) = delete;
    DeferredWorkPackage& operator=(DeferredWorkPackage &&) = delete;

    void SetTimePoint(time::TimePoint const & _tp);
    void SetTimeSpan(time::TimeSpan const & delay);

  private:
    /// States of the work package.
    enum class States
    {
      staticNotInQ,
      staticInQ,
      staticExec,
      staticExecInQ,
      dynamicNotInQ,
      dynamicInQ
    };

    /// Pointer to the object that has created the work package.
    /** nullptr = anonymous. */
    void const * const pOwnerObject;

    /// ID assigned by the owner of the work package.
    /** Note: The ID is also applicable, if @ref pOwnerObject is `nullptr` (anonymous owner). */
    uint32_t const ownerID;

    /// Functor to the function/method to be invoked when the work package is processed.
    tFunctor const functor;

    /// Absolute point in time until when execution of the work package shall be deferred.
    /** The time point is specified using the monotonous system clock (@ref gpcc::time::Clocks::monotonic). */
    time::TimePoint tp;

    /// Pointer to next @ref DeferredWorkPackage in a work queue.
    DeferredWorkPackage* pNext;

    /// Pointer to previous @ref DeferredWorkPackage in a work queue.
    DeferredWorkPackage* pPrev;

    /// Current state of the work package.
    std::atomic<States> state;
};

} // namespace async
} // namespace execution
} // namespace gpcc

#endif /* DEFERREDWORKPACKAGE_HPP_201612212229 */
