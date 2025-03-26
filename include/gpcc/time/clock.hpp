/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
*/

#ifndef CLOCK_HPP_201701162120
#define CLOCK_HPP_201701162120

#include <cstdint>
#include <ctime>

namespace gpcc {
namespace time {

/**
 * \ingroup GPCC_TIME
 * \brief Enumeration of clocks.
 *
 * Note:
 * - On some platforms, the precise variants of the clocks are more expensive to read.\n
 *   If you do not need a high-precision clock reading, then the non-precise variants should be preferred.
 * - The precision of a clock can be queried via @ref gpcc::time::GetPrecision_ns(). \n
 *   On systems with a periodic system tick, the non-precise variant typically has a precision of one tick period.
 * - If the precise (non-precise) variant of a clock is not available on a platform, then GPCC will use the non-precise
 *   (precise) variant as a substitution.
 * - The precise and non-precise variants of a clock have the same base.\n
 *   It is safe to compare them against each other and to calculate time-spans from a mix of both, __but note:__\n
 *   There is a delta between the precise variant and the non-precise variant:\n
 *   `delta = precise time - non-precise time`\n
 *   The delta is always positive. Its maximum value is the precision of the non-precise variant of the clock.
 */
enum class Clocks
{
  realtimeCoarse,   ///<UTC system time.
  realtimePrecise,  ///<Like @ref Clocks::realtimeCoarse, but with highest available precision.
  monotonicCoarse,  ///<Monotonic rising time (not any jumps) starting at some arbitrary point in time.
                    /**<It has no jumps, but may change in speed due to NTP on some systems. */
  monotonicPrecise  ///<Like @ref Clocks::monotonicCoarse, but with highest available precision.
};

uint32_t GetPrecision_ns(Clocks const clock) noexcept;
void GetTime(Clocks const clock, struct ::timespec& ts) noexcept;

} // namespace time
} // namespace gpcc

#endif // CLOCK_HPP_201701162120
