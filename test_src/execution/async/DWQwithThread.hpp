/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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