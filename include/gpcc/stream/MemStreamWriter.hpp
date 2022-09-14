/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
