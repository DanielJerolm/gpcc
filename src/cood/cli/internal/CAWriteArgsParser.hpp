/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef CAWRITEARGSPARSER_HPP_202106052158
#define CAWRITEARGSPARSER_HPP_202106052158

#include <string>
#include <cstdint>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \ingroup GPCC_COOD_CLI_INTERNAL
 * \class CAWriteArgsParser CAWriteArgsParser.hpp "src/cood/cli/internal/CAWriteArgsParser.hpp"
 * \brief Parses the arguments passed to a CLI command that shall write a CANopen object using complete access.
 *
 * The following information is extracted from the args:
 * - index of the object
 *
 * Examples for valid input to CTOR (without quotation marks):
 * - "0x1000"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class CAWriteArgsParser final
{
  public:
    CAWriteArgsParser(void) = delete;
    CAWriteArgsParser(std::string const & args);
    CAWriteArgsParser(CAWriteArgsParser const &) = default;
    CAWriteArgsParser(CAWriteArgsParser &&) noexcept = default;
    ~CAWriteArgsParser(void) = default;

    CAWriteArgsParser& operator=(CAWriteArgsParser const &) = default;
    CAWriteArgsParser& operator=(CAWriteArgsParser &&) noexcept = default;

    uint16_t GetIndex(void) const noexcept;

  private:
    /// Index of the object.
    uint16_t index;
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
inline uint16_t CAWriteArgsParser::GetIndex(void) const noexcept
{
  return index;
}

} // namespace internal
} // namespace cood
} // namespace gpcc

#endif // CAWRITEARGSPARSER_HPP_202106052158
