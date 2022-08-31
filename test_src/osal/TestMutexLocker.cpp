/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
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
