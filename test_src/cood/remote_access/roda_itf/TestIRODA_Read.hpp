/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_READ_HPP_202010111703
#define TESTIRODA_READ_HPP_202010111703

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include "gpcc/src/cood/Object.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReadRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReadRequestResponse.hpp"
#include <gpcc/osal/MutexLocker.hpp>

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::ReadRequest;

template <typename T>
using IRODA_ReadTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_ReadTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_ReadTestsF, OK_singleSubindex_byteBased_withRSI)
{
  // set the data that will be read
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x1000 = 0xDEADBEEFUL;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1000:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1000U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), 4U * 8U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 4U);
  EXPECT_EQ(data[0], 0xEFU);
  EXPECT_EQ(data[1], 0xBEU);
  EXPECT_EQ(data[2], 0xADU);
  EXPECT_EQ(data[3], 0xDEU);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_singleSubindex_byteBased_NoRSI)
{
  // set the data that will be read
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x1000 = 0xDEADBEEFUL;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1000:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1000U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), 4U * 8U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 4U);
  EXPECT_EQ(data[0], 0xEFU);
  EXPECT_EQ(data[1], 0xBEU);
  EXPECT_EQ(data[2], 0xADU);
  EXPECT_EQ(data[3], 0xDEU);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_singleSubindex_bitBased_0_NoRSI)
{
  // set the data that will be read
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x3000.data_bool = false;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x3000:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x3000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), 1U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 1U);
  EXPECT_EQ(data[0], 0x00U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_singleSubindex_bitBased_1_NoRSI)
{
  // set the data that will be read
  {
    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x3000.data_bool = true;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x3000:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x3000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), 1U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 1U);
  EXPECT_EQ(data[0], 0x01U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_CompleteAccess_8bit_InclSI0_NoRSI)
{
  // set the data that will be read
  {
    this->testbench.Set0x2000_SI0(6U);

    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x2000[0] = 0xFEUL;
    this->testbench.data0x2000[1] = 0x12UL;
    this->testbench.data0x2000[2] = 0x5CUL;
    this->testbench.data0x2000[3] = 0xAAUL;
    this->testbench.data0x2000[4] = 0xC3UL;
    this->testbench.data0x2000[5] = 0x79UL;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading 0x2000 completely
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                  0x2000U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  auto & data = response.GetData();
  ASSERT_EQ(response.GetDataSize(), (1U + 6U) * 8U);
  ASSERT_EQ(data.size(), 7U);
  EXPECT_EQ(data[0], 6U);
  EXPECT_EQ(data[1], 0xFEU);
  EXPECT_EQ(data[2], 0x12U);
  EXPECT_EQ(data[3], 0x5CU);
  EXPECT_EQ(data[4], 0xAAU);
  EXPECT_EQ(data[5], 0xC3U);
  EXPECT_EQ(data[6], 0x79U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_CompleteAccess_8bit_ExclSI0_NoRSI)
{
  // set the data that will be read
  {
    this->testbench.Set0x2000_SI0(6U);

    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x2000[0] = 0xFEUL;
    this->testbench.data0x2000[1] = 0x12UL;
    this->testbench.data0x2000[2] = 0x5CUL;
    this->testbench.data0x2000[3] = 0xAAUL;
    this->testbench.data0x2000[4] = 0xC3UL;
    this->testbench.data0x2000[5] = 0x79UL;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading 0x2000 completely
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                  0x2000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), (0U + 6U) * 8U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 6U);
  EXPECT_EQ(data[0], 0xFEU);
  EXPECT_EQ(data[1], 0x12U);
  EXPECT_EQ(data[2], 0x5CU);
  EXPECT_EQ(data[3], 0xAAU);
  EXPECT_EQ(data[4], 0xC3U);
  EXPECT_EQ(data[5], 0x79U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_CompleteAccess_16bit_InclSI0_NoRSI)
{
  // set the data that will be read
  {
    this->testbench.Set0x2000_SI0(6U);

    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x2000[0] = 0xFEUL;
    this->testbench.data0x2000[1] = 0x12UL;
    this->testbench.data0x2000[2] = 0x5CUL;
    this->testbench.data0x2000[3] = 0xAAUL;
    this->testbench.data0x2000[4] = 0xC3UL;
    this->testbench.data0x2000[5] = 0x79UL;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading 0x2000 completely
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                  0x2000U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), (2U + 6U) * 8U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 8U);
  EXPECT_EQ(data[0], 6U);
  EXPECT_EQ(data[1], 0U);
  EXPECT_EQ(data[2], 0xFEU);
  EXPECT_EQ(data[3], 0x12U);
  EXPECT_EQ(data[4], 0x5CU);
  EXPECT_EQ(data[5], 0xAAU);
  EXPECT_EQ(data[6], 0xC3U);
  EXPECT_EQ(data[7], 0x79U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, OK_CompleteAccess_16bit_ExclSI0_NoRSI)
{
  // set the data that will be read
  {
    this->testbench.Set0x2000_SI0(6U);

    gpcc::osal::MutexLocker ml(this->testbench.dataMutex);
    this->testbench.data0x2000[0] = 0xFEUL;
    this->testbench.data0x2000[1] = 0x12UL;
    this->testbench.data0x2000[2] = 0x5CUL;
    this->testbench.data0x2000[3] = 0xAAUL;
    this->testbench.data0x2000[4] = 0xC3UL;
    this->testbench.data0x2000[5] = 0x79UL;
  }

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading 0x2000 completely
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                  0x2000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  ASSERT_EQ(response.GetDataSize(), (0U + 6U) * 8U);
  auto & data = response.GetData();
  ASSERT_EQ(data.size(), 6U);
  EXPECT_EQ(data[0], 0xFEU);
  EXPECT_EQ(data[1], 0x12U);
  EXPECT_EQ(data[2], 0x5CU);
  EXPECT_EQ(data[3], 0xAAU);
  EXPECT_EQ(data[4], 0xC3U);
  EXPECT_EQ(data[5], 0x79U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_ObjNotExisting)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x0999:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x0999U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::ObjectDoesNotExist) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_SubindexNotExisting_NotCA)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1000:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::SubindexDoesNotExist) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_SubindexNotExisting_CA_8bit)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  this->testbench.Set0x2000_SI0(0U);

  // create a read request reading from 0x1000:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_8bit,
                                  0x2000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::SubindexDoesNotExist) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_SubindexNotExisting_CA_16bit)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  this->testbench.Set0x2000_SI0(0U);

  // create a read request reading from 0x1000:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                  0x2000U, 1U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::SubindexDoesNotExist) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_ObjectDoesNotSupport_CA)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1000:1
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::completeAccess_SI0_16bit,
                                  0x1000U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::UnsupportedAccessToObject) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_CallbackRejectsAccess)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1004:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1004U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::GeneralError) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_CallbackThrowsRuntimeError)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1001:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1001U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::GeneralError) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, Error_CallbackThrowsStdBadAlloc)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a read request reading from 0x1002:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1002U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the read access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OutOfMemory) << "Read access did not fail as exepected";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, ResponseFitsExactly)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  size_t const overhead = this->stdMaxResponseSize_wo_RSI - gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(this->stdMaxResponseSize_wo_RSI, 0U);
  size_t const dataSize = sizeof(TestbenchBase::data0x1003);
  size_t const respSize = dataSize + overhead;
  ASSERT_GE(respSize, gpcc::cood::ResponseBase::minimumUsefulResponseSize) << "Test suite internal error";

  // create a read request reading from 0x1003:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1003U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  respSize);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Read access failed, but it should have succeeded";

  // check the data that has been read
  {
    ASSERT_EQ(response.GetDataSize(), dataSize * 8U);
    auto & data = response.GetData();
    ASSERT_EQ(data.size(), dataSize);
    bool ok = true;
    for (size_t i = 0U; i < dataSize; i++)
    {
      if (data[i] != i)
      {
        ok = false;
        break;
      }
    }

    EXPECT_TRUE(ok) << "Data read from the object does not match the expected value";
  }

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ReadTestsF, ResponseTooLarge)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  size_t const overhead = this->stdMaxResponseSize_wo_RSI - gpcc::cood::ReadRequestResponse::CalcMaxDataPayload(this->stdMaxResponseSize_wo_RSI, 0U);
  size_t const dataSize = sizeof(TestbenchBase::data0x1003);
  size_t const respSize = (dataSize + overhead) - 1U;
  ASSERT_GE(respSize, gpcc::cood::ResponseBase::minimumUsefulResponseSize) << "Test suite internal error";

  // create a read request reading from 0x1003:0
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<ReadRequest>(ReadRequest::AccessType::singleSubindex,
                                  0x1003U, 0U,
                                  gpcc::cood::Object::attr_ACCESS_RD,
                                  respSize);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::readRequestResponse);
  auto & response = dynamic_cast<gpcc::cood::ReadRequestResponse&>(*spResponse);

  // examine the result of the write access
  EXPECT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::ObjectLengthExceedsMbxSize) << "Read access did not failed as expected";

  // check data that no data is contained
  EXPECT_THROW((void)response.GetDataSize(), std::logic_error);
  EXPECT_THROW((void)response.GetData(), std::logic_error);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

// TODO: Add test case: Invalid permission

REGISTER_TYPED_TEST_SUITE_P(IRODA_ReadTestsF,
                            OK_singleSubindex_byteBased_withRSI,
                            OK_singleSubindex_byteBased_NoRSI,
                            OK_singleSubindex_bitBased_0_NoRSI,
                            OK_singleSubindex_bitBased_1_NoRSI,
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
                            ResponseFitsExactly,
                            ResponseTooLarge);

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_READ_HPP_202010111703
