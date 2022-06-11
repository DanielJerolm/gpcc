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
