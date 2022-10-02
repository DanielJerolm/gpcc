/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TestIWorkQueue.hpp"
#include <iostream>
#include <iomanip>

// Time span used to delay execution of deferred work packages in ms.
#define DELAY_TIME_MS 10


namespace gpcc_tests {
namespace execution {
namespace async {

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_execution_async_DeferredWorkQueue_, IWorkQueue_Tests1F, DeferredWorkQueue);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_execution_async_DeferredWorkQueue_, IWorkQueue_Tests2F, DeferredWorkQueue);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_execution_async_DeferredWorkQueue_, IWorkQueue_DeathTests1F, DeferredWorkQueue);

// Test fixture for DeferredWorkQueue related tests, This extends the test fixture for class WorkQueue.
class gpcc_execution_async_DeferredWorkQueue_TestsF : public IWorkQueue_TestsF<DeferredWorkQueue>
{
  protected:
    // methods that can be executed in uut's workpackage context
    void WQ_RemoveDWPbyRef(DeferredWorkPackage* pDWP);
    void WQ_RemoveDWPByRefAndPushToCheckList(uint32_t const checkListValue, DeferredWorkPackage* pDWP);
    void WQ_AddDWPbyRef(DeferredWorkPackage* pDWP);
    void WQ_AddDynamicDWP(uint32_t const checkListValue);
    void WQ_PushToCheckListAndEnqueueDWPByRef(uint32_t const checkListValue, DeferredWorkPackage* pDWP);
};

void gpcc_execution_async_DeferredWorkQueue_TestsF::WQ_RemoveDWPbyRef(DeferredWorkPackage* pDWP)
{
  uut.Remove(*pDWP);
}

void gpcc_execution_async_DeferredWorkQueue_TestsF::WQ_RemoveDWPByRefAndPushToCheckList(uint32_t const checkListValue, DeferredWorkPackage* pDWP)
{
  uut.Remove(*pDWP);

  checkList.push_back(checkListValue);
  timestampList.push_back(TimePoint::FromSystemClock(ConditionVariable::clockID));
}

void gpcc_execution_async_DeferredWorkQueue_TestsF::WQ_AddDWPbyRef(DeferredWorkPackage* pDWP)
{
  uut.Add(*pDWP);
}

void gpcc_execution_async_DeferredWorkQueue_TestsF::WQ_AddDynamicDWP(uint32_t const checkListValue)
{
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&gpcc_execution_async_DeferredWorkQueue_TestsF::WQ_PushToCheckList, this, checkListValue),
                                             TimePoint::FromSystemClock(ConditionVariable::clockID) + TimeSpan::ms(DELAY_TIME_MS)));
}

void gpcc_execution_async_DeferredWorkQueue_TestsF::WQ_PushToCheckListAndEnqueueDWPByRef(uint32_t const checkListValue, DeferredWorkPackage* pDWP)
{
  checkList.push_back(checkListValue);
  timestampList.push_back(TimePoint::FromSystemClock(ConditionVariable::clockID));

  if (repeats != 0)
  {
    repeats--;
    pDWP->SetTimeSpan(TimeSpan::ms(DELAY_TIME_MS));
    uut.Add(*pDWP);
  }
  else
    WQ_AddWPTerminate();
}

typedef gpcc_execution_async_DeferredWorkQueue_TestsF gpcc_execution_async_DeferredWorkQueue_DeathTestsF;


TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Instantiation)
{
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_copyFunctor)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_copyFunctor)::WQ_PushToCheckList, this, 1);
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0, f, now + TimeSpan::ms(DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_copyFunctor)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_moveFunctor)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_moveFunctor)::WQ_PushToCheckList, this, 1);
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0, std::move(f), now + TimeSpan::ms(DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_moveFunctor)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  // first in list
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));
  // added to front of list
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));
  // added to back of list
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred)::WQ_PushToCheckList,
                                                       this, 3),
                                             now + TimeSpan::ms(15 * DELAY_TIME_MS)));
  // inserted in middle of list
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred)::WQ_PushToCheckList,
                                                       this, 4),
                                             now + TimeSpan::ms(10 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(20 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 1, 4, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                                       this, 3),
                                             now + TimeSpan::ms(6 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                                       this, 4),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_first)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 4, 1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                                       this, 3),
                                             now + TimeSpan::ms(6 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                                       this, 4),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_mid)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 1, 4, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                                       this, 3),
                                             now + TimeSpan::ms(6 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                                       this, 4),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                                       this, 5),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FifoIfSameTime_last)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[5] = {4, 1, 3, 2, 5};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 5));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_TimepointAlreadyReached)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(5 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(8 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(4 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                                       this, 4),
                                             now - TimeSpan::ms(8 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                                 std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_TimepointAlreadyReached)::WQ_AddWPTerminate,
                                                           this),
                                                 now - TimeSpan::ms(2 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 4, 1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_nullptr)
{
  ASSERT_THROW(uut.Add(std::unique_ptr<DeferredWorkPackage>()), std::invalid_argument);
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FromWQContext)
{
  WorkPackage wp1(this, 0,
                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddDynamic_Deferred_FromWQContext)::WQ_AddDynamicDWP,
                            this, 5));

  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(wp1);
  EnterUUTWork();

  uut.FlushNonDeferredWorkPackages();

  // Wait for 2*DELAY_TIME_MS. The deferred work package is guaranteed to be ready for execution then.
  // Remember that deferred work-packages which are ready for execution have priority above normal work packages.
  // TFC is not required and there is also no dependency on machine performance.
  gpcc::osal::Thread::Sleep_ms(2 * DELAY_TIME_MS);
  WQ_AddWPTerminate();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {5};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_dyn)
{
  // Add some work packages. helgrind/memcheck must not detect any memory leaks.
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_dyn)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_dyn)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_dyn)::WQ_PushToCheckList,
                                                       this, 3),
                                             now + TimeSpan::ms(10 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_dyn)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(12 * DELAY_TIME_MS)));

  // note: NO EXECUTION
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred)::WQ_PushToCheckList,
                                     this, 3),
                           now + TimeSpan::ms(15 * DELAY_TIME_MS));
  DeferredWorkPackage dwp4(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred)::WQ_PushToCheckList,
                                     this, 4),
                           now + TimeSpan::ms(10 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  // first in list
  uut.Add(dwp1);
  // added to front of list
  uut.Add(dwp2);
  // added to back of list
  uut.Add(dwp3);
  // inserted in middle of list
  uut.Add(dwp4);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(20 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 1, 4, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                     this, 3),
                           now + TimeSpan::ms(6 * DELAY_TIME_MS));
  DeferredWorkPackage dwp4(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_first)::WQ_PushToCheckList,
                                     this, 4),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);
  uut.Add(dwp4);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_first)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 4, 1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                     this, 3),
                           now + TimeSpan::ms(6 * DELAY_TIME_MS));
  DeferredWorkPackage dwp4(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_mid)::WQ_PushToCheckList,
                                     this, 4),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);
  uut.Add(dwp4);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_mid)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 1, 4, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(8 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                     this, 3),
                           now + TimeSpan::ms(6 * DELAY_TIME_MS));
  DeferredWorkPackage dwp4(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                     this, 4),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  DeferredWorkPackage dwp5(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)::WQ_PushToCheckList,
                                     this, 5),
                           now + TimeSpan::ms(8 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);
  uut.Add(dwp4);
  uut.Add(dwp5);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FifoIfSameTime_last)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(8 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[5] = {4, 1, 3, 2, 5};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 5));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_TimepointAlreadyReached)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(5 * DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(8 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(4 * DELAY_TIME_MS));
  DeferredWorkPackage dwp4(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_TimepointAlreadyReached)::WQ_PushToCheckList,
                                     this, 4),
                           now - TimeSpan::ms(8 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);
  uut.Add(dwp4);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_TimepointAlreadyReached)::WQ_AddWPTerminate,
                                                       this),
                                             now - TimeSpan::ms(2 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {2, 4, 1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_DynamicDWP)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  auto spDWP = DeferredWorkPackage::CreateDynamic(this, 0,
                                                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_DynamicDWP)::WQ_PushToCheckList,
                                                            this, 1),
                                                  now + TimeSpan::ms(DELAY_TIME_MS));

  ASSERT_THROW({ uut.Add(*spDWP.get()); spDWP.release(); }, std::logic_error);
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FromWQContext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FromWQContext)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(DELAY_TIME_MS));
  WorkPackage wp1(this, 0,
                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, AddStatic_Deferred_FromWQContext)::WQ_AddDWPbyRef,
                            this, &dwp1));

  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(wp1);
  EnterUUTWork();

  uut.FlushNonDeferredWorkPackages();

  // Wait for at least 2*DELAY_TIME_MS. dwp1 is guaranteed to be ready for execution then.
  // Remember that deferred work-packages which are ready for execution have priority above normal work packages.
  // TFC is not required and there is also no dependency on machine performance.
  gpcc::osal::Thread::Sleep_ms(2 * DELAY_TIME_MS);
  WQ_AddWPTerminate();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_stat)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  auto spUUT = std::unique_ptr<DeferredWorkQueue>(new DeferredWorkQueue);
  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Cleanup_stat)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(DELAY_TIME_MS));
  spUUT->Add(dwp1);
  spUUT.reset();

  // Note: NO EXECUTION
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)
{
  TimePoint now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)::WQ_PushToCheckList,
                                     this, 3),
                           now + TimeSpan::ms(15 * DELAY_TIME_MS));
  DeferredWorkPackage dwp4(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)::WQ_PushToCheckList,
                                     this, 4),
                           now + TimeSpan::ms(10 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);
  uut.Add(dwp4);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(20 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

  now.LatchSystemClock(ConditionVariable::clockID);
  dwp1.SetTimePoint(now + TimeSpan::ms(5 * DELAY_TIME_MS));
  dwp2.SetTimePoint(now + TimeSpan::ms(10 * DELAY_TIME_MS));
  dwp3.SetTimePoint(now + TimeSpan::ms(10 * DELAY_TIME_MS));
  dwp4.SetTimePoint(now + TimeSpan::ms(2 * DELAY_TIME_MS));

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);
  uut.Add(dwp4);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(20 * DELAY_TIME_MS)));

  RestartThread();
  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[8] = {2, 1, 4, 3, 4, 1, 2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 8));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_EnqueuedWhileExecuting)
{
  TimePoint now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_EnqueuedWhileExecuting)::WQ_PushToCheckListAndEnqueueDWPByRef,
                                     this, 1, &dwp1),
                           now + TimeSpan::ms(DELAY_TIME_MS));

  ON_SCOPE_EXIT() { uut.Remove(this); };
  repeats = 3;

  uut.Add(dwp1);
  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[4] = {1, 1, 1, 1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 4));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_ChangeExpirationTimeWhileEnqueued)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_ChangeExpirationTimeWhileEnqueued)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(5 * DELAY_TIME_MS));

  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_ChangeExpirationTimeWhileEnqueued)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_ChangeExpirationTimeWhileEnqueued)::WQ_PushToCheckList,
                                     this, 3),
                           now + TimeSpan::ms(15 * DELAY_TIME_MS));

  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, ReuseOfStaticDWPs_ChangeExpirationTimeWhileEnqueued)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(20 * DELAY_TIME_MS)));

  EXPECT_THROW(dwp1.SetTimePoint(now + TimeSpan::ms(100 * DELAY_TIME_MS)), std::logic_error);
  EXPECT_THROW(dwp2.SetTimeSpan(TimeSpan::ms(100 * DELAY_TIME_MS)), std::logic_error);

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[3] = {2, 1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 3));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(WorkPackage::CreateDynamic(this, 0,
                                     std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)::WQ_PushToCheckList,
                                               this, 1)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)::WQ_PushToCheckList,
                                                       this, 3),
                                             now + TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)::WQ_PushToCheckListAndEnqueueSelf,
                                                       this, 4, 5),
                                             now + TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)::WQ_PushToCheckList,
                                                       this, 6),
                                             now + TimeSpan::ms(DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Work_DeferredHasPriority)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));

  EnterUUTWork();
  JoinWorkThread();

