/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_OBJECTINFO_HPP_202103012213
#define TESTIRODA_OBJECTINFO_HPP_202103012213

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include "gpcc/src/cood/Object.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ObjectInfoRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ObjectInfoResponse.hpp"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

template <typename T>
using IRODA_ObjectInfoTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_ObjectInfoTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_ObjectInfoTestsF, OK_withASM_withRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a meta data query request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectInfoRequest>(0x1000U, 0U, 255U, true, true, this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectInfoResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectInfoResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Meta data query failed, but it should have succeeded";

  ASSERT_TRUE(response.IsInclusiveNames());
  ASSERT_TRUE(response.IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(response.GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(response.GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(response.IsComplete(nullptr));

  EXPECT_EQ(response.GetObjectCode(), gpcc::cood::Object::ObjectCode::Variable);
  EXPECT_EQ(response.GetObjectDataType(), gpcc::cood::DataType::unsigned32);
  EXPECT_STREQ(response.GetObjectName().c_str(), "Testobject 1");
  EXPECT_EQ(response.GetMaxNbOfSubindices(), 1U);

  EXPECT_FALSE(response.IsSubIndexEmpty(0U));
  EXPECT_EQ(response.GetSubIdxDataType(0U), gpcc::cood::DataType::unsigned32);
  EXPECT_EQ(response.GetSubIdxAttributes(0U), gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR);
  EXPECT_EQ(response.GetSubIdxMaxSize(0U), 32U);
  EXPECT_STREQ(response.GetSubIdxName(0U).c_str(), "Testobject 1");

  ASSERT_EQ(response.GetAppSpecificMetaDataSize(0U), 4U);
  auto const appSpecMetaData = response.GetAppSpecificMetaData(0U);
  ASSERT_EQ(appSpecMetaData.size(), 4U);
  EXPECT_EQ(appSpecMetaData[0], 0xDEU);
  EXPECT_EQ(appSpecMetaData[1], 0xADU);
  EXPECT_EQ(appSpecMetaData[2], 0xBEU);
  EXPECT_EQ(appSpecMetaData[3], 0xEFU);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectInfoTestsF, OK_noASM_withRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a meta data query request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectInfoRequest>(0x1000U, 0U, 255U, true, false, this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectInfoResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectInfoResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Meta data query failed, but it should have succeeded";

  ASSERT_TRUE(response.IsInclusiveNames());
  ASSERT_FALSE(response.IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(response.GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(response.GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(response.IsComplete(nullptr));

  EXPECT_EQ(response.GetObjectCode(), gpcc::cood::Object::ObjectCode::Variable);
  EXPECT_EQ(response.GetObjectDataType(), gpcc::cood::DataType::unsigned32);
  EXPECT_STREQ(response.GetObjectName().c_str(), "Testobject 1");
  EXPECT_EQ(response.GetMaxNbOfSubindices(), 1U);

  EXPECT_FALSE(response.IsSubIndexEmpty(0U));
  EXPECT_EQ(response.GetSubIdxDataType(0U), gpcc::cood::DataType::unsigned32);
  EXPECT_EQ(response.GetSubIdxAttributes(0U), gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR);
  EXPECT_EQ(response.GetSubIdxMaxSize(0U), 32U);
  EXPECT_STREQ(response.GetSubIdxName(0U).c_str(), "Testobject 1");

  EXPECT_THROW((void)response.GetAppSpecificMetaDataSize(0U), std::logic_error);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectInfoTestsF, OK_noRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a meta data query request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectInfoRequest>(0x1000U, 0U, 255U, true, false, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectInfoResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectInfoResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Meta data query failed, but it should have succeeded";

  ASSERT_TRUE(response.IsInclusiveNames());
  ASSERT_FALSE(response.IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(response.GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(response.GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(response.IsComplete(nullptr));

  EXPECT_EQ(response.GetObjectCode(), gpcc::cood::Object::ObjectCode::Variable);
  EXPECT_EQ(response.GetObjectDataType(), gpcc::cood::DataType::unsigned32);
  EXPECT_STREQ(response.GetObjectName().c_str(), "Testobject 1");
  EXPECT_EQ(response.GetMaxNbOfSubindices(), 1U);

  EXPECT_FALSE(response.IsSubIndexEmpty(0U));
  EXPECT_EQ(response.GetSubIdxDataType(0U), gpcc::cood::DataType::unsigned32);
  EXPECT_EQ(response.GetSubIdxAttributes(0U), gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR);
  EXPECT_EQ(response.GetSubIdxMaxSize(0U), 32U);
  EXPECT_STREQ(response.GetSubIdxName(0U).c_str(), "Testobject 1");

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectInfoTestsF, OK_noNames_noRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a meta data query request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectInfoRequest>(0x1000U, 0U, 255U, false, false, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectInfoResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectInfoResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Meta data query failed, but it should have succeeded";

  ASSERT_FALSE(response.IsInclusiveNames());
  ASSERT_FALSE(response.IsInclusiveAppSpecificMetaData());

  EXPECT_EQ(response.GetFirstQueriedSubindex(), 0U);
  EXPECT_EQ(response.GetLastQueriedSubindex(), 0U);
  EXPECT_TRUE(response.IsComplete(nullptr));

  EXPECT_EQ(response.GetObjectCode(), gpcc::cood::Object::ObjectCode::Variable);
  EXPECT_EQ(response.GetObjectDataType(), gpcc::cood::DataType::unsigned32);
  EXPECT_THROW((void)response.GetObjectName(), std::logic_error);
  EXPECT_EQ(response.GetMaxNbOfSubindices(), 1U);

  EXPECT_FALSE(response.IsSubIndexEmpty(0U));
  EXPECT_EQ(response.GetSubIdxDataType(0U), gpcc::cood::DataType::unsigned32);
  EXPECT_EQ(response.GetSubIdxAttributes(0U), gpcc::cood::Object::attr_ACCESS_RD | gpcc::cood::Object::attr_ACCESS_WR);
  EXPECT_EQ(response.GetSubIdxMaxSize(0U), 32U);
  EXPECT_THROW((void)response.GetSubIdxName(0U), std::logic_error);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectInfoTestsF, ErrorObjNotExisting)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a meta data query request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectInfoRequest>(0x0999U, 0U, 255U, true, false, this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectInfoResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectInfoResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::ObjectDoesNotExist);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

REGISTER_TYPED_TEST_SUITE_P(IRODA_ObjectInfoTestsF,
                            OK_withASM_withRSI,
                            OK_noASM_withRSI,
                            OK_noRSI,
                            OK_noNames_noRSI,
                            ErrorObjNotExisting);

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_OBJECTINFO_HPP_202103012213
