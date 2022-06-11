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

#ifdef OS_LINUX_X64

#include "clock.hpp"
#include <system_error>
#include <cerrno>

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
  int const ret = clock_gettime(static_cast<clockid_t>(clock), &ts);
  if (ret != 0)
    throw std::system_error(errno, std::generic_category(), "GetTime: clock_gettime failed");
}

} // namespace time
} // namespace gpcc

#endif // #ifdef OS_LINUX_X64
