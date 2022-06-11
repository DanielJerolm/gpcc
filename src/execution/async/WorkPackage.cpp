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

#include "WorkPackage.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <stdexcept>

namespace gpcc {
namespace execution {
namespace async {

/**
 * \brief Constructor. Creates a static work package. Copies the given functor.
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
WorkPackage::WorkPackage(void const * const _pOwnerObject, uint32_t const _ownerID, tFunctor const & _functor)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(_functor)
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("WorkPackage::WorkPackage: !_functor");
}

/**
 * \brief Constructor. Creates a static work package. Moves the given functor.
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
WorkPackage::WorkPackage(void const * const _pOwnerObject, uint32_t const _ownerID, tFunctor && _functor)
: pOwnerObject(_pOwnerObject)
, ownerID(_ownerID)
, functor(std::move(_functor))
, pNext(nullptr)
, pPrev(nullptr)
, state(States::staticNotInQ)
{
  if (!functor)
    throw std::invalid_argument("WorkPackage::WorkPackage: !_functor");
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
WorkPackage::~WorkPackage(void)
{
  if ((state != States::staticNotInQ) &&
      (state != States::dynamicNotInQ))
  {
    gpcc::osal::Panic("WorkPackage::~WorkPackage: Enqueued in work queue");
  }
}

/**
 * \brief Factory. Creates a dynamic work package. Copies the given functor.
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
 * \return
 * An `std::unique_ptr` to a new @ref WorkPackage instance.
 */
std::unique_ptr<WorkPackage> WorkPackage::CreateDynamic(void const * const _pOwnerObject,
                                                        uint32_t const _ownerID,
                                                        tFunctor const & _functor)
{
  auto pWP = new WorkPackage(_pOwnerObject, _ownerID, _functor);
  pWP->state = States::dynamicNotInQ;
  return std::unique_ptr<WorkPackage>(pWP);
}

/**
 * \brief Factory. Creates a dynamic work package. Moves the given functor.
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
 * \return
 * An `std::unique_ptr` to a new @ref WorkPackage instance.
 */
std::unique_ptr<WorkPackage> WorkPackage::CreateDynamic(void const * const _pOwnerObject,
                                                        uint32_t const _ownerID,
                                                        tFunctor && _functor)
{
  auto pWP = new WorkPackage(_pOwnerObject, _ownerID, std::move(_functor));
  pWP->state = States::dynamicNotInQ;
  return std::unique_ptr<WorkPackage>(pWP);
}

} // namespace async
} // namespace execution
} // namespace gpcc
