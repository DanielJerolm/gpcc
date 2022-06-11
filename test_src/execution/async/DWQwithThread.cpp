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

#include "DWQwithThread.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
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
