/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/RWLock.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "gtest/gtest.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstddef>

// Sleeptime in ms for the main thread to allow the TestHelper threads to run into the RWLock.
#define SLEEPTIME_MS                10

// Timeout in ms when waiting for acquiring an RWLOCK with timeout.
#define TIMEOUT_MS                  500

// Timeout in ms when waiting for an RWLOCK without any chance to acquire it.
#define NO_CHANCE_TIMEOUT_MS        20

// Timeout after which the TestHelper must be idle again.
#define TIMEOUT_TESTHELPER_JOB_MS   1000

namespace gpcc_tests {
namespace osal {

using namespace gpcc::osal;
using namespace gpcc::time;

using namespace testing;

// Helper for executing tests with multiple threads.
// The test fixture will provide several TestHelper instances.
// Each TestHelper instance encapsulates one thread that can invoke the UUT's methods.
// The TestHelper can be requested from the outside to stimulate the UUT. This is done
// by invoking Do(...) and passing a value from the Requests-enumeration as parameter.
// While the TestHelper is busy (e.g. because the UUT's method blocks), IsBusy() will return true.
// Some of the UUT's methods have a boolean return value. After the TestHelper is not busy any
// more, the value can be retrieved via GetUutRetVal().
class TestHelper final
{
  public:
    // Requests that can be passed to Do(...).
    enum class Requests
    {
      none,
      tryWriteLock,
      writeLock,
      writeLockTimeout,
      writeLockTimeoutNoChance,
      releaseWriteLock,
      tryReadLock,
      readLock,
      readLockTimeout,
      readLockTimeoutNoChance,
      releaseReadLock,
      terminate
    };

    // Internal states of the TestHelper.
    // Used to discover misuse via Do(...), e.g. attempt to release a write-lock that has never been acquired.
    // Also used to unlock the UUT if Requests::terminate is requested.
    enum class States
    {
      noLock,
      writeLock,
      readLock
    };

    TestHelper(void)
    : thread("GPCC unit test helper thread")
    , state(States::noLock)
    , mutex()
    , pUUT(nullptr)
    , request(Requests::none)
    , conVarRequest()
    , busy(false)
    , conVarBusy()
    , uut_retVal(false)
    {
      thread.Start(std::bind(&TestHelper::ThreadEntry, this), Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
    }
    ~TestHelper(void)
    {
      Do(Requests::terminate);
      thread.Join();
    }
    void SetUUT(RWLock* const _pUUT)
    {
      MutexLocker mutexLocker(mutex);
      if (pUUT != nullptr)
        throw std::logic_error("TestHelper::SetUUT: UUT already set");

      pUUT = _pUUT;
    }

    void Do(Requests const _request)
    {
      MutexLocker mutexLocker(mutex);

      if (_request != Requests::terminate)
      {
        if (pUUT == nullptr)
          throw std::runtime_error("TestHelper::Do: No UUT set via SetUUT(...)");

        if (busy)
          throw std::runtime_error("TestHelper::Do: TestHelper is still busy");

        if (request != Requests::none)
          throw std::logic_error("TestHelper::Do: Not busy, but request != requests::none");
      }

      request = _request;
      conVarRequest.Signal();
    }
    bool IsBusy(void)
    {
      MutexLocker mutexLocker(mutex);
      return busy;
    }
    void WaitUntilNotBusy(void)
    {
      MutexLocker mutexLocker(mutex);
      TimePoint const tp = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(TIMEOUT_TESTHELPER_JOB_MS);
      while ((busy) || (request != Requests::none))
      {
        if (conVarBusy.TimeLimitedWait(mutex, tp))
          throw std::runtime_error("TestHelper::WaitUntilNotBusy: Seems as if the test failed");
      }
    }

    bool GetUutRetVal(void)
    {
      MutexLocker mutexLocker(mutex);

      if (busy)
        throw std::runtime_error("TestHelper::GetUutRetVal: TestHelper is busy");

      return uut_retVal;
    }

    void* ThreadEntry(void)
    {
      try
      {
        AdvancedMutexLocker mutexLocker(mutex);

        while (request != Requests::terminate)
        {
          while (request == Requests::none)
            conVarRequest.Wait(mutex);

          busy = true;
          uut_retVal = false;
          mutexLocker.Unlock();

          switch (request)
          {
            case Requests::none:
            {
              break;
            }
            case Requests::tryWriteLock:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (tryWriteLock)");

              uut_retVal = pUUT->TryWriteLock();

              if (uut_retVal)
                state = States::writeLock;
              break;
            }
            case Requests::writeLock:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (writeLock)");

              pUUT->WriteLock();

              state = States::writeLock;
              break;
            }
            case Requests::writeLockTimeout:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (writeLockTimeout)");

              TimePoint const tp = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(TIMEOUT_MS);
              uut_retVal = pUUT->WriteLock(tp);

              if (uut_retVal)
                state = States::writeLock;
              break;
            }
            case Requests::writeLockTimeoutNoChance:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (writeLockTimeoutNoChance)");

              TimePoint const tp = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(NO_CHANCE_TIMEOUT_MS);
              uut_retVal = pUUT->WriteLock(tp);

              if (uut_retVal)
                state = States::writeLock;
              break;
            }
            case Requests::releaseWriteLock:
            {
              if (state != States::writeLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (releaseWriteLock)");

              pUUT->ReleaseWriteLock();

              state = States::noLock;
              break;
            }
            case Requests::tryReadLock:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (tryReadLock)");

              uut_retVal = pUUT->TryReadLock();

              if (uut_retVal)
                state = States::readLock;
              break;
            }
            case Requests::readLock:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (readLock)");

              pUUT->ReadLock();

              state = States::readLock;
              break;
            }
            case Requests::readLockTimeout:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (readLockTimeout)");

