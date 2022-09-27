/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_OBJECTENUM_HPP_202103161547
#define TESTIRODA_OBJECTENUM_HPP_202103161547

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include <gpcc/cood/Object.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp>

namespace gpcc_tests {
namespace cood       {

using namespace testing;

template <typename T>
using IRODA_ObjectEnumTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_ObjectEnumTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_ObjectEnumTestsF, OK_withRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, 0xFFFFU, this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();
  auto const expectedIndices = this->testbench.EnumerateObjects(0xFFFFU);
  EXPECT_TRUE(indices == expectedIndices);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, OK_noRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, 0xFFFFU, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();
  auto const expectedIndices = this->testbench.EnumerateObjects(0xFFFFU);
  EXPECT_TRUE(indices == expectedIndices);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, OK_wrObjsOnly_noRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, gpcc::cood::Object::attr_ACCESS_WR, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();
  auto const expectedIndices = this->testbench.EnumerateObjects(gpcc::cood::Object::attr_ACCESS_WR);
  EXPECT_TRUE(indices == expectedIndices);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, OK_noRSI_noObjectsInRange)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0xF000U, 0xFFFFU, 0xFFFFU, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();

  EXPECT_EQ(indices.size(), 0U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, OK_noRSI_noObjectsWithSuitableAttributes)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, gpcc::cood::Object::attr_SETTINGS, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();

  EXPECT_EQ(indices.size(), 0U);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, SomeObjectsInRange1)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x1000U, 0x1002U, 0xFFFFU, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();
  std::vector<uint16_t> const expectedIndices = {0x1000U, 0x1001U, 0x1002U};
  EXPECT_TRUE(indices == expectedIndices);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, SomeObjectsInRange2)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a enum request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x1002U, 0x1004U, 0xFFFFU, this->stdMaxResponseSize_wo_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
  auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // examine the result of the meta data query
  ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";
  ASSERT_TRUE(response.IsComplete(nullptr));

  auto const & indices = response.GetIndices();
  std::vector<uint16_t> const expectedIndices = {0x1002U, 0x1003U, 0x1004U};
  EXPECT_TRUE(indices == expectedIndices);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_ObjectEnumTestsF, FragmentedTransfer)
{
  this->testbench.CreateDublicatesOf0x1000(250U);

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // range of indices that shall be enumerated
  uint16_t const firstIndex = 0x0000U;
  uint16_t const lastIndex = 0xFFFFU;

  std::unique_ptr<gpcc::cood::RequestBase> spRequest;
  std::unique_ptr<gpcc::cood::ObjectEnumResponse> spResponse;

  // create initial enum request
  spRequest = std::make_unique<gpcc::cood::ObjectEnumRequest>(firstIndex, lastIndex, 0xFFFFU, gpcc::cood::ResponseBase::minimumUsefulResponseSize);

  size_t loops = 0U;
  do
  {
    ++loops;

    // transmit the request
    ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

    // get response, check type and cast to specific type
    auto spReceivedResponse = this->rodanListener.PopResponse();
    ASSERT_TRUE(spReceivedResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::objectEnumResponse);
    auto & response = dynamic_cast<gpcc::cood::ObjectEnumResponse&>(*spReceivedResponse);

    // examine the result of the meta data query
    ASSERT_EQ(response.GetResult(), gpcc::cood::SDOAbortCode::OK) << "Enum failed, but it should have succeeded";

    // first response?
    if (!spResponse)
    {
      // first response: move to spResponse
      spResponse.reset(&response);
      spReceivedResponse.release();
    }
    else
    {
      // not first: append fragment to *spResponse
      spResponse->AddFragment(response);
    }

    // Done? If yes, break the loop
    uint16_t nextIndex = 0U;
    if (spResponse->IsComplete(&nextIndex))
      break;

    // create next request which continues the query
    spRequest = std::make_unique<gpcc::cood::ObjectEnumRequest>(nextIndex, lastIndex, 0xFFFFU, gpcc::cood::ResponseBase::minimumUsefulResponseSize);
  }
  while (true);

  // check that there were at least two loop cycles because we want to test a fragmented transfer
  EXPECT_GE(loops, 2U) << "Fragmentation did not takt place as expected.";

  // check that defragmented response is OK
  ASSERT_EQ(spResponse->GetResult(), gpcc::cood::SDOAbortCode::OK) << "Defragmented enum response has bad status";

  // check indices
  auto expectedIndices = this->testbench.EnumerateObjects(0xFFFFU);
  auto const & indices = spResponse->GetIndices();
  EXPECT_TRUE(expectedIndices == indices);

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), loops);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

REGISTER_TYPED_TEST_SUITE_P(IRODA_ObjectEnumTestsF,
                            OK_withRSI,
                            OK_noRSI,
                            OK_wrObjsOnly_noRSI,
                            OK_noRSI_noObjectsInRange,
                            OK_noRSI_noObjectsWithSuitableAttributes,
                            SomeObjectsInRange1,
                            SomeObjectsInRange2,
                            FragmentedTransfer);

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_OBJECTENUM_HPP_202103161547
