/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include <gpcc/cood/ObjectDictionary.hpp>
#include <gpcc/cood/ObjectPtr.hpp>
#include <gpcc/cood/ObjectVAR.hpp>
#include <gpcc_test/compiler/warnings.hpp>
#include <gtest/gtest.h>
#include <memory>

namespace gpcc_tests {
namespace cood       {

using namespace gpcc::cood;

using namespace testing;

// Test fixture for gpcc::cood::ObjectPtr related tests.
// Creates two ObjectDictionary instances and adds three VARIABLE objects at 0x1000, 0x1001, and 0x1002 to each instance.
class gpcc_cood_ObjectPtr_TestsF: public Test
{
  public:
    gpcc_cood_ObjectPtr_TestsF(void);

  protected:
    uint32_t data1000_1;
    uint32_t data1001_1;
    uint32_t data1002_1;

    std::unique_ptr<ObjectDictionary> spOD1;

    uint32_t data1000_2;
    uint32_t data1001_2;
    uint32_t data1002_2;

    std::unique_ptr<ObjectDictionary> spOD2;

    virtual ~gpcc_cood_ObjectPtr_TestsF(void);

    void SetUp(void) override;
    void TearDown(void) override;
};

typedef gpcc_cood_ObjectPtr_TestsF gpcc_cood_ObjectPtr_DeathTestsF;

gpcc_cood_ObjectPtr_TestsF::gpcc_cood_ObjectPtr_TestsF(void)
: Test()
, data1000_1(0)
, data1001_1(0)
, data1002_1(0)
, spOD1()
, data1000_2(0)
, data1001_2(0)
, data1002_2(0)
, spOD2()
{
}

gpcc_cood_ObjectPtr_TestsF::~gpcc_cood_ObjectPtr_TestsF(void)
{
}

void gpcc_cood_ObjectPtr_TestsF::SetUp(void)
{
  spOD1 = std::make_unique<ObjectDictionary>();

  std::unique_ptr<Object> spObj;

  spObj = std::make_unique<ObjectVAR>("Test 0",
                                      DataType::unsigned32,
                                      1,
                                      Object::attr_ACCESS_RD_PREOP,
                                      &data1000_1,
                                      nullptr,
                                      nullptr);
  spOD1->Add(spObj, 0x1000U);

  spObj = std::make_unique<ObjectVAR>("Test 1",
                                      DataType::unsigned32,
                                      1,
                                      Object::attr_ACCESS_RD_PREOP,
                                      &data1001_1,
                                      nullptr,
                                      nullptr);
  spOD1->Add(spObj, 0x1001U);

  spObj = std::make_unique<ObjectVAR>("Test 2",
                                      DataType::unsigned32,
                                      1,
                                      Object::attr_ACCESS_RD_PREOP,
                                      &data1002_1,
                                      nullptr,
                                      nullptr);
  spOD1->Add(spObj, 0x1002U);

  spOD2 = std::make_unique<ObjectDictionary>();

  spObj = std::make_unique<ObjectVAR>("Test 3",
                                      DataType::unsigned32,
                                      1,
                                      Object::attr_ACCESS_RD_PREOP,
                                      &data1000_2,
                                      nullptr,
                                      nullptr);
  spOD2->Add(spObj, 0x1000U);

  spObj = std::make_unique<ObjectVAR>("Test 4",
                                      DataType::unsigned32,
                                      1,
                                      Object::attr_ACCESS_RD_PREOP,
                                      &data1001_2,
                                      nullptr,
                                      nullptr);
  spOD2->Add(spObj, 0x1001U);

  spObj = std::make_unique<ObjectVAR>("Test 5",
                                      DataType::unsigned32,
                                      1,
                                      Object::attr_ACCESS_RD_PREOP,
                                      &data1002_2,
                                      nullptr,
                                      nullptr);
  spOD2->Add(spObj, 0x1002U);
}

void gpcc_cood_ObjectPtr_TestsF::TearDown(void)
{
  spOD1.reset();
  spOD2.reset();
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CreateAndDestroyTestFixture)
{
  EXPECT_EQ(3U, spOD1->GetNumberOfObjects());
  EXPECT_EQ(3U, spOD2->GetNumberOfObjects());
}

TEST(gpcc_cood_ObjectPtr_Tests, DefaultCTOR)
{
  ObjectPtr p;
  EXPECT_TRUE(!p);
}

TEST(gpcc_cood_ObjectPtr_Tests, CopyCTOR_nullptr)
{
  ObjectPtr p;
  EXPECT_TRUE(!p);

  ObjectPtr p2(p);
  EXPECT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CopyCTOR)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2(p1);

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1000U);
}