              TimePoint const tp = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(TIMEOUT_MS);
              uut_retVal = pUUT->ReadLock(tp);

              if (uut_retVal)
                state = States::readLock;
              break;
            }
            case Requests::readLockTimeoutNoChance:
            {
              if (state != States::noLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (readLockTimeoutNoChance)");

              TimePoint const tp = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(NO_CHANCE_TIMEOUT_MS);
              uut_retVal = pUUT->ReadLock(tp);

              if (uut_retVal)
                state = States::readLock;
              break;
            }
            case Requests::releaseReadLock:
            {
              if (state != States::readLock)
                throw std::runtime_error("TestHelper::ThreadEntry: Wrong state (releaseReadLock)");

              pUUT->ReleaseReadLock();

              state = States::noLock;
              break;
            }
            case Requests::terminate:
            {
              break;
            }
            default:
            {
              throw std::runtime_error("TestHelper::ThreadEntry: unknown request");
              break;
            }
          } // switch (request)

          mutexLocker.Relock();

          if (request != Requests::terminate)
            request = Requests::none;
          busy = false;
          conVarBusy.Signal();
        } // while (request != Requests::terminate)
        request = Requests::none;

        if (state == States::writeLock)
          pUUT->ReleaseWriteLock();
        else if (state == States::readLock)
          pUUT->ReleaseReadLock();
        state = States::noLock;
      } // try
      catch (std::exception const & e)
      {
        gpcc::osal::Panic("TestHelper::Threadentry (TestRWLock.cpp): ", e);
      } // try... catch...

      return nullptr;
    }

  private:
    Thread thread;

    States state;

    Mutex mutex;

    RWLock* pUUT;                     // mutex required

    Requests request;                 // mutex required
    ConditionVariable conVarRequest;  // mutex required

    bool busy;                        // mutex required
    ConditionVariable conVarBusy;     // mutex required

    bool uut_retVal;                  // mutex required
};

/// Test fixture for gpcc::osal::RWLock related tests.
class gpcc_osal_RWLock_TestsF: public Test
{
  public:
    static size_t const nbOfTestHelpers = 4U;

  protected:
    RWLock uut;
    TestHelper testHelpers[nbOfTestHelpers];

    gpcc_osal_RWLock_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;
};

gpcc_osal_RWLock_TestsF::gpcc_osal_RWLock_TestsF(void)
: Test()
, uut()
, testHelpers()
{
}

