/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/IThreadRegistry.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/Thread.hpp>
#include "gtest/gtest.h"
#include <memory>
#include <cstdint>

namespace gpcc_tests {
namespace osal {

using namespace gpcc::osal;

using namespace testing;

TEST(gpcc_osal_ThreadRegistry_Tests, Lock)
{
  IThreadRegistry& uut = Thread::GetThreadRegistry();

  for (uint_fast8_t i = 0; i < 2; i++)
  {
    MutexLocker locker(uut.Lock());
  }
}

TEST(gpcc_osal_ThreadRegistry_Tests, LockViaAdvancedMutexLocker)
{
  IThreadRegistry& uut = Thread::GetThreadRegistry();

  for (uint_fast8_t i = 0; i < 2; i++)
  {
    AdvancedMutexLocker locker(uut.Lock());
    locker.Unlock();
  }
}

TEST(gpcc_osal_ThreadRegistry_Tests, NoThreads)
{
  IThreadRegistry& uut = Thread::GetThreadRegistry();
  MutexLocker locker(uut.Lock());

  ASSERT_EQ(0U, uut.GetNbOfThreads());
  ASSERT_TRUE(uut.ThreadListBegin() == uut.ThreadListEnd());
}

TEST(gpcc_osal_ThreadRegistry_Tests, EnumerateThreads)
{
  IThreadRegistry& uut = Thread::GetThreadRegistry();

  std::unique_ptr<Thread> spT1(new Thread("xy..."));
  std::unique_ptr<Thread> spT2(new Thread("ab..."));
  std::unique_ptr<Thread> spT3(new Thread("b..."));

  AdvancedMutexLocker locker(uut.Lock());

  // all 3 threads ------------------------------------------------------------
  ASSERT_EQ(3U, uut.GetNbOfThreads());
  uint_fast8_t i = 0;
  for (auto it = uut.ThreadListBegin(); it != uut.ThreadListEnd(); ++it, i++)
  {
    Thread const * pThread = *it;
    ASSERT_TRUE(pThread != nullptr);

    switch (i)
    {
      case 0: ASSERT_EQ(spT2.get(), pThread); break;
      case 1: ASSERT_EQ(spT3.get(), pThread); break;
      case 2: ASSERT_EQ(spT1.get(), pThread); break;
      default:
        FAIL();
        break;
    }
  }

  locker.Unlock();

  // threads spT1 and spT3 ----------------------------------------------------
  spT2.reset();

  locker.Relock();

  ASSERT_EQ(2U, uut.GetNbOfThreads());
  i = 0;
  for (auto it = uut.ThreadListBegin(); it != uut.ThreadListEnd(); ++it, i++)
  {
    Thread const * pThread = *it;
    ASSERT_TRUE(pThread != nullptr);

    switch (i)
    {
      case 0: ASSERT_EQ(spT3.get(), pThread); break;
      case 1: ASSERT_EQ(spT1.get(), pThread); break;
      default:
        FAIL();
        break;
    }
  }

  locker.Unlock();

  // thread spT1 only ---------------------------------------------------------
  spT3.reset();

  locker.Relock();

  ASSERT_EQ(1U, uut.GetNbOfThreads());
  i = 0;
  for (auto it = uut.ThreadListBegin(); it != uut.ThreadListEnd(); ++it, i++)
  {
    Thread const * pThread = *it;
    ASSERT_TRUE(pThread != nullptr);

    switch (i)
    {
      case 0: ASSERT_EQ(spT1.get(), pThread); break;
      default:
        FAIL();
        break;
    }
  }

  locker.Unlock();

  // no threads ---------------------------------------------------------------
  spT1.reset();

  locker.Relock();

  ASSERT_EQ(0U, uut.GetNbOfThreads());
  ASSERT_TRUE(uut.ThreadListBegin() == uut.ThreadListEnd());
}

} // namespace osal
} // namespace gpcc_tests
