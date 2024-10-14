/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_EPOS_ARM

#include <gpcc/time/clock.hpp>
#include <gpcc/osal/Panic.hpp>
#include <epos/time/time.h>
#include <stdexcept>

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
  switch (clock)
  {
    case Clocks::realtimeCoarse:   return epos_time_GetResolutionOfRealtimeClock_ns();
    case Clocks::realtimePrecise:  return epos_time_GetResolutionOfPreciseRealtimeClock_ns();
    case Clocks::monotonicCoarse:  return epos_time_GetResolutionOfMonotonicClock_ns();
    case Clocks::monotonicPrecise: return epos_time_GetResolutionOfPreciseMonotonicClock_ns();
  }

  gpcc::osal::Panic();
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
  switch (clock)
  {
    case Clocks::realtimeCoarse:   epos_time_ReadRealtimeClock(&ts); break;
    case Clocks::realtimePrecise:  epos_time_ReadPreciseRealtimeClock(&ts); break;
    case Clocks::monotonicCoarse:  epos_time_ReadMonotonicClock(&ts); break;
    case Clocks::monotonicPrecise: epos_time_ReadPreciseMonotonicClock(&ts); break;
  }
}

} // namespace time
} // namespace gpcc

#endif // #ifdef OS_EPOS_ARM
