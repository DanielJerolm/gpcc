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

#include "TestbenchMultiplexer.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_LoanExecutionContext.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_ObjectEnum.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_ObjectInfo.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_Ping.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_Read.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_RegisterUnregisterStartStop.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_Send.hpp"
#include "gpcc/test_src/cood/remote_access/roda_itf/TestIRODA_Write.hpp"

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
