/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018, 2021, 2022 Daniel Jerolm

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

#include "gpcc/src/cood/ObjectDictionary.hpp"
#include "gpcc/src/cood/ObjectPtr.hpp"
#include "gpcc/src/cood/ObjectVAR.hpp"
#include <memory>
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;

using namespace testing;

// Test fixture for gpcc::cood::ObjectDictionary related tests.
class gpcc_cood_ObjectDictionary_TestsF: public Test
{
  public:
    gpcc_cood_ObjectDictionary_TestsF(void);

  protected:
    uint32_t data1000;
    uint32_t data1001;
    uint32_t data1002;

    std::unique_ptr<ObjectDictionary> spUUT;

    virtual ~gpcc_cood_ObjectDictionary_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;

    std::unique_ptr<Object> CreateTestObj1000(void);
    std::unique_ptr<Object> CreateTestObj1001(void);
    std::unique_ptr<Object> CreateTestObj1002(void);
};

typedef gpcc_cood_ObjectDictionary_TestsF gpcc_cood_ObjectDictionary_DeathTestsF;

gpcc_cood_ObjectDictionary_TestsF::gpcc_cood_ObjectDictionary_TestsF(void)
: Test()
, data1000(0)
, data1001(0)
, data1002(0)
, spUUT()
{
}

gpcc_cood_ObjectDictionary_TestsF::~gpcc_cood_ObjectDictionary_TestsF(void)
{
}

void gpcc_cood_ObjectDictionary_TestsF::SetUp(void)
{
  spUUT = std::make_unique<ObjectDictionary>();
}

void gpcc_cood_ObjectDictionary_TestsF::TearDown(void)
{
  spUUT.reset();
}

std::unique_ptr<Object> gpcc_cood_ObjectDictionary_TestsF::CreateTestObj1000(void)
{
  return std::make_unique<ObjectVAR>("Test 0",
                                     DataType::unsigned32,
                                     1,
                                     Object::attr_ACCESS_RD_PREOP,
                                     &data1000,
                                     nullptr,
                                     nullptr);
}

std::unique_ptr<Object> gpcc_cood_ObjectDictionary_TestsF::CreateTestObj1001(void)
{
  return std::make_unique<ObjectVAR>("Test 1",
                                     DataType::unsigned32,
                                     1,
                                     Object::attr_ACCESS_RD_PREOP,
                                     &data1001,
                                     nullptr,
                                     nullptr);
}

std::unique_ptr<Object> gpcc_cood_ObjectDictionary_TestsF::CreateTestObj1002(void)
{
  return std::make_unique<ObjectVAR>("Test 2",
                                     DataType::unsigned32,
                                     1,
                                     Object::attr_ACCESS_RD_PREOP,
                                     &data1002,
                                     nullptr,
                                     nullptr);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, CreateAndDestroy)
{
}

TEST_F(gpcc_cood_ObjectDictionary_DeathTestsF, Destroy_WhileLocked)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto locker = spUUT->LockForObjAccess();
  EXPECT_DEATH(spUUT.reset(), ".*ObjectDictionary::~ObjectDictionary: In use.*");
}

