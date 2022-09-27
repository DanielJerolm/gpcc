/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef SKIP_TFC_BASED_TESTS

#include "TestbenchMultiplexer.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_LoanExecutionContext.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_ObjectEnum.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_ObjectInfo.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Ping.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Read.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_RegisterUnregisterStartStop.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Send.hpp"
#include "test_src/cood/remote_access/roda_itf/TestIRODA_Write.hpp"

namespace gpcc_tests {
namespace cood       {

using namespace testing;

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_LoanExecutionContextTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_ObjectEnumTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_ObjectInfoTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_PingTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_ReadTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_RegisterUnregisterStartStopTestsF, TestbenchMultiplexer);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_RegisterUnregisterStartStopDeathTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_SendTestsF, TestbenchMultiplexer);

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_cood_Multiplexer_, IRODA_WriteTestsF, TestbenchMultiplexer);

} // namespace cood
} // namespace gpcc_tests

#endif // SKIP_TFC_BASED_TESTS
