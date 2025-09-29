/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR

#include <gpcc/time/clock.hpp>
#include <gpcc/osal/Panic.hpp>
#include <zephyr/sys/clock.h>

namespace gpcc {
namespace time {

/**
 * \ingroup GPCC_TIME
 * \brief Queries the precision of a clock.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param clock
 * Clock whose precision shall be queried.
 *
 * \return
 * Minimum precision of any reading from the clock specified by `clock` in ns.
 */
uint32_t GetPrecision_ns(Clocks const clock) noexcept
{
  (void)clock;
  return 1000000UL; // 1ms
}

/**
 * \ingroup GPCC_TIME
 * \brief Reads the time from a clock.
 *
 * Consider using class @ref TimePoint instead of using this function directly.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param clock
 * ID of the clock that shall be read.
 *
 * \param ts
 * The reading is written into the referenced timespec structure.
 */
void GetTime(Clocks const clock, struct ::timespec& ts) noexcept
{
  int status = 1;
  switch (clock)
  {
    case Clocks::realtimeCoarse:
      // intentional fall-through
    case Clocks::realtimePrecise:
      status = sys_clock_gettime(SYS_CLOCK_REALTIME, &ts);
      break;

    case Clocks::monotonicCoarse:
    // intentional fall-through
    case Clocks::monotonicPrecise:
      status = sys_clock_gettime(SYS_CLOCK_MONOTONIC, &ts);
      break;
  }

  if (status != 0)
    gpcc::osal::Panic();
}

} // namespace time
} // namespace gpcc

#endif // #ifdef OS_ZEPHYR
