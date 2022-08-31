/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_LOANEXECUTIONCONTEXT_HPP_202009251909
#define TESTIRODA_LOANEXECUTIONCONTEXT_HPP_202009251909

#ifndef SKIP_TFC_BASED_TESTS

#include "TestIRODA.hpp"
#include "gpcc/src/cood/Object.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequest.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/exceptions.hpp"
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>

namespace gpcc_tests {
namespace cood       {

using namespace testing;

template <typename T>
using IRODA_LoanExecutionContextTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;

TYPED_TEST_SUITE_P(IRODA_LoanExecutionContextTestsF);

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, OK)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);

  this->uut.RequestExecutionContext();
  gpcc::osal::Thread::Sleep_ms(RODAN_Listener::loanExecContextDuration_ms + 1U);

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 1U);

  this->UnregisterFromRODA();
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, NotRegistered)
{
  ASSERT_THROW(this->uut.RequestExecutionContext(), std::logic_error);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, NotReady_A)
{
  this->StopUUT();
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(false));

  ASSERT_THROW(this->uut.RequestExecutionContext(), std::runtime_error);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, NotReady_B)
{
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));
  this->StopUUT();

  ASSERT_THROW(this->uut.RequestExecutionContext(), std::runtime_error);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, UnregisterWhileRequestPending)
{
  /* This test checks the behaviour if the client unregisters from the RODA interface while there is a outstanding
     request for a call to LoanExecutionContext().
     The test will emit a remote access request and wait until processing of the request is in process. Then it will
     request invocation of LoanExecutionContext() and unregister.
     The expected behaviour is:
     - The remote access request may be processed.
     - There is no call to LoanExecutionContext().
     - A response may be received for the remote access request.
  */

  uint32_t waitTime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waitTime == 0U)
    waitTime = this->testbench.GetTimeUntilMiddleOfProcessing_ms();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  this->uut.Send(spRequest);

  // wait until processing of the request has started and then request execution context
  gpcc::osal::Thread::Sleep_ms(waitTime);
  this->uut.RequestExecutionContext();

  this->UnregisterFromRODA();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetResponseTimeout_ms() + (2U * RODAN_Listener::loanExecContextDuration_ms));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, UnregisterAndRegisterWhileRequestPending)
{
  /* This test checks the behaviour if the client unregisters from the RODA interface and immediately registers
     again while there is a outstanding request for a call to LoanExecutionContext().
     The test will emit a remote access request and wait until processing of the request is in process. Then it will
     request invocation of LoanExecutionContext(), unregister and register.
     The expected behaviour is:
     - The remote access request may be processed.
     - There is no call to LoanExecutionContext().
     - A response may be received for the remote access request.
  */

  uint32_t waitTime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waitTime == 0U)
    waitTime = this->testbench.GetTimeUntilMiddleOfProcessing_ms();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  this->uut.Send(spRequest);

  // wait until processing of the request has started and then request execution context
  gpcc::osal::Thread::Sleep_ms(waitTime);
  this->uut.RequestExecutionContext();

  this->UnregisterFromRODA();
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetResponseTimeout_ms() + (2U * RODAN_Listener::loanExecContextDuration_ms));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, StopWhileRequestPending)
{
  /* This test checks the behaviour if the UUT is stopped while there is a outstanding request for a call to
     LoanExecutionContext().
     The test will emit a remote access request and wait until processing of the request is in process. Then it will
     request invocation of LoanExecutionContext() and stop the UUT.
     The expected behaviour is:
     - The remote access request may be processed.
     - There is no call to LoanExecutionContext().
     - A response may be received for the remote access request.
  */

  uint32_t waitTime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waitTime == 0U)
    waitTime = this->testbench.GetTimeUntilMiddleOfProcessing_ms();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  this->uut.Send(spRequest);

  // wait until processing of the request has started and then request execution context
  gpcc::osal::Thread::Sleep_ms(waitTime);
  this->uut.RequestExecutionContext();

  this->StopUUT();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetResponseTimeout_ms() + (2U * RODAN_Listener::loanExecContextDuration_ms));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, StopAndRestartWhileRequestPending)
{
  /* This test checks the behaviour if the UUT is stopped and immediately restarted while there is a outstanding request
     for a call to LoanExecutionContext().
     The test will emit a remote access request and wait until processing of the request is in process. Then it will
     request invocation of LoanExecutionContext(), stop the UUT, and restart the UUT.
     The expected behaviour is:
     - The remote access request may be processed.
     - There is no call to LoanExecutionContext().
     - A response may be received for the remote access request.
  */

  uint32_t waitTime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waitTime == 0U)
    waitTime = this->testbench.GetTimeUntilMiddleOfProcessing_ms();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  this->uut.Send(spRequest);

  // wait until processing of the request has started and then request execution context
  gpcc::osal::Thread::Sleep_ms(waitTime);
  this->uut.RequestExecutionContext();

  this->StopUUT();
  this->StartUUT();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetResponseTimeout_ms() + (2U * RODAN_Listener::loanExecContextDuration_ms));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 2U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 1U);
  EXPECT_LE(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 0U);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, DoubleRequest)
{
  /* This test checks that multiple requests will result in one call to LoanExecutionContext() only.
  */

  uint32_t waitTime = this->testbench.GetTimeUntilMiddleOfTransmittingRequest_ms();
  if (waitTime == 0U)
    waitTime = this->testbench.GetTimeUntilMiddleOfProcessing_ms();

  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  std::vector<uint8_t> data = { 0xEFU, 0xBEU, 0xADU, 0xDEU };
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::WriteRequest>(gpcc::cood::WriteRequest::AccessType::singleSubindex,
                                               0x1000U, 0U,
                                               gpcc::cood::Object::attr_ACCESS_WR,
                                               std::move(data),
                                               this->stdMaxResponseSize_wo_RSI);

  this->uut.Send(spRequest);

  // wait until processing of the request has started and then request execution context
  gpcc::osal::Thread::Sleep_ms(waitTime);
  this->uut.RequestExecutionContext();
  this->uut.RequestExecutionContext();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetResponseTimeout_ms() + (3U * RODAN_Listener::loanExecContextDuration_ms));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 1U);
}