void gpcc_osal_RWLock_TestsF::SetUp(void)
{
  for (size_t i = 0; i < nbOfTestHelpers; i++)
    testHelpers[i].SetUUT(&uut);
}
void gpcc_osal_RWLock_TestsF::TearDown(void)
{
}

TEST(gpcc_osal_RWLock_Tests, BasicLockUnlock)
{
  RWLock uut;

  ASSERT_TRUE(uut.TryWriteLock());
  uut.ReleaseWriteLock();

  uut.WriteLock();
  uut.ReleaseWriteLock();

  // TFC not required and no load dependency:
  // If the lock is free, then WriteLock() will even succeed if the timeout is already expired.
  TimePoint timeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(TIMEOUT_MS);
  ASSERT_TRUE(uut.WriteLock(timeout));
  uut.ReleaseWriteLock();

  ASSERT_TRUE(uut.TryReadLock());
  uut.ReleaseReadLock();

  uut.ReadLock();
  uut.ReleaseReadLock();

  // TFC not required and no load dependency:
  // If the lock is free, then ReadLock() will even succeed if the timeout is already expired.
  timeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(TIMEOUT_MS);
  ASSERT_TRUE(uut.ReadLock(timeout));
  uut.ReleaseReadLock();
}

TEST(gpcc_osal_RWLock_Tests, BadRelease)
{
  RWLock uut;

  ASSERT_THROW(uut.ReleaseWriteLock(), std::logic_error);
  ASSERT_THROW(uut.ReleaseReadLock(), std::logic_error);

  uut.WriteLock();
  ASSERT_THROW(uut.ReleaseReadLock(), std::logic_error);
  uut.ReleaseWriteLock();

  uut.ReadLock();
  ASSERT_THROW(uut.ReleaseWriteLock(), std::logic_error);
  uut.ReleaseReadLock();
}

