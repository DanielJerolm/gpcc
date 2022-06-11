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

#include "DeferredWorkPackage.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <stdexcept>

namespace gpcc {
namespace execution {
namespace async {

using namespace gpcc::time;

/**
 * \brief Constructor. Creates a static work package. The given functor is copied.\n
 *        The execution delay is specified by a [TimePoint](@ref gpcc::time::TimePoint).
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * A copy is generated.
 * \param _tp
 * Time point until when execution of the work package shall be deferred.\n
 * A copy is generated.\n
 * The time point must be specified using the monotonous system clock (@ref gpcc::time::Clocks::monotonic).
 */
DeferredWorkPackage::DeferredWorkPackage(void const * const _pOwnerObject,
                                         uint32_t const _ownerID,
                                         tFunctor const & _functor,
                                         TimePoint const & _tp)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(_functor)
, tp(_tp)
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("DeferredWorkPackage::DeferredWorkPackage: !_functor");
}

/**
 * \brief Constructor. Creates a static work package. The given functor is moved.\n
 *        The execution delay is specified by a [TimePoint](@ref gpcc::time::TimePoint).
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Paramater `_functor` could be left in an undefined state.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * The referenced object will be moved into the new created work package.
 * \param _tp
 * Time point until when execution of the work package shall be deferred.\n
 * A copy is generated.\n
 * The time point must be specified using the monotonous system clock (@ref gpcc::time::Clocks::monotonic).
 */
DeferredWorkPackage::DeferredWorkPackage(void const * const _pOwnerObject,
                                         uint32_t const _ownerID,
                                         tFunctor && _functor,
                                         TimePoint const & _tp)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(std::move(_functor))
, tp(_tp)
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("DeferredWorkPackage::DeferredWorkPackage: !_functor");
}

/**
 * \brief Constructor. Creates a static work package. The given functor is copied.\n
 *        The execution delay is specified by a [TimeSpan](@ref gpcc::time::TimeSpan) measured from now.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * A copy is generated.
 * \param delay
 * Time span measured from now until when execution of the work package shall be deferred.\n
 * A copy is generated.
 */
DeferredWorkPackage::DeferredWorkPackage(void const * const _pOwnerObject,
                                         uint32_t const _ownerID,
                                         tFunctor const & _functor,
                                         TimeSpan const & delay)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(_functor)
, tp(TimePoint::FromSystemClock(Clocks::monotonic) + delay)
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("DeferredWorkPackage::DeferredWorkPackage: !_functor");
}

/**
 * \brief Constructor. Creates a static work package. The given functor is moved.\n
 *        The execution delay is specified by a [TimeSpan](@ref gpcc::time::TimeSpan) measured from now.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Paramater `_functor` could be left in an undefined state.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * The referenced object will be moved into the new created work package.
 * \param delay
 * Time span measured from now until when execution of the work package shall be deferred.\n
 * A copy is generated.
 */
DeferredWorkPackage::DeferredWorkPackage(void const * const _pOwnerObject,
                                         uint32_t const _ownerID,
                                         tFunctor && _functor,
                                         TimeSpan const & delay)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(std::move(_functor))
, tp(TimePoint::FromSystemClock(Clocks::monotonic) + delay)
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("DeferredWorkPackage::DeferredWorkPackage: !_functor");
}

/**
 * \brief Constructor. Creates a static work package. The given functor is copied.\n
 *        The execution delay not specified yet.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * A copy is generated.
 */
DeferredWorkPackage::DeferredWorkPackage(void const * const _pOwnerObject,
                                         uint32_t const _ownerID,
                                         tFunctor const & _functor)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(_functor)
, tp()
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("DeferredWorkPackage::DeferredWorkPackage: !_functor");
}

/**
 * \brief Constructor. Creates a static work package. The given functor is moved.\n
 *        The execution delay not specified yet.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Paramater `_functor` could be left in an undefined state.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * The referenced object will be moved into the new created work package.
 */
DeferredWorkPackage::DeferredWorkPackage(void const * const _pOwnerObject,
                                         uint32_t const _ownerID,
                                         tFunctor && _functor)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(std::move(_functor))
, tp()
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("DeferredWorkPackage::DeferredWorkPackage: !_functor");
}

/**
 * \brief Destructor.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 */
DeferredWorkPackage::~DeferredWorkPackage(void)
{
  if ((state != States::staticNotInQ) &&
      (state != States::dynamicNotInQ))
  {
    gpcc::osal::Panic("DeferredWorkPackage::~DeferredWorkPackage: Enqueued in work queue");
  }
}

/**
 * \brief Factory. Creates a dynamic work package. The given functor is copied.\n
 *        The execution delay is specified by a [TimePoint](@ref gpcc::time::TimePoint).
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * A copy is generated.
 * \param _tp
 * Time point until when execution of the work package shall be deferred.\n
 * A copy is generated.\n
 * The time point must be specified using the monotonous system clock (@ref gpcc::time::Clocks::monotonic).
 * \return
 * An `std::unique_ptr` to a new @ref DeferredWorkPackage instance.
 */
std::unique_ptr<DeferredWorkPackage> DeferredWorkPackage::CreateDynamic(void const * const _pOwnerObject,
                                                                        uint32_t const _ownerID,
                                                                        tFunctor const & _functor,
                                                                        time::TimePoint const & _tp)
{
  auto pDWP = new DeferredWorkPackage(_pOwnerObject, _ownerID, _functor, _tp);
  pDWP->state = States::dynamicNotInQ;
  return std::unique_ptr<DeferredWorkPackage>(pDWP);
}

