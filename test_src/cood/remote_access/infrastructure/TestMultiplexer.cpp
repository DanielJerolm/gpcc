/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "gpcc/src/cood/remote_access/infrastructure/Multiplexer.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/PingRequest.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/PingResponse.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/exceptions.hpp"
#include "gpcc/src/cood/sdo_abort_codes.hpp"
#include "gpcc/src/execution/async/DeferredWorkPackage.hpp"
#include "gpcc/src/execution/async/WorkPackage.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessMock.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiableMock.hpp"
#include "gpcc/test_src/execution/async/DWQwithThread.hpp"

#include "gtest/gtest.h"
#include <memory>
#include <stdexcept>
#include <vector>

namespace {
  class InjectedError final : public std::runtime_error
  {
    public:
      InjectedError(void) : std::runtime_error("Injected") {};
      ~InjectedError(void) = default;
  };
}

namespace gpcc_tests {
namespace cood       {

using namespace testing;
using gpcc::cood::Multiplexer;
using gpcc::cood::MultiplexerPort;
using gpcc::cood::IRemoteObjectDictionaryAccess;
using gpcc::cood::IRemoteObjectDictionaryAccessNotifiable;
using gpcc::cood::ReturnStackItem;
using gpcc::cood::RequestBase;
using gpcc::cood::ResponseBase;
using gpcc::execution::async::WorkPackage;

// Test fixture for class gpcc::cood::Multiplexer
// It provides a work queue, a mock for the unit providing a RODA/RODAN pair to the mux and two mocks for clients
// connected to the multiplexer.
class gpcc_cood_Multiplexer_TestsF: public Test
{
  public:
    gpcc_cood_Multiplexer_TestsF(void);
    ~gpcc_cood_Multiplexer_TestsF(void);

  protected:
    // Properties of server passed to UUT upon invocation of OnReady(...)
    static size_t constexpr maxRequestSizeSupportedByServer = 500U;
    static size_t constexpr maxResponseSizeSupportedByServer = 600U;

    // UUT's return stack item: Maximum number of session IDs (= 2^n with n being the bit-width of the sessionID)
    static uint32_t constexpr maxSessionIDs = 256U;

    // UUT's return stack item: Mask and offset for field "sessionID"
    static uint32_t constexpr mask_sessionID = 0x000000FFUL;
    static uint8_t constexpr offset_sessionID = 0U;

    // UUT's return stack item: Mask and offset for field "gap"
    static uint32_t constexpr mask_gap = 0x007FFF00UL;
    static uint8_t constexpr offset_gap = 8U;

    // UUT's return stack item: Mask for myPing-bit
    static uint32_t constexpr mask_myPing = 0x00800000UL;
    static uint8_t constexpr offset_myPing = 23U;

    // UUT's return stack item: Mask and offset for field "index"
    static uint32_t constexpr mask_index = 0xFF000000UL;
    static uint8_t constexpr offset_index = 24U;


    // Workqueue + thread
    gpcc_tests::execution::async::DWQwithThread dwqWithThread;
    gpcc::execution::async::IDeferredWorkQueue & dwq;

    // mocks for server and 2 clients
    StrictMock<IRemoteObjectDictionaryAccessMock> serverItf;
    StrictMock<IRemoteObjectDictionaryAccessNotifiableMock> clientItf1;
    StrictMock<IRemoteObjectDictionaryAccessNotifiableMock> clientItf2;

    // UUT
    std::unique_ptr<Multiplexer> spUUT;

    // UUT's RODAN interface registered at the server by the UUT
    IRemoteObjectDictionaryAccessNotifiable * pRODAN_of_Mux;


    void SetUp(void) override;
    void TearDown(void) override;

    void ConnectMuxToServer(void);
    void DisconnectMuxFromServer(void);
    void ServerInvokesOnReady(void);
    void ServerInvokesOnDisconnected(void);
    void ServerInvokesOnReadyAndOnDisconnected(void);
    void ServerInvokesLoanExecCtxtAndOnDisconnectedAndOnReady(void);
    void ServerInvokesLoanExecCtxt(void);

