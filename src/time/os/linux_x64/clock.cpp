/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
*/

#ifdef OS_LINUX_X64

#include <gpcc/time/clock.hpp>
#include <gpcc/osal/Panic.hpp>

namespace {

clockid_t ToPosixClockID(gpcc::time::Clocks const clock) noexcept
{
  switch (clock)
  {
    case gpcc::time::Clocks::realtimeCoarse:   return CLOCK_REALTIME_COARSE;
    case gpcc::time::Clocks::realtimePrecise:  return CLOCK_REALTIME;
    case gpcc::time::Clocks::monotonicCoarse:  return CLOCK_MONOTONIC_COARSE;
    case gpcc::time::Clocks::monotonicPrecise: return CLOCK_MONOTONIC;
  }

  PANIC();
}

class ClockResolutionCache final
{
  public:
    uint32_t const resolution_clock_realtimeCoarse_ns;
    uint32_t const resolution_clock_realtimePrecise_ns;
    uint32_t const resolution_clock_monotonicCoarse_ns;
    uint32_t const resolution_clock_monotonicPrecise_ns;

    ClockResolutionCache(void) noexcept;

    uint32_t Get_ns(gpcc::time::Clocks const clock) const noexcept;

  private:
    static uint32_t Query(gpcc::time::Clocks const clock) noexcept;
};

ClockResolutionCache::ClockResolutionCache(void) noexcept
: resolution_clock_realtimeCoarse_ns(Query(gpcc::time::Clocks::realtimeCoarse))
, resolution_clock_realtimePrecise_ns(Query(gpcc::time::Clocks::realtimePrecise))
, resolution_clock_monotonicCoarse_ns(Query(gpcc::time::Clocks::monotonicCoarse))
, resolution_clock_monotonicPrecise_ns(Query(gpcc::time::Clocks::monotonicPrecise))
{
}

uint32_t ClockResolutionCache::Query(gpcc::time::Clocks const clock) noexcept
{
  struct ::timespec ts;
  int const ret = clock_getres(ToPosixClockID(clock), &ts);
  if (ret != 0)
    PANIC();

  if ((ts.tv_sec != 0) || (ts.tv_nsec <= 0) || (ts.tv_nsec >= 1000000000L))
    PANIC();

  return static_cast<uint32_t>(ts.tv_nsec);
}

uint32_t ClockResolutionCache::Get_ns(gpcc::time::Clocks const clock) const noexcept
{
  switch (clock)
  {
    case gpcc::time::Clocks::realtimeCoarse:   return resolution_clock_realtimeCoarse_ns;
    case gpcc::time::Clocks::realtimePrecise:  return resolution_clock_realtimePrecise_ns;
    case gpcc::time::Clocks::monotonicCoarse:  return resolution_clock_monotonicCoarse_ns;
    case gpcc::time::Clocks::monotonicPrecise: return resolution_clock_monotonicPrecise_ns;
  }

  PANIC();
}

} // namespace internal

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
  static ClockResolutionCache cache;
  return cache.Get_ns(clock);
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
  int const ret = clock_gettime(ToPosixClockID(clock), &ts);
  if (ret != 0)
    PANIC();
}

} // namespace time
} // namespace gpcc

#endif // #ifdef OS_LINUX_X64
