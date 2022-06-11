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

#ifdef COMPILER_GCC_ARM

#include "builtins.hpp"
#include <limits>

namespace gpcc
{
namespace Compiler
{

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

} // namespace Compiler
} // namespace gpcc

#endif /* COMPILER_GCC_ARM */
