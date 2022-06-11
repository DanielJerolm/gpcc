/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/IThreadRegistry.hpp"
#include "gpcc/src/osal/Thread.hpp"
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
