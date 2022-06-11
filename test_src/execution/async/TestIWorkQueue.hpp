/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef TESTIWORKQUEUE_HPP_201701061557
#define TESTIWORKQUEUE_HPP_201701061557

#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/execution/async/DeferredWorkPackage.hpp"
#include "gpcc/src/execution/async/WorkQueue.hpp"
#include "gpcc/src/execution/async/DeferredWorkQueue.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/osal/Semaphore.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <vector>
#include <iostream>
#include <iomanip>
#include <exception>
#include <cassert>
#include <cstddef>

#include "gtest/gtest.h"

// Duration of the sleep contained in some work packages in ms.
#define WP_SLEEPTIME_MS   50

// Time the test case waits for some condition to become true in ms.
// This is far smaller than WP_SLEEPTIME_MS.
#define WAITTIME_MS       10


namespace gpcc_tests {
namespace execution {
namespace async {

using gpcc::execution::async::WorkPackage;
using gpcc::execution::async::WorkQueue;
using gpcc::execution::async::DeferredWorkPackage;
using gpcc::execution::async::DeferredWorkQueue;
using gpcc::osal::Thread;
using gpcc::osal::Semaphore;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;
using gpcc::time::Clocks;
using namespace testing;

/// Test fixture for gpcc::execution::async::IWorkQueue related tests.
/** Usage hints:
 *  - In case of a failed ASSERT_EQ or similar, TearDown() will cancel the work queue thread and join it,
 *    so aborting a test is possible at any time.
 *  - If the unit test itself already joined the work queue thread, then TearDown() will not attempt to
 *    join a second time.
 *  - CheckCheckList() shall only be invoked either after the work queue thread has been joined or
 *    if the design of the test case ensures, that there is no work package invoking WQ_PushToCheckList() or
 *    WQ_PushToCheckListAndEnqueueSelf(). Reason: There is no lock mechanism implemented.
 *
 *  - Typical valid sequences for invoking thread management functions are: (there may be more!)
 *    - EnterUUTWork() -> JoinWorkThread() -> Exit test
 *    - EnterUUTWork() -> JoinWorkThread() -> RestartThread() -> EnterUUTWork() -> ...
 *    - EnterUUTWork() -> RequestThreadCancel() -> JoinWorkThread() -> Exit test
 *    - EnterUUTWork() -> RequestThreadCancel() -> JoinWorkThread() -> RestartThread() -> EnterUUTWork() -> ...
 */
template <typename T>
class IWorkQueue_TestsF: public Test
{
  public:
    IWorkQueue_TestsF(void);

  protected:
    // UUT
    T uut;

    // dummy owners for created work packages
    int owner1;
    int owner2;

    // semaphore used to signal that execution of WQ_WaitForCancel() has started
    Semaphore enteredWaitForCancel;

    // checklist to reproduce calls to WQ_PushToCheckList()
    std::vector<uint32_t> checkList;

    // list containing time stamps
    std::vector<TimePoint> timestampList;

    // repeats for tests enqueing themselves again
    size_t repeats;

    virtual ~IWorkQueue_TestsF(void) = default;

    void SetUp(void) override;
    void TearDown(void) noexcept override;

    // management of the thread executing uut.Work()
    void RestartThread(void);
    void RequestThreadCancel(void);
    void EnterUUTWork(void);
    void JoinWorkThread(void);

    // methods that can be executed in uut's workpackage context
    void WQ_PushToCheckList(uint32_t const checkListValue);
    void WQ_RemoveByRefAndPushToCheckList(uint32_t const checkListValue, WorkPackage* pWP);
    void WQ_PushToCheckListAndEnqueueSelf(uint32_t const checkListValue1, uint32_t const checkListValue2);
    void WQ_PushToCheckListAndEnqueueByRef(uint32_t const checkListValue, WorkPackage* pWP);
    void WQ_PushToCheckListAndInsertAtHeadByRef(uint32_t const checkListValue, WorkPackage* pWP);
    void WQ_Sleep(uint32_t const ms);
    void WQ_WaitForCancel(void);
    void WQ_AddWPTerminate(void);
    void WQ_WaitUntilCurrentWorkPackageHasBeenExecuted(void);
    void WQ_RemoveByRef(WorkPackage* pWP);
    void WQ_Remove(const void* const pOwnerObject);
    void WQ_RemoveAndID(const void* const pOwnerObject, const uint32_t ownerID);

    // check of checkList against expected values
    void PrintCheckList(uint32_t const * pExpectedValues, size_t const n);
    bool CheckCheckList(uint32_t const * const pExpectedValues, size_t const n);

  private:
    // thread used to execute uut.Work()
    Thread thread;

    // flag indicating if thread is joined or not
    bool joined;

    // semaphore used as gate in front of uut.Work()
    Semaphore threadStartTrigger;

