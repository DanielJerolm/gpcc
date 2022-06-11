/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef SRC_GPCC_STREAM_STREAMWRITERBASE_HPP_
#define SRC_GPCC_STREAM_STREAMWRITERBASE_HPP_

#include "IStreamWriter.hpp"

namespace gpcc
{
namespace Stream
{

/**
 * @ingroup GPCC_STREAM
 * @{
 */

/**
 * \brief Convenient base class for all classes implementing @ref IStreamWriter.
 *
 * Subclasses just have to implement @ref Push() and @ref PushBits() to implement the @ref IStreamWriter interface.
 */
class StreamWriterBase: public IStreamWriter
{
  public:
    // --> IStreamWriter
    virtual States GetState(void) const override;
    virtual Endian GetEndian(void) const override;

    virtual uint_fast8_t AlignToByteBoundary(bool const fillWithOnesNotZeros) override;
    virtual void FillBits(size_t n, bool const oneNotZero) override;
    virtual void FillBytes(size_t n, uint8_t const value) override;

    virtual void Write_uint8(uint8_t data) override;
    virtual void Write_uint8(uint8_t const * pData, size_t n) override;
    virtual void Write_uint16(uint16_t data) override;
    virtual void Write_uint16(uint16_t const * pData, size_t n) override;
    virtual void Write_uint32(uint32_t data) override;
    virtual void Write_uint32(uint32_t const * pData, size_t n) override;
    virtual void Write_uint64(uint64_t data) override;
    virtual void Write_uint64(uint64_t const * pData, size_t n) override;
    virtual void Write_int8(int8_t data) override;
    virtual void Write_int8(int8_t const * pData, size_t n) override;
    virtual void Write_int16(int16_t data) override;
    virtual void Write_int16(int16_t const * pData, size_t n) override;
    virtual void Write_int32(int32_t data) override;
    virtual void Write_int32(int32_t const * pData, size_t n) override;
    virtual void Write_int64(int64_t data) override;
    virtual void Write_int64(int64_t const * pData, size_t n) override;
    virtual void Write_float(float data) override;
    virtual void Write_float(float const * pData, size_t n) override;
    virtual void Write_double(double data) override;
    virtual void Write_double(double const * pData, size_t n) override;
    virtual void Write_bool(bool data) override;
    virtual void Write_bool(bool const * pData, size_t n) override;
    virtual void Write_Bit(bool data) override;
    virtual void Write_Bits(uint8_t bits, uint_fast8_t n) override;
    virtual void Write_Bits(uint8_t const * pData, size_t n) override;
    virtual void Write_char(char data) override;
    virtual void Write_char(char const * pData, size_t n) override;
    virtual void Write_string(std::string const & str) override;
    virtual void Write_line(std::string const & str) override;
    // <-- IStreamWriter

  protected:
    /// Current state of the stream writer.
    States state;

    /// Endian of the data to be written.
    Endian endian;

    StreamWriterBase(void) = delete;
    StreamWriterBase(States const _state, Endian const _endian) noexcept;
    StreamWriterBase(const StreamWriterBase&) noexcept = default;
    StreamWriterBase(StreamWriterBase&&) noexcept = default;
    virtual ~StreamWriterBase(void) = default;


    StreamWriterBase& operator=(const StreamWriterBase&) noexcept = default;
    StreamWriterBase& operator=(StreamWriterBase&&) noexcept = default;


    virtual void Push(char c) = 0;
    virtual void Push(void const * pData, size_t n) = 0;
    virtual void PushBits(uint8_t bits, uint_fast8_t n) = 0;
};

/**
 * \fn virtual void StreamWriterBase::Push(char c)
 *
 * \brief Pushes one byte of data onto the stream.
 *
 * There is an overloaded version of this method available that pushes multiple bytes onto
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream cannot
 *   be recovered (e.g. undo a write)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - [ClosedError](@ref gpcc::Stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::Stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream cannot
 *   be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param c Byte of data that shall be pushed onto the stream.
 */
/**
 * \fn virtual void StreamWriterBase::Push(void const * pData, size_t n)
 *
 * \brief Pushes multiple bytes of byte-based-data onto the stream.
 *
 * There is an overloaded version of this method available that pushes single bytes onto
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream cannot
 *   be recovered (e.g. undo a write)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - [ClosedError](@ref gpcc::Stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::Stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream cannot
 *   be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param pData
 * Pointer to the data that shall be pushed onto the stream.
 * \param n
 * Number of bytes that shall be pushed onto the stream. Zero is allowed.\n
 * _Note: Writing zero will not trigger insertion of padding bits, if there are any bits that have not_
 * _yet been written to the stream._
 */
/**
 * \fn virtual void StreamWriterBase::PushBits(uint8_t bits, uint_fast8_t n)
 *
 * \brief Pushes up to 8 bits of data onto the stream.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream cannot
 *   be recovered (e.g. undo a write)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - [ClosedError](@ref gpcc::Stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::Stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream cannot
 *   be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param bits
 * A byte containing the bits that shall be written. The bits must be aligned to the LSB.
 * Upper bits that are not written are ignored.
 * \param n
 * Number of bits that shall be written (0..8).
 */

/**
 * @}
 */

} // namespace Stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_STREAMWRITERBASE_HPP_ */