#ifndef SKIP_TFC_BASED_TESTS
  uint32_t const expectedChecklist1[6] = {2, 1, 3, 4, 6, 5};
  ASSERT_TRUE(CheckCheckList(expectedChecklist1, 6));
#else
  uint32_t const expectedChecklist1[6] = {2, 1, 3, 4, 6, 5};
  uint32_t const expectedChecklist2[6] = {2, 3, 4, 6, 1, 5};
  ASSERT_TRUE((CheckCheckList(expectedChecklist1, 6)) || (CheckCheckList(expectedChecklist2, 6)));
#endif
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_first)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_first)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_first)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(dwp1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_mid)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_mid)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_mid)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(dwp2);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_last)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_last)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_last)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(dwp3);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_fromWQcontext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_fromWQcontext)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_fromWQcontext)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_fromWQcontext)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  DeferredWorkPackage dwpRem(this, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_fromWQcontext)::WQ_RemoveDWPbyRef,
                                                this, &dwp2),
                             now - TimeSpan::ms(DELAY_TIME_MS));

  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwpRem);
  uut.Add(dwp2);
  uut.Add(dwp3);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_TheLastOne)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_TheLastOne)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);

  uut.Remove(dwp1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_Empty)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_Empty)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Remove(dwp1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_NoHit)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_NoHit)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_NoHit)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);

  uut.Remove(dwp2);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_DynamicWP)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));
  auto dwp1 = DeferredWorkPackage::CreateDynamic(this, 0,
                                                 std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_DynamicWP)::WQ_PushToCheckList,
                                                           this, 1),
                                                 now - TimeSpan::ms(DELAY_TIME_MS));
  ASSERT_THROW(uut.Remove(*dwp1.get()), std::invalid_argument);
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_itself)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_itself)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_itself)::WQ_RemoveDWPByRefAndPushToCheckList,
                                     this, 2, &dwp2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Remove0_itself)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 3));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_first)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_first)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_first)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_mid)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_mid)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_mid)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_last)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_last)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_last)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_nullptr)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_nullptr)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(nullptr, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_nullptr)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(nullptr, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_nullptr)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(nullptr);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_fromWQcontext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_fromWQcontext)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_fromWQcontext)::WQ_Remove,
                                                       this, &owner1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_fromWQcontext)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_fromWQcontext)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_TheLastOne)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_TheLastOne)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_Empty)
{
  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_NoHit)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_dyn_NoHit)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner2);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_first)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_first)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_first)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(this);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_mid)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_mid)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_mid)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(this);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_last)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_last)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_last)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(this);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_nullptr)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_nullptr)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(nullptr, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_nullptr)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(nullptr, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_nullptr)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(nullptr);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(nullptr);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_fromWQcontext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_fromWQcontext)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_fromWQcontext)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_fromWQcontext)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  DeferredWorkPackage dwpRem(this, 0,
                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_fromWQcontext)::WQ_Remove,
                                       this, &owner1),
                             now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(this);
    uut.Remove(&owner1);
  };

  uut.Add(dwp1);
  uut.Add(dwpRem);
  uut.Add(dwp2);
  uut.Add(dwp3);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_TheLastOne)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_TheLastOne)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(&owner1); };

  uut.Add(dwp1);

  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_Empty)
{
  uut.Remove(&owner1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_NoHit)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove1_stat_NoHit)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(&owner1); };

  uut.Add(dwp1);

  uut.Remove(&owner2);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_first)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 2,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_first)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_first)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 2,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_mid)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_mid)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_mid)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_last)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 2,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_last)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_last)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_nullptr)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_nullptr)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(nullptr, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_nullptr)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(nullptr, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_nullptr)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(nullptr, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_fromWQcontext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_fromWQcontext)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_fromWQcontext)::WQ_RemoveAndID,
                                                       this, &owner1, 33),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 33,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_fromWQcontext)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 32,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_fromWQcontext)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_TheLastOne)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_TheLastOne)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_Empty)
{
  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_NoHit)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 1,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_dyn_NoHit)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  uut.Remove(&owner1, 2);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_first)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_first)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 2,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_first)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_first)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(this);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_mid)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 2,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_mid)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_mid)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(this, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_mid)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(this);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_last)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_last)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 2,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_last)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(&owner1, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_last)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(this);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_nullptr)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_nullptr)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(nullptr, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_nullptr)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(nullptr, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_nullptr)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(&owner1);
    uut.Remove(nullptr);
  };

  uut.Add(dwp1);
  uut.Add(dwp2);
  uut.Add(dwp3);

  uut.Remove(nullptr, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_fromWQcontext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_fromWQcontext)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 33,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_fromWQcontext)::WQ_PushToCheckList,
                                     this, 2),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp3(&owner1, 32,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_fromWQcontext)::WQ_PushToCheckList,
                                     this, 3),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  DeferredWorkPackage dwpRem(this, 0,
                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_fromWQcontext)::WQ_RemoveAndID,
                                       this, &owner1, 33),
                             now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT()
  {
    uut.Remove(this);
    uut.Remove(&owner1);
  };

  uut.Add(dwp1);
  uut.Add(dwpRem);
  uut.Add(dwp2);
  uut.Add(dwp3);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_TheLastOne)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_TheLastOne)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(&owner1); };

  uut.Add(dwp1);
  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_Empty)
{
  uut.Remove(&owner1, 1);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(nullptr, 0));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_NoHit)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(&owner1, 1,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Remove2_stat_NoHit)::WQ_PushToCheckList,
                                     this, 1),
                           now - TimeSpan::ms(DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(&owner1); };

  uut.Add(dwp1);
  uut.Remove(&owner1, 2);

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
}

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  EnterUUTWork();

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted)::WQ_Sleep,
                                                       this, WP_SLEEPTIME_MS),
                                                       now + TimeSpan::ms(WAITTIME_MS)));

  // allow WQ thread to block on the sleep in the work package
  Thread::Sleep_ms(WAITTIME_MS + 1);

  TimePoint const startTime(TimePoint::FromSystemClock(ConditionVariable::clockID));
  uut.WaitUntilCurrentWorkPackageHasBeenExecuted(this);
  TimePoint const endTime(TimePoint::FromSystemClock(ConditionVariable::clockID));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == WP_SLEEPTIME_MS - 1);

  WQ_AddWPTerminate();
  JoinWorkThread();
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  EnterUUTWork();

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork)::WQ_Sleep,
                                                       this, 5 * WP_SLEEPTIME_MS),
                                                       now + TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork)::WQ_Sleep,
                                                       this, WP_SLEEPTIME_MS),
                                                       now + TimeSpan::ms(DELAY_TIME_MS)));

  // allow WQ thread to block on the sleep in the work package
  Thread::Sleep_ms(DELAY_TIME_MS + 1);

  TimePoint const startTime(TimePoint::FromSystemClock(ConditionVariable::clockID));
  uut.WaitUntilCurrentWorkPackageHasBeenExecuted(this);
  TimePoint const endTime(TimePoint::FromSystemClock(ConditionVariable::clockID));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == (5 * WP_SLEEPTIME_MS) - 1);

  WQ_AddWPTerminate();
  JoinWorkThread();
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_nowait)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  EnterUUTWork();

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_nowait)::WQ_Sleep,
                                                       this, 5 * WP_SLEEPTIME_MS),
                                                       now + TimeSpan::ms(DELAY_TIME_MS)));

  // allow WQ thread to block on the sleep in the work package
  Thread::Sleep_ms(DELAY_TIME_MS + 1);

  TimePoint const startTime(TimePoint::FromSystemClock(ConditionVariable::clockID));
  uut.WaitUntilCurrentWorkPackageHasBeenExecuted(&owner1);
  TimePoint const endTime(TimePoint::FromSystemClock(ConditionVariable::clockID));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == 0);

  WQ_AddWPTerminate();
  JoinWorkThread();
}
#endif

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext)::WQ_WaitUntilCurrentWorkPackageHasBeenExecuted,
                                                       this),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 3));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_IsAnyInQueue_dyn)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(uut.IsAnyInQueue(&owner1));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_IsAnyInQueue_dyn)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(DELAY_TIME_MS)));

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(uut.IsAnyInQueue(&owner1));

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_IsAnyInQueue_dyn)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_TRUE(uut.IsAnyInQueue(&owner1));

  // Sleep until the two deferred workpackages are for sure runnable before we invoke WQ_AddWPTerminate.
  // TFC is not required and there is no load dependency.
  Thread::Sleep_ms(2 * DELAY_TIME_MS + 1);
  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(uut.IsAnyInQueue(&owner1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_IsAnyInQueue_stat)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  DeferredWorkPackage dwp1(this, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_IsAnyInQueue_stat)::WQ_PushToCheckList,
                                     this, 1),
                           now + TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(&owner1, 0,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_IsAnyInQueue_stat)::WQ_PushToCheckList,
                                     this, 2),
                           now + TimeSpan::ms(2 * DELAY_TIME_MS));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(uut.IsAnyInQueue(&owner1));

  uut.Add(dwp1);

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(uut.IsAnyInQueue(&owner1));

  uut.Add(dwp2);

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_TRUE(uut.IsAnyInQueue(&owner1));

  // Sleep until the two deferred workpackages are for sure runnable before we invoke WQ_AddWPTerminate.
  // TFC is not required and there is no load dependency.
  Thread::Sleep_ms(2 * DELAY_TIME_MS + 1);
  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));

  ASSERT_FALSE(uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(uut.IsAnyInQueue(&owner1));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)::WQ_PushToCheckList,
                                                       this, 4),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)::WQ_PushToCheckList,
                                                       this, 5),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Restart)::WQ_PushToCheckList,
                                                       this, 6),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  WQ_AddWPTerminate();

  RestartThread();
  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[6] = {1, 2, 3, 4, 5, 6};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 6));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)
{
  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uint32_t const expectedChecklist[6] = {1, 2, 3, 4, 5, 6};

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_PushToCheckList,
                                                       this, 1),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_PushToCheckList,
                                                       this, 2),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_PushToCheckList,
                                                       this, 3),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_WaitForCancel,
                                                       this),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_PushToCheckList,
                                                       this, 4),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_PushToCheckList,
                                                       this, 5),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));
  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, Deferred_Work_Cancel_Restart)::WQ_PushToCheckList,
                                                       this, 6),
                                             now - TimeSpan::ms(DELAY_TIME_MS)));

  WQ_AddWPTerminate();

  EnterUUTWork();

  // wait until work queue thread has entered WQ_WaitForCancel()
  enteredWaitForCancel.Wait();

  RequestThreadCancel();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(expectedChecklist, 3));

  RestartThread();
  EnterUUTWork();
  JoinWorkThread();

  ASSERT_TRUE(CheckCheckList(expectedChecklist, 6));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, InsertionWithShorterDelay)
{
  EnterUUTWork();

  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, InsertionWithShorterDelay)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));

  Thread::Sleep_ms(1 * DELAY_TIME_MS);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, InsertionWithShorterDelay)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(3 * DELAY_TIME_MS)));

  Thread::Sleep_ms(1 * DELAY_TIME_MS);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, InsertionWithShorterDelay)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(6 * DELAY_TIME_MS)));

  JoinWorkThread();