    // internal thread entry
    void* ThreadEntry(void);
};

template <typename T>
IWorkQueue_TestsF<T>::IWorkQueue_TestsF()
: Test()
, uut()
, owner1(0)
, owner2(0)
, enteredWaitForCancel(0)
, checkList()
, timestampList()
, repeats(0)
, thread("WQTests")
, joined(true)
, threadStartTrigger(0)
{
  checkList.reserve(32);
  timestampList.reserve(32);
}

template <typename T>
void IWorkQueue_TestsF<T>::SetUp(void)
{
  RestartThread();
}

template <typename T>
void IWorkQueue_TestsF<T>::TearDown(void) noexcept
{
  if (!joined)
  {
    thread.Cancel();
    threadStartTrigger.Post();
    JoinWorkThread();
  }
}

template <typename T>
void IWorkQueue_TestsF<T>::RestartThread(void)
{
  assert(joined);
  thread.Start(std::bind(&IWorkQueue_TestsF::ThreadEntry, this),
               Thread::SchedPolicy::Other,
               0,
               Thread::GetDefaultStackSize());
  joined = false;
}

template <typename T>
void IWorkQueue_TestsF<T>::RequestThreadCancel(void)
{
  thread.Cancel();
}

template <typename T>
void IWorkQueue_TestsF<T>::EnterUUTWork(void)
{
  threadStartTrigger.Post();
}

template <typename T>
void IWorkQueue_TestsF<T>::JoinWorkThread(void)
{
  assert(!joined);
  thread.Join(nullptr);
  joined = true;
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_PushToCheckList(uint32_t const checkListValue)
{
  checkList.push_back(checkListValue);
  timestampList.push_back(TimePoint::FromSystemClock(Clocks::monotonic));
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_RemoveByRefAndPushToCheckList(uint32_t const checkListValue, WorkPackage* pWP)
{
  uut.Remove(*pWP);

  checkList.push_back(checkListValue);
  timestampList.push_back(TimePoint::FromSystemClock(Clocks::monotonic));
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_PushToCheckListAndEnqueueSelf(uint32_t const checkListValue1, uint32_t const checkListValue2)
{
  checkList.push_back(checkListValue1);
  timestampList.push_back(TimePoint::FromSystemClock(Clocks::monotonic));
  uut.Add(WorkPackage::CreateDynamic(this, 0,
                                         std::bind(&IWorkQueue_TestsF<T>::WQ_PushToCheckList, this, checkListValue2)));
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_PushToCheckListAndEnqueueByRef(uint32_t const checkListValue, WorkPackage* pWP)
{
  checkList.push_back(checkListValue);
  timestampList.push_back(TimePoint::FromSystemClock(Clocks::monotonic));

  if (repeats != 0)
  {
    repeats--;
    uut.Add(*pWP);
  }
  else
    WQ_AddWPTerminate();
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_PushToCheckListAndInsertAtHeadByRef(uint32_t const checkListValue, WorkPackage* pWP)
{
  checkList.push_back(checkListValue);
  timestampList.push_back(TimePoint::FromSystemClock(Clocks::monotonic));

  if (repeats != 0)
  {
    repeats--;
    uut.InsertAtHeadOfList(*pWP);
  }
  else
    WQ_AddWPTerminate();
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_Sleep(uint32_t const ms)
{
  Thread::Sleep_ms(ms);
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_WaitForCancel(void)
{
  enteredWaitForCancel.Post();
  while (true)
  {
    thread.TestForCancellation();
    Thread::Sleep_ms(5);
  }
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_AddWPTerminate(void)
{
  uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&T::RequestTermination, &uut)));
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_WaitUntilCurrentWorkPackageHasBeenExecuted(void)
{
  uut.WaitUntilCurrentWorkPackageHasBeenExecuted(&owner2);
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_RemoveByRef(WorkPackage* pWP)
{
  uut.Remove(*pWP);
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_Remove(const void* const pOwnerObject)
{
  uut.Remove(pOwnerObject);
}

template <typename T>
void IWorkQueue_TestsF<T>::WQ_RemoveAndID(const void* const pOwnerObject, const uint32_t ownerID)
{
  uut.Remove(pOwnerObject, ownerID);
}

template <typename T>
void IWorkQueue_TestsF<T>::PrintCheckList(uint32_t const * pExpectedValues, size_t const n)
{
  std::cout << "Recorded: " << std::right << std::setw(3) << std::setfill(' ') << checkList.size() << " items: ";

  for (uint32_t i: checkList)
    std::cout << std::setw(3) << std::setfill(' ') << i << ' ';

  std::cout << std::endl;

  std::cout << "Expected: " << std::right << std::setw(3) << std::setfill(' ') << n << " items: ";

  for (size_t i = 0; i < n; i++)
    std::cout << std::setw(3) << std::setfill(' ') << *pExpectedValues++ << ' ';

  std::cout << std::endl;
}

template <typename T>
bool IWorkQueue_TestsF<T>::CheckCheckList(uint32_t const * const pExpectedValues, size_t const n)
{
  if (checkList.size() != n)
  {
    PrintCheckList(pExpectedValues, n);
    return false;
  }

  uint32_t const * pExVal = pExpectedValues;
  for (size_t i = 0; i < n; i++)
  {
    if (checkList[i] != *pExVal)
    {
      PrintCheckList(pExpectedValues, n);
      return false;
    }
    pExVal++;
  }

  return true;
}

template <typename T>
void* IWorkQueue_TestsF<T>::ThreadEntry(void)
{
  threadStartTrigger.Wait();
  thread.TestForCancellation();
  uut.Work();
  return nullptr;
}


// googletest only supports up to 50 test cases per parametrized test, so we have to
// split our tests into IWorkQueue_Tests1F and IWorkQueue_Tests2F.
template <typename T>
using IWorkQueue_Tests1F = IWorkQueue_TestsF<T>;

template <typename T>
using IWorkQueue_Tests2F = IWorkQueue_TestsF<T>;

template <typename T>
using IWorkQueue_DeathTests1F = IWorkQueue_TestsF<T>;

TYPED_TEST_SUITE_P(IWorkQueue_Tests1F);
TYPED_TEST_SUITE_P(IWorkQueue_Tests2F);
TYPED_TEST_SUITE_P(IWorkQueue_DeathTests1F);


TYPED_TEST_P(IWorkQueue_Tests1F, Instantiation)
{
}

TYPED_TEST_P(IWorkQueue_Tests1F, AddDynamic_copyFunctor)
{
  auto const f1 = std::bind(&AddDynamic_copyFunctor::WQ_PushToCheckList, this, 1);
  auto const f2 = std::bind(&AddDynamic_copyFunctor::WQ_PushToCheckList, this, 2);
  auto const f3 = std::bind(&AddDynamic_copyFunctor::WQ_PushToCheckList, this, 3);

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, f1));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, f2));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, f3));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));
}

TYPED_TEST_P(IWorkQueue_Tests1F, AddDynamic_moveFunctor)
{
  auto f1 = std::bind(&AddDynamic_moveFunctor::WQ_PushToCheckList, this, 1);
  auto f2 = std::bind(&AddDynamic_moveFunctor::WQ_PushToCheckList, this, 2);
  auto f3 = std::bind(&AddDynamic_moveFunctor::WQ_PushToCheckList, this, 3);

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::move(f1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::move(f2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::move(f3)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));
}

TYPED_TEST_P(IWorkQueue_Tests1F, AddDynamic_FromWQContext)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&AddDynamic_FromWQContext::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&AddDynamic_FromWQContext::WQ_PushToCheckListAndEnqueueSelf, this, 2, 4)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&AddDynamic_FromWQContext::WQ_PushToCheckList, this, 3)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  // take a second run to execute the work package enqueued by WQ_PushToCheckListAndEnqueueSelf()

