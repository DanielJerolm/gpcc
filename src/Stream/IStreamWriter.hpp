/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_STREAM_ISTREAMWRITER_HPP_
#define SRC_GPCC_STREAM_ISTREAMWRITER_HPP_

#include <cstdint>
#include <cstddef>
#include <string>
#include <limits>

namespace gpcc
{
namespace Stream
{

/**
 * @ingroup GPCC_STREAM
 * @{
 */

/**
 * \brief Interface for encoding data into a binary stream.
 *
 * This is an abstract base class for subclasses offering write access to data streams:
 * - @ref MemStreamWriter
 * - Classes offering write access to EEPROM sections
 * - Classes offering write access to files
 *
 * This is the opposite to class @ref IStreamReader.
 *
 * # States of the stream
 * The stream can be in one of four states:
 * - @ref States::open
 * - @ref States::full
 * - @ref States::closed
 * - @ref States::error
 *
 * The current state can be retrieved via @ref GetState().
 *
 * After instantiating a subclass of this class, the stream is usually in the _open_-state
 * and data can be written to it. The stream accepts data until it is either closed, an error
 * occurs, or the storage behind it is exhausted. A subclass is also allowed to initialize
 * the stream in the _full_- or _error_-state.
 *
 * If the capacity of the stream is exhausted, it will enter the _full_-state.\n
 * If any error occurs during writing to the stream (either full or not), it will enter
 * the _error_-state.\n
 * If the stream is closed, then it will enter the _closed_-state. The _closed_-state cannot be left.
 *
 * Any write to a stream that is not in the _open_-state will fail, except zero bits/bytes are written.
 *
 * # Closing a stream
 * _Before a stream instance can be released, it must be closed._
 *
 * It is recommended to invoke @ref Close() before releasing the stream object in order to close the
 * stream. If @ref Close() is not invoked, then the object's destructor will finally invoke it.
 *
 * If @ref Close() is invoked by the destructor and if the close-operation fails, then the application
 * will be terminated via @ref gpcc::osal::Panic(). It is therefore recommended to invoke @ref Close()
 * _before_ releasing the object. This also gives you the chance to catch potential exceptions.
 *
 * # Data Encoding
 * The data written into the stream is packed. There are no padding bytes included in the stream
 * to align the data elements to the natural alignment of their underlying types.
 *
 * Bit-based data is packed on bit-level. If byte-based data follows after bit-based data, then
 * spare bits are inserted to align the byte-based data to the next byte boundary if necessary.
 * No spare bits are inserted if any write-operation is invoked with number of elements set to zero.
 *
 * Data type `bool` is encoded as bit.
 *
 * Word-based data (16bit and above) can be encoded in little or big endian format. The
 * configured endian can be retrieved via @ref GetEndian().
 *
 * # Writing bit-based data
 * The smallest piece of data that can be written to the stream is one byte. Bit-based data written to
 * this interface is therefore not immediately written to the stream. Instead it is cached in a separate
 * storage location until 8 bits have been accumulated in the storage location or until padding bits are
 * added to achieve byte alignment for the next written data. When 8 bits have been accumulated then one
 * byte is written to the stream and the remaining capacity (details: see next chapter) of the stream is
 * reduced.
 *
 * _Writing less than 8 bits to the stream will therefore not decrease its capacity immediately!_\n
 * However, writing a single bit to a _full_ stream will fail immediately.
 *
 * Invoking any write-method with number of elements set to zero will not trigger any write to the
 * stream and therefore no padding bits will be inserted.
 *
 * Example:\n
 * When writing one bit and one byte to the stream, then the stream's capacity will be decremented by 2
 * when the byte is written.
 *
 * # Capacity
 * The capacity of the stream is determined by the subclass. The currently remaining capacity can
 * be retrieved via @ref RemainingCapacity().
 *
 * __Note:__\n
 * - Some sub-classes are not capable of calculating the remaining capacity. In these cases, @ref RemainingCapacity()
 *   will throw. @ref IsRemainingCapacitySupported() can be used to determine if the sub-class supports
 *   @ref RemainingCapacity() or not.
 * - Bits written to the stream are accumulated in a special storage location. They do not decrement the
 *   stream's capacity until at least 8 bits have been accumulated and are written to the stream.
 *
 * # Performance
 * Data is written to the stream byte by byte.
 *
 * Methods writing std::string and methods writing arrays of `uint8_t`, `int8_t`, and `char` provide a higher
 * performance because they do not need to care about the endianess of the written data. This allows sub-classes
 * to use optimized copy methods like `memcpy`.
 */
class IStreamWriter
{
  public:
    /// States of the @ref IStreamWriter.
    enum class States
    {
      open,     ///<Stream is open and data can be written.
      full,     ///<Stream is full. No more data can be written.
      closed,   ///<Stream is closed. No data can be written.
                /**<The stream can be released in this state. */
      error     ///<Stream is in error state. No more data can be written.
    };