#ifndef SKIP_TFC_BASED_TESTS
  uint32_t const expectedChecklist[2] = {2, 1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 2));
#endif

  TimeSpan const delay1 = timestampList[0] - now;
  TimeSpan const delay2 = timestampList[1] - now;

#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE(delay1.ms() == 3 * DELAY_TIME_MS);
  ASSERT_TRUE(delay2.ms() == 5 * DELAY_TIME_MS);
#else
  #ifndef SKIP_LOAD_DEPENDENT_TESTS
    ASSERT_TRUE((delay1.ms() >= 3 * DELAY_TIME_MS) && (delay1.ms() <= 4 * DELAY_TIME_MS));
    ASSERT_TRUE((delay2.ms() >= 5 * DELAY_TIME_MS) && (delay2.ms() <= 6 * DELAY_TIME_MS));
  #endif
#endif

  // print results
  std::cout << "Delay 1: " << delay1.us() << "us" << std::endl;
  std::cout << "Delay 2: " << delay2.us() << "us" << std::endl;
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, RemoveFirstDeferredWorkPackageFromQueue)
{
  // this test checks that the wake-up time is recalculated when the first work package is removed
  EnterUUTWork();

  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, RemoveFirstDeferredWorkPackageFromQueue)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(5 * DELAY_TIME_MS)));

  Thread::Sleep_ms(1 * DELAY_TIME_MS);

  uut.Add(DeferredWorkPackage::CreateDynamic(&owner1, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, RemoveFirstDeferredWorkPackageFromQueue)::WQ_PushToCheckList,
                                                       this, 2),
                                             now + TimeSpan::ms(3 * DELAY_TIME_MS)));

  Thread::Sleep_ms(1 * DELAY_TIME_MS);

  uut.Remove(&owner1);

  Thread::Sleep_ms(1 * DELAY_TIME_MS);

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, RemoveFirstDeferredWorkPackageFromQueue)::WQ_AddWPTerminate,
                                                       this),
                                             now + TimeSpan::ms(6 * DELAY_TIME_MS)));

  JoinWorkThread();