  this->WQ_AddWPTerminate();
  this->RestartThread();
  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[4] = {1, 2, 3, 4};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 4));
}

TYPED_TEST_P(IWorkQueue_Tests1F, AddDynamic_nullptr)
{
  ASSERT_THROW(this->uut.Add(std::unique_ptr<WorkPackage>()), std::invalid_argument);
}

TYPED_TEST_P(IWorkQueue_Tests1F, AddStatic)
{
  WorkPackage wp1(this, 0, std::bind(&AddStatic::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&AddStatic::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&AddStatic::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));
}

TYPED_TEST_P(IWorkQueue_Tests1F, AddStatic_DynamicWP)
{
  auto spWP = WorkPackage::CreateDynamic(this, 0, std::bind(&AddStatic_DynamicWP::WQ_PushToCheckList, this, 1));

  ASSERT_THROW({ this->uut.Add(*spWP.get()); spWP.release(); }, std::logic_error);
}

TYPED_TEST_P(IWorkQueue_Tests1F, InsertAtHeadOfListDynamic)
{
  this->uut.InsertAtHeadOfList(WorkPackage::CreateDynamic(this, 0, std::bind(&InsertAtHeadOfListDynamic::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&InsertAtHeadOfListDynamic::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&InsertAtHeadOfListDynamic::WQ_PushToCheckList, this, 3)));
  this->uut.InsertAtHeadOfList(WorkPackage::CreateDynamic(this, 0, std::bind(&InsertAtHeadOfListDynamic::WQ_PushToCheckList, this, 4)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[4] = {4, 1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 4));
}

TYPED_TEST_P(IWorkQueue_Tests1F, InsertAtHeadOfListDynamic_nullptr)
{
  ASSERT_THROW(this->uut.InsertAtHeadOfList(std::unique_ptr<WorkPackage>()), std::invalid_argument);
}

TYPED_TEST_P(IWorkQueue_Tests1F, InsertAtHeadOfListStatic)
{
  WorkPackage wp1(this, 0, std::bind(&InsertAtHeadOfListStatic::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&InsertAtHeadOfListStatic::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&InsertAtHeadOfListStatic::WQ_PushToCheckList, this, 3));
  WorkPackage wp4(this, 0, std::bind(&InsertAtHeadOfListStatic::WQ_PushToCheckList, this, 4));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.InsertAtHeadOfList(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.InsertAtHeadOfList(wp4);
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[4] = {4, 1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 4));
}

TYPED_TEST_P(IWorkQueue_Tests1F, InsertAtHeadOfListStatic_DynamicWP)
{
  auto spWP = WorkPackage::CreateDynamic(this, 0, std::bind(&InsertAtHeadOfListStatic_DynamicWP::WQ_PushToCheckList, this, 1));

  ASSERT_THROW( { this->uut.InsertAtHeadOfList(*spWP.get()); spWP.release(); }, std::logic_error);
}

TYPED_TEST_P(IWorkQueue_Tests1F, ReuseOfStaticWPs)
{
  WorkPackage wp1(this, 0, std::bind(&ReuseOfStaticWPs::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 0, std::bind(&ReuseOfStaticWPs::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&ReuseOfStaticWPs::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1);
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->WQ_AddWPTerminate();

  this->RestartThread();
  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[5] = {1, 3, 1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 5));
}

TYPED_TEST_P(IWorkQueue_Tests1F, ReuseItself_Add)
{
  WorkPackage wp1(this, 0, std::bind(&ReuseItself_Add::WQ_PushToCheckListAndEnqueueByRef, this, 1, &wp1));
  this->repeats = 3;
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
  };

  this->uut.Add(wp1);
  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[4] = {1, 1, 1, 1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 4));
}

