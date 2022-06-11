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

#include "gtest/gtest.h"
#include "gpcc/src/ResourceManagement/Objects/internal/NamedRWLockEntry.hpp"
#include <string>

namespace gpcc_tests
{
namespace ResourceManagement
{
namespace Objects
{
namespace internal
{

using namespace testing;
using gpcc::ResourceManagement::Objects::internal::NamedRWLockEntry;

TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, CreateUnlocked)
{
  uint32_t dummy = 0x12345678U;
  std::string name("Test");

  NamedRWLockEntry uut(reinterpret_cast<NamedRWLockEntry*>(&dummy), name);

  // check public attributes
  ASSERT_TRUE(reinterpret_cast<uint32_t*>(uut.pNext) == &dummy);
  ASSERT_TRUE(name == uut.name);

  // check that uut is unlocked
  ASSERT_EQ(0, uut.GetNbOfReadLocks());
  ASSERT_FALSE(uut.IsWriteLocked());
  ASSERT_FALSE(uut.IsLocked());
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, CreateWriteLocked)
{
  uint32_t dummy = 0x12345678U;
  std::string name("Test");

  NamedRWLockEntry uut(reinterpret_cast<NamedRWLockEntry*>(&dummy), name, true);

  // check public attributes
  ASSERT_TRUE(reinterpret_cast<uint32_t*>(uut.pNext) == &dummy);
  ASSERT_TRUE(name == uut.name);

  // check that uut is write-locked
  ASSERT_EQ(0, uut.GetNbOfReadLocks());
  ASSERT_TRUE(uut.IsWriteLocked());
  ASSERT_TRUE(uut.IsLocked());

  ASSERT_NO_THROW(uut.ReleaseWriteLock());
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, CreateReadLocked)
{
  uint32_t dummy = 0x12345678U;
  std::string name("Test");

  NamedRWLockEntry uut(reinterpret_cast<NamedRWLockEntry*>(&dummy), name, false);

  // check public attributes
  ASSERT_TRUE(reinterpret_cast<uint32_t*>(uut.pNext) == &dummy);
  ASSERT_TRUE(name == uut.name);

  // check that uut is unlocked
  ASSERT_EQ(1, uut.GetNbOfReadLocks());
  ASSERT_FALSE(uut.IsWriteLocked());
  ASSERT_TRUE(uut.IsLocked());

  ASSERT_NO_THROW(uut.ReleaseReadLock());
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, ReadLock)
{
  NamedRWLockEntry uut(nullptr, "Test");

  /* check */ ASSERT_EQ(0, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_FALSE(uut.IsLocked());

  ASSERT_TRUE(uut.GetReadLock());

  /* check */ ASSERT_EQ(1, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_TRUE(uut.IsLocked());

  ASSERT_TRUE(uut.GetReadLock());

  /* check */ ASSERT_EQ(2, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_TRUE(uut.IsLocked());

  uut.ReleaseReadLock();

  /* check */ ASSERT_EQ(1, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_TRUE(uut.IsLocked());

  uut.ReleaseReadLock();

  /* check */ ASSERT_EQ(0, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_FALSE(uut.IsLocked());
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, WriteLock)
{
  NamedRWLockEntry uut(nullptr, "Test");

  /* check */ ASSERT_EQ(0, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_FALSE(uut.IsLocked());

  ASSERT_TRUE(uut.GetWriteLock());

  /* check */ ASSERT_EQ(0, uut.GetNbOfReadLocks());
  /* check */ ASSERT_TRUE(uut.IsWriteLocked());
  /* check */ ASSERT_TRUE(uut.IsLocked());

  uut.ReleaseWriteLock();

  /* check */ ASSERT_EQ(0, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_FALSE(uut.IsLocked());
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, DenyDoubleWriteLock)
{
  NamedRWLockEntry uut(nullptr, "Test");

  ASSERT_TRUE(uut.GetWriteLock());
  ASSERT_FALSE(uut.GetWriteLock());

  uut.ReleaseWriteLock();

  /* check */ ASSERT_EQ(0, uut.GetNbOfReadLocks());
  /* check */ ASSERT_FALSE(uut.IsWriteLocked());
  /* check */ ASSERT_FALSE(uut.IsLocked());
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, DenyUnlockWhenNotLocked)
{
  NamedRWLockEntry uut(nullptr, "Test");

  /* check */ ASSERT_THROW(uut.ReleaseReadLock(), std::logic_error);
  /* check */ ASSERT_THROW(uut.ReleaseWriteLock(), std::logic_error);

  ASSERT_TRUE(uut.GetReadLock());

  /* check */ ASSERT_THROW(uut.ReleaseWriteLock(), std::logic_error);

  uut.ReleaseReadLock();

  ASSERT_TRUE(uut.GetWriteLock());

  /* check */ ASSERT_THROW(uut.ReleaseReadLock(), std::logic_error);

  uut.ReleaseWriteLock();
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, DenyReadLockWhileWriteLocked)
{
  NamedRWLockEntry uut(nullptr, "Test");

  ASSERT_TRUE(uut.GetWriteLock());
  ASSERT_FALSE(uut.GetReadLock());

  uut.ReleaseWriteLock();
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_Tests, DenyWriteLockWhileReadLocked)
{
  NamedRWLockEntry uut(nullptr, "Test");

  ASSERT_TRUE(uut.GetReadLock());
  ASSERT_FALSE(uut.GetWriteLock());

  uut.ReleaseReadLock();
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_DeathTests, ReadLockNotReleased)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // this test checks if process dies if uut is released with an active read-lock

  NamedRWLockEntry* const pUUT = new NamedRWLockEntry(nullptr, "Test", false);
  EXPECT_DEATH(delete pUUT, ".*gpcc/src/ResourceManagement/Objects/internal/NamedRWLockEntry.cpp.*");
  ASSERT_NO_THROW(pUUT->ReleaseReadLock());
  delete pUUT;
}
TEST(GPCC_ResourceManagement_Objects_internal_NamedRWLockEntry_DeathTests, WriteLockNotReleased)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // this test checks if process dies if uut is released with an active write-lock

  NamedRWLockEntry* const pUUT = new NamedRWLockEntry(nullptr, "Test", true);
  EXPECT_DEATH(delete pUUT, ".*gpcc/src/ResourceManagement/Objects/internal/NamedRWLockEntry.cpp.*");
  ASSERT_NO_THROW(pUUT->ReleaseWriteLock());
  delete pUUT;
}

} // namespace internal
} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc_tests