#ifndef SKIP_TFC_BASED_TESTS
  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));
#endif

  TimeSpan const delay = timestampList[0] - now;

#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE(delay.ms() == 5 * DELAY_TIME_MS);
#else
  #ifndef SKIP_LOAD_DEPENDENT_TESTS
    ASSERT_TRUE((delay.ms() >= 5 * DELAY_TIME_MS) && (delay.ms() <= 6 * DELAY_TIME_MS));
  #endif
#endif

  // print results
  std::cout << "Delay: " << delay.us() << "us" << std::endl;
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, DeferredWPbecomesRunnableDuringWPExec)
{
  EnterUUTWork();

  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, DeferredWPbecomesRunnableDuringWPExec)::WQ_PushToCheckList,
                                                       this, 1),
                                             now + TimeSpan::ms(1 * DELAY_TIME_MS)));
  uut.Add(WorkPackage::CreateDynamic(this, 0,
                                     std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, DeferredWPbecomesRunnableDuringWPExec)::WQ_Sleep,
                                     this, 5 * DELAY_TIME_MS)));

  uut.Add(DeferredWorkPackage::CreateDynamic(this, 0,
                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, DeferredWPbecomesRunnableDuringWPExec)::WQ_AddWPTerminate,
                                             this),
                                             now + TimeSpan::ms(2 * DELAY_TIME_MS)));

  JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 1));

  TimeSpan const delay = timestampList[0] - now;

