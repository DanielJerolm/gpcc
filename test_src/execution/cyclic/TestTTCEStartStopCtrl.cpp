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
#include "UUT_TTCEStartStopCtrl.hpp"
#include "WaitUntilStoppedHelper.hpp"
#include <gpcc/execution/async/WorkPackage.hpp>
#include <gpcc/execution/async/WorkQueue.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
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


#define RESTARTS_AFTER_LOSS_OF_LOCK 3U

// On before restart after loss of lock return value
#define OBRALOL_RETVAL 3U

namespace gpcc_tests {
namespace execution {
namespace cyclic {

using namespace gpcc::execution::async;
using namespace gpcc::osal;
using namespace testing;

// Test fixture for gpcc::execution::cyclic::TTCEStartStopCtrl related tests.
class gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF: public Test
{
  public:
    gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF(void);

  protected:
    // Logger for logging events.
    Trace trace;

    // Provider for trigger events.
    TriggerProvider triggerProvider;

    // Workqueue.
    WorkQueue wq;

    // Thread for wq.
    Thread thread;

    // The uut's companion. This is a TriggeredThreadedCyclicExec instance that shall be controlled by the uut.
    UUT_TriggeredThreadedCyclicExec controlledTTCE;
    bool ttceRunning;

    // The uut.
    UUT_TTCEStartStopCtrl uut;


    void SetUp(void) override;
    void TearDown(void) override;

    void WaitForThread_WQ(void);
    void CreateStimulus_TriggerAndStopRequest_WQ(void);
    void CreateStimulus_TriggerWithTimeoutAndStopRequest_WQ(void);

  private:
    void* ThreadEntry(void);
};

gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF(void)
: Test()
, trace()
, triggerProvider(gpcc::time::TimeSpan::ms(TTCETIMEOUT_MS), PERMANENT_TRIGGER_SLEEP_MS)
, wq()
, thread("TestTTCEStartStopCtrol")
, controlledTTCE(trace, triggerProvider, gpcc::time::TimeSpan::ms(TTCETIMEOUT_MS))
, ttceRunning(false)
, uut(controlledTTCE, RESTARTS_AFTER_LOSS_OF_LOCK, wq, trace, OBRALOL_RETVAL)
{
  controlledTTCE.SetTTCEStartStopCtrl(&uut);
}

void gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::SetUp(void)
{
  thread.Start(std::bind(&gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::ThreadEntry, this), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  wq.FlushNonDeferredWorkPackages();

  controlledTTCE.StartThread(Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ttceRunning = true;
}

void gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::TearDown(void)
{
  try
  {
    if (ttceRunning)
    {
      controlledTTCE.StopThread();
      ttceRunning = false;
    }

    wq.RequestTermination();
    thread.Join(nullptr);
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
}

void gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::WaitForThread_WQ(void)
{
  // Invokes "triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS)" in work queue context

  if (!triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS))
    throw std::runtime_error("gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::WaitForThread_WQ()");
}

void gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::CreateStimulus_TriggerAndStopRequest_WQ(void)
{
  // This creates three work packages doing the following:
  // 1st: trigger (OK)
  // 2nd: wait for thread of controlled TTCE (IN WORKQUEUE CONTEXT)
  // 3rd: Invoke StopAsync at UUT

  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&TriggerProvider::Trigger, &triggerProvider, gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::WaitForThread_WQ, this)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&TTCEStartStopCtrl::StopAsync, &uut)));
}

void gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::CreateStimulus_TriggerWithTimeoutAndStopRequest_WQ(void)
{
  // This creates three work packages doing the following:
  // 1st: trigger (with timeout)
  // 2nd: wait for thread of controlled TTCE (IN WORKQUEUE CONTEXT)
  // 3rd: Invoke StopAsync at UUT

  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&TriggerProvider::Trigger, &triggerProvider, gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::WaitForThread_WQ, this)));
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&TTCEStartStopCtrl::StopAsync, &uut)));
}

void* gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF::ThreadEntry(void)
{
  wq.Work();
  return nullptr;
}

// =====================================================================================================================
// =====================================================================================================================
// =====================================================================================================================

