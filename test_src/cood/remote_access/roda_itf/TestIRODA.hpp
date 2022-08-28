/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTIRODA_HPP_202008152225
#define TESTIRODA_HPP_202008152225

#ifndef SKIP_TFC_BASED_TESTS

#include "RODAN_Listener.hpp"
#include "TestbenchBase.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/RequestBase.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ResponseBase.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <gpcc/string/tools.hpp>

#include "gtest/gtest.h"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

namespace gpcc_tests {
namespace cood       {

using namespace testing;

/**
 * \ingroup GPCC_TESTS_COOD_RA
 * \brief Type-parametrized test fixture for a RODA/RODAN-interface pair
 *        (@ref gpcc::cood::IRemoteObjectDictionaryAccess and @ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable).
 *
 * # Purpose
 * GPCC provides mutiple classes realizing the @ref gpcc::cood::IRemoteObjectDictionaryAccess interface. The test cases
 * using this type-parametrized test fixture can be applied to any class realizing the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface. For details about the
 * overall test concept please refer to @ref GPCC_TESTS_COOD_RA.
 *
 * # How to apply test cases to a UUT
 * To apply the test cases based on this test fixture to a UUT which realizes the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface, a testbench __T__ for the
 * specific UUT must be provided. All testbenches __T__ must be derived from @ref TestbenchBase. For details about the
 * overall test concept please refer to @ref GPCC_TESTS_COOD_RA.
 *
 * The test cases can be instantiated for a UUT as shown in this example:
 * ~~~{.cpp}
 * #include "TestIRODA_RegisterUnregisterStartStop.hpp" // Test cases for registration, unregistration, start, stop, etc.
 * #include "TestIRODA_Write.hpp"                       // Test cases for write-access to object dictionary
 * // [...] Optional more includes for tests on other topics
 *
 * #include "TestbenchThreadBasedRAS.hpp"     // Testbench for UUT "ThreadBasedRemoteAccessServer"
 *
 * INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_, IRODA_RegisterUnregisterStartStopTestsF, TestbenchThreadBasedRAS);
 * INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_, IRODA_WriteTestsF, TestbenchThreadBasedRAS);
 * // [...] Maybe more directives for more tests
 * ~~~
 *
 * # How to create test cases
 * Testcases shall be grouped by topic and implemented in a separate hpp-files for each topic. The header files shall
 * follow the naming scheme TestIRODA_*.hpp, where * represents the name of the topic. A list of topics is provided
 * in chapter "Available tests" below.
 *
 * For each topic, there shall be a using-declaration to create a separte test fixture name associated with the topic.\n
 * Example:
 * ~~~{.cpp}
 * template <typename T>
 * using IRODA_ConnectDisconnectTestsF = IRemoteObjectDictionaryAccess_TestsF<T>;
 *
 * TYPED_TEST_SUITE_P(IRODA_ConnectDisconnectTestsF);
 *
 * TYPED_TEST_P(IRODA_ConnectDisconnectTestsF, SomeTestCaseXYZ)
 * {
 *   // [...]
 * }
 *
 * ~~~
 *
 * # How test cases shall use this test fixture
 * ## Conditions at test case entry
 * Upon test case entry, the test fixture is in the following state:
 * - UUT instatiated
 * - UUT started
 * - RODAN-Listener not yet registered at UUT
 *
 * ## Test-case post-conditions
 * There are no strict post-conditions. The text fixture is tolerant regarding the state of the RODAN listener,
 * the UUT and potential outstanding responses:
 * - RODAN-Listener is registered or not registered at the UUT
 * - UUT is started or stopped
 * - Outstanding responses are not harmful
 *
 * ## Typical usage
 * - Use @ref RegisterAtRODA() and @ref UnregisterFromRODA() to register and unregister the RODAN-Listener at the UUT.
 * - Use @ref TransmitAndReceive() to transmit a single request and wait for the response.
 * - If required, use @ref StartUUT() and @ref StopUUT() to start and stop the UUT.
 * - Use @ref uut to directly stimulate the UUT
 * - Use @ref rodanListener to examine the UUT's reaction
 *
 * # Available tests
 *
 * File                                      | Topic
 * ----------------------------------------- | -----
 * TestIRODA_LoanExecutionContext.hpp        | Tests IRemoteObjectDictionaryAccess::RequestExecutionContext()
 * TestIRODA_ObjectEnum.hpp                  | Tests @ref gpcc::cood::ObjectEnumRequest in conjunction with the UUT.
 * TestIRODA_ObjectInfo.hpp                  | Tests @ref gpcc::cood::ObjectInfoRequest in conjunction with the UUT.
 * TestIRODA_Ping.hpp                        | Tests @ref gpcc::cood::PingRequest in conjunction with the UUT.
 * TestIRODA_Read                            | Tests @ref gpcc::cood::ReadRequest in conjunction with the UUT.
 * TestIRODA_RegisterUnregisterStartStop.hpp | Tests registration, unregistration, start, and stop of UUT and combinations.
 * TestIRODA_Send.hpp                        | Tests IRemoteObjectDictionaryAccess::Send()
 * TestIRODA_Write.hpp                       | Tests @ref gpcc::cood::WriteRequest in conjunction with the UUT.
 *
 * - - -
 *
 * \tparam T
 * Type of the testbench. The testbench provides the UUT.\n
 * Testbenches must be derived from @ref TestbenchBase. See @ref TestbenchBase for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
template <typename T>
class IRemoteObjectDictionaryAccess_TestsF : public Test
{
  public:
    IRemoteObjectDictionaryAccess_TestsF(void);

  protected:
    /// Testbench (specific).
    T specificTestbench;

    /// Testbench (common).
    TestbenchBase & testbench;

    /// UUT, provided by @ref testbench.
    gpcc::cood::IRemoteObjectDictionaryAccess & uut;

    /// Listener for registration at UUT.
    RODAN_Listener rodanListener;

    /// Standard value for the maximum response size used in tests without return stack item.
    /** The value is determined by the maximum possible response size announced by OnReady(). */
    size_t stdMaxResponseSize_wo_RSI;

