/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#include <gpcc/string/tools.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <stdexcept>
#include <cstdio>

namespace gpcc   {
namespace string {

/**
 * \ingroup GPCC_STRING
 * \brief Converts an unsigned integral value into an `std::string` using hexadecimal representation and prefix "0x".
 *
 * Example:\n
 * ToHex(11U, 2) -> 0x0B
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
 * \tparam T
 * Type of parameter @p value that shall be converted.\n
 * The type must be an unsigned integral type of up to 8 bytes size.
 *
 * \param value
 * Value that shall be converted.
 *
 * \param width
 * Minimum number of digits. Range: 0..16.
 *
 * \return
 * Result of the conversion.
 */
template<typename T>
std::string ToHex(T const value, uint8_t const width)
{
  static_assert(   (std::is_integral_v<T> == true)
                && (std::is_signed_v<T> == false)
                && (sizeof(T) <= 8U),
                "ToHex() is only defined for unsigned integral types of up to 8 bytes size.");
  static_assert(sizeof(unsigned long long) >= 8U, "'unsigned long long' has unexpected size");

  if (width > 16U)
    throw std::invalid_argument("ToHex: width invalid");

  char buffer[24];
  int status;
  if (sizeof(T) <= sizeof(unsigned long))
  {
    #if defined(_NEWLIB_VERSION)
      status = sniprintf(buffer, sizeof(buffer), "0x%0*lX", static_cast<int>(width), static_cast<unsigned long>(value));
    #else
      status = snprintf(buffer, sizeof(buffer), "0x%0*lX", static_cast<int>(width), static_cast<unsigned long>(value));
    #endif
  }
  else
  {
    #if defined(_NEWLIB_VERSION)
      status = sniprintf(buffer, sizeof(buffer), "0x%0*llX", static_cast<int>(width), static_cast<unsigned long long>(value));
    #else
      status = snprintf(buffer, sizeof(buffer), "0x%0*llX", static_cast<int>(width), static_cast<unsigned long long>(value));
    #endif
  }

  if (status < 0)
    throw std::logic_error("snprintf failed");
  else if (static_cast<size_t>(status) >= sizeof(buffer))
    throw std::logic_error("buffer too small");

  return buffer;
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts an unsigned integral value into an `std::string` using binary representation and prefix "0b".
 *
 * Example:\n
 * ToBin(11U, 6) -> 0b001011
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
 * \tparam T
 * Type of parameter @p value that shall be converted.\n
 * The type must be an unsigned integral type of up to 8 bytes size.
 *
 * \param value
 * Value that shall be converted.
 *
 * \param width
 * Minimum number of digits. Range: 0..64.
 *
 * \return
 * Result of the conversion.
 */
template<typename T>
std::string ToBin(T value, uint8_t width)
{
  static_assert(   (std::is_integral_v<T> == true)
                && (std::is_signed_v<T> == false)
                && (sizeof(T) <= 8U),
                "ToBin() is only defined for unsigned integral types of up to 8 bytes size.");
  static_assert(sizeof(unsigned long long) >= 8U, "'unsigned long long' has unexpected size");

  if (width > 64U)
    throw std::invalid_argument("ToBin: width invalid");

  // we need at least one zero behind the prefix "0b"
  if (width == 0U)
    width = 1U;

  char buffer[72];
  char* p = &buffer[sizeof(buffer) - 1U];

  *p-- = 0;

  while ((value != 0) || (width != 0U))
  {
    if ((value & 1) != 0)
      *p-- = '1';
    else
      *p-- = '0';

    value >>= 1;

    if (width != 0U)
      width--;
  }

  *p-- = 'b';
  *p = '0';

  return p;
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts an unsigned integral value into an `std::string` using hexadecimal representation with no prefix.
 *
 * Example:\n
 * ToHexNoPrefix(11U, 2) -> 0B
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam T
 * Type of parameter @p value that shall be converted.\n
 * The type must be an unsigned integral type of up to 8 bytes size.
 *
 * \param value
 * Value that shall be converted.
 *
 * \param width
 * Minimum number of digits. Range: 0..16.
 *
 * \return
 * Result of the conversion.
 */
template<typename T>
std::string ToHexNoPrefix(T const value, uint8_t const width)
{
  static_assert(   (std::is_integral_v<T> == true)
                && (std::is_signed_v<T> == false)
                && (sizeof(T) <= 8U),
                "ToHexNoPrefix() is only defined for unsigned integral types of up to 8 bytes size.");
  static_assert(sizeof(unsigned long long) >= 8U, "'unsigned long long' has unexpected size");

  if (width > 16U)
    throw std::invalid_argument("ToHexNoPrefix: width invalid");

  char buffer[24];
  int status;
  if (sizeof(T) <= sizeof(unsigned long))
  {
    #if defined(_NEWLIB_VERSION)
      status = sniprintf(buffer, sizeof(buffer), "%0*lX", static_cast<int>(width), static_cast<unsigned long>(value));
    #else
      status = snprintf(buffer, sizeof(buffer), "%0*lX", static_cast<int>(width), static_cast<unsigned long>(value));
    #endif
  }
  else
  {
    #if defined(_NEWLIB_VERSION)
      status = sniprintf(buffer, sizeof(buffer), "%0*llX", static_cast<int>(width), static_cast<unsigned long long>(value));
    #else
      status = snprintf(buffer, sizeof(buffer), "%0*llX", static_cast<int>(width), static_cast<unsigned long long>(value));
    #endif
  }

  if (status < 0)
    throw std::logic_error("snprintf failed");
  else if (static_cast<size_t>(status) >= sizeof(buffer))
    throw std::logic_error("buffer too small");

  return buffer;
}

/**
 * \ingroup GPCC_STRING
 * \brief Converts an unsigned integral value into an `std::string` using decimal and hexadecimal representation.
 *
 * Example:\n
 * ToDecAndHex(11U, 2) -> 11 (0x0B)
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam T
 * Type of parameter @p value that shall be converted.
 *
 * \param value
 * Value that shall be converted.
 *
 * \param width
 * Minimum number of digits used for the hexadecimal representation. Range: 0..16.
 *
 * \return
 * Result of the conversion.
 */
template<typename T>
std::string ToDecAndHex(T const value, uint8_t const width)
{
  static_assert(   (std::is_integral_v<T> == true)
                && (std::is_signed_v<T> == false)
                && (sizeof(T) <= 8U),
                "ToDecAndHex() is only defined for unsigned integral types of up to 8 bytes size.");

  using gpcc::string::StringComposer;

  if (width > 16U)
    throw std::invalid_argument("ToDecAndHex: width invalid");

  StringComposer s;
  s << value << " ("
    << StringComposer::BaseHex << StringComposer::Uppercase
    << StringComposer::AlignRightPadZero << "0x" << StringComposer::Width(width) << value << ')';
  return s.Get();
}

template<>
inline std::string ToDecAndHex<>(unsigned char const value, uint8_t const width)
{
  return ToDecAndHex(static_cast<uint32_t>(value), width);
}

} // namesapce string
} // namesapce gpcc
