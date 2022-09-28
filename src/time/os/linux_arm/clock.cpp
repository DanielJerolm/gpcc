/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM

#include <gpcc/time/clock.hpp>
#include <gpcc/osal/Panic.hpp>

namespace gpcc {
namespace time {

static clockid_t ToClockID(Clocks const clock) noexcept
{
  switch (clock)
  {
    case Clocks::realtime:         return CLOCK_REALTIME_COARSE;
    case Clocks::realtimePrecise:  return CLOCK_REALTIME;
    case Clocks::monotonic:        return CLOCK_MONOTONIC_COARSE;
    case Clocks::monotonicPrecise: return CLOCK_MONOTONIC;
  }
}

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
  struct ::timespec ts;

  int const ret = clock_getres(ToClockID(clock), &ts);
  if (ret != 0)
    PANIC();

  if ((ts.tv_sec != 0) || (ts.tv_nsec <= 0) || (ts.tv_nsec >= 1000000000L))
    PANIC();

  return static_cast<uint32_t>(ts.tv_nsec);
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
  int const ret = clock_gettime(ToClockID(clock), &ts);
  if (ret != 0)
    PANIC();
}

} // namespace time
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM
