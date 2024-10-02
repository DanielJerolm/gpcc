/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include <gpcc_test/osal/tfc_traps.hpp>
#include "src/osal/os/linux_arm_tfc/internal/TFCCore.hpp"
#include <gtest/gtest.h>

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

/**
 * \brief Constructor. Monitoring is disabled.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
BlockWithExpiredTimeoutTrap::BlockWithExpiredTimeoutTrap(void) noexcept
: enabled(false)
{
}

/**
 * \brief Destructor.
 *
 * \pre   Monitoring may be enabled or disabled.
 *
 * \post  Monitoring is disabled.
 *
 * \note  If monitoring was enabled and the trap was triggered, then the trapping event will be discarded.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
BlockWithExpiredTimeoutTrap::~BlockWithExpiredTimeoutTrap(void)
{
  if (enabled)
  {
    auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
    (void)pTFCCore->DisableWatchForAlreadyExpiredTimeout();
  }
}

/**
 * \brief Enables monitoring.
 *
 * \pre   Monitoring is disabled.
 *
 * \pre   There is no other @ref BlockWithExpiredTimeoutTrap instance in the process, which has monitoring enabled.
 *
 * \post  Monitoring is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void BlockWithExpiredTimeoutTrap::BeginMonitoring(void)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  pTFCCore->EnableWatchForAlreadyExpiredTimeout();
  enabled = true;
}

/**
 * \brief Disables monitoring and checks if the trap has been triggered.
 *
 * If the trap was triggered while it was enabled, then a failure will be added to the current unit test case.
 *
 * \pre   Monitoring is enabled.
 *
 * \post  Monitoring is disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void BlockWithExpiredTimeoutTrap::EndMonitoring(void)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  bool const trapped = pTFCCore->DisableWatchForAlreadyExpiredTimeout();
  enabled = false;

  if (trapped)
  {
    ADD_FAILURE() << "TFC: Trap for threads that attempt to block with already expired timeout was triggered." << std::endl;
  }
}

/**
 * \brief Constructor. Monitoring is disabled.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
PotentialUnreproducibleBehaviourTrap::PotentialUnreproducibleBehaviourTrap(void) noexcept
: enabled(false)
{
}

/**
 * \brief Destructor.
 *
 * \pre   Monitoring may be enabled or disabled.
 *
 * \post  Monitoring is disabled.
 *
 * \note  If monitoring was enabled and the trap was triggered, then the trapping event will be discarded.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
PotentialUnreproducibleBehaviourTrap::~PotentialUnreproducibleBehaviourTrap(void)
{
  if (enabled)
  {
    auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
    (void)pTFCCore->DisableWatchForBlockWithSameTimeout();
  }
}

/**
 * \brief Enables monitoring.
 *
 * \pre   Monitoring is disabled.
 *
 * \pre   There is no other @ref PotentialUnreproducibleBehaviourTrap instance in the process, which has monitoring
 *        enabled.
 *
 * \post  Monitoring is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void PotentialUnreproducibleBehaviourTrap::BeginMonitoring(void)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  pTFCCore->EnableWatchForBlockWithSameTimeout();
  enabled = true;
}

/**
 * \brief Disables monitoring and checks if the trap has been triggered.
 *
 * If the trap was triggered while it was enabled, then a failure will be added to the current unit test case.
 *
 * \pre   Monitoring is enabled.
 *
 * \post  Monitoring is disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void PotentialUnreproducibleBehaviourTrap::EndMonitoring(void)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  bool const trapped = pTFCCore->DisableWatchForBlockWithSameTimeout();
  enabled = false;

  if (trapped)
  {
    ADD_FAILURE() << "TFC: Trap for potential unreproducible behaviour was triggered." << std::endl
                  << "     The trap has been triggered, because at least two threads were blocked" << std::endl
                  << "     with same timeout.";
  }
}

/**
 * \brief Constructor. Monitoring is disabled.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
UnreproducibleBehaviourTrap::UnreproducibleBehaviourTrap(void) noexcept
: enabled(false)
{
}

/**
 * \brief Destructor.
 *
 * \pre   Monitoring may be enabled or disabled.
 *
 * \post  Monitoring is disabled.
 *
 * \note  If monitoring was enabled and the trap was triggered, then the trapping event will be discarded.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
UnreproducibleBehaviourTrap::~UnreproducibleBehaviourTrap(void)
{
  if (enabled)
  {
    auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
    (void)pTFCCore->DisableWatchForSimultaneousResumeOfMultipleThreads();
  }
}

/**
 * \brief Enables monitoring.
 *
 * \pre   Monitoring is disabled.
 *
 * \pre   There is no other @ref UnreproducibleBehaviourTrap instance in the process, which has monitoring enabled.
 *
 * \post  Monitoring is enabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void UnreproducibleBehaviourTrap::BeginMonitoring(void)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  pTFCCore->EnableWatchForSimultaneousResumeOfMultipleThreads();
  enabled = true;
}

/**
 * \brief Disables monitoring and checks if the trap has been triggered.
 *
 * If the trap was triggered while it was enabled, then a failure will be added to the current unit test case.
 *
 * \pre   Monitoring is enabled.
 *
 * \post  Monitoring is disabled.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void UnreproducibleBehaviourTrap::EndMonitoring(void)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  bool const trapped = pTFCCore->DisableWatchForSimultaneousResumeOfMultipleThreads();
  enabled = false;

  if (trapped)
  {
    ADD_FAILURE() << "TFC: Trap for unreproducible behaviour was triggered." << std::endl
                  << "     The trap has been triggered, because at least two threads were resumed" << std::endl
                  << "     at the same point in time.";
  }
}

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // #ifdef OS_LINUX_ARM_TFC