    /// Endians for encoding of the data.
    enum class Endian
    {
      Little,   ///<Streamed data is encoded in little-endian format.
      Big       ///<Streamed data is encoded in big-endian format.
    };

    /// Native/preferred endian on the machine.
    static Endian const nativeEndian;

    virtual ~IStreamWriter(void) = default;


    inline IStreamWriter& operator<< (uint8_t     const   value) { Write_uint8(value);  return *this; }
    inline IStreamWriter& operator<< (uint16_t    const   value) { Write_uint16(value); return *this; }
    inline IStreamWriter& operator<< (uint32_t    const   value) { Write_uint32(value); return *this; }
    inline IStreamWriter& operator<< (uint64_t    const   value) { Write_uint64(value); return *this; }
    inline IStreamWriter& operator<< (int8_t      const   value) { Write_int8(value);   return *this; }
    inline IStreamWriter& operator<< (int16_t     const   value) { Write_int16(value);  return *this; }
    inline IStreamWriter& operator<< (int32_t     const   value) { Write_int32(value);  return *this; }
    inline IStreamWriter& operator<< (int64_t     const   value) { Write_int64(value);  return *this; }
    inline IStreamWriter& operator<< (float       const   value) { Write_float(value);  return *this; }
    inline IStreamWriter& operator<< (double      const   value) { Write_double(value); return *this; }
    inline IStreamWriter& operator<< (bool        const   value) { Write_bool(value);   return *this; }
    inline IStreamWriter& operator<< (char        const   value) { Write_char(value);   return *this; }
    inline IStreamWriter& operator<< (std::string const & value) { Write_string(value); return *this; }


    virtual States GetState(void) const = 0;
    virtual Endian GetEndian(void) const = 0;

    virtual bool IsRemainingCapacitySupported(void) const = 0;
    virtual size_t RemainingCapacity(void) const = 0;
    virtual uint_fast8_t GetNbOfCachedBits(void) const = 0;

    virtual void Close(void) = 0;

    virtual uint_fast8_t AlignToByteBoundary(bool const fillWithOnesNotZeros) = 0;
    virtual void FillBits(size_t n, bool const oneNotZero) = 0;
    virtual void FillBytes(size_t n, uint8_t const value) = 0;

