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

#include "Trace.hpp"
#include <iostream>
#include <iomanip>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

using gpcc::execution::cyclic::TriggeredThreadedCyclicExec;

uint32_t Trace::BuildTraceValue_Sample(bool const overrun)
{
  if (overrun)
    return TRACE_SAMPLEOVR;
  else
    return TRACE_SAMPLE;
}
uint32_t Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States const newState, TriggeredThreadedCyclicExec::StopReasons const stopReason)
{
  return TRACE_ONSTATECHG |
         (static_cast<uint32_t>(newState) << 8) |
         (static_cast<uint32_t>(stopReason) << 16);
}
uint32_t Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons const stopReason)
{
  return TRACE_OSST_STOPPED |
         (static_cast<uint32_t>(stopReason) << 8);
}
uint32_t Trace::BuildTraceValue_OSST_STOPPEDSTOPPEND(TriggeredThreadedCyclicExec::StopReasons const stopReason)
{
  return TRACE_OSST_STOPPEDSTOPPEND |
         (static_cast<uint32_t>(stopReason) << 8);
}
void Trace::Clear(void)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  log.clear();
}
void Trace::Record(uint32_t const value)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  log.push_back(value);
}
size_t Trace::RemoveAll(uint32_t const value)
{
  // Removes all record matching "value" and returns the number of removed records.
  size_t cnt = 0;
  gpcc::osal::MutexLocker mutexLocker(mutex);
  auto it = log.begin();
  while (it != log.end())
  {
    if (*it == value)
    {
      it = log.erase(it);
      cnt++;
    }
    else
      ++it;
  }

  return cnt;
}
size_t Trace::Count(uint32_t const value)
{
  size_t cnt = 0;
  gpcc::osal::MutexLocker mutexLocker(mutex);
  for (auto const e: log)
  {
    if (e == value)
      cnt++;
  }
  return cnt;
}
bool Trace::Check(size_t n, uint32_t const * pExpectedValues)
{
  // Checks the records against expected records.
  // Return: true = match, false = different

  gpcc::osal::MutexLocker mutexLocker(mutex);

  for (auto e: log)
  {
    // log entry, but no (more) expectations?
    if (n == 0)
      return false;

    while (e != (*pExpectedValues & static_cast<uint32_t>(~EXPECT_FLAG_OPTIONAL)))
    {
      // if the expectation was not optional, then the check failed
      if ((*pExpectedValues & EXPECT_FLAG_OPTIONAL) == 0)
        return false;

      // no more expectations? -> fail
      if (n == 1U)
        return false;

      // move to next expectation
      n--;
      pExpectedValues++;
    }

    n--;
    pExpectedValues++;
  }

  // if there are any expectations remaining, then they must all be optional
  while (n != 0)
  {
    if ((*pExpectedValues & EXPECT_FLAG_OPTIONAL) == 0)
      return false;

    n--;
    pExpectedValues++;
  }

  return true;
}
void Trace::Dump(void) const
{
  // Dumps the records to stdout for debugging purposes.
  // This is intended to be invoked if "Check" returned false.

  gpcc::osal::MutexLocker mutexLocker(mutex);

  std::cout << ">> LOG >>" << std::endl;
  for (auto const e: log)
  {
    uint32_t const lowerBits = e & 0xFFU;

    switch (lowerBits)
    {
      case TRACE_CYCLIC:
        std::cout << "TriggeredThreadedCyclicExec::Cyclic" << std::endl;
        break;

      case TRACE_ONSTART:
        std::cout << "TriggeredThreadedCyclicExec::OnStart" << std::endl;
        break;

      case TRACE_ONSTOP:
        std::cout << "TriggeredThreadedCyclicExec::OnStop" << std::endl;
        break;

      case TRACE_SAMPLE:
        std::cout << "TriggeredThreadedCyclicExec::Sample (no overrun)" << std::endl;
        break;

      case TRACE_SAMPLEOVR:
        std::cout << "TriggeredThreadedCyclicExec::Sample (overrun)" << std::endl;
        break;

      case TRACE_ONSTATECHG:
      {
        std::cout << "TriggeredThreadedCyclicExec::OnStateChg(";

        TriggeredThreadedCyclicExec::States const newState = static_cast<TriggeredThreadedCyclicExec::States>((e >> 8U) & 0xFFU);
        TriggeredThreadedCyclicExec::StopReasons const stopReason = static_cast<TriggeredThreadedCyclicExec::StopReasons>((e >> 16U) & 0xFFU);

        std::cout << TriggeredThreadedCyclicExec::State2String(newState) << ", " << TriggeredThreadedCyclicExec::StopReasons2String(stopReason) << ")" << std::endl;
        break;
      }

      case TRACE_ISPLLRUN:
        std::cout << "TriggeredThreadedCyclicExec::IsPllRunning" << std::endl;
        break;

      case TRACE_OBRALOL:
        std::cout << "TTCEStartStopCtrl::OnBeforeRestartAfterLossOfLock" << std::endl;
        break;

      case TRACE_OSST_STOPPED:
      {
        std::cout << "TTCEStartStopCtrl::OnStateSwitchedTo_Stopped(";
        TriggeredThreadedCyclicExec::StopReasons const stopReason = static_cast<TriggeredThreadedCyclicExec::StopReasons>((e >> 8U) & 0xFFU);
        std::cout << TriggeredThreadedCyclicExec::StopReasons2String(stopReason) << ")" << std::endl;
        break;
      }

      case TRACE_OSST_STARTING:
        std::cout << "TTCEStartStopCtrl::OnStateSwitchedTo_Starting" << std::endl;
        break;

      case TRACE_OSST_RUNNING:
        std::cout << "TTCEStartStopCtrl::OnStateSwitchedTo_Running" << std::endl;
        break;

      case TRACE_OSST_STOPPEND:
        std::cout << "TTCEStartStopCtrl::OnStateSwitchedTo_StopPending" << std::endl;
        break;

      case TRACE_OSST_STOPPEDSTOPPEND:
      {
        std::cout << "TTCEStartStopCtrl::OnStateSwitchedTo_StoppedStopPending(";
        TriggeredThreadedCyclicExec::StopReasons const stopReason = static_cast<TriggeredThreadedCyclicExec::StopReasons>((e >> 8U) & 0xFFU);
        std::cout << TriggeredThreadedCyclicExec::StopReasons2String(stopReason) << ")" << std::endl;
        break;
      }

      case TRACE_ONBADALLOC:
        std::cout << "TTCEStartStopCtrl::OnBadAllocWQ" << std::endl;
        break;

      default:
        std::cout << "Unknown" << std::endl;
        break;
    } // switch (lowerBits)
  } // for (auto e: log)
  std::cout << "<< END LOG <<" << std::endl;
}

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