#ifndef SKIP_TFC_BASED_TESTS
  ASSERT_TRUE(delay.ms() == 5 * DELAY_TIME_MS);
#else
  #ifndef SKIP_LOAD_DEPENDENT_TESTS
    ASSERT_TRUE((delay.ms() >= 5 * DELAY_TIME_MS) && (delay.ms() <= 6 * DELAY_TIME_MS));
  #endif
#endif

  // print results
  std::cout << "Delay: " << delay.us() << "us" << std::endl;
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_DeathTestsF, EnqueuedStaticDWPDestroyed)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  auto spDWP1 = std::unique_ptr<DeferredWorkPackage>(new DeferredWorkPackage(this, 0,
                                                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_DeathTestsF, EnqueuedStaticDWPDestroyed)::WQ_PushToCheckList,
                                                                                       this, 1),
                                                     now - TimeSpan::ms(DELAY_TIME_MS)));
  auto spDWP2 = std::unique_ptr<DeferredWorkPackage>(new DeferredWorkPackage(this, 0,
                                                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_DeathTestsF, EnqueuedStaticDWPDestroyed)::WQ_PushToCheckList,
                                                                                       this, 2),
                                                     now - TimeSpan::ms(DELAY_TIME_MS)));
  auto spDWP3 = std::unique_ptr<DeferredWorkPackage>(new DeferredWorkPackage(this, 0,
                                                                             std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_DeathTestsF, EnqueuedStaticDWPDestroyed)::WQ_PushToCheckList,
                                                                                       this, 3),
                                                     now - TimeSpan::ms(DELAY_TIME_MS)));
  ON_SCOPE_EXIT() { uut.Remove(this); };

  uut.Add(*spDWP1.get());
  uut.Add(*spDWP2.get());
  uut.Add(*spDWP3.get());

  EXPECT_DEATH(spDWP2.reset(), ".*DeferredWorkPackage::~DeferredWorkPackage: Enqueued in work queue.*");

  WQ_AddWPTerminate();

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 3));
}

