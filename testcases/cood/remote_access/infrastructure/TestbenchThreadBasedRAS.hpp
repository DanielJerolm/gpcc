/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef TESTBENCHTHREADBASEDRAS_HPP_202008162052
#define TESTBENCHTHREADBASEDRAS_HPP_202008162052

#ifndef SKIP_TFC_BASED_TESTS

#include "testcases/cood/remote_access/roda_itf/TestbenchBase.hpp"
#include <memory>

namespace gpcc {
namespace cood {
  class ThreadBasedRemoteAccessServer;
}
}

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTS_COOD_RA
 * \brief Testbench for class @ref gpcc::cood::ThreadBasedRemoteAccessServer.
 *
 * Please refer to @ref GPCC_TESTS_COOD_RA for detailed information about the test strategy for the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TestbenchThreadBasedRAS final : public TestbenchBase
{
  public:
    TestbenchThreadBasedRAS(void);
    TestbenchThreadBasedRAS(TestbenchThreadBasedRAS const &) = delete;
    TestbenchThreadBasedRAS(TestbenchThreadBasedRAS &&) = delete;
    ~TestbenchThreadBasedRAS(void);

    TestbenchThreadBasedRAS& operator=(TestbenchThreadBasedRAS const &) = delete;
    TestbenchThreadBasedRAS& operator=(TestbenchThreadBasedRAS &&) = delete;

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

    /// Remote access server (in this testbench it is the UUT).
    std::unique_ptr<gpcc::cood::ThreadBasedRemoteAccessServer> spRemoteAccessServer;
};

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTBENCHTHREADBASEDRAS_HPP_202008162052
