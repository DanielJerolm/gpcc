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

#ifndef CHECKS_HPP_201701161920
#define CHECKS_HPP_201701161920

#include <limits>
#include <cstdint>

namespace gpcc {
namespace math {

/**
 * \ingroup GPCC_MATH
 * \brief Checks if an unsigned value is a power of 2.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \tparam T
 * Type of the value that shall be checked.\n
 * _This must be an unsigned type. For signed type this method generated undefined results._
 * \param value Value to be checked.
 * \return
 * true  = `value` is zero or a power of 0.\n
 * false = `value` if neither zero, nor a power of 2.
 */
template<typename T>
bool IsPowerOf2(T const value) noexcept
{
  return (value == 0U) || ((value & (value - 1U)) == 0U);
}

} // namespace math
} // namespace gpcc

#endif // CHECKS_HPP_201701161920
