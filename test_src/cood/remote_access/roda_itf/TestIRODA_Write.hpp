/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_WRITE_HPP_202008311035
#define TESTIRODA_WRITE_HPP_202008311035

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include <gpcc/cood/Object.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp>
#include <gpcc/osal/MutexLocker.hpp>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::WriteRequest;

template <typename T>
using IRODA_WriteTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_WriteTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_WriteTestsF, OK_singleSubindex_withRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1000U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access failed, but it should have succeeded";

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

TYPED_TEST_P(IRODA_WriteTestsF, OK_singleSubindex_NoRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
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

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access failed, but it should have succeeded";

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

TYPED_TEST_P(IRODA_WriteTestsF, OK_CompleteAccess_8bit_InclSI0_NoRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a CA write request writing to 0x2000, incl. SI0
  std::vector<uint8_t> data = { 0x04U, 0x12U, 0x21U, 0x33U, 0x45U};
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                   0x2000U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access failed, but it should have succeeded";

  // check if the correct data has been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    ASSERT_EQ(this->testbench.GetNbOfSI0x2000(), 5U);
    EXPECT_EQ(this->testbench.data0x2000[0], 0x12U);
    EXPECT_EQ(this->testbench.data0x2000[1], 0x21U);
    EXPECT_EQ(this->testbench.data0x2000[2], 0x33U);
    EXPECT_EQ(this->testbench.data0x2000[3], 0x45U);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, OK_CompleteAccess_8bit_ExclSI0_NoRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // set 0x2000:0 to 4
  this->testbench.Set0x2000_SI0(4U);

  // create a CA write request writing to 0x2000, excl. SI0
  std::vector<uint8_t> data = { 0x12U, 0x21U, 0x33U, 0x45U};
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                   0x2000U, 1U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access failed, but it should have succeeded";

  // check if the correct data has been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    ASSERT_EQ(this->testbench.GetNbOfSI0x2000(), 5U);
    EXPECT_EQ(this->testbench.data0x2000[0], 0x12U);
    EXPECT_EQ(this->testbench.data0x2000[1], 0x21U);
    EXPECT_EQ(this->testbench.data0x2000[2], 0x33U);
    EXPECT_EQ(this->testbench.data0x2000[3], 0x45U);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, OK_CompleteAccess_16bit_InclSI0_NoRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a CA write request writing to 0x2000 incl. SI0
  std::vector<uint8_t> data = { 0x04U, 0x00U, 0x12U, 0x21U, 0x33U, 0x45U};
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                   0x2000U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access failed, but it should have succeeded";

  // check if the correct data has been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    ASSERT_EQ(this->testbench.GetNbOfSI0x2000(), 5U);
    EXPECT_EQ(this->testbench.data0x2000[0], 0x12U);
    EXPECT_EQ(this->testbench.data0x2000[1], 0x21U);
    EXPECT_EQ(this->testbench.data0x2000[2], 0x33U);
    EXPECT_EQ(this->testbench.data0x2000[3], 0x45U);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, OK_CompleteAccess_16bit_ExclSI0_NoRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

// set 0x2000:0 to 4
  this->testbench.Set0x2000_SI0(4U);

  // create a CA write request writing to 0x2000, excl. SI0
  std::vector<uint8_t> data = { 0x12U, 0x21U, 0x33U, 0x45U};
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                   0x2000U, 1U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Write access failed, but it should have succeeded";

  // check if the correct data has been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    ASSERT_EQ(this->testbench.GetNbOfSI0x2000(), 5U);
    EXPECT_EQ(this->testbench.data0x2000[0], 0x12U);
    EXPECT_EQ(this->testbench.data0x2000[1], 0x21U);
    EXPECT_EQ(this->testbench.data0x2000[2], 0x33U);
    EXPECT_EQ(this->testbench.data0x2000[3], 0x45U);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_ObjNotExisting)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x0999:1
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x0999U, 1U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::ObjectDoesNotExist) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_SubindexNotExisting_NotCA)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1000:1
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1000U, 1U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::SubindexDoesNotExist) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_SubindexNotExisting_CA_8bit)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // set 0x2000:0 to 0
  this->testbench.Set0x2000_SI0(0U);

  // create a CA write request writing 0x12 to 0x2000:1
  std::vector<uint8_t> data = { 0x12U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                   0x2000U, 1U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::DataTypeMismatchTooLong) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_SubindexNotExisting_CA_16bit)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // set 0x2000:0 to 0
  this->testbench.Set0x2000_SI0(0U);

  // create a CA write request writing 0x12 to 0x2000:1
  std::vector<uint8_t> data = { 0x12U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_16bit,
                                   0x2000U, 1U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::DataTypeMismatchTooLong) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_ObjectDoesNotSupport_CA)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a CA write request writing 0x12 to 0x1000:0
  std::vector<uint8_t> data = { 0x12U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                   0x1000U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::UnsupportedAccessToObject) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_CallbackRejectsAccess)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1004:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1004U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::GeneralError) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_CallbackThrowsRuntimeError)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1001:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1001U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::GeneralError) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_CallbackThrowsStdBadAlloc)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF into 0x1002:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1002U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OutOfMemory) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_TooMuchData_SingleSubindex)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xDEADBEEF + 1 extra byte into 0x1002:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU, 0xFFU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1002U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::DataTypeMismatchTooLong) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_NotEnoughData_SingleSubindex)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a write request writing 0xADBEEF (24 bit only!) into 0x1002:0
  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::singleSubindex,
                                   0x1002U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::DataTypeMismatchTooSmall) << "Write access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_TooMuchData_CA)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a CA write request writing to 0x2000 incl. SI0, but with one byte more data than required
  std::vector<uint8_t> data = { 0x04U, 0x12U, 0x21U, 0x33U, 0x45U, 0x99U};
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                   0x2000U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::DataTypeMismatchTooLong) << "Write access did not fail as exepected";

  // check that the data has NOT been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    ASSERT_EQ(this->testbench.GetNbOfSI0x2000(), 7U);
    EXPECT_EQ(this->testbench.data0x2000[0], 0U);
    EXPECT_EQ(this->testbench.data0x2000[1], 1U);
    EXPECT_EQ(this->testbench.data0x2000[2], 2U);
    EXPECT_EQ(this->testbench.data0x2000[3], 3U);
    EXPECT_EQ(this->testbench.data0x2000[4], 4U);
    EXPECT_EQ(this->testbench.data0x2000[5], 5U);
    EXPECT_EQ(this->testbench.data0x2000[6], 6U);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_WriteTestsF, Error_NotEnoughData_CA)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a CA write request writing to 0x2000 incl. SI0, but with one byte less data than required
  std::vector<uint8_t> data = { 0x04U, 0x12U, 0x21U, 0x33U };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<WriteRequest>(WriteRequest::AccessType::completeAccess_SI0_8bit,
                                   0x2000U, 0U,
                                   gpcc::cood::Object::attr_ACCESS_WR,
                                   std::move(data),
                                   this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::writeRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::WriteRequestResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::DataTypeMismatchTooSmall) << "Write access did not fail as exepected";

  // check that the data has NOT been written
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    ASSERT_EQ(this->testbench.GetNbOfSI0x2000(), 7U);
    EXPECT_EQ(this->testbench.data0x2000[0], 0U);
    EXPECT_EQ(this->testbench.data0x2000[1], 1U);
    EXPECT_EQ(this->testbench.data0x2000[2], 2U);
    EXPECT_EQ(this->testbench.data0x2000[3], 3U);
    EXPECT_EQ(this->testbench.data0x2000[4], 4U);
    EXPECT_EQ(this->testbench.data0x2000[5], 5U);
    EXPECT_EQ(this->testbench.data0x2000[6], 6U);
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

REGISTER_TYPED_TEST_SUITE_P(IRODA_WriteTestsF,
                            OK_singleSubindex_withRSI,
                            OK_singleSubindex_NoRSI,
                            OK_CompleteAccess_8bit_InclSI0_NoRSI,
                            OK_CompleteAccess_8bit_ExclSI0_NoRSI,
                            OK_CompleteAccess_16bit_InclSI0_NoRSI,
                            OK_CompleteAccess_16bit_ExclSI0_NoRSI,
                            Error_ObjNotExisting,
                            Error_SubindexNotExisting_NotCA,
                            Error_SubindexNotExisting_CA_8bit,
                            Error_SubindexNotExisting_CA_16bit,
                            Error_ObjectDoesNotSupport_CA,
                            Error_CallbackRejectsAccess,
                            Error_CallbackThrowsRuntimeError,
                            Error_CallbackThrowsStdBadAlloc,
                            Error_TooMuchData_SingleSubindex,
                            Error_NotEnoughData_SingleSubindex,
                            Error_TooMuchData_CA,
                            Error_NotEnoughData_CA);

// TODO: Add test case: Invalid permission

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_WRITE_HPP_202008311035