TYPED_TEST_P(IWorkQueue_Tests1F, ReuseItself_Insert)
{
  WorkPackage wp1(this, 0, std::bind(&ReuseItself_Insert::WQ_PushToCheckListAndInsertAtHeadByRef, this, 1, &wp1));
  this->repeats = 3;
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
  };

  this->uut.Add(wp1);
  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[4] = {1, 1, 1, 1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 4));
}

TYPED_TEST_P(IWorkQueue_Tests1F, ReuseButStillInQueue)
{
  WorkPackage wp1(this, 0, std::bind(&ReuseButStillInQueue::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  ASSERT_THROW(this->uut.Add(wp1), std::logic_error);
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Cleanup_dyn)
{
  // Add some dynamic work packages. helgrind/memcheck must not detect any memory leaks.
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Cleanup_dyn::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Cleanup_dyn::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Cleanup_dyn::WQ_PushToCheckList, this, 3)));
  this->WQ_AddWPTerminate();

  // Note: NO EXECUTION
}

TYPED_TEST_P(IWorkQueue_Tests1F, Cleanup_stat)
{
  auto spUUT = std::unique_ptr<gtest_TypeParam_>(new gtest_TypeParam_());
  WorkPackage wp1(this, 0, std::bind(&Cleanup_stat::WQ_PushToCheckList, this, 1));
  spUUT->Add(wp1);
  spUUT.reset();

  // Note: NO EXECUTION
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_first)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_first::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove0_first::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove0_first::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);

  this->uut.Remove(wp1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_mid)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_mid::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove0_mid::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove0_mid::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);

  this->uut.Remove(wp2);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_last)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_last::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove0_last::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove0_last::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);

  this->uut.Remove(wp3);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_fromWQcontext)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_fromWQcontext::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove0_fromWQcontext::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove0_fromWQcontext::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove0_fromWQcontext::WQ_RemoveByRef, this, &wp2)));
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_TheLastOne)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_TheLastOne::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Remove(wp1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_Empty)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_Empty::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Remove(wp1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_NoHit)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_NoHit::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove0_NoHit::WQ_PushToCheckList, this, 2));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);

  this->uut.Remove(wp2);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_DynamicWP)
{
  auto wp1 = WorkPackage::CreateDynamic(this, 0, std::bind(&Remove0_DynamicWP::WQ_PushToCheckList, this, 1));
  ASSERT_THROW(this->uut.Remove(*wp1.get()), std::invalid_argument);
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove0_itself)
{
  WorkPackage wp1(this, 0, std::bind(&Remove0_itself::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove0_itself::WQ_RemoveByRefAndPushToCheckList, this, 2, &wp2));
  WorkPackage wp3(this, 0, std::bind(&Remove0_itself::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_first)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&Remove1_dyn_first::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_first::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_first::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_mid)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_mid::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&Remove1_dyn_mid::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_mid::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_last)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_last::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_last::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&Remove1_dyn_last::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_nullptr)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&Remove1_dyn_nullptr::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(nullptr, 0, std::bind(&Remove1_dyn_nullptr::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(nullptr, 0, std::bind(&Remove1_dyn_nullptr::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(nullptr);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_fromWQcontext)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_fromWQcontext::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_fromWQcontext::WQ_Remove, this, &this->owner1)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&Remove1_dyn_fromWQcontext::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_dyn_fromWQcontext::WQ_PushToCheckList, this, 3)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_TheLastOne)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove1_dyn_TheLastOne::WQ_PushToCheckList, this, 1)));

  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_Empty)
{
  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_dyn_NoHit)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove1_dyn_NoHit::WQ_PushToCheckList, this, 1)));

  this->uut.Remove(&this->owner2);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_first)
{
  WorkPackage wp1(&this->owner1, 0, std::bind(&Remove1_stat_first::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove1_stat_first::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove1_stat_first::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_mid)
{
  WorkPackage wp1(this, 0, std::bind(&Remove1_stat_mid::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 0, std::bind(&Remove1_stat_mid::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove1_stat_mid::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_last)
{
  WorkPackage wp1(this, 0, std::bind(&Remove1_stat_last::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(this, 0, std::bind(&Remove1_stat_last::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(&this->owner1, 0, std::bind(&Remove1_stat_last::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_nullptr)
{
  WorkPackage wp1(&this->owner1, 0, std::bind(&Remove1_stat_nullptr::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(nullptr, 0, std::bind(&Remove1_stat_nullptr::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(nullptr, 0, std::bind(&Remove1_stat_nullptr::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(&this->owner1);
    this->uut.Remove(nullptr);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(nullptr);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_fromWQcontext)
{
  WorkPackage wp1(this, 0, std::bind(&Remove1_stat_fromWQcontext::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 0, std::bind(&Remove1_stat_fromWQcontext::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove1_stat_fromWQcontext::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove1_stat_fromWQcontext::WQ_Remove, this, &this->owner1)));
  this->uut.Add(wp2);
  this->uut.Add(wp3);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_TheLastOne)
{
  WorkPackage wp1(&this->owner1, 1, std::bind(&Remove1_stat_TheLastOne::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(&this->owner1); };

  this->uut.Add(wp1);
  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests1F, Remove1_stat_Empty)
{
  this->uut.Remove(&this->owner1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove1_stat_NoHit)
{
  WorkPackage wp1(&this->owner1, 1, std::bind(&Remove1_stat_NoHit::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(&this->owner1); };

  this->uut.Add(wp1);
  this->uut.Remove(&this->owner2);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_first)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove2_dyn_first::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 2, std::bind(&Remove2_dyn_first::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 1, std::bind(&Remove2_dyn_first::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_mid)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 2, std::bind(&Remove2_dyn_mid::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove2_dyn_mid::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 1, std::bind(&Remove2_dyn_mid::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_last)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 1, std::bind(&Remove2_dyn_last::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 2, std::bind(&Remove2_dyn_last::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove2_dyn_last::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_nullptr)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&Remove2_dyn_nullptr::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(nullptr, 1, std::bind(&Remove2_dyn_nullptr::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(nullptr, 0, std::bind(&Remove2_dyn_nullptr::WQ_PushToCheckList, this, 3)));

  this->uut.Remove(nullptr, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_fromWQcontext)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove2_dyn_fromWQcontext::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove2_dyn_fromWQcontext::WQ_RemoveAndID, this, &this->owner1, 33)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 33, std::bind(&Remove2_dyn_fromWQcontext::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 32, std::bind(&Remove2_dyn_fromWQcontext::WQ_PushToCheckList, this, 3)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_TheLastOne)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove2_dyn_TheLastOne::WQ_PushToCheckList, this, 1)));

  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_Empty)
{
  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_dyn_NoHit)
{
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 1, std::bind(&Remove2_dyn_NoHit::WQ_PushToCheckList, this, 1)));

  this->uut.Remove(&this->owner1, 2);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_first)
{
  WorkPackage wp1(&this->owner1, 1, std::bind(&Remove2_stat_first::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 2, std::bind(&Remove2_stat_first::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove2_stat_first::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_mid)
{
  WorkPackage wp1(&this->owner1, 2, std::bind(&Remove2_stat_mid::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 1, std::bind(&Remove2_stat_mid::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(this, 0, std::bind(&Remove2_stat_mid::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_last)
{
  WorkPackage wp1(this, 1, std::bind(&Remove2_stat_last::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 2, std::bind(&Remove2_stat_last::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(&this->owner1, 1, std::bind(&Remove2_stat_last::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_nullptr)
{
  WorkPackage wp1(&this->owner1, 0, std::bind(&Remove2_stat_nullptr::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(nullptr, 1, std::bind(&Remove2_stat_nullptr::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(nullptr, 0, std::bind(&Remove2_stat_nullptr::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(nullptr);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(wp2);
  this->uut.Add(wp3);
  this->uut.Remove(nullptr, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_fromWQcontext)
{
  WorkPackage wp1(this, 0, std::bind(&Remove2_stat_fromWQcontext::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 33, std::bind(&Remove2_stat_fromWQcontext::WQ_PushToCheckList, this, 2));
  WorkPackage wp3(&this->owner1, 32, std::bind(&Remove2_stat_fromWQcontext::WQ_PushToCheckList, this, 3));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  this->uut.Add(wp1);
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Remove2_stat_fromWQcontext::WQ_RemoveAndID, this, &this->owner1, 33)));
  this->uut.Add(wp2);
  this->uut.Add(wp3);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_TheLastOne)
{
  WorkPackage wp1(&this->owner1, 1, std::bind(&Remove2_stat_TheLastOne::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(&this->owner1); };

  this->uut.Add(wp1);
  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_Empty)
{
  this->uut.Remove(&this->owner1, 1);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(nullptr, 0));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Remove2_stat_NoHit)
{
  WorkPackage wp1(&this->owner1, 1, std::bind(&Remove2_stat_NoHit::WQ_PushToCheckList, this, 1));
  ON_SCOPE_EXIT() { this->uut.Remove(&this->owner1); };

  this->uut.Add(wp1);
  this->uut.Remove(&this->owner1, 2);

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[1] = {1};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 1));
}

TYPED_TEST_P(IWorkQueue_Tests2F, WaitUntilCurrentWorkPackageHasBeenExecuted_nullptr)
{
  ASSERT_THROW(this->uut.WaitUntilCurrentWorkPackageHasBeenExecuted(nullptr), std::invalid_argument);
}

#ifndef SKIP_TFC_BASED_TESTS
TYPED_TEST_P(IWorkQueue_Tests2F, WaitUntilCurrentWorkPackageHasBeenExecuted)
{
  this->EnterUUTWork();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted::WQ_Sleep, this, WP_SLEEPTIME_MS)));

  // allow WQ thread to start
  Thread::Sleep_ms(WAITTIME_MS);

  TimePoint const startTime(TimePoint::FromSystemClock(Clocks::monotonic));
  this->uut.WaitUntilCurrentWorkPackageHasBeenExecuted(this);
  TimePoint const endTime(TimePoint::FromSystemClock(Clocks::monotonic));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == WP_SLEEPTIME_MS - WAITTIME_MS);

  this->WQ_AddWPTerminate();
  this->JoinWorkThread();
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TYPED_TEST_P(IWorkQueue_Tests2F, WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork)
{
  this->EnterUUTWork();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork::WQ_Sleep, this, WP_SLEEPTIME_MS)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork::WQ_Sleep, this, WAITTIME_MS)));

  // allow WQ thread to start
  Thread::Sleep_ms(WAITTIME_MS);

  TimePoint const startTime(TimePoint::FromSystemClock(Clocks::monotonic));
  this->uut.WaitUntilCurrentWorkPackageHasBeenExecuted(this);
  TimePoint const endTime(TimePoint::FromSystemClock(Clocks::monotonic));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == WP_SLEEPTIME_MS - WAITTIME_MS);

  this->WQ_AddWPTerminate();
  this->JoinWorkThread();
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TYPED_TEST_P(IWorkQueue_Tests2F, WaitUntilCurrentWorkPackageHasBeenExecuted_nowait)
{
  this->EnterUUTWork();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_nowait::WQ_Sleep, this, WP_SLEEPTIME_MS)));

  // allow WQ thread to start
  Thread::Sleep_ms(WAITTIME_MS);

  TimePoint const startTime(TimePoint::FromSystemClock(Clocks::monotonic));
  this->uut.WaitUntilCurrentWorkPackageHasBeenExecuted(&this->owner1);
  TimePoint const endTime(TimePoint::FromSystemClock(Clocks::monotonic));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == 0);

  this->WQ_AddWPTerminate();
  this->JoinWorkThread();
}
#endif

TYPED_TEST_P(IWorkQueue_Tests2F, WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext::WQ_WaitUntilCurrentWorkPackageHasBeenExecuted, this)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext::WQ_PushToCheckList, this, 3)));

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));
}

TYPED_TEST_P(IWorkQueue_Tests2F, IsAnyInQueue_dyn)
{
  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&IsAnyInQueue_dyn::WQ_PushToCheckList, this, 1)));

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));

  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&IsAnyInQueue_dyn::WQ_PushToCheckList, this, 2)));

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_TRUE(this->uut.IsAnyInQueue(&this->owner1));

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));
}

TYPED_TEST_P(IWorkQueue_Tests2F, IsAnyInQueue_stat)
{
  WorkPackage wp1(this, 0, std::bind(&IsAnyInQueue_stat::WQ_PushToCheckList, this, 1));
  WorkPackage wp2(&this->owner1, 0, std::bind(&IsAnyInQueue_stat::WQ_PushToCheckList, this, 2));
  ON_SCOPE_EXIT()
  {
    this->uut.Remove(this);
    this->uut.Remove(&this->owner1);
  };

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));

  this->uut.Add(wp1);

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));

  this->uut.Add(wp2);

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_TRUE(this->uut.IsAnyInQueue(&this->owner1));

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[2] = {1, 2};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 2));

  ASSERT_FALSE(this->uut.IsAnyInQueue(nullptr));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));
}

