/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include "src/osal/os/linux_arm_tfc/internal/AdvancedUnmanagedMutexLocker.hpp"
#include "src/osal/os/linux_arm_tfc/internal/UnmanagedMutex.hpp"
#include "src/osal/os/linux_arm_tfc/internal/UnmanagedMutexLocker.hpp"
#include <gtest/gtest.h>
#include <memory>

namespace gpcc_tests {
namespace osal {
namespace internal {

using gpcc::osal::internal::UnmanagedMutex;
using gpcc::osal::internal::UnmanagedMutexLocker;
using gpcc::osal::internal::AdvancedUnmanagedMutexLocker;

using namespace testing;

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, CreateFromReference)
{
  UnmanagedMutex m;

  {
    AdvancedUnmanagedMutexLocker uut(m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, CreateFromPointer)
{
  UnmanagedMutex m;

  {
    AdvancedUnmanagedMutexLocker uut(&m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, CreatePassive)
{
  AdvancedUnmanagedMutexLocker uut(nullptr);
}

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, MoveCtor)
{
  UnmanagedMutex m;

  std::unique_ptr<AdvancedUnmanagedMutexLocker> spUUT1(new AdvancedUnmanagedMutexLocker(m));

  // check: m must be locked
  ASSERT_FALSE(m.TryLock());

  {
    // responsibility to unlock moves from pUUT1 to uut2
    AdvancedUnmanagedMutexLocker uut2(std::move(*spUUT1));

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

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, MoveCtorFromMutexLocker)
{
  UnmanagedMutex m;

  std::unique_ptr<UnmanagedMutexLocker> spUUT1(new UnmanagedMutexLocker(m));

  // check: m must be locked
  ASSERT_FALSE(m.TryLock());

  {
    // responsibility to unlock moves from pUUT1 to uut2
    AdvancedUnmanagedMutexLocker uut2(std::move(*spUUT1));

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

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, UnlockAndRelock)
{
  UnmanagedMutex m;

  {
    AdvancedUnmanagedMutexLocker uut(m);

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

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, UnlockAndRelease)
{
  UnmanagedMutex m;

  {
    AdvancedUnmanagedMutexLocker uut(m);

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

TEST(gpcc_osal_internal_AdvancedUnmanagedMutexLocker_Tests, LockUnlockWhenPassive)
{
  {
    AdvancedUnmanagedMutexLocker uut(nullptr);

    uut.Unlock();
    uut.Relock();
  }
}

TEST(gpcc_osal_internal_AdvancedMutexLocker_DeathTests, RelockWhenAlreadyLocked)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  UnmanagedMutex m;

  {
    AdvancedUnmanagedMutexLocker uut(m);

    EXPECT_DEATH(uut.Relock(), ".*UnmanagedMutex already locked.*");
  }

  // check: m must be unlocked
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_internal_AdvancedMutexLocker_DeathTests, UnlockWhenAlreadyUnlocked)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  UnmanagedMutex m;

  {
    AdvancedUnmanagedMutexLocker uut(m);
    uut.Unlock();

    EXPECT_DEATH(uut.Unlock(), ".*UnmanagedMutex already unlocked.*");
  }

  // check: m must be unlocked
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_ARM_TFC
