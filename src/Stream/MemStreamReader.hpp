/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2020, 2022 Daniel Jerolm

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

#ifndef SRC_GPCC_STREAM_MEMSTREAMREADER_HPP_
#define SRC_GPCC_STREAM_MEMSTREAMREADER_HPP_

#include "StreamReaderBase.hpp"

namespace gpcc
{
namespace Stream
{

/**
 * @ingroup GPCC_STREAM
 * @{
 */

/**
 * \brief This class allows to read from a block of memory via an @ref IStreamReader interface.
 *
 * @ref IStreamReader::RemainingBytes() is supported.
 */
class MemStreamReader: public StreamReaderBase
{
  public:
    MemStreamReader(void) = delete;
    MemStreamReader(void const * const _pMem, size_t const _size, Endian const _endian);
    MemStreamReader(MemStreamReader const & other) noexcept;
    MemStreamReader(MemStreamReader&& other) noexcept;
    ~MemStreamReader(void) = default;

    MemStreamReader& operator=(MemStreamReader const & rhv) noexcept;
    MemStreamReader& operator=(MemStreamReader&& rhv) noexcept;

    MemStreamReader SubStream(size_t const n);
    void Shrink(size_t const newRemainingBytes);

    void const * GetReadPtr(void const * const _pMem, size_t const _size) const;

    // --> IStreamReader
    bool IsRemainingBytesSupported(void) const override;
    size_t RemainingBytes(void) const override;
    void EnsureAllDataConsumed(RemainingNbOfBits const expectation) const override;
    void Close(void) noexcept override;

    void Skip(size_t nBits) override;

    std::string Read_string(void) override;
    std::string Read_line(void) override;
    // <-- IStreamReader

  private:
    /// Pointer to the next byte to be read from memory. nullptr = none.
    char const * pMem;

    /// Number of bytes left to be read from memory via `pMem`.
    /** This is valid in stream's states @ref States::open and @ref States::empty. */
    size_t remainingBytes;

    /// Number of bits left to be read. The bits are stored in @ref bitData.
    /** This is valid in stream's states @ref States::open and @ref States::empty. */
    uint8_t nbOfBitsInBitData;

    /// Bits of the last read byte that have not yet been read. The number of bits is stored in @ref nbOfBitsInBitData.
    /** This is only valid if the stream's state is @ref States::open. */
    uint16_t bitData;


    // --> StreamReaderBase
    unsigned char Pop(void) override;
    void Pop(void* p, size_t n) override;
    uint8_t PopBits(uint_fast8_t n) override;
    // <-- StreamReaderBase
};

/**
 * @}
 */

} // namespace Stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_MEMSTREAMREADER_HPP_ */
