/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#include "levenshtein_distance.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cctype>
#include <cstdint>
#include <cstring>

namespace gpcc {
namespace string {

/**
 * \brief Calculates the Levenshtein Distance between two strings.
 *
 * The Levenshtein Distance is the minimum number of insert-, remove-, and replace-operations required
 * to convert one given string into a second given string.
 *
 * For performance reasons, the caller should check the two strings for equality (Levenshtein Distance 0)
 * and save the call to this method if the strings are equal (this is not strictly required).
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \tparam T
 * Smallest available unsigned data type capable to hold `lenS1`+1 and `lenS2`+1.
 * \param pS1
 * Pointer to the first string.
 * \param lenS1
 * Length of the first string (without null-terminator).
 * \param pS2
 * Pointer to the second string.
 * \param lenS2
 * Length of the second string (without null-terminator).
 * \param caseSensitive
 * Controls if the calculation shall be case sensitive (true) or not (false).
 * \return
 * Levenshtein distance between the strings referenced by `pS1` and `pS2`.
 */
template <typename T>
static T LevenshteinDistance_Core_T(char const * pS1, T lenS1,
                                    char const * pS2, T lenS2,
                                    bool const caseSensitive)
{
  // check special cases
  if (lenS1 == 0)
    return lenS2;

  if (lenS2 == 0)
    return lenS1;

  // check constraints
  if ((lenS1 == std::numeric_limits<T>::max()) || (lenS2 == std::numeric_limits<T>::max()))
    throw std::invalid_argument("LevenshteinDistance_Core: Constraints T<->lenS1/T<->lenS2 violated");

  // ensure that the first string is larger (will result in smaller v0 and v1)
  if (lenS2 > lenS1)
  {
    std::swap<char const *>(pS1, pS2);
    std::swap<T>(lenS1, lenS2);
  }

  std::vector<T> v0(lenS2 + 1U);
  std::vector<T> v1(lenS2 + 1U);

  for (T i = 0; i <= lenS2; i++)
    v0[i] = i;

  for (T i = 0; i < lenS1; i++)
  {
    v1[0] = i + 1U;

    char cS1 = pS1[i];
    if (!caseSensitive)
      cS1 = toupper(cS1);

    for (T j = 0; j < lenS2; j++)
    {
      T cost;
      if (caseSensitive)
        cost = (cS1 == pS2[j]) ? 0 : 1;
      else
        cost = (cS1 == toupper(pS2[j])) ? 0 : 1;

      v1[j + 1U] = std::min(std::min(v1[j], v0[j + 1U]) + static_cast<T>(1), v0[j] + cost);
    }

    v0.swap(v1);
  }

  return v0[lenS2];
}

/**
 * \brief Calculates the Levenshtein Distance between two strings.
 *
 * The Levenshtein Distance is the minimum number of insert-, remove-, and replace-operations required
 * to convert one given string into a second given string.
 *
 * For performance reasons, the caller should check the two strings for equality (Levenshtein Distance 0)
 * and save the call to this method if the strings are equal (this is not strictly required).
 *
 * This method chooses the smallest possible data type for T and delegates the call to @ref LevenshteinDistance_Core_T.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pS1
 * Pointer to the first string.
 * \param lenS1
 * Length of the first string (without null-terminator).
 * \param pS2
 * Pointer to the second string.
 * \param lenS2
 * Length of the second string (without null-terminator).
 * \param caseSensitive
 * Controls if the calculation shall be case sensitive (true) or not (false).
 * \return
 * Levenshtein distance between the strings referenced by `pS1` and `pS2`.
 */
static size_t LevenshteinDistance_Core(char const * pS1, size_t lenS1,
                                       char const * pS2, size_t lenS2,
                                       bool const caseSensitive)
{
  size_t const max = std::max(lenS1, lenS2);

  if (max < std::numeric_limits<uint8_t>::max())
    return LevenshteinDistance_Core_T<uint8_t>(pS1, static_cast<uint8_t>(lenS1), pS2, static_cast<uint8_t>(lenS2), caseSensitive);
  else if (max < std::numeric_limits<uint16_t>::max())
    return LevenshteinDistance_Core_T<uint16_t>(pS1, static_cast<uint16_t>(lenS1), pS2, static_cast<uint16_t>(lenS2), caseSensitive);
  else if (max < std::numeric_limits<uint32_t>::max())
    return LevenshteinDistance_Core_T<uint32_t>(pS1, static_cast<uint32_t>(lenS1), pS2, static_cast<uint32_t>(lenS2), caseSensitive);
  else
    return LevenshteinDistance_Core_T<size_t>(pS1, lenS1, pS2, lenS2, caseSensitive);
}

/**
 * \ingroup GPCC_STRING
 * \brief Calculates the Levenshtein Distance between two strings.
 *
 * The Levenshtein Distance is the minimum number of insert-, remove-, and replace-operations required
 * to convert one given string into a second given string.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param str1
 * Reference to the first string.
 * \param str2
 * Reference to the second string.
 * \param caseSensitive
 * Controls if the calculation shall be case sensitive (true) or not (false).
 * \return
 * Levenshtein distance between the strings referenced by `str1` and `str2`.
 */
size_t LevenshteinDistance(std::string const & str1, std::string const & str2, bool const caseSensitive)
{
  if (str1 == str2)
    return 0;

  return LevenshteinDistance_Core(str1.c_str(), str1.length(), str2.c_str(), str2.length(), caseSensitive);
}

/**
 * \ingroup GPCC_STRING
 * \brief Calculates the Levenshtein Distance between two strings.
 *
 * The Levenshtein Distance is the minimum number of insert-, remove-, and replace-operations required
 * to convert one given string into a second given string.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param str1
 * Reference to the first string.
 * \param pStr2
 * Pointer to the second string. This must refer to a null-terminated c-string.
 * \param caseSensitive
 * Controls if the calculation shall be case sensitive (true) or not (false).
 * \return
 * Levenshtein distance between the strings referenced by `str1` and `pStr2`.
 */
size_t LevenshteinDistance(std::string const & str1, char const * const pStr2, bool const caseSensitive)
{
  if (str1.compare(pStr2) == 0)
    return 0;

  return LevenshteinDistance_Core(str1.c_str(), str1.length(), pStr2, strlen(pStr2), caseSensitive);
}

size_t LevenshteinDistance(char const * const pStr1, char const * const pStr2, bool const caseSensitive)
/**
 * \ingroup GPCC_STRING
 * \brief Calculates the Levenshtein Distance between two strings.
 *
 * The Levenshtein Distance is the minimum number of insert-, remove-, and replace-operations required
 * to convert one given string into a second given string.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \param pStr1
 * Pointer to the first string. This must refer to a null-terminated c-string.
 * \param pStr2
 * Pointer to the second string. This must refer to a null-terminated c-string.
 * \param caseSensitive
 * Controls if the calculation shall be case sensitive (true) or not (false).
 * \return Levenshtein distance between the strings referenced by `pStr1` and `pStr2`.
 */
{
  if (strcmp(pStr1, pStr2) == 0)
    return 0;

  return LevenshteinDistance_Core(pStr1, strlen(pStr1), pStr2, strlen(pStr2), caseSensitive);
}

} // namespace string
} // namespace gpcc
