/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2018, 2022 Daniel Jerolm

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

#include "gpcc/src/ResourceManagement/Objects/HierarchicNamedRWLock.hpp"
#include "gpcc/src/ResourceManagement/Objects/exceptions.hpp"
#include "gtest/gtest.h"
#include <memory>
#include <stdexcept>

namespace gpcc_tests          {
namespace ResourceManagement  {
namespace Objects             {

using namespace testing;
using gpcc::ResourceManagement::Objects::HierarchicNamedRWLock;
using gpcc::ResourceManagement::Objects::NotLockedError;

// Test fixture for class HierarchicNamedRWLock.
// Provides the UUT and prevents a Panic in ~HierarchicNamedRWLock() if a test case did
// not unlock all locks. In this case, a non-fatal failure is added to the test.
class gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF: public Test
{
  public:
    gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF(void);

  protected:
    HierarchicNamedRWLock uut;

    void SetUp(void) override;
    void TearDown(void) override;
};

gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF::gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF(void)
: uut()
{
}

void gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF::SetUp(void)
{
  // intentionally empty
}

void gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF::TearDown(void)
{
  if (uut.IsAnyLock())
  {
    ADD_FAILURE() << "Unit test case did not unlock all R/W locks at UUT";
    uut.Reset();
  }
}


TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, CreateAndRelease)
{
}

TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_LockAndUnlock)
{
  ASSERT_TRUE(uut.GetReadLock("Test"));
  uut.ReleaseReadLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_LockAndUnlockWithEmptyStrings)
{
  ASSERT_THROW(uut.GetReadLock(""), std::invalid_argument);
  ASSERT_THROW(uut.ReleaseReadLock(""), std::invalid_argument);
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockButNeverLocked)
{
  ASSERT_THROW(uut.ReleaseReadLock("Test"), NotLockedError);
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockButLockedByWriter)
{
  uut.GetWriteLock("Test");
  ASSERT_THROW(uut.ReleaseReadLock("Test"), NotLockedError);
  uut.ReleaseWriteLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_MultipleReadersCanLock)
{
  ASSERT_TRUE(uut.GetReadLock("Test"));
  ASSERT_TRUE(uut.GetReadLock("Test"));
  uut.ReleaseReadLock("Test");
  uut.ReleaseReadLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_WriterCannotLock)
{
  ASSERT_TRUE(uut.GetReadLock("Test"));
  ASSERT_FALSE(uut.GetWriteLock("Test"));
  uut.ReleaseReadLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_WriterCannotLockParent)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A"));
  ASSERT_FALSE(uut.GetWriteLock("Test/"));
  uut.ReleaseReadLock("Test/A");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_ReaderCanLockParent)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A"));
  ASSERT_TRUE(uut.GetReadLock("Test/"));
  uut.ReleaseReadLock("Test/");
  uut.ReleaseReadLock("Test/A");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_WriterCanLockChild)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetWriteLock("Test/A/B"));
  uut.ReleaseWriteLock("Test/A/B");
  uut.ReleaseReadLock("Test/A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_ReaderCanLockChild)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B"));
  uut.ReleaseReadLock("Test/A/B");
  uut.ReleaseReadLock("Test/A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_ResourcesInSameGroupCanBeLocked)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/R1/"));
  ASSERT_TRUE(uut.GetWriteLock("Test/A/R2/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/R3/"));
  uut.ReleaseReadLock("Test/A/R1/");
  uut.ReleaseWriteLock("Test/A/R2/");
  uut.ReleaseReadLock("Test/A/R3/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockOrderStraight2)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B"));
  uut.ReleaseReadLock("Test/A/B");
  uut.ReleaseReadLock("Test/A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockOrderStraight2Revers)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B"));
  uut.ReleaseReadLock("Test/A/");
  uut.ReleaseReadLock("Test/A/B");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockOrderStraight3)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/C"));
  uut.ReleaseReadLock("Test/A/B/C");
  uut.ReleaseReadLock("Test/A/B/");
  uut.ReleaseReadLock("Test/A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockOrderStraight3Revers)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/C"));
  uut.ReleaseReadLock("Test/A/");
  uut.ReleaseReadLock("Test/A/B/");
  uut.ReleaseReadLock("Test/A/B/C");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockOrderStraight3MidFirst1)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/C"));
  uut.ReleaseReadLock("Test/A/B/");
  uut.ReleaseReadLock("Test/A/");
  uut.ReleaseReadLock("Test/A/B/C");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ReadLock_UnlockOrderStraight3MidFirst2)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/B/C"));
  uut.ReleaseReadLock("Test/A/B/");
  uut.ReleaseReadLock("Test/A/B/C");
  uut.ReleaseReadLock("Test/A/");
}

TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_LockAndUnlock)
{
  ASSERT_TRUE(uut.GetWriteLock("Test"));
  uut.ReleaseWriteLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_LockAndUnlockWithEmptyStrings)
{
  ASSERT_THROW(uut.GetWriteLock(""), std::invalid_argument);
  ASSERT_THROW(uut.ReleaseWriteLock(""), std::invalid_argument);
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_UnlockButNeverLocked)
{
  ASSERT_THROW(uut.ReleaseWriteLock("Test"), NotLockedError);
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_UnlockButLockedByReader)
{
  uut.GetReadLock("Test");
  ASSERT_THROW(uut.ReleaseWriteLock("Test"), NotLockedError);
  uut.ReleaseReadLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_OnlyOneWriterCanLock)
{
  ASSERT_TRUE(uut.GetWriteLock("Test"));
  ASSERT_FALSE(uut.GetWriteLock("Test"));
  uut.ReleaseWriteLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_ReaderCannotLock)
{
  ASSERT_TRUE(uut.GetWriteLock("Test"));
  ASSERT_FALSE(uut.GetReadLock("Test"));
  uut.ReleaseWriteLock("Test");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_ReaderCanLockParent)
{
  ASSERT_TRUE(uut.GetWriteLock("Test/A"));
  ASSERT_TRUE(uut.GetReadLock("Test/"));
  uut.ReleaseReadLock("Test/");
  uut.ReleaseWriteLock("Test/A");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_WriterCannotLockParent)
{
  ASSERT_TRUE(uut.GetWriteLock("Test/A"));
  ASSERT_FALSE(uut.GetWriteLock("Test/"));
  uut.ReleaseWriteLock("Test/A");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_ReaderCannotLockChild)
{
  ASSERT_TRUE(uut.GetWriteLock("Test/"));
  ASSERT_FALSE(uut.GetReadLock("Test/A"));
  uut.ReleaseWriteLock("Test/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_WriterCannotLockChild)
{
  ASSERT_TRUE(uut.GetWriteLock("Test/"));
  ASSERT_FALSE(uut.GetWriteLock("Test/A"));
  uut.ReleaseWriteLock("Test/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, WriteLock_ResourcesInSameGroupCanBeLocked)
{
  ASSERT_TRUE(uut.GetWriteLock("Test/A/R1/"));
  ASSERT_TRUE(uut.GetWriteLock("Test/A/R2/"));
  ASSERT_TRUE(uut.GetReadLock("Test/A/R3/"));
  uut.ReleaseWriteLock("Test/A/R1/");
  uut.ReleaseWriteLock("Test/A/R2/");
  uut.ReleaseReadLock("Test/A/R3/");
}

TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_NoLockEver)
{
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_Readers1)
{
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/"));
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseReadLock("Test/");
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseReadLock("Test/A/B");
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_Readers2)
{
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/"));
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseReadLock("Test/A/B");
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseReadLock("Test/");
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_Writers1)
{
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/C"));
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseWriteLock("Test/C");
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseWriteLock("Test/A/B");
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_Writers2)
{
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/C"));
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseWriteLock("Test/A/B");
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseWriteLock("Test/C");
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_ReadersAndWriters1)
{
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/C"));
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseWriteLock("Test/A/B");
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseReadLock("Test/C");
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, IsAnyLock_ReadersAndWriters2)
{
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/C"));
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseReadLock("Test/C");
  ASSERT_TRUE(uut.IsAnyLock());

  uut.ReleaseWriteLock("Test/A/B");
  ASSERT_FALSE(uut.IsAnyLock());
}

TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ResetNoLocks)
{
  ASSERT_FALSE(uut.IsAnyLock());
  uut.Reset();
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetWriteLock("Test/A/B"));
  ASSERT_TRUE(uut.IsAnyLock());
  uut.ReleaseWriteLock("Test/A/B");
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ResetLocksAreCleared1)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A"));
  ASSERT_TRUE(uut.GetWriteLock("Test/B"));
  ASSERT_TRUE(uut.IsAnyLock());
  uut.Reset();
  ASSERT_FALSE(uut.IsAnyLock());
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ResetLocksAreCleared2)
{
  ASSERT_TRUE(uut.GetReadLock("Test/A"));
  ASSERT_TRUE(uut.GetWriteLock("Test/B"));
  ASSERT_TRUE(uut.IsAnyLock());
  uut.Reset();
  ASSERT_FALSE(uut.IsAnyLock());

  ASSERT_TRUE(uut.GetReadLock("Test/A"));
  ASSERT_TRUE(uut.GetWriteLock("Test/B"));
  uut.ReleaseReadLock("Test/A");
  uut.ReleaseWriteLock("Test/B");
}

TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ChainedLockUnlock1)
{
  ASSERT_TRUE(uut.GetReadLock("A/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/D/"));
  uut.ReleaseReadLock("A/B/C/D/");
  uut.ReleaseReadLock("A/B/C/");
  uut.ReleaseReadLock("A/B/");
  uut.ReleaseReadLock("A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ChainedLockUnlock2)
{
  ASSERT_TRUE(uut.GetReadLock("A/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/D/"));
  uut.ReleaseReadLock("A/");
  uut.ReleaseReadLock("A/B/");
  uut.ReleaseReadLock("A/B/C/");
  uut.ReleaseReadLock("A/B/C/D/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ChainedLockUnlock3)
{
  ASSERT_TRUE(uut.GetReadLock("A/B/C/D/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/"));
  ASSERT_TRUE(uut.GetReadLock("A/"));
  uut.ReleaseReadLock("A/");
  uut.ReleaseReadLock("A/B/");
  uut.ReleaseReadLock("A/B/C/");
  uut.ReleaseReadLock("A/B/C/D/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ChainedLockUnlock4)
{
  ASSERT_TRUE(uut.GetReadLock("A/B/C/D/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/"));
  ASSERT_TRUE(uut.GetReadLock("A/"));
  uut.ReleaseReadLock("A/B/C/D/");
  uut.ReleaseReadLock("A/B/C/");
  uut.ReleaseReadLock("A/B/");
  uut.ReleaseReadLock("A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, ChainedLockUnlock5)
{
  ASSERT_TRUE(uut.GetReadLock("A/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C/D/"));
  uut.ReleaseReadLock("A/B/");
  uut.ReleaseReadLock("A/B/C/");
  uut.ReleaseReadLock("A/B/C/D/");
  uut.ReleaseReadLock("A/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, TreeLockUnlock1)
{
  ASSERT_TRUE(uut.GetReadLock("A/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C1/"));
  ASSERT_TRUE(uut.GetReadLock("A/B/C2/"));
  // A/ -> B/C -+-> 1/
  //           |
  //           +--> 2/

  uut.ReleaseReadLock("A/B/C1/");
  // A/ -> B/C2/

  uut.ReleaseReadLock("A/");
  // A/B/C2/

  uut.ReleaseReadLock("A/B/C2/");
}

TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, RLS1)
{
  // '/' is used as the separating character recommended by class HierarchicNamedRWLock doc.
  // Therefore files and directories need a trailing '/'. See doc of HierarchicNamedRWLock why this is necessary.

  ASSERT_TRUE(uut.GetWriteLock("~/demo/tests/file1.txt/"));          // A
  ASSERT_TRUE(uut.GetReadLock("~/demo/tests/"));                     // B
  ASSERT_TRUE(uut.GetReadLock("~/demo/tests/file2.txt/"));           // C
  ASSERT_TRUE(uut.GetWriteLock("~/demo/file.dat/"));                 // D
  ASSERT_TRUE(uut.GetReadLock("~/stuff/file1.txt/"));                // E
  ASSERT_TRUE(uut.GetReadLock("~/demo/tests/file1.txt.old/"));       // F
  uut.ReleaseWriteLock("~/demo/tests/file1.txt/");                   // a
  ASSERT_TRUE(uut.GetReadLock("~/demo/tests/file1.txt/"));           // A
  uut.ReleaseReadLock("~/demo/tests/file1.txt.old/");                // f
  ASSERT_TRUE(uut.GetWriteLock("~/demo/tests/file1.txt.old/"));      // F
  uut.ReleaseWriteLock("~/demo/tests/file1.txt.old/");               // f
  uut.ReleaseReadLock("~/demo/tests/");                              // b
  uut.ReleaseReadLock("~/demo/tests/file2.txt/");                    // c
  uut.ReleaseWriteLock("~/demo/file.dat/");                          // d
  uut.ReleaseReadLock("~/stuff/file1.txt/");                         // e
  uut.ReleaseReadLock("~/demo/tests/file1.txt/");                    // a
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, RLS2)
{
  // '.' is used as the separating character recommended by class HierarchicNamedRWLock doc.
  // Therefore group and resource names need a trailing '/'. See doc of HierarchicNamedRWLock why this is necessary.

  ASSERT_TRUE(uut.GetReadLock("devices.dev1.params.p3."));
  ASSERT_TRUE(uut.GetWriteLock("devices.dev3.params.p33."));
  ASSERT_TRUE(uut.GetWriteLock("devices.dev3.params.p3."));
  ASSERT_TRUE(uut.GetReadLock("devices.dev1.params.p3."));
  ASSERT_TRUE(uut.GetWriteLock("devices.dev1.params.p33."));
  ASSERT_TRUE(uut.GetWriteLock("devices.dev2.params.p3."));
  ASSERT_TRUE(uut.GetReadLock("devices.dev1."));
  uut.ReleaseReadLock("devices.dev1.params.p3.");
  uut.ReleaseReadLock("devices.dev1.params.p3.");
  uut.ReleaseReadLock("devices.dev1.");
  uut.ReleaseWriteLock("devices.dev3.params.p33.");
  uut.ReleaseWriteLock("devices.dev1.params.p33.");
  uut.ReleaseWriteLock("devices.dev2.params.p3.");
  uut.ReleaseWriteLock("devices.dev3.params.p3.");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, RLS3)
{
  // '/' is used as the separating character recommended by class HierarchicNamedRWLock doc.
  // Therefore files and directories need a trailing '/'. See doc of HierarchicNamedRWLock why this is necessary.
  // This test case checks proper behavior if the root "/" directory itself is locked.

  ASSERT_TRUE(uut.GetWriteLock("/"));
  ASSERT_FALSE(uut.GetReadLock("/dir/"));
  uut.ReleaseWriteLock("/");
}
TEST_F(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_TestsF, RLS4)
{
  // '/' is used as the separating character recommended by class HierarchicNamedRWLock doc.
  // Therefore files and directories need a trailing '/'. See doc of HierarchicNamedRWLock why this is necessary.
  // This test case checks proper behavior if the root "/" directory itself is locked.

  ASSERT_TRUE(uut.GetReadLock("/dir/"));
  ASSERT_FALSE(uut.GetWriteLock("/"));
  uut.ReleaseReadLock("/dir/");
}

TEST(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_DeathTests, DestroyWithWriteLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<HierarchicNamedRWLock> spUUT(new HierarchicNamedRWLock());

  ASSERT_TRUE(spUUT->GetWriteLock("Test"));

  EXPECT_DEATH(spUUT.reset(), ".*gpcc/src/ResourceManagement/Objects/HierarchicNamedRWLock.cpp.*");

  spUUT->ReleaseWriteLock("Test");
}
TEST(gpcc_ResourceManagement_Objects_HierarchicNamedRWLock_DeathTests, DestroyWithReadLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  std::unique_ptr<HierarchicNamedRWLock> spUUT(new HierarchicNamedRWLock());

  ASSERT_TRUE(spUUT->GetWriteLock("Test"));

  EXPECT_DEATH(spUUT.reset(), ".*gpcc/src/ResourceManagement/Objects/HierarchicNamedRWLock.cpp.*");

  spUUT->ReleaseWriteLock("Test");
}

} // namespace Objects
} // namespace ResourceManagement
} // namespace gpcc
