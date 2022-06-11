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

#ifndef TESTBENCHMULTIPLEXER_HPP_202109062025
#define TESTBENCHMULTIPLEXER_HPP_202109062025

#ifndef SKIP_TFC_BASED_TESTS

#include "gpcc/test_src/cood/remote_access/roda_itf/TestbenchBase.hpp"
#include <memory>

namespace gpcc {
namespace cood {
  class Multiplexer;
  class MultiplexerPort;
  class ThreadBasedRemoteAccessServer;
}
}

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTS_COOD_RA
 * \brief Testbench for class @ref gpcc::cood::Multiplexer.
 *
 * This testbench is based on @ref TestbenchThreadBasedRAS and just adds a `Multiplexer` as UUT in front of the
 * remote access server.
 *
 * Please refer to @ref GPCC_TESTS_COOD_RA for detailed information about the test strategy for the
 * [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TestbenchMultiplexer final : public TestbenchBase
{
  public:
    TestbenchMultiplexer(void);
    TestbenchMultiplexer(TestbenchMultiplexer const &) = delete;
    TestbenchMultiplexer(TestbenchMultiplexer &&) = delete;
    ~TestbenchMultiplexer(void);

    TestbenchMultiplexer& operator=(TestbenchMultiplexer const &) = delete;
    TestbenchMultiplexer& operator=(TestbenchMultiplexer &&) = delete;

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

    /// Remote access server.
    std::unique_ptr<gpcc::cood::ThreadBasedRemoteAccessServer> spRemoteAccessServer;


    /// Multiplexer (UUT)
    std::unique_ptr<gpcc::cood::Multiplexer> spMultiplexer;

    /// Multiplexer Port #1
    std::shared_ptr<gpcc::cood::MultiplexerPort> spPort1;

    /// Multiplexer Port #2
    std::shared_ptr<gpcc::cood::MultiplexerPort> spPort2;
};

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
#endif // TESTBENCHMULTIPLEXER_HPP_202109062025
