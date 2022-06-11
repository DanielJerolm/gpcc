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

#include "builtins.hpp"

namespace gpcc
{
namespace Compiler
{

namespace internal
{
  // Table for reversal of the bits of a byte.
  uint8_t const bitReverseTable[] =
  {
    0x00U, 0x80U, 0x40U, 0xC0U, 0x20U, 0xA0U, 0x60U, 0xE0U, 0x10U, 0x90U, 0x50U, 0xD0U, 0x30U, 0xB0U, 0x70U, 0xF0U,
    0x08U, 0x88U, 0x48U, 0xC8U, 0x28U, 0xA8U, 0x68U, 0xE8U, 0x18U, 0x98U, 0x58U, 0xD8U, 0x38U, 0xB8U, 0x78U, 0xF8U,
    0x04U, 0x84U, 0x44U, 0xC4U, 0x24U, 0xA4U, 0x64U, 0xE4U, 0x14U, 0x94U, 0x54U, 0xD4U, 0x34U, 0xB4U, 0x74U, 0xF4U,
    0x0CU, 0x8CU, 0x4CU, 0xCCU, 0x2CU, 0xACU, 0x6CU, 0xECU, 0x1CU, 0x9CU, 0x5CU, 0xDCU, 0x3CU, 0xBCU, 0x7CU, 0xFCU,
    0x02U, 0x82U, 0x42U, 0xC2U, 0x22U, 0xA2U, 0x62U, 0xE2U, 0x12U, 0x92U, 0x52U, 0xD2U, 0x32U, 0xB2U, 0x72U, 0xF2U,
    0x0AU, 0x8AU, 0x4AU, 0xCAU, 0x2AU, 0xAAU, 0x6AU, 0xEAU, 0x1AU, 0x9AU, 0x5AU, 0xDAU, 0x3AU, 0xBAU, 0x7AU, 0xFAU,
    0x06U, 0x86U, 0x46U, 0xC6U, 0x26U, 0xA6U, 0x66U, 0xE6U, 0x16U, 0x96U, 0x56U, 0xD6U, 0x36U, 0xB6U, 0x76U, 0xF6U,
    0x0EU, 0x8EU, 0x4EU, 0xCEU, 0x2EU, 0xAEU, 0x6EU, 0xEEU, 0x1EU, 0x9EU, 0x5EU, 0xDEU, 0x3EU, 0xBEU, 0x7EU, 0xFEU,
    0x01U, 0x81U, 0x41U, 0xC1U, 0x21U, 0xA1U, 0x61U, 0xE1U, 0x11U, 0x91U, 0x51U, 0xD1U, 0x31U, 0xB1U, 0x71U, 0xF1U,
    0x09U, 0x89U, 0x49U, 0xC9U, 0x29U, 0xA9U, 0x69U, 0xE9U, 0x19U, 0x99U, 0x59U, 0xD9U, 0x39U, 0xB9U, 0x79U, 0xF9U,
    0x05U, 0x85U, 0x45U, 0xC5U, 0x25U, 0xA5U, 0x65U, 0xE5U, 0x15U, 0x95U, 0x55U, 0xD5U, 0x35U, 0xB5U, 0x75U, 0xF5U,
    0x0DU, 0x8DU, 0x4DU, 0xCDU, 0x2DU, 0xADU, 0x6DU, 0xEDU, 0x1DU, 0x9DU, 0x5DU, 0xDDU, 0x3DU, 0xBDU, 0x7DU, 0xFDU,
    0x03U, 0x83U, 0x43U, 0xC3U, 0x23U, 0xA3U, 0x63U, 0xE3U, 0x13U, 0x93U, 0x53U, 0xD3U, 0x33U, 0xB3U, 0x73U, 0xF3U,
    0x0BU, 0x8BU, 0x4BU, 0xCBU, 0x2BU, 0xABU, 0x6BU, 0xEBU, 0x1BU, 0x9BU, 0x5BU, 0xDBU, 0x3BU, 0xBBU, 0x7BU, 0xFBU,
    0x07U, 0x87U, 0x47U, 0xC7U, 0x27U, 0xA7U, 0x67U, 0xE7U, 0x17U, 0x97U, 0x57U, 0xD7U, 0x37U, 0xB7U, 0x77U, 0xF7U,
    0x0FU, 0x8FU, 0x4FU, 0xCFU, 0x2FU, 0xAFU, 0x6FU, 0xEFU, 0x1FU, 0x9FU, 0x5FU, 0xDFU, 0x3FU, 0xBFU, 0x7FU, 0xFFU
  };
}

bool OverflowAwareAdd(int64_t const a, int64_t const b, int64_t * const pResult) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Overflow-aware addition of `int64_t` and `int64_t` resulting in `int64_t`.
 *
 * This is intended to be replaced by `__builtin_add_overflow(...)` in the future, as soon as a
 * suitable version of gcc is available.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param a First operand.
 * \param b Second operand.
 * \param pResult
 * The result is stored into the referenced variable.\n
 * _Be aware that future versions of this function may set the referenced variable to an_
 * _undefined value even though they detect and report an overflow._
 * \return
 * true  = overflow occurred
 * false = no overflow occurred
 */
{
  if ((a > 0) && (b > 0))
  {
    if (a > std::numeric_limits<int64_t>::max() - b)
      return true;
  }
  else if ((a < 0) && (b < 0))
  {
    if (a < std::numeric_limits<int64_t>::min() - b)
      return true;
  }

  *pResult = a + b;
  return false;
}
bool OverflowAwareAdd(int64_t const a, int64_t const b, int32_t * const pResult) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Overflow-aware addition of `int64_t` and `int64_t` resulting in `int32_t`.
 *
 * This is intended to be replaced by `__builtin_add_overflow(...)` in the future, as soon as a
 * suitable version of gcc is available.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param a First operand.
 * \param b Second operand.
 * \param pResult
 * The result is stored into the referenced variable.\n
 * _Be aware that future versions of this function may set the referenced variable to an_
 * _undefined value even though they detect and report an overflow._
 * \return
 * true  = overflow occurred
 * false = no overflow occurred
 */
{
  if ((a > 0) && (b > 0))
  {
    if (a > std::numeric_limits<int32_t>::max() - b)
      return true;
    *pResult = a + b;
  }
  else if ((a < 0) && (b < 0))
  {
    if (a < std::numeric_limits<int32_t>::min() - b)
      return true;
    *pResult = a + b;
  }
  else
  {
    int64_t const result = a + b;
    if ((result < std::numeric_limits<int32_t>::min()) ||
        (result > std::numeric_limits<int32_t>::max()))
      return true;
    *pResult = result;
  }

  return false;
}
bool OverflowAwareSub(int64_t const a, int64_t const b, int64_t * const pResult) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Overflow-aware subtraction of `int64_t` and `int64_t` resulting in `int64_t`.
 *
 * This is intended to be replaced by `__builtin_sub_overflow(...)` in the future, as soon as a
 * suitable version of gcc is available.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param a First operand.
 * \param b Second operand.
 * \param pResult
 * The result is stored into the referenced variable.\n
 * _Be aware that future versions of this function may set the referenced variable to an_
 * _undefined value even though they detect and report an overflow._
 * \return
 * true  = overflow occurred
 * false = no overflow occurred
 */
{
  if ((a > 0) && (b < 0))
  {
    if (b < a - std::numeric_limits<int64_t>::max())
      return true;
  }
  else if ((a < 0) && (b > 0))
  {
    if (a < b - std::numeric_limits<int64_t>::max())
      return true;
  }

  *pResult = a - b;
  return false;
}
bool OverflowAwareSub(int64_t const a, int64_t const b, int32_t * const pResult) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Overflow-aware subtraction of `int64_t` and `int64_t` resulting in `int32_t`.
 *
 * This is intended to be replaced by `__builtin_sub_overflow(...)` in the future, as soon as a
 * suitable version of gcc is available.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is reentrant if different data is used.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param a First operand.
 * \param b Second operand.
 * \param pResult
 * The result is stored into the referenced variable.\n
 * _Be aware that future versions of this function may set the referenced variable to an_
 * _undefined value even though they detect and report an overflow._
 * \return
 * true  = overflow occurred
 * false = no overflow occurred
 */
{
  if ((a > 0) && (b < 0))
  {
    if (b < a - std::numeric_limits<int32_t>::max())
      return true;
    *pResult = a - b;
  }
  else if ((a < 0) && (b > 0))
  {
    if (a < b - std::numeric_limits<int32_t>::max())
      return true;
    *pResult = a - b;
  }
  else
  {
    int64_t const result = a - b;
    if ((result < std::numeric_limits<int32_t>::min()) ||
        (result > std::numeric_limits<int32_t>::max()))
      return true;
    *pResult = result;
  }

  return false;
}

uint16_t ReverseBits16(uint16_t const value) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Reverses the bit order in an 16 bit value.
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
 * `value` with bits being reversed.
 */
{
  return (static_cast<uint16_t>(ReverseBits8((value >>  0U) & 0xFFU)) << 8U) |
         (static_cast<uint16_t>(ReverseBits8((value >>  8U) & 0xFFU)) << 0U);
}

uint32_t ReverseBits32(uint32_t const value) noexcept
/**
 * \ingroup GPCC_COMPILER_BUILTINS
 * \brief Reverses the bit order in an 32 bit value.
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
 * `value` with bits being reversed.
 */
{
  return (static_cast<uint32_t>(ReverseBits8((value >>  0U) & 0xFFU)) << 24U) |
         (static_cast<uint32_t>(ReverseBits8((value >>  8U) & 0xFFU)) << 16U) |
         (static_cast<uint32_t>(ReverseBits8((value >> 16U) & 0xFFU)) <<  8U) |
         (static_cast<uint32_t>(ReverseBits8((value >> 24U) & 0xFFU)) <<  0U);
}

} // namespace Compiler
} // namespace gpcc

#endif /* COMPILER_GCC_X64 */