    virtual void Write_uint8(uint8_t data) = 0;
    virtual void Write_uint8(uint8_t const * pData, size_t n) = 0;
    virtual void Write_uint16(uint16_t data) = 0;
    virtual void Write_uint16(uint16_t const * pData, size_t n) = 0;
    virtual void Write_uint32(uint32_t data) = 0;
    virtual void Write_uint32(uint32_t const * pData, size_t n) = 0;
    virtual void Write_uint64(uint64_t data) = 0;
    virtual void Write_uint64(uint64_t const * pData, size_t n) = 0;
    virtual void Write_int8(int8_t data) = 0;
    virtual void Write_int8(int8_t const * pData, size_t n) = 0;
    virtual void Write_int16(int16_t data) = 0;
    virtual void Write_int16(int16_t const * pData, size_t n) = 0;
    virtual void Write_int32(int32_t data) = 0;
    virtual void Write_int32(int32_t const * pData, size_t n) = 0;
    virtual void Write_int64(int64_t data) = 0;
    virtual void Write_int64(int64_t const * pData, size_t n) = 0;
    virtual void Write_float(float data) = 0;
    virtual void Write_float(float const * pData, size_t n) = 0;
    virtual void Write_double(double data) = 0;
    virtual void Write_double(double const * pData, size_t n) = 0;
    virtual void Write_bool(bool data) = 0;
    virtual void Write_bool(bool const * pData, size_t n) = 0;
    virtual void Write_Bit(bool data) = 0;
    virtual void Write_Bits(uint8_t bits, uint_fast8_t n) = 0;
    virtual void Write_Bits(uint8_t const * pData, size_t n) = 0;
    virtual void Write_char(char data) = 0;
    virtual void Write_char(char const * pData, size_t n) = 0;
    virtual void Write_string(std::string const & str) = 0;
    virtual void Write_line(std::string const & str) = 0;

  protected:
    IStreamWriter(void) noexcept = default;
    IStreamWriter(const IStreamWriter&) noexcept = default;
    IStreamWriter(IStreamWriter&&) noexcept = default;

