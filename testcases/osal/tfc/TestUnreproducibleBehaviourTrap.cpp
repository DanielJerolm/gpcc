/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#include <gpcc_test/osal/tfc/UnreproducibleBehaviourTrap.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include <gtest/gtest.h>
#include <functional>
#include <stdexcept>

namespace
{
  void* ThreadEntry_Sleep100ms(void)
  {
    gpcc::osal::Thread::Sleep_ms(100);
    return nullptr;
  }
}

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

using namespace testing;

TEST(gpcc_tests_osal_tfc_UnreproducibleBehaviourTrap_Tests, InstantiationAndMonitoring)
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

TEST(gpcc_tests_osal_tfc_UnreproducibleBehaviourTrap_Tests, DTORendsMonitoring)
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

TEST(gpcc_tests_osal_tfc_UnreproducibleBehaviourTrap_Tests, InvalidUse)
{
  UnreproducibleBehaviourTrap uut;

  // 2x begin
  uut.BeginMonitoring();
  EXPECT_THROW(uut.BeginMonitoring(), std::logic_error);

  // 2x end
  uut.EndMonitoring();
  EXPECT_THROW(uut.EndMonitoring(), std::logic_error);

  // QueryAndReset() with monitoring not enabled
  EXPECT_THROW((void)uut.QueryAndReset(), std::logic_error);
}

TEST(gpcc_tests_osal_tfc_UnreproducibleBehaviourTrap_Tests, QueryAndResetTrap)
{
  using gpcc::osal::Thread;

  UnreproducibleBehaviourTrap uut;
  uut.BeginMonitoring();

  Thread thread1("Thread1");
  Thread thread2("Thread2");

  thread1.Start(std::bind(&ThreadEntry_Sleep100ms),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinThread1) { (void)thread1.Join(nullptr); };

  thread2.Start(std::bind(&ThreadEntry_Sleep100ms),
                Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
  ON_SCOPE_EXIT(joinThread2) { (void)thread2.Join(nullptr); };

  Thread::Sleep_ms(99U);
  EXPECT_FALSE(uut.QueryAndReset()) << "Trap triggered, but there was no incident yet";

  Thread::Sleep_ms(2U);
  EXPECT_TRUE(uut.QueryAndReset()) << "Trap did not trigger";

  EXPECT_FALSE(uut.QueryAndReset()) << "Trap's trigger state was not reset";

  uut.EndMonitoring();
}

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
