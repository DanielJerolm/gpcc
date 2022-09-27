/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include "src/osal/os/linux_arm_tfc/internal/UnmanagedMutexLocker.hpp"
#include "src/osal/os/linux_arm_tfc/internal/UnmanagedMutex.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace osal {
namespace internal {

using gpcc::osal::internal::UnmanagedMutex;
using gpcc::osal::internal::UnmanagedMutexLocker;

using namespace testing;

TEST(gpcc_osal_internal_UnmanagedMutexLocker_Tests, CreateFromReference)
{
  UnmanagedMutex m;

  {
    UnmanagedMutexLocker uut(m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_internal_UnmanagedMutexLocker_Tests, CreateFromPointer)
{
  UnmanagedMutex m;

  {
    UnmanagedMutexLocker uut(&m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_internal_UnmanagedMutexLocker_Tests, CreatePassive)
{
  UnmanagedMutexLocker uut(nullptr);
}

TEST(gpcc_osal_internal_UnmanagedMutexLocker_Tests, MoveCtor)
{
  UnmanagedMutex m;

  std::unique_ptr<UnmanagedMutexLocker> spUUT1(new UnmanagedMutexLocker(m));

  // check: m must be locked
  ASSERT_FALSE(m.TryLock());

  {
    // responsibility to unlock moves from pUUT2 to uut2
    UnmanagedMutexLocker uut2(std::move(*spUUT1));

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

} // namespace internal
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_ARM_TFC
