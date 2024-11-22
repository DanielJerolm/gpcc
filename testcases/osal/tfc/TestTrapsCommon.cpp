/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#include <gpcc_test/osal/tfc/BlockWithExpiredTimeoutTrap.hpp>
#include <gpcc_test/osal/tfc/PotentialUnreproducibleBehaviourTrap.hpp>
#include <gpcc_test/osal/tfc/UnreproducibleBehaviourTrap.hpp>
#include <gtest/gtest.h>

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

using namespace testing;

TEST(gpcc_tests_osal_tfc_Tests, UseAllTrapsTogether)
{
  BlockWithExpiredTimeoutTrap uut1;
  PotentialUnreproducibleBehaviourTrap uut2;
  UnreproducibleBehaviourTrap uut3;
  uut1.BeginMonitoring();
  uut2.BeginMonitoring();
  uut3.BeginMonitoring();

  SUCCEED();
}

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
