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
#include "gpcc/src/ResourceManagement/Objects/LargeDynamicNamedRWLock.hpp"
#include <string>
#include <stdexcept>

namespace gpcc_tests
{
namespace ResourceManagement
{
namespace Objects
{

using namespace testing;
using gpcc::ResourceManagement::Objects::LargeDynamicNamedRWLock;

TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, CreateAndRelease)
{
  LargeDynamicNamedRWLock uut;
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, WriteLock_SameResource)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  /* check */ ASSERT_TRUE(uut.TestWriteLock(res));

  ASSERT_TRUE(uut.GetWriteLock(res));

  /* check */ ASSERT_FALSE(uut.TestWriteLock(res));
  /* check */ ASSERT_FALSE(uut.GetWriteLock(res));

  uut.ReleaseWriteLock(res);

  /* check */ ASSERT_TRUE(uut.TestWriteLock(res));
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, ReadLock_SameResource)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  /* check */ ASSERT_TRUE(uut.TestReadLock(res));

  ASSERT_TRUE(uut.GetReadLock(res));

  /* check */ ASSERT_TRUE(uut.TestReadLock(res));

  ASSERT_TRUE(uut.GetReadLock(res));

  /* check */ ASSERT_TRUE(uut.TestReadLock(res));

  uut.ReleaseReadLock(res);

  /* check */ ASSERT_TRUE(uut.TestReadLock(res));

  uut.ReleaseReadLock(res);

  /* check */ ASSERT_TRUE(uut.TestReadLock(res));
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyReadLockWhileWriteLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_TRUE(uut.GetWriteLock(res));

  /* check */ ASSERT_FALSE(uut.TestReadLock(res));
  /* check */ ASSERT_FALSE(uut.GetReadLock(res));

  uut.ReleaseWriteLock(res);

  /* check */ ASSERT_TRUE(uut.TestReadLock(res));
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyWriteLockWhileReadLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_TRUE(uut.GetReadLock(res));

  /* check */ ASSERT_FALSE(uut.TestWriteLock(res));
  /* check */ ASSERT_FALSE(uut.GetWriteLock(res));

  uut.ReleaseReadLock(res);

  /* check */ ASSERT_TRUE(uut.TestWriteLock(res));
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyWriteLockWhileWriteLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_TRUE(uut.GetWriteLock(res));

  /* check */ ASSERT_FALSE(uut.TestWriteLock(res));
  /* check */ ASSERT_FALSE(uut.GetWriteLock(res));

  uut.ReleaseWriteLock(res);

  /* check */ ASSERT_TRUE(uut.TestWriteLock(res));
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyWriteUnlockWhileNotLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_THROW(uut.ReleaseWriteLock(res), std::logic_error);
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyReadUnlockWhileNotLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_THROW(uut.ReleaseReadLock(res), std::logic_error);
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyWriteUnlockWhileReadLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_TRUE(uut.GetReadLock(res));
  ASSERT_THROW(uut.ReleaseWriteLock(res), std::logic_error);
  uut.ReleaseReadLock(res);
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, DenyReadUnlockWhileWriteLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  ASSERT_TRUE(uut.GetWriteLock(res));
  ASSERT_THROW(uut.ReleaseReadLock(res), std::logic_error);
  uut.ReleaseWriteLock(res);
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, IsLocked)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  /* check */ ASSERT_FALSE(uut.IsLocked(res));

  ASSERT_TRUE(uut.GetWriteLock(res));

  /* check */ ASSERT_TRUE(uut.IsLocked(res));

  uut.ReleaseWriteLock(res);

  /* check */ ASSERT_FALSE(uut.IsLocked(res));

  ASSERT_TRUE(uut.GetReadLock(res));

  /* check */ ASSERT_TRUE(uut.IsLocked(res));

  uut.ReleaseReadLock(res);

  /* check */ ASSERT_FALSE(uut.IsLocked(res));
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, AnyLocks)
{
  std::string res("Resource A");

  LargeDynamicNamedRWLock uut;

  /* check */ ASSERT_FALSE(uut.AnyLocks());

  ASSERT_TRUE(uut.GetWriteLock(res));

  /* check */ ASSERT_TRUE(uut.AnyLocks());

  uut.ReleaseWriteLock(res);

  /* check */ ASSERT_FALSE(uut.AnyLocks());

  ASSERT_TRUE(uut.GetReadLock(res));

  /* check */ ASSERT_TRUE(uut.AnyLocks());

  uut.ReleaseReadLock(res);

  /* check */ ASSERT_FALSE(uut.AnyLocks());
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_Tests, MultipleResources)
{
  std::string resA("Resource A");
  std::string resB("Resource B");

  LargeDynamicNamedRWLock uut;

  ASSERT_TRUE(uut.GetWriteLock(resA));
  ASSERT_TRUE(uut.GetReadLock(resB));

  /* check */ ASSERT_FALSE(uut.GetWriteLock(resA));
  /* check */ ASSERT_FALSE(uut.GetWriteLock(resB));
  /* check */ ASSERT_FALSE(uut.GetReadLock(resA));
  /* check */ ASSERT_TRUE(uut.GetReadLock(resB));

  uut.ReleaseReadLock(resB);
  uut.ReleaseReadLock(resB);
  ASSERT_TRUE(uut.GetWriteLock(resB));

  /* check */ ASSERT_FALSE(uut.GetWriteLock(resA));
  /* check */ ASSERT_FALSE(uut.GetWriteLock(resB));
  /* check */ ASSERT_FALSE(uut.GetReadLock(resA));
  /* check */ ASSERT_FALSE(uut.GetReadLock(resB));

  uut.ReleaseWriteLock(resA);

  /* check */ ASSERT_FALSE(uut.IsLocked(resA));
  /* check */ ASSERT_TRUE(uut.IsLocked(resB));
  /* check */ ASSERT_TRUE(uut.AnyLocks());

  ASSERT_TRUE(uut.GetReadLock(resA));

  /* check */ ASSERT_FALSE(uut.GetWriteLock(resA));
  /* check */ ASSERT_FALSE(uut.GetWriteLock(resB));
  /* check */ ASSERT_TRUE(uut.GetReadLock(resA));
  /* check */ ASSERT_FALSE(uut.GetReadLock(resB));

  uut.ReleaseReadLock(resA);
  uut.ReleaseReadLock(resA);
  uut.ReleaseWriteLock(resB);

  /* check */ ASSERT_FALSE(uut.IsLocked(resA));
  /* check */ ASSERT_FALSE(uut.IsLocked(resB));
  /* check */ ASSERT_FALSE(uut.AnyLocks());
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_DeathTests, ReleaseButWriteLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::string res("Resource A");

  LargeDynamicNamedRWLock* pUUT = new LargeDynamicNamedRWLock();

  ASSERT_TRUE(pUUT->GetWriteLock(res));

  EXPECT_DEATH(delete pUUT, ".*gpcc/src/ResourceManagement/Objects/LargeDynamicNamedRWLock.cpp.*");

  pUUT->ReleaseWriteLock(res);
  delete pUUT;
}
TEST(GPCC_ResourceManagement_Objects_LargeDynamicNamedRWLock_DeathTests, ReleaseButReadLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::string res("Resource A");

  LargeDynamicNamedRWLock* pUUT = new LargeDynamicNamedRWLock();

  ASSERT_TRUE(pUUT->GetReadLock(res));

  EXPECT_DEATH(delete pUUT, ".*gpcc/src/ResourceManagement/Objects/LargeDynamicNamedRWLock.cpp.*");

  pUUT->ReleaseReadLock(res);
  delete pUUT;
}

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc_tests
