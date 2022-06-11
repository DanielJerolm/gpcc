/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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