    void ProcessRequests(std::vector<std::unique_ptr<RequestBase>> & reqs);
};

gpcc_cood_Multiplexer_TestsF::gpcc_cood_Multiplexer_TestsF(void)
: Test()
, dwqWithThread("gpcc_cood_Multiplexer_TestsF")
, dwq(dwqWithThread.GetDWQ())
, serverItf()
, clientItf1()
, clientItf2()
, spUUT()
, pRODAN_of_Mux(nullptr)
{
}

gpcc_cood_Multiplexer_TestsF::~gpcc_cood_Multiplexer_TestsF(void)
{
  try
  {
    pRODAN_of_Mux = nullptr;
    spUUT.reset();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("gpcc_cood_Multiplexer_TestsF::~gpcc_cood_Multiplexer_TestsF: Caught an exception:", e);
  }
  catch (...)
  {
    gpcc::osal::Panic("gpcc_cood_Multiplexer_TestsF::~gpcc_cood_Multiplexer_TestsF: Caught an unknown exception");
  }
}

void gpcc_cood_Multiplexer_TestsF::SetUp(void)
{
  spUUT = std::make_unique<Multiplexer>();
}

void gpcc_cood_Multiplexer_TestsF::TearDown(void)
{
  if (spUUT != nullptr)
  {
    spUUT->Disconnect();
    pRODAN_of_Mux = nullptr;
    spUUT.reset();
  }
}

/**
 * \brief Connects the UUT (mux) to the server.
 *
 * Basically, this invokes Multiplexer::Connect(...).
 *
 * \pre   The multiplexer is not yet connected to a server.
 *
 * \post  The multiplexer is connected to the server.
 *
 * \post  The multiplexer's RODAN interface is stored in @ref pRODAN_of_Mux
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Expectations are not reverted
 * - Connect() may have been invoked.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::ConnectMuxToServer(void)
{
  // ===========================================
  // Use ASSERT_NO_FATAL_FAILURE to invoke this!
  // ===========================================

  EXPECT_CALL(serverItf, Register(_)).Times(1).WillOnce(SaveArg<0>(&pRODAN_of_Mux));
  spUUT->Connect(serverItf);
  ASSERT_TRUE(pRODAN_of_Mux != nullptr);
  Mock::VerifyAndClearExpectations(&serverItf);
}

/**
 * \brief Disconnects the UUT (mux) from the server.
 *
 * This will not clear @ref pRODAN_of_Mux.
 *
 * \pre    The multiplexer is connected to the server.
 *
 * \post   The multiplexer is disconnected from the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Call to Multipler::Disconnect() is not undone.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::DisconnectMuxFromServer(void)
{
  // ===========================================
  // Use ASSERT_NO_FATAL_FAILURE to invoke this!
  // ===========================================

  EXPECT_CALL(serverItf, Unregister()).Times(1);
  spUUT->Disconnect();
  Mock::VerifyAndClearExpectations(&serverItf);
}

/**
 * \brief Invokes OnReady() at UUT's RODAN interface via the workqueue.
 *
 * This blocks until OnReady() has been invoked.
 *
 * @ref maxRequestSizeSupportedByServer and @ref maxResponseSizeSupportedByServer are passed as parameters.
 *
 * \pre   The UUT is connected to the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Work package may be enqueued but not executed yet.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::ServerInvokesOnReady(void)
{
  if (pRODAN_of_Mux == nullptr)
    throw std::logic_error("gpcc_cood_Multiplexer_TestsF::ServerInvokesOnReady: pRodan_of_Mux is nullptr");

  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(maxRequestSizeSupportedByServer, maxResponseSizeSupportedByServer); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.FlushNonDeferredWorkPackages();
}

/**
 * \brief Invokes OnDisconnected() at UUT's RODAN interface via the workqueue.
 *
 * This blocks until OnDisconnected() has been invoked.
 *
 * \pre   The UUT is connected to the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Work package may be enqueued but not executed yet.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::ServerInvokesOnDisconnected(void)
{
  if (pRODAN_of_Mux == nullptr)
    throw std::logic_error("gpcc_cood_Multiplexer_TestsF::ServerInvokesOnDisconnected: pRodan_of_Mux is nullptr");

  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));
  dwq.FlushNonDeferredWorkPackages();
}

/**
 * \brief Invokes OnReady() and OnDisconnected() at UUT's RODAN interface via the workqueue.
 *
 * This blocks until OnReady() and OnDisconnected() have been invoked.
 *
 * @ref maxRequestSizeSupportedByServer and @ref maxResponseSizeSupportedByServer are passed as parameters.
 *
 * \pre   The UUT is connected to the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Work package may be enqueued but not executed yet.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::ServerInvokesOnReadyAndOnDisconnected(void)
{
  if (pRODAN_of_Mux == nullptr)
    throw std::logic_error("gpcc_cood_Multiplexer_TestsF::ServerInvokesOnReadyAndOnDisconnected: pRodan_of_Mux is nullptr");

  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(maxRequestSizeSupportedByServer, maxResponseSizeSupportedByServer); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));
  dwq.FlushNonDeferredWorkPackages();
}

/**
 * \brief Invokes LoanExecutionContext(), OnDisconnected() and OnReady() at UUT's RODAN interface via the workqueue.
 *
 * This blocks until all 3 calls have completed.
 *
 * @ref maxRequestSizeSupportedByServer and @ref maxResponseSizeSupportedByServer are passed as parameters.
 *
 * \pre   The UUT is connected to the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Work package may be enqueued but not executed yet.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::ServerInvokesLoanExecCtxtAndOnDisconnectedAndOnReady(void)
{
  if (pRODAN_of_Mux == nullptr)
    throw std::logic_error("gpcc_cood_Multiplexer_TestsF::ServerInvokesLoanExecCtxtAndOnDisconnectedAndOnReady: pRodan_of_Mux is nullptr");

  auto invokeLoanExecContext = [&]() { pRODAN_of_Mux->LoanExecutionContext(); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(maxRequestSizeSupportedByServer, maxResponseSizeSupportedByServer); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeLoanExecContext));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.FlushNonDeferredWorkPackages();
}

/**
 * \brief Invokes LoanExecutionContext() at UUT's RODAN interface via the workqueue.
 *
 * This blocks until the call has completed.
 *
 * \pre   The UUT is connected to the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Work package may be enqueued but not executed yet.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
void gpcc_cood_Multiplexer_TestsF::ServerInvokesLoanExecCtxt(void)
{
  if (pRODAN_of_Mux == nullptr)
    throw std::logic_error("gpcc_cood_Multiplexer_TestsF::ServerInvokesLoanExecCtxt: pRodan_of_Mux is nullptr");

  auto invokeLoanExecContext = [&]() { pRODAN_of_Mux->LoanExecutionContext(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeLoanExecContext));
  dwq.FlushNonDeferredWorkPackages();
}

/**
 * \brief Processes requests and sends a response for each request. Pings are properly processed.
 *
 * \pre   The UUT is connected to the server.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is intended to be invoked by the test case's thread only.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Not all requests may have been processed
 * - The return item stack of the currently processed request may be lost
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \param reqs
 * Reference to a vector containing requests.\n
 * The return stack is consumed from each request.\n
 * For each `PingRequest` a `PingResponse` is transmitted back to the client.\n
 * For all other requests, a `ObjectEnumResponse` indicating a `general error` is send back to the client.
 */
void gpcc_cood_Multiplexer_TestsF::ProcessRequests(std::vector<std::unique_ptr<RequestBase>> & reqs)
{
  for (auto & req : reqs)
  {
    std::unique_ptr<gpcc::cood::ResponseBase> spResponse;

    if (typeid(*req) == typeid(gpcc::cood::PingRequest))
    {
      spResponse = std::make_unique<gpcc::cood::PingResponse>();
    }
    else
    {
      spResponse = std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);
    }

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }
}

typedef gpcc_cood_Multiplexer_TestsF gpcc_cood_Multiplexer_DeathTestsF;

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, InstantiateAndDestroy)
{
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Connection and disconnection of multiplexer to/from a server's provided RODA interface in different scenarios.
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, Mux_ConnectAndDisconnect_WhileServerOff)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());
  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Mux_DisconnectWhileServerOn)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Mux_ConnectTwice_WhileServerOff)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // attempt to connect to another server
  StrictMock<IRemoteObjectDictionaryAccessMock> serverItf2;
  ASSERT_THROW(spUUT->Connect(serverItf2), std::logic_error);

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Mux_DisconnectTwice)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());
  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());

  // attempt to disconnect a second time
  EXPECT_NO_THROW(spUUT->Disconnect());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Mux_DisconnectButNeverConnected)
{
  EXPECT_NO_THROW(spUUT->Disconnect());
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, Mux_CallToRODANafterDisconnected)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());
  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());

  std::unique_ptr<gpcc::cood::ResponseBase> spDummyResp =
    std::make_unique<gpcc::cood::WriteRequestResponse>(gpcc::cood::SDOAbortCode::OK);

  auto stimulus1 = [&]() { pRODAN_of_Mux->OnReady(maxRequestSizeSupportedByServer, maxResponseSizeSupportedByServer); };
  auto stimulus2 = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spDummyResp)); };
  auto stimulus3 = [&]() { pRODAN_of_Mux->LoanExecutionContext(); };
  auto stimulus4 = [&]() { pRODAN_of_Mux->OnDisconnected(); };

  ON_SCOPE_EXIT()
  {
    dwq.Remove(this);
    dwq.WaitUntilCurrentWorkPackageHasBeenExecuted(this);
  };

  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus1)); dwq.FlushNonDeferredWorkPackages();}, ".*Not connected to any RODA interface.*");
  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus2)); dwq.FlushNonDeferredWorkPackages();}, ".*Not connected to any RODA interface.*");
  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus3)); dwq.FlushNonDeferredWorkPackages();}, ".*Not connected to any RODA interface.*");
  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus4)); dwq.FlushNonDeferredWorkPackages();}, ".*Not connected to any RODA interface.*");
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Mux_Disconnect_WhileClientRegistered_WhileSeverOn)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client 1 at port
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // server delivers OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer  - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // disconnect mux from server
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
    EXPECT_CALL(serverItf, Unregister()).Times(1);
  }

  spUUT->Disconnect();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);

  // disconnect client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Server becomes "ready" and "not-ready" ======================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, ServerOnOff_NoPorts)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReadyAndOnDisconnected();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, ServerOnOff_Twice_NoPorts)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReadyAndOnDisconnected();
  ServerInvokesOnReadyAndOnDisconnected();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, ServerOnOff_WhileClientRegistered)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  ServerInvokesOnReadyAndOnDisconnected();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, ServerOffOn_WhileClientRegistered)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext, OnDisconnected, OnReady
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  }

  ServerInvokesLoanExecCtxtAndOnDisconnectedAndOnReady();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, Mux_CallToRODANwhileServerOff)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  std::unique_ptr<gpcc::cood::ResponseBase> spDummyResp =
    std::make_unique<gpcc::cood::WriteRequestResponse>(gpcc::cood::SDOAbortCode::OK);

  auto stimulus1 = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spDummyResp)); };
  auto stimulus2 = [&]() { pRODAN_of_Mux->LoanExecutionContext(); };
  auto stimulus3 = [&]() { pRODAN_of_Mux->OnDisconnected(); };

  ON_SCOPE_EXIT()
  {
    dwq.Remove(this);
    dwq.WaitUntilCurrentWorkPackageHasBeenExecuted(this);
  };

  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus1)); dwq.FlushNonDeferredWorkPackages();}, ".*Unexpected call, RODA interface is 'not ready'.*");
  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus2)); dwq.FlushNonDeferredWorkPackages();}, ".*Unexpected call, RODA interface is 'not ready'.*");
  EXPECT_DEATH({dwq.Add(WorkPackage::CreateDynamic(this, 0U, stimulus3)); dwq.FlushNonDeferredWorkPackages();}, ".*Already disconnected / not ready.*");

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Creating and discarding ports ===============================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, CreatePort_WhileNotConnected)
{
  std::vector<std::shared_ptr<MultiplexerPort>> ports;
  ports.reserve(Multiplexer::maxNbOfPorts);

  for (size_t i = 0U; i < Multiplexer::maxNbOfPorts; ++i)
  {
    ASSERT_NO_THROW(ports.emplace_back(spUUT->CreatePort())) << "Could not create the expected number of ports!";
    ASSERT_TRUE(ports.back() != nullptr);
  }

  std::shared_ptr<MultiplexerPort> anotherPort;
  ASSERT_THROW(anotherPort = spUUT->CreatePort(), std::runtime_error);

  // drop one port
  ports.front().reset();

  EXPECT_NO_THROW(anotherPort = spUUT->CreatePort()) << "Could not create another port after dropping one";
  EXPECT_TRUE(anotherPort != nullptr);
}