TEST(GPCC_OSAL_RWLock_DeathTests, LockedUponDestruction)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<RWLock> spUUT(new RWLock());

  spUUT->WriteLock();
  EXPECT_DEATH(spUUT.reset(), ".*RWLock is locked.*");
  spUUT->ReleaseWriteLock();

  spUUT->ReadLock();
  EXPECT_DEATH(spUUT.reset(), ".*RWLock is locked.*");
  spUUT->ReleaseReadLock();
}

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLock_TestsF, MultipleReadLocks)
{
  for (size_t i = 0; i < nbOfTestHelpers; i++)
  {
    testHelpers[i].Do(TestHelper::Requests::readLock);
    testHelpers[i].WaitUntilNotBusy();
  }

  for (size_t i = 0; i < nbOfTestHelpers; i++)
  {
    testHelpers[i].Do(TestHelper::Requests::releaseReadLock);
    testHelpers[i].WaitUntilNotBusy();
  }
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLock_TestsF, OneWriteLockOnly)
{
  ASSERT_TRUE(nbOfTestHelpers >= 3);

  TestHelper* const pWriter1 = &testHelpers[0];
  TestHelper* const pWriter2 = &testHelpers[1];
  TestHelper* const pReader  = &testHelpers[2];

  pWriter1->Do(TestHelper::Requests::writeLock);
  pWriter1->WaitUntilNotBusy();

  pWriter2->Do(TestHelper::Requests::tryWriteLock);
  pWriter2->WaitUntilNotBusy();
  ASSERT_FALSE(pWriter2->GetUutRetVal());

  pWriter2->Do(TestHelper::Requests::writeLockTimeoutNoChance);
  pWriter2->WaitUntilNotBusy();
  ASSERT_FALSE(pWriter2->GetUutRetVal());

  pReader->Do(TestHelper::Requests::tryReadLock);
  pReader->WaitUntilNotBusy();
  ASSERT_FALSE(pReader->GetUutRetVal());

  pReader->Do(TestHelper::Requests::readLockTimeoutNoChance);
  pReader->WaitUntilNotBusy();
  ASSERT_FALSE(pReader->GetUutRetVal());

  pWriter1->Do(TestHelper::Requests::releaseWriteLock);
  pWriter1->WaitUntilNotBusy();
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLock_TestsF, NewReadersWaitTillWritersAreServed)
{
  ASSERT_TRUE(nbOfTestHelpers >= 3U);

  TestHelper* const pReader     = &testHelpers[0];
  TestHelper* const pWriter     = &testHelpers[1];
  TestHelper* const pNewReader  = &testHelpers[2];

  // reader locks
  pReader->Do(TestHelper::Requests::readLock);
  pReader->WaitUntilNotBusy();

  // writer locks (will be blocked)
  pWriter->Do(TestHelper::Requests::writeLock);
  // allow pWriter to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pWriter->IsBusy());

  // a new reader locks (will be blocked because there is a blocked writer)
  pNewReader->Do(TestHelper::Requests::readLock);
  // allow pNewReader to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pNewReader->IsBusy());

  // Reader releases it's lock. The blocked writer must acquire it, the new reader must wait.
  pReader->Do(TestHelper::Requests::releaseReadLock);
  pReader->WaitUntilNotBusy();

  // allow pWriter to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_FALSE(pWriter->IsBusy());
  ASSERT_TRUE(pNewReader->IsBusy());

  // Writer releases it's lock. The blocked new reader must acquire it.
  pWriter->Do(TestHelper::Requests::releaseWriteLock);
  pWriter->WaitUntilNotBusy();

  // allow pNewReader to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_FALSE(pNewReader->IsBusy());

  // finally then new reader releases the lock
  pNewReader->Do(TestHelper::Requests::releaseReadLock);
  pNewReader->WaitUntilNotBusy();
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLock_TestsF, NewReadersWaitTill2WritersAreServed)
{
  ASSERT_TRUE(nbOfTestHelpers >= 4U);

  TestHelper* const pReader     = &testHelpers[0];
  TestHelper* const pWriter1    = &testHelpers[1];
  TestHelper* const pWriter2    = &testHelpers[2];
  TestHelper* const pNewReader  = &testHelpers[3];

  // reader locks
  pReader->Do(TestHelper::Requests::readLock);
  pReader->WaitUntilNotBusy();

  // writer #1 locks (will be blocked)
  pWriter1->Do(TestHelper::Requests::writeLock);
  // allow pWriter1 to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pWriter1->IsBusy());

  // a new reader locks (will be blocked because there is a blocked writer)
  pNewReader->Do(TestHelper::Requests::readLock);
  // allow pNewReader to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pNewReader->IsBusy());

  // writer #2 locks (will be blocked)
  pWriter2->Do(TestHelper::Requests::writeLock);
  // allow pWriter2 to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pWriter2->IsBusy());

  // Reader releases it's lock. One of the blocked writers must acquire it, the new reader must wait.
  pReader->Do(TestHelper::Requests::releaseReadLock);
  pReader->WaitUntilNotBusy();

  // allow one of the writers to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_TRUE(((pWriter1->IsBusy()) && (!pWriter2->IsBusy())) || ((!pWriter1->IsBusy()) && (pWriter2->IsBusy())));
  ASSERT_TRUE(pNewReader->IsBusy());

  {
    TestHelper* pWriter;         // the writer who acquired the lock
    TestHelper* pOtherWriter;    // the other write that is still blocked

    // find out who is who
    if (!pWriter1->IsBusy())
    {
      pWriter      = pWriter1;
      pOtherWriter = pWriter2;
    }
    else
    {
      pWriter      = pWriter2;
      pOtherWriter = pWriter1;
    }

    // The writer releases it's lock. The other writer must acquire it while the new reader keeps blocked.
    pWriter->Do(TestHelper::Requests::releaseWriteLock);
    pWriter->WaitUntilNotBusy();

    // allow the other writer to wake up and acquire the lock
    Thread::Sleep_ms(SLEEPTIME_MS);

    ASSERT_FALSE(pOtherWriter->IsBusy());
    ASSERT_TRUE(pNewReader->IsBusy());

    // The other writer releases the lock. The new reader must acquire it.
    pOtherWriter->Do(TestHelper::Requests::releaseWriteLock);
    pOtherWriter->WaitUntilNotBusy();

    // allow the new reader to wake up and acquire the lock
    Thread::Sleep_ms(SLEEPTIME_MS);

    ASSERT_FALSE(pNewReader->IsBusy());
  }

  // Finally the new reader releases the lock
  pNewReader->Do(TestHelper::Requests::releaseReadLock);
  pNewReader->WaitUntilNotBusy();
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLock_TestsF, NewWritersHavePriorityAboveBlockedReaders)
{
  ASSERT_TRUE(nbOfTestHelpers >= 4U);

  TestHelper* const pReader     = &testHelpers[0];
  TestHelper* const pWriter     = &testHelpers[1];
  TestHelper* const pNewReader  = &testHelpers[2];
  TestHelper* const pNewWriter  = &testHelpers[3];

  // reader locks
  pReader->Do(TestHelper::Requests::readLock);
  pReader->WaitUntilNotBusy();

  // writer locks (will be blocked)
  pWriter->Do(TestHelper::Requests::writeLock);
  // allow pWriter to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pWriter->IsBusy());

  // a new reader locks (will be blocked because there is a blocked writer)
  pNewReader->Do(TestHelper::Requests::readLock);
  // allow pNewReader to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pNewReader->IsBusy());

  // Reader releases it's lock. The blocked writer must acquire it, the new reader must wait.
  pReader->Do(TestHelper::Requests::releaseReadLock);
  pReader->WaitUntilNotBusy();

  // allow pWriter to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_FALSE(pWriter->IsBusy());
  ASSERT_TRUE(pNewReader->IsBusy());

  // a new writer locks (will be blocked, because there can be only one writer who holds the lock)
  pNewWriter->Do(TestHelper::Requests::writeLock);
  // allow pNewWriter to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pNewWriter->IsBusy());

  // Writer releases it's lock. The blocked new writer must acquire it.
  pWriter->Do(TestHelper::Requests::releaseWriteLock);
  pWriter->WaitUntilNotBusy();

  // allow pNewWriter to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_FALSE(pNewWriter->IsBusy());
  ASSERT_TRUE(pNewReader->IsBusy());

  // The new writer released it's lock. The new reader must acquire it now.
  pNewWriter->Do(TestHelper::Requests::releaseWriteLock);
  pNewWriter->WaitUntilNotBusy();

  // allow pNewReader to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);

  ASSERT_FALSE(pNewReader->IsBusy());

  // finally the new reader releases the lock
  pNewReader->Do(TestHelper::Requests::releaseReadLock);
  pNewReader->WaitUntilNotBusy();
}
#endif

