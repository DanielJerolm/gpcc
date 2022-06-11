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

#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace osal {

using gpcc::osal::Mutex;
using gpcc::osal::MutexLocker;
using gpcc::osal::AdvancedMutexLocker;

using namespace testing;

TEST(gpcc_osal_AdvancedMutexLocker_Tests, CreateFromReference)
{
  Mutex m;

  {
    AdvancedMutexLocker uut(m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, CreateFromPointer)
{
  Mutex m;

  {
    AdvancedMutexLocker uut(&m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, CreatePassive)
{
  AdvancedMutexLocker uut(nullptr);
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, MoveCtor)
{
  Mutex m;

  std::unique_ptr<AdvancedMutexLocker> spUUT1(new AdvancedMutexLocker(m));

  // check: m must be locked
  ASSERT_FALSE(m.TryLock());

  {
    // responsibility to unlock moves from spUUT1 to uut2
    AdvancedMutexLocker uut2(std::move(*spUUT1));

    // check: m must still be locked
    ASSERT_FALSE(m.TryLock());

    // destroy spUUT1 (m should NOT be unlocked, because responsibility has moved to uut2
    spUUT1.reset();

    // check: m must still be locked
    ASSERT_FALSE(m.TryLock());
  }

  // scope left, uut2 has been released
  // check: m must be unlocked now
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, MoveCtorFromMutexLocker)
{
  Mutex m;

  std::unique_ptr<MutexLocker> spUUT1(new MutexLocker(m));

  // check: m must be locked
  ASSERT_FALSE(m.TryLock());

  {
    // responsibility to unlock moves from pUUT1 to uut2
    AdvancedMutexLocker uut2(std::move(*spUUT1));

    // check: m must still be locked
    ASSERT_FALSE(m.TryLock());

    // destroy spUUT1
    spUUT1.reset();

    // check: m must still be locked
    ASSERT_FALSE(m.TryLock());
  }

  // scope left, uut2 has been released
  // check: m must be unlocked now
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, UnlockAndRelock)
{
  Mutex m;

  {
    AdvancedMutexLocker uut(m);

    // check: m must be locked
    ASSERT_TRUE(uut.IsLocked());
    ASSERT_FALSE(m.TryLock());

    // unlock
    uut.Unlock();

    // check: m must be unlocked
    ASSERT_FALSE(uut.IsLocked());
    ASSERT_TRUE(m.TryLock());
    m.Unlock();

    // relock
    uut.Relock();

    // check: m must be locked
    ASSERT_TRUE(uut.IsLocked());
    ASSERT_FALSE(m.TryLock());
  }

  // check: m must be unlocked
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, UnlockAndRelease)
{
  Mutex m;

  {
    AdvancedMutexLocker uut(m);

    ASSERT_FALSE(m.TryLock());

    uut.Unlock();

    // check: m must be unlocked
    ASSERT_TRUE(m.TryLock());

    // m is locked when uut is released. uut must not attempt to unlock m.
  }

  // check: m must still be locked
  ASSERT_FALSE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_Tests, LockUnlockWhenPassive)
{
  {
    AdvancedMutexLocker uut(nullptr);

    uut.Unlock();
    uut.Relock();
  }
}

TEST(gpcc_osal_AdvancedMutexLocker_DeathTests, RelockWhenAlreadyLocked)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  Mutex m;

  {
    AdvancedMutexLocker uut(m);

    EXPECT_DEATH(uut.Relock(), ".*Mutex already locked.*");
  }

  // check: m must be unlocked
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_AdvancedMutexLocker_DeathTests, UnlockWhenAlreadyUnlocked)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  Mutex m;

  {
    AdvancedMutexLocker uut(m);
    uut.Unlock();

    EXPECT_DEATH(uut.Unlock(), ".*Mutex already unlocked.*");
  }

  // check: m must be unlocked
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

} // namespace osal
} // namespace gpcc_tests
