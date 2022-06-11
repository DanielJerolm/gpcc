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

#ifdef OS_LINUX_ARM_TFC

#include "gpcc/src/osal/os/linux_arm_tfc/internal/UnmanagedMutex.hpp"
#include "gpcc/src/osal/os/linux_arm_tfc/internal/UnmanagedMutexLocker.hpp"
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
