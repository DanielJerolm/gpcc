/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#include <gpcc_test/osal/tfc_traps.hpp>
#include "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

using namespace testing;

TEST(gpcc_tests_osal_tfc_BlockWithExpiredTimeoutTrap, InstantiationAndMonitoring)
{
  using gpcc::osal::internal::TFCCore;
  TFCCore* pTFC = TFCCore::Get();

  {
    BlockWithExpiredTimeoutTrap uut;
    uut.BeginMonitoring();
    EXPECT_THROW(pTFC->EnableWatchForAlreadyExpiredTimeout(), std::logic_error);
    uut.EndMonitoring();
  }

  EXPECT_THROW((void)pTFC->DisableWatchForAlreadyExpiredTimeout(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_BlockWithExpiredTimeoutTrap, DTORendsMonitoring)
{
  using gpcc::osal::internal::TFCCore;
  TFCCore* pTFC = TFCCore::Get();

  {
    BlockWithExpiredTimeoutTrap uut;
    uut.BeginMonitoring();
    EXPECT_THROW(pTFC->EnableWatchForAlreadyExpiredTimeout(), std::logic_error);
  }

  EXPECT_THROW((void)pTFC->DisableWatchForAlreadyExpiredTimeout(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_PotentialUnreproducibleBehaviourTrap, InstantiationAndMonitoring)
{
  using gpcc::osal::internal::TFCCore;
  TFCCore* pTFC = TFCCore::Get();

  {
    PotentialUnreproducibleBehaviourTrap uut;
    uut.BeginMonitoring();
    EXPECT_THROW(pTFC->EnableWatchForBlockWithSameTimeout(), std::logic_error);
    uut.EndMonitoring();
  }

  EXPECT_THROW((void)pTFC->DisableWatchForBlockWithSameTimeout(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_PotentialUnreproducibleBehaviourTrap, DTORendsMonitoring)
{
  using gpcc::osal::internal::TFCCore;
  TFCCore* pTFC = TFCCore::Get();

  {
    PotentialUnreproducibleBehaviourTrap uut;
    uut.BeginMonitoring();
    EXPECT_THROW(pTFC->EnableWatchForBlockWithSameTimeout(), std::logic_error);
  }

  EXPECT_THROW((void)pTFC->DisableWatchForBlockWithSameTimeout(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_UnreproducibleBehaviourTrap, InstantiationAndMonitoring)
{
  using gpcc::osal::internal::TFCCore;
  TFCCore* pTFC = TFCCore::Get();

  {
    UnreproducibleBehaviourTrap uut;
    uut.BeginMonitoring();
    EXPECT_THROW(pTFC->EnableWatchForSimultaneousResumeOfMultipleThreads(), std::logic_error);
    uut.EndMonitoring();
  }

  EXPECT_THROW((void)pTFC->DisableWatchForSimultaneousResumeOfMultipleThreads(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_UnreproducibleBehaviourTrap, DTORendsMonitoring)
{
  using gpcc::osal::internal::TFCCore;
  TFCCore* pTFC = TFCCore::Get();

  {
    UnreproducibleBehaviourTrap uut;
    uut.BeginMonitoring();
    EXPECT_THROW(pTFC->EnableWatchForSimultaneousResumeOfMultipleThreads(), std::logic_error);
  }

  EXPECT_THROW((void)pTFC->DisableWatchForSimultaneousResumeOfMultipleThreads(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_X_Trap, UseAllTrapsSimultaneously)
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

#endif // #ifdef OS_LINUX_X64_TFC