TEST_F(gpcc_cood_Multiplexer_TestsF, CreatePort_WhileConnected_ServerOff)
{
   ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create ports
  std::vector<std::shared_ptr<MultiplexerPort>> ports;
  ports.reserve(Multiplexer::maxNbOfPorts);

  for (size_t i = 0U; i < Multiplexer::maxNbOfPorts; ++i)
  {
    ASSERT_NO_THROW(ports.emplace_back(spUUT->CreatePort())) << "Could not create the expected number of ports!";
    ASSERT_TRUE(ports.back() != nullptr);
  }

  std::shared_ptr<MultiplexerPort> anotherPort;
  ASSERT_THROW(anotherPort = spUUT->CreatePort(), std::runtime_error);

  // drop one port
  ports.front().reset();

  EXPECT_NO_THROW(anotherPort = spUUT->CreatePort()) << "Could not create another port after dropping one";
  EXPECT_TRUE(anotherPort != nullptr);

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, CreatePort_WhileConnected_ServerOn)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  // create ports
  std::vector<std::shared_ptr<MultiplexerPort>> ports;
  ports.reserve(Multiplexer::maxNbOfPorts);

  for (size_t i = 0U; i < Multiplexer::maxNbOfPorts; ++i)
  {
    ASSERT_NO_THROW(ports.emplace_back(spUUT->CreatePort())) << "Could not create the expected number of ports!";
    ASSERT_TRUE(ports.back() != nullptr);
  }

  std::shared_ptr<MultiplexerPort> anotherPort;
  ASSERT_THROW(anotherPort = spUUT->CreatePort(), std::runtime_error);

  // drop one port
  ports.front().reset();

  EXPECT_NO_THROW(anotherPort = spUUT->CreatePort()) << "Could not create another port after dropping one";
  EXPECT_TRUE(anotherPort != nullptr);

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, DropPort_ClientStillReg_ThenDestroyMux)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);

  // drop the port and destroy UUT
  EXPECT_DEATH({ spPort.reset(); spUUT.reset(); }, ".*Client still registered.*");

  // unregister client and drop the port
  pPortRODA->Unregister();
  spPort.reset();
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, DropPort_ClientStillReg_ThenCreateNewPort)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);

  // drop the port and attempt to create a new one
  EXPECT_DEATH( { spPort.reset(); spPort = spUUT->CreatePort(); }, ".*Dropped port has still a RODAN interface registered.*");

  // unregister client
  pPortRODA->Unregister();
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Port_CallsToRODANRejected_WhileNoClientReg_WhileDisconnected)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  std::unique_ptr<gpcc::cood::RequestBase> spDummyReq =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, 0xFFFFU, 256U);

  ASSERT_THROW(pPortRODA->Send(spDummyReq), std::logic_error);
  ASSERT_THROW(pPortRODA->RequestExecutionContext(), std::logic_error);
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Port_CallsToRODANRejected_WhileClientReg_WhileDisconnected)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT() { pPortRODA->Unregister(); };

  std::unique_ptr<gpcc::cood::RequestBase> spDummyReq =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, 0xFFFFU, 256U);

  ASSERT_THROW(pPortRODA->Send(spDummyReq), gpcc::cood::RemoteAccessServerNotReadyError);
  ASSERT_THROW(pPortRODA->RequestExecutionContext(), gpcc::cood::RemoteAccessServerNotReadyError);
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Port_CallsToRODANRejected_WhileClientReg_WhileServerOff)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT() { pPortRODA->Unregister(); };

  std::unique_ptr<gpcc::cood::RequestBase> spDummyReq =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0xFFFFU, 0xFFFFU, 256U);

  ASSERT_THROW(pPortRODA->Send(spDummyReq), gpcc::cood::RemoteAccessServerNotReadyError);
  ASSERT_THROW(pPortRODA->RequestExecutionContext(), gpcc::cood::RemoteAccessServerNotReadyError);

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Destruction of multiplexer ==================================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, Mux_Destroy_StillConnected_ServerOff)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  EXPECT_DEATH(spUUT.reset(), ".*Still connected to a RODA interface.*");

  if (spUUT)
  {
    ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
  }
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, Mux_Destroy_StillConnected_ServerOn)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  EXPECT_DEATH(spUUT.reset(), ".*Still connected to a RODA interface.*");

  if (spUUT)
  {
    ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
  }
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, Mux_Destroy_PortStillInUse)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  // create a port
  std::shared_ptr<MultiplexerPort> somePort = spUUT->CreatePort();
  ASSERT_TRUE(somePort != nullptr);

  EXPECT_DEATH(spUUT.reset(), ".*Port still referenced by someone.*");
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Client registration and unregistration ======================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndUnreg_WhileMuxNotConnected)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register and unregister client
  EXPECT_FALSE(spPort->IsClientRegistered());
  pPortRODA->Register(&clientItf1);
  EXPECT_TRUE(spPort->IsClientRegistered());
  pPortRODA->Unregister();
  EXPECT_FALSE(spPort->IsClientRegistered());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndUnreg_WhileMuxNotConnected_Twice)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);

  // attemt to register another client at the same port
  EXPECT_THROW(pPortRODA->Register(&clientItf2), std::logic_error);

  // unregister client
  pPortRODA->Unregister();
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_Unreg_Twice)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);

  // unregister client
  pPortRODA->Unregister();

  // unregister (2nd call)
  EXPECT_NO_THROW(pPortRODA->Unregister());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_Unreg_ButNeverReg)
{
  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // unregister though never registered
  EXPECT_NO_THROW(pPortRODA->Unregister());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndUnreg_WhileMuxConnected_ServerOff)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register and unregister client
  pPortRODA->Register(&clientItf1);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndUnregTwice_WhileMuxConnected_ServerOff)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register and unregister client (2 times)
  pPortRODA->Register(&clientItf1);
  pPortRODA->Unregister();
  pPortRODA->Register(&clientItf1);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndUnreg_WhileMuxConnected_ServerOn)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndImmediateUnreg_WhileMuxConnected_ServerOn)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client 1 at port and unregister immediately
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->Register(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);
  pPortRODA->Unregister();

  // server provides execution context (which is not needed any more)
  ServerInvokesLoanExecCtxt();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Client_RegAndImmediateUnreg_ThenRegAgain_WhileMuxConnected_ServerOn)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client 1 at port and unregister immediately
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->Register(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);
  pPortRODA->Unregister();

  // register again
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };
  Mock::VerifyAndClearExpectations(&serverItf);

  // server provides execution context
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // disconnect from server
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
    EXPECT_CALL(serverItf, Unregister()).Times(1);
  }
  spUUT->Disconnect();
  Mock::VerifyAndClearExpectations(&serverItf);
  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client 1
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Loan execution context ======================================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ClientRequestsExecutionContext_OK)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ClientRequestsExecutionContextTwice_A)
{
  // There are two variants of this test case:
  // A: Without second spurious call to LoanExecutionContext
  // B: With second spurious call to LoanExecutionContext

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context twice
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(2);
  pPortRODA->RequestExecutionContext();
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // This is variant A: No second (spurious) call to LoanExecutionContext

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ClientRequestsExecutionContextTwice_B)
{
  // There are two variants of this test case:
  // A: Without second spurious call to LoanExecutionContext
  // B: With second spurious call to LoanExecutionContext

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context twice
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(2);
  pPortRODA->RequestExecutionContext();
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // This is variant B:
  // server invokes LoanExecutionContext (spurious 2nd call)
  ServerInvokesLoanExecCtxt();

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_RequestFromLoanExecutionContext)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // Server invokes LoanExecutionContext.
  // Client requests execution context from within the callback.
  auto invokeRequestExecContext = [&]() { pPortRODA->RequestExecutionContext(); };
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1).WillOnce(InvokeWithoutArgs(invokeRequestExecContext));
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext to serve the 2nd request
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // server invokes LoanExecutionContext (spurious 3rd call)
  ServerInvokesLoanExecCtxt();

  // client unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_RequestFromOnReady_A)
{
  // There are two variants of this test case:
  // A: Without second spurious call to LoanExecutionContext
  // B: With second spurious call to LoanExecutionContext

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // Server invokes OnReady
  // Client requests execution context from within OnReady
  auto invokeRequestExecContext = [&]() { pPortRODA->RequestExecutionContext(); };
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1).WillOnce(InvokeWithoutArgs(invokeRequestExecContext));
    EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  }
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // This is variant A: No spurious second call to LoanExecutionContext

  // client unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_RequestFromOnReady_B)
{
  // There are two variants of this test case:
  // A: Without second spurious call to LoanExecutionContext
  // B: With second spurious call to LoanExecutionContext

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // Server invokes OnReady
  // Client requests execution context from within OnReady
  auto invokeRequestExecContext = [&]() { pPortRODA->RequestExecutionContext(); };
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1).WillOnce(InvokeWithoutArgs(invokeRequestExecContext));
    EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  }
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // This is variant B: Server invokes LoanExecutionContext (spurious 2nd call)
  ServerInvokesLoanExecCtxt();

  // client unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_TwoClients_OneRequestsExecContext)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create 2 ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA1 = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA2 = spPort2.get();

  // register client 1
  pPortRODA1->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPortRODA1->Unregister(); };

  // register client 2
  pPortRODA2->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPortRODA2->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // client #1 requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA1->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client #2 requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA2->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf2, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf2);

  // unregister client #2
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPortRODA2->Unregister();

  // unregister client #1
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPortRODA1->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_TwoClients_BothRequestsExecContext)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create 2 ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA1 = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA2 = spPort2.get();

  // register client 1
  pPortRODA1->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPortRODA1->Unregister(); };

  // register client 2
  pPortRODA2->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPortRODA2->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // client #1 and #2 both request execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(2);
  pPortRODA1->RequestExecutionContext();
  pPortRODA2->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, LoanExecutionContext()).Times(1);
  EXPECT_CALL(clientItf2, LoanExecutionContext()).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // unregister client #2
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPortRODA2->Unregister();

  // unregister client #1
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPortRODA1->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_SpuriousCall_NoPorts)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  ServerInvokesOnReady();

  // server invokes LoanExecutionContext (spurious call)
  ServerInvokesLoanExecCtxt();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_SpuriousCall_OnePortWithClient)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // get a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client 1 at port
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT() { pPortRODA->Unregister(); };

  // server delivers OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // server delivers LoanExecContext (spurious call)
  ServerInvokesLoanExecCtxt();

  // server delivers OnDisconnected
  EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  ServerInvokesOnDisconnected();
  Mock::VerifyAndClearExpectations(&clientItf1);

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ServerNotReady_A)
{
  // There are two variants of this test case:
  // A: Server was never ready
  // B: Server becomes not ready just before client issues the request

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // client attempts to request execution context
  EXPECT_THROW(pPortRODA->RequestExecutionContext(), gpcc::cood::RemoteAccessServerNotReadyError);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ServerNotReady_B)
{
  // There are two variants of this test case:
  // A: Server was never ready
  // B: Server becomes not ready just before client issues the request

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // Varaint B: server delivers OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  ServerInvokesOnReadyAndOnDisconnected();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client attempts to request execution context just after server became not-ready
  EXPECT_THROW(pPortRODA->RequestExecutionContext(), gpcc::cood::RemoteAccessServerNotReadyError);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ServerOffWhileRequestPending)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes OnDisconnected
  EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  ServerInvokesOnDisconnected();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ServerOffOnWhileRequestPending)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes OnDisconnected
  EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  ServerInvokesOnDisconnected();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // server delivers LoanExecContext (spurious call)
  ServerInvokesLoanExecCtxt();

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_DeathTestsF, LoanExecContext_SpuriousCallAfterServerOffWhileRequestPending)
{
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes OnDisconnected
  EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  ServerInvokesOnDisconnected();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // server invokes LoanExecutionContext (leathal)
  auto invokeLoanExecContext = [&]() { pRODAN_of_Mux->LoanExecutionContext(); };
  EXPECT_DEATH({ dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeLoanExecContext)); dwq.FlushNonDeferredWorkPackages(); }, ".*Unexpected call, RODA interface is 'not ready'.*");

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ServerOffBeforeRequest)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // server invokes OnDisconnected
  EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  ServerInvokesOnDisconnected();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client attempts to request execution context
  EXPECT_THROW(pPortRODA->RequestExecutionContext(), gpcc::cood::RemoteAccessServerNotReadyError);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_MuxDisconnectWhileRequestPending)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // disconnect mux from server
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
    EXPECT_CALL(serverItf, Unregister()).Times(1);
  }
  spUUT->Disconnect();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&serverItf);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ClientDisconnectsWhileRequestPending)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // client unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  // server invokes LoanExecutionContext
  ServerInvokesLoanExecCtxt();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, LoanExecContext_ClientDisconnectsAndAnotherConnectsWhileRequestPending)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client 1
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client 1 requests execution context
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->RequestExecutionContext();
  Mock::VerifyAndClearExpectations(&serverItf);

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPortRODA->Unregister();

  // register client 2
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  pPortRODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPortRODA->Unregister(); };
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf2);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Message transmission and reception ==========================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_PassNullptrToSend)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // client invokes Send(...) with nullptr
  std::unique_ptr<RequestBase> spReq;
  EXPECT_THROW(pPortRODA->Send(spReq), std::invalid_argument);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ServerSendThrowsBadAlloc)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                  maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // create a request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // client sends the prepared request
  EXPECT_CALL(serverItf, Send(_)).Times(1).WillOnce(Throw(std::bad_alloc()));

  ASSERT_THROW(pPortRODA->Send(spRequest), std::bad_alloc);
  EXPECT_TRUE(spRequest != nullptr) << "The request object has been consumed, but it should have not!";

  Mock::VerifyAndClearExpectations(&serverItf);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_OK)
{
  // container for requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // container for responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };

  // container for responses received by client #2
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient2;
  auto receiveClient2 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    { rxClient2.emplace_back(std::move(spResp)); };


  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create three requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf2, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient2);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }

  ProcessRequests(mux2server);

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // examine response received by client 2
  ASSERT_TRUE(rxClient2.size() == 1U);
  ASSERT_FALSE(rxClient2[0]->IsReturnStackEmpty());
  rsi = rxClient2[0]->PopReturnStack();
  EXPECT_TRUE(rxClient2[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 2U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ResponseHasEmptyReturnStack)
{
  // container for requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // container for responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };


  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create three requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }

  for (auto & req : mux2server)
  {
    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);

    ASSERT_EQ(v.size(), 2U);
    if (v[0].GetID() != 2U)
      spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ResponseAddressedToSomeoneElse)
{
  // container for requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // container for responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };


  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create three requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }

  for (auto & req : mux2server)
  {
    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);

    ASSERT_EQ(v.size(), 2U);
    if (v[0].GetID() == 2U)
    {
      v[1] = ReturnStackItem(~(v[1].GetID()), v[1].GetInfo());
    }
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ResponseHasInvalidPortID)
{
  // container for requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // container for responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };


  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create three requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }

  for (auto & req : mux2server)
  {
    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);

    ASSERT_EQ(v.size(), 2U);
    if (v[0].GetID() == 2U)
    {
      v[1] = ReturnStackItem(v[1].GetID(), v[1].GetInfo() | mask_index);
    }
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ResponseInvalidGap)
{
  // container for requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // container for responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };


  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create three requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }

  for (auto & req : mux2server)
  {
    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);

    ASSERT_EQ(v.size(), 2U);
    if (v[0].GetID() == 2U)
    {
      v[1] = ReturnStackItem(v[1].GetID(), v[1].GetInfo() | mask_gap);
    }
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ResponseInvalidSessionID)
{
  // container for requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // container for responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };


  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create three requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }

  for (auto & req : mux2server)
  {
    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);

    ASSERT_EQ(v.size(), 2U);
    if (v[0].GetID() == 2U)
    {
      v[1] = ReturnStackItem(v[1].GetID(), v[1].GetInfo() ^ mask_sessionID);
    }
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ClientDisconnectsBeforeReceivingResponse)
{
  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // Responses received by client #2
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient2;
  auto receiveClient2 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    { rxClient2.emplace_back(std::move(spResp)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create two requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // client 1 disconnects
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  // create responses
  EXPECT_CALL(clientItf2, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient2);
  ProcessRequests(mux2server);
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine response received by client 2
  ASSERT_TRUE(rxClient2.size() == 1U);
  ASSERT_FALSE(rxClient2[0]->IsReturnStackEmpty());
  auto rsi = rxClient2[0]->PopReturnStack();
  EXPECT_TRUE(rxClient2[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 2U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ClientDisconnectsAndDropsPortBeforeReceivingResponse)
{
  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // Responses received by client #2
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient2;
  auto receiveClient2 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    { rxClient2.emplace_back(std::move(spResp)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create two requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // client 1 disconnects and drops port
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();
  pPort1RODA = nullptr;
  spPort1.reset();

  // create responses
  EXPECT_CALL(clientItf2, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient2);
  ProcessRequests(mux2server);
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine response received by client 2
  ASSERT_TRUE(rxClient2.size() == 1U);
  ASSERT_FALSE(rxClient2[0]->IsReturnStackEmpty());
  auto rsi = rxClient2[0]->PopReturnStack();
  EXPECT_TRUE(rxClient2[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 2U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ClientUnregistersAndRegisters)
{
  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // Responses received by client #2
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient2;
  auto receiveClient2 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    { rxClient2.emplace_back(std::move(spResp)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1_a) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create two requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // client 1 disconnects
  ON_SCOPE_EXIT_DISMISS(unregClient1_a);
  pPort1RODA->Unregister();

  // client 1 reconnects
  EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
  EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1_b) { pPort1RODA->Unregister(); };
  Mock::VerifyAndClearExpectations(&serverItf);

  // server invokes LoanExecutionContext
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesLoanExecCtxt();
  Mock::VerifyAndClearExpectations(&clientItf1);

  // create responses
  EXPECT_CALL(clientItf2, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient2);
  ProcessRequests(mux2server);
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine response received by client 2
  ASSERT_TRUE(rxClient2.size() == 1U);
  ASSERT_FALSE(rxClient2[0]->IsReturnStackEmpty());
  auto rsi = rxClient2[0]->PopReturnStack();
  EXPECT_TRUE(rxClient2[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 2U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  ON_SCOPE_EXIT_DISMISS(unregClient1_b);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_MaxMessageSizesBothZero)
{
  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // Responses received by client #1
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient1;
  auto receiveClient1 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    {  rxClient1.emplace_back(std::move(spResp)); };

  // Responses received by client #2
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient2;
  auto receiveClient2 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    { rxClient2.emplace_back(std::move(spResp)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(0U, 0U)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(0U, 0U)).Times(1);

  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(0U, 0U); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  // create two requests
  std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x2000U, 0x3000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);
  std::unique_ptr<gpcc::cood::RequestBase> spRequest3 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x4000U, 0x5000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use ReturnStackItems to identify them later
  spRequest1->Push(ReturnStackItem(1U, 0U));
  spRequest2->Push(ReturnStackItem(2U, 0U));
  spRequest3->Push(ReturnStackItem(1U, 1U));

  // clients send the prepared requests
  EXPECT_CALL(serverItf, Send(_)).Times(3).WillRepeatedly(Invoke(server_Send));

  pPort1RODA->Send(spRequest1);
  EXPECT_TRUE(spRequest1 == nullptr);
  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);
  pPort1RODA->Send(spRequest3);
  EXPECT_TRUE(spRequest3 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_TRUE(mux2server.size() == 3U);

  // create responses
  {
    InSequence s;

    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
    EXPECT_CALL(clientItf2, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient2);
    EXPECT_CALL(clientItf1, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient1);
  }
  ProcessRequests(mux2server);
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine responses received by client 1
  ASSERT_TRUE(rxClient1.size() == 2U);
  ASSERT_FALSE(rxClient1[0]->IsReturnStackEmpty());
  auto rsi = rxClient1[0]->PopReturnStack();
  EXPECT_TRUE(rxClient1[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  ASSERT_FALSE(rxClient1[1]->IsReturnStackEmpty());
  rsi = rxClient1[1]->PopReturnStack();
  EXPECT_TRUE(rxClient1[1]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 1U);
  EXPECT_EQ(rsi.GetInfo(), 1U);

  // examine response received by client 2
  ASSERT_TRUE(rxClient2.size() == 1U);
  ASSERT_FALSE(rxClient2[0]->IsReturnStackEmpty());
  rsi = rxClient2[0]->PopReturnStack();
  EXPECT_TRUE(rxClient2[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 2U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  // client 1 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient1);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_SessionIDsExpired)
{
  static_assert(maxSessionIDs > 50U, "Switch-case for injection of errors below will not work");

  // This test case forces the UUT to increment the session ID at one port. At the same time, the ping messages send by
  // the port in order to flush the connection to the master are dropped or invalidated in various ways. Due to the
  // failed pings, old (used) session IDs cannot be reused and the UUT's port will finally run out of session IDs.
  // After the port ran out of session IDs, transmission of messages via a second port is tested. The second port
  // should not be affected by the issue of the first port.

  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  // Responses received by client #2
  std::vector<std::unique_ptr<gpcc::cood::ResponseBase>> rxClient2;
  auto receiveClient2 = [&](std::unique_ptr<gpcc::cood::ResponseBase> spResp)
    { rxClient2.emplace_back(std::move(spResp)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1_a) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------
  // Client 1 wears out all its session IDs.
  // -----------------------------------------------------------
  for (uint_fast32_t i = 0U; i < maxSessionIDs; i++)
  {
    // create a request
    std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
      std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

    // client1 sends the prepared request
    EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));

    pPort1RODA->Send(spRequest1);
    EXPECT_TRUE(spRequest1 == nullptr);

    Mock::VerifyAndClearExpectations(&serverItf);
    ASSERT_TRUE(mux2server.size() == 1U);

    // client 1 disconnects
    pPort1RODA->Unregister();

    if (i == maxSessionIDs - 1U)
    {
      // client 1 attempts to reconnect (expected next session ID is 0, but 0 should be the oldest used session ID)
      ASSERT_THROW(pPort1RODA->Register(&clientItf1), std::runtime_error)
        << "Register() should have failed due to no unused session ID available.";
      break;
    }
    else
    {
      // client 1 reconnects (expected session ID is i+1)
      EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
      EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));
      ASSERT_NO_THROW(pPort1RODA->Register(&clientItf1)) << "Failed in loop cycle " << i;
      Mock::VerifyAndClearExpectations(&serverItf);
    }

    // server invokes LoanExecutionContext
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
    ServerInvokesLoanExecCtxt();
    Mock::VerifyAndClearExpectations(&clientItf1);

    // create responses (ALL ping responses are dropped!)
    for (auto & req : mux2server)
    {
      std::unique_ptr<gpcc::cood::ResponseBase> spResponse;
      std::vector<ReturnStackItem> v;
      req->ExtractReturnStack(v);

      if (typeid(*req) == typeid(gpcc::cood::PingRequest))
      {
        // drop the ping using various modifications
        switch (i)
        {
          case maxSessionIDs - 50U:
            // Invalidate ID
            ASSERT_EQ(v.size(), 1U);
            v[0] = ReturnStackItem(~(v[0].GetID()), v[0].GetInfo());
            spResponse = std::make_unique<gpcc::cood::PingResponse>();
            break;

          case maxSessionIDs - 49U:
            // Invalidate "myping" and "sessionID"
            ASSERT_EQ(v.size(), 1U);
            v[0] = ReturnStackItem(v[0].GetID(), v[0].GetInfo() ^ (mask_myPing | mask_sessionID));
            spResponse = std::make_unique<gpcc::cood::PingResponse>();
            break;

          case maxSessionIDs - 48U:
            // Invalidate "gap"
            ASSERT_EQ(v.size(), 1U);
            v[0] = ReturnStackItem(v[0].GetID(), v[0].GetInfo() ^ mask_gap);
            spResponse = std::make_unique<gpcc::cood::PingResponse>();
            break;

          case maxSessionIDs - 47U:
            // Invalidate "index"
            ASSERT_EQ(v.size(), 1U);
            v[0] = ReturnStackItem(v[0].GetID(), v[0].GetInfo() ^ mask_index);
            spResponse = std::make_unique<gpcc::cood::PingResponse>();
            break;

          case maxSessionIDs - 46U:
            // Invalidate "sessionID"
            ASSERT_EQ(v.size(), 1U);
            v[0] = ReturnStackItem(v[0].GetID(), v[0].GetInfo() ^ mask_sessionID);
            spResponse = std::make_unique<gpcc::cood::PingResponse>();
            break;

          case maxSessionIDs - 45U:
            // Push an unexpected item on the return stack
            ASSERT_EQ(v.size(), 1U);
            v.insert(v.begin(), ReturnStackItem(34U, 43U));
            spResponse = std::make_unique<gpcc::cood::PingResponse>();
            break;

          default:
            // Just drop the Ping
            continue;
        }
      }
      else
      {
         spResponse = std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);
      }

      spResponse->SetReturnStack(std::move(v));

      auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
      dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
      dwq.FlushNonDeferredWorkPackages();
    }
    mux2server.clear();

    Mock::VerifyAndClearExpectations(&clientItf1);
    Mock::VerifyAndClearExpectations(&clientItf2);
  }

  ASSERT_EQ(mux2server.size(), 1U)
    << "There should be exactly one message: The ObjectEnumRequest send at the beginning of the for-loop.";

  // create responses (ALL ping responses are dropped!)
  for (auto & req : mux2server)
  {
    if (typeid(*req) == typeid(gpcc::cood::PingRequest))
      continue;

    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }
  mux2server.clear();

  // -----------------------------------------------------------
  // Check that client 2 can transmit and receive
  // -----------------------------------------------------------

  // create a request
  std::unique_ptr<gpcc::cood::RequestBase> spRequest2 =
    std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

  // use a ReturnStackItem to identify ir later
  spRequest2->Push(ReturnStackItem(2U, 0U));

  // client2 sends the prepared request
  EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));

  pPort2RODA->Send(spRequest2);
  EXPECT_TRUE(spRequest2 == nullptr);

  Mock::VerifyAndClearExpectations(&serverItf);
  ASSERT_EQ(mux2server.size(), 1U);

  EXPECT_CALL(clientItf2, OnRequestProcessed(_)).Times(1).WillOnce(receiveClient2);

  // create responses
  for (auto & req : mux2server)
  {
    ASSERT_FALSE(typeid(*req) == typeid(gpcc::cood::PingRequest))
      << "The mux send or forwarded a ping request. This was not anticipated!";

    std::unique_ptr<gpcc::cood::ResponseBase> spResponse =
      std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);

    std::vector<ReturnStackItem> v;
    req->ExtractReturnStack(v);
    spResponse->SetReturnStack(std::move(v));

    auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
    dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
    dwq.FlushNonDeferredWorkPackages();
  }
  mux2server.clear();

  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // examine response received by client 2
  ASSERT_EQ(rxClient2.size(), 1U);
  ASSERT_FALSE(rxClient2[0]->IsReturnStackEmpty());
  auto rsi = rxClient2[0]->PopReturnStack();
  EXPECT_TRUE(rxClient2[0]->IsReturnStackEmpty());
  EXPECT_EQ(rsi.GetID(), 2U);
  EXPECT_EQ(rsi.GetInfo(), 0U);

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  ON_SCOPE_EXIT_DISMISS(unregClient1_a);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_PingRecoversSessionID)
{
  // This test case forces the UUT to increment the session ID at one port. At the same time, the ping messages send by
  // the port in order to flush the connection to the master are dropped. Due to the failed pings, old (used) session
  // IDs cannot be reused and the UUT's port will finally run out of session IDs. BUT BEFORE this happens, the last
  // ping is not dropped and the UUT's port will reuse the old session IDs.

  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1_a) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  for (uint_fast32_t i = 0U; i < maxSessionIDs + 2U; i++)
  {
    // create a request
    std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
      std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

    // client1 sends the prepared request
    EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));

    pPort1RODA->Send(spRequest1);
    EXPECT_TRUE(spRequest1 == nullptr);

    Mock::VerifyAndClearExpectations(&serverItf);
    ASSERT_TRUE(mux2server.size() == 1U);

    // client 1 disconnects
    pPort1RODA->Unregister();

    // client 1 reconnects (expected session ID is i+1)
    EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
    EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));
    ASSERT_NO_THROW(pPort1RODA->Register(&clientItf1)) << "Failed in loop cycle " << i;
    Mock::VerifyAndClearExpectations(&serverItf);

    // server invokes LoanExecutionContext
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
    ServerInvokesLoanExecCtxt();
    Mock::VerifyAndClearExpectations(&clientItf1);

    // create responses
    // Pings are dropped except for loop cycle maxSessionIDs - 2U (session ID = maxSessionIDs - 1U)
    for (auto & req : mux2server)
    {
      std::unique_ptr<gpcc::cood::ResponseBase> spResponse;

      std::vector<ReturnStackItem> v;
      req->ExtractReturnStack(v);

      if (typeid(*req) == typeid(gpcc::cood::PingRequest))
      {
        ASSERT_EQ(v.size(), 1U);
        ASSERT_EQ((v[0].GetInfo() & mask_sessionID) >> offset_sessionID, (i + 1U) % maxSessionIDs) << "Unexpected session ID";

        if (i != maxSessionIDs - 2U)
          continue;

        spResponse = std::make_unique<gpcc::cood::PingResponse>();
      }
      else
      {
        spResponse = std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);
      }

      spResponse->SetReturnStack(std::move(v));

      auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
      dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
      dwq.FlushNonDeferredWorkPackages();
    }
    mux2server.clear();

    Mock::VerifyAndClearExpectations(&clientItf1);
    Mock::VerifyAndClearExpectations(&clientItf2);
  }

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  ON_SCOPE_EXIT_DISMISS(unregClient1_a);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, TxRx_ServerThrowUponClientRegistrationDoesNotWearSessionID)
{
  static_assert(maxSessionIDs > 10U, "For-loop below will not work");

  // Requests passed from mux to server
  std::vector<std::unique_ptr<gpcc::cood::RequestBase>> mux2server;
  auto server_Send = [&](std::unique_ptr<gpcc::cood::RequestBase> & spRequest)
    { mux2server.emplace_back(std::move(spRequest)); };

  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create two ports
  auto spPort1 = spUUT->CreatePort();
  ASSERT_TRUE(spPort1 != nullptr);
  IRemoteObjectDictionaryAccess * pPort1RODA = spPort1.get();

  auto spPort2 = spUUT->CreatePort();
  ASSERT_TRUE(spPort2 != nullptr);
  IRemoteObjectDictionaryAccess * pPort2RODA = spPort2.get();

  // register clients
  pPort1RODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient1_a) { pPort1RODA->Unregister(); };

  pPort2RODA->Register(&clientItf2);
  ON_SCOPE_EXIT(unregClient2) { pPort2RODA->Unregister(); };

  // server invokes OnReady
  EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  EXPECT_CALL(clientItf2, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize, maxResponseSizeSupportedByServer  - ReturnStackItem::binarySize)).Times(1);
  ServerInvokesOnReady();
  Mock::VerifyAndClearExpectations(&clientItf1);
  Mock::VerifyAndClearExpectations(&clientItf2);

  // -----------------------------------------------------------

  for (uint_fast32_t i = 0U; i < 10U; i++)
  {
    // create a request
    std::unique_ptr<gpcc::cood::RequestBase> spRequest1 =
      std::make_unique<gpcc::cood::ObjectEnumRequest>(0x0000U, 0x1000U, 0xFFFFU, maxResponseSizeSupportedByServer - ReturnStackItem::binarySize);

    // client1 sends the prepared request
    EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));

    pPort1RODA->Send(spRequest1);
    EXPECT_TRUE(spRequest1 == nullptr);

    Mock::VerifyAndClearExpectations(&serverItf);
    ASSERT_TRUE(mux2server.size() == 1U);

    // client 1 disconnects
    pPort1RODA->Unregister();

    // insert some attempts to reconnect, but they fail because server throws
    switch (i)
    {
      case 3U:
      {
        EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1).WillOnce(Throw(InjectedError()));
        ASSERT_THROW(pPort1RODA->Register(&clientItf1), InjectedError);
        Mock::VerifyAndClearExpectations(&serverItf);
        break;
      }

      case 5U:
      {
        EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
        EXPECT_CALL(serverItf, Send(_)).Times(1).WillOnce(Throw(InjectedError()));
        ASSERT_THROW(pPort1RODA->Register(&clientItf1), InjectedError);
        Mock::VerifyAndClearExpectations(&serverItf);
        break;
      }
    }

    // client 1 reconnects (expected session ID is i+1)
    EXPECT_CALL(serverItf, RequestExecutionContext()).Times(1);
    EXPECT_CALL(serverItf, Send(_)).Times(1).WillRepeatedly(Invoke(server_Send));
    ASSERT_NO_THROW(pPort1RODA->Register(&clientItf1)) << "Failed in loop cycle " << i;
    Mock::VerifyAndClearExpectations(&serverItf);

    // server invokes LoanExecutionContext
    EXPECT_CALL(clientItf1, OnReady(maxRequestSizeSupportedByServer - ReturnStackItem::binarySize,
                                    maxResponseSizeSupportedByServer - ReturnStackItem::binarySize)).Times(1);
    ServerInvokesLoanExecCtxt();
    Mock::VerifyAndClearExpectations(&clientItf1);

    // create responses (all pings are dropped)
    for (auto & req : mux2server)
    {
      std::unique_ptr<gpcc::cood::ResponseBase> spResponse;

      std::vector<ReturnStackItem> v;
      req->ExtractReturnStack(v);

      if (typeid(*req) == typeid(gpcc::cood::PingRequest))
      {
        ASSERT_EQ(v.size(), 1U);
        ASSERT_EQ((v[0].GetInfo() & mask_sessionID) >> offset_sessionID, (i + 1U) % maxSessionIDs) << "Unexpected session ID";
        continue;
      }
      else
      {
        spResponse = std::make_unique<gpcc::cood::ObjectEnumResponse>(gpcc::cood::SDOAbortCode::GeneralError);
      }

      spResponse->SetReturnStack(std::move(v));

      auto invokeOnRequestProcessed = [&]() { pRODAN_of_Mux->OnRequestProcessed(std::move(spResponse)); };
      dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnRequestProcessed));
      dwq.FlushNonDeferredWorkPackages();
    }
    mux2server.clear();

    Mock::VerifyAndClearExpectations(&clientItf1);
    Mock::VerifyAndClearExpectations(&clientItf2);
  }

  // client 2 unregisters
  ON_SCOPE_EXIT_DISMISS(unregClient2);
  pPort2RODA->Unregister();

  ON_SCOPE_EXIT_DISMISS(unregClient1_a);
  pPort1RODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================