#ifndef SKIP_TFC_BASED_TESTS
TYPED_TEST_P(IWorkQueue_Tests2F, FlushNonDeferredWorkPackages)
{
  this->EnterUUTWork();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&FlushNonDeferredWorkPackages::WQ_Sleep, this, WP_SLEEPTIME_MS)));
  this->uut.Add(WorkPackage::CreateDynamic(&this->owner1, 0, std::bind(&FlushNonDeferredWorkPackages::WQ_Sleep, this, WAITTIME_MS)));

  TimePoint const startTime(TimePoint::FromSystemClock(Clocks::monotonic));
  this->uut.FlushNonDeferredWorkPackages();
  TimePoint const endTime(TimePoint::FromSystemClock(Clocks::monotonic));

  TimeSpan const duration = endTime - startTime;

  ASSERT_TRUE(duration.ms() == WP_SLEEPTIME_MS + WAITTIME_MS);

  ASSERT_FALSE(this->uut.IsAnyInQueue(this));
  ASSERT_FALSE(this->uut.IsAnyInQueue(&this->owner1));

  this->WQ_AddWPTerminate();
  this->JoinWorkThread();
}
#endif

TYPED_TEST_P(IWorkQueue_Tests2F, Work_Restart)
{
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Restart::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Restart::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Restart::WQ_PushToCheckList, this, 3)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Restart::WQ_PushToCheckList, this, 4)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Restart::WQ_PushToCheckList, this, 5)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Restart::WQ_PushToCheckList, this, 6)));
  this->WQ_AddWPTerminate();

  this->RestartThread();
  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[6] = {1, 2, 3, 4, 5, 6};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 6));
}

