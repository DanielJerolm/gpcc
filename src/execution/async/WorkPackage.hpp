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

#ifndef WORKPACKAGE_HPP_201612212208
#define WORKPACKAGE_HPP_201612212208

#include <atomic>
#include <functional>
#include <memory>
#include <cstdint>

namespace gpcc {
namespace execution {
namespace async {

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Work package which can be processed by class @ref WorkQueue and @ref DeferredWorkQueue.
 *
 * # Content
 * The work package encapsulates the following:
 * - A functor to a function or method that shall be executed.
 * - A pointer to the owner (originator) of the work package (nullptr = anonymous).
 * - An ID for further identification of @ref WorkPackage instances on a per-owner basis.
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
class WorkPackage final
{
    friend class WorkQueue;
    friend class DeferredWorkQueue;

  public:
    /// Type definition of the functor encapsulated by the work package.
    typedef std::function<void(void)> tFunctor;

    WorkPackage(void) = delete;
    WorkPackage(void const * const _pOwnerObject, uint32_t const _ownerID, tFunctor const & _functor);
    WorkPackage(void const * const _pOwnerObject, uint32_t const _ownerID, tFunctor && _functor);
    WorkPackage(WorkPackage const &) = delete;
    WorkPackage(WorkPackage &&) = delete;
    ~WorkPackage(void);

    static std::unique_ptr<WorkPackage> CreateDynamic(void const * const _pOwnerObject,
                                                      uint32_t const _ownerID,
                                                      tFunctor const & _functor);
    static std::unique_ptr<WorkPackage> CreateDynamic(void const * const _pOwnerObject,
                                                      uint32_t const _ownerID,
                                                      tFunctor && _functor);

    WorkPackage& operator=(WorkPackage const &) = delete;
    WorkPackage& operator=(WorkPackage &&) = delete;

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
    /** Note: The ID is also applicable, if @ref pOwnerObject is nullptr (anonymous owner). */
    uint32_t const ownerID;

    /// Functor to the function/method to be invoked when the work package is processed.
    tFunctor const functor;

    /// Pointer to next @ref WorkPackage in a work queue.
    WorkPackage* pNext;

    /// Pointer to previous @ref WorkPackage in a work queue.
    WorkPackage* pPrev;

    /// Current state of the work package.
    std::atomic<States> state;
};

} // namespace async
} // namespace execution
} // namespace gpcc

#endif /* WORKPACKAGE_HPP_201612212208 */