//
// TESTS: Different maximum message sizes =============================================================================
//
// ====================================================================================================================
// ====================================================================================================================
// ====================================================================================================================

TEST_F(gpcc_cood_Multiplexer_TestsF, Sizes_BothMaximum)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(RequestBase::maxRequestSize - ReturnStackItem::binarySize,
                                    ResponseBase::maxResponseSize - ReturnStackItem::binarySize)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(RequestBase::maxRequestSize, ResponseBase::maxResponseSize); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  ON_SCOPE_EXIT(flushWQ) { dwq.FlushNonDeferredWorkPackages(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));

  ON_SCOPE_EXIT_DISMISS(flushWQ);
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Sizes_BothMinumum)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(RequestBase::minimumUsefulRequestSize, ResponseBase::minimumUsefulResponseSize)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(RequestBase::minimumUsefulRequestSize + ReturnStackItem::binarySize,
                                                      ResponseBase::minimumUsefulResponseSize +  ReturnStackItem::binarySize); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  ON_SCOPE_EXIT(flushWQ) { dwq.FlushNonDeferredWorkPackages(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));

  ON_SCOPE_EXIT_DISMISS(flushWQ);
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Sizes_MinReqSizeTooSmall)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(0U, ResponseBase::minimumUsefulResponseSize)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(RequestBase::minimumUsefulRequestSize + ReturnStackItem::binarySize - 1U,
                                                      ResponseBase::minimumUsefulResponseSize +  ReturnStackItem::binarySize); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  ON_SCOPE_EXIT(flushWQ) { dwq.FlushNonDeferredWorkPackages(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));

  ON_SCOPE_EXIT_DISMISS(flushWQ);
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Sizes_MinRespSizeTooSmall)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(RequestBase::minimumUsefulRequestSize, 0U)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(RequestBase::minimumUsefulRequestSize + ReturnStackItem::binarySize,
                                                      ResponseBase::minimumUsefulResponseSize +  ReturnStackItem::binarySize - 1U); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  ON_SCOPE_EXIT(flushWQ) { dwq.FlushNonDeferredWorkPackages(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));

  ON_SCOPE_EXIT_DISMISS(flushWQ);
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Sizes_BothTooSmall)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(0U, 0U)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(RequestBase::minimumUsefulRequestSize + ReturnStackItem::binarySize - 1U,
                                                      ResponseBase::minimumUsefulResponseSize +  ReturnStackItem::binarySize - 1U); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  ON_SCOPE_EXIT(flushWQ) { dwq.FlushNonDeferredWorkPackages(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));

  ON_SCOPE_EXIT_DISMISS(flushWQ);
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

