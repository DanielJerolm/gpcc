/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "TestbenchWorkQueueBasedRAS.hpp"
#include <gpcc/cood/remote_access/infrastructure/WorkQueueBasedRemoteAccessServer.hpp>
#include "test_src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiableMock.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_LoanExecutionContext.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_ObjectEnum.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_ObjectInfo.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Ping.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Read.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_RegisterUnregisterStartStop.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Send.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Write.hpp"
#include "test_src/execution/async/DWQwithThread.hpp"

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

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_Tests, CTOR_OK)
{
  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

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

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_Tests, CTOR_invalidParams)
{
  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

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

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTests, DTOR_stillRunning)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

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

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTests, DTOR_ClientStillRegistered)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;
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

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_Tests, StartStop)
{
  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  auto spUUT = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwqWithThread.GetDWQ(),
                                                                              10U,
                                                                              od,
                                                                              &logger,
                                                                              gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                              gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start();
  spUUT->Stop();
}

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_Tests, StartTwice)
{
  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

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

TEST(gpcc_cood_WorkQueueBasedRemoteAccessServer_DeathTests, StopTwice)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  gpcc_tests::execution::async::DWQwithThread dwqWithThread("DWQThread");
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

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
