/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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

#ifndef ENUMERATEARGSPARSER_HPP_202106030812
#define ENUMERATEARGSPARSER_HPP_202106030812

#include <string>
#include <cstdint>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \ingroup GPCC_COOD_CLI_INTERNAL
 * \brief Parses the arguments passed to a CLI command that shall enumerate the objects contained in an
 *        object dictionary.
 *
 * The following information is extracted from the args:
 * - first index where enumeration shall start (default: 0x0000).
 * - last index where enumeration shall stop (default: 0xFFFF).\n
 *   No objects will be enumerated from beyond this index.
 *
 * Examples for valid input to CTOR (without quotation marks):
 * - "0x1000-0x2000"
 * - "0x1000 - 0x2000"
 * - "0x10-0x20"
 * - ""
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class EnumerateArgsParser final
{
  public:
    EnumerateArgsParser(void) = delete;
    EnumerateArgsParser(std::string const & args);
    EnumerateArgsParser(EnumerateArgsParser const &) = default;
    EnumerateArgsParser(EnumerateArgsParser &&) noexcept = default;
    ~EnumerateArgsParser(void) = default;

    EnumerateArgsParser& operator=(EnumerateArgsParser const &) = default;
    EnumerateArgsParser& operator=(EnumerateArgsParser &&) noexcept = default;

    uint16_t GetFirstIndex(void) const noexcept;
    uint16_t GetLastIndex(void) const noexcept;

  private:
    /// First index.
    uint16_t firstIndex;

    /// Last index.
    uint16_t lastIndex;
};

/**
 * \brief Retrieves the extracted "first index" where enumeration shall start.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * "First index" where enumeration shall start.
 */
inline uint16_t EnumerateArgsParser::GetFirstIndex(void) const noexcept
{
  return firstIndex;
}

/**
 * \brief Retrieves the extracted "last index" where enumeration shall stop.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * "Last index" where enumeration shall stop.
 */
inline uint16_t EnumerateArgsParser::GetLastIndex(void) const noexcept
{
  return lastIndex;
}

} // namespace internal
} // namespace cood
} // namespace gpcc

#endif // ENUMERATEARGSPARSER_HPP_202106030812
