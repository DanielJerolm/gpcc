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

#ifdef COMPILER_GCC_X64

#ifndef BUILTINS_HPP_202206112145
#define BUILTINS_HPP_202206112145

#include <cstdint>
#include <limits>

namespace gpcc
{
namespace Compiler
{

namespace internal
{
  extern uint8_t const bitReverseTable[];
}

bool OverflowAwareAdd(int64_t const a, int64_t const b, int64_t * const pResult) noexcept;
bool OverflowAwareAdd(int64_t const a, int64_t const b, int32_t * const pResult) noexcept;
bool OverflowAwareSub(int64_t const a, int64_t const b, int64_t * const pResult) noexcept;
bool OverflowAwareSub(int64_t const a, int64_t const b, int32_t * const pResult) noexcept;

inline int CountLeadingZeros(unsigned int const x) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the leading zeros in a value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param x Value to be examined.
 * \return
 * Number of leading zeros in `x`.\n
 * Example: CountLeadingZeros(8) = 28 (on a machine where unsigned int is 32 bit)\n
 * If `x` is zero, than the number of bits in the underlying data type is returned.
 */
{
  if (x != 0)
    return __builtin_clz(x);
  else
    return std::numeric_limits<unsigned int>::digits;
}
inline int CountLeadingOnes(unsigned int const x) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the leading ones in a value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param x Value to be examined.
 * \return
 * Number of leading ones in `x`.\n
 * Example: CountLeadingZeros(0xFF...FF0) = 28 (on a machine where unsigned int is 32 bit)\n
 * If `x` is 0xFFF...FF (all ones), than the number of bits in the underlying data type is returned.
 */
{
  return CountLeadingZeros(~x);
}

inline int CountTrailingZeros(unsigned int const x) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the trailing zeros in a value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param x Value to be examined.
 * \return
 * Number of trailing zeros in `x`.\n
 * Example: CountTrailingZeros(8) = 3\n
 * If `x` is zero, than the number of bits in the underlying data type is returned.
 */
{
  if (x != 0)
    return __builtin_ctz(x);
  else
    return std::numeric_limits<unsigned int>::digits;
}
inline int CountTrailingOnes(unsigned int const x) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the trailing ones in a value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param x Value to be examined.
 * \return
 * Number of trailing ones in `x`.\n
 * Example: CountTrailingOnes(7) = 3\n
 * If `x` is 0xFFF...FF (all ones), than the number of bits in the underlying data type is returned.
 */
{
  return CountTrailingZeros(~x);
}

inline uint8_t ReverseBits8(uint8_t const value) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Reverses the bit order in an 8 bit value (abcdefgh => hgfedcba)
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
 * \param value
 * Input value.
 * \return
 * `value` with bits being reversed: abcdefgh => hgfedcba
 */
{
  return internal::bitReverseTable[value];
}

uint16_t ReverseBits16(uint16_t const value) noexcept;
uint32_t ReverseBits32(uint32_t const value) noexcept;


} // namespace Compiler
} // namespace gpcc

#endif /* BUILTINS_HPP_202206112145 */
#endif /* COMPILER_GCC_X64 */
