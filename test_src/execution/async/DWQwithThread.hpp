/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#ifndef DWQWITHTHREAD_HPP_202105190841
#define DWQWITHTHREAD_HPP_202105190841

#include "gpcc/src/execution/async/DeferredWorkQueue.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <string>

namespace gpcc_tests {
namespace execution  {
namespace async      {

/**
 * \ingroup GPCC_TESTS_SUPCLASS
 * \brief Provides a [DeferredWorkQueue](@ref gpcc::execution::async::DeferredWorkQueue) plus a thread for
 *        running the work queue.
 *
 * This class is intended to be used in unit tests and unit test fixtures that require one or more work queues.
 * This class does not expect work packages to throw. If a work package throws, then this class will panic.
 *
 * The thread will be setup with scheduling policy "other" and the default stack size for the OS configuration.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class DWQwithThread final
{
  public:
    DWQwithThread(void) = delete;
    DWQwithThread(std::string const & threadName);
    DWQwithThread(DWQwithThread const &) = delete;
    DWQwithThread(DWQwithThread &&) = delete;
    ~DWQwithThread(void);

    DWQwithThread& operator=(DWQwithThread const &) = delete;
    DWQwithThread& operator=(DWQwithThread &&) = delete;

    gpcc::execution::async::IDeferredWorkQueue & GetDWQ(void);

  private:
    /// Deferred work queue.
    gpcc::execution::async::DeferredWorkQueue dwq;

    /// Thread used to run @ref dwq.
    gpcc::osal::Thread dwqThread;


    void* ThreadEntry(void);
};

/**
 * \brief Retrieves a reference to the [IDeferredWorkQueue](@ref gpcc::execution::async::IDeferredWorkQueue)
 *        interface of the provided work queue instance.
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
 *
 * - - -
 *
 * \return
 * Reference to the [IDeferredWorkQueue](@ref gpcc::execution::async::IDeferredWorkQueue) interface of the provided
 * work queue instance.\n
 * The life-cycle of the referenced interface is limited to the life-cycle of this @ref DWQwithThread instance.
 */
inline gpcc::execution::async::IDeferredWorkQueue & DWQwithThread::GetDWQ(void)
{
  return dwq;
}

} // namespace execution
} // namespace async
} // namespace gpcc_tests

#endif // DWQWITHTHREAD_HPP_202105190841