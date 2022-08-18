/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef READARGSPARSER_HPP_202106042132
#define READARGSPARSER_HPP_202106042132

#include <string>
#include <cstdint>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \ingroup GPCC_COOD_CLI_INTERNAL
 * \brief Parses the arguments passed to a CLI command that shall read a subindex of a CANopen object.
 *
 * The following information is extracted from the args:
 * - index of the object
 * - subindex that shall be read
 *
 * Examples for valid input to CTOR (without quotation marks):
 * - "0x1000:2"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ReadArgsParser final
{
  public:
    ReadArgsParser(void) = delete;
    ReadArgsParser(std::string const & args);
    ReadArgsParser(ReadArgsParser const &) = default;
    ReadArgsParser(ReadArgsParser &&) noexcept = default;
    ~ReadArgsParser(void) = default;

    ReadArgsParser& operator=(ReadArgsParser const &) = default;
    ReadArgsParser& operator=(ReadArgsParser &&) noexcept = default;

    uint16_t GetIndex(void) const noexcept;
    uint8_t GetSubIndex(void) const noexcept;

  private:
    /// Index of the object.
    uint16_t index;

    /// Subindex of the object that shall be read.
    uint8_t subIndex;
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
inline uint16_t ReadArgsParser::GetIndex(void) const noexcept
{
  return index;
}

/**
 * \brief Retrieves the extracted subindex.
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
 * Subindex of the object that shall be read.
 */
inline uint8_t ReadArgsParser::GetSubIndex(void) const noexcept
{
  return subIndex;
}

} // namespace internal
} // namespace cood
} // namespace gpcc

#endif // READARGSPARSER_HPP_202106042132
