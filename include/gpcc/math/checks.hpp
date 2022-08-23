/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
