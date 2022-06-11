/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#ifdef OS_CHIBIOS_ARM

#ifndef CLOCK_HPP_201701162118
#define CLOCK_HPP_201701162118

#ifndef _POSIX_MONOTONIC_CLOCK
#error "_POSIX_MONOTONIC_CLOCK is required"
#endif

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
  realtime,         ///<System time.
  monotonic,        ///<Monotonic rising time (not any jumps) starting at some arbitrary point in time.
  monotonicPrecise  ///<Like @ref Clocks::monotonic, but with highest available precision.
};

void GetTime(Clocks const clock, struct ::timespec& ts);

} // namespace time
} // namespace gpcc

#endif /* CLOCK_HPP_201701162118 */
#endif /* #ifdef OS_CHIBIOS_ARM */
