/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)

#include <gpcc_test/osal/tfc/BlockWithExpiredTimeoutTrap.hpp>
#include "src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

namespace gpcc_tests {
namespace osal       {
namespace tfc        {

/**
 * \brief Constructor.
 *
 * After construction, use @ref BeginMonitoring() to start monitoring. Note that at any point in time there must be no
 * more than one @ref BlockWithExpiredTimeoutTrap instance in the process which has monitoring enabled.
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
 * If monitoring is enabled, then it will be disabled.
 *
 * \note  If monitoring is enabled and the trap has been triggered, then the trapping event will be discarded. There
 *        will be no failure added to the current unittest case. The recommended use is to invoke @ref EndMonitoring()
 *        before destroying the trap.
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
 * \post  Monitoring is enabled and the trap is _not triggered_.
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
 * \brief Queries if the trap has been triggered and resets the trigger.
 *
 * Query and reset are carried out as one atomic operation.
 *
 * \pre   Monitoring is enabled.
 *
 * \post  Monitoring is enabled and the trap is _not triggered_.
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
 *
 * - - -
 *
 * \retval true   The trap was triggered.
 * \retval false  The trap was not triggered.
 */
bool BlockWithExpiredTimeoutTrap::QueryAndReset(void)
{
  if (!enabled)
    throw std::logic_error("Trap is not enabled");

  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  return pTFCCore->QueryAndResetWatchForAlreadyExpiredTimeout();
}

/**
 * \brief Disables monitoring and adds a failure to the current unittest case, if the trap has been triggered while it
 *        was enabled.
 *
 * \pre   Monitoring is enabled.
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
  if (!enabled)
    throw std::logic_error("Trap is not enabled");

  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();
  bool const trapped = pTFCCore->DisableWatchForAlreadyExpiredTimeout();
  enabled = false;

  if (trapped)
  {
    ADD_FAILURE() << "TFC: Trap for threads that attempt to block with already expired timeout has been triggered." << std::endl;
  }
}

} // namespace tfc
} // namespace osal
} // namespace gpcc_tests

#endif // #if defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64_TFC)