TEST(gpcc_cood_ObjectPtr_Tests, MoveCTOR_nullptr)
{
  ObjectPtr p;
  EXPECT_TRUE(!p);

  ObjectPtr p2(std::move(p));
  EXPECT_TRUE(!p);
  EXPECT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveCTOR)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2(std::move(p1));

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p2->GetIndex(), 0x1000U);
}

TEST(gpcc_cood_ObjectPtr_Tests, CopyAssign_Self_nullptr)
{
  ObjectPtr p;
  p = p;
  EXPECT_TRUE(!p);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CopyAssign_SelfNoNullptr)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  p1 = p1;
  ASSERT_TRUE(p1);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
}

TEST(gpcc_cood_ObjectPtr_Tests, CopyAssign_nullptr_to_nullptr)
{
  ObjectPtr p1;
  ObjectPtr p2;
  p1 = p2;
  EXPECT_TRUE(!p1);
  EXPECT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CopyAssign_Ptr_to_nullptr)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p2 = p1;

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CopyAssign_nullptr_to_ptr)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p1 = p2;

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CopyAssign_ptr_to_ptr_sameOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p1 = p2;

  ASSERT_FALSE(!p1);
  ASSERT_FALSE(!p2);

  EXPECT_EQ(p1->GetIndex(), 0x1001U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, CopyAssign_ptr_to_ptr_differentOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p1 = p2;

  ASSERT_FALSE(!p1);
  ASSERT_FALSE(!p2);

  EXPECT_EQ(p1->GetIndex(), 0x1001U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);
}

TEST(gpcc_cood_ObjectPtr_Tests, MoveAssign_Self_nullptr)
{
  ObjectPtr p;

  GPCC_DISABLE_WARN_SELFMOVE();
  p = std::move(p);
  GPCC_RESTORE_WARN_SELFMOVE();

  EXPECT_TRUE(!p);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveAssign_SelfNoNullptr)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  GPCC_DISABLE_WARN_SELFMOVE();
  p1 = std::move(p1);
  GPCC_RESTORE_WARN_SELFMOVE();

  ASSERT_TRUE(p1);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
}

TEST(gpcc_cood_ObjectPtr_Tests, MoveAssign_nullptr_to_nullptr)
{
  ObjectPtr p1;
  ObjectPtr p2;
  p1 = std::move(p2);
  EXPECT_TRUE(!p1);
  EXPECT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveAssign_Ptr_to_nullptr)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p2 = std::move(p1);

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p2->GetIndex(), 0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveAssign_nullptr_to_ptr)
{
  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p1 = std::move(p2);

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveAssign_ptr_to_ptr_sameOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p1 = std::move(p2);

  ASSERT_FALSE(!p1);
  ASSERT_FALSE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1001U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveAssign_ptr_to_ptr_differentOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p1 = std::move(p2);

  ASSERT_FALSE(!p1);
  ASSERT_FALSE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1001U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPreFix)
{
  auto p1 = spOD1->GetFirstObject();
  ASSERT_TRUE(p1);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);

  auto p2 = ++p1;
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);
  EXPECT_EQ(p1->GetIndex(), 0x1001U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPreFixOrder)
{
  auto p1 = spOD1->GetFirstObject();
  ASSERT_TRUE(p1);
  EXPECT_EQ(p1->GetIndex(), 0x1000U);

  ++p1;
  ASSERT_TRUE(p1);
  EXPECT_EQ(p1->GetIndex(), 0x1001U);

  ++p1;
  ASSERT_TRUE(p1);
  EXPECT_EQ(p1->GetIndex(), 0x1002U);

  ++p1;
  ASSERT_FALSE(p1);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPreFix_LastObj)
{
  auto p1 = spOD1->GetObject(0x1002U);
  ASSERT_TRUE(p1);

  auto p2 = ++p1;
  ASSERT_FALSE(p1);
  ASSERT_FALSE(p2);
}

TEST(gpcc_cood_ObjectPtr_Tests, OperatorPlusPlusPreFix_nullptr)
{
  ObjectPtr p1;
  EXPECT_THROW(++p1, std::logic_error);
  ASSERT_TRUE(!p1);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPostFix)
{
  auto p1 = spOD1->GetFirstObject();
  ASSERT_TRUE(p1);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);

  auto p2 = p1++;
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);
  EXPECT_EQ(p1->GetIndex(), 0x1001U);
  EXPECT_EQ(p2->GetIndex(), 0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPostFixOrder)
{
  auto p1 = spOD1->GetFirstObject();
  ASSERT_TRUE(p1);
  EXPECT_EQ(p1->GetIndex(), 0x1000U);

  p1++;
  ASSERT_TRUE(p1);
  EXPECT_EQ(p1->GetIndex(), 0x1001U);

  p1++;
  ASSERT_TRUE(p1);
  EXPECT_EQ(p1->GetIndex(), 0x1002U);

  p1++;
  ASSERT_FALSE(p1);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPostFix_LastObj)
{
  auto p1 = spOD1->GetObject(0x1002U);
  ASSERT_TRUE(p1);

  auto p2 = p1++;
  ASSERT_FALSE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p2->GetIndex(), 0x1002U);
}

