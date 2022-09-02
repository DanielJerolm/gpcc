/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/ResourceManagement/semaphores/NonBlockingSemaphore.hpp"
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/execution/async/DeferredWorkQueue.hpp>
#include <gpcc/execution/async/DeferredWorkPackage.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include <memory>
#include "gtest/gtest.h"

namespace gpcc_tests         {
namespace ResourceManagement {
namespace semaphores         {

using gpcc::ResourceManagement::semaphores::NonBlockingSemaphore;
using gpcc::execution::async::DeferredWorkPackage;
using namespace testing;

// Test fixture for class NonBlockingSemaphore.
// Test cases may use "Callback()" (referenced by pCallback) to receive callbacks from UUT's Wait()-method.
// "cbCnt" will be decremented upon each invocation of the callback. "cbCnt" shall not become negative, so assign a
// positive value before a callback is expected. "WaitForCbCntZero()" can be used to block the calling thread until
// "cbCnt" is zero again.
class gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF: public Test
{
  public:
    gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF(void);

  protected:
    // workqueue and thread as second execution context used by test cases
    gpcc::execution::async::DeferredWorkQueue dwq;
    gpcc::osal::Thread thread;

    // counter for callbacks received from UUT's Wait()-Method
    gpcc::osal::Mutex cbMutex;
    gpcc::osal::ConditionVariable cbCntZero;
    int32_t cbCnt;

    // functor to callback provided by test fixture
    NonBlockingSemaphore::tSemAcquiredCallback pCallback;

    std::unique_ptr<NonBlockingSemaphore> spUUT;


    void SetUp(void) override;
    void TearDown(void) override;

    void Callback(void);
    void WaitForCbCntZero(void);

  private:
    void* ThreadEntry(void);
};

gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF(void)
: Test()
, dwq()
, thread("RWLockReadLocker_Tests")
, cbMutex()
, cbCntZero()
, cbCnt(0U)
, pCallback(std::bind(&gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::Callback, this))
, spUUT()
{
}

void gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::SetUp(void)
{
  thread.Start(std::bind(&gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::ThreadEntry, this),
                         gpcc::osal::Thread::SchedPolicy::Other, 0U, gpcc::osal::Thread::GetDefaultStackSize());
  dwq.FlushNonDeferredWorkPackages();
}

void gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::TearDown(void)
{
  dwq.RequestTermination();
  thread.Join(nullptr);
}

void gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::Callback(void)
{
  try
  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);

    if (cbCnt == 0)
      gpcc::osal::Panic("gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::Callback: cbCnt is zero");

    if (--cbCnt == 0)
      cbCntZero.Signal();
  }
  catch (...)
  {
    gpcc::osal::Panic("gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::Callback");
  }
}

void gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::WaitForCbCntZero(void)
{
  gpcc::osal::MutexLocker cbMutexLocker(cbMutex);

  while (cbCnt != 0U)
    cbCntZero.Wait(cbMutex);
}

void* gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF::ThreadEntry(void)
{
  dwq.Work();
  return nullptr;
}


using gpcc_resource_management_semaphores_NonBlockingSemaphore_DeathTestsF = gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF;

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, CTOR_OK)
{
  ASSERT_NO_THROW(spUUT = std::make_unique<NonBlockingSemaphore>(0U));
  ASSERT_NO_THROW(spUUT = std::make_unique<NonBlockingSemaphore>(1U));
  ASSERT_NO_THROW(spUUT = std::make_unique<NonBlockingSemaphore>(NonBlockingSemaphore::MAX));
}

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, CTOR_invalidArgs)
{
  // This test is only reasonable, if we can add 1U to NonBlockingSemaphore::MAX without arithmetic overflow
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if (NonBlockingSemaphore::MAX > (std::numeric_limits<size_t>::max() - 1U))
    return;
  #pragma GCC diagnostic pop

  ASSERT_THROW(spUUT = std::make_unique<NonBlockingSemaphore>(NonBlockingSemaphore::MAX + 1U), std::invalid_argument);
}

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_DeathTestsF, DTOR_OutstandingCallbacks)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto callback = []() {};

  spUUT = std::make_unique<NonBlockingSemaphore>(0U);
  ASSERT_FALSE(spUUT->Wait(callback));

  EXPECT_DEATH(spUUT.reset(), ".*At least one waiting thread.*");

  spUUT->Post();
}

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, PostChecksForOverflow)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(NonBlockingSemaphore::MAX - 1U);

  ASSERT_NO_THROW(spUUT->Post());
  EXPECT_THROW(spUUT->Post(), std::logic_error);
}

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, PostIncrementsCounter_SingleThreaded)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  spUUT->Post();
  spUUT->Post();

  ASSERT_TRUE(spUUT->Wait(pCallback));
  ASSERT_TRUE(spUUT->Wait(pCallback));
}

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, PostIncrementsCounter_MultiThreaded)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  // schedule two calls to spUUT->Post()
  auto dwp1 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(100U));
  auto dwp2 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(200U));
  dwq.Add(std::move(dwp1));
  dwq.Add(std::move(dwp2));

  // wait until both calls to Post() have happened (requires TFC)
  gpcc::osal::Thread::Sleep_ms(201U);

  ASSERT_TRUE(spUUT->Wait(pCallback));
  ASSERT_TRUE(spUUT->Wait(pCallback));
}
#endif

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, PostTriggersCallbackForWaiter_SingleThreaded)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  ASSERT_FALSE(spUUT->Wait(pCallback));
  ASSERT_FALSE(spUUT->Wait(pCallback));

  cbCnt = 2;
  spUUT->Post();
  spUUT->Post();
  ASSERT_EQ(cbCnt, 0);
}

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, PostTriggersCallbackForWaiter_MultiThreaded)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  // schedule two calls to spUUT->Post()
  auto dwp1 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(100U));
  auto dwp2 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(200U));
  dwq.Add(std::move(dwp1));
  dwq.Add(std::move(dwp2));

  ASSERT_FALSE(spUUT->Wait(pCallback));
  ASSERT_FALSE(spUUT->Wait(pCallback));

  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);
    cbCnt = 2;
  }

  WaitForCbCntZero();
}
#endif

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, CntInitializedWithPositiveValue)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(2U);

  ASSERT_TRUE(spUUT->Wait(pCallback));
  ASSERT_TRUE(spUUT->Wait(pCallback));
}

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, WaitAndPost_SingleThreaded)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(2U);

  ASSERT_TRUE(spUUT->Wait(pCallback));
  ASSERT_TRUE(spUUT->Wait(pCallback));

  ASSERT_FALSE(spUUT->Wait(pCallback));
  ASSERT_FALSE(spUUT->Wait(pCallback));

  cbCnt = 2;
  spUUT->Post();
  spUUT->Post();
  ASSERT_EQ(cbCnt, 0);
}

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, WaitAndPost_MultiThreaded)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(2U);

  ASSERT_TRUE(spUUT->Wait(pCallback));
  ASSERT_TRUE(spUUT->Wait(pCallback));

  // schedule two calls to spUUT->Post()
  auto dwp1 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(100U));
  auto dwp2 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(200U));
  dwq.Add(std::move(dwp1));
  dwq.Add(std::move(dwp2));

  ASSERT_FALSE(spUUT->Wait(pCallback));
  ASSERT_FALSE(spUUT->Wait(pCallback));

  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);
    cbCnt = 2;
  }

  WaitForCbCntZero();
}
#endif

TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, WaitRejectsNullptr)
{
  spUUT = std::make_unique<NonBlockingSemaphore>(2U);

  ASSERT_THROW(spUUT->Wait(nullptr), std::invalid_argument);

  ASSERT_TRUE(spUUT->Wait(pCallback));
  ASSERT_TRUE(spUUT->Wait(pCallback));
}

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, DeadLockFreeWaitFromCallback_SingleThreaded)
{
  // TFC is used to detect any dead-lock

  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  auto callback = [&]() { EXPECT_FALSE(spUUT->Wait(pCallback)); };

  ASSERT_FALSE(spUUT->Wait(callback));

  spUUT->Post();

  cbCnt = 1;
  spUUT->Post();
  ASSERT_EQ(cbCnt, 0);
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, DeadLockFreeWaitFromCallback_MultiThreaded)
{
  // TFC is used to detect any dead-lock

  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  auto callback = [&]() { EXPECT_FALSE(spUUT->Wait(pCallback)); };

  ASSERT_FALSE(spUUT->Wait(callback));

  // schedule two calls to spUUT->Post()
  auto dwp1 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(100U));
  auto dwp2 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(200U));
  dwq.Add(std::move(dwp1));
  dwq.Add(std::move(dwp2));

  // wait until first call to Post() has happened (requires TFC)
  gpcc::osal::Thread::Sleep_ms(101U);

  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);
    cbCnt = 1;
  }

  // wait until 2nd call to Post() has happened (requires TFC)
  gpcc::osal::Thread::Sleep_ms(100U);

  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);
    ASSERT_EQ(cbCnt, 0);
  }
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, DeadLockFreePostFromCallback_SingleThreaded)
{
  // TFC is used to detect any dead-lock

  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  auto callback = [&]() { spUUT->Post(); };

  ASSERT_FALSE(spUUT->Wait(callback));
  spUUT->Post();

  ASSERT_TRUE(spUUT->Wait(pCallback));
}
#endif

#ifndef SKIP_TFC_BASED_TESTS
TEST_F(gpcc_resource_management_semaphores_NonBlockingSemaphore_TestsF, DeadLockFreePostFromCallback_MultiThreaded)
{
  // TFC is used to detect any dead-lock

  spUUT = std::make_unique<NonBlockingSemaphore>(0U);

  auto callback = [&]() { spUUT->Post(); };

  // schedule one call to spUUT->Post()
  auto dwp1 = DeferredWorkPackage::CreateDynamic(this, 0U, std::bind(&NonBlockingSemaphore::Post, spUUT.get()), gpcc::time::TimeSpan::ms(100U));
  dwq.Add(std::move(dwp1));

  ASSERT_FALSE(spUUT->Wait(callback));
  ASSERT_FALSE(spUUT->Wait(pCallback));

  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);
    cbCnt = 1;
  }

  gpcc::osal::Thread::Sleep_ms(101U);

  {
    gpcc::osal::MutexLocker cbMutexLocker(cbMutex);
    ASSERT_EQ(cbCnt, 0);
  }
}
#endif

} // namespace semaphores
} // namespace ResourceManagement
} // namespace gpcc_tests
