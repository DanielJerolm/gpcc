/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#include <gpcc/string/tools.hpp>
#include <gpcc/osal/definitions.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <limits>
#include <stdexcept>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace
{

/**
 * \ingroup GPCC_STRING
 * \brief Appends the description (returned by `what()`) of an exception and all nested exceptions (if any) to an
 *        @ref gpcc::string::StringComposer.
 *
 * This is used by @ref ExceptionDescriptionToString().
 *
 * \note  If the number of nested exceptions exceeds @ref maxDepthForExceptionToStringTranslation, then evaluation
 *        of nested exceptions will stop and a message will be appended to the returned string:\n
 *        `**Max depth reached. Skipping the rest**`
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Incomplete or undefined text may have been written to @p s.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param e
 * Reference to the exception whose description (returned by `what()`) shall be appended to @p s. \n
 * The descriptions of all nested exceptions (if any) will also be appended to @p s. \n
 * Each nested exception's description will start on a new line.
 *
 * \param level
 * Nesting level. It is prepended to the text output to indicate the nesting level.
 *
 * \param s
 * Reference to the @ref StringComposer to which the descriptions of the exceptions shall be appended to.
 */
void ExceptionDescriptionToStringHelper(std::exception const & e, size_t level, gpcc::string::StringComposer & s)
{
  s << level << ": " << e.what();

  try
  {
    std::rethrow_if_nested(e);
  }
  catch (std::exception const & e2)
  {
    s << gpcc::osal::endl;

    if (level < gpcc::string::maxDepthForExceptionToStringTranslation)
    {
      ExceptionDescriptionToStringHelper(e2, level + 1U, s);
    }
    else
    {
      s << "**Max depth reached. Skipping the rest**";
    }
  }
  catch (...)
  {
    s << gpcc::osal::endl;
    s << (level + 1U) << ": " << "Unknown exception";
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Throws an `std::invalid_argument` with verbose description indicating that @p s contains an invalid
 *        representation of a number.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * This always throws.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the invalid representation of a number.
 */
void ThrowInvalidNumberRepresentation(std::string const & s)
{
  gpcc::string::StringComposer sc;
  sc << "Invalid number/format: \"" << s << '\"';
  throw std::invalid_argument(sc.Get());
}

/**
 * \ingroup GPCC_STRING
 * \brief Throws an `std::out_of_range` with verbose description indicating that @p s contains a number that is out of
 *        range [`min`;`max`].
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * This always throws.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam T
 * Type of @p min and @p max.
 *
 * \param s
 * String containing the representation of the number.
 *
 * \param min
 * Minimum value.
 *
 * \param max
 * Maximum value.
 */
template <typename T>
void ThrowOutOfRange(std::string const & s, T const min, T const max)
{
  gpcc::string::StringComposer sc;
  sc << "Value '" << s << "' is out of range [" << min << ';' << max << ']';
  throw std::out_of_range(sc.Get());
}

/**
 * \ingroup GPCC_STRING
 * \brief Wrapper for `std::stoul` adding verbose error checks.
 *
 * Additional functionality:
 * - Checks if the result is within a given range [`min`;`max`].
 * - Does not accept any leading or trailing extra characters (incl. whitespace characters), except for one optional
 *   leading sign character (+/-).
 * - Throws verbose exceptions in case of any error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint32_t`.
 *
 * \param base
 * Base for interpreting @p s. Valid range: 0..36\n
 * Special values:
 * - 0 = auto detect base by prefix (0 = octal, 0x or 0X = hex, other = decimal)
 * - 2 = binary
 * - 8 = octal (prefix 0 is ignored if present)
 * - 10 = decimal
 * - 16 = hexadecimal (prefix 0x or 0X is ignored if present)
 *
 * \param min
 * Minimum allowed value for the result of the conversion.
 *
 * \param max
 * Maximum allowed value for the result of the conversion.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint32_t ToU32(std::string const & s, uint_fast8_t const base, uint32_t const min, uint32_t const max)
{
  static_assert(sizeof(unsigned long) >= sizeof(uint32_t), "Unexpected size of 'unsigned long' on this platform.");

  // reject empty strings and leading whitespace characters
  if ((s.empty()) || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

#if defined(__GLIBC__)
  // std::stoul() typically uses strtoul() from the C library. In recent versions of glibc (e.g. 2.39), strtoul()
  // recognizes prefix "0b"/"0B" for binary numbers if base is 2 or 0, though prefix "b0"/"B0" is not specified to be
  // recognized at all. Instead the prefix "0b"/"0B" should result in an error.
  // To ensure proper function of ToU32(), we have to test for invalid input here.
  if (   ((base == 0U) || (base == 2U))
      && (   (gpcc::string::StartsWith(s, "0b"))
          || (gpcc::string::StartsWith(s, "0B"))))
  {
    ThrowInvalidNumberRepresentation(s);
  }
#endif

  size_t n;
  unsigned long value;
  try
  {
    value = std::stoul(s, &n, base);

    // std::stoul accepts negative values. The only value with leading minus we accept here is zero.
    if ((s.front() == '-') && (value != 0U))
      ThrowOutOfRange(s, min, max);
  }
  catch (std::out_of_range const &)
  {
    ThrowOutOfRange(s, min, max);
  }
  catch (std::exception const &)
  {
    ThrowInvalidNumberRepresentation(s);
  }

  // reject if the whole string has not been consumed
  if (n != s.size())
    ThrowInvalidNumberRepresentation(s);

  if ((value < min) || (value > max))
    ThrowOutOfRange(s, min, max);

  return static_cast<uint32_t>(value);
}

/**
 * \ingroup GPCC_STRING
 * \brief Wrapper for `std::stoi` adding verbose error checks.
 *
 * Additional functionality:
 * - Checks if the result is within a given range [`min`;`max`].
 * - Does not accept any leading or trailing extra characters (incl. whitespace characters), except for one optional
 *   leading sign character (+/-).
 * - Throws verbose exceptions in case of any error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `int32_t`.
 *
 * \param base
 * Base for interpreting @p s. Valid range: 0..36\n
 * Special values:
 * - 0 = auto detect base by prefix (0 = octal, 0x or 0X = hex, other = decimal)
 * - 2 = binary
 * - 8 = octal (prefix 0 is ignored if present)
 * - 10 = decimal
 * - 16 = hexadecimal (prefix 0x or 0X is ignored if present)
 *
 * \param min
 * Minimum allowed value for the result of the conversion.
 *
 * \param max
 * Maximum allowed value for the result of the conversion.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
int32_t ToI32(std::string const & s, uint_fast8_t const base, int32_t const min, int32_t const max)
{
  static_assert(sizeof(int) >= sizeof(int32_t), "Unexpected size of 'int' on this platform.");

  // reject empty strings and leading whitespace characters
  if ((s.empty()) || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

#if defined(__GLIBC__)
  // std::stoi() typically uses strtol() from the C library. In recent versions of glibc (e.g. 2.39), strtol()
  // recognizes prefix "0b"/"0B" for binary numbers if base is 2 or 0, though prefix "b0"/"B0" is not specified to be
  // recognized at all. Instead the prefix "0b"/"0B" should result in an error.
  // To ensure proper function of ToI32(), we have to test for invalid input here.
  if (   ((base == 0U) || (base == 2U))
      && (   (gpcc::string::StartsWith(s, "0b"))
          || (gpcc::string::StartsWith(s, "0B"))
          || (gpcc::string::StartsWith(s, "+0b"))
          || (gpcc::string::StartsWith(s, "+0B"))
          || (gpcc::string::StartsWith(s, "-0b"))
          || (gpcc::string::StartsWith(s, "-0B"))))
  {
    ThrowInvalidNumberRepresentation(s);
  }
#endif

  size_t n;
  int value;
  try
  {
    value = std::stoi(s, &n, base);
  }
  catch (std::out_of_range const &)
  {
    ThrowOutOfRange(s, min, max);
  }
  catch (std::exception const &)
  {
    ThrowInvalidNumberRepresentation(s);
  }

  // reject if the whole string has not been consumed
  if (n != s.size())
    ThrowInvalidNumberRepresentation(s);

  if ((value < min) || (value > max))
    ThrowOutOfRange(s, min, max);

  return static_cast<int32_t>(value);
}

/**
 * \ingroup GPCC_STRING
 * \brief Wrapper for `std::stoull` adding verbose error checks.
 *
 * Additional functionality:
 * - Checks if the result is within a given range [`min`;`max`].
 * - Does not accept any leading or trailing extra characters (incl. whitespace characters), except for one optional
 *   leading sign character (+/-).
 * - Throws verbose exceptions in case of any error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint64_t`.
 *
 * \param base
 * Base for interpreting @p s. Valid range: 0..36\n
 * Special values:
 * - 0 = auto detect base by prefix (0 = octal, 0x or 0X = hex, other = decimal)
 * - 2 = binary
 * - 8 = octal (prefix 0 is ignored if present)
 * - 10 = decimal
 * - 16 = hexadecimal (prefix 0x or 0X is ignored if present)
 *
 * \param min
 * Minimum allowed value for the result of the conversion.
 *
 * \param max
 * Maximum allowed value for the result of the conversion.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint64_t ToU64(std::string const & s, uint_fast8_t const base, uint64_t const min, uint64_t const max)
{
  static_assert(sizeof(unsigned long long) >= sizeof(uint64_t),
                "Unexpected size of 'unsigned long long' on this platform.");

  // reject empty strings and leading whitespace characters
  if ((s.empty()) || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

#if defined(__GLIBC__)
  // std::stoull() typically uses strtoull() from the C library. In recent versions of glibc (e.g. 2.39), strtoull()
  // recognizes prefix "0b"/"0B" for binary numbers if base is 2 or 0, though prefix "b0"/"B0" is not specified to be
  // recognized at all. Instead the prefix "0b"/"0B" should result in an error.
  // To ensure proper function of ToU64(), we have to test for invalid input here.
  if (   ((base == 0U) || (base == 2U))
      && (   (gpcc::string::StartsWith(s, "0b"))
          || (gpcc::string::StartsWith(s, "0B"))))
  {
    ThrowInvalidNumberRepresentation(s);
  }
#endif

  size_t n;
  unsigned long long value;
  try
  {
    value = std::stoull(s, &n, base);

    // std::stoull accepts negative values. The only value with leading minus we accept here is zero.
    if ((s.front() == '-') && (value != 0U))
      ThrowOutOfRange(s, min, max);
  }
  catch (std::out_of_range const &)
  {
    ThrowOutOfRange(s, min, max);
  }
  catch (std::exception const &)
  {
    ThrowInvalidNumberRepresentation(s);
  }

  // reject if the whole string has not been consumed
  if (n != s.size())
    ThrowInvalidNumberRepresentation(s);

  if ((value < min) || (value > max))
    ThrowOutOfRange(s, min, max);

  return static_cast<uint64_t>(value);
}

/**
 * \ingroup GPCC_STRING
 * \brief Wrapper for `std::stoll` adding verbose error checks.
 *
 * Additional functionality:
 * - Checks if the result is within a given range [`min`;`max`].
 * - Does not accept any leading or trailing extra characters (incl. whitespace characters), except for one optional
 *   leading sign character (+/-).
 * - Throws verbose exceptions in case of any error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `int64_t`.
 *
 * \param base
 * Base for interpreting @p s. Valid range: 0..36\n
 * Special values:
 * - 0 = auto detect base by prefix (0 = octal, 0x or 0X = hex, other = decimal)
 * - 2 = binary
 * - 8 = octal (prefix 0 is ignored if present)
 * - 10 = decimal
 * - 16 = hexadecimal (prefix 0x or 0X is ignored if present)
 *
 * \param min
 * Minimum allowed value for the result of the conversion.
 *
 * \param max
 * Maximum allowed value for the result of the conversion.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
int64_t ToI64(std::string const & s, uint_fast8_t const base, int64_t const min, int64_t const max)
{
  static_assert(sizeof(long long) >= sizeof(int64_t), "Unexpected size of 'long long' on this platform.");

  // reject empty strings and leading whitespace characters
  if ((s.empty()) || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

#if defined(__GLIBC__)
  // std::stoll() typically uses strtoll() from the C library. In recent versions of glibc (e.g. 2.39), strtoll()
  // recognizes prefix "0b"/"0B" for binary numbers if base is 2 or 0, though prefix "b0"/"B0" is not specified to be
  // recognized at all. Instead the prefix "0b"/"0B" should result in an error.
  // To ensure proper function of ToI64(), we have to test for invalid input here.
  if (   ((base == 0U) || (base == 2U))
      && (   (gpcc::string::StartsWith(s, "0b"))
          || (gpcc::string::StartsWith(s, "0B"))
          || (gpcc::string::StartsWith(s, "+0b"))
          || (gpcc::string::StartsWith(s, "+0B"))
          || (gpcc::string::StartsWith(s, "-0b"))
          || (gpcc::string::StartsWith(s, "-0B"))))
  {
    ThrowInvalidNumberRepresentation(s);
  }
#endif

  size_t n;
  long long value;
  try
  {
    value = std::stoll(s, &n, base);
  }
  catch (std::out_of_range const &)
  {
    ThrowOutOfRange(s, min, max);
  }
  catch (std::exception const &)
  {
    ThrowInvalidNumberRepresentation(s);
  }

  // reject if the whole string has not been consumed
  if (n != s.size())
    ThrowInvalidNumberRepresentation(s);

  if ((value < min) || (value > max))
    ThrowOutOfRange(s, min, max);

  return static_cast<int64_t>(value);
}

} // anonymous namespace

namespace gpcc {
namespace string {

/**
 * \ingroup GPCC_STRING
 * \brief Removes leading and trailing white-spaces from a string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Constant reference to the string that shall be trimmed.
 *
 * \return
 * Trimmed version of @p s.
 */
std::string Trim(std::string const & s)
{
  return Trim(s, ' ');
}

/**
 * \ingroup GPCC_STRING
 * \brief Removes specific leading and trailing characters from a string.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Constant reference to the string that shall be trimmed.
 *
 * \param c
 * Characters that shall be trimmed
 *
 * \return
 * Trimmed version of @p s.
 */
std::string Trim(std::string const & s, char const c)
{
  std::string result;

  auto const start = s.find_first_not_of(c);
  auto const last  = s.find_last_not_of(c);

  if (start != std::string::npos)
    result = s.substr(start, (last - start) + 1U);

  return result;
}

/**
 * \ingroup GPCC_STRING
 * \brief Splits the string @p s into sub-strings separated by @p separator.
 *
 * If @p s does not contain @p separator, then a vector containing one string (`s`) will be returned.
 *
 * __Example 1:__\n
 * s = 55, 23, 77,88,,2,\n
 * separator = ','\n
 * skipEmptyParts = false\n
 * Result:\n
 * "55", " 23", " 77", "88", "", "2", ""
 *
 * __Example 2:__\n
 * s = 55, 23, 77,88,,2\n
 * separator = ','\n
 * skipEmptyParts = true\n
 * Result:\n
 * "55", " 23", " 77", "88", "2"
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to the string that shall be separated.
 *
 * \param separator
 * Separating character.
 *
 * \param skipEmptyParts
 * Controls if empty parts shall appear in the output vector or not.\n
 * true  = empty parts shall not appear in the output vector\n
 * false = empty parts shall appear in the output vector
 *
 * \return
 * Vector containing the sub-strings.
 */
std::vector<std::string> Split(std::string const & s, char const separator, bool const skipEmptyParts)
{
  std::vector<std::string> v;

  if (s.length() == 0U)
    return v;

  size_t pos1 = 0U;
  size_t idx = s.find(separator);

  while (idx != std::string::npos)
  {
    if (idx == pos1)
    {
      // (empty string)
      if (!skipEmptyParts)
        v.emplace_back();
    }
    else
    {
      // (not an empty string)
      v.emplace_back(s.substr(pos1, idx - pos1));
    }

    // Is "separator" the last character? If yes, then the end of the string is reached but there
    // is one empty string left
    if (idx == (s.length() - 1U))
    {
      // (last empty string)
      if (!skipEmptyParts)
        v.emplace_back();

      // finished
      return v;
    }

    // look for next occurrence
    pos1 = idx + 1U;
    idx = s.find(separator, pos1);
  }

  // rest of string
  v.emplace_back(s.substr(pos1));

  return v;
}

/**
 * \ingroup GPCC_STRING
 * \brief Splits the string @p s into sub-strings separated by @p separator. @p separator characters within areas
 *        surrounded by @p quotationMark characters are ignored.
 *
 * If @p s does not contain @p separator, then a vector containing one string (`s`) will be returned.
 *
 * __Example 1:__\n
 * s = Monkey Ball "Dog Cat Bird" Airplane\n
 * separator = ' '\n
 * skipEmptyParts = true\n
 * quotationMark = '"'\n
 * Result:\n
 * 1. Monkey
 * 2. Ball
 * 3. "Dog Cat Bird" (`separator` was ignored)
 * 4. Airplane
 *
 * __Example 2:__\n
 * s = Name:"Willy" Age:5 Address : "Grey Road 5"\n
 * separator = ' '\n
 * skipEmptyParts = true\n
 * quotationMark = '"'\n
 * Result:\n
 * 1. Name:"Willy"
 * 2. Age:5
 * 3. Address
 * 4. :
 * 5. "Grey Road 5" (`separator` was ignored)
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s invalid, e.g. odd number of @p quotationMark characters.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to the string that shall be separated.
 *
 * \param separator
 * Separating character.
 *
 * \param skipEmptyParts
 * Controls if empty parts shall appear in the output vector or not.\n
 * true  = empty parts shall not appear in the output vector\n
 * false = empty parts shall appear in the output vector
 *
 * \param quotationMark
 * @p separator characters within parts of @p s surrounded by this character will be ignored.\n
 * @p separator and @p quotationMark must be different characters.
 *
 * \return
 * Vector containing the sub-strings.
 */
std::vector<std::string> Split(std::string const & s, char const separator, bool const skipEmptyParts, char const quotationMark)
{
  if (separator == quotationMark)
    throw std::invalid_argument("Split: Characters for separator and quotation mark are the same.");

  std::vector<std::string> v;

  if (s.length() == 0U)
    return v;

  auto separatorFinder = [&](size_t startPos) -> size_t
  {
    while (true)
    {
      // locate first quotation mark character
      size_t const qm1 = s.find(quotationMark, startPos);
      if (qm1 == std::string::npos)
      {
        // there is none, so just look for a separator character
        return s.find(separator, startPos);
      }

      // locate second quotation mark character (there must be a second one!)
      size_t const qm2 = s.find(quotationMark, qm1 + 1U);
      if (qm2 == std::string::npos)
        throw std::invalid_argument("Split: Can't find second quotation mark character.");

      // locate next separator character
      size_t nextSeparator = s.find(separator, startPos);

      // If there is a separator character and if it is outside the area surrounded by the quotation mark characters,
      // then we have the result
      if (   (nextSeparator != std::string::npos)
          && ((nextSeparator < qm1) || (nextSeparator > qm2)))
      {
        return nextSeparator;
      }

      // otherwise continue looking for a separator character behind the second quotation mark character
      startPos = qm2 + 1U;
    }
  };

  size_t pos1 = 0U;
  size_t idx = separatorFinder(0U);

  while (idx != std::string::npos)
  {
    if (idx == pos1)
    {
      // (empty string)
      if (!skipEmptyParts)
        v.emplace_back();
    }
    else
    {
      // (not an empty string)
      v.emplace_back(s.substr(pos1, idx - pos1));
    }

    // Is "separator" the last character? If yes, then the end of the string is reached but there
    // is one empty string left
    if (idx == (s.length() - 1U))
    {
      // (last empty string)
      if (!skipEmptyParts)
        v.emplace_back();

      // finished
      return v;
    }

    // look for next occurrence
    pos1 = idx + 1U;
    idx = separatorFinder(pos1);
  }

  // rest of string
  v.emplace_back(s.substr(pos1));

  return v;
}

/**
 * \ingroup GPCC_STRING
 * \brief Concatenates neighbouring strings in an `std::vector` of strings, if a special character is present at the
 *        begin and/or end of the strings.
 *
 * __Rules:__\n
 * - Concatenation of two neighbouring elements `v[n]` and `v[n+1]` takes place if one of the two conditions is valid:
 *   - The last char of `v[n]` is `glueChar`
 *   - The first char of `v[n+1]` is `glueChar`
 * - Concatenation of three neighbouring elements `v[n-1]`, `v[n]` and `v[n+1]` takes place if:
 *   - `v[n]` is comprised of one `glueChar` only
 * - The result of a concatenation may be involved in further concatenations if the required conditions are met.\n
 *   This may result in removal of empty strings from `v`.
 *
 * __Examples:__\n
 * All following examples use ':' as `glueChar`.\n
 * _Examples for common input:_
 * - "Name:Willy" -> "Name:Willy"
 * - "Name:Willy", "Age:5" -> "Name:Willy", "Age:5"
 * - "Name:", "Willy" -> "Name:Willy"
 * - "Name", ":Willy" -> "Name:Willy"
 * - "Name", ":", "Willy" -> "Name:Willy"
 * - "Name", ":", "Willy", "Age", ":", "50" -> "Name:Willy", "Age:50"
 *
 * _Examples containing empty strings:_
 * - "Name:", "", "Willy", "" -> "Name:Willy", ""
 * - "Name", "", ":", "", "Willy", "" -> "Name:Willy", ""
 * - "Name:", "", "", "Willy", "" -> "Name:Willy", ""
 *
 * _Examples for not-so-common input:_
 * - "Name", "::", "Willy" -> "Name::Willy"
 * - "Name:", ":Willy" -> "Name::Willy"
 * - "Name", ":", "Willy:", "Age:", "50" -> "Name:Willy:Age:50"
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of `v` is undefined
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param v
 * Reference to vector of std::string objects that shall be processed.
 *
 * \param glueChar
 * Character that triggers concatenation of neighbouring elements in `v`.
 */
void ConditionalConcat(std::vector<std::string> & v, char const glueChar)
{
  // iterator to currently examined and processed string
  auto it_curr = v.begin();

  // iterator to previous processed string that may be target of concatenation
  auto it_target = v.begin();

  // flag indicating if "*it_curr" shall be concatenated (appended) to "*it_target"
  bool concat = false;

  // counter for preserving empty strings if no concatenation takes place
  size_t nEmptyStrings = 0U;

  while (it_curr != v.end())
  {
    std::string const & curr = *it_curr;

    // first cycle?
    if (it_curr == it_target)
    {
      if ((!curr.empty()) && (curr.back() == glueChar))
        concat = true;

      ++it_curr;
      continue;
    }

    // check if "curr" requires concatenation with preceding string
    if ((!curr.empty()) && (curr.front() == glueChar))
      concat = true;

    // concat *it_curr to *it_target ?
    if (concat)
    {
      // eliminate any empty strings between it_target and it_curr
      nEmptyStrings = 0U;

      // current string empty?
      // (may only happen it *it_target has a trailing glueChar)
      if (curr.empty())
      {
        ++it_curr;
        continue;
      }

      // concat
      *it_target += *it_curr;

      // move on and figure out if concatenation continues or not
      ++it_curr;
      concat = ((*it_target).back() == glueChar);
      continue;
    }

    if (curr.empty())
    {
      // Count the empty string.
      // If there is a 'glueChar' at the beginning of the next non-empty subsequent string, then all counted empty
      // strings will be removed from 'v'. If there is no 'glueChar' at the beginning of the next non-empty subsequent
      // string, then the counted empty strings will be preserved in 'v'.
      nEmptyStrings++;
      ++it_curr;
      continue;
    }

    // "curr" is not empty and there was obviously no leading 'glueChar'. We have to preserve potential empty strings
    // in 'v' between it_target and it_curr.
    while (nEmptyStrings != 0U)
    {
      ++it_target;
      (*it_target).clear();
      nEmptyStrings--;
    }

    // check if "curr" has a trailing 'glueChar' and thus requires concatenation with a subsequent string
    if (curr.back() == glueChar)
      concat = true;

    // Move the current string to the proper position in 'v' and remember it in it_target as potential target for
    // concatenation with a subsequent string.
    ++it_target;
    if (it_target != it_curr)
      *it_target = std::move(*it_curr);

    // process next string during next loop cycle
    ++it_curr;
  }

  // preserve counted empty strings
  while (nEmptyStrings != 0U)
  {
    ++it_target;
    (*it_target).clear();
    nEmptyStrings--;
  }

  // finally truncate 'v' at it_target+1
  if (it_target != v.end())
  {
    ++it_target;
    v.erase(it_target, v.end());
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Inserts white-spaces after each "\\n" into an string in order to achieve indention.
 *
 * Example:\n
 * "Text\\nLine1\\nLine2"\n
 * ...passed to `InsertIndention()` with `n = 2` will result in: (with * = white space)\n
 * Text\n
 * **Line1\n
 * **Line2\n
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - content of @p s will be undefined
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String into which white-spaces for indention shall be inserted.
 *
 * \param n
 * Number of white-spaces to be inserted after each "\\n" in @p s.
 */
void InsertIndention(std::string & s, size_t const n)
{
  if (n == 0)
    return;

  auto pos = s.find_first_of('\n', 0);
  while (pos != std::string::npos)
  {
    s.insert(pos + 1, n, ' ');
    pos += n;
    if (pos != s.size())
      pos = s.find_first_of('\n', pos);
    else
      break;
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Checks if a string starts with a given character sequence.
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
 * \param s
 * String that shall be tested.
 *
 * \param pCharSeq
 * Pointer to a null-terminated c-string containing the character sequence.
 *
 * \return
 * true = @p s starts with @p pCharSeq or @p pCharSeq is an empty string.\n
 * false = @p s does not start with @p pCharSeq.
 */
bool StartsWith(std::string const & s, char const * pCharSeq) noexcept
{
  for (auto const c: s)
  {
    // get next character to test from character sequence
    char const cseqc = *pCharSeq;

    if (cseqc == 0)
      return true;

    // mismatch? -> out
    if (c != cseqc)
      return false;

    pCharSeq++;
  }

  return (*pCharSeq == 0x00);
}

/**
 * \ingroup GPCC_STRING
 * \brief Checks if a string ends with a given character sequence.
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
 * \param s
 * String that shall be tested.
 *
 * \param pCharSeq
 * Pointer to a null-terminated c-string containing the character sequence.
 *
 * \return
 * true = @p s ends with @p pCharSeq or @p pCharSeq is an empty string.\n
 * false = @p s does not end with @p pCharSeq.
 */
bool EndsWith(std::string const & s, char const * pCharSeq) noexcept
{
  size_t const l = strlen(pCharSeq);

  if (l == 0)
    return true;

  if (s.length() < l)
    return false;

  return (memcmp(s.c_str() + (s.length() - l), pCharSeq, l) == 0);
}

/**
 * \ingroup GPCC_STRING
 * \brief Counts the number of occurrences of an specific character in a string.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be examined.
 *
 * \param c
 * Character whose number of occurrences in @p s shall be counted.
 *
 * \return
 * Number of occurrences of @p c in @p s.
 */
size_t CountChar(std::string const & s, char const c) noexcept
{
  size_t cnt = 0;
  char const * p = s.c_str();
  char sc;
  while ((sc = *p) != 0)
  {
    if (sc == c)
      cnt++;
    p++;
  }

  return cnt;
}

/**
 * \ingroup GPCC_STRING
 * \brief Compares an `std::string` against a character sequence containing simple wildcards ('*' and '?').
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   Bad wildcard sequence (**), trailing '\\' or invalid escape sequence.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be checked for matching the character sequence @p pCharSeq.
 *
 * \param pCharSeq
 * Pointer to a null-terminated string containing the character sequence used for the comparison.\n
 * The character sequence may contain wildcard characters:
 * - '*' = any string
 * - '?' = any character
 * - '\' = escape for characters '*', '?', and '\'
 *
 * \param caseSensitive
 * Controls if the comparison shall be case sensitive:
 * `true` = case sensitive\n
 * `false` = case insensitive
 * Note: Case sensitive comparison is slightly faster than case insensitive comparison.
 *
 * \retval true   Match
 * \retval false  No match
 */
bool TestSimplePatternMatch(std::string const & s, char const * pCharSeq, bool const caseSensitive)
{
  return TestSimplePatternMatch(s.c_str(), pCharSeq, caseSensitive);
}

/**
 * \ingroup GPCC_STRING
 * \brief Compares an null-terminated string against a character sequence containing simple wildcards ('*' and '?').
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   Bad wildcard sequence (**), trailing '\\' or invalid escape sequence.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pStr
 * Pointer to a null-terminated string that shall be checked for matching the character sequence @p pCharSeq.
 *
 * \param pCharSeq
 * Pointer to a null-terminated string containing the character sequence used for the comparison.\n
 * The character sequence may contain wildcard characters:
 * - '*' = any string
 * - '?' = any character
 * - '\' = escape for characters '*', '?', and '\'
 *
 * \param caseSensitive
 * Controls if the comparison shall be case sensitive:
 * `true` = case sensitive\n
 * `false` = case insensitive
 * Note: Case sensitive comparison is slightly faster than case insensitive comparison.
 *
 * \retval true   Match
 * \retval false  No match
 */
bool TestSimplePatternMatch(char const * pStr, char const * pCharSeq, bool const caseSensitive)
{
  // Character where to retry looking for a matching sequence after a wildcard in the
  // reference character sequence.
  // nullptr = no wildcard yet or no initial match yet
  char const * pStrRetry = nullptr;

  // Character behind the *-wildcard inside the reference character sequence.
  // nullptr = no wildcard yet
  char const * pRefBehindWildCard = nullptr;

  // Flag indicating if the character referenced by pCharSeq is an escaped '*', '?' or '/'
  bool escapeActive = false;

  while (true)
  {
    char actual   = *pStr;
    char expected = *pCharSeq;

    bool singleCharWildcard;
    if (!escapeActive)
    {
      // check for "any string" wildcard
      if (expected == '*')
      {
        expected = *(++pCharSeq);
        if (expected == 0)
          return true;

        if (expected == '*')
          throw std::invalid_argument("TestSimplePatternMatch: pCharSeq contains '**'");

        pStrRetry = nullptr;
        pRefBehindWildCard = pCharSeq;
      }

      // check for "single character" wildcard
      singleCharWildcard = (expected == '?');

      // resolve escaped characters
      if (expected == '\\')
      {
        expected = *(++pCharSeq);
        if (expected == 0)
          throw std::invalid_argument("TestSimplePatternMatch: pCharSeq contains single trailing '\\'");
        if ((expected != '*') && (expected != '?') && (expected != '\\'))
          throw std::invalid_argument("TestSimplePatternMatch: pCharSeq contains invalid escape sequence");
        escapeActive = true;
      }
    } // if (!escapeActive)
    else
    {
      singleCharWildcard = false;
    } // if (!escapeActive)... else...

    // end of string to be checked reached?
    if (actual == 0)
    {
      // expected character sequence also at its end?
      if (expected == 0)
        return true;
      else
        return false;
    }

    // end of expected character sequence reached?
    // (note: string to be checked is not at its end yet)
    if (expected == 0)
      return false;

    // apply case in-sensitivity if required
    if ((!caseSensitive) && (!singleCharWildcard))
    {
      actual   = toupper(actual);
      expected = toupper(expected);
    }

    // compare
    if ((actual == expected) || (singleCharWildcard))
    {
      // (MATCH)

      // no any-string-wildcard yet?
      if (pRefBehindWildCard == nullptr)
      {
        // just move on
        pStr++;
        pCharSeq++;
        escapeActive = false;
      }
      else
      {
        // do we have a matching sequence after any-string-wildcard?
        if (pStrRetry != nullptr)
        {
          // just move on
          pStr++;
          pCharSeq++;
          escapeActive = false;
        }
        else
        {
          // Move on, but store the current value of pStr.
          // The stored position is required to retry looking for a matching sequence
          // if we get a mismatch.
          pStr++;
          pCharSeq++;
          escapeActive = false;
          pStrRetry = pStr;
        }
      }
    } // if match...
    else
    {
      // (MISMATCH)

      // no any-string-wildcard yet?
      if (pRefBehindWildCard == nullptr)
      {
        return false;
      }
      else
      {
        // do we have a matching sequence after any-string-wildcard?
        if (pStrRetry != nullptr)
        {
          // Restart looking for a matching sequence. The result is, that one
          // more character of pStr is covered by the wildcard in the reference character sequence.
          pStr = pStrRetry;
          pStrRetry = nullptr;
          pCharSeq = pRefBehindWildCard;
          escapeActive = false;
        }
        else
        {
          // move on looking for a matching sequence
          pStr++;
        }
      }
    } // if match... else...
  } // while (true)
}

/**
 * \ingroup GPCC_STRING
 * \brief Queries if a character is a printable ASCII character.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * This is thread-safe.
 *
 * __Exception-safety:__\n
 * No-throw guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param c
 * Character, that shall be checked.
 *
 * \retval true   Character is a printable ASCII character
 * \retval false  Character is not a printable ASCII character
 */
bool IsPrintableASCII(char const c) noexcept
{
  return ((c >= 0x20) && (c <= 0x7E));
}

/**
 * \ingroup GPCC_STRING
 * \brief Checks if a string contains printable ASCII characters only.
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
 * \param s
 * String to be checked.
 *
 * \retval true   The string is comprised of printable ASCII characters only, or the string is empty.
 * \retval false  The string contains at least one not-printable character.
 */
bool IsPrintableASCIIOnly(std::string const & s) noexcept
{
  for (char const c : s)
  {
    if (!IsPrintableASCII(c))
      return false;
  }

  return true;
}

/**
 * \ingroup GPCC_STRING
 * \brief Checks if a string contains a decimal number with optional leading sign (-).
 *
 * Examples for positives (returns true):\n
 * "0", "1", "2", "3", "44", "-0", "-1", "-2", "-3"
 *
 * Examples for negatives (returns false):\n
 * "a", "b", "--4", "2f", "+5", " 3", " "
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
 * \param s
 * String to be checked.
 *
 * \retval true   The string represents a number comprised of decimal digits and a sign (-) only.
 * \retval false  The string does not represent a decimal number.
 */
bool IsDecimalDigitsOnly(std::string const & s) noexcept
{
  if (s.find_first_not_of("-1234567890") != std::string::npos)
    return false;

  auto const last_minus = s.find_last_of('-');
  if ((last_minus != std::string::npos) && (last_minus > 0))
    return false;

  if (s.empty())
    return false;

  return true;
}

/**
 * \ingroup GPCC_STRING
 * \brief Creates an `std::string` from the description (returned by `what()`) of an exception and all nested
 *        exceptions (if any).
 *
 * \note  If the number of nested exceptions exceeds @ref maxDepthForExceptionToStringTranslation, then evaluation
 *        of nested exceptions will stop and a message will be appended to the returned string:\n
 *        `**Max depth reached. Skipping the rest**`
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param e
 * Reference to the exception whose description (returned by `what()`) shall be contained in the string. If there are
 * any nested exceptions, then their descriptions will also be contained in the string. Each nested exception's
 * description will start on a new line.\n
 * Unknown exceptions (exceptions not derived from `std::exception`) are handled gracefully.
 *
 * \return
 * The created string. Example output for a 3-level nested exception:\n
 * 1: X gave up, because Y failed\\n\n
 * 2: Y failed\\n\n
 * 3: Could not complete
 */
std::string ExceptionDescriptionToString(std::exception const & e)
{
  gpcc::string::StringComposer s;
  ExceptionDescriptionToStringHelper(e, 1U, s);
  return s.Get();
}

/**
 * \ingroup GPCC_STRING
 * \brief Creates an std::string from the description (returned by `what()`) of an exception and all nested
 *        exceptions (if any).
 *
 * \note  If the number of nested exceptions exceeds @ref maxDepthForExceptionToStringTranslation, then evaluation
 *        of nested exceptions will stop and a message will be appended to the returned string:\n
 *        `**Max depth reached. Skipping the rest**`
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param ePtr
 * Unmodifiable reference to an exception pointer referencing the exception whose description (returned by `what()`)
 * shall be contained in the string. If there are any nested exceptions, then their descriptions will also be contained
 * in the string. Each nested exception's description will start on a new line.\n
 * If this is a nullptr, then this will throw an std::invalid_argument.\n
 * Unknown exceptions (exceptions not derived from `std::exception`) are handled gracefully.
 *
 * \return
 * The created string. Example output for a 3-level nested exception:\n
 * 1: X gave up, because Y failed\\n\n
 * 2: Y failed\\n\n
 * 3: Could not complete
 */
std::string ExceptionDescriptionToString(std::exception_ptr const & ePtr)
{
  if (!ePtr)
    throw std::invalid_argument("ExceptionDescriptionToString: !ePtr");

  try
  {
    std::rethrow_exception(ePtr);
  }
  catch (std::exception const & e)
  {
    return ExceptionDescriptionToString(e);
  }
  catch (...)
  {
    return "1: Unknown exception";
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Creates a string containing a detailed hex-dump of a chunk of binary data.
 *
 * The generated output string contains the address, the data in hexadecimal format, and the data in ASCII format. Bytes
 * that are not printable ASCII characters are replaced by dots ('.').
 *
 * Parameters @p address, @p pData, and @p n are updated to allow for convenient generation of large dumps comprised of
 * multiple lines in a tight loop:
 * ~~~{.cpp}
 * void DumpNetworkPacketToCLI(std::vector<uint8_t> const & msgData, gpcc::cli::CLI & cli)
 * {
 *   size_t n = msgData.size();
 *   if (n > 2048U)
 *     throw std::invalid_argument("Message to large");
 *
 *   void const * pData = msgData.data();
 *   uintptr_t address = 0U;
 *
 *   cli.WriteLine("Offset  +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F 0123456789ABCDEF");
 *   //             0x0000: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ................
 *   while (n != 0)
 *   {
 *     cli.WriteLine(HexDump(address, 4U, pData, n, 1, 16U));
 *   }
 * }
 * ~~~
 *
 * Example output (@p nbOfAddressDigits = 8, @p n = 25, @p wordSize = 1, @p valuePerLine = 4):\n
 * ~~~{.txt}
 * 0x10005320: 21 41 5C 87 .A..
 * ~~~
 *
 * Example output (@p nbOfAddressDigits = 8, @p n = 4, @p wordSize = 1, @p valuePerLine = 8):\n
 * Note: '_' explicitly shows white space characters used to pad the line until 8 values are contained\n
 * ~~~{.txt}
 * 0x10005320: 21 41 5C 87 __ __ __ __ .A..
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Update of @p address, @p pData, and/or @p n may be incomplete.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param address
 * Address where the first byte of data is located.\n
 * This is used for printing only. The actual data is read from @p pData. \n
 * The referenced variable will be incremented by the number of bytes dumped to the output string.
 *
 * \param nbOfAddressDigits
 * Minimum number of hexadecimal digits that shall be reserved for @p address in the output string.\n
 * If required, then the address will be padded with zeros on the left.
 *
 * \param pData
 * Pointer to the data that shall be dumped. The byte/word referenced by this shall correspond to @p address. \n
 * This must be aligned to @p wordSize. \n
 * The referenced pointer will be incremented by the number of bytes dumped to the output string.
 *
 * \param n
 * Number of __bytes__ inside the buffer referenced by @p pData. \n
 * @p wordSize must divide this without any remainder. Zero is allowed.\n
 * The referenced variable will be decremented by the number of bytes dumped to the output string.
 *
 * \param wordSize
 * Word size in byte for displaying data in hex-format. This must be 1, 2, 4, or 8.
 *
 * \param wordsPerLine
 * Number of words per line. If @p n divided by @p wordSize is less than this, then white spaces will be inserted
 * into the output until @p wordsPerLine words are contained in the output.
 *
 * \return
 * String containing a detailed hex-dump of the binary data referenced by @p pData.
 */
std::string HexDump(uintptr_t & address,
                    uint8_t const nbOfAddressDigits,
                    void const * & pData,
                    size_t & n,
                    uint8_t const wordSize,
                    uint_fast8_t wordsPerLine)
{
  static_assert(sizeof(unsigned int) >= sizeof(uint32_t));

  if (pData == nullptr)
    throw std::invalid_argument("HexDump: pData");

  if ((wordSize != 1U) && (wordSize != 2U) && (wordSize != 4U) && (wordSize != 8U))
    throw std::invalid_argument("HexDump: wordSize");

  if ((n % wordSize) != 0U)
    throw std::invalid_argument("HexDump: n <-> wordSize");

  if ((reinterpret_cast<uintptr_t>(pData) % wordSize) != 0U)
    throw std::invalid_argument("HexDump: pData <-> wordSize");

  if (wordsPerLine == 0U)
    throw std::invalid_argument("HexDump: wordsPerLine");


  using gpcc::string::StringComposer;
  StringComposer sc;

  // print address
  sc << StringComposer::AlignRightPadZero << StringComposer::BaseHex << StringComposer::Uppercase
      << "0x" << StringComposer::Width(nbOfAddressDigits) << address << ": ";

  if (n != 0)
  {
    // print hex values
    uintptr_t pCurrRdAddress = reinterpret_cast<uintptr_t>(pData);
    size_t bytesDumped = 0U;
    while ((n != 0U) && (wordsPerLine != 0U))
    {
      sc << StringComposer::Width(wordSize * 2U);

      switch (wordSize)
      {
        case 1U: sc << static_cast<unsigned int>(*reinterpret_cast<uint8_t  const *>(pCurrRdAddress)); break;
        case 2U: sc << static_cast<unsigned int>(*reinterpret_cast<uint16_t const *>(pCurrRdAddress)); break;
        case 4U: sc << static_cast<unsigned int>(*reinterpret_cast<uint32_t const *>(pCurrRdAddress)); break;
        case 8U: sc <<                           *reinterpret_cast<uint64_t const *>(pCurrRdAddress);  break;
      }

      sc << ' ';

      n -= wordSize;
      wordsPerLine--;
      bytesDumped += wordSize;
      pCurrRdAddress += wordSize;
    }

    address += bytesDumped;

    // print empty fields as needed
    while (wordsPerLine-- != 0U)
      sc << StringComposer::Width((wordSize * 2U) + 1U) << ' ';

    // print values as ASCII characters
    char const * p = reinterpret_cast<char const *>(pData);
    while (bytesDumped != 0)
    {
      char c = *p++;
      if (!IsPrintableASCII(c))
        c = '.';
      sc << c;
      bytesDumped--;
    }

    pData = static_cast<void const*>(p);
  }

  return sc.Get();
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `uint8_t`.
 *
 * This function accepts the following textual representations of data of type `uint8_t`:
 * - Decimal numbers, digits 0..9 only; Range: 0..255
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint8_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint8_t`.
 *
 * \return
 * Result of the conversion.
 */
uint8_t DecimalToU8(std::string const & s)
{
  return static_cast<uint8_t>(ToU32(s,
                                    10U,
                                    std::numeric_limits<uint8_t>::min(),
                                    std::numeric_limits<uint8_t>::max()));
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `uint8_t`.
 *
 * This function accepts the following textual representations of data of type `uint8_t`:
 * - Hex values: 0x12, 0xAB; Range: 0x00..0xFF
 * - Binary values: 0b01, 0b001000; Range: 0b00000000..0b11111111
 * - Decimal numbers, digits 0..9 only, range 0..255; A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `uint8_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * If single ASCII characters shall also be accepted, consider using @ref AnyStringToU8().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint8_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint8_t`.
 *
 * \return
 * Result of the conversion.
 */
uint8_t AnyNumberToU8(std::string const & s)
{
  return static_cast<uint8_t>(AnyNumberToU32(s,
                                             std::numeric_limits<uint8_t>::min(),
                                             std::numeric_limits<uint8_t>::max()));
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation (incl. single ASCII characters) into a value of
 *        type `uint8_t`.
 *
 * This function accepts the following textual representations of data of type `uint8_t`:
 * - Hex values: 0x12, 0xAB; Range: 0x00..0xFF
 * - Binary values: 0b01, 0b001000; Range: 0b00000000..0b11111111
 * - Decimal numbers, digits 0..9 only, range 0..255; A leading +/- character is optional.
 * - Single ASCII characters in quoted in '': 'a', 'b', ''' -> '
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `uint8_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint8_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint8_t`.
 *
 * \return
 * Result of the conversion.
 */
uint8_t AnyStringToU8(std::string const & s)
{
  if (StartsWith(s, "'"))
  {
    if ((s.length() != 3U) || (s[2] != '\''))
      ThrowInvalidNumberRepresentation(s);

    return static_cast<uint8_t>(s[1]);
  }
  else
  {
    return static_cast<uint8_t>(AnyNumberToU32(s,
                                               std::numeric_limits<uint8_t>::min(),
                                               std::numeric_limits<uint8_t>::max()));
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a two digit hexadecimal number into a value of type `uint8_t`.
 *
 * The input format is strict:
 * - exactly two digits, hexadecimal format
 * - no "0x" prefix
 * - Leading and trailing space characters are not allowed
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the 2-digit hexadecimal number that shall be converted into an `uint8_t`.
 *
 * \return
 * Result of the conversion.
 */
uint8_t TwoDigitHexToU8(std::string const & s)
{
  if ((s.size() != 2U) || (s.front() == '+') || (s.front() == '-') || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

  return static_cast<uint8_t>(ToU32(s, 16U, std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()));
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a two digit hexadecimal number into a value of type `uint16_t`.
 *
 * The input format is strict:
 * - exactly four digits, hexadecimal format
 * - no "0x" prefix
 * - Leading and trailing space characters are not allowed
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the 4-digit hexadecimal number that shall be converted to an `uint16_t`.
 *
 * \return
 * Result of the conversion.
 */
uint16_t FourDigitHexToU16(std::string const & s)
{
  if ((s.size() != 4U) || (s.front() == '+') || (s.front() == '-') || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

  return static_cast<uint16_t>(ToU32(s, 16U, std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max()));
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `uint32_t`.
 *
 * This function accepts the following textual representations of data of type `uint32_t`:
 * - Decimal numbers, digits 0..9 only; Range: 0..2^32-1
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint32_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint32_t`.
 *
 * \return
 * Result of the conversion.
 */
uint32_t DecimalToU32(std::string const & s)
{
  return ToU32(s,
               10U,
               std::numeric_limits<uint32_t>::min(),
               std::numeric_limits<uint32_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `uint32_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `uint32_t`:
 * - Decimal numbers, digits 0..9 only; Range: [`min`;`max`]
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint32_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint32_t DecimalToU32(std::string const & s, uint32_t const min, uint32_t const max)
{
  return ToU32(s, 10U, min, max);
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in hexadecimal representation into a value of type `uint32_t`.
 *
 * This function accepts the following textual representations of data of type `uint32_t`:
 * - Prefix "0x" is mandatory
 * - Hexadecimal numbers, digits 0..9, a..f, A..F only; Range: 0..2^32-1
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint32_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint32_t`.
 *
 * \return
 * Result of the conversion.
 */
uint32_t HexToU32(std::string const & s)
{
  return HexToU32(s, std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in hexadecimal representation into a value of type `uint32_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `uint32_t`:
 * - Prefix "0x" is mandatory
 * - Hexadecimal numbers, digits 0..9, a..f, A..F only; Range: [`min`;`max`]
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint32_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint32_t HexToU32(std::string const & s, uint32_t const min, uint32_t const max)
{
  if (!StartsWith(s, "0x"))
    ThrowInvalidNumberRepresentation(s);

  return ToU32(s, 16U, min, max);
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `uint32_t`.
 *
 * This function accepts the following textual representations of data of type `uint32_t`:
 * - Hex values: 0x120000CD, 0xAB000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `uint32_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint32_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint32_t`.
 *
 * \return
 * Result of the conversion.
 */
uint32_t AnyNumberToU32(std::string const & s)
{
  return AnyNumberToU32(s,
                        std::numeric_limits<uint32_t>::min(),
                        std::numeric_limits<uint32_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `uint32_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `uint32_t`:
 * - Hex values: 0x120000CD, 0xAB000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `uint32_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint32_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint32_t AnyNumberToU32(std::string const & s, uint32_t const min, uint32_t const max)
{
  if (StartsWith(s, "0x"))
  {
    return ToU32(s, 16U, min, max);
  }
  else if (StartsWith(s, "0b"))
  {
    return ToU32(s.substr(2), 2U, min, max);
  }
  else
  {
    return ToU32(s, 10U, min, max);
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `uint64_t`.
 *
 * This function accepts the following textual representations of data of type `uint64_t`:
 * - Decimal numbers, digits 0..9 only; Range: 0..2^64-1
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint64_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint64_t`.
 *
 * \return
 * Result of the conversion.
 */
uint64_t DecimalToU64(std::string const & s)
{
  return ToU64(s,
               10U,
               std::numeric_limits<uint64_t>::min(),
               std::numeric_limits<uint64_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `uint64_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `uint64_t`:
 * - Decimal numbers, digits 0..9 only; Range: [`min`;`max`]
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint64_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint64_t DecimalToU64(std::string const & s, uint64_t const min, uint64_t const max)
{
  return ToU64(s, 10U, min, max);
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in hexadecimal representation into a value of type `uint64_t`.
 *
 * This function accepts the following textual representations of data of type `uint64_t`:
 * - Prefix "0x" is mandatory
 * - Hexadecimal numbers, digits 0..9, a..f, A..F only; Range: 0..2^64-1
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint64_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint64_t`.
 *
 * \return
 * Result of the conversion.
 */
uint64_t HexToU64(std::string const & s)
{
  return HexToU64(s, std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in hexadecimal representation into a value of type `uint64_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `uint64_t`:
 * - Prefix "0x" is mandatory
 * - Hexadecimal numbers, digits 0..9, a..f, A..F only; Range: [`min`;`max`]
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `uint64_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint64_t HexToU64(std::string const & s, uint64_t const min, uint64_t const max)
{
  if (!StartsWith(s, "0x"))
    ThrowInvalidNumberRepresentation(s);

  return ToU64(s, 16U, min, max);
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `uint64_t`.
 *
 * This function accepts the following textual representations of data of type `uint64_t`:
 * - Hex values: 0x12000000000000CD, 0xAB00000000000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `uint64_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `uint64_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint64_t`.
 *
 * \return
 * Result of the conversion.
 */
uint64_t AnyNumberToU64(std::string const & s)
{
  return AnyNumberToU64(s,
                        std::numeric_limits<uint64_t>::min(),
                        std::numeric_limits<uint64_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `uint64_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `uint64_t`:
 * - Hex values: 0x12000000000000CD, 0xAB00000000000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `uint64_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `uint64_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
uint64_t AnyNumberToU64(std::string const & s, uint64_t const min, uint64_t const max)
{
  if (StartsWith(s, "0x"))
  {
    return ToU64(s, 16U, min, max);
  }
  else if (StartsWith(s, "0b"))
  {
    return ToU64(s.substr(2), 2U, min, max);
  }
  else
  {
    return ToU64(s, 10U, min, max);
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid character representation into a value of type `char`.
 *
 * This function accepts the following textual representations of data of type `char`:
 * - Hex values: 0x12, 0xAB; Range: 0x00..0xFF
 * - Binary values: 0b01, 0b001000; Range: 0b00000000..0b11111111
 * - Decimal numbers, digits 0..9 only, range -128..127; A leading +/- character is optional.
 * - Single ASCII characters in quoted in '': 'a', 'b', ''' -> '
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `char` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a character.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `char`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `char`.
 *
 * \return
 * Result of the conversion.
 */
char AnyStringToChar(std::string const & s)
{
  if (StartsWith(s, "0x"))
  {
    return static_cast<char>(ToU32(s, 16U, 0U, 255U));
  }
  else if (StartsWith(s, "0b"))
  {
    return static_cast<char>(ToU32(s.substr(2), 2U, 0U, 255U));
  }
  else if (StartsWith(s, "'"))
  {
    if ((s.length() != 3U) || (s[2] != '\''))
      ThrowInvalidNumberRepresentation(s);

    return s[1];
  }
  else
  {
    return static_cast<char>(ToI32(s, 10U, std::numeric_limits<char>::min(), std::numeric_limits<char>::max()));
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `int32_t`.
 *
 * This function accepts the following textual representations of data of type `int32_t`:
 * - Decimal numbers, digits 0..9 only; Range: -2^31 .. 2^31-1
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `int32_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `int32_t`.
 *
 * \return
 * Result of the conversion.
 */
int32_t DecimalToI32(std::string const & s)
{
  return ToI32(s,
               10U,
               std::numeric_limits<int32_t>::min(),
               std::numeric_limits<int32_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `int32_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `int32_t`:
 * - Decimal numbers, digits 0..9 only; Range: [`min`;`max`]
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `int32_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
int32_t DecimalToI32(std::string const & s, int32_t const min, int32_t const max)
{
  return ToI32(s, 10U, min, max);
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `int32_t`.
 *
 * This function accepts the following textual representations of data of type `int32_t`:
 * - Hex values: 0x120000CD, 0xAB000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `int32_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `int32_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `int32_t`.
 *
 * \return
 * Result of the conversion.
 */
int32_t AnyNumberToI32(std::string const & s)
{
  return AnyNumberToI32(s,
                        std::numeric_limits<int32_t>::min(),
                        std::numeric_limits<int32_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `int32_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `int32_t`:
 * - Hex values: 0x120000CD, 0xAB000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `int32_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `int32_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
int32_t AnyNumberToI32(std::string const & s, int32_t const min, int32_t const max)
{
  if (StartsWith(s, "0x"))
  {
    int32_t value = static_cast<int32_t>(ToU32(s,
                                               16U,
                                               0U,
                                               std::numeric_limits<uint32_t>::max()));

    if ((value < min) || (value > max))
      ThrowOutOfRange(s, min, max);

    return value;
  }
  else if (StartsWith(s, "0b"))
  {
    int32_t value = static_cast<int32_t>(ToU32(s.substr(2),
                                               2U,
                                               0U,
                                               std::numeric_limits<uint32_t>::max()));

    if ((value < min) || (value > max))
      ThrowOutOfRange(s, min, max);

    return value;
  }
  else
  {
    return ToI32(s, 10U, min, max);
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `int64_t`.
 *
 * This function accepts the following textual representations of data of type `int64_t`:
 * - Decimal numbers, digits 0..9 only; Range: -2^63 .. 2^63-1
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `int64_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `int64_t`.
 *
 * \return
 * Result of the conversion.
 */
int64_t DecimalToI64(std::string const & s)
{
  return ToI64(s,
               10U,
               std::numeric_limits<int64_t>::min(),
               std::numeric_limits<int64_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a number in decimal representation into a value of type `int64_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `int64_t`:
 * - Decimal numbers, digits 0..9 only; Range: [`min`;`max`]
 * - Leading and trailing space characters are not allowed.
 * - A leading +/- character is optional.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String containing the number that shall be converted into an `int64_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
int64_t DecimalToI64(std::string const & s, int64_t const min, int64_t const max)
{
  return ToI64(s, 10U, min, max);
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `int64_t`.
 *
 * This function accepts the following textual representations of data of type `int64_t`:
 * - Hex values: 0x12000000000000CD, 0xAB00000000000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `int64_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `int64_t`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `int64_t`.
 *
 * \return
 * Result of the conversion.
 */
int64_t AnyNumberToI64(std::string const & s)
{
  return AnyNumberToI64(s,
                        std::numeric_limits<int64_t>::min(),
                        std::numeric_limits<int64_t>::max());
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing any valid number representation into a value of type `int64_t` and checks
 *        the result against a given `min` and `max`.
 *
 * This function accepts the following textual representations of data of type `int64_t`:
 * - Hex values: 0x12000000000000CD, 0xAB00000000000000, 0xab00
 * - Binary values: 0b01, 0b001000
 * - Decimal numbers, digits 0..9 only. A leading +/- character is optional.
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. It provides maximum flexibility to the user when
 * a `int64_t` shall be entered. Potential exceptions will contain a verbose error message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of [`min`;`max`].
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `int64_t`.
 *
 * \param min
 * Minimum allowed value.
 *
 * \param max
 * Maximum allowed value.\n
 * If `max` < `min`, then the range-check will always fail.
 *
 * \return
 * Result of the conversion.
 */
int64_t AnyNumberToI64(std::string const & s, int64_t const min, int64_t const max)
{
  if (StartsWith(s, "0x"))
  {
    int64_t value = static_cast<int64_t>(ToU64(s,
                                               16U,
                                               0U,
                                               std::numeric_limits<uint64_t>::max()));

    if ((value < min) || (value > max))
      ThrowOutOfRange(s, min, max);

    return value;
  }
  else if (StartsWith(s, "0b"))
  {
    int64_t value = static_cast<int64_t>(ToU64(s.substr(2),
                                               2U,
                                               0U,
                                               std::numeric_limits<uint64_t>::max()));

    if ((value < min) || (value > max))
      ThrowOutOfRange(s, min, max);

    return value;
  }
  else
  {
    return ToI64(s, 10U, min, max);
  }
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts a string containing a double-precision floating point number into a value of type `double`.
 *
 * The function accepts the following textual representations of data of type `double`:
 * - 0, +0, -0, 0.5, +0.5, -0.5, 0.5E+1, 0.5E1. 0.5E-1, 0.5e1
 * - INF, INFINITY (both case-insensitive)
 * - NAN, NAN(*) (both case-insensitive; * may be any sequence of digits, letters, and underscores)
 * - Leading and trailing space characters are not allowed.
 *
 * This function is intended to be used for interpreting user input. Potential exceptions will contain a verbose error
 * message.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   @p s contains no valid representation of a number.
 *
 * \throws std::out_of_range       The result of the conversion exceeds the range of `double`.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * String that shall be converted into an `double`.
 *
 * \return
 * Result of the conversion.
 */
double ToDouble(std::string const & s)
{
  // reject empty strings and leading whitespace characters
  if ((s.empty()) || (std::isspace(s.front()) != 0))
    ThrowInvalidNumberRepresentation(s);

  size_t n;
  double value;
  try
  {
    value = std::stod(s, &n);
  }
  catch (std::out_of_range const &)
  {
    throw;
  }
  catch (std::exception const &)
  {
    ThrowInvalidNumberRepresentation(s);
  }

  if (n != s.size())
    ThrowInvalidNumberRepresentation(s);

  return value;
}

/**
 * \ingroup GPCC_STRING
 * \brief Disassembles strings following the pattern "Field1 = Value1, Field2 = Value2" (or configurable variants of the
 *        pattern) into a vector of pairs of field and value.
 *
 * __Example:__\n
 * _Input:_ Name: "Willy Black" Age: 50\n
 * _separatorChar_: space\n
 * _assignmentChar_: colon (:)\n
 * _quotationMarkChar_: double quote (")\n
 * _Result_:\n
 * 1st pair: {"Name", "Willy Black"}\n
 * 2nd pair: {"Age", "50"}\n
 * \n
 * _Input:_ Name: "Willy Black", Age: 50\n
 * _separatorChar_: comma (,)\n
 * _assignmentChar_: colon (:)\n
 * _quotationMarkChar_: double quote (")\n
 * _Result_:\n
 * 1st pair: {"Name", "Willy Black"}\n
 * 2nd pair: {"Age", "50"}\n
 * \n
 * _Input:_ Type=Potatoe; maxSize=12; maxWeight=3000\n
 * _separatorChar_: semicolon (;)\n
 * _assignmentChar_: equality sign (=)\n
 * _quotationMarkChar_: double quote (")\n
 * _Result_:\n
 * 1st pair: {"Type", "Potatoe"}\n
 * 2nd pair: {"maxSize", 12}\n
 * 3rd pair: {"maxWeight", 3000}
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   `input` is invalid
 *
 * \throws std::invalid_argument   Separator and quotation mark characters are not different.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param input
 * String that shall be disassembled.
 *
 * \param separatorChar
 * Character that shall be recognized as separator between _pairs_ of field and value.\n
 * Typical examples: space, comma, or semicolon
 *
 * \param assignmentChar
 * Character that shall be recognized as separator between field and value _within a pair_.\n
 * Typical examples: colon or equality sign\n
 * If @p separatorChar is the space character, then the separator between field and value may be surrounded by any
 * number of space characters.
 *
 * \param quotationMarkChar
 * Character that shall be recognized as quotation mark. Each field and value may be surrounded by quotation mark
 * characters. If so, then any occurrence of @p separatorChar and @p assignmentChar will be ignored within the quoted
 * section. Quotation mark characters will be removed from the results.
 *
 * \return
 * Vector of pairs of field and value extracted from `input`.
 */
std::vector<std::pair<std::string,std::string>> ExtractFieldAndValue(std::string const & input,
                                                                     char const separatorChar,
                                                                     char const assignmentChar,
                                                                     char const quotationMarkChar)
{
  if (   (separatorChar == assignmentChar)
      || (separatorChar == quotationMarkChar)
      || (assignmentChar == quotationMarkChar))
  {
    throw std::invalid_argument("ExtractFieldAndValue: Separator and quotation mark characters must be different.");
  }

  // Split input by 'separatorChar'. Adjacent separators will be ignored if the space character is the separator.
  auto pairs = Split(input, separatorChar, (separatorChar == ' '), quotationMarkChar);

  // ensure that there are no empty pairs
  if (separatorChar != ' ')
  {
    for (auto const & pair: pairs)
    {
      if (pair.empty())
        throw std::invalid_argument("ExtractFieldAndValue: 'input' is malformed (two adjacent separators)");
    }
  }

  // If 'separatorChar' is the space character, then undo the split in cases where 'assignmentChar' characters
  // were surrounded by space characters.
  if (separatorChar == ' ')
    ConditionalConcat(pairs, assignmentChar);

  // -------------------------------------------------------------------------------
  // At this point, each item in "pairs" should have the following structure:
  // field<assignmentChar>value
  //
  // Note:
  // - field and value may or may not be surrounded by quotationMarkChar characters
  // - field and value may or may not have leading or trailing space characters in
  //   addition to potential quotationMarkChar characters.
  // - presence of assignmentChar character is not guaranteed
  // -------------------------------------------------------------------------------

  // process each pair and fill vector "result"
  std::vector<std::pair<std::string,std::string>> result;
  for (auto const & pair: pairs)
  {
    // split the pair into field and value
    auto fieldAndValue = Split(pair, assignmentChar, false, quotationMarkChar);
    if (fieldAndValue.size() != 2U)
      throw std::invalid_argument("ExtractFieldAndValue: Field/value-pair \"" + pair + "\" from 'input' is malformed.");

    // Ensure that both field and value contain exactly zero or exactly two quotation mark characters.
    // Further trim any space characters, then trim any quotation mark characters from both field and value.
    // Finally ensure that there are not any quotation mark characters left in field and value.
    for (auto & str: fieldAndValue)
    {
      // ensure that there are exactly zero or exactly two quotation mark characters
      auto nbOfQMC = CountChar(str, quotationMarkChar);
      if ((nbOfQMC != 0U) && (nbOfQMC != 2U))
        throw std::invalid_argument("ExtractFieldAndValue: Field/value-pair \"" + pair + "\" from 'input' is malformed (invalid number of quotation mark characters).");

      // trim any space characters, then trim any quotation mark characters
      auto strNew = Trim(Trim(str), quotationMarkChar);

      // ensure that there are not any quotation mark characters left
      if (strNew.find(quotationMarkChar) != std::string::npos)
        throw std::invalid_argument("ExtractFieldAndValue: Field/value-pair \"" + pair + "\" from 'input' is malformed (invalid placement of quotation mark characters).");

      str = std::move(strNew);
    }

    result.emplace_back(std::move(fieldAndValue[0]), std::move(fieldAndValue[1]));
  }

  return result;
}

/**
 * \ingroup GPCC_STRING
 * \brief Allocates storage for a null-terminated c-string and prints formatted text into it.
 *
 * This provides approximatedly the same functionality as `vasprintf()` (extensions of the C-library), which is not
 * available on all platforms.
 *
 * __Format specifiers:__\n
 * Parameter `pFmt` shall refer to a null-terminated c-string containing the log message text and printf-style
 * conversion specifiers. Each conversion specifier must have the following format:\n
 * %[flags][width][.prec][size]type
 *
 * _%:_\n
 * Indicates start of a conversion specifier. Use %% to print a %-character.
 *
 * _flags (optional):_
 * - '-' Output shall be left justified and will be padded with space characters on the right. The default is right
 *   justified with padding on the left.
 * - '+' Always print sign (+/-) in case of a signed conversion (type d, i, f, F, e, E, a, A, g, G). By default, the
 *   sign is only printed for negative values.
 * - ' ' Print a space character instead of '+' in case of a signed convserion. This is ignored if '+' flag is present.
 * - '0' Use leading zeros for padding of integers and floating point numbers (type d, i, o, x, X, u, f, F, e, E, a, A,
 *   g, G) instead of space characters.\n
 *   For integers (type d, i, o, x, X, u) this is ignored if '.prec' is specified.\n
 *   This is ignored if '-' flag is present.\n
 *   This is ignored if .prec is present and type is d, i, o, u, x, or X.\n
 *   Specifying this flag for conversions other than integer and floating-point will result in undefined behaviour.
 * - '#' Use alternative format:\n
 *   Type o: Increase precision so that first digit is '0'.\n
 *   Type x,X: Prepend 0x / 0X\n
 *   Type f,F,e,E,a,A: Always print decimal point. Trailing zeros are removed.\n
 *   Type g,G: Always print decimal point. Trailing zeros are not removed.\n
 *   All other types: undefined
 *
 * _width (optional):_\n
 * If present, then this controls the minimum width of the field. If necessary, the field will be padded with space
 * characters or zeros.\n
 * If this is '*', then an additional argument of type `int` must be present in the list of variable arguments in front
 * of the value that shall be printed.
 *
 * _prec (optional):_\n
 * '.' followed by a decimal number or '*'.\n
 * If this is '*' then an additional argument of type `int` must be present in the list of variable arguments in front
 * of the value that shall be printed.
 * - For decimal numbers (type d, i, o, x, X, u), this specifies the minimum number of digits that shall appear in the
 *   output.
 * - For floating point numbers (type a, A), this specifies the exact number of digits after the decimal point.
 * - For floating point numbers (type e, E, f, F), this specifies the exact number of digits after the decimal point.
 *   Default is 6.
 * - For floating point numbers (type g, G), this specifies the maximum number of significant digits after the decimal
 *   point. Default is 6.
 * - For a character string (type s), this specifies the maximum number of characters that shall be written. By default
 *   the complete string is printed.
 *
 * _size (optional):_\n
 * Length modifier. In conjunction with 'type', this specifies the expected argument type, see table below.
 *
 * _type:_\n
 * Conversion format specifier.
 *
 * type  | output format                            | size hh       | size h         | none         | size l        | size ll            | size L
 * ----- | ---------------------------------------- | ------------- | -------------- | ------------ | ------------- | ------------------ | ------
 * c     | single char                              | -             | -              | int          | wint_t        | -                  | -
 * s     | character string                         | -             | -              | char*        | wchar_t *     | -                  | -
 * d/i   | signed integer, decimal                  | signed char   | short          | int          | long          | long long          | -
 * o     | unsigned integer, octal                  | unsigned char | unsigned short | unsigned int | unsigned long | unsigned long long | -
 * x/X   | unsigned integer, hex                    | unsigned char | unsigned short | unsigned int | unsigned long | unsigned long long | -
 * u     | unsigned integer, decimal                | unsigned char | unsigned short | unsigned int | unsigned long | unsigned long long | -
 * f/F   | floating point [-]d.d                    | -             | -              | double       | double        | -                  | long double
 * e/E   | floating point, exp. notation            | -             | -              | double       | double        | -                  | long double
 * a/A   | floating point, hex exp. notation        | -             | -              | double       | double        | -                  | long double
 * g/G   | floating point, decimal or exp. notation | -             | -              | double       | double        | -                  | long double
 * p     | Pointer, impl. defined format            | -             | -              | void*        | -             | -                  | -
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pFmt
 * Pointer to a null-terminated c-string containing the text that shall be printed and printf-style conversion
 * specifications that control how the `args` shall be converted and integrated into the text.\n
 * nullptr is not allowed.
 *
 * \param args
 * Variable number of arguments that shall be printed. The number and type of arguments must match the conversion
 * specifiers embedded in `pFmt`.
 *
 * \return
 * Pointer to the created null-terminated c-string.
 */
std::unique_ptr<char[]> VASPrintf(char const * const pFmt, va_list args)
{
  if (pFmt == nullptr)
    throw std::invalid_argument("VASPrintf: !pFmt");

  va_list args2;
  va_copy(args2, args);
  ON_SCOPE_EXIT(end_args) { va_end(args2); };

  // determine size of output string
  int size = vsnprintf(nullptr, 0U, pFmt, args);
  if (size < 0)
    throw std::runtime_error("VASPrintf: Failed (1)");

  // trailing NUL is not counted by vsnprintf()
  size += 1;

  // allocate memory
  std::unique_ptr<char[]> spBuffer(new char[size]);

  // Create the output string.
  // This time size includes the trailing NUL.
  int const status = vsnprintf(spBuffer.get(), size, pFmt, args2);
  if ((status < 0) || (status != (size - 1)))
    throw std::runtime_error("VASPrintf: Failed (2)");

  // success
  return spBuffer;
}

/**
 * \ingroup GPCC_STRING
 * \brief Allocates storage for a null-terminated c-string and prints formatted text into it.
 *
 * This provides approximatedly the same functionality as `asprintf()` (extensions of the C-library), which is not
 * available on all platforms.
 *
 * For details about format specifiers, please refer to @ref VASPrintf().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pFmt
 * Pointer to a null-terminated c-string containing the text that shall be printed and printf-style conversion
 * specifications that control how the `args` shall be converted and integrated into the text.\n
 * nullptr is not allowed.
 *
 * \param ...
 * Variable number of arguments that shall be printed. The number and type of arguments must match the conversion
 * specifiers embedded in `pFmt`.
 *
 * \return
 * Pointer to the created null-terminated c-string.
 */
std::unique_ptr<char[]> ASPrintf(char const * const pFmt, ...)
{
  va_list args;
  va_start(args, pFmt);
  ON_SCOPE_EXIT(end_args) { va_end(args); };

  return VASPrintf(pFmt, args);
}

} // namespace string
} // namespace gpcc