#ifndef SKIP_TFC_BASED_TESTS

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, CreateAndDestroy)
{
  ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());
  ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  Thread::Sleep_ms(PERMANENT_TRIGGER_SLEEP_MS);

  ASSERT_TRUE(trace.Check(0, nullptr));
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, ControlledTTCEisAlive)
{
  // this test checks that the controlled TriggeredThreadedCyclicExec is alive

  for (size_t i = 0U; i < 3U; i++)
  {
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  }
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));

  const uint32_t expected[3] = { Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC, Trace::TRACE_CYCLIC };
  ASSERT_TRUE(trace.Check(3U, expected));
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, TurnOnRunTurnOff)
{
  // this test requests start, runs, and requests stop

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  for (size_t i = 0U; i < 3U; i++)
  {
    // --------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // --------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
  }

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StopAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());



  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, TurnOnRunTurnOff_ExtraStartRequests)
{
  // This test requests start, runs, and requests stop. During this procedure, StartAsync() is
  // invoked in UUT states STARTING, RUNNING, and STOPPENDING

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStarted, uut.StartAsync());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyRunning, uut.StartAsync());

  for (size_t i = 0U; i < 3U; i++)
  {
    // --------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // --------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
  }

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StopAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StartAsync());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());



  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, TurnOnRunTurnOff_ExtraStopRequests_1)
{
  // This test requests start, runs, and requests stop. During this procedure, StopAsync() is
  // invoked in UUT states STOPPED, RUNNING (as part of the test), and STOPPENDING.
  // Invocation of StopAsync() in state STARTING is checked in another test case.

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopped, uut.StopAsync());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  for (size_t i = 0U; i < 3U; i++)
  {
    // --------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // --------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
  }

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StopAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StopAsync());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopped, uut.StopAsync());



  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, TurnOnRunTurnOff_ExtraStopRequests_2)
{
  // This test requests start, and requests stop while the UUT is in state STARTING.

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StopAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  // check for misbehavior: flush wq and check states

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());


  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, OnRunWQIgnoredInStopPending)
{
  // This test checks that the run-notification delivered via work queue from the controlled
  // TriggeredThreadedCyclicExec is ignored in UUT state STOPPENDING.

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // Now we create a very special stimulus using the work queue:
  // 1st: Trigger
  // 2nd: Wait for thread (in WQ context)
  // 3rd: Request stop, BEFORE uut's OnRun_WQ is executed in work queue context.
  //      This is guaranteed, because steps 1..3 are implemented as work packages.
  //      Since a work package is used to add the work packages, any work packages
  //      generated by the UUT are added BEHIND step 3.
  // Note:
  // - Three work queue flushes are needed:
  //   1. For the work package added here
  //   2. For the work packages added by the work package added here
  //   3. For OnRun_WQ
  // - OnStop_WQ won't be executed before next trigger or trigger timeout event.
  //   The stop request just sets a flag, which is processed when a trigger or trigger
  //   timeout occurs.
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, OnRunWQIgnoredInStopPending)::CreateStimulus_TriggerAndStopRequest_WQ, this)));
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();

  // OnRun_WQ has been executed here

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());



  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_ONSTART,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, TriggerTimeoutWhileStarting)
{
  // This test checks proper behavior if the controlled TriggeredThreadedCyclicExec reports
  // an trigger timeout during WAITLOCK (uut: STARTING).

  controlledTTCE.SetIsPllRunningRetVal(false);

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  wq.FlushNonDeferredWorkPackages();

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::triggerTimeout),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, TriggerTimeoutWhileRunning)
{
  // This test checks proper behavior if the controlled TriggeredThreadedCyclicExec reports
  // an trigger timeout during RUN (uut: RUNNING).

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // --------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // --------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // --------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::Timeout, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // --------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());




  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::triggerTimeout),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, SampleReturnedFalseWhileRunning)
{
  // This test checks proper behavior if the controlled TriggeredThreadedCyclicExec's Sample()-method
  // returns false during RUN (uut: RUNNING).

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // --------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // --------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  controlledTTCE.SetSampleRetVal(false);

  // --------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // --------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());




  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, LossOfLockWhileRunning_NoAutoRestart)
{
  // This test checks proper behavior if an PLL loss of lock occurs in RUN (uut: RUNNING) with
  // zero automatic restart attempts configured.

  uut.SetRestartAttemptsAfterLossOfLock(0U);

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // --------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // --------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  controlledTTCE.SetIsPllRunningRetVal(false);
  // --------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // --------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());




  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, StateStoppedStopPending_TriggerTimeout)
{
  // This test checks state stoppedStopPending. Error used to enter state: Trigger Timeout

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
  wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  // Now we create a very special stimulus using the work queue:
  // 1st: Trigger (timeout)
  // 2nd: Wait for thread (in WQ context)
  // 3rd: Request stop, BEFORE uut's OnStop_WQ is executed in work queue context.
  //      This is guaranteed, because steps 1..3 are implemented as work packages.
  //      Since a work package is used to add the work packages, any work packages
  //      generated by the UUT are added BEHIND step 3.
  // Note:
  // - Three work queue flushes are needed:
  //   1. For the work package added here
  //   2. For the work packages added by the work package added here
  //   3. For OnStop_WQ
  // - OnStop_WQ (the one due to stop request) won't be executed before next trigger or
  //   trigger timeout event. The stop request just sets a flag, which is processed when a
  //   trigger or trigger timeout occurs.
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, StateStoppedStopPending_TriggerTimeout)::CreateStimulus_TriggerWithTimeoutAndStopRequest_WQ, this)));
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stoppedStopPending, uut.GetCurrentState());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StopAsync());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StartAsync());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());


  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::triggerTimeout),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::BuildTraceValue_OSST_STOPPEDSTOPPEND(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout),
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, StateStoppedStopPending_SampleRetFalse)
{
  // This test checks state stoppedStopPending. Error used to enter state: Sample returned false

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
  wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  controlledTTCE.SetSampleRetVal(false);

  // Now we create a very special stimulus using the work queue:
  // 1st: Trigger
  // 2nd: Wait for thread (in WQ context)
  // 3rd: Request stop, BEFORE uut's OnStop_WQ is executed in work queue context.
  //      This is guaranteed, because steps 1..3 are implemented as work packages.
  //      Since a work package is used to add the work packages, any work packages
  //      generated by the UUT are added BEHIND step 3.
  // Note:
  // - Three work queue flushes are needed:
  //   1. For the work package added here
  //   2. For the work packages added by the work package added here
  //   3. For OnStop_WQ
  // - OnStop_WQ (the one due to stop request) won't be executed before next trigger or
  //   trigger timeout event. The stop request just sets a flag, which is processed when a
  //   trigger or trigger timeout occurs.
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, StateStoppedStopPending_SampleRetFalse)::CreateStimulus_TriggerAndStopRequest_WQ, this)));
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stoppedStopPending, uut.GetCurrentState());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StopAsync());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StartAsync());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());


  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::BuildTraceValue_OSST_STOPPEDSTOPPEND(TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse),
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, StateStoppedStopPending_PllLossOfLock)
{
  // This test checks state stoppedStopPending. Error used to enter state: PLL loss of lock

  assert(RESTARTS_AFTER_LOSS_OF_LOCK != 0);

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
  wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  controlledTTCE.SetIsPllRunningRetVal(false);

  // Now we create a very special stimulus using the work queue:
  // 1st: Trigger
  // 2nd: Wait for thread (in WQ context)
  // 3rd: Request stop, BEFORE uut's OnStop_WQ is executed in work queue context.
  //      This is guaranteed, because steps 1..3 are implemented as work packages.
  //      Since a work package is used to add the work packages, any work packages
  //      generated by the UUT are added BEHIND step 3.
  // Note:
  // - Three work queue flushes are needed:
  //   1. For the work package added here
  //   2. For the work packages added by the work package added here
  //   3. For OnStop_WQ
  // - OnStop_WQ (the one due to stop request) won't be executed before next trigger or
  //   trigger timeout event. The stop request just sets a flag, which is processed when a
  //   trigger or trigger timeout occurs.
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, StateStoppedStopPending_PllLossOfLock)::CreateStimulus_TriggerAndStopRequest_WQ, this)));
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();
  wq.FlushNonDeferredWorkPackages();

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stoppedStopPending, uut.GetCurrentState());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StopAsync());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::alreadyStopping, uut.StartAsync());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  // do some loops to ensure that there is no automatic restart
  for (size_t i = 0U; i < 5U; i++)
  {
    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    /* check */ wq.FlushNonDeferredWorkPackages();
    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());
  }

  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::BuildTraceValue_OSST_STOPPEDSTOPPEND(TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock),
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_CYCLIC,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_CYCLIC
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, AutomaticRestartAfterPllLossOfLock)
{
  // this test checks the automatic restart after PLL loss of lock

  // this will check that the remaining number of attempts is refreshed upon start
  uut.RefreshRemainingStartAttempts();
  assert(RESTARTS_AFTER_LOSS_OF_LOCK != 2);
  uut.SetRestartAttemptsAfterLossOfLock(2);

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  for (size_t i = 0U; i < 3U; i++)
  {
    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

    size_t const startCycles = (i == 0U)? 1U : (1U + OBRALOL_RETVAL);

    for (size_t j = 0U; j < startCycles; j++)
    {
      // ----------------------------------------------------------------------------------------------
      triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
      ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
      // ----------------------------------------------------------------------------------------------
    }

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

    /* check */ wq.FlushNonDeferredWorkPackages();
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

    for (size_t j = 0U; j < 3U; j++)
    {
      // --------------------------------------------------------------------------------------------
      triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
      ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
      // --------------------------------------------------------------------------------------------

      /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
      /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
    }

    controlledTTCE.SetIsPllRunningRetVal(false);

    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    controlledTTCE.SetIsPllRunningRetVal(true);

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

    wq.FlushNonDeferredWorkPackages();
    if (i < 2U)
    {
      /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());
    }
    else
    {
      /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());
    }
  }

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());


  // An exact comparison of the trace against an expectation is not possible, because the work queue
  // introduces too many possibilities for execution of Cyclic() and other events.

  ASSERT_EQ(3U, trace.Count(Trace::TRACE_ONSTART));
  ASSERT_EQ(3U, trace.Count(Trace::TRACE_ONSTOP));
  ASSERT_EQ(9U, trace.Count(Trace::TRACE_SAMPLE));
  ASSERT_EQ(0U, trace.Count(Trace::TRACE_SAMPLEOVR));

  ASSERT_EQ(2U, trace.Count(Trace::TRACE_OBRALOL));
  ASSERT_EQ(0U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));
  ASSERT_EQ(0U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout)));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock)));
  ASSERT_EQ(0U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse)));
  ASSERT_EQ(3U, trace.Count(Trace::TRACE_OSST_STARTING));
  ASSERT_EQ(3U, trace.Count(Trace::TRACE_OSST_RUNNING));
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, RefreshRemainingStartAttempts)
{
  // same as AutomaticRestartAfterPllLossOfLock, but this test invokes RefreshRemainingStartAttempts() once

  // this will check that the remaining number of attempts is refreshed upon start
  uut.RefreshRemainingStartAttempts();
  assert(RESTARTS_AFTER_LOSS_OF_LOCK != 2U);
  uut.SetRestartAttemptsAfterLossOfLock(2U);

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  bool RefreshRemainingStartAttemptsCalled = false;
  for (size_t i = 0U; i < 4U; i++)
  {
    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

    size_t const startCycles = (i == 0U)? 1U : (1U + OBRALOL_RETVAL);

    for (size_t j = 0U; j < startCycles; j++)
    {
      // ----------------------------------------------------------------------------------------------
      triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
      ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
      // ----------------------------------------------------------------------------------------------
    }

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

    /* check */ wq.FlushNonDeferredWorkPackages();
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

    for (size_t j = 0U; j < 3U; j++)
    {
      if ((i == 1U) && (!RefreshRemainingStartAttemptsCalled))
      {
        uut.RefreshRemainingStartAttempts();
        RefreshRemainingStartAttemptsCalled = true;
      }

      // --------------------------------------------------------------------------------------------
      triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
      ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
      // --------------------------------------------------------------------------------------------

      /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
      /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
    }

    controlledTTCE.SetIsPllRunningRetVal(false);

    // ----------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // ----------------------------------------------------------------------------------------------

    controlledTTCE.SetIsPllRunningRetVal(true);

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

    wq.FlushNonDeferredWorkPackages();
    if (i < 3U)
    {
      /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());
    }
    else
    {
      /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());
    }
  }

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());


  // An exact comparison of the trace against an expectation is not possible, because the work queue
  // introduces too many possibilities for execution of Cyclic() and other events.

  ASSERT_EQ(4U, trace.Count(Trace::TRACE_ONSTART));
  ASSERT_EQ(4U, trace.Count(Trace::TRACE_ONSTOP));
  ASSERT_EQ(12U, trace.Count(Trace::TRACE_SAMPLE));
  ASSERT_EQ(0U, trace.Count(Trace::TRACE_SAMPLEOVR));

  ASSERT_EQ(3U, trace.Count(Trace::TRACE_OBRALOL));
  ASSERT_EQ(0U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));
  ASSERT_EQ(0U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::triggerTimeout)));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::pllLossOfLock)));
  ASSERT_EQ(0U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::sampleRetFalse)));
  ASSERT_EQ(4U, trace.Count(Trace::TRACE_OSST_STARTING));
  ASSERT_EQ(4U, trace.Count(Trace::TRACE_OSST_RUNNING));
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, LockAndUnlockStart)
{
  // This test performs one cycle of request start, run, and stop. During the cycle, start lock and unlock
  // is checked

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  uut.LockStart();
  uut.LockStart();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::locked, uut.StartAsync());

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  uut.UnlockStart();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::locked, uut.StartAsync());

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  uut.UnlockStart();
  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  uut.LockStart();

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());

  for (size_t i = 0U; i < 3U; i++)
  {
    // --------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // --------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
  }

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StopAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::Result::locked, uut.StartAsync());
  uut.UnlockStart();


  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, MultipleLocks)
{
  size_t const n = TTCEStartStopCtrl::MaxNbOfLocks;

  if (n <= 0xFFFFU)
  {
    for (size_t i = 0U; i < n; i++)
    {
      ASSERT_NO_THROW(uut.LockStart());
    }

    ASSERT_THROW(uut.LockStart(), std::logic_error);

    for (size_t i = 0U; i < n; i++)
    {
      ASSERT_NO_THROW(uut.UnlockStart());
    }

    ASSERT_THROW(uut.UnlockStart(), std::logic_error);
  }
}

