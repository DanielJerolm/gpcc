/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
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
