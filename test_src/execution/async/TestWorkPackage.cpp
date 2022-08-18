/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace execution {
namespace async {

using gpcc::execution::async::WorkPackage;
using namespace testing;

/// Test fixture for gpcc::execution::async::WorkPackage related tests.
class gpcc_execution_async_WorkPackage_TestsF: public Test
{
  public:
    gpcc_execution_async_WorkPackage_TestsF(void);

  protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void DummyFunc(void);

  int dummyOwner;
  std::unique_ptr<WorkPackage> spUUT;
};

gpcc_execution_async_WorkPackage_TestsF::gpcc_execution_async_WorkPackage_TestsF()
: Test()
, dummyOwner(0)
, spUUT()
{
}
void gpcc_execution_async_WorkPackage_TestsF::SetUp(void)
{
}
void gpcc_execution_async_WorkPackage_TestsF::TearDown(void)
{
}
void gpcc_execution_async_WorkPackage_TestsF::DummyFunc(void)
{
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_copyFunctor)::DummyFunc, this);
  spUUT.reset(new WorkPackage(&dummyOwner, 0, f));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_moveFunctor)::DummyFunc, this);
  spUUT.reset(new WorkPackage(&dummyOwner, 0, std::move(f)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT.reset(new WorkPackage(nullptr, 0, f));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT.reset(new WorkPackage(nullptr, 0, std::move(f)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_NoFunctionReferenced_copyFunctor)
{
  auto const f = WorkPackage::tFunctor();
  EXPECT_THROW(spUUT.reset(new WorkPackage(&dummyOwner, 0, f)), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateStatic_NoFunctionReferenced_moveFunctor)
{
  auto f = WorkPackage::tFunctor();
  EXPECT_THROW(spUUT.reset(new WorkPackage(&dummyOwner, 0, std::move(f))), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_copyFunctor)::DummyFunc, this);
  spUUT = WorkPackage::CreateDynamic(&dummyOwner, 0, f);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_moveFunctor)::DummyFunc, this);
  spUUT = WorkPackage::CreateDynamic(&dummyOwner, 0, std::move(f));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT = WorkPackage::CreateDynamic(nullptr, 0, f);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT = WorkPackage::CreateDynamic(nullptr, 0, std::move(f));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_NoFunctionReferenced_copyFunctor)
{
  auto const f = WorkPackage::tFunctor();
  ASSERT_THROW(spUUT = WorkPackage::CreateDynamic(&dummyOwner, 0, f), std::invalid_argument);
}

TEST_F(gpcc_execution_async_WorkPackage_TestsF, CreateDynamic_NoFunctionReferenced_moveFunctor)
{
  auto f = WorkPackage::tFunctor();
  ASSERT_THROW(spUUT = WorkPackage::CreateDynamic(&dummyOwner, 0, std::move(f)), std::invalid_argument);
}

} // namespace execution
} // namespace async
} // namespace gpcc_tests
