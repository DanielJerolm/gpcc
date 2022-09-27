/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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
 * \class EnumerateArgsParser EnumerateArgsParser.hpp "src/cood/cli/internal/EnumerateArgsParser.hpp"
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