#if (!defined(SKIP_TFC_BASED_TESTS)) || (!defined(SKIP_LOAD_DEPENDENT_TESTS))
TEST_F(gpcc_osal_RWLock_TestsF, WritersAreBlockedTillAllReadersHaveReleased)
{
  ASSERT_TRUE(nbOfTestHelpers >= 3U);

  TestHelper* const pReader1    = &testHelpers[0];
  TestHelper* const pReader2    = &testHelpers[1];
  TestHelper* const pWriter     = &testHelpers[2];

  // reader 1 locks
  pReader1->Do(TestHelper::Requests::readLock);
  pReader1->WaitUntilNotBusy();

  // reader 2 locks
  pReader2->Do(TestHelper::Requests::readLock);
  pReader2->WaitUntilNotBusy();

  // writer locks, but is blocked
  pWriter->Do(TestHelper::Requests::writeLock);
  // allow pWriter to run into the RWLock and block
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pWriter->IsBusy());

  // reader 1 unlocks, writer is still blocked
  pReader1->Do(TestHelper::Requests::releaseReadLock);
  pReader1->WaitUntilNotBusy();

  // allow pWriter in case of an error to wake up
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_TRUE(pWriter->IsBusy());

  // reader 2 unlocks, writer wakes up and acquires lock
  pReader2->Do(TestHelper::Requests::releaseReadLock);
  pReader2->WaitUntilNotBusy();

  // allow pWriter to wake up and acquire the lock
  Thread::Sleep_ms(SLEEPTIME_MS);
  ASSERT_FALSE(pWriter->IsBusy());

  // finally the writer releases the lock
  pWriter->Do(TestHelper::Requests::releaseWriteLock);
  pWriter->WaitUntilNotBusy();
}
#endif

} // namespace osal
} // namespace gpcc_tests
