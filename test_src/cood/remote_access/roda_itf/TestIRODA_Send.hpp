/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_SIZES_HPP_202009272049
#define TESTIRODA_SIZES_HPP_202009272049

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include "gpcc/src/cood/Object.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequest.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/exceptions.hpp"
#include <gpcc/osal/MutexLocker.hpp>

namespace gpcc_tests {
namespace cood       {

using namespace testing;

template <typename T>
using IRODA_SendTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_SendTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_SendTestsF, Pass_nullptr)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  std::unique_ptr<gpcc::cood::RequestBase> spRequest;
  ASSERT_THROW(this->uut.Send(spRequest), std::invalid_argument);

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_SendTestsF, CallButNotRegisteredAtRODA)
{
  // create a write request writing 0xDEADBEEF to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  ASSERT_THROW(this->uut.Send(spRequest), std::logic_error);
}

TYPED_TEST_P(IRODA_SendTestsF, AttemptToSendWhileRodaNotReady_A)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));
  this->StopUUT();

  // create a write request writing 0xDEADBEEF to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  ASSERT_THROW(this->uut.Send(spRequest), gpcc::cood::RemoteAccessServerNotReadyError);
}

TYPED_TEST_P(IRODA_SendTestsF, AttemptToSendWhileRodaNotReady_B)
{
  this->StopUUT();
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(false));

  // create a write request writing 0xDEADBEEF to 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  ASSERT_THROW(this->uut.Send(spRequest), gpcc::cood::RemoteAccessServerNotReadyError);
}

TYPED_TEST_P(IRODA_SendTestsF, RequestIsConsumedInCaseOfSuccess)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_THROW(this->uut.Send(spRequest));
  EXPECT_TRUE(spRequest == nullptr) << "Request was not consumed";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_SendTestsF, RequestIsNotConsumedInCaseOfError)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data1 = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data1),
                                               this->stdMaxResponseSize_wo_RSI + 1U);

  ASSERT_THROW(this->uut.Send(spRequest), gpcc::cood::ResponseTooLargeError);
  EXPECT_FALSE(spRequest == nullptr) << "Request was consumed";

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_SendTestsF, MaximumRequestSize)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  size_t const maxDataPayload = gpcc::cood::WriteRequest::CalcMaxDataPayload(this->rodanListener.GetMaxRequestSize(), false);
  ASSERT_NE(maxDataPayload, 0U);

  // create a write request writing "maxDataPayload" bytes into 0x1000:0
  std::vector<uint8_t> data(maxDataPayload);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // examine the result of the write access
  EXPECT_NE(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access suceeded, but it should have failed.";

  // check if the data has not been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    EXPECT_EQ(this->testbench.data0x1000, 0UL) << "Data has been written, but it should have not.";
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_SendTestsF, RequestTooLarge_Idle)
{
  /* This test attempts to send a request which exceeds the maximum request size permitted by the UUT.
     The UUT is idle when the request is send.

     Expected behaviour:
     - Request is rejected
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  size_t const maxDataPayload = gpcc::cood::WriteRequest::CalcMaxDataPayload(this->rodanListener.GetMaxRequestSize(), false);
  ASSERT_NE(maxDataPayload, 0U);

  // create write request writing "maxDataPayload + 1" bytes to 0x1000:0
  std::vector<uint8_t> data(maxDataPayload + 1U);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  ASSERT_THROW(this->uut.Send(spRequest), gpcc::cood::RequestTooLargeError);

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_SendTestsF, RequestTooLarge_Processing)
{
  /* This test attempts to send a request which exceeds the maximum request size permitted by the UUT.
     The UUT is busy with a valid request send before.

     Expected behaviour:
     - 1st request is properly processed
     - 2nd request is rejected
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  size_t const maxDataPayload = gpcc::cood::WriteRequest::CalcMaxDataPayload(this->rodanListener.GetMaxRequestSize(), false);
  ASSERT_NE(maxDataPayload, 0U);

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data1 = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data1),
                                               this->stdMaxResponseSize_wo_RSI);


  // create write request writing "maxDataPayload + 1" bytes to 0x1000:0
  std::vector<uint8_t> data2(maxDataPayload + 1U);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data2),
                                               this->stdMaxResponseSize_wo_RSI);

  this->uut.Send(spRequest1);

  // wait until processing of the first request has started and try to transmit the 2nd request
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetTimeUntilMiddleOfProcessing_ms());
  ASSERT_THROW(this->uut.Send(spRequest2), gpcc::cood::RequestTooLargeError);

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    EXPECT_EQ(this->testbench.data0x1000, 0xDEADBEEFUL);
  }

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_SendTestsF, MinimumResponseSize)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // check if the correct data has been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    EXPECT_EQ(this->testbench.data0x1000, 0xDEADBEEFUL);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_SendTestsF, MaximumResponseSize)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // check if the correct data has been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    EXPECT_EQ(this->testbench.data0x1000, 0xDEADBEEFUL);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_SendTestsF, ResponseTooLarge_Idle)
{
  /* This test attempts to send a request whose "maxResponseSize" attribute exceeds the maximum response size permitted
     by the UUT. The UUT is idle when the request is send.

     Expected behaviour:
     - Request is rejected
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data1 = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data1),
                                               this->stdMaxResponseSize_wo_RSI + 1U);

  ASSERT_THROW(this->uut.Send(spRequest), gpcc::cood::ResponseTooLargeError);

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_SendTestsF, ResponseTooLarge_Processing)
{
  /* This test attempts to send a request whose "maxResponseSize" attribute exceeds the maximum response size permitted
     by the UUT. The UUT is busy with a valid request send before.

     Expected behaviour:
     - 1st request is properly processed
     - 2nd request is rejected
  */

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data1 = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data1),
                                               this->stdMaxResponseSize_wo_RSI);


  // create a write request writing 0x12345678 into 0x1000:0
  std::vector<uint8_t> data2 = { 0x78U, 0x56U, 0x34U, 0x12U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data2),
                                               this->stdMaxResponseSize_wo_RSI + 1U);

  this->uut.Send(spRequest1);

  // wait until processing of the first request has started and try to transmit the 2nd request
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetTimeUntilMiddleOfProcessing_ms());
  ASSERT_THROW(this->uut.Send(spRequest2), gpcc::cood::ResponseTooLargeError);

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(2U * this->testbench.GetResponseTimeout_ms());

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  // check value of 0x1000:0
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    EXPECT_EQ(this->testbench.data0x1000, 0xDEADBEEFUL);
  }

  this->UnregisterFromRODA();
}

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

REGISTER_TYPED_TEST_SUITE_P(IRODA_SendTestsF,
                            Pass_nullptr,
                            CallButNotRegisteredAtRODA,
                            AttemptToSendWhileRodaNotReady_A,
                            AttemptToSendWhileRodaNotReady_B,
                            RequestIsConsumedInCaseOfSuccess,
                            RequestIsNotConsumedInCaseOfError,
                            MaximumRequestSize,
                            RequestTooLarge_Idle,
                            RequestTooLarge_Processing,
                            MinimumResponseSize,
                            MaximumResponseSize,
                            ResponseTooLarge_Idle,
                            ResponseTooLarge_Processing);

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_SIZES_HPP_202009272049