TEST_F(gpcc_cood_ObjectDictionary_DeathTestsF, Destroy_WhileThereIsAObjPtr)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto spObj = CreateTestObj1000();

  spUUT->Add(spObj, 0x1000U);
  ASSERT_TRUE(!spObj);

  {
    auto objPtr = spUUT->GetObject(0x1000U);
    EXPECT_DEATH(spUUT.reset(), ".*ObjectDictionary::~ObjectDictionary: In use.*");
  }
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Add_OK)
{
  auto spObj = CreateTestObj1000();

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Add_IndexInUse)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();

  spUUT->Add(spObj1, 0x1000U);
  EXPECT_TRUE(!spObj1);

  EXPECT_THROW(spUUT->Add(spObj2, 0x1000U), std::logic_error);
  EXPECT_FALSE(!spObj2);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Add_MultipleObjects)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  spUUT->Add(spObj3, 0x1002U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 3U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Remove_OK)
{
  auto spObj = CreateTestObj1000();

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);

  spUUT->Remove(0x1000U);
  EXPECT_EQ(spUUT->GetNumberOfObjects(), 0U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Remove_NoObject)
{
  auto spObj = CreateTestObj1000();

  // (OD is empty)
  spUUT->Remove(0x1000U);
  EXPECT_EQ(spUUT->GetNumberOfObjects(), 0U);

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);

  // (OD does not contain an object 0x1001)
  spUUT->Remove(0x1001U);
  EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Remove_One)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  spUUT->Add(spObj3, 0x1002U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 3U);

  spUUT->Remove(0x1001U);
  EXPECT_EQ(spUUT->GetNumberOfObjects(), 2U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Clear)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  spUUT->Add(spObj3, 0x1002U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 3U);

  spUUT->Clear();
  EXPECT_EQ(spUUT->GetNumberOfObjects(), 0U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, Clear_NoObjects)
{
  spUUT->Clear();
  EXPECT_EQ(spUUT->GetNumberOfObjects(), 0U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, LockForObjectAccess)
{
  auto locker = spUUT->LockForObjAccess();

  // test recursive locking
  auto locker2 = spUUT->LockForObjAccess();
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetNumberOfObjects)
{
  auto spObj = CreateTestObj1000();

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 0U);

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetNumberOfObjects_ExtraLock)
{
  auto spObj = CreateTestObj1000();

  {
    auto locker = spUUT->LockForObjAccess();
    EXPECT_EQ(spUUT->GetNumberOfObjects(), 0U);
  }

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  {
    auto locker = spUUT->LockForObjAccess();
    EXPECT_EQ(spUUT->GetNumberOfObjects(), 1U);
  }
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetIndices_NoObjects)
{
  auto v = spUUT->GetIndices();
  EXPECT_TRUE(v.empty());
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetIndices_OneObject)
{
  auto spObj = CreateTestObj1000();

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  auto v = spUUT->GetIndices();
  ASSERT_EQ(v.size(), 1U);
  EXPECT_EQ(v[0], 0x1000U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetIndices_ExtraLock)
{
  auto spObj = CreateTestObj1000();

  spUUT->Add(spObj, 0x1000U);
  EXPECT_TRUE(!spObj);

  {
    auto locker = spUUT->LockForObjAccess();
    auto v = spUUT->GetIndices();
    ASSERT_EQ(v.size(), 1U);
    EXPECT_EQ(v[0], 0x1000U);
  }
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetIndices_Multiple)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  // note: expect indices sorted by value (ascending)
  auto v = spUUT->GetIndices();
  ASSERT_EQ(v.size(), 3U);
  EXPECT_EQ(v[0], 0x1000U);
  EXPECT_EQ(v[1], 0x1001U);
  EXPECT_EQ(v[2], 0x1002U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetFirstObject)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  auto objPtr = spUUT->GetFirstObject();

  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1000U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetFirstObject_ExtraLock)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  {
    auto locker = spUUT->LockForObjAccess();
    auto objPtr = spUUT->GetFirstObject();

    ASSERT_FALSE(!objPtr);
    EXPECT_EQ(objPtr->GetIndex(), 0x1000U);
  }
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetFirstObject_ODEmpty)
{
  auto objPtr = spUUT->GetFirstObject();
  ASSERT_TRUE(!objPtr);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetObject)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  auto objPtr = spUUT->GetObject(0x1001U);

  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1001U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetObject_ExtraLock)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  {
    auto locker = spUUT->LockForObjAccess();
    auto objPtr = spUUT->GetObject(0x1001U);

    ASSERT_FALSE(!objPtr);
    EXPECT_EQ(objPtr->GetIndex(), 0x1001U);
  }
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetObject_NoObj)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  auto objPtr = spUUT->GetObject(0x1003U);

  ASSERT_TRUE(!objPtr);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetObject_SameObjTwice)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  // note: Added in not sorted order
  spUUT->Add(spObj3, 0x1002U);
  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1001U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  auto objPtr1 = spUUT->GetObject(0x1001U);
  auto objPtr2 = spUUT->GetObject(0x1001U);
  ASSERT_FALSE(!objPtr1);
  EXPECT_EQ(objPtr1->GetIndex(), 0x1001U);
  ASSERT_FALSE(!objPtr2);
  EXPECT_EQ(objPtr2->GetIndex(), 0x1001U);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetNextNearestObject_ODEmpty)
{
  auto objPtr = spUUT->GetNextNearestObject(0x0000U);
  ASSERT_TRUE(!objPtr);

  objPtr = spUUT->GetNextNearestObject(0x0001U);
  ASSERT_TRUE(!objPtr);

  objPtr = spUUT->GetNextNearestObject(0x1000U);
  ASSERT_TRUE(!objPtr);

  objPtr = spUUT->GetNextNearestObject(0xFFFFU);
  ASSERT_TRUE(!objPtr);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetNextNearestObject_OK1)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj1, 0x1000U);
  spUUT->Add(spObj2, 0x1010U);
  spUUT->Add(spObj3, 0x1011U);

  // -- direct hit --
  auto objPtr = spUUT->GetNextNearestObject(0x1000U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1000U);

  objPtr = spUUT->GetNextNearestObject(0x1010U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1010U);

  objPtr = spUUT->GetNextNearestObject(0x1011U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1011U);

  // -- no direct hit --
  objPtr = spUUT->GetNextNearestObject(0x0000U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1000U);

  objPtr = spUUT->GetNextNearestObject(0x0FFFU);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1000U);

  objPtr = spUUT->GetNextNearestObject(0x1001U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1010U);

  objPtr = spUUT->GetNextNearestObject(0x1009U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1010U);

  // -- no direct hit and no subsequent object --
  objPtr = spUUT->GetNextNearestObject(0x1012U);
  ASSERT_TRUE(!objPtr);
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, GetNextNearestObject_OK2)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj1, 0x0U);
  spUUT->Add(spObj2, 0x1010U);
  spUUT->Add(spObj3, 0x1011U);

  // -- direct hit at 0x0000 --
  auto objPtr = spUUT->GetNextNearestObject(0x0000U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x0000U);

  objPtr = spUUT->GetNextNearestObject(0x0001U);
  ASSERT_FALSE(!objPtr);
  EXPECT_EQ(objPtr->GetIndex(), 0x1010U);
}

