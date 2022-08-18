/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
