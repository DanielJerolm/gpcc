/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#include <gpcc_test/osal/tfc/BlockWithExpiredTimeoutTrap.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

using namespace testing;

TEST(gpcc_tests_osal_tfc_BlockWithExpiredTimeoutTrap_Tests, InstantiationAndMonitoring)
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

TEST(gpcc_tests_osal_tfc_BlockWithExpiredTimeoutTrap_Tests, DTORendsMonitoring)
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

TEST(gpcc_tests_osal_tfc_BlockWithExpiredTimeoutTrap_Tests, InvalidUse)
{
  BlockWithExpiredTimeoutTrap uut;

  // 2x begin
  uut.BeginMonitoring();
  EXPECT_THROW(uut.BeginMonitoring(), std::logic_error);

  // 2x end
  uut.EndMonitoring();
  EXPECT_THROW(uut.EndMonitoring(), std::logic_error);

  // QueryAndReset() with monitoring not enabled
  EXPECT_THROW((void)uut.QueryAndReset(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_BlockWithExpiredTimeoutTrap_Tests, QueryAndResetTrap)
{
  using namespace gpcc::time;
  using gpcc::osal::ConditionVariable;

  BlockWithExpiredTimeoutTrap uut;
  uut.BeginMonitoring();

  // sleep a millisecond to ensure that we can subtract 1ns from the system time below
  gpcc::osal::Thread::Sleep_ms(1);

  EXPECT_FALSE(uut.QueryAndReset()) << "Trap triggered, but there was incident yet";

  ConditionVariable cv;
  TimePoint const timeout = TimePoint::FromSystemClock(ConditionVariable::clockID) - TimeSpan::ns(1);

  {
    gpcc::osal::Mutex m;
    gpcc::osal::MutexLocker ml(m);
    ASSERT_TRUE(cv.TimeLimitedWait(m, timeout));
  }

  EXPECT_TRUE(uut.QueryAndReset()) << "Trap did not trigger";

  EXPECT_FALSE(uut.QueryAndReset()) << "Trap's trigger state was not reset";

  uut.EndMonitoring();
}

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
