/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021 Daniel Jerolm

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

#ifndef TESTBENCHWORKQUEUEBASEDRAS_HPP_202105180843
#define TESTBENCHWORKQUEUEBASEDRAS_HPP_202105180843

#ifndef SKIP_TFC_BASED_TESTS

#include "gpcc/test_src/cood/remote_access/roda_itf/TestbenchBase.hpp"
#include "gpcc/src/execution/async/DeferredWorkQueue.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include <memory>

namespace gpcc {
namespace cood {
  class WorkQueueBasedRemoteAccessServer;
}
}

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTS_COOD_RA
 * \brief Testbench for class @ref gpcc::cood::WorkQueueBasedRemoteAccessServer.
 *
 * Please refer to @ref GPCC_TESTS_COOD_RA for detailed information about the test strategy for the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TestbenchWorkQueueBasedRAS final : public TestbenchBase
{
  public:
    TestbenchWorkQueueBasedRAS(void);
    TestbenchWorkQueueBasedRAS(TestbenchWorkQueueBasedRAS const &) = delete;
    TestbenchWorkQueueBasedRAS(TestbenchWorkQueueBasedRAS &&) = delete;
    ~TestbenchWorkQueueBasedRAS(void);

    TestbenchWorkQueueBasedRAS& operator=(TestbenchWorkQueueBasedRAS const &) = delete;
    TestbenchWorkQueueBasedRAS& operator=(TestbenchWorkQueueBasedRAS &&) = delete;

    // <== TestbenchBase
    void StartUUT(void) override;
    void StopUUT(void) override;

    gpcc::cood::IRemoteObjectDictionaryAccess & GetUUT(void) override;

    uint32_t GetOnReadyTimeout_ms(void) const override;
    uint32_t GetMinimumResponseTime_ms(void) const override;
    uint32_t GetResponseTimeout_ms(void) const override;
    uint32_t GetTimeUntilMiddleOfTransmittingRequest_ms(void) const override;
    uint32_t GetTimeUntilMiddleOfProcessing_ms(void) const override;
    uint32_t GetTimeUntilMiddleOfTransmittingResponse_ms(void) const override;
    size_t   GetExpectedMaxRequestSize(void) const override;
    size_t   GetExpectedMaxResponseSize(void) const override;
    // ==> TestbenchBase

  private:
    /// Maximum request size that can be processed by the server.
    static size_t const serverMaxRequestSize = 256U;

    /// Maximum response size that can be send by the server.
    static size_t const serverMaxResponseSize = 256U;


    /// Logger for the remote access server.
    gpcc::log::Logger rasLogger;

    /// Deferred work queue used as execution context for the remote access server.
    gpcc::execution::async::DeferredWorkQueue dwq;

    /// Thread used to drive @ref dwq.
    gpcc::osal::Thread dwqThread;

    /// Remote access server (in this testbench it is the UUT).
    std::unique_ptr<gpcc::cood::WorkQueueBasedRemoteAccessServer> spRemoteAccessServer;


    void* DWQThreadEntry(void);
};

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTBENCHWORKQUEUEBASEDRAS_HPP_202105180843
