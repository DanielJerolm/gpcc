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

#ifndef WRITEARGSPARSER_HPP_202106042143
#define WRITEARGSPARSER_HPP_202106042143

#include "gpcc/src/cood/data_types.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc      {
namespace cood      {
namespace internal  {

/**
 * \ingroup GPCC_COOD_CLI_INTERNAL
 * \brief Parses the arguments passed to a CLI command that shall write a subindex of a CANopen object.
 *
 * The following information is extracted from the args:
 * - index of the object
 * - subindex that shall be read
 * - data that shall be written
 *
 * Data is extracted in two steps:
 * 1. Create object.
 * 2. Invoke @ref ExtractData().
 *
 * Examples for valid input to CTOR (without quotation marks):
 * - "0x1000:2 DATA"
 *
 * The format of DATA must meet the data type of the subindex:\n
 * For BOOLEAN: TRUE, FALSE, true, false\n
 * For REAL32/64: [+|-]digits[.][digits][(e|E)[+|-]digits]\n
 * For VISIBLE_STRING: \"Text...\"\n
 * For OCTET_STRING: 5B A3 ... (8bit hex values, separated by spaces)\n
 * For UNICODE_STRING: 5B33 A6CF (16bit hex values, separated by spaces)\n
 * For BIT1..BIT8: 0, 1, 3, 0x3, 0b11 (unused upper bits must be zero)
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class WriteArgsParser final
{
  public:
    WriteArgsParser(void) = delete;
    WriteArgsParser(std::string const & args);
    WriteArgsParser(WriteArgsParser const &) = default;
    WriteArgsParser(WriteArgsParser &&) noexcept = default;
    ~WriteArgsParser(void) = default;

    WriteArgsParser& operator=(WriteArgsParser const &) = default;
    WriteArgsParser& operator=(WriteArgsParser &&) noexcept = default;

    void ExtractData(DataType const dataType,
                     size_t const subIndexMaxSize,
                     gpcc::Stream::IStreamWriter::Endian const endian);

    uint16_t GetIndex(void) const noexcept;
    uint8_t GetSubIndex(void) const noexcept;

    size_t GetDataSize(void) const noexcept;
    std::vector<uint8_t> & GetData(void) noexcept;

  private:
    /// Index of the object.
    uint16_t index;

    /// Subindex of the object that shall be written.
    uint8_t subIndex;

    /// Data that shall be written, not yet analyzed.
    std::string dataStr;

    /// Size of data in bit.
    size_t sizeInBit;

    /// Data that shall be written.
    std::vector<uint8_t> data;
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
inline uint16_t WriteArgsParser::GetIndex(void) const noexcept
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
 * Subindex of the object that shall be written.
 */
inline uint8_t WriteArgsParser::GetSubIndex(void) const noexcept
{
  return subIndex;
}

/**
 * \brief Retrieves the size of the extracted data in bit.
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
 * Size of the extracted data in bit.
 */
inline size_t WriteArgsParser::GetDataSize(void) const noexcept
{
  return sizeInBit;
}

/**
 * \brief Retrieves the extracted data.
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
 * Reference to a container containing the extracted data.\n
 * The life-time of the referenced container is limited to the life-time of the @ref WriteArgsParser instance.\n
 * The caller may move the content somewhere, but the container will be empty/undefined afterwards.
 */
inline std::vector<uint8_t> & WriteArgsParser::GetData(void) noexcept
{
  return data;
}

} // namespace internal
} // namespace cood
} // namespace gpcc

#endif // WRITEARGSPARSER_HPP_202106042143
