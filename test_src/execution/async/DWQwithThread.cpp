/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "DWQwithThread.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/string/tools.hpp"

namespace gpcc_tests {
namespace execution  {
namespace async      {

/**
 * \brief Constructor. The thread is started and the provided work queue is ready to use.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param threadName
 * Name for the thread that will run the work queue.
 */
DWQwithThread::DWQwithThread(std::string const & threadName)
: dwq()
, dwqThread(threadName)
{
  dwqThread.Start(std::bind(&DWQwithThread::ThreadEntry, this),
                  gpcc::osal::Thread::SchedPolicy::Other, 0U,
                  gpcc::osal::Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(stopDwqThread)
  {
    dwq.RequestTermination();
    dwqThread.Join();
  };

  dwq.FlushNonDeferredWorkPackages();

  ON_SCOPE_EXIT_DISMISS(stopDwqThread);
}

/**
 * \brief Destructor.
 *
 * Any static work packages that are still enqueued in the work queue will be removed from the work queue.\n
 * Any dynamic work packages that are still enqueued in the work queue will be destroyed.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
DWQwithThread::~DWQwithThread(void)
{
  try
  {
    dwq.RequestTermination();
    dwqThread.Join();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("DWQwithThread::~DWQwithThread: Caught an exception:", e);
  }
  catch (...)
  {
    gpcc::osal::Panic("DWQwithThread::~DWQwithThread: Caught an unknown exception");
  }
}

/**
 * \brief Entry function for the thread running the work queue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures that there is only one thread per @ref DWQwithThread instance executing this.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation will be disabled by this.
 *
 * - - -
 *
 * \return
 * Value returned by Thread::Join(). Here: Always nullptr.
 */
void* DWQwithThread::ThreadEntry(void)
{
  dwqThread.SetCancelabilityEnabled(false);

  try
  {
    dwq.Work();
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "DWQwithThread::ThreadEntry: A work package threw:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("DWQwithThread::ThreadEntry: A work package threw: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("DWQwithThread::ThreadEntry: Caught an unknown exception thrown by a work package.");
  }

  return nullptr;
}

} // namespace execution
} // namespace async
} // namespace gpcc_tests