TEST_F(gpcc_cood_Multiplexer_TestsF, Sizes_BothZero)
{
  ASSERT_NO_FATAL_FAILURE(ConnectMuxToServer());

  // create a port
  auto spPort = spUUT->CreatePort();
  ASSERT_TRUE(spPort != nullptr);
  IRemoteObjectDictionaryAccess * pPortRODA = spPort.get();

  // register client
  pPortRODA->Register(&clientItf1);
  ON_SCOPE_EXIT(unregClient) { pPortRODA->Unregister(); };

  // server invokes OnReady and OnDisconnected
  {
    InSequence s;
    EXPECT_CALL(clientItf1, OnReady(0U, 0U)).Times(1);
    EXPECT_CALL(clientItf1, OnDisconnected()).Times(1);
  }
  auto invokeOnReady = [&]() { pRODAN_of_Mux->OnReady(0U, 0U); };
  auto invokeOnDisconnected = [&]() { pRODAN_of_Mux->OnDisconnected(); };
  ON_SCOPE_EXIT(flushWQ) { dwq.FlushNonDeferredWorkPackages(); };
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnReady));
  dwq.Add(WorkPackage::CreateDynamic(this, 0U, invokeOnDisconnected));

  ON_SCOPE_EXIT_DISMISS(flushWQ);
  dwq.FlushNonDeferredWorkPackages();

  Mock::VerifyAndClearExpectations(&clientItf1);

  // unregister client
  ON_SCOPE_EXIT_DISMISS(unregClient);
  pPortRODA->Unregister();

  ASSERT_NO_FATAL_FAILURE(DisconnectMuxFromServer());
}

} // namespace cood
} // namespace gpcc_tests
