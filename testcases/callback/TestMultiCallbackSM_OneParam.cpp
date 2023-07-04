/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/callback/MultiCallbackSM.hpp>
#include <gpcc/execution/async/WorkPackage.hpp>
#include <gpcc/execution/async/WorkQueue.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace callback {

using namespace gpcc::execution::async;
using namespace gpcc::callback;
using namespace testing;

// Test fixture for gpcc::callback::MultiCallbackSM related tests (one uint8_t parameter passed to registered function)
class gpcc_callback_MultiCallbackSMOneParam_TestsF: public Test
{
  public:
    gpcc_callback_MultiCallbackSMOneParam_TestsF(void);

  protected:
    // Mutex to be used by the UUT.
    gpcc::osal::Mutex uutMutex;

    // UUT
    gpcc::callback::MultiCallbackSM<uint8_t> uut;

    // Mutex used to make stuff thread-safe.
    gpcc::osal::Mutex mutex;

    // Trace
    std::vector<uint8_t> trace;

    // Workqueue used by some tests.
    gpcc::execution::async::WorkQueue wq;

    // Thread for wq.
    gpcc::osal::Thread thread;


    void SetUp(void) override;
    void TearDown(void) override;

    void Trace_Record(uint8_t const i);
    bool Trace_Check(size_t const n, uint8_t const * pExpected);

    void* ThreadEntry(void);
};

gpcc_callback_MultiCallbackSMOneParam_TestsF::gpcc_callback_MultiCallbackSMOneParam_TestsF(void)
: Test()
, uutMutex()
, uut(&uutMutex)
, mutex()
, trace()
, wq()
, thread("gpcc_callback_MultiCallbackSMOneParam_TestsF")
{
  trace.reserve(32);
}

void gpcc_callback_MultiCallbackSMOneParam_TestsF::SetUp(void)
{
  thread.Start(std::bind(&gpcc_callback_MultiCallbackSMOneParam_TestsF::ThreadEntry, this),
               gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  wq.FlushNonDeferredWorkPackages();
}
void gpcc_callback_MultiCallbackSMOneParam_TestsF::TearDown(void)
{
  wq.RequestTermination();
  thread.Join(nullptr);
}

void gpcc_callback_MultiCallbackSMOneParam_TestsF::Trace_Record(uint8_t const i)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  trace.push_back(i);
}
bool gpcc_callback_MultiCallbackSMOneParam_TestsF::Trace_Check(size_t const n, uint8_t const * pExpected)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  if (trace.size() != n)
    return false;

  for (auto const i: trace)
  {
    if (i != *pExpected++)
      return false;
  }

  return true;
}

void* gpcc_callback_MultiCallbackSMOneParam_TestsF::ThreadEntry(void)
{
  wq.Work();
  return nullptr;
}

TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, CreateRelease)
{
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, CreateRelease_OneRegistrationLeft)
{
  uut.Register(this, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, CreateRelease_OneRegistrationLeft)::Trace_Record, this, std::placeholders::_1));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_ZeroRegistrations)
{
  uut.Notify(5);
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_OneRegistration)
{
  uint8_t a = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_OneRegistration)::Trace_Record, this, std::placeholders::_1));

  uut.Notify(3);

  uint8_t const expected[] = { 3 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_ThreeRegistrations)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_ThreeRegistrations)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_ThreeRegistrations)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Notify_ThreeRegistrations)::Trace_Record, this, std::placeholders::_1));

  uut.Notify(23);

  uint8_t const expected[] = { 23, 23, 23 };

  ASSERT_TRUE(Trace_Check(3, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoRegistrationWithoutClient)
{
  uint8_t a = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoRegistrationWithoutClient)::Trace_Record, this, std::placeholders::_1));
  ASSERT_THROW(uut.Register(nullptr, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoRegistrationWithoutClient)::Trace_Record, this, std::placeholders::_1)), std::invalid_argument);

  uut.Notify(44);

  uint8_t const expected[] = { 44 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoRegistrationWithoutFunctor)
{
  uint8_t a = 0;
  uint8_t b = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoRegistrationWithoutFunctor)::Trace_Record, this, std::placeholders::_1));
  ASSERT_THROW(uut.Register(&b, ICallback<uint8_t>::tFunctor()), std::invalid_argument);

  uut.Notify(82);

  uint8_t const expected[] = { 82 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoDoubleRegistration)
{
  uint8_t a = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoDoubleRegistration)::Trace_Record, this, std::placeholders::_1));
  ASSERT_THROW(uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NoDoubleRegistration)::Trace_Record, this, std::placeholders::_1)), std::logic_error);

  uut.Notify(37);

  uint8_t const expected[] = { 37 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister)::Trace_Record, this, std::placeholders::_1));

  uut.Unregister(&b);

  uut.Notify(85);

  uint8_t const expected[] = { 85, 85 };

  ASSERT_TRUE(Trace_Check(2, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_NotRegistered)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;
  uint8_t d = 3;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_NotRegistered)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_NotRegistered)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_NotRegistered)::Trace_Record, this, std::placeholders::_1));

  uut.Unregister(&d);

  uut.Notify(24);

  uint8_t const expected[] = { 24, 24, 24 };

  ASSERT_TRUE(Trace_Check(3, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_All)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_All)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_All)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, Unregister_All)::Trace_Record, this, std::placeholders::_1));

  uut.Unregister(&a);
  uut.Unregister(&b);
  uut.Unregister(&c);

  uut.Notify(65);

  uint8_t const expected[] = { };

  ASSERT_TRUE(Trace_Check(0, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, DifferentThreads)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;
  uint8_t d = 3;

  // Register a & b
  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1));

  // register c (via workqueue) & d
  ICallback<uint8_t>::tFunctor fnc = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1);
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallbackSM<uint8_t>::Register, &uut, &c, fnc)));
  uut.Register(&d, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1));

  // unregister a (via workqueue)
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallbackSM<uint8_t>::Unregister, &uut, &a)));

  // notify (via workqueue)
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallbackSM<uint8_t>::Notify, &uut, 22)));

  // flush workqueue
  wq.FlushNonDeferredWorkPackages();

  // notify
  uut.Notify(45);

  uint8_t const expected[] = { 22, 22, 22, 45, 45, 45 };

  ASSERT_TRUE(Trace_Check(6, expected));
}
TEST_F(gpcc_callback_MultiCallbackSMOneParam_TestsF, NotifyMutexAlreadyLocked)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;
  uint8_t d = 3;

  // Register a & b
  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NotifyMutexAlreadyLocked)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NotifyMutexAlreadyLocked)::Trace_Record, this, std::placeholders::_1));

  // register c (via workqueue) & d
  ICallback<uint8_t>::tFunctor fnc = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NotifyMutexAlreadyLocked)::Trace_Record, this, std::placeholders::_1);
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallbackSM<uint8_t>::Register, &uut, &c, fnc)));
  uut.Register(&d, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackSMOneParam_TestsF, NotifyMutexAlreadyLocked)::Trace_Record, this, std::placeholders::_1));

  // unregister a (via workqueue)
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallbackSM<uint8_t>::Unregister, &uut, &a)));

  // notify (via workqueue)
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallbackSM<uint8_t>::Notify, &uut, 22)));

  // flush workqueue
  wq.FlushNonDeferredWorkPackages();

  // notify
  gpcc::osal::MutexLocker mutexLocker(uutMutex);
  uut.NotifyMutexAlreadyLocked(45);

  uint8_t const expected[] = { 22, 22, 22, 45, 45, 45 };

  ASSERT_TRUE(Trace_Check(6, expected));
}

} // namespace callback
} // namespace gpcc_tests
