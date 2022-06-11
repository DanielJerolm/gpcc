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

#include "UUT_TriggeredThreadedCyclicExec.hpp"
#include "Trace.hpp"
#include "gpcc/src/execution/cyclic/TTCEStartStopCtrl.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <functional>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

UUT_TriggeredThreadedCyclicExec::UUT_TriggeredThreadedCyclicExec(Trace & _trace,
                                                                 gpcc::StdIf::IIRQ2ThreadWakeup & trigger,
                                                                 gpcc::time::TimeSpan const & waitForTriggerTimeout)
: TriggeredThreadedCyclicExec("UUT", trigger, waitForTriggerTimeout, std::bind(&UUT_TriggeredThreadedCyclicExec::IsPllRunning, this))
, trace(_trace)
, pTTCEStartStopCtrl(nullptr)
, mutex()
, sampleRetVal(true)
, isPllRunningRetVal(true)
{
}

void UUT_TriggeredThreadedCyclicExec::SetTTCEStartStopCtrl(TTCEStartStopCtrl* const _pTTCEStartStopCtrl)
{
  pTTCEStartStopCtrl = _pTTCEStartStopCtrl;
}


void UUT_TriggeredThreadedCyclicExec::SetSampleRetVal(bool const value)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  sampleRetVal = value;
}
void UUT_TriggeredThreadedCyclicExec::SetIsPllRunningRetVal(bool const value)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  isPllRunningRetVal = value;
}

void UUT_TriggeredThreadedCyclicExec::Cyclic(void)
{
  trace.Record(Trace::TRACE_CYCLIC);
}
void UUT_TriggeredThreadedCyclicExec::OnStart(void)
{
  trace.Record(Trace::TRACE_ONSTART);
}
void UUT_TriggeredThreadedCyclicExec::OnStop(void)
{
  trace.Record(Trace::TRACE_ONSTOP);
}
bool UUT_TriggeredThreadedCyclicExec::Sample(bool const overrun)
{
  trace.Record(Trace::BuildTraceValue_Sample(overrun));

  gpcc::osal::MutexLocker mutexLocker(mutex);
  return sampleRetVal;
}
void UUT_TriggeredThreadedCyclicExec::OnStateChange(States const newState, StopReasons const stopReason)
{
  trace.Record(Trace::BuildTraceValue_OnStateChange(newState, stopReason));
  if (pTTCEStartStopCtrl)
    pTTCEStartStopCtrl->OnTTCEStateChange(newState, stopReason);
}

bool UUT_TriggeredThreadedCyclicExec::IsPllRunning(void)
{
  trace.Record(Trace::TRACE_ISPLLRUN);

  gpcc::osal::MutexLocker mutexLocker(mutex);
  return isPllRunningRetVal;
}

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
