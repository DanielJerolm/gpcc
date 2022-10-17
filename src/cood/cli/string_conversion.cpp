/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#include <gpcc/cood/cli/string_conversion.hpp>
#include <gpcc/string/tools.hpp>
#include <limits>
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_CLI
 * \brief Converts a string into a CANopen object index.
 *
 * This is intended to be used with error-prone user input.\n
 * The expected input format is:
 * - prefix '0x'
 * - at least one digit, hexadecimal format (0..9, a..f, A..F)
 * - maximum value 0xFFFF
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
 * String that shall be converted.\n
 * Example: 0x1203
 *
 * \return
 * Result of the conversion.
 */
uint16_t StringToObjIndex(std::string const & s)
{
  return static_cast<uint16_t>(gpcc::string::HexToU32(s, 0U, std::numeric_limits<uint16_t>::max()));
}

/**
 * \ingroup GPCC_COOD_CLI
 * \brief Converts a string into a CANopen object index and subindex.
 *
 * This is intended to be used with error-prone user input.\n
 * The expected input format is strict:
 * - Object index and subindex are separated by ':'
 * - Object index has prefix '0x'
 * - Object index is comprised of at least one digit, hexadecimal format (0..9, a..f, A..F)
 * - Object index is equal to or less than 0xFFFF
 * - Subindex in comprised of at least one digit, decimal format (0..9)
 * - Subindex is equal to or less than 255
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
 * String that shall be converted.\n
 * Example: 0x1203:67
 *
 * \param idx
 * The extracted object index is written to the referenced variable.
 *
 * \param subIdx
 * The extracted subindex is written to the referenced variable.
 */
void StringToObjIndexAndSubindex(std::string const & s, uint16_t & idx, uint8_t & subIdx)
{
  try
  {
    auto const separator = s.find(':');

    if (separator == std::string::npos)
      throw std::invalid_argument("Can't find separator ':' between index and subindex");

    uint16_t const i = StringToObjIndex(s.substr(0, separator));
    subIdx = gpcc::string::DecimalToU8(s.substr(separator + 1));
    idx = i;
  }
  catch (std::bad_alloc const &) { throw; }
  catch (std::exception const &)
  {
    std::throw_with_nested(std::runtime_error("StringToObjIndexAndSubindex: Cannot convert '" + s + "' to object index and subindex (e.g. 0x1000:5)."));
  }
}

} // namespace cood
} // namespace gpcc