TYPED_TEST_P(IWorkQueue_Tests2F, Work_Cancel_Restart)
{
  uint32_t const expectedChecklist[6] = {1, 2, 3, 4, 5, 6};

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_PushToCheckList, this, 1)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_PushToCheckList, this, 2)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_PushToCheckList, this, 3)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_WaitForCancel, this)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_PushToCheckList, this, 4)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_PushToCheckList, this, 5)));
  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&Work_Cancel_Restart::WQ_PushToCheckList, this, 6)));
  this->WQ_AddWPTerminate();

  this->EnterUUTWork();

  // wait until work queue thread has entered WQ_WaitForCancel()
  this->enteredWaitForCancel.Wait();

  this->RequestThreadCancel();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));

  this->RestartThread();
  this->EnterUUTWork();
  this->JoinWorkThread();

  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 6));
}

TYPED_TEST_P(IWorkQueue_Tests2F, AbortBeforeStart)
{
  this->RequestThreadCancel();
  this->JoinWorkThread();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&AbortBeforeStart::WQ_PushToCheckList, this, 1)));
  this->WQ_AddWPTerminate();

  this->uut.RequestTermination();
  this->RestartThread();
  this->EnterUUTWork();

  this->RequestThreadCancel();
  this->JoinWorkThread();

  ASSERT_TRUE(this->checkList.empty()) << "No work package should have been executed, but it was.";
}

