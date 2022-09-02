/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64

#ifndef CLOCK_HPP_201701162119
#define CLOCK_HPP_201701162119

#include <ctime>

namespace gpcc {
namespace time {

/**
 * \ingroup GPCC_TIME
 * \brief Enumeration with clocks available in the system.
 *
 * Note:\n
 * The available clocks depend on the system.\n
 * 'realtime' and 'monotonic' are available on each platform. On some platforms,
 * more clocks may be available.
 */
enum class Clocks
{
  realtime          = CLOCK_REALTIME,   ///<System time.
  monotonic         = CLOCK_MONOTONIC,  ///<Monotonic rising time (not any jumps) starting at some arbitrary point in time.
  monotonicPrecise  = CLOCK_MONOTONIC   ///<Like @ref Clocks::monotonic, but with highest available precision.
};

void GetTime(Clocks const clock, struct ::timespec& ts);

} // namespace time
} // namespace gpcc

#endif // #ifndef CLOCK_HPP_201701162119
#endif // #ifdef OS_LINUX_X64
