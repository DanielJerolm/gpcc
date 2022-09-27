/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "UUT_TriggeredThreadedCyclicExec.hpp"
#include <gpcc/execution/cyclic/TTCEStartStopCtrl.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "Trace.hpp"
#include <functional>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

UUT_TriggeredThreadedCyclicExec::UUT_TriggeredThreadedCyclicExec(Trace & _trace,
                                                                 gpcc::stdif::IIRQ2ThreadWakeup & trigger,
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