TEST_F(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)
{
  using gpcc::execution::async::IDeferredWorkQueue;
  IDeferredWorkQueue& idwq = static_cast<IDeferredWorkQueue&>(uut);

  TimePoint const now(TimePoint::FromSystemClock(ConditionVariable::clockID));

  auto spWP1 = WorkPackage::CreateDynamic(this, 0,
                                          std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                                                    this, 1));

  WorkPackage wp2(this, 1,
                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                            this, 2));

  WorkPackage wp3(this, 2,
                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                            this, 3));

  WorkPackage wp4(&wp3, 3,
                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                            this, 4));

  WorkPackage wp5(this, 4,
                  std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                            this, 5));


  auto spDWP1 = DeferredWorkPackage::CreateDynamic(this, 6,
                                                   std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                                                             this, 5),
                                                   now - TimeSpan::ms(DELAY_TIME_MS));
  DeferredWorkPackage dwp2(this, 7,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                                     this, 6),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  DeferredWorkPackage dwp3(this, 8,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                                     this, 7),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  DeferredWorkPackage dwp4(&dwp3, 9,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                                     this, 8),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  DeferredWorkPackage dwp5(this, 10,
                           std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkQueue_TestsF, UseIDeferredWorkQueue)::WQ_PushToCheckList,
                                     this, 9),
                           now - TimeSpan::ms(DELAY_TIME_MS));

  ON_SCOPE_EXIT()
  {
    uut.Remove(this);
    uut.Remove(static_cast<void*>(&wp3));   // casted to void* to remove any doubt
    uut.Remove(static_cast<void*>(&dwp3));
  };

  idwq.Add(std::move(spWP1));
  idwq.Add(wp2);
  idwq.Add(wp3);
  idwq.Add(wp4);
  idwq.Add(wp5);

  idwq.Add(std::move(spDWP1));
  idwq.Add(dwp2);
  idwq.Add(dwp3);
  idwq.Add(dwp4);
  idwq.Add(dwp5);

  WQ_AddWPTerminate();

  idwq.Remove(wp2);
  idwq.Remove(&wp3); // wp3 is owner of wp4
  idwq.Remove(this, 4);
  idwq.Remove(dwp3);

  EnterUUTWork();
  JoinWorkThread();

  uint32_t const expectedChecklist[6] = {5, 6, 8, 9, 1, 3};
  ASSERT_TRUE(CheckCheckList(expectedChecklist, 6));
}

} // namespace execution
} // namespace async
} // namespace gpcc_tests
