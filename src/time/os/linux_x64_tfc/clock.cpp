/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#include "clock.hpp"
#include "gpcc/src/osal/os/linux_x64_tfc/internal/TFCCore.hpp"

namespace gpcc {
namespace time {

/**
 * \ingroup GPCC_TIME
 * \brief Fetches the value of a system clock.
 *
 * Consider using class @ref TimePoint instead of using this function directly.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - `ts` may be set to a random value.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param clock ID of the clock that shall be read.
 * \param ts The fetched time is written into the referenced timespec structure.
 */
void GetTime(Clocks const clock, struct ::timespec& ts)
{
  auto pTFCCore = gpcc::osal::internal::TFCCore::Get();

  switch (clock)
  {
    case Clocks::realtime:
      pTFCCore->GetEmulatedRealtime(ts);
      break;

    case Clocks::monotonic:
    case Clocks::monotonicPrecise:
      pTFCCore->GetEmulatedMonotonicTime(ts);
      break;
  }
}

} // namespace time
} // namespace gpcc

#endif // #ifdef OS_LINUX_X64_TFC
