/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef SKIP_TFC_BASED_TESTS

#include "TestbenchThreadBasedRAS.hpp"
#include "gpcc/src/cood/remote_access/infrastructure/ThreadBasedRemoteAccessServer.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/tools.hpp"
#include <algorithm>
#include <stdexcept>

namespace gpcc_tests {
namespace cood       {

size_t const TestbenchThreadBasedRAS::serverMaxRequestSize;
size_t const TestbenchThreadBasedRAS::serverMaxResponseSize;

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
TestbenchThreadBasedRAS::TestbenchThreadBasedRAS(void)
: TestbenchBase()
, rasLogger("Server")
, spRemoteAccessServer()
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
TestbenchThreadBasedRAS::~TestbenchThreadBasedRAS(void)
{
  try
  {
    spRemoteAccessServer.reset();

    logFacility.Unregister(rasLogger);
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "TestbenchThreadBasedRAS::~TestbenchThreadBasedRAS: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchThreadBasedRAS::~TestbenchThreadBasedRAS: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("TestbenchThreadBasedRAS::~TestbenchThreadBasedRAS: Caught an unknown exception");
  }
}

// <== TestbenchBase

/// \copydoc gpcc_tests::cood::TestbenchBase::StartUUT
void TestbenchThreadBasedRAS::StartUUT(void)
{
  tbLogger.Log(gpcc::log::LogType::Info, "Starting UUT...");

  spRemoteAccessServer->Start(gpcc::osal::Thread::SchedPolicy::Other,
                              0U,
                              gpcc::osal::Thread::GetDefaultStackSize());

  tbLogger.LogTS(gpcc::log::LogType::Info, "UUT started");
}

/// \copydoc gpcc_tests::cood::TestbenchBase::StopUUT
void TestbenchThreadBasedRAS::StopUUT(void)
{
  tbLogger.LogTS(gpcc::log::LogType::Info, "Stopping UUT...");

  spRemoteAccessServer->Stop();

  tbLogger.Log(gpcc::log::LogType::Info, "UUT stopped");
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetUUT
gpcc::cood::IRemoteObjectDictionaryAccess & TestbenchThreadBasedRAS::GetUUT(void)
{
  return *spRemoteAccessServer;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetOnReadyTimeout_ms
uint32_t TestbenchThreadBasedRAS::GetOnReadyTimeout_ms(void) const
{
  // result: Timeout for processing one request plus 1.
  return GetResponseTimeout_ms() + 1U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetMinimumResponseTime_ms
uint32_t TestbenchThreadBasedRAS::GetMinimumResponseTime_ms(void) const
{
  return std::min(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms);
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfTransmittingRequest_ms
uint32_t TestbenchThreadBasedRAS::GetTimeUntilMiddleOfTransmittingRequest_ms(void) const
{
  return 0U; // (scenario not supported)
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfProcessing_ms
uint32_t TestbenchThreadBasedRAS::GetTimeUntilMiddleOfProcessing_ms(void) const
{
  return std::min(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms) / 2U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfTransmittingResponse_ms
uint32_t TestbenchThreadBasedRAS::GetTimeUntilMiddleOfTransmittingResponse_ms(void) const
{
  return 0U; // (scenario not supported)
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetResponseTimeout_ms
uint32_t TestbenchThreadBasedRAS::GetResponseTimeout_ms(void) const
{
  return std::max(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms) + 1U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetExpectedMaxRequestSize
size_t TestbenchThreadBasedRAS::GetExpectedMaxRequestSize(void) const
{
  return serverMaxRequestSize;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetExpectedMaxResponseSize
size_t TestbenchThreadBasedRAS::GetExpectedMaxResponseSize(void) const
{
  return serverMaxResponseSize;
}

// ==> TestbenchBase

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
