/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "Trace.hpp"
#include "TriggerProvider.hpp"
#include "UUT_TriggeredThreadedCyclicExec.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>

// Universal timeout when waiting for things that will happen (of course if the UUT behaves as expected)
#define GENERAL_TIMEOUT_MS 500

// Sleep applied in "permanent trigger mode"
#define PERMANENT_TRIGGER_SLEEP_MS 10

// Timeout to be used by UUT when waiting for trigger
#define TTCETIMEOUT_MS 100

namespace gpcc_tests {
namespace execution {
namespace cyclic {

using namespace testing;

// Test fixture for gpcc::execution::cyclic::TriggeredThreadedCyclicExec related tests.
class gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF: public Test
{
  public:
    gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF(void);

  protected:
    // Logger for logging events.
    Trace trace;

    // Provider for trigger events.
    TriggerProvider triggerProvider;

    // The uut.
    UUT_TriggeredThreadedCyclicExec uut;


    void SetUp(void) override;
    void TearDown(void) override;

    void StartUUTThread(void);
    void StopUUTThread(void);
};

gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF::gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF(void)
: Test()
, trace()
, triggerProvider(gpcc::time::TimeSpan::ms(TTCETIMEOUT_MS), PERMANENT_TRIGGER_SLEEP_MS)
, uut(trace, triggerProvider, gpcc::time::TimeSpan::ms(TTCETIMEOUT_MS))
{
}

void gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF::SetUp(void)
{
}

void gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF::TearDown(void)
{
}

void gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF::StartUUTThread(void)
{
  uut.StartThread(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
}

void gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF::StopUUTThread(void)
{
  uut.StopThread();
}

// =====================================================================================================================
// =====================================================================================================================
// =====================================================================================================================

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, CreateAndDestroy)
{
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, CreateAndDestroyWithStartStopThread_A)
{
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  // Allow thread to start. This is not cruical for passing the test.
  gpcc::osal::Thread::Sleep_ms(PERMANENT_TRIGGER_SLEEP_MS);
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, CreateAndDestroyWithStartStopThread_B)
{
  StartUUTThread();

  // Immediate stop. This is "variant B" of the test. Presence of a delay is not crucial for passing the test,
  // but however, both variants A und B must always suceed.
  StopUUTThread();
}

#ifndef SKIP_TFC_BASED_TESTS

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, SamplingOff_NoTrigger_CyclicInvoked)
{
  // this test checks that Cyclic() is invoked if sampling is off and NO trigger is received
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  for (size_t i = 0U; i < 3U; i++)
  {
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false);
  }
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  const uint32_t expected[3] = { Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC };
  ASSERT_TRUE(trace.Check(3U, expected));
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, SamplingOff_Trigger_CyclicInvoked)
{
  // this test checks that Cyclic() is invoked if sampling is off and trigger is received
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  for (size_t i = 0U; i < 3U; i++)
  {
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  }
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  const uint32_t expected[3] = { Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC };
  ASSERT_TRUE(trace.Check(3U, expected));
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, SamplingOff_TriggerWithOverrun_CyclicInvoked)
{
  // this test checks that Cyclic() is invoked if sampling is off and trigger is received (with overrun condition)
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  for (size_t i = 0U; i < 3U; i++)
  {
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  }
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  const uint32_t expected[3] = { Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC };
  ASSERT_TRUE(trace.Check(3U, expected));
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, TurnOn_OperateNormal_TurnOff)
{
  // this test checks normal operation with enabling of sampling and disabling of sampling after some time
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, ImmediateTurnOn_OperateNormal_TurnOff)
{
  // this test checks normal operation with enabling of sampling and disabling of sampling after some time
  // Variation: RequestStartSampling is invoked before first trigger.

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, StartDelay)
{
  // this test checks proper application of "start delay"

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(3U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, Overrun_AllStates)
{
  // this test checks proper handling of "overrun" in all states
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::AlreadySignalled, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLEOVR,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLEOVR,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, SampleReturnsFalse)
{
  // this test checks correct behaviour if Sample() returns false
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.SetSampleRetVal(false);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, PllLossOfLockInRun)
{
  // this test checks correct behaviour if PLL looses lock in RUN
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.SetIsPllRunningRetVal(false);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.SetIsPllRunningRetVal(true);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, RemainInWaitLockUntilPllHasLocked)
{
  // this test checks if the uut waits for PLL lock correctly

  uut.SetIsPllRunningRetVal(false);

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  uut.SetIsPllRunningRetVal(true);

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, RequestStopWhileInStart)
{
  // this test checks proper behaviour if stop is requested while UUT is in state START

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(5U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, RequestStopWhileInWaitLock)
{
  // this test checks proper behaviour if stop is requested while the UUT is in state WAITLOCK
  uut.SetIsPllRunningRetVal(false);

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, RestartAfterStop)
{
  // this test checks proper behaviour if start is requested after sampling has been stopped
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());



  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC};

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, TriggerTimeoutInStart)
{
  // this test checks behaviour if a trigger timeout occurs while the UUT is in state START
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, TriggerTimeoutInWaitLock)
{
  // this test checks proper behaviour if a trigger timeout occurs while the UUT is in state WAITLOCK
  uut.SetIsPllRunningRetVal(false);

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::triggerTimeout),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, TriggerTimeoutInRun)
{
  // this checks proper behaviour if a trigger timeout occurrs while the UUT is in state RUN

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::triggerTimeout),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, ThreadTerminationAndRestart)
{
  // this test checks proper behaviour if the UUT's thread is terminated and restarted
  StartUUTThread();
  ON_SCOPE_EXIT(threadstart1) { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS(threadstart1);
  StopUUTThread();

  // OSAL implementations where ConVar Wait is not a cancellation point need an extra trigger...
  try
  {
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  }
  catch (std::exception const &)
  {
    // Ignored by intention:
    // Thread might not be blocked in trigger provider if cancellation already took place.
  }

  StartUUTThread();
  ON_SCOPE_EXIT(threadstart2) { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS(threadstart2);
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC | Trace::EXPECT_FLAG_OPTIONAL, // thread cancel

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC};

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, RequestStartTwice)
{
  // this test checks proper behaviour if start is requested twice
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  EXPECT_THROW(uut.RequestStartSampling(2U), std::logic_error);

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, RequestStopTwice)
{
  // this test checks proper behaviour if stop is requested twice
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  EXPECT_THROW(uut.RequestStopSampling(), std::logic_error);

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, StopCancelsStart)
{
  // this test checks proper behaviour if stop is requested directly after start

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(5U);
  uut.RequestStopSampling();

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, StopInStop)
{
  // this test checks proper behaviour if stop is requested while the UUT is in state STOP

  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStopSampling();

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TriggeredThreadedCyclicExec_TestsF, StartWhenNotInStop)
{
  // this test checks behaviour if start is requested while the UUT is not in state STOPPED.
  StartUUTThread();
  ON_SCOPE_EXIT() { StopUUTThread(); };

  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  uut.RequestStartSampling(0U);
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, uut.GetCurrentState());

  ASSERT_THROW(uut.RequestStartSampling(0U), std::logic_error);

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, uut.GetCurrentState());

  ASSERT_THROW(uut.RequestStartSampling(0U), std::logic_error);

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  ASSERT_THROW(uut.RequestStartSampling(0U), std::logic_error);

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, uut.GetCurrentState());

  uut.RequestStopSampling();
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, uut.GetCurrentState());

  ON_SCOPE_EXIT_DISMISS();
  StopUUTThread();

  uint32_t const expectedSeq[] = { Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
                                   Trace::TRACE_ONSTART,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 1
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 2
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN, // 3
                                   Trace::TRACE_SAMPLE,
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_ISPLLRUN,
                                   Trace::TRACE_ONSTOP,
                                   Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
                                   Trace::TRACE_CYCLIC,

                                   Trace::TRACE_CYCLIC };

  if (!trace.Check(sizeof(expectedSeq) / sizeof(uint32_t), expectedSeq))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

#endif // #ifndef SKIP_TFC_BASED_TESTS

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
