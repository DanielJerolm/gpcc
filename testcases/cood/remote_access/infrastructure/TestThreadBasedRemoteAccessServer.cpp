/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "TestbenchThreadBasedRAS.hpp"
#include <gpcc/cood/remote_access/infrastructure/ThreadBasedRemoteAccessServer.hpp>
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

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_LoanExecutionContextTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_ObjectEnumTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_ObjectInfoTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_PingTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_ReadTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_RegisterUnregisterStartStopTestsF, TestbenchThreadBasedRAS);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_RegisterUnregisterStartStopDeathTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_SendTestsF, TestbenchThreadBasedRAS);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_ThreadBasedRemoteAccessServer_, IRODA_WriteTestsF, TestbenchThreadBasedRAS);

#endif

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_Tests, CTOR_OK)
{
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  std::unique_ptr<gpcc::cood::ThreadBasedRemoteAccessServer> spUUT;

  // minimum sizes
  ASSERT_NO_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
                                                                                      1U,
                                                                                      od,
                                                                                      &logger,
                                                                                      gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                      gpcc::cood::ResponseBase::minimumUsefulResponseSize));

  // maximum sizes
  ASSERT_NO_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
                                                                                      1U,
                                                                                      od,
                                                                                      &logger,
                                                                                      gpcc::cood::RequestBase::maxRequestSize,
                                                                                      gpcc::cood::ResponseBase::maxResponseSize));

  // no logger
  ASSERT_NO_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
                                                                                      1U,
                                                                                      od,
                                                                                      nullptr,
                                                                                      gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                      gpcc::cood::ResponseBase::minimumUsefulResponseSize));
}

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_Tests, CTOR_invalidParams)
{
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  std::unique_ptr<gpcc::cood::ThreadBasedRemoteAccessServer> spUUT;

  // invalid OOM retry delay
  ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
                                                                                   0U,
                                                                                   od,
                                                                                   &logger,
                                                                                   gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                                   gpcc::cood::ResponseBase::minimumUsefulResponseSize),
               std::invalid_argument);

  // request size too small
  ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
                                                                                   1U,
                                                                                   od,
                                                                                   &logger,
                                                                                   gpcc::cood::RequestBase::minimumUsefulRequestSize - 1U,
                                                                                   gpcc::cood::ResponseBase::minimumUsefulResponseSize),
               std::invalid_argument);

  // response size too small
  ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
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
    ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
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
    ASSERT_THROW(spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("TN",
                                                                                     1U,
                                                                                     od,
                                                                                     &logger,
                                                                                     gpcc::cood::RequestBase::maxRequestSize,
                                                                                     v),
                 std::invalid_argument);
  }
}

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_DeathTests, DTOR_stillRunning)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  auto spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("UUT",
                                                                           10U,
                                                                           od,
                                                                           &logger,
                                                                           gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                           gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());

  EXPECT_DEATH(spUUT.reset(), ".*Still running.*");

  spUUT->Stop();
}

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_DeathTests, DTOR_ClientStillRegistered)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;
  StrictMock<IRemoteObjectDictionaryAccessNotifiableMock> rodanMock;

  auto spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("UUT",
                                                                           10U,
                                                                           od,
                                                                           &logger,
                                                                           gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                           gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Register(&rodanMock);

  EXPECT_DEATH(spUUT.reset(), ".*Client still registered.*");

  spUUT->Unregister();
}

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_Tests, StartStop)
{
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  auto spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("UUT",
                                                                           10U,
                                                                           od,
                                                                           &logger,
                                                                           gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                           gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  spUUT->Stop();
}

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_Tests, StartTwice)
{
  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  auto spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("UUT",
                                                                           10U,
                                                                           od,
                                                                           &logger,
                                                                           gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                           gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());

  EXPECT_THROW(spUUT->Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize()), std::logic_error);

  spUUT->Stop();
}

TEST(gpcc_cood_ThreadBasedRemoteAccessServer_DeathTests, StopTwice)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  gpcc::log::Logger logger("Test");
  gpcc::cood::ObjectDictionary od;

  auto spUUT = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("UUT",
                                                                           10U,
                                                                           od,
                                                                           &logger,
                                                                           gpcc::cood::RequestBase::minimumUsefulRequestSize,
                                                                           gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  spUUT->Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
  spUUT->Stop();

  EXPECT_DEATH(spUUT->Stop(), ".*Not running.*");
}

} // namespace cood
} // namespace gpcc_tests