TYPED_TEST_P(IWorkQueue_Tests2F, AbortTwiceBeforeStart)
{
  this->RequestThreadCancel();
  this->JoinWorkThread();

  this->uut.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&AbortTwiceBeforeStart::WQ_PushToCheckList, this, 1)));
  this->WQ_AddWPTerminate();

  this->uut.RequestTermination();
  EXPECT_NO_THROW(this->uut.RequestTermination());
  this->RestartThread();
  this->EnterUUTWork();

  this->RequestThreadCancel();
  this->JoinWorkThread();

  ASSERT_TRUE(this->checkList.empty()) << "No work package should have been executed, but it was.";
}

TYPED_TEST_P(IWorkQueue_DeathTests1F, EnqueuedStaticWPDestroyed)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto spWP1 = std::unique_ptr<WorkPackage>(new WorkPackage(this, 0, std::bind(&EnqueuedStaticWPDestroyed::WQ_PushToCheckList, this, 1)));
  auto spWP2 = std::unique_ptr<WorkPackage>(new WorkPackage(this, 0, std::bind(&EnqueuedStaticWPDestroyed::WQ_PushToCheckList, this, 2)));
  auto spWP3 = std::unique_ptr<WorkPackage>(new WorkPackage(this, 0, std::bind(&EnqueuedStaticWPDestroyed::WQ_PushToCheckList, this, 3)));
  ON_SCOPE_EXIT() { this->uut.Remove(this); };

  this->uut.Add(*spWP1.get());
  this->uut.Add(*spWP2.get());
  this->uut.Add(*spWP3.get());

  EXPECT_DEATH(spWP2.reset(), ".*WorkPackage::~WorkPackage: Enqueued in work queue.*");

  this->WQ_AddWPTerminate();

  this->EnterUUTWork();
  this->JoinWorkThread();

  uint32_t const expectedChecklist[3] = {1, 2, 3};
  ASSERT_TRUE(this->CheckCheckList(expectedChecklist, 3));
}