    IStreamWriter& operator=(const IStreamWriter&) noexcept = default;
    IStreamWriter& operator=(IStreamWriter&&) noexcept = default;
};

/**
 * \fn IStreamWriter& IStreamWriter::operator<< (uint8_t const value)
 *
 * \brief Writes data to the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param value Value to be written.
 * \return Reference to this.
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (uint16_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (uint32_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (uint64_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (int8_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (int16_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (int32_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (int64_t const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (float const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (double const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (bool const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (char const value)
 * \copydoc gpcc::Stream::IStreamWriter::operator<< (uint8_t const)
 */
/**
 * \fn IStreamWriter& IStreamWriter::operator<< (std::string const & value)
 *
 * \brief Writes a string to the stream (incl. null-terminator).
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param value
 * Reference to an std::string instance that contains the text that shall be written.
 * The null-terminator is written into the stream, too.
 */

/**
 * \fn IStreamWriter::States IStreamWriter::GetState(void) const
 *
 * \brief Retrieves the current state of the stream writer.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return Current state of the stream writer.
 */
/**
 * \fn IStreamWriter::Endian IStreamWriter::GetEndian(void) const
 *
 * \brief Retrieves the endian of the data encoded in the stream.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return Endian of the data encoded in the stream.
 */

/**
 * \fn bool IStreamWriter::IsRemainingCapacitySupported(void) const
 *
 * \brief Queries if @ref RemainingCapacity() is supported.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true   @ref RemainingCapacity() is supported.
 * \retval false  @ref RemainingCapacity() is not supported.
 */

/**
 * \fn size_t IStreamWriter::RemainingCapacity(void) const
 *
 * \brief Retrieves the remaining capacity of the stream.
 *
 * This operation is not supported by all implementations of this interface.\n
 * Use @ref IsRemainingCapacitySupported() to query if the method is supported.
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open) or
 *        [States::full](@ref gpcc::Stream::IStreamWriter::States::full).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong exception safety:\n
 * Operations can fail, but failed operations are guaranteed to have no side effects, so all data retain their original values.
 *
 * \throws ClosedError       Stream is already closed ([details](@ref gpcc::Stream::ClosedError)).
 * \throws ErrorStateError   Stream is in error state ([details](@ref gpcc::Stream::ErrorStateError)).
 * \throws std::logic_error  Operation not supported.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Number of bytes that can be written to the stream until the stream or the storage behind it is full.
 */

/**
 * \fn IStreamWriter::GetNbOfCachedBits
 * \brief Retrieves the number of cached bits which have not yet been written to the stream.
 *
 * Bits written to a stream are cached and are not immediately written to the stream. A byte of data will be written
 * to the stream after at least eight bits have been accumulated or if byte-based data shall be written, or if the
 * stream shall be closed.
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open) or
 *        [States::full](@ref gpcc::Stream::IStreamWriter::States::full).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws ClosedError       Stream is already closed ([details](@ref gpcc::Stream::ClosedError)).
 * \throws ErrorStateError   Stream is in error state ([details](@ref gpcc::Stream::ErrorStateError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Number of bits in the cache that have not yet been written to the stream.
 */

/**
 * \fn void IStreamWriter::Close(void)
 *
 * \brief Closes the stream if it is not yet closed.
 *
 * Depending on the sub-class, this method may write buffered data to the stream
 * before the stream is closed. These operations may fail, so be aware that this
 * method may throw an exception.
 *
 * If the stream is in state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), or if the
 * close-operation fails, then the exact behavior of [Close](@ref gpcc::Stream::IStreamWriter::Close()) depends
 * on the underlying sub-class:
 * - if the target of the stream is plain memory, then the memory could contain undefined/incomplete data.
 * - if the target of the stream is a new file or a new EEPROM section, then the file/section could
 *   be erased again or it is simply never created, or it could be left with undefined data.
 * It is strongly recommended to check the sub-class' documentation for behavior in case of an exception.
 *
 * The stream must always be closed before it is released. If it is not closed when
 * it is released, then the destructor of the sub-class will close it before release.
 * If an error occurs during close in this situation, then the destructor cannot handle
 * it and the application will be terminated via @ref gpcc::osal::Panic(). This behavior
 * is usually not desired, so it is recommended to close the stream manually before
 * releasing the stream object.
 *
 * If the stream is already in state [States::closed](@ref gpcc::Stream::IStreamWriter::States::closed), then this
 * method has no effect and it will not throw any exception.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - In any case, the stream will always be closed
 * - If the sub-class works on a file, then the file-descriptor might be left in an undefined state (-> POSIX).\n
 *   Discussions on the www show that most operating systems close and recycle the file descriptor even
 *   though close(), fclose(), or whatever reported an error. The discussions also show that there is not
 *   really anything more one can do in such a situation.
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Cancellation not allowed. Cancellation could corrupt the object or lead to undefined behavior.
 */

/**
 * \fn IStreamWriter::AlignToByteBoundary
 * \brief Aligns the stream to the next byte boundary by writing ones or zeros.
 *
 * This will have no effect, if the stream is already aligned to a byte boundary (= no cached bits).
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * \post  The number of cached bits will be zero.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - [ClosedError](@ref gpcc::Stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::Stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * - - -
 *
 * \param fillWithOnesNotZeros
 * Determines if ones (true) or zeros (false) shall be added to achieve byte alignment.
 *
 * \return
 * Number of bits added to the stream in order to align to the next byte boundary.\n
 * This is always in the range [0..7].
 */

/**
 * \fn void IStreamWriter::FillBits(size_t n, bool const oneNotZero)
 * \brief Writes a couple of bits (all ones or all zeros) to the stream.
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - [ClosedError](@ref gpcc::Stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::Stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * - - -
 *
 * \param n
 * Number of bits that shall be written. Zero is allowed.
 * \param oneNotZero
 * Value that shall be written:\n
 * true  = '1'\n
 * false = '0'
 */

/**
 * \fn void IStreamWriter::FillBytes(size_t n, uint8_t const value)
 * \brief Writes a couple of bytes (all with the same value) to the stream.
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::Stream::IOError)
 * - [FullError](@ref gpcc::Stream::FullError)
 * - [ClosedError](@ref gpcc::Stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::Stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * - - -
 *
 * \param n
 * Number of bytes that shall be written. Zero is allowed.
 * \param value
 * Value that shall be written.
 */

/**
 * \fn void IStreamWriter::Write_uint8(uint8_t data)
 *
 * \brief Writes data to the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * Note: This method writes one element of data. There is an overloaded version
 * writing `n` elements of data.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param data Value to be written.
 */
/**
 * \fn void IStreamWriter::Write_uint8(uint8_t const * pData, size_t n)
 *
 * \brief Writes data to the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * Note: This method writes `n` elements of data. There is an overloaded version
 * writing one element of data.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param pData
 * Pointer to the data elements to be written.
 * \param n
 * Number of elements to be written. Zero is allowed.\n
 * _Note: Writing zero will not trigger insertion of padding bits, if there are any bits that have not_
 * _yet been written to the stream._
 */
/**
 * \fn void IStreamWriter::Write_uint16(uint16_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_uint16(uint16_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_uint32(uint32_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_uint32(uint32_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_uint64(uint64_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_uint64(uint64_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_int8(int8_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_int8(int8_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_int16(int16_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_int16(int16_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_int32(int32_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_int32(int32_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_int64(int64_t data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_int64(int64_t const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_float(float data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_float(float const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_double(double data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_double(double const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_bool(bool data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_bool(bool const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn void IStreamWriter::Write_Bit(bool data)
 *
 * \brief Writes one bit of data to the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * Note: This method writes one bit of data. There are methods
 * [Write_Bits(uint8_t bits, uint_fast8_t n)](@ref gpcc::Stream::IStreamWriter::Write_Bits(uint8_t bits, uint_fast8_t n))
 * and [Write_Bits(uint8_t const * pData, size_t n)](@ref gpcc::Stream::IStreamWriter::Write_Bits(uint8_t const * pData, size_t n))
 * writing multiple bits of data to the stream.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param data Data to be written.
 */
/**
 * \fn void IStreamWriter::Write_Bits(uint8_t bits, uint_fast8_t n)
 *
 * \brief Writes up to 8 bits of data to the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * Note: This method writes `n` (max 8) bits of data. There is a method [Write_Bit()](@ref gpcc::Stream::IStreamWriter::Write_Bit())
 * writing one bit of data and [Write_Bits(uint8_t const * pData, size_t n)](@ref gpcc::Stream::IStreamWriter::Write_Bits(uint8_t const * pData, size_t n))
 * writing multiple bits of data to the stream.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param bits
 * A byte containing the bits that shall be written. The bits must be aligned to the LSB.
 * Upper bits that are not written are ignored.
 * \param n
 * Number of bits to be written (0..8).
 */
/**
 * \fn void IStreamWriter::Write_Bits(uint8_t const * pData, size_t n)
 *
 * \brief Writes multiple bits of data to the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * Note: This method writes `n` bits of data. There is a method [Write_Bit()](@ref gpcc::Stream::IStreamWriter::Write_Bit())
 * writing one bit of data and [Write_Bits(uint8_t bits, uint_fast8_t n)](@ref gpcc::Stream::IStreamWriter::Write_Bits(uint8_t bits, uint_fast8_t n))
 * writing up to 8 bits of data to the stream.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param pData
 * Pointer to an array of bytes containing the bits to be written.\n
 * The first bit must be located at the LSB of the first 8-bit word of data.
 * Upper bits in the last 8-bit word that are not written are ignored.
 * \param n
 * Number of bits to be written. Zero is allowed.
 */
/**
 * \fn void IStreamWriter::Write_char(char data)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t)
 */
/**
 * \fn void IStreamWriter::Write_char(char const * pData, size_t n)
 * \copydoc gpcc::Stream::IStreamWriter::Write_uint8(uint8_t const *,size_t)
 */
/**
 * \fn bool IStreamWriter::Write_string(std::string const & str)
 *
 * \brief Writes a string to the stream (incl. null-terminator).
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param str
 * Reference to an std::string instance that contains the text that shall be written.
 * The null-terminator is written into the stream, too.
 */
/**
 * \fn bool IStreamWriter::Write_line(std::string const & str)
 *
 * \brief Writes a line to the stream. Basically a string is written, but instead of
 * using a null-terminator, the string is terminated using '\\n'.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::Stream::IStreamWriter::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
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
 * - the stream enters state [States::error](@ref gpcc::Stream::IStreamWriter::States::error), if the stream
 *   cannot be recovered (e.g. undo a write)
 *
 * ---
 *
 * \param str
 * Reference to an std::string instance that contains the line of text that shall be written.
 * The line is terminated using '\n'. A null-terminator is not written into the stream.
 */

/**
 * @}
 */

} // namespace Stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_ISTREAMWRITER_HPP_ */
