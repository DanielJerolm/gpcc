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

#include "int_math.hpp"

namespace gpcc {
namespace math {

/**
 * \ingroup GPCC_MATH
 * \brief Calculates the square root of an uint32_t.
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
 * \param x
 * Value whose square root shall be calculated.
 * \return
 * Square root of `x`.\n
 * Any fraction is truncated. Examples:
 * - sqrt32(8) = 2
 * - sqrt32(9) = 3
 */
uint16_t sqrt32(uint32_t const x) noexcept
{
  uint_fast16_t result = 0;
  uint_fast16_t testedBit = 0x8000U;

  for (uint_fast8_t i = 0; i < 16U; i++)
  {
    uint_fast32_t const guess = result | testedBit;
    uint_fast32_t const guess_square = guess * guess;
    if (x >= guess_square)
      result = guess;
    testedBit >>= 1U;
  }

  return result;
}

} // namespace math
} // namespace gpcc
