/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018, 2022 Daniel Jerolm

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

#include "gpcc/src/osal/exceptions.hpp"
#include "gpcc/src/osal/RWLock.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/osal/RWLockReadLocker.hpp"
#include "gpcc/src/osal/RWLockWriteLocker.hpp"
#include "gpcc/src/execution/async/DeferredWorkQueue.hpp"
#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"
#include <atomic>
#include <memory>

namespace gpcc_tests {
namespace osal       {

using namespace gpcc::osal;
using namespace testing;

// Test fixture for class RWLockReadLocker.
// This provides a work queue plus thread which allows unit tests to issue stimuli using different threads.
class gpcc_osal_RWLockReadLocker_TestsF: public Test
{
  public:
    gpcc_osal_RWLockReadLocker_TestsF(void);

  protected:
    gpcc::execution::async::DeferredWorkQueue dwq;
    gpcc::osal::Thread thread;

    void SetUp(void) override;
    void TearDown(void) override;

    void TestReadLocked(RWLock & lock);
    void TestNotReadLocked(RWLock & lock);

  private:
    void* ThreadEntry(void);
};

gpcc_osal_RWLockReadLocker_TestsF::gpcc_osal_RWLockReadLocker_TestsF(void)
: Test()
, dwq()
, thread("RWLockReadLocker_Tests")
{
}

void gpcc_osal_RWLockReadLocker_TestsF::SetUp(void)
{
  thread.Start(std::bind(&gpcc_osal_RWLockReadLocker_TestsF::ThreadEntry, this), osal::Thread::SchedPolicy::Other, 0, osal::Thread::GetDefaultStackSize());
  dwq.FlushNonDeferredWorkPackages();
}

void gpcc_osal_RWLockReadLocker_TestsF::TearDown(void)
{
  dwq.RequestTermination();
  thread.Join(nullptr);
}

void gpcc_osal_RWLockReadLocker_TestsF::TestReadLocked(RWLock & lock)
{
  // Adds a non-fatal failure to the test, if the given lock is not read-locked.
  // A different thread (dwq) than the calling one is used to perform the test.

  auto func1 = [&]()
  {
    // write-lock should not be acquirable
    if (lock.TryWriteLock())
    {
      ADD_FAILURE();
      lock.ReleaseWriteLock();
    }
  };

  auto func2 = [&]()
  {
    // another read-lock should be acquirable
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
  dwq.Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0, func2));
  dwq.FlushNonDeferredWorkPackages();
}

void gpcc_osal_RWLockReadLocker_TestsF::TestNotReadLocked(RWLock & lock)
{
  // Adds a non-fatal failure to the test, if the given lock is read-locked (or write-locked).
  // A different thread (dwq) than the calling one is used to perform the test.

  auto func1 = [&]()
  {
    if (!lock.TryWriteLock())
    {
      ADD_FAILURE();
    }
    else
    {
      lock.ReleaseWriteLock();
    }
  };

  dwq.Add(gpcc::execution::async::WorkPackage::CreateDynamic(this, 0, func1));
  dwq.FlushNonDeferredWorkPackages();
}

void* gpcc_osal_RWLockReadLocker_TestsF::ThreadEntry(void)
{
  dwq.Work();
  return nullptr;
}

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockByPtr)
{
  RWLock lock;

  {
    RWLockReadLocker uut(&lock);

    TestReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

TEST(gpcc_osal_RWLockReadLocker_Tests, LockNullptr)
{
  RWLockReadLocker uut(nullptr);
}

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockByRef)
{
  RWLock lock;

  {
    RWLockReadLocker uut(lock);

    TestReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockWithAbsTimeout_NoTimeoutExpiration)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    // TFC not required and no load dependency:
    // If the lock is free, then ReadLock() will even succeed if the timeout is already expired.
    auto const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(100);

    RWLockReadLocker uut(lock, absTimeout);

    TestReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockWithAbsTimeout_TimeoutAlreadyExpired)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    // TFC not required and no load dependency:
    // If the lock is free, then ReadLock() will even succeed if the timeout is already expired.
    auto const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) - TimeSpan::ms(100);

    RWLockReadLocker uut(lock, absTimeout);

    TestReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockWithAbsTimeout_TimeoutExpires)
{
  using namespace gpcc::time;

  RWLock lock;

  std::atomic<bool> ok(false);
  auto const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(10);

  // this acquires a write-lock and will prevent acquisition of the read-lock
  std::unique_ptr<RWLockWriteLocker> spWriteLocker(new RWLockWriteLocker(lock));

  // try to acquire read-lock from a different thread
  auto func1 = [&]()
  {
    try
    {
      RWLockReadLocker uut(lock, absTimeout);

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

  // if no success until now, then release the write-lock to allow the other thread to continue
  // acquiring the read-lock
  if (!ok)
  {
    ADD_FAILURE();
    spWriteLocker.reset();
  }

  dwq.FlushNonDeferredWorkPackages();
}
#endif

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockWithRelTimeout_NoTimeoutExpiration)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    // TFC not required and no load dependency:
    // If the lock is free, then ReadLock() will even succeed if the timeout is already expired.
    RWLockReadLocker uut(lock, TimeSpan::ms(100));

    TestReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockWithRelTimeout_TimeoutAlreadyExpired)
{
  using namespace gpcc::time;

  RWLock lock;

  {
    // TFC not required and no load dependency:
    // If the lock is free, then ReadLock() will even succeed if the timeout is already expired.
    RWLockReadLocker uut(lock, TimeSpan::ms(-100));

    TestReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLockReadLocker_TestsF, LockWithRelTimeout_TimeoutExpires)
{
  using namespace gpcc::time;

  RWLock lock;

  std::atomic<bool> ok(false);

  // this acquires a write-lock and will prevent acquisition of the read-lock
  std::unique_ptr<RWLockWriteLocker> spWriteLocker(new RWLockWriteLocker(lock));

  // try to acquire read-lock from a different thread
  auto func1 = [&]()
  {
    try
    {
      RWLockReadLocker uut(lock, TimeSpan::ms(10));

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

  // if no success until now, then release the write-lock to allow the other thread to continue
  // acquiring the read-lock
  if (!ok)
  {
    ADD_FAILURE();
    spWriteLocker.reset();
  }

  dwq.FlushNonDeferredWorkPackages();
}
#endif

TEST_F(gpcc_osal_RWLockReadLocker_TestsF, MoveConstruction)
{
  RWLock lock;

  {
    RWLockReadLocker uut(lock);

    {
      RWLockReadLocker uut2(std::move(uut));

      TestReadLocked(lock);
    }

    TestNotReadLocked(lock);
  }

  TestNotReadLocked(lock);
}

TEST(gpcc_osal_RWLockReadLocker_Tests, MoveConstructionNullptr)
{
  RWLockReadLocker uut(nullptr);
  RWLockReadLocker uut2(std::move(uut));
}

} // namespace osal
} // namespace gpcc_tests
