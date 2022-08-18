/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_REGISTERUNREGISTERSTARTSTOP_HPP_202008311034
#define TESTIRODA_REGISTERUNREGISTERSTARTSTOP_HPP_202008311034

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include "IRemoteObjectDictionaryAccessNotifiableMock.hpp"
#include "gpcc/src/cood/Object.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/exceptions.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Thread.hpp"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

template <typename T>
using IRODA_RegisterUnregisterStartStopTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

template <typename T>
using IRODA_RegisterUnregisterStartStopDeathTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_RegisterUnregisterStartStopTestsF);
TYPED_TEST_SUITE_P(IRODA_RegisterUnregisterStartStopDeathTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, InstantiationStartStop)
{
  EXPECT_TRUE(this->uutNeedsStop);
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StillRegisteredAtEndOfTestcase)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StoppedAtEndOfTestcase)
{
  this->StopUUT();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StillRegisteredAndStoppedAtEndOfTestcase)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));
  this->StopUUT();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, RegisterAndUnregisterWhileRunning)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // check expectation on calls to RODAN
  ASSERT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetMaxRequestSize(), this->testbench.GetExpectedMaxRequestSize());
  EXPECT_EQ(this->rodanListener.GetMaxResponseSize(), this->testbench.GetExpectedMaxResponseSize());
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, RegisterAndUnregisterWhileNotRunning)
{
  this->StopUUT();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(false));
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StartAndStopWhileNotRegistered)
{
  this->StopUUT();
  this->StartUUT();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StartAndStopWhileNotRegisteredTwoCycles)
{
  this->StopUUT();
  this->StartUUT();

  this->StopUUT();
  this->StartUUT();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StartAndStopWhileRegistered)
{
  this->StopUUT();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(false));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->StartUUT();

  // wait and check if ready
  ASSERT_TRUE(this->rodanListener.WaitForStateReady(this->testbench.GetOnReadyTimeout_ms()));

  // check expectation on calls to RODAN
  ASSERT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetMaxRequestSize(), this->testbench.GetExpectedMaxRequestSize());
  EXPECT_EQ(this->rodanListener.GetMaxResponseSize(), this->testbench.GetExpectedMaxResponseSize());
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->StopUUT();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StartAndStopWhileRegisteredTwoCycles)
{
  this->StopUUT();
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(false));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->StartUUT();
  ASSERT_TRUE(this->rodanListener.WaitForStateReady(this->testbench.GetOnReadyTimeout_ms()));
  this->StopUUT();

  this->StartUUT();
  ASSERT_TRUE(this->rodanListener.WaitForStateReady(this->testbench.GetOnReadyTimeout_ms()));
  this->StopUUT();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StartTwice)
{
  EXPECT_THROW(this->StartUUT(), std::logic_error);
  this->StopUUT();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, RegisterWithNullptr)
{
  ASSERT_THROW(this->uut.Register(nullptr), std::invalid_argument);
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, RegisterButAlreadyRegistered)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // attempt to register a mock
  StrictMock<IRemoteObjectDictionaryAccessNotifiableMock> rodanMock;
  EXPECT_THROW(this->uut.Register(&rodanMock), std::logic_error);

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, UnregisterButAlreadyUnregistered)
{
  // test (never registered before)
  this->uut.Unregister();

  // register and unregister from RODA-interface
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));
  this->UnregisterFromRODA();

  // test (at least registered once before)
  this->uut.Unregister();
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, UnregisterWhenRequestTransmittedHalfway)
{
  /* This test checks the behaviour if the client unregisters from the RODA interface when the first of two
     consecutively transmitted requests has travelled half-way from the client to the server.
     The expected behaviour is:
     - None of the requests has been processed
  */

  auto const waittime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waittime == 0U)
  {
    SUCCEED() << "Test skipped (scenario not supported by testbench)";
    return;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until the first request has travelled half-way to the server and then unregister
  gpcc::osal::Thread::Sleep_ms(waittime);
  this->UnregisterFromRODA();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, UnregisterDuringProcessing)
{
  /* This test checks the behaviour if the client unregisters from the RODA interface during processing of the first
     of two consecutively transmitted requests.
     The expected behaviour is:
     - The first request is processed.
     - The second request may be processed.
     - Either no responses are received for any of the two requests, or the response for the first request is received.
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until processing of the first request has started and then unregister
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetTimeUntilMiddleOfProcessing_ms());
  this->UnregisterFromRODA();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      ADD_FAILURE() << "The first request has not been processed. This was not expected.";
    }
    else if (this->testbench.data0x1000 == 0xDEADBEEFUL)
    {
      SUCCEED();
    }
    else if (this->testbench.data0x1000 == 0x78563412UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, UnregisterWhenResponseTransmittedHalfway)
{
  /* This test checks the behaviour if the client unregisters from the RODA interface when the response associated
     with the first of two consecutively transmitted requests has travelled half-way from the server back to the client.
     The expected behaviour is:
     - The first request has be processed.
     - The second request may have been processed.
     - No responses are received for any of the two requests.
  */

  auto const waittime = this->testbench.GetTimeUntilMiddleOfTransmittingResponse_ms();
  if (waittime == 0U)
  {
    SUCCEED() << "Test skipped (scenario not supported by testbench)";
    return;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until the response associated with the first request has travelled half-way from the server back to the
  // client and then unregister
  gpcc::osal::Thread::Sleep_ms(waittime);
  this->UnregisterFromRODA();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      ADD_FAILURE() << "First request has not been executed. This was not expected.";
    }
    else if (this->testbench.data0x1000 == 0xDEADBEEFUL)
    {
      SUCCEED();
    }
    else if (this->testbench.data0x1000 == 0x78563412UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, UnregisterAndRegisterDuringProcessing)
{
  /* This test checks the behaviour if the client unregisters from the RODA interface during processing of the first
     of two consecutively transmitted requests and then registers again immediately.
     The expected behaviour is:
     - The first request is processed.
     - The second request may be processed.
     - Either no responses are received for any of the two requests, or the response for the first request is received.
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until processing of the first request has started and then unregister
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetTimeUntilMiddleOfProcessing_ms());
  this->UnregisterFromRODA();

  // immediately register again at RODA-interface
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      ADD_FAILURE() << "The first request has not been executed. This was not expected.";
    }
    else if (this->testbench.data0x1000 == 0xDEADBEEFUL)
    {
      SUCCEED();
    }
    else if (this->testbench.data0x1000 == 0x78563412UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StopWhenRequestTransmittedHalfway)
{
  /* This test checks the behaviour if the UUT is stopped when the first of two consecutively transmitted requests has
     travelled half-way from the client to the server.
     The expected behaviour is:
     - None of the requests has been processed
  */

  auto const waittime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waittime == 0U)
  {
    SUCCEED() << "Test skipped (scenario not supported by testbench)";
    return;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until the first request has travelled half-way to the server and then stop the UUT
  gpcc::osal::Thread::Sleep_ms(waittime);
  this->StopUUT();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StopDuringProcessing)
{
  /* This test checks the behaviour if the UUT is stopped during processing of the first of two consecutively
     transmitted requests.
     The expected behaviour is:
     - The first request is processed.
     - The second request may be processed.
     - Either no responses are received for any of the two requests, or the response for the first request is received.
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until processing of the first request has started and then stop the UUT
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetTimeUntilMiddleOfProcessing_ms());
  this->StopUUT();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      ADD_FAILURE() << "The first request has not been executed. This was not expected.";
    }
    else if (this->testbench.data0x1000 == 0xDEADBEEFUL)
    {
      SUCCEED();
    }
    else if (this->testbench.data0x1000 == 0x78563412UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StopWhenResponseTransmittedHalfway)
{
  /* This test checks the behaviour if the UUT is stopped when the response associated with the first of two
     consecutively transmitted requests has travelled half-way from the server back to the client.
     The expected behaviour is:
     - The first request has be processed.
     - The second request may have been processed.
     - No responses are received for any of the two requests.
  */

  auto const waittime = this->testbench.GetTimeUntilMiddleOfTransmittingResponse_ms();
  if (waittime == 0U)
  {
    SUCCEED() << "Test skipped (scenario not supported by testbench)";
    return;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until the response associated with the first request has travelled half-way from the server back to the
  // client and then stop the UUT
  gpcc::osal::Thread::Sleep_ms(waittime);
  this->StopUUT();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      ADD_FAILURE() << "First request has not been executed. This was not expected.";
    }
    else if (this->testbench.data0x1000 == 0xDEADBEEFUL)
    {
      SUCCEED();
    }
    else if (this->testbench.data0x1000 == 0x78563412UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopTestsF, StopAndRestartDuringProcessing)
{
  /* This test checks the behaviour if the UUT is stopped during processing of the first of two consecutively
     transmitted requests and then restarted immediately.
     The expected behaviour is:
     - The first request is processed.
     - The second request may be processed.
     - Either no responses are received for any of the two requests, or the response for the first request is received.
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create two write requests writing 0xDEADBEEF and 0x78563412 to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  data = { 0x12U, 0x34U, 0x56U, 0x78U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit both requests
  this->uut.Send(spRequest1);
  this->uut.Send(spRequest2);

  // wait until processing of the first request has started and then stop UUT
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetTimeUntilMiddleOfProcessing_ms());
  this->StopUUT();

  // immediately start again
  this->StartUUT();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);

    if (this->testbench.data0x1000 == 0UL)
    {
      ADD_FAILURE() << "The first request has not been executed. This was not expected.";
    }
    else if (this->testbench.data0x1000 == 0xDEADBEEFUL)
    {
      SUCCEED();
    }
    else if (this->testbench.data0x1000 == 0x78563412UL)
    {
      SUCCEED();
    }
    else
    {
      ADD_FAILURE() << "Target of write requests (0x1000:0) contains unexpected data";
    }
  }
}

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_RegisterUnregisterStartStopDeathTestsF, StopTwice)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  this->StopUUT();
  EXPECT_DEATH(this->StopUUT(), ".*");
}

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

REGISTER_TYPED_TEST_SUITE_P(IRODA_RegisterUnregisterStartStopTestsF,
                            InstantiationStartStop,
                            StillRegisteredAtEndOfTestcase,
                            StoppedAtEndOfTestcase,
                            StillRegisteredAndStoppedAtEndOfTestcase,
                            RegisterAndUnregisterWhileRunning,
                            RegisterAndUnregisterWhileNotRunning,
                            StartAndStopWhileNotRegistered,
                            StartAndStopWhileNotRegisteredTwoCycles,
                            StartAndStopWhileRegistered,
                            StartAndStopWhileRegisteredTwoCycles,
                            StartTwice,
                            RegisterWithNullptr,
                            RegisterButAlreadyRegistered,
                            UnregisterButAlreadyUnregistered,
                            UnregisterWhenRequestTransmittedHalfway,
                            UnregisterDuringProcessing,
                            UnregisterWhenResponseTransmittedHalfway,
                            UnregisterAndRegisterDuringProcessing,
                            StopWhenRequestTransmittedHalfway,
                            StopDuringProcessing,
                            StopWhenResponseTransmittedHalfway,
                            StopAndRestartDuringProcessing);

REGISTER_TYPED_TEST_SUITE_P(IRODA_RegisterUnregisterStartStopDeathTestsF,
                            StopTwice);

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_REGISTERUNREGISTERSTARTSTOP_HPP_202008311034
