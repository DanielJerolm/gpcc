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

#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gtest/gtest.h"
#include <memory>

namespace gpcc_tests {
namespace osal {

using gpcc::osal::Mutex;
using gpcc::osal::MutexLocker;

using namespace testing;

static MutexLocker Func1(Mutex& m)
{
  return MutexLocker(m);
}
static MutexLocker Func2(Mutex& m)
{
  MutexLocker ml(m);
  return ml;
}

TEST(gpcc_osal_MutexLocker_Tests, CreateFromReference)
{
  Mutex m;

  {
    MutexLocker uut(m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_MutexLocker_Tests, CreateFromPointer)
{
  Mutex m;

  {
    MutexLocker uut(&m);

    ASSERT_FALSE(m.TryLock());
  }

  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_MutexLocker_Tests, CreatePassive)
{
  MutexLocker uut(nullptr);
}

TEST(gpcc_osal_MutexLocker_Tests, MoveCtor)
{
  Mutex m;

  std::unique_ptr<MutexLocker> spUUT1(new MutexLocker(m));

  // check: m must be locked
  ASSERT_FALSE(m.TryLock());

  {
    // responsibility to unlock moves from pUUT2 to uut2
    MutexLocker uut2(std::move(*spUUT1));

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

TEST(gpcc_osal_MutexLocker_Tests, ReturnValue_Variant1)
{
  Mutex m;

  {
    MutexLocker ml = Func1(m);

    // check: m must be locked
    ASSERT_FALSE(m.TryLock());
  }

  // check: m must be unlocked now
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

TEST(gpcc_osal_MutexLocker_Tests, ReturnValue_Variant2)
{
  Mutex m;

  {
    MutexLocker ml = Func2(m);

    // check: m must be locked
    ASSERT_FALSE(m.TryLock());
  }

  // check: m must be unlocked now
  ASSERT_TRUE(m.TryLock());
  m.Unlock();
}

} // namespace osal
} // namespace gpcc_tests
