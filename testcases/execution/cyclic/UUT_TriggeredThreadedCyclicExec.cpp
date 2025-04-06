/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
*/

#include "UUT_TriggeredThreadedCyclicExec.hpp"
#include <gpcc/execution/cyclic/TTCEStartStopCtrl.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "Trace.hpp"
#include <functional>

namespace gpcc_tests {
namespace execution  {
namespace cyclic     {

UUT_TriggeredThreadedCyclicExec::UUT_TriggeredThreadedCyclicExec(Trace & trace,
                                                                 gpcc::stdif::IIRQ2ThreadWakeup & trigger,
                                                                 gpcc::time::TimeSpan const & waitForTriggerTimeout)
: TriggeredThreadedCyclicExec("UUT",
                              trigger,
                              waitForTriggerTimeout,
                              std::bind(&UUT_TriggeredThreadedCyclicExec::IsPllRunning, this))
, trace_(trace)
, pTTCEStartStopCtrl_(nullptr)
, mutex_()
, sampleRetVal_(true)
, isPllRunningRetVal_(true)
{
}

void UUT_TriggeredThreadedCyclicExec::SetTTCEStartStopCtrl(TTCEStartStopCtrl* const pTTCEStartStopCtrl)
{
  pTTCEStartStopCtrl_ = pTTCEStartStopCtrl;
}

void UUT_TriggeredThreadedCyclicExec::SetSampleRetVal(bool const value)
{
  gpcc::osal::MutexLocker mutexLocker(mutex_);
  sampleRetVal_ = value;
}

void UUT_TriggeredThreadedCyclicExec::SetIsPllRunningRetVal(bool const value)
{
  gpcc::osal::MutexLocker mutexLocker(mutex_);
  isPllRunningRetVal_ = value;
}

void UUT_TriggeredThreadedCyclicExec::Cyclic(void)
{
  trace_.Record(Trace::TRACE_CYCLIC);
}

void UUT_TriggeredThreadedCyclicExec::OnStart(void)
{
  trace_.Record(Trace::TRACE_ONSTART);
}

void UUT_TriggeredThreadedCyclicExec::OnStop(void)
{
  trace_.Record(Trace::TRACE_ONSTOP);
}

bool UUT_TriggeredThreadedCyclicExec::Sample(bool const overrun)
{
  trace_.Record(Trace::BuildTraceValue_Sample(overrun));

  gpcc::osal::MutexLocker mutexLocker(mutex_);
  return sampleRetVal_;
}

void UUT_TriggeredThreadedCyclicExec::OnStateChange(States const newState, StopReasons const stopReason)
{
  trace_.Record(Trace::BuildTraceValue_OnStateChange(newState, stopReason));
  if (pTTCEStartStopCtrl_)
    pTTCEStartStopCtrl_->OnTTCEStateChange(newState, stopReason);
}

bool UUT_TriggeredThreadedCyclicExec::IsPllRunning(void)
{
  trace_.Record(Trace::TRACE_ISPLLRUN);

  gpcc::osal::MutexLocker mutexLocker(mutex_);
  return isPllRunningRetVal_;
}

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
