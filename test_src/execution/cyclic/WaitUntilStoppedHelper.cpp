/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "WaitUntilStoppedHelper.hpp"
#include "gpcc/src/execution/cyclic/TTCEStartStopCtrl.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <stdexcept>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

using namespace gpcc;

WaitUntilStoppedHelper::WaitUntilStoppedHelper(gpcc::execution::cyclic::TTCEStartStopCtrl & _uut)
: uut(_uut)
, mutex()
, state(States::idle)
, state_ReqStartWaiting_Entered_ConVar()
, state_Waiting_Entered_ConVar()
, state_Stopped_Entered_ConVar()
, thread("WaitUntilStoppedHelper")
{
}

void WaitUntilStoppedHelper::Start(void)
{
  // Starts the test helper's internal thread.

  thread.Start(std::bind(&WaitUntilStoppedHelper::ThreadEntry, this), osal::Thread::SchedPolicy::Other, 0, osal::Thread::GetDefaultStackSize());
}
void WaitUntilStoppedHelper::Stop(void)
{
  // Stops the test helper's internal thread and joins with it.
  // This MUST be invoked before the test helper instance is released.

  thread.Cancel();
  thread.Join();

  osal::MutexLocker mutexLocker(mutex);
  state = States::idle;
}

void WaitUntilStoppedHelper::StartWaiting(void)
{
  // Requests the test helper to invoke uut.WaitUntilStopped().
  // This blocks until the test helper has recognized the request and until the test helper
  // is just about to invoke uut.WaitUntilStopped().

  osal::MutexLocker mutexLocker(mutex);

  if ((state != States::idle) && (state != States::stopped))
    throw std::logic_error("WaitUntilStoppedHelper::StartWaiting: State is not States::idle or States::stopped");

  // request start
  state = States::reqStartWaiting;
  state_ReqStartWaiting_Entered_ConVar.Signal();

  // wait until start request has been recognized
  do
  {
    state_Waiting_Entered_ConVar.Wait(mutex);
  }
  while (state == States::reqStartWaiting);
}
bool WaitUntilStoppedHelper::IsStopped(void) const
{
  // Checks if uut.WaitUntilStopped() has returned.

  osal::MutexLocker mutexLocker(mutex);
  return (state == States::stopped);
}
bool WaitUntilStoppedHelper::WaitUntilStopped(gpcc::time::TimeSpan const & timeout) const
{
  // Wait (with timeout) until uut.WaitUntilStopped() has returned.

  osal::MutexLocker mutexLocker(mutex);

  time::TimePoint const absTimeout = time::TimePoint::FromSystemClock(time::Clocks::monotonic) + timeout;
  while (state != States::stopped)
  {
    if (state_Stopped_Entered_ConVar.TimeLimitedWait(mutex, absTimeout))
      break;
  }
  return (state == States::stopped);
}

void* WaitUntilStoppedHelper::ThreadEntry(void)
{
  osal::AdvancedMutexLocker mutexLocker(mutex);

  while (true)
  {
    // wait for start request
    while (state != States::reqStartWaiting)
      state_ReqStartWaiting_Entered_ConVar.Wait(mutex);

    // switch state to States::waiting
    state = States::waiting;
    state_Waiting_Entered_ConVar.Signal();

    // wait
    mutexLocker.Unlock();
    uut.WaitUntilStopped();
    mutexLocker.Relock();

    // signal that uut.WaitUntilStopped() has returned (uut has stopped)
    state = States::stopped;
    state_Stopped_Entered_ConVar.Signal();
  }

  return nullptr;
}

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
