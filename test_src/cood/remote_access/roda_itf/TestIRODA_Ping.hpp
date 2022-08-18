/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_PING_HPP_202108052254
#define TESTIRODA_PING_HPP_202108052254

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/PingRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/PingResponse.hpp"

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::PingRequest;

template <typename T>
using IRODA_PingTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_PingTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_PingTestsF, OK_withRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a ping request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<PingRequest>(this->stdMaxResponseSize_w_RSI);

  this->CreateAndPushReturnStackItem(*spRequest);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::pingResponse);
  auto & response = dynamic_cast<gpcc::cood::PingResponse&>(*spResponse);
  (void)response;

  ASSERT_NO_FATAL_FAILURE(this->PopCheckAndConsumeReturnStackItem(*spResponse));

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_PingTestsF, OK_NoRSI)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // create a ping request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<PingRequest>(this->stdMaxResponseSize_w_RSI);

  // transmit the request
  ASSERT_NO_FATAL_FAILURE(this->TransmitAndReceive(spRequest));

  // get response, check type and cast to specific type
  auto spResponse = this->rodanListener.PopResponse();
  ASSERT_TRUE(spResponse->GetType() == gpcc::cood::ResponseBase::ResponseTypes::pingResponse);
  auto & response = dynamic_cast<gpcc::cood::PingResponse&>(*spResponse);

  // check that the return stack of the response is empty
  EXPECT_TRUE(response.IsReturnStackEmpty()) << "Nothing pushed on the requests's stack, but the response has a item on its stack.";

  // finally explicitly unregister from RODA-interface
  this->UnregisterFromRODA();

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

REGISTER_TYPED_TEST_SUITE_P(IRODA_PingTestsF,
                            OK_withRSI,
                            OK_NoRSI);


} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_PING_HPP_202108052254
