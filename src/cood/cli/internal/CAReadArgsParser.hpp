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

#ifndef CAREADARGSPARSER_HPP_202106051940
#define CAREADARGSPARSER_HPP_202106051940

#include <string>
#include <cstdint>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \ingroup GPCC_COOD_CLI_INTERNAL
 * \brief Parses the arguments passed to a CLI command that shall read a CANopen object using complete access.
 *
 * The following information is extracted from the args:
 * - index of the object
 * - verbose flag
 *
 * Examples for valid input to CTOR (without quotation marks):
 * - "0x1000"
 * - "0x1000 v"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class CAReadArgsParser final
{
  public:
    CAReadArgsParser(void) = delete;
    CAReadArgsParser(std::string const & args);
    CAReadArgsParser(CAReadArgsParser const &) = default;
    CAReadArgsParser(CAReadArgsParser &&) noexcept = default;
    ~CAReadArgsParser(void) = default;

    CAReadArgsParser& operator=(CAReadArgsParser const &) = default;
    CAReadArgsParser& operator=(CAReadArgsParser &&) noexcept = default;

    uint16_t GetIndex(void) const noexcept;
    bool GetVerbose(void) const noexcept;

  private:
    /// Index of the object.
    uint16_t index;

    /// Flag indicating if output shall be verbose.
    bool verbose;
};

/**
 * \brief Retrieves the extracted index of the object.
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
 * Index of the object.
 */
inline uint16_t CAReadArgsParser::GetIndex(void) const noexcept
{
  return index;
}

/**
 * \brief Retrieves the verbose flag.
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
 * \retval true   Output shall be verbose.
 * \retval false  Output shall not be verbose.
 */
inline bool CAReadArgsParser::GetVerbose(void) const noexcept
{
  return verbose;
}

} // namespace internal
} // namespace cood
} // namespace gpcc

#endif // CAREADARGSPARSER_HPP_202106051940
