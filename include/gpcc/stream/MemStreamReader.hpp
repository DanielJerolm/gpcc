/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_STREAM_MEMSTREAMREADER_HPP_
#define SRC_GPCC_STREAM_MEMSTREAMREADER_HPP_

#include "StreamReaderBase.hpp"

namespace gpcc
{
namespace stream
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

} // namespace stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_MEMSTREAMREADER_HPP_ */
