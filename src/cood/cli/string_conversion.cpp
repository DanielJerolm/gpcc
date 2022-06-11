/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019 Daniel Jerolm

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

#include "string_conversion.hpp"
#include "gpcc/src/string/tools.hpp"
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
  try
  {
    if ((!gpcc::string::StartsWith(s, "0x")) || (s.size() < 3U) || (s.size() > 6U))
      throw std::invalid_argument("String does not match expected format: Hex, 1..4 digits");

    uint32_t i = gpcc::string::AnyStringToU32(s);
    if (i > std::numeric_limits<uint16_t>::max())
      throw std::invalid_argument("Object index exceeds maximum value");

    return i;
  }
  catch (std::bad_alloc const &) { throw; }
  catch (std::exception const &)
  {
    std::throw_with_nested(std::runtime_error("StringToObjIndex: Cannot convert '" + s + "' to object index"));
  }
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