TEST(gpcc_cood_ObjectPtr_Tests, OperatorPlusPlusPostFix_nullptr)
{
  ObjectPtr p1;
  EXPECT_THROW(p1++, std::logic_error);
  ASSERT_TRUE(!p1);
}

TEST(gpcc_cood_ObjectPtr_Tests, DereferencingOperator_nullptr)
{
  ObjectPtr p1;
  ASSERT_THROW((void)(*p1).GetIndex(), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, DereferencingOperator)
{
  auto p1 = spOD1->GetObject(0x1002U);
  ASSERT_TRUE(p1);

  EXPECT_EQ((*p1).GetIndex(), 0x1002U);
}

TEST(gpcc_cood_ObjectPtr_Tests, PointerMemberAccessOperator_nullptr)
{
  ObjectPtr p1;
  ASSERT_THROW((void)p1->GetIndex(), std::runtime_error);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, PointerMemberAccessOperator)
{
  auto p1 = spOD1->GetObject(0x1002U);
  ASSERT_TRUE(p1);

  EXPECT_EQ(p1->GetIndex(), 0x1002U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorNot)
{
  auto p1 = spOD1->GetObject(0x1002U);
  EXPECT_FALSE(!p1);

  ObjectPtr p2;
  EXPECT_TRUE(!p2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorBool)
{
  auto p1 = spOD1->GetObject(0x1002U);
  bool b1 = p1 ? true : false;

  ObjectPtr p2;
  bool b2 = p2 ? true : false;

  EXPECT_TRUE(b1);
  EXPECT_FALSE(b2);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorEqual_sameOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);

  ObjectPtr p3;
  ObjectPtr p4;

  // compare to self
  EXPECT_TRUE(p1 == p1);
  EXPECT_TRUE(p3 == p3);

  // compare in-equal pointers
  EXPECT_FALSE(p1 == p2);
  EXPECT_FALSE(p1 == p3);

  auto p5 = spOD1->GetObject(0x1000U);

  // compare equal pointers
  EXPECT_TRUE(p1 == p5);
  EXPECT_TRUE(p3 == p4);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorEqual_differentOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1000U);
  auto p3 = spOD2->GetObject(0x1001U);

  EXPECT_FALSE(p1 == p2);
  EXPECT_FALSE(p1 == p3);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorNotEqual_sameOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);

  ObjectPtr p3;
  ObjectPtr p4;

  // compare to self
  EXPECT_FALSE(p1 != p1);
  EXPECT_FALSE(p3 != p3);

  // compare in-equal pointers
  EXPECT_TRUE(p1 != p2);
  EXPECT_TRUE(p1 != p3);

  auto p5 = spOD1->GetObject(0x1000U);

  // compare equal pointers
  EXPECT_FALSE(p1 != p5);
  EXPECT_FALSE(p3 != p4);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorNotEqual_differentOD)
{
  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1000U);
  auto p3 = spOD2->GetObject(0x1001U);

  EXPECT_TRUE(p1 != p2);
  EXPECT_TRUE(p1 != p3);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, Reset)
{
  ObjectPtr p1;
  auto p2 = spOD1->GetObject(0x1000U);

  EXPECT_TRUE(!p1);
  EXPECT_FALSE(!p2);

  p1.Reset();
  p2.Reset();

  EXPECT_TRUE(!p1);
  EXPECT_TRUE(!p2);
}

// ----------------------------------------------------------------------------
// - Tests checking OD-Lock for Object Access hold by ObjectPtr               -
// ----------------------------------------------------------------------------
// The tests try to remove objects from the object dictionary. If TFC detects a dead-lock,
// then the OD was locked.

