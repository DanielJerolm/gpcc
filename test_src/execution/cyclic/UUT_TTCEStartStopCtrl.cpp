/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "UUT_TTCEStartStopCtrl.hpp"
#include "Trace.hpp"

namespace gpcc_tests {
namespace execution {
namespace cyclic {

UUT_TTCEStartStopCtrl::UUT_TTCEStartStopCtrl(TriggeredThreadedCyclicExec & _ttce,
                                             uint8_t const _restartAttemptsAfterLossOfLock,
                                             gpcc::execution::async::IWorkQueue & _wq,
                                             Trace & _trace,
                                             uint8_t const _onBeforeRestartAfterLossOfLockRetVal)
: TTCEStartStopCtrl(_ttce, _restartAttemptsAfterLossOfLock, _wq)
, trace(_trace)
, onBeforeRestartAfterLossOfLockRetVal(_onBeforeRestartAfterLossOfLockRetVal)
{
}

uint8_t UUT_TTCEStartStopCtrl::OnBeforeRestartAfterLossOfLock(void)
{
  trace.Record(Trace::TRACE_OBRALOL);
  return onBeforeRestartAfterLossOfLockRetVal;
}
void UUT_TTCEStartStopCtrl::OnStateSwitchedTo_Stopped(TriggeredThreadedCyclicExec::StopReasons const stopReason)
{
  trace.Record(Trace::BuildTraceValue_OSST_STOPPED(stopReason));
}
void UUT_TTCEStartStopCtrl::OnStateSwitchedTo_Starting(void)
{
  trace.Record(Trace::TRACE_OSST_STARTING);
}
void UUT_TTCEStartStopCtrl::OnStateSwitchedTo_Running(void)
{
  trace.Record(Trace::TRACE_OSST_RUNNING);
}
void UUT_TTCEStartStopCtrl::OnStateSwitchedTo_StopPending(void)
{
  trace.Record(Trace::TRACE_OSST_STOPPEND);
}
void UUT_TTCEStartStopCtrl::OnStateSwitchedTo_StoppedStopPending(TriggeredThreadedCyclicExec::StopReasons const stopReason)
{
  trace.Record(Trace::BuildTraceValue_OSST_STOPPEDSTOPPEND(stopReason));
}
void UUT_TTCEStartStopCtrl::OnBadAllocWQ(void)
{
  trace.Record(Trace::TRACE_ONBADALLOC);
}

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
