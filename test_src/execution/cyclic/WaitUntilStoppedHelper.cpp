/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "WaitUntilStoppedHelper.hpp"
#include "gpcc/src/execution/cyclic/TTCEStartStopCtrl.hpp"
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
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
