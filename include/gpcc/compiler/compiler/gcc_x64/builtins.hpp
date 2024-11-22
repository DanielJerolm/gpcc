/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef COMPILER_GCC_X64

#ifndef BUILTINS_HPP_202206112145
#define BUILTINS_HPP_202206112145

#include <limits>
#include <type_traits>
#include <cstdint>

namespace gpcc     {
namespace compiler {

namespace internal
{
  extern uint8_t const bitReverseTable[];
}

/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Overflow-aware addition.
 *
 * The types @p TA, @p TB, and @p TRES may all be different types.\n
 * The width of @p TRES may be less that @p TA and/or @p TB.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different pointers @p pResult are used.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam TA   Type of first operand @p a.
 * \tparam TB   Type of second operand @p b.
 * \tparam TRES Type of result.
 *
 * \param a
 * First operand.
 *
 * \param b
 * Second operand.
 *
 * \param pResult
 * The result is stored into the referenced variable.\n
 * `nullptr` is not allowed.
 *
 * \retval false  No overflow occurred.
 * \retval true   Arithmetic overflow. Undefined data may have been written to @p pResult.
 */
template<typename TA, typename TB, typename TRES>
bool OverflowAwareAdd(TA const a, TB const b, TRES* const pResult) noexcept
{
  return __builtin_add_overflow(a, b, pResult);
}

/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Overflow-aware subtraction.
 *
 * The types @p TA, @p TB, and @p TRES may all be different types.\n
 * The width of @p TRES may be less that @p TA and/or @p TB.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant if different pointers @p pResult are used.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam TA   Type of first operand @p a.
 * \tparam TB   Type of second operand @p b.
 * \tparam TRES Type of result.
 *
 * \param a
 * First operand.
 *
 * \param b
 * Second operand.
 *
 * \param pResult
 * The result of the subtraction `a - b` is stored into the referenced variable.\n
 * `nullptr` is not allowed.
 *
 * \retval false  No overflow occurred.
 * \retval true   Arithmetic overflow. Undefined data may have been written to @p pResult.
 */
template<typename T1, typename T2, typename T3>
bool OverflowAwareSub(T1 const a, T2 const b, T3* const pResult) noexcept
{
  return __builtin_sub_overflow(a, b, pResult);
}

/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the leading zeros in a value.
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
 * \tparam T
 * Type of the value that shall be examined.\n
 * The type must be an integral unsigned type.
 *
 * \param x
 * Value to be examined.
 *
 * \return
 * Number of leading zeros in @p x. \n
 * If @p x is zero, then the bit width of @p T is returned.
 */
template<typename T>
int CountLeadingZeros(T const x) noexcept
{
  static_assert(std::is_integral_v<T> == true, "CountLeadingZeros() is only defined for unsigned integral types");
  static_assert(std::is_unsigned_v<T> == true, "CountLeadingZeros() is undefined for signed types");

  if (x != 0)
  {
    static_assert(std::numeric_limits<unsigned long long>::digits == 64);
    return __builtin_clzll(x) - (64 - std::numeric_limits<T>::digits);
  }
  else
  {
    return std::numeric_limits<T>::digits;
  }
}

/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the leading ones in a value.
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
 * \tparam T
 * Type of the value that shall be examined.\n
 * The type must be an integral unsigned type.
 *
 * \param x
 * Value to be examined.
 *
 * \return
 * Number of leading ones in @p x. \n
 * If @p x is all '1', then the bit width of @p T is returned.
 */
template<typename T>
int CountLeadingOnes(T const x) noexcept
{
  return CountLeadingZeros(static_cast<T>(~x));
}

/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the trailing zeros in a value.
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
 * \tparam T
 * Type of the value that shall be examined.\n
 * The type must be an integral unsigned type.
 *
 * \param x
 * Value to be examined.
 *
 * \return
 * Number of trailing zeros in @p x. \n
 * If @p x is zero, then the bit width of @p T is returned.
 */
template<typename T>
int CountTrailingZeros(T const x) noexcept
{
  static_assert(std::is_integral_v<T> == true, "CountTrailingZeros() is only defined for unsigned integral types");
  static_assert(std::is_unsigned_v<T> == true, "CountTrailingZeros() is undefined for signed types");

  if (x != 0)
  {
    static_assert(std::numeric_limits<unsigned long long>::digits == 64);
    return __builtin_ctzll(x);
  }
  else
  {
    return std::numeric_limits<T>::digits;
  }
}

/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Counts the trailing ones in a value.
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
 * \tparam T
 * Type of the value that shall be examined.\n
 * The type must be an integral unsigned type.
 *
 * \param x
 * Value to be examined.
 *
 * \return
 * Number of trailing ones in @p x. \n
 * If @p x is all '1', then the bit width of @p T is returned.
 */
template<typename T>
int CountTrailingOnes(T const x) noexcept
{
  return CountTrailingZeros(static_cast<T>(~x));
}

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
 *
 * \return
 * @p value with bits being reversed: abcdefgh => hgfedcba
 */
inline uint8_t ReverseBits8(uint8_t const value) noexcept
{
  return internal::bitReverseTable[value];
}

uint16_t ReverseBits16(uint16_t const value) noexcept;
uint32_t ReverseBits32(uint32_t const value) noexcept;


} // namespace compiler
} // namespace gpcc

#endif // BUILTINS_HPP_202206112145
#endif // COMPILER_GCC_X64
