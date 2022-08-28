/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef SKIP_TFC_BASED_TESTS

#include "TestbenchMultiplexer.hpp"
#include "gpcc/src/cood/remote_access/infrastructure/Multiplexer.hpp"
#include "gpcc/src/cood/remote_access/infrastructure/ThreadBasedRemoteAccessServer.hpp"
#include "gpcc/src/cood/remote_access/requests_and_responses/ReturnStackItem.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/tools.hpp>
#include <algorithm>
#include <stdexcept>

namespace gpcc_tests {
namespace cood       {

size_t const TestbenchMultiplexer::serverMaxRequestSize;
size_t const TestbenchMultiplexer::serverMaxResponseSize;

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TestbenchMultiplexer::TestbenchMultiplexer(void)
: TestbenchBase()
, rasLogger("Server")
, spRemoteAccessServer()
, spMultiplexer()
, spPort1()
, spPort2()
{
  rasLogger.SetLogLevel(gpcc::log::LogLevel::DebugOrAbove);
  logFacility.Register(rasLogger);
  ON_SCOPE_EXIT(unregRASLogger) { logFacility.Unregister(rasLogger); };

  spRemoteAccessServer = std::make_unique<gpcc::cood::ThreadBasedRemoteAccessServer>("RAS",
                                                                                     100U,
                                                                                     od,
                                                                                     &rasLogger,
                                                                                     serverMaxRequestSize,
                                                                                     serverMaxResponseSize);

  spMultiplexer = std::make_unique<gpcc::cood::Multiplexer>();
  spPort1 = spMultiplexer->CreatePort();
  spPort2 = spMultiplexer->CreatePort();

  spMultiplexer->Connect(*spRemoteAccessServer);

  ON_SCOPE_EXIT_DISMISS(unregRASLogger);
}

/**
 * \brief Destructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TestbenchMultiplexer::~TestbenchMultiplexer(void)
{
  try
  {
    spMultiplexer->Disconnect();
    spPort1.reset();
    spPort2.reset();
    spMultiplexer.reset();
    spRemoteAccessServer.reset();

    logFacility.Unregister(rasLogger);
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "TestbenchMultiplexer::~TestbenchMultiplexer: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchMultiplexer::~TestbenchMultiplexer: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("TestbenchMultiplexer::~TestbenchMultiplexer: Caught an unknown exception");
  }
}

// <== TestbenchBase

/// \copydoc gpcc_tests::cood::TestbenchBase::StartUUT
void TestbenchMultiplexer::StartUUT(void)
{
  tbLogger.Log(gpcc::log::LogType::Info, "Starting UUT...");

  spRemoteAccessServer->Start(gpcc::osal::Thread::SchedPolicy::Other,
                              0U,
                              gpcc::osal::Thread::GetDefaultStackSize());

  tbLogger.LogTS(gpcc::log::LogType::Info, "UUT started");
}

/// \copydoc gpcc_tests::cood::TestbenchBase::StopUUT
void TestbenchMultiplexer::StopUUT(void)
{
  tbLogger.LogTS(gpcc::log::LogType::Info, "Stopping UUT...");

  spRemoteAccessServer->Stop();

  tbLogger.Log(gpcc::log::LogType::Info, "UUT stopped");
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetUUT
gpcc::cood::IRemoteObjectDictionaryAccess & TestbenchMultiplexer::GetUUT(void)
{
  return *spPort1;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetOnReadyTimeout_ms
uint32_t TestbenchMultiplexer::GetOnReadyTimeout_ms(void) const
{
  // result: Timeout for processing one request plus 1.
  return GetResponseTimeout_ms() + 1U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetMinimumResponseTime_ms
uint32_t TestbenchMultiplexer::GetMinimumResponseTime_ms(void) const
{
  return std::min(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms);
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfTransmittingRequest_ms
uint32_t TestbenchMultiplexer::GetTimeUntilMiddleOfTransmittingRequest_ms(void) const
{
  return 0U; // (scenario not supported)
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfProcessing_ms
uint32_t TestbenchMultiplexer::GetTimeUntilMiddleOfProcessing_ms(void) const
{
  return std::min(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms) / 2U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfTransmittingResponse_ms
uint32_t TestbenchMultiplexer::GetTimeUntilMiddleOfTransmittingResponse_ms(void) const
{
  return 0U; // (scenario not supported)
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetResponseTimeout_ms
uint32_t TestbenchMultiplexer::GetResponseTimeout_ms(void) const
{
  return std::max(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms) + 1U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetExpectedMaxRequestSize
size_t TestbenchMultiplexer::GetExpectedMaxRequestSize(void) const
{
  return serverMaxRequestSize - gpcc::cood::ReturnStackItem::binarySize;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetExpectedMaxResponseSize
size_t TestbenchMultiplexer::GetExpectedMaxResponseSize(void) const
{
  return serverMaxResponseSize - gpcc::cood::ReturnStackItem::binarySize;
}

// ==> TestbenchBase

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
