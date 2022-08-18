/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef INFOARGSPARSER_HPP_202106030848
#define INFOARGSPARSER_HPP_202106030848

#include <string>
#include <cstdint>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \ingroup GPCC_COOD_CLI_INTERNAL
 * \brief Parses the arguments passed to a CLI command that shall query information about a CANopen object.
 *
 * The following information is extracted from the args:
 * - index of the object
 * - flag if application specific meta data shall be included in the query
 *
 * Examples for valid input to CTOR (without quotation marks):
 * - "0x1000"
 * - "0x1000 asm"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class InfoArgsParser final
{
  public:
    InfoArgsParser(void) = delete;
    InfoArgsParser(std::string const & args);
    InfoArgsParser(InfoArgsParser const &) = default;
    InfoArgsParser(InfoArgsParser &&) noexcept = default;
    ~InfoArgsParser(void) = default;

    InfoArgsParser& operator=(InfoArgsParser const &) = default;
    InfoArgsParser& operator=(InfoArgsParser &&) noexcept = default;

    uint16_t GetIndex(void) const noexcept;
    bool GetInclASM(void) const noexcept;

  private:
    /// Index of the object.
    uint16_t index;

    /// Flag indicating if application specific meta data shall be included in the query.
    bool inclASM;
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
inline uint16_t InfoArgsParser::GetIndex(void) const noexcept
{
  return index;
}

/**
 * \brief Retrieves if application specific meta data shall be included in the query.
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
 * \retval true    Application specific meta data shall be included in the query.
 * \retval false   Application specific meta data shall __not__ be included in the query.
 */
inline bool InfoArgsParser::GetInclASM(void) const noexcept
{
  return inclASM;
}

} // namespace internal
} // namespace cood
} // namespace gpcc

#endif // INFOARGSPARSER_HPP_202106030848