#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
#warning "Some tests have been skipped due to absence of GPCC's TFC feature. Compile using an OSAL with TFC."
#else
TEST_F(gpcc_cood_ObjectDictionary_DeathTestsF, ObjectPtr_ODLock_HoldDuringLife)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj3, 0x1000U);
  spUUT->Add(spObj1, 0x1001U);
  spUUT->Add(spObj2, 0x1002U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  auto objPtr = spUUT->GetObject(0x1001U);

  // objPtr keeps the object dictionary locked for object access.
  // Invoking Remove() will lock for object dictionary modification. This will result in a dead-lock which
  // is detected by TFC.
  EXPECT_DEATH(spUUT->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");
}

TEST_F(gpcc_cood_ObjectDictionary_TestsF, ObjectPtr_ODLock_UnlockUponDestruction)
{
  auto spObj1 = CreateTestObj1000();
  auto spObj2 = CreateTestObj1001();
  auto spObj3 = CreateTestObj1002();

  spUUT->Add(spObj3, 0x1000U);
  spUUT->Add(spObj1, 0x1001U);
  spUUT->Add(spObj2, 0x1002U);
  EXPECT_TRUE(!spObj1);
  EXPECT_TRUE(!spObj2);
  EXPECT_TRUE(!spObj3);

  {
    auto objPtr = spUUT->GetObject(0x1001U);
    EXPECT_FALSE(!objPtr);
  }

  // objPtr locks and unlocks the object dictionary for object access.
  // Invoking Remove() will lock for object dictionary modification. There should be no dead-lock.
  // A dead-lock would be detected by TFC.
  spUUT->Remove(0x1002U);
}
#endif

} // namespace cood
} // namespace gpcc_tests
