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