    /// Standard value for the maximum response size used in tests with one return stack item.
    /** The value is determined by the maximum possible response size announced by OnReady(). */
    size_t stdMaxResponseSize_w_RSI;

    /// UUT may be started or stopped during tests. This flag indicates if @ref TearDown() needs to stop the UUT or not.
    bool uutNeedsStop;


    virtual ~IRemoteObjectDictionaryAccess_TestsF(void) = default;

    void SetUp(void) override;
    void TearDown(void) override;


    void StartUUT(void);
    void StopUUT(void);

    void RegisterAtRODA(bool const readyExpected);
    void UnregisterFromRODA(void);

    void TransmitAndReceive(std::unique_ptr<gpcc::cood::RequestBase> & spReq);

    void CreateAndPushReturnStackItem(gpcc::cood::RequestBase & req);
    void PopCheckAndConsumeReturnStackItem(gpcc::cood::ResponseBase & resp);
};

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
template <typename T>
IRemoteObjectDictionaryAccess_TestsF<T>::IRemoteObjectDictionaryAccess_TestsF(void)
: Test()
, specificTestbench()
, testbench(specificTestbench)
, uut(testbench.GetUUT())
, rodanListener(testbench.rodanLogger)
, stdMaxResponseSize_wo_RSI(0U)
, stdMaxResponseSize_w_RSI(0U)
, uutNeedsStop(false)
{
}

/**
 * \brief Prepares the test fixture.
 *
 * The following actions are taken:
 * - The UUT is started.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::SetUp(void)
{
  StartUUT();
}

/**
 * \brief Shuts the test fixture down.
 *
 * The following actions are taken:
 * - The RODAN-Listener is disconnected from the UUT (if necessary).
 * - The UUT is stopped (if necessary).
 * - Errors and stati are collected.
 * - Log messages are printed to stdout (only in case of an error).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::TearDown(void)
{
  try
  {
    if (rodanListener.IsRegistered())
      UnregisterFromRODA();

    if (uutNeedsStop)
      StopUUT();

    if (rodanListener.AnyError())
      ADD_FAILURE() << "RODAN-Listener has error flag set!";

    if (HasFailure())
      testbench.PrintLogMessagesToStdout();
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "IRemoteObjectDictionaryAccess_TestsF<T>::TearDown: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-tests are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("IRemoteObjectDictionaryAccess_TestsF<T>::TearDown: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("IRemoteObjectDictionaryAccess_TestsF<T>::TearDown: Caught an unknown exception");
  }
}

/**
 * \brief Starts the UUT.
 *
 * \pre   The UUT is not running.\n
 *        You may query @ref uutNeedsStop to figure out if the UUT is running or not.
 *
 * \post  The UUT is running.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::StartUUT(void)
{
  testbench.StartUUT();
  uutNeedsStop = true;
}

/**
 * \brief Stops the UUT.
 *
 * \pre   The UUT is running.\n
 *        You may query @ref uutNeedsStop to figure out if the UUT is running or not.
 *
 * \post  The UUT is not running.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::StopUUT(void)
{
  testbench.StopUUT();
  uutNeedsStop = false;
}

/**
 * \brief Registers the RODAN-listener at the UUT and tests the state of the RODA-interface after registration.
 *
 * __Use ASSERT_NO_FATAL_FAILURE to invoke this!__
 *
 * \pre   The RODAN-listener is not registered at the UUT.
 *
 * \post  The RODAN-listener is registered and will receive and process notifications emitted by the UUT.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Sleep and check of post-conditions may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Sleep and check of post-conditions may be incomplete
 *
 * - - -
 *
 * \param readyExpected
 * Indicates if the RODA-interface is expected to be in _ready-state_ or in _not-ready-state_ after registration and
 * after a small delay.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::RegisterAtRODA(bool const readyExpected)
{
  // ===========================================
  // Use ASSERT_NO_FATAL_FAILURE to invoke this!
  // ===========================================
  stdMaxResponseSize_wo_RSI = 0U;
  stdMaxResponseSize_w_RSI = 0U;

  rodanListener.Register(uut);

  if (readyExpected)
  {
    ASSERT_TRUE(rodanListener.WaitForStateReady(testbench.GetOnReadyTimeout_ms())) << "RODA not ready within timeout";
    stdMaxResponseSize_wo_RSI = rodanListener.GetMaxResponseSize();
    if (stdMaxResponseSize_wo_RSI > gpcc::cood::ReturnStackItem::binarySize)
      stdMaxResponseSize_w_RSI = stdMaxResponseSize_wo_RSI - gpcc::cood::ReturnStackItem::binarySize;
  }
  else
  {
    ASSERT_FALSE(rodanListener.WaitForStateReady(testbench.GetOnReadyTimeout_ms())) <<
      "Did not expect RODA to enter ready-state!";

    ASSERT_TRUE(rodanListener.GetState() == RODAN_Listener::States::notReady);
  }
}

/**
 * \brief Unregisters the RODAN-listener from the UUT.
 *
 * \pre   The RODAN-listener is registered at the UUT.
 *
 * \post  The RODAN-listener is unregistered from the UUT.\n
 *        Any pending responses should be dropped by the UUT.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::UnregisterFromRODA(void)
{
  rodanListener.Unregister(uut);
}

/**
 * \brief Transmits a request and waits for reception of the response. Wait time is limited by a timeout.
 *
 * __Use ASSERT_NO_FATAL_FAILURE to invoke this!__
 *
 * If the response is not received in time, then a fatal failure will be added to the test.
 *
 * \pre   The receive buffer of the RODAN-Listener is empty. It shall not contain any received response message.
 *
 * \pre   There shall be no response pending due to a prior transmisson of a request.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The request may not be transmitted.
 * - The request may be transmitted, but the response may not yet be received.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - The request may not be transmitted.
 * - The request may be transmitted, but the response may not yet be received.
 *
 * - - -
 *
 * \param spReq
 * Request that shall be transmitted. nullptr is not allowed, but it will be forwarded to the UUT.\n
 * If the request was transmitted, then ownership will move to the UUT and the referenced unique_ptr will be cleared.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::TransmitAndReceive(std::unique_ptr<gpcc::cood::RequestBase> & spReq)
{
  // ===========================================
  // Use ASSERT_NO_FATAL_FAILURE to invoke this!
  // ===========================================

  ASSERT_EQ(rodanListener.GetNbOfAvailableResponses(), 0U) << "Precondition violated. There should be no response available yet!";

  uut.Send(spReq);
  ASSERT_TRUE(spReq == nullptr) << "Request object was not consumed by RODA::Send()";
  ASSERT_TRUE(rodanListener.WaitForResponseAvailable(testbench.GetResponseTimeout_ms())) << "Response not received in time";
}

/**
 * \brief Creates a [ReturnStackItem](@ref gpcc::cood::ReturnStackItem) (ID = 356, INFO = 33) and pushes it onto the
 *        stack of a request.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param req
 * Request object.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::CreateAndPushReturnStackItem(gpcc::cood::RequestBase & req)
{
  req.Push(gpcc::cood::ReturnStackItem(356U, 33U));
}

/**
 * \brief Pops a [ReturnStackItem](@ref gpcc::cood::ReturnStackItem) from the stack of a response and checks if the
 *        item corresponds to the item pushed onto the request via @ref CreateAndPushReturnStackItem().
 *
 * __Use ASSERT_NO_FATAL_FAILURE to invoke this!__
 *
 * \pre   There is exactly one item on the stack of the response.
 *
 * \post  The top item has been removed from the stack of the response.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param resp
 * Reference to the response message.
 */
template <typename T>
void IRemoteObjectDictionaryAccess_TestsF<T>::PopCheckAndConsumeReturnStackItem(gpcc::cood::ResponseBase & resp)
{
  // ===========================================
  // Use ASSERT_NO_FATAL_FAILURE to invoke this!
  // ===========================================

  // check that there is exactly one return stack item in the reponse and pop it from the response
  ASSERT_FALSE(resp.IsReturnStackEmpty()) << "The return stack of the message is empty";
  auto rse = resp.PopReturnStack();
  EXPECT_TRUE(resp.IsReturnStackEmpty()) << "There should have been only one return stack item in the response";

  // check if the return stack item contains the information that has been pushed onto the request
  EXPECT_EQ(rse.GetID(), 356U);
  EXPECT_EQ(rse.GetInfo(), 33U);
}

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTIRODA_HPP_202008152225
