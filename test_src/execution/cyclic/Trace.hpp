/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TRACE_HPP_201612302043
#define TRACE_HPP_201612302043

#include <gpcc/execution/cyclic/TriggeredThreadedCyclicExec.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
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
