/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#ifndef SKIP_TFC_BASED_TESTS

#include "TestbenchWorkQueueBasedRAS.hpp"
#include "gpcc/src/cood/remote_access/infrastructure/WorkQueueBasedRemoteAccessServer.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/tools.hpp"
#include <algorithm>
#include <stdexcept>

namespace gpcc_tests {
namespace cood       {

size_t const TestbenchWorkQueueBasedRAS::serverMaxRequestSize;
size_t const TestbenchWorkQueueBasedRAS::serverMaxResponseSize;

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
TestbenchWorkQueueBasedRAS::TestbenchWorkQueueBasedRAS(void)
: TestbenchBase()
, rasLogger("Server")
, dwq()
, dwqThread("DWQThread")
, spRemoteAccessServer()
{
  rasLogger.SetLogLevel(gpcc::log::LogLevel::DebugOrAbove);
  logFacility.Register(rasLogger);
  ON_SCOPE_EXIT(unregRASLogger) { logFacility.Unregister(rasLogger); };


  dwqThread.Start(std::bind(&TestbenchWorkQueueBasedRAS::DWQThreadEntry, this),
                  gpcc::osal::Thread::SchedPolicy::Other, 0U,
                  gpcc::osal::Thread::GetDefaultStackSize());

  ON_SCOPE_EXIT(stopDwqThread)
  {
    dwq.RequestTermination();
    dwqThread.Join();
  };

  dwq.FlushNonDeferredWorkPackages();


  spRemoteAccessServer = std::make_unique<gpcc::cood::WorkQueueBasedRemoteAccessServer>(dwq,
                                                                                        100U,
                                                                                        od,
                                                                                        &rasLogger,
                                                                                        serverMaxRequestSize,
                                                                                        serverMaxResponseSize);

  ON_SCOPE_EXIT_DISMISS(stopDwqThread);
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
TestbenchWorkQueueBasedRAS::~TestbenchWorkQueueBasedRAS(void)
{
  try
  {
    spRemoteAccessServer.reset();

    dwq.RequestTermination();
    dwqThread.Join();

    logFacility.Unregister(rasLogger);
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "TestbenchWorkQueueBasedRAS::~TestbenchWorkQueueBasedRAS: Failed:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchWorkQueueBasedRAS::~TestbenchWorkQueueBasedRAS: Failed: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("TestbenchWorkQueueBasedRAS::~TestbenchWorkQueueBasedRAS: Caught an unknown exception");
  }
}

// <== TestbenchBase

/// \copydoc gpcc_tests::cood::TestbenchBase::StartUUT
void TestbenchWorkQueueBasedRAS::StartUUT(void)
{
  tbLogger.Log(gpcc::log::LogType::Info, "Starting UUT...");

  spRemoteAccessServer->Start();

  tbLogger.LogTS(gpcc::log::LogType::Info, "UUT started");
}

/// \copydoc gpcc_tests::cood::TestbenchBase::StopUUT
void TestbenchWorkQueueBasedRAS::StopUUT(void)
{
  tbLogger.LogTS(gpcc::log::LogType::Info, "Stopping UUT...");

  spRemoteAccessServer->Stop();

  tbLogger.Log(gpcc::log::LogType::Info, "UUT stopped");
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetUUT
gpcc::cood::IRemoteObjectDictionaryAccess & TestbenchWorkQueueBasedRAS::GetUUT(void)
{
  return *spRemoteAccessServer;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetOnReadyTimeout_ms
uint32_t TestbenchWorkQueueBasedRAS::GetOnReadyTimeout_ms(void) const
{
  // result: Timeout for processing one request plus 1.
  return GetResponseTimeout_ms() + 1U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetMinimumResponseTime_ms
uint32_t TestbenchWorkQueueBasedRAS::GetMinimumResponseTime_ms(void) const
{
  return std::min(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms);
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfTransmittingRequest_ms
uint32_t TestbenchWorkQueueBasedRAS::GetTimeUntilMiddleOfTransmittingRequest_ms(void) const
{
  return 0U; // (scenario not supported)
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfProcessing_ms
uint32_t TestbenchWorkQueueBasedRAS::GetTimeUntilMiddleOfProcessing_ms(void) const
{
  return std::min(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms) / 2U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetTimeUntilMiddleOfTransmittingResponse_ms
uint32_t TestbenchWorkQueueBasedRAS::GetTimeUntilMiddleOfTransmittingResponse_ms(void) const
{
  return 0U; // (scenario not supported)
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetResponseTimeout_ms
uint32_t TestbenchWorkQueueBasedRAS::GetResponseTimeout_ms(void) const
{
  return std::max(beforeReadCallbackDuration_ms, beforeWriteCallbackDuration_ms) + 1U;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetExpectedMaxRequestSize
size_t TestbenchWorkQueueBasedRAS::GetExpectedMaxRequestSize(void) const
{
  return serverMaxRequestSize;
}

/// \copydoc gpcc_tests::cood::TestbenchBase::GetExpectedMaxResponseSize
size_t TestbenchWorkQueueBasedRAS::GetExpectedMaxResponseSize(void) const
{
  return serverMaxResponseSize;
}

// ==> TestbenchBase

/**
 * \brief Entry function for @ref dwqThread. The thread will invoke the Work()-method of @ref dwq.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures that there is no more than one thread at any time.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 *
 * - - -
 *
 * \return
 * Value that will be returned by Thread::Join(). Here always nullptr.
 */
void* TestbenchWorkQueueBasedRAS::DWQThreadEntry(void)
{
  try
  {
    dwq.Work();
  }
  catch (std::exception const & e)
  {
    // create a detailed panic message
    try
    {
      std::string str = "TestbenchWorkQueueBasedRAS::DWQThreadEntry: dwq.Work() threw:\n";
      str += gpcc::string::ExceptionDescriptionToString(e);
      gpcc::osal::Panic(str.c_str());
    }
    catch (...)
    {
      // unit-test are usually executed on a large machine using memory overcommittment techniques, so
      // std::bad_alloc is not exepected here
      gpcc::osal::Panic("TestbenchWorkQueueBasedRAS::DWQThreadEntry: dwq.Work() threw: ", e);
    }
  }
  catch (...)
  {
    gpcc::osal::Panic("TestbenchWorkQueueBasedRAS::DWQThreadEntry: Caught an unknown exception");
  }

  return nullptr;
}

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
