/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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

#ifndef NONBLOCKINGSEMAPHORE_HPP_202104021310
#define NONBLOCKINGSEMAPHORE_HPP_202104021310

#include "gpcc/src/osal/Mutex.hpp"
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <cstddef>
#include <cstdint>

namespace gpcc               {
namespace ResourceManagement {
namespace semaphores         {

/**
 * \ingroup GPCC_RESOURCEMANAGEMENT_SEMAPHORES
 * \brief A counting semaphore with non-blocking wait/decrement-operation.
 *
 * # Features
 * - Initial value configurable during instantiation.
 * - Post/inkrement and wait/decrement.
 * - Wait/decrement is non-blocking. Instead of blocking, a callback will be invoked when the semaphore is decremented.
 *
 * # Constraints/Restrictions
 * - _All threads using instances of class NonBlockingSemaphore must live in the same process._
 * - This does not use a semaphore primitive offered by the operating system.\n
 *   If you need a standard semaphore with blocking wait/decrement-operation, then choose the semaphore implementation
 *   from the OSAL: @ref gpcc::osal::Semaphore.
 * - This implementation is simple at the cost of efficiency: Wait/Descrement comprises a heap-allocation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class NonBlockingSemaphore final
{
  public:
    /**
     * \brief Typedef for a callback invoked after the semaphore has been acquired/decremented.
     *
     * \pre   Someone has invoked @ref Wait(), and the semaphore was not immediately decremented because its counter
     *        was already zero (@ref Wait() has returned false).
     *
     * \post  The function/method invoked by this callback has decremented/acquired the semaphore.
     *
     * - - -
     *
     * __Thread safety requirements/hints:__\n
     * This will be invoked in the context of a thread invoking @ref Post(). \n
     * The following functions/methods may be invoked from this context without deadlock:
     * - @ref Post()
     * - @ref Wait()
     *
     * __Exception safety requirements/hints:__\n
     * The referenced function/method shall provide the no-throw guarantee.
     *
     * __Thread cancellation safety requirements/hints:__\n
     * The referenced function/method shall not contain any cancellation point.
     */
    typedef std::function<void(void)> tSemAcquiredCallback;


    /// Maximum value for the semaphore's counter.
    static size_t const MAX = std::numeric_limits<size_t>::max();


    NonBlockingSemaphore(void) = delete;
    explicit NonBlockingSemaphore(size_t const initialValue);
    NonBlockingSemaphore(NonBlockingSemaphore const &) = delete;
    NonBlockingSemaphore(NonBlockingSemaphore &&) = delete;
    ~NonBlockingSemaphore(void);

    NonBlockingSemaphore& operator=(NonBlockingSemaphore const &) = delete;
    NonBlockingSemaphore& operator=(NonBlockingSemaphore &&) = delete;

    void Post(void);
    bool Wait(tSemAcquiredCallback const & cb);

  private:
    /// Mutex used to make this class thread-safe.
    gpcc::osal::Mutex mutex;

    /// Semaphore's counter.
    /** @ref mutex required. */
    size_t cnt;

    /// Callbacks of threads (users) that are waiting for decrement/acquisition of the semaphore.
    std::list<std::unique_ptr<tSemAcquiredCallback>> usersBlockedOnWait;
};

} // namespace semaphores
} // namespace ResourceManagement
} // namespace gpcc

#endif // NONBLOCKINGSEMAPHORE_HPP_202104021310