#if !(defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC))
#warning "Some tests have been skipped due to absence of GPCC's TFC feature. Compile using an OSAL with TFC."
#else
TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyCTOR_ODLock1)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2(p1);

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  EXPECT_DEATH(spOD1->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyCTOR_ODLock2)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2(p1);

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  EXPECT_DEATH(spOD1->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, MoveCTOR_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2(std::move(p1));

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(p2);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, DTOR_ReleaseODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  {
    auto p1 = spOD1->GetObject(0x1000U);
    ASSERT_TRUE(p1);

    EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");
  }

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_SelfNoNullptr_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  p1 = p1;
  ASSERT_TRUE(p1);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  EXPECT_EQ(p1->GetIndex(), 0x1000U);

  p1.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_Ptr_to_nullptr_ODLock1)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p2 = p1;

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1000U);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_Ptr_to_nullptr_ODLock2)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p2 = p1;

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1000U);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_nullptr_to_ptr_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1 = p2;

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(!p2);

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_ptr_to_ptr_sameOD_Lock1)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1 = p2;

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1001U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p2.Reset();

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_ptr_to_ptr_sameOD_Lock2)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1 = p2;

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1001U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p1.Reset();

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_ptr_to_ptr_differentOD_Lock1)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1001U);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");
  EXPECT_DEATH(spOD2->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1 = p2;

  // spOD1 should be unlocked now
  spOD1->Remove(0x1001U);

  // spOD2 has two locks now...
  EXPECT_DEATH(spOD2->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  EXPECT_DEATH(spOD2->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  spOD2->Remove(0x1001U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, CopyAssign_ptr_to_ptr_differentOD_Lock2)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1001U);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");
  EXPECT_DEATH(spOD2->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1 = p2;

  // spOD1 should be unlocked now
  spOD1->Remove(0x1001U);

  // spOD2 has two locks now...
  EXPECT_DEATH(spOD2->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  EXPECT_DEATH(spOD2->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD2->Remove(0x1001U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, MoveAssign_SelfNoNullptr_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  GPCC_DISABLE_WARN_SELFMOVE();
  p1 = std::move(p1);
  GPCC_RESTORE_WARN_SELFMOVE();

  ASSERT_TRUE(p1);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, MoveAssign_Ptr_to_nullptr_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p2 = std::move(p1);

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(p2);

  EXPECT_DEATH(spOD1->Remove(0x1001U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p2.Reset();

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, MoveAssign_nullptr_to_ptr_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  ASSERT_TRUE(p1);

  ObjectPtr p2;
  ASSERT_TRUE(!p2);

  p1 = std::move(p2);

  ASSERT_TRUE(!p1);
  ASSERT_TRUE(!p2);

  spOD1->Remove(0x1000U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, MoveAssign_ptr_to_ptr_sameOD_Lock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD1->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  p1 = std::move(p2);

  ASSERT_FALSE(!p1);
  ASSERT_FALSE(p2);

  EXPECT_DEATH(spOD1->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1002U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, MoveAssign_ptr_to_ptr_differentOD_Lock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetObject(0x1000U);
  auto p2 = spOD2->GetObject(0x1001U);
  ASSERT_TRUE(p1);
  ASSERT_TRUE(p2);

  EXPECT_EQ(p1->GetIndex(), 0x1000U);
  EXPECT_EQ(p2->GetIndex(), 0x1001U);

  EXPECT_DEATH(spOD1->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");
  EXPECT_DEATH(spOD2->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1 = std::move(p2);

  ASSERT_FALSE(!p1);
  ASSERT_FALSE(p2);

  // spOD1 should be unlocked now
  spOD1->Remove(0x1002U);

  // spOD2 should still be locked
  EXPECT_DEATH(spOD2->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD2->Remove(0x1002U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, OperatorPlusPlusPreFix_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetFirstObject();
  ASSERT_TRUE(p1);

  ++p1;

  EXPECT_DEATH(spOD1->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1002U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPreFix_LastObj_ODLock)
{
  auto p1 = spOD1->GetObject(0x1002U);
  ASSERT_TRUE(p1);

  ++p1;
  ASSERT_FALSE(p1);

  spOD1->Remove(0x1002U);
}

TEST_F(gpcc_cood_ObjectPtr_DeathTestsF, OperatorPlusPlusPostFix_ODLock)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto p1 = spOD1->GetFirstObject();
  ASSERT_TRUE(p1);

  p1++;

  EXPECT_DEATH(spOD1->Remove(0x1002U), ".*Dead-Lock detected. All threads permanently blocked.*");

  p1.Reset();

  spOD1->Remove(0x1002U);
}

TEST_F(gpcc_cood_ObjectPtr_TestsF, OperatorPlusPlusPostFix_LastObj_ODLock)
{
  auto p1 = spOD1->GetObject(0x1002U);
  ASSERT_TRUE(p1);

  p1++;
  ASSERT_FALSE(p1);

  spOD1->Remove(0x1002U);
}

#endif

} // namespace cood
} // namespace gpcc_tests
