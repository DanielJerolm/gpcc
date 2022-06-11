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

#include "gpcc/src/execution/async/DeferredWorkPackage.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include "gtest/gtest.h"

namespace gpcc_tests {
namespace execution {
namespace async {

using gpcc::execution::async::DeferredWorkPackage;
using gpcc::time::TimePoint;
using gpcc::time::TimeSpan;
using namespace testing;

/// Test fixture for gpcc::execution::async::DeferredWorkPackage related tests.
class gpcc_execution_async_DeferredWorkPackage_TestsF: public Test
{
  public:
    gpcc_execution_async_DeferredWorkPackage_TestsF(void);

  protected:
    void SetUp(void) override;
    void TearDown(void) override;

    void DummyFunc(void);

  int dummyOwner;
  std::unique_ptr<DeferredWorkPackage> spUUT;
};

gpcc_execution_async_DeferredWorkPackage_TestsF::gpcc_execution_async_DeferredWorkPackage_TestsF()
: Test()
, dummyOwner(0)
, spUUT()
{
}
void gpcc_execution_async_DeferredWorkPackage_TestsF::SetUp(void)
{
}
void gpcc_execution_async_DeferredWorkPackage_TestsF::TearDown(void)
{
}
void gpcc_execution_async_DeferredWorkPackage_TestsF::DummyFunc(void)
{
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_copyFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, f, TimePoint()));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_moveFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::move(f), TimePoint()));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_copyFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, f, TimeSpan::ms(5)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_moveFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::move(f), TimeSpan::ms(5)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_copyFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, f));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_moveFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::move(f)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(nullptr, 0, f, TimePoint()));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(nullptr, 0, std::move(f), TimePoint()));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(nullptr, 0, f, TimeSpan::ms(5)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(nullptr, 0, std::move(f), TimeSpan::ms(5)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(nullptr, 0, f));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT.reset(new DeferredWorkPackage(nullptr, 0, std::move(f)));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_NoFunctionReferenced_copyFunctor)
{
  auto const f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, f, TimePoint())), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_TimePoint_NoFunctionReferenced_moveFunctor)
{
  auto f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::move(f), TimePoint())), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_NoFunctionReferenced_copyFunctor)
{
  auto const f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, f, TimeSpan::ms(5))), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_Timespan_NoFunctionReferenced_moveFunctor)
{
  auto f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::move(f), TimeSpan::ms(5))), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_NoFunctionReferenced_copyFunctor)
{
  auto const f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, f)), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateStatic_NoTime_NoFunctionReferenced_moveFunctor)
{
  auto f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::move(f))), std::invalid_argument);
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_copyFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, f, TimePoint());
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_moveFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, std::move(f), TimePoint());
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_copyFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, f, TimeSpan::ms(5));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_moveFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, std::move(f), TimeSpan::ms(5));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(nullptr, 0, f, TimePoint());
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(nullptr, 0, std::move(f), TimePoint());
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_NoOwner_copyFunctor)
{
  auto const f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_NoOwner_copyFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(nullptr, 0, f, TimeSpan::ms(5));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_NoOwner_moveFunctor)
{
  auto f = std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_NoOwner_moveFunctor)::DummyFunc, this);
  spUUT = DeferredWorkPackage::CreateDynamic(nullptr, 0, std::move(f), TimeSpan::ms(5));
  spUUT.reset();
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_NoFunctionReferenced_copyFunctor)
{
  auto const f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, f, TimePoint()), std::invalid_argument);
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_TimePoint_NoFunctionReferenced_moveFunctor)
{
  auto f = DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, std::move(f), TimePoint()), std::invalid_argument);
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_NoFunctionReferenced_copyFunctor)
{
  auto const f =  DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, f, TimeSpan::ms(5)), std::invalid_argument);
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, CreateDynamic_Timespan_NoFunctionReferenced_moveFunctor)
{
  auto f =  DeferredWorkPackage::tFunctor();
  ASSERT_THROW(spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, std::move(f), TimeSpan::ms(5)), std::invalid_argument);
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, SetTimePoint)
{
  // static work package
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, SetTimePoint)::DummyFunc, this), TimePoint()));
  spUUT->SetTimePoint(TimePoint());

  // dynamic work package
  spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, SetTimePoint)::DummyFunc, this), TimePoint());
  ASSERT_THROW(spUUT->SetTimePoint(TimePoint()), std::logic_error);
}

TEST_F(gpcc_execution_async_DeferredWorkPackage_TestsF, SetTimeSpan)
{
  // static work package
  spUUT.reset(new DeferredWorkPackage(&dummyOwner, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, SetTimeSpan)::DummyFunc, this), TimePoint()));
  spUUT->SetTimeSpan(TimeSpan::ms(10));

  // dynamic work package
  spUUT = DeferredWorkPackage::CreateDynamic(&dummyOwner, 0, std::bind(&GTEST_TEST_CLASS_NAME_(gpcc_execution_async_DeferredWorkPackage_TestsF, SetTimeSpan)::DummyFunc, this), TimePoint());
  ASSERT_THROW(spUUT->SetTimeSpan(TimeSpan::ms(10)), std::logic_error);
}

} // namespace execution
} // namespace async
} // namespace gpcc_tests
