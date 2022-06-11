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

#include "gpcc/src/callback/MultiCallback.hpp"
#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/execution/async/WorkQueue.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gtest/gtest.h"
#include <stdexcept>
#include <vector>
#include <cstdint>

namespace gpcc_tests {
namespace callback {

using namespace gpcc::execution::async;
using namespace gpcc::callback;
using namespace testing;

// Test fixture for gpcc::callback::MultiCallback related tests (one uint8_t parameter passed to registered function)
class gpcc_callback_MultiCallbackOneParam_TestsF: public Test
{
  public:
    gpcc_callback_MultiCallbackOneParam_TestsF(void);

  protected:
    // UUT
    gpcc::callback::MultiCallback<uint8_t> uut;

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

gpcc_callback_MultiCallbackOneParam_TestsF::gpcc_callback_MultiCallbackOneParam_TestsF(void)
: Test()
, uut()
, mutex()
, trace()
, wq()
, thread("gpcc_callback_MultiCallbackOneParam_TestsF")
{
  trace.reserve(32);
}

void gpcc_callback_MultiCallbackOneParam_TestsF::SetUp(void)
{
  thread.Start(std::bind(&gpcc_callback_MultiCallbackOneParam_TestsF::ThreadEntry, this),
               gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  wq.FlushNonDeferredWorkPackages();
}
void gpcc_callback_MultiCallbackOneParam_TestsF::TearDown(void)
{
  wq.RequestTermination();
  thread.Join(nullptr);
}

void gpcc_callback_MultiCallbackOneParam_TestsF::Trace_Record(uint8_t const i)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);
  trace.push_back(i);
}
bool gpcc_callback_MultiCallbackOneParam_TestsF::Trace_Check(size_t const n, uint8_t const * pExpected)
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

void* gpcc_callback_MultiCallbackOneParam_TestsF::ThreadEntry(void)
{
  wq.Work();
  return nullptr;
}

TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, CreateRelease)
{
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, CreateRelease_OneRegistrationLeft)
{
  uut.Register(this, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, CreateRelease_OneRegistrationLeft)::Trace_Record, this, std::placeholders::_1));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_ZeroRegistrations)
{
  uut.Notify(0);
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_OneRegistration)
{
  uint8_t a = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_OneRegistration)::Trace_Record, this, std::placeholders::_1));

  uut.Notify(12);

  uint8_t const expected[] = { 12 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_ThreeRegistrations)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_ThreeRegistrations)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_ThreeRegistrations)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Notify_ThreeRegistrations)::Trace_Record, this, std::placeholders::_1));

  uut.Notify(3);

  uint8_t const expected[] = { 3, 3, 3 };

  ASSERT_TRUE(Trace_Check(3, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, NoRegistrationWithoutClient)
{
  uint8_t a = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, NoRegistrationWithoutClient)::Trace_Record, this, std::placeholders::_1));
  ASSERT_THROW(uut.Register(nullptr, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, NoRegistrationWithoutClient)::Trace_Record, this, std::placeholders::_1)), std::invalid_argument);

  uut.Notify(4);

  uint8_t const expected[] = { 4 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, NoRegistrationWithoutFunctor)
{
  uint8_t a = 0;
  uint8_t b = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, NoRegistrationWithoutFunctor)::Trace_Record, this, std::placeholders::_1));
  ASSERT_THROW(uut.Register(&b, ICallback<uint8_t>::tFunctor()), std::invalid_argument);

  uut.Notify(5);

  uint8_t const expected[] = { 5 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, NoDoubleRegistration)
{
  uint8_t a = 0;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, NoDoubleRegistration)::Trace_Record, this, std::placeholders::_1));
  ASSERT_THROW(uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, NoDoubleRegistration)::Trace_Record, this, std::placeholders::_1)), std::logic_error);

  uut.Notify(6);

  uint8_t const expected[] = { 6 };

  ASSERT_TRUE(Trace_Check(1, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister)::Trace_Record, this, std::placeholders::_1));

  uut.Unregister(&b);

  uut.Notify(7);

  uint8_t const expected[] = { 7, 7 };

  ASSERT_TRUE(Trace_Check(2, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_NotRegistered)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;
  uint8_t d = 3;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_NotRegistered)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_NotRegistered)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_NotRegistered)::Trace_Record, this, std::placeholders::_1));

  uut.Unregister(&d);

  uut.Notify(8);

  uint8_t const expected[] = { 8, 8, 8 };

  ASSERT_TRUE(Trace_Check(3, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_All)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;

  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_All)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_All)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&c, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, Unregister_All)::Trace_Record, this, std::placeholders::_1));

  uut.Unregister(&a);
  uut.Unregister(&b);
  uut.Unregister(&c);

  uut.Notify(9);

  uint8_t const expected[] = { };

  ASSERT_TRUE(Trace_Check(0, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, DifferentThreads)
{
  uint8_t a = 0;
  uint8_t b = 1;
  uint8_t c = 2;
  uint8_t d = 3;

  // Register a & b
  uut.Register(&a, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1));
  uut.Register(&b, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1));

  // register c (via workqueue) & d
  ICallback<uint8_t>::tFunctor fnc = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1);
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallback<uint8_t>::Register, &uut, &c, fnc)));
  uut.Register(&d, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_callback_MultiCallbackOneParam_TestsF, DifferentThreads)::Trace_Record, this, std::placeholders::_1));

  // unregister a (via workqueue)
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallback<uint8_t>::Unregister, &uut, &a)));

  // notify (via workqueue)
  wq.Add(WorkPackage::CreateDynamic(this, 0, std::bind(&MultiCallback<uint8_t>::Notify, &uut, 12)));

  // flush workqueue
  wq.FlushNonDeferredWorkPackages();

  // notify
  uut.Notify(10);

  uint8_t const expected[] = { 12, 12, 12, 10, 10, 10 };

  ASSERT_TRUE(Trace_Check(6, expected));
}
TEST_F(gpcc_callback_MultiCallbackOneParam_TestsF, NoNotifyMutexAlreadyLocked)
{
  ASSERT_THROW(uut.NotifyMutexAlreadyLocked(5), std::logic_error);
}

} // namespace callback
} // namespace gpcc_tests
