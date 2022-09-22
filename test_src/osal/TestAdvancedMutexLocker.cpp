/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
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