TEST_F(gpcc_execution_cyclic_TTCEStartStopCtrl_TestsF, WaitUntilStopped)
{
  // this test requests start, runs, and requests stop and checks uut.WaitUntilStopped

  WaitUntilStoppedHelper wus_helper(uut);

  wus_helper.Start();
  ON_SCOPE_EXIT() { wus_helper.Stop(); };

  // ----------------------------------------------------------------------------------------------
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  wus_helper.StartWaiting();
  /* check */ ASSERT_TRUE(wus_helper.WaitUntilStopped(gpcc::time::TimeSpan::ms(GENERAL_TIMEOUT_MS)));

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StartAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::starting, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());

  wus_helper.StartWaiting();

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::waitLock, controlledTTCE.GetCurrentState());
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::starting, uut.GetCurrentState());
  /* check */ ASSERT_FALSE(wus_helper.IsStopped());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
  /* check */ ASSERT_FALSE(wus_helper.IsStopped());

  for (size_t i = 0U; i < 3U; i++)
  {
    // --------------------------------------------------------------------------------------------
    triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
    ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
    // --------------------------------------------------------------------------------------------

    /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::running, controlledTTCE.GetCurrentState());
    /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::running, uut.GetCurrentState());
    /* check */ ASSERT_FALSE(wus_helper.IsStopped());
  }

  ASSERT_EQ(TTCEStartStopCtrl::Result::ok, uut.StopAsync());

  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopPending, uut.GetCurrentState());
  /* check */ ASSERT_FALSE(wus_helper.IsStopped());

  // ----------------------------------------------------------------------------------------------
  triggerProvider.Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK, false);
  ASSERT_TRUE(triggerProvider.WaitForThread(GENERAL_TIMEOUT_MS));
  // ----------------------------------------------------------------------------------------------

  /* check */ ASSERT_EQ(TriggeredThreadedCyclicExec::States::stopped, controlledTTCE.GetCurrentState());

  /* check */ wq.FlushNonDeferredWorkPackages();
  /* check */ ASSERT_EQ(TTCEStartStopCtrl::States::stopped, uut.GetCurrentState());

  /* check */ ASSERT_TRUE(wus_helper.WaitUntilStopped(gpcc::time::TimeSpan::ms(GENERAL_TIMEOUT_MS)));



  ASSERT_EQ(1U, trace.Count(Trace::TRACE_OSST_RUNNING));
  ASSERT_EQ(1U, trace.Count(Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling)));

  const uint32_t expected[] =
  {
    Trace::TRACE_OSST_STARTING,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::starting, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::waitLock, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::running, TriggeredThreadedCyclicExec::StopReasons::none),
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_ONSTART,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::TRACE_OSST_RUNNING | Trace::EXPECT_FLAG_OPTIONAL,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_SAMPLE,
    Trace::TRACE_CYCLIC,

    Trace::TRACE_OSST_STOPPEND,
    Trace::TRACE_ISPLLRUN,
    Trace::TRACE_ONSTOP,
    Trace::BuildTraceValue_OnStateChange(TriggeredThreadedCyclicExec::States::stopped, TriggeredThreadedCyclicExec::StopReasons::reqStopSampling),
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL,
    Trace::TRACE_CYCLIC,
    Trace::BuildTraceValue_OSST_STOPPED(TriggeredThreadedCyclicExec::StopReasons::reqStopSampling) | Trace::EXPECT_FLAG_OPTIONAL
  };

  if (!trace.Check(sizeof(expected) / sizeof(uint32_t), expected))
  {
    trace.Dump();
    ASSERT_TRUE(false);
  }
}

#endif // #ifndef SKIP_TFC_BASED_TESTS

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
