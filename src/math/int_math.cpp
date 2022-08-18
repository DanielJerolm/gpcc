/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
