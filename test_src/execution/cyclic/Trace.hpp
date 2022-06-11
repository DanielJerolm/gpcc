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
#ifndef TRACE_HPP_201612302043
#define TRACE_HPP_201612302043

#include "gpcc/src/execution/cyclic/TriggeredThreadedCyclicExec.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include <vector>
#include <cstdint>
#include <cstddef>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

using gpcc::execution::cyclic::TriggeredThreadedCyclicExec;

// Simple thread-safe tracer for logging events during execution of unit tests for classes
// TriggeredThreadedCyclicExec and TTCEStartStopCtrl. Logged events can be compared against an
// expected sequence of events.
// All API-methods are thread-safe.
// Trace format:
// Bit  0..7 : Event ID. One of the static values TRACE_CYCLIC, TRACE_ONSTART, ...
// Bit  8..15: First parameter (depends on Event ID).
// Bit 16..23: Second parameter (depends on Event ID).
// Bit 24    : Flag: optional event (only for expected values)
// Bit 25..31: unused
class Trace final
{
  public:
    // Events that can be recorded during test execution (TriggeredThreadedCyclicExec)
    static uint32_t const TRACE_CYCLIC     = 0x01U;
    static uint32_t const TRACE_ONSTART    = 0x02U;
    static uint32_t const TRACE_ONSTOP     = 0x03U;
    static uint32_t const TRACE_SAMPLE     = 0x04U;
    static uint32_t const TRACE_SAMPLEOVR  = 0x05U;
    static uint32_t const TRACE_ONSTATECHG = 0x06U;
    static uint32_t const TRACE_ISPLLRUN   = 0x07U;

    // Events that can be recorded during test execution (TTCEStartStopCtrl)
    static uint32_t const TRACE_OBRALOL               = 0x08U; // (OnBeforeRestartAfterLossOfLock)
    static uint32_t const TRACE_OSST_STOPPED          = 0x09U; // (OnStateSwitchedTo_Stopped)
    static uint32_t const TRACE_OSST_STARTING         = 0x0AU; // (OnStateSwitchedTo_Starting)
    static uint32_t const TRACE_OSST_RUNNING          = 0x0BU; // (OnStateSwitchedTo_Running)
    static uint32_t const TRACE_OSST_STOPPEND         = 0x0CU; // (OnStateSwitchedTo_StopPending)
    static uint32_t const TRACE_OSST_STOPPEDSTOPPEND  = 0x0DU; // (OnStateSwitchedTo_StoppedStopPending)
    static uint32_t const TRACE_ONBADALLOC            = 0x0EU; // (OnBadAllocWQ)

    // Flags for qualification of expected values
    static uint32_t const EXPECT_FLAG_OPTIONAL = 0x1000000U;


    // Helpers to create high-sophisticated event records
    static uint32_t BuildTraceValue_Sample(bool const overrun);
    static uint32_t BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States const newState, TriggeredThreadedCyclicExec::StopReasons const stopReason);
    static uint32_t BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons const stopReason);
    static uint32_t BuildTraceValue_OSST_STOPPEDSTOPPEND(TriggeredThreadedCyclicExec::StopReasons const stopReason);


    void Clear(void);
    void Record(uint32_t const value);
    size_t RemoveAll(uint32_t const value);
    size_t Count(uint32_t const value);
    bool Check(size_t n, uint32_t const * pExpectedValues);
    void Dump(void) const;

  private:
    mutable gpcc::osal::Mutex mutex;
    std::vector<uint32_t> log;
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // TRACE_HPP_201612302043