TYPED_TEST_P(IRODA_LoanExecutionContextTestsF, RequestFromLoanExecutionContext)
{
  /* This test case will request 3 calls to LoanExecutionContext().
     Two requests will be issued from within LoanExecutionContext().
  */
  ASSERT_NO_FATAL_FAILURE(this->RegisterAtRODA(true));

  uint8_t n = 2U;
  auto callback = [&]()
  {
    if (n != 0U)
    {
      n--;
      this->uut.RequestExecutionContext();
    }
  };

  this->rodanListener.SetOnLoanExecutionContext(callback);
  ON_SCOPE_EXIT() { this->rodanListener.SetOnLoanExecutionContext(nullptr); };

  this->uut.RequestExecutionContext();

  // wait until all requests have been processed for sure
  gpcc::osal::Thread::Sleep_ms(this->testbench.GetResponseTimeout_ms() + (4U * RODAN_Listener::loanExecContextDuration_ms));

  // check expectation on calls to RODAN
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnReady(), 1U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnDisconnected(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsOnRequestProcessed(), 0U);
  EXPECT_EQ(this->rodanListener.GetNbOfCallsLoanExecutionContext(), 3U);
}

// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

REGISTER_TYPED_TEST_SUITE_P(IRODA_LoanExecutionContextTestsF,
                            OK,
                            NotRegistered,
                            NotReady_A,
                            NotReady_B,
                            UnregisterWhileRequestPending,
                            UnregisterAndRegisterWhileRequestPending,
                            StopWhileRequestPending,
                            StopAndRestartWhileRequestPending,
                            DoubleRequest,
                            RequestFromLoanExecutionContext);

} // namespace gpcc_tests
} // namespace cood

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_LOANEXECUTIONCONTEXT_HPP_202009251909
