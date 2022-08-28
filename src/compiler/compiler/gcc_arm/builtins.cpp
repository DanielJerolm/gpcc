/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef COMPILER_GCC_ARM

#include <gpcc/compiler/compiler/gcc_arm/builtins.hpp>
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
