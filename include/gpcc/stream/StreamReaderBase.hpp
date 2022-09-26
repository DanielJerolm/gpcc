/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_STREAM_STREAMREADERBASE_HPP_
#define SRC_GPCC_STREAM_STREAMREADERBASE_HPP_

#include <gpcc/stream/IStreamReader.hpp>

namespace gpcc
{
namespace stream
{

/**
 * @ingroup GPCC_STREAM
 * @{
 */

/**
 * \brief Convenient base class for all classes implementing @ref IStreamReader.
 *
 * Subclasses just have to implement the following methods to implement the @ref IStreamReader interface:
 * - @ref IStreamReader::IsRemainingBytesSupported()
 * - @ref IStreamReader::RemainingBytes()
 * - @ref IStreamReader::EnsureAllDataConsumed()
 * - @ref IStreamReader::Close()
 * - @ref IStreamReader::Read_line()
 * - @ref StreamReaderBase::Pop(void)
 * - @ref StreamReaderBase::Pop(void* p, size_t n)
 * - @ref StreamReaderBase::PopBits()
 *
 * For performance reasons, the following methods should be reimplemented:
 * - @ref StreamReaderBase::Skip()
 * - @ref StreamReaderBase::Read_string()
 */
class StreamReaderBase: public IStreamReader
{
  public:
    // --> IStreamReader
    virtual States GetState(void) const override;
    virtual Endian GetEndian(void) const override;

    virtual void Skip(size_t nBits) override;

    virtual uint8_t     Read_uint8(void) override;
    virtual uint16_t    Read_uint16(void) override;
    virtual uint32_t    Read_uint32(void) override;
    virtual uint64_t    Read_uint64(void) override;
    virtual int8_t      Read_int8(void) override;
    virtual int16_t     Read_int16(void) override;
    virtual int32_t     Read_int32(void) override;
    virtual int64_t     Read_int64(void) override;
    virtual float       Read_float(void) override;
    virtual double      Read_double(void) override;
    virtual bool        Read_bool(void) override;
    virtual bool        Read_bit(void) override;
    virtual uint8_t     Read_bits(uint_fast8_t n) override;
    virtual char        Read_char(void) override;
    virtual std::string Read_string(void) override;

    virtual void Read_uint8( uint8_t*  pDest, size_t n) override;
    virtual void Read_uint16(uint16_t* pDest, size_t n) override;
    virtual void Read_uint32(uint32_t* pDest, size_t n) override;
    virtual void Read_uint64(uint64_t* pDest, size_t n) override;
    virtual void Read_int8(  int8_t*   pDest, size_t n) override;
    virtual void Read_int16( int16_t*  pDest, size_t n) override;
    virtual void Read_int32( int32_t*  pDest, size_t n) override;
    virtual void Read_int64( int64_t*  pDest, size_t n) override;
    virtual void Read_float( float*    pDest, size_t n) override;
    virtual void Read_double(double*   pDest, size_t n) override;
    virtual void Read_bool(  bool*     pDest, size_t n) override;
    virtual void Read_bits(  uint8_t*  pDest, size_t n) override;
    virtual void Read_char(  char*     pDest, size_t n) override;
    // <-- IStreamReader

  protected:
    /// Current state of the stream reader.
    States state;

    /// Endian of the data to be read.
    Endian endian;


    StreamReaderBase(void) = delete;
    StreamReaderBase(States const _state, Endian const _endian) noexcept;
    StreamReaderBase(const StreamReaderBase&) noexcept = default;
    StreamReaderBase(StreamReaderBase&&) noexcept = default;
    virtual ~StreamReaderBase(void) = default;


    StreamReaderBase& operator=(const StreamReaderBase&) noexcept = default;
    StreamReaderBase& operator=(StreamReaderBase&&) noexcept = default;


    virtual unsigned char Pop(void) = 0;
    virtual void Pop(void* p, size_t n) = 0;
    virtual uint8_t PopBits(uint_fast8_t n) = 0;
};

/**
 * \fn unsigned char StreamReaderBase::Pop(void)
 *
 * \brief Pops one byte of data from the stream.
 *
 * There is an overloaded version of this method available that pops multiple bytes of data from
 * the stream. The overloaded version should be preferred for arrays of 8-bit data due to
 * better performance.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream cannot
 *   be recovered (e.g. undo a read)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::stream::IOError)
 * - [EmptyError](@ref gpcc::stream::EmptyError)
 * - [ClosedError](@ref gpcc::stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream cannot
 *   be recovered (e.g. undo a read)
 *
 * ---
 *
 * \return The byte popped from the stream.
 */
/**
 * \fn void StreamReaderBase::Pop(void* p, size_t n)
 *
 * \brief Pops multiple bytes of data from the stream.
 *
 * There is an overloaded version of this method available that pops one byte of data from
 * the stream. This version provides better performance for arrays of 8-bit data.
 * The overloaded version is optimized for single bytes.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream cannot
 *   be recovered (e.g. undo a read)
 * - the memory referenced by parameter `p` may contain undefined data
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::stream::IOError)
 * - [EmptyError](@ref gpcc::stream::EmptyError)
 * - [ClosedError](@ref gpcc::stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream cannot
 *   be recovered (e.g. undo a read)
 * - the memory referenced by parameter `p` may contain undefined data
 *
 * ---
 *
 * \param p
 * The popped data is written to the storage referenced by this.
 * \param n
 * Number of bytes to be popped. Zero is allowed.\n
 * _Note: In case of zero, any bits from the last read byte that have not yet been read will not be discarded._
 */
/**
 * \fn uint8_t StreamReaderBase::PopBits(uint_fast8_t n)
 *
 * \brief Pops up to 8 bits of data from the stream.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream cannot
 *   be recovered (e.g. undo a read)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::stream::IOError)
 * - [EmptyError](@ref gpcc::stream::EmptyError)
 * - [ClosedError](@ref gpcc::stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream cannot
 *   be recovered (e.g. undo a read)
 *
 * ---
 *
 * \param n
 * Number of bits that shall be popped. Zero is allowed.
 * \return
 * Byte containing the popped bits. The first bit is sitting at the byte's LSB. Unused upper bits are zero.
 */

/**
 * @}
 */

} // namespace stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_STREAMREADERBASE_HPP_ */
