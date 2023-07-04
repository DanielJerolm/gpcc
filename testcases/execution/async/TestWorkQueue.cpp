/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TestIWorkQueue.hpp"

namespace gpcc_tests {
namespace execution {
namespace async {

INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_execution_async_WorkQueue_, IWorkQueue_Tests1F, WorkQueue);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_execution_async_WorkQueue_, IWorkQueue_Tests2F, WorkQueue);
INSTANTIATE_TYPED_TEST_SUITE_P(gpcc_execution_async_WorkQueue_, IWorkQueue_DeathTests1F, WorkQueue);

} // namespace execution
} // namespace async
} // namespace gpcc_tests
