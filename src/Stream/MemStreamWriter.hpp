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

#ifndef SRC_GPCC_STREAM_MEMSTREAMWRITER_HPP_
#define SRC_GPCC_STREAM_MEMSTREAMWRITER_HPP_

#include "StreamWriterBase.hpp"

namespace gpcc
{
namespace Stream
{

/**
 * @ingroup GPCC_STREAM
 * @{
 */

/**
 * \brief This class allows to write into a block of memory via an @ref IStreamWriter interface.
 *
 * @ref IStreamWriter::RemainingCapacity() is supported.
 *
 */
class MemStreamWriter: public StreamWriterBase
{
  public:
    MemStreamWriter(void) = delete;
    MemStreamWriter(void* const _pMem, size_t const _size, Endian const _endian);
    MemStreamWriter(MemStreamWriter const & other) noexcept;
    MemStreamWriter(MemStreamWriter&& other) noexcept;
    ~MemStreamWriter(void);

    MemStreamWriter& operator=(MemStreamWriter const & rhv) noexcept;
    MemStreamWriter& operator=(MemStreamWriter&& rhv) noexcept;

    // --> IStreamWriter
    bool IsRemainingCapacitySupported(void) const override;
    size_t RemainingCapacity(void) const override;
    uint_fast8_t GetNbOfCachedBits(void) const override;

    void Close(void) noexcept override;
    // <-- IStreamWriter

  private:
    /// Pointer to the next byte that shall be written. nullptr = none.
    char* pMem;

    /// Remaining number of bytes that can be written.
    /** This is valid in stream's states @ref States::open and @ref States::full. */
    size_t remainingBytes;

    /// Number of bits written via bit based write methods. The bits are stored in @ref bitData.
    /** This is only valid if the stream's state is @ref States::open or @ref States::full. */
    uint8_t nbOfBitsWritten;

    /// Bits written via bit based write methods. The number of bits is stored in @ref nbOfBitsWritten.
    /** This is only valid if the stream's state is @ref States::open. */
    uint8_t bitData;


    // --> StreamWriterBase
    void Push(char c) override;
    void Push(void const * pData, size_t n) override;
    void PushBits(uint8_t bits, uint_fast8_t n) override;
    // <-- StreamWriterBase
};

/**
 * @}
 */

} // namespace Stream
} // namespace gpcc
#endif /* SRC_GPCC_STREAM_MEMSTREAMWRITER_HPP_ */
