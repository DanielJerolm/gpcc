/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/exceptions.hpp>
#include <gpcc/osal/RWLock.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/osal/RWLockReadLocker.hpp>
#include <gpcc/osal/RWLockWriteLocker.hpp>
#include <gpcc/execution/async/DeferredWorkQueue.hpp>
#include <gpcc/execution/async/WorkPackage.hpp>
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <memory>

namespace gpcc_tests {
namespace osal       {

using namespace gpcc::osal;
using namespace testing;

// Test fixture for class RWLockWriteLocker.
// This provides a work queue plus thread which allows unit tests to issue stimuli using different threads.
class gpcc_osal_RWLockWriteLocker_TestsF: public Test
{
  public:
    gpcc_osal_RWLockWriteLocker_TestsF(void);

  protected:
    gpcc::execution::async::DeferredWorkQueue dwq;
    gpcc::osal::Thread thread;

    void SetUp(void) override;
    void TearDown(void) override;

    void TestWriteLocked(RWLock & lock);
    void TestNotWriteLocked(RWLock & lock);

  private:
    void* ThreadEntry(void);
};

gpcc_osal_RWLockWriteLocker_TestsF::gpcc_osal_RWLockWriteLocker_TestsF(void)
: Test()
, dwq()
, thread("RWLockWriteLocker_Tests")
{
}

void gpcc_osal_RWLockWriteLocker_TestsF::SetUp(void)
{
  thread.Start(std::bind(&gpcc_osal_RWLockWriteLocker_TestsF::ThreadEntry, this), osal::Thread::SchedPolicy::Other, 0, osal::Thread::GetDefaultStackSize());
  dwq.FlushNonDeferredWorkPackages();
}

void gpcc_osal_RWLockWriteLocker_TestsF::TearDown(void)
{
  dwq.RequestTermination();
  thread.Join(nullptr);
}

void gpcc_osal_RWLockWriteLocker_TestsF::TestWriteLocked(RWLock & lock)
{
  // Adds a non-fatal failure to the test, if the given lock is not write-locked.
  // A different thread (dwq) than the calling one is used to perform the test.

  auto func1 = [&]()
  {
    if (lock.TryReadLock())
    {
      ADD_FAILURE();
      lock.ReleaseReadLock();
    }
  };

  dwq.Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0, func1));
  dwq.FlushNonDeferredWorkPackages();
}

void gpcc_osal_RWLockWriteLocker_TestsF::TestNotWriteLocked(RWLock & lock)
{
  // Adds a non-fatal failure to the test, if the given lock is write-locked.
  // A different thread (dwq) than the calling one is used to perform the test.

  auto func1 = [&]()
  {
    if (!lock.TryReadLock())
    {
      ADD_FAILURE();
    }
    else
    {
      lock.ReleaseReadLock();
    }
  };

  dwq.Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0, func1));
  dwq.FlushNonDeferredWorkPackages();
}

void* gpcc_osal_RWLockWriteLocker_TestsF::ThreadEntry(void)
{
  dwq.Work();
  return nullptr;
}

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockByPtr)
{
  RWLock lock;

  {
    RWLockWriteLocker uut(&lock);

    TestWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

TEST(gpcc_osal_RWLockWriteLocker_Tests, LockNullptr)
{
  RWLockWriteLocker uut(nullptr);
}

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockByRef)
{
  RWLock lock;

  {
    RWLockWriteLocker uut(lock);

    TestWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockWithAbsTimeout_NoTimeoutExpiration)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    // TFC not required and no load dependency:
    // If the lock is free, then WriteLock() will even succeed if the timeout is already expired.
    auto const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(100);

    RWLockWriteLocker uut(lock, absTimeout);

    TestWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockWithAbsTimeout_TimeoutAlreadyExpired)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    // TFC not required and no load dependency:
    // If the lock is free, then WriteLock() will even succeed if the timeout is already expired.
    auto const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) - TimeSpan::ms(100);

    RWLockWriteLocker uut(lock, absTimeout);

    TestWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockWithAbsTimeout_TimeoutExpires)
{
  using namespace gpcc::time;

  RWLock lock;

  std::atomic<bool> ok(false);
  auto const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(10);

  // this acquires a read-lock and will prevent acquisition of the write-lock
  std::unique_ptr<RWLockReadLocker> spReadLocker(new RWLockReadLocker(lock));

  // try to acquire write-lock from a different thread
  auto func1 = [&]()
  {
    try
    {
      RWLockWriteLocker uut(lock, absTimeout);

      ADD_FAILURE();
    }
    catch (gpcc::osal::TimeoutError const &)
    {
      // good
      ok = true;
    }
    catch (std::exception const &)
    {
      ADD_FAILURE();
    }
  };

  dwq.Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0, func1));
  gpcc::osal::Thread::Sleep_ms(20);

  // if no success until now, then release the read-lock to allow the other thread to continue
  // acquiring the write-lock
  if (!ok)
  {
    ADD_FAILURE();
    spReadLocker.reset();
  }

  dwq.FlushNonDeferredWorkPackages();
}
#endif

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockWithRelTimeout_NoTimeoutExpiration)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    RWLockWriteLocker uut(lock, TimeSpan::ms(100));

    TestWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockWithRelTimeout_TimeoutAlreadyExpired)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    RWLockWriteLocker uut(lock, TimeSpan::ms(-100));

    TestWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, LockWithRelTimeout_TimeoutExpires)
{
  using namespace gpcc::time;

  RWLock lock;

  std::atomic<bool> ok(false);

  // this acquires a read-lock and will prevent acquisition of the write-lock
  std::unique_ptr<RWLockReadLocker> spReadLocker(new RWLockReadLocker(lock));

  // try to acquire write-lock from a different thread
  auto func1 = [&]()
  {
    try
    {
      RWLockWriteLocker uut(lock, TimeSpan::ms(10));

      ADD_FAILURE();
    }
    catch (gpcc::osal::TimeoutError const &)
    {
      // good
      ok = true;
    }
    catch (std::exception const &)
    {
      ADD_FAILURE();
    }
  };

  dwq.Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0, func1));
  gpcc::osal::Thread::Sleep_ms(20);

  // if no success until now, then release the read-lock to allow the other thread to continue
  // acquiring the write-lock
  if (!ok)
  {
    ADD_FAILURE();
    spReadLocker.reset();
  }

  dwq.FlushNonDeferredWorkPackages();
}
#endif

TEST_F(gpcc_osal_RWLockWriteLocker_TestsF, MoveConstruction)
{
  RWLock lock;

  {
    RWLockWriteLocker uut(lock);

    {
      RWLockWriteLocker uut2(std::move(uut));

      TestWriteLocked(lock);
    }

    TestNotWriteLocked(lock);
  }

  TestNotWriteLocked(lock);
}

TEST(gpcc_osal_RWLockWriteLocker_Tests, MoveConstructionNullptr)
{
  RWLockWriteLocker uut(nullptr);
  RWLockWriteLocker uut2(std::move(uut));
}

} // namespace osal
} // namespace gpcc_tests
