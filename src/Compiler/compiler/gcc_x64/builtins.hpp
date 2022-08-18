/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