/**
 * \brief Factory. Creates a dynamic work package. The given functor is moved.\n
 *        The execution delay is specified by a [TimePoint](@ref gpcc::time::TimePoint).
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Paramater `_functor` could be left in an undefined state.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * The referenced object will be moved into the new created work package.
 * \param _tp
 * Time point until when execution of the work package shall be deferred.\n
 * A copy is generated.\n
 * The time point must be specified using the monotonous system clock (@ref gpcc::time::Clocks::monotonic).
 * \return
 * An `std::unique_ptr` to a new @ref DeferredWorkPackage instance.
 */
std::unique_ptr<DeferredWorkPackage> DeferredWorkPackage::CreateDynamic(void const * const _pOwnerObject,
                                                                        uint32_t const _ownerID,
                                                                        tFunctor && _functor,
                                                                        time::TimePoint const & _tp)
{
  auto pDWP = new DeferredWorkPackage(_pOwnerObject, _ownerID, std::move(_functor), _tp);
  pDWP->state = States::dynamicNotInQ;
  return std::unique_ptr<DeferredWorkPackage>(pDWP);
}

/**
 * \brief Factory. Creates a dynamic work package. The given functor is copied.\n
 *        The execution delay is specified by a [TimeSpan](@ref gpcc::time::TimeSpan) measured from now.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * A copy is generated.
 * \param delay
 * Time span measured from now until when execution of the work package shall be deferred.\n
 * A copy is generated.
 * \return
 * An `std::unique_ptr` to a new @ref DeferredWorkPackage instance.
 */
std::unique_ptr<DeferredWorkPackage> DeferredWorkPackage::CreateDynamic(void const * const _pOwnerObject,
                                                                        uint32_t const _ownerID,
                                                                        tFunctor const & _functor,
                                                                        time::TimeSpan const & delay)
{
  auto pDWP = new DeferredWorkPackage(_pOwnerObject, _ownerID, _functor, TimePoint::FromSystemClock(Clocks::monotonic) + delay);
  pDWP->state = States::dynamicNotInQ;
  return std::unique_ptr<DeferredWorkPackage>(pDWP);
}

/**
 * \brief Factory. Creates a dynamic work package. The given functor is moved.\n
 *        The execution delay is specified by a [TimeSpan](@ref gpcc::time::TimeSpan) measured from now.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Paramater `_functor` could be left in an undefined state.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param _pOwnerObject
 * Pointer to the object which is the owner of the work package.\n
 * The owner object must not be destroyed before the work package is destroyed or before its execution has finished.\n
 * This may be `nullptr` if there is no owner object (anonymous owner).\n
 * The owner pointer can later be used to remove specific work packages from a work queue.
 * \param _ownerID
 * ID assigned by the owner of the work package.\n
 * The ID can later be used to remove specific work packages from a work queue.\n
 * The ID is also applicable, if `_pOwnerObject` is `nullptr` (anonymous owner).\n
 * The same ID may be assigned to multiple work packages of the same owner.
 * \param _functor
 * Functor to the function/method which shall be invoked when the work package is processed.\n
 * The referenced object will be moved into the new created work package.
 * \param delay
 * Time span measured from now until when execution of the work package shall be deferred.\n
 * A copy is generated.
 * \return
 * An `std::unique_ptr` to a new @ref DeferredWorkPackage instance.
 */
std::unique_ptr<DeferredWorkPackage> DeferredWorkPackage::CreateDynamic(void const * const _pOwnerObject,
                                                                        uint32_t const _ownerID,
                                                                        tFunctor && _functor,
                                                                        time::TimeSpan const & delay)
{
  auto pDWP = new DeferredWorkPackage(_pOwnerObject, _ownerID, std::move(_functor), TimePoint::FromSystemClock(Clocks::monotonic) + delay);
  pDWP->state = States::dynamicNotInQ;
  return std::unique_ptr<DeferredWorkPackage>(pDWP);
}

/**
 * \brief Sets the point in time until when execution shall be deferred using a [TimePoint](@ref gpcc::time::TimePoint).
 *
 * This method is only allowed to be called on _static_ work packages which are _currently not enqueued_
 * in any work queue.
 *
 * ---
 *
 * __Thread-safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _tp
 * Time point until when execution of the work package shall be deferred.\n
 * A copy is generated.\n
 * The time point must be specified using the monotonous system clock (@ref gpcc::time::Clocks::monotonic).
 */
void DeferredWorkPackage::SetTimePoint(time::TimePoint const & _tp)
{
  if ((state != States::staticNotInQ) && (state != States::staticExec))
    throw std::logic_error("DeferredWorkPackage::SetTimepoint: Wrong state");

  tp = _tp;
}

/**
 * \brief Sets the point in time until when execution shall be deferred using a [TimeSpan](@ref gpcc::time::TimeSpan) measured from now.
 *
 * This method is only allowed to be called on _static_ work packages which are _currently not enqueued_
 * in any work queue.
 *
 * ---
 *
 * __Thread-safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param delay
 * Time span measured from now until when execution of the work package shall be deferred.\n
 * A copy is generated.
 */
void DeferredWorkPackage::SetTimeSpan(time::TimeSpan const & delay)
{
  if ((state != States::staticNotInQ) && (state != States::staticExec))
    throw std::logic_error("DeferredWorkPackage::SetTimespan: Wrong state");

  tp = TimePoint::FromSystemClock(Clocks::monotonic) + delay;
}

} // namespace async
} // namespace execution
} // namespace gpcc