REGISTER_TYPED_TEST_SUITE_P(IWorkQueue_Tests1F,
                            Instantiation,
                            AddDynamic_copyFunctor,
                            AddDynamic_moveFunctor,
                            AddDynamic_FromWQContext,
                            AddDynamic_nullptr,
                            AddStatic,
                            AddStatic_DynamicWP,
                            InsertAtHeadOfListDynamic,
                            InsertAtHeadOfListDynamic_nullptr,
                            InsertAtHeadOfListStatic,
                            InsertAtHeadOfListStatic_DynamicWP,
                            ReuseOfStaticWPs,
                            ReuseItself_Add,
                            ReuseItself_Insert,
                            ReuseButStillInQueue,
                            Cleanup_dyn,
                            Cleanup_stat,
                            Remove0_first,
                            Remove0_mid,
                            Remove0_last,
                            Remove0_fromWQcontext,
                            Remove0_TheLastOne,
                            Remove0_Empty,
                            Remove0_NoHit,
                            Remove0_DynamicWP,
                            Remove0_itself,
                            Remove1_dyn_first,
                            Remove1_dyn_mid,
                            Remove1_dyn_last,
                            Remove1_dyn_nullptr,
                            Remove1_dyn_fromWQcontext,
                            Remove1_dyn_TheLastOne,
                            Remove1_dyn_Empty,
                            Remove1_dyn_NoHit,
                            Remove1_stat_first,
                            Remove1_stat_mid,
                            Remove1_stat_last,
                            Remove1_stat_nullptr,
                            Remove1_stat_fromWQcontext,
                            Remove1_stat_TheLastOne,
                            Remove1_stat_Empty); // 41

REGISTER_TYPED_TEST_SUITE_P(IWorkQueue_Tests2F,
                            Remove1_stat_NoHit,
                            Remove2_dyn_first,
                            Remove2_dyn_mid,
                            Remove2_dyn_last,
                            Remove2_dyn_nullptr,
                            Remove2_dyn_fromWQcontext,
                            Remove2_dyn_TheLastOne,
                            Remove2_dyn_Empty,
                            Remove2_dyn_NoHit,
                            Remove2_stat_first,
                            Remove2_stat_mid,
                            Remove2_stat_last,
                            Remove2_stat_nullptr,
                            Remove2_stat_fromWQcontext,
                            Remove2_stat_TheLastOne,
                            Remove2_stat_Empty,
                            Remove2_stat_NoHit,
                            WaitUntilCurrentWorkPackageHasBeenExecuted_nullptr,
#ifndef SKIP_TFC_BASED_TESTS
                            WaitUntilCurrentWorkPackageHasBeenExecuted,
                            WaitUntilCurrentWorkPackageHasBeenExecuted_otherwork,
                            WaitUntilCurrentWorkPackageHasBeenExecuted_nowait,
#endif
                            WaitUntilCurrentWorkPackageHasBeenExecuted_WQcontext,
                            IsAnyInQueue_dyn,
                            IsAnyInQueue_stat,
#ifndef SKIP_TFC_BASED_TESTS
                            FlushNonDeferredWorkPackages,
#endif
                            Work_Restart,
                            Work_Cancel_Restart,
                            AbortBeforeStart,
                            AbortTwiceBeforeStart); // 29

REGISTER_TYPED_TEST_SUITE_P(IWorkQueue_DeathTests1F,
                            EnqueuedStaticWPDestroyed);

} // namespace execution
} // namespace async
} // namespace gpcc_tests

#endif // TESTIWORKQUEUE_HPP_201701061557
