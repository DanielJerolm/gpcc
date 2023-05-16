/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "TestbenchWorkQueueBasedRAS.hpp"
#include <gpcc/cood/remote_access/infrastructure/WorkQueueBasedRemoteAccessServer.hpp>
#include <gpcc/execution/async/DWQwithThread.hpp>
#include <gpcc/osal/Thread.hpp>
#include "testcases/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiableMock.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_LoanExecutionContext.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_ObjectEnum.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_ObjectInfo.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_Ping.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_Read.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_RegisterUnregisterStartStop.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_Send.hpp"
#include "testcases/cood/remote_access/roda_itf/TestIRODA_Write.hpp"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

#ifndef SKIP_TFC_BASED_TESTS

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_LoanExecutionContextTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_ObjectEnumTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_ObjectInfoTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_PingTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_ReadTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_RegisterUnregisterStartStopTestsF, TestbenchWorkQueueBasedRAS);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_RegisterUnregisterStartStopDeathTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_SendTestsF, TestbenchWorkQueueBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_WorkQueueBasedRemoteAccessServer_, IRODA_WriteTestsF, TestbenchWorkQueueBasedRAS);

#endif

class gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF: public Test
{
  public:
    gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF(void);
    ~gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF(void);

  protected:
    gpcc::execution::async::DWQwithThread dwqWithThread;
    gpcc::log::Logger logger;
    gpcc::cood::ObjectDictionary od;

    void SetUp(void) override;
    void TearDown(void) override;
};

gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF::gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF(void)
: Test()
, dwqWithThread("DWQThread")
, logger("Test")
, od()
{
  dwqWithThread.Start(gpcc::osal::Thread::SchedPolicy::Other, 0U, gpcc::osal::Thread::GetDefaultStackSize());
}

gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF::~gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF(void)
{
  dwqWithThread.Stop();
}

void gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF::SetUp(void)
{
}

void gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF::TearDown(void)
{
}

// alias for death tests
using gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTestsF = gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF;


TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF, CTOR_OK)
{
  std::unique_ptr<gpcc::cood::WorkQueueBasedRemoteAccessServer> spUUT;

  // minimum sizes
  ASSERT_NO_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                         1U,
                                                                                         od,
                                                                                         &logger,
                                                                                         gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                         gpcc::cood::ResponseBase::minimumUsefulResponseSize));

  // maximum sizes
  ASSERT_NO_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                         1U,
                                                                                         od,
                                                                                         &logger,
                                                                                         gpcc::cood::RequestBase::maxRequestSize,
                                                                                         gpcc::cood::ResponseBase::maxResponseSize));

  // no logger
  ASSERT_NO_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                         1U,
                                                                                         od,
                                                                                         nullptr,
                                                                                         gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                         gpcc::cood::ResponseBase::minimumUsefulResponseSize));
}

TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF, CTOR_invalidParams)
{
  std::unique_ptr<gpcc::cood::WorkQueueBasedRemoteAccessServer> spUUT;

  // invalid OOM retry delay
  ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                      0U,
                                                                                      od,
                                                                                      &logger,
                                                                                      gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                      gpcc::cood::ResponseBase::minimumUsefulResponseSize),
               std::invalid_argument);

  // request size too small
  ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                      1U,
                                                                                      od,
                                                                                      &logger,
                                                                                      gpcc::cood::RequestBase::minimumUsefulRequestSize - 1U,
                                                                                      gpcc::cood::ResponseBase::minimumUsefulResponseSize),
               std::invalid_argument);

  // response size too small
  ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                      1U,
                                                                                      od,
                                                                                      &logger,
                                                                                      gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                      gpcc::cood::ResponseBase::minimumUsefulResponseSize - 1U),
               std::invalid_argument);

  // request size too large
  size_t v = gpcc::cood::RequestBase::maxRequestSize + 1U;
  // (on some systems, the addition may overflow)
  if (v != 0U)
  {
    ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                        1U,
                                                                                        od,
                                                                                        &logger,
                                                                                        v,
                                                                                        gpcc::cood::ResponseBase::maxResponseSize),
                 std::invalid_argument);
  }

  // response size too large
  v = gpcc::cood::ResponseBase::maxResponseSize + 1U;
  // (on some systems, the addition may overflow)
  if (v != 0U)
  {
    ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                                        1U,
                                                                                        od,
                                                                                        &logger,
                                                                                        gpcc::cood::RequestBase::maxRequestSize,
                                                                                        v),
                 std::invalid_argument);
  }
}

TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTestsF, DTOR_stillRunning)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                              10U,
                                                                              od,
                                                                              &logger,
                                                                              gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                              gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start();

  EXPECT_DEATH(spUUT.reset(), ".*Still running.*");

  spUUT->Stop();
}

TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTestsF, DTOR_ClientStillRegistered)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  StrictMock<IRemoteObjectDictionaryAccessNotifiableMock> rodanMock;

  auto spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                              10U,
                                                                              od,
                                                                              &logger,
                                                                              gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                              gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Register(&rodanMock);

  EXPECT_DEATH(spUUT.reset(), ".*Client still registered.*");

  spUUT->Unregister();
}

TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF, StartStop)
{
  auto spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                              10U,
                                                                              od,
                                                                              &logger,
                                                                              gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                              gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start();
  spUUT->Stop();
}

TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_TestsF, StartTwice)
{
  auto spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                              10U,
                                                                              od,
                                                                              &logger,
                                                                              gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                              gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start();

  EXPECT_THROW(spUUT->Start(), std::logic_error);

  spUUT->Stop();
}

TEST_F(gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTestsF, StopTwice)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  auto spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                              10U,
                                                                              od,
                                                                              &logger,
                                                                              gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                              gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start();
  spUUT->Stop();

  EXPECT_DEATH(spUUT->Stop(), ".*Not running.*");
}

} // namespace cood
} // namespace gpcc_tests
