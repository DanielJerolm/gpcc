/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SRC_GPCC_STREAM_ISTREAMREADER_HPP_
#define SRC_GPCC_STREAM_ISTREAMREADER_HPP_

#include <limits>
#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc
{
namespace stream
{

/**
 * @ingroup GPCC_STREAM
 * @{
 */

/**
 * \brief Interface for decoding data from a binary stream.
 *
 * This is an abstract base class for subclasses offering read access to data streams:
 * - @ref MemStreamReader
 * - Classes offering read access to EEPROM sections
 * - Classes offering read access to files
 *
 * This is the opposite to class @ref IStreamWriter.
 *
 * # States of the stream
 * The stream can be in one of four states:
 * - @ref States::open
 * - @ref States::empty
 * - @ref States::closed
 * - @ref States::error
 *
 * The current state can be retrieved via @ref GetState().
 *
 * After instantiating a subclass of this class, the stream is usually in the _open_-state
 * and data can be read from it. The stream accepts read-accesses until it is either closed,
 * an error occurs, or until all available data has been read. A subclass is also allowed to
 * initialize the stream in the _empty_- or _error_-state.
 *
 * If all available data has been read, the stream will enter the _empty_-state.\n
 * If any error occurs during reading from the stream (either empty or not), it will enter
 * the _error_-state.\n
 * If the stream is closed, then it will enter the _closed_-state. The _closed_-state cannot be left.
 *
 * Any read access to a stream that is not in the _open_-state will fail, except zero bits/bytes are
 * requested to be read.
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
 * The data inside the stream is packed. There are no padding bytes included in the stream
 * to align the data elements to the natural alignment of their underlying types.
 *
 * Bit-based data inside the stream is packed on bit-level. If byte-based data follows after bit-
 * based data, then spare bits are expected in the stream which align the byte-based data to the
 * next byte-boundary if necessary.
 *
 * Data type `bool` is mapped to single bits inside the stream.
 *
 * Word-based data (16bit and above) can be decoded in little- or big-endian format. The
 * configured endianess can be retrieved via @ref GetEndian().
 *
 * # Reading bit-based data
 * The smallest piece of data that can be read from the stream is one byte. If bit-based data shall be
 * read from this interface, then one byte is read from the stream and the bits are retrieved by the read-
 * method. Bits included in the read byte that are not read are stored in a special intermediate storage
 * location. If further bits shall be read, then the data stored in the intermediate storage location is
 * used first before a new byte is read from the stream.
 *
 * This means:\n
 * When reading one bit, the stream's remaining number of bytes will be decremented.\n
 * Reading up to 7 further bits will not decrease the remaining number of bytes, because data from the
 * intermediate storage location will be used.
 *
 * If byte-based data is read after reading bit-based data, then the content of the intermediate storage
 * location will be discarded. This happens because any byte-based data must start on a byte-boundary.\n
 * Example:\n
 * After reading one bit and one byte from the stream, the stream's remaining number of bytes
 * will be decremented by 2.
 *
 * Invoking any read-method with number of elements to be read set to zero will not clear the
 * intermediate storage.
 *
 * # Remaining number of bytes
 * The remaining number of bytes that can be read from the stream is tracked by the subclass.
 * The currently remaining number of bytes can be retrieved via @ref RemainingBytes().
 *
 * __Note:__\n
 * - Some sub-classes are not capable to determine the remaining number of bytes. In these cases, @ref RemainingBytes()
 *   will throw. @ref IsRemainingBytesSupported() can be used to determine if the sub-class supports
 *   @ref RemainingBytes() or not.
 * - A stream with zero remaining bytes might have 7 bits that could still be read.
 * - @ref EnsureAllDataConsumed() can be used to check if the remaining number of bits meets the user's
 *   expectations.
 *
 * # Performance
 * Data is read from the stream byte by byte.
 *
 * Methods reading arrays of `uint8_t`, `int8_t`, and `char` provide a higher performance because they do
 * not need to care for the endianess of the read data. This allows sub-classes to use optimized copy
 * methods like memcpy.
 */
class IStreamReader
{
  public:
    /// States of the @ref IStreamReader.
    enum class States
    {
      open,     ///<Stream is open and data can be read.
      empty,    ///<Stream is empty. No more data can be read.
      closed,   ///<Stream is closed. No data can be read.
                /**<The stream can be released in this state. */
      error     ///<Stream is in error state. No more data can be read.
    };

    /// Endians for encoding of the data.
    enum class Endian
    {
      Little,   ///<Streamed data is encoded in little-endian format.
      Big       ///<Streamed data is encoded in big-endian format.
    };

    /// Expectations for remaining number of bits.
    /** This can be used in conjunction with @ref EnsureAllDataConsumed() to check if the complete stream
        has been read. */
    enum class RemainingNbOfBits : uint8_t
    {
      zero = 0U,      ///<Zero bits remaining.
      one,            ///<One bit remaining.
      two,            ///<Two bits remaining.
      three,          ///<Three bits remaining.
      four,           ///<Four bits remaining.
      five,           ///<Five bits remaining.
      six,            ///<Six bits remaining.
      seven,          ///<Seven bits remaning.
      sevenOrLess,    ///<Up to seven bits remaining.
      moreThanSeven,  ///<More than seven bits remaining.
      any             ///<Any number of bits remaining (= don´t care)
    };

    /// Native/preferred endian on the machine.
    static Endian const nativeEndian;

    virtual ~IStreamReader(void) = default;


    inline IStreamReader& operator>> (uint8_t     & value) { value = Read_uint8();  return *this; }
    inline IStreamReader& operator>> (uint16_t    & value) { value = Read_uint16(); return *this; }
    inline IStreamReader& operator>> (uint32_t    & value) { value = Read_uint32(); return *this; }
    inline IStreamReader& operator>> (uint64_t    & value) { value = Read_uint64(); return *this; }
    inline IStreamReader& operator>> (int8_t      & value) { value = Read_int8();   return *this; }
    inline IStreamReader& operator>> (int16_t     & value) { value = Read_int16();  return *this; }
    inline IStreamReader& operator>> (int32_t     & value) { value = Read_int32();  return *this; }
    inline IStreamReader& operator>> (int64_t     & value) { value = Read_int64();  return *this; }
    inline IStreamReader& operator>> (float       & value) { value = Read_float();  return *this; }
    inline IStreamReader& operator>> (double      & value) { value = Read_double(); return *this; }
    inline IStreamReader& operator>> (bool        & value) { value = Read_bool();   return *this; }
    inline IStreamReader& operator>> (char        & value) { value = Read_char();   return *this; }
    inline IStreamReader& operator>> (std::string & value) { value = Read_string(); return *this; }


    virtual States GetState(void) const = 0;
    virtual Endian GetEndian(void) const = 0;

    virtual bool IsRemainingBytesSupported(void) const = 0;
    virtual size_t RemainingBytes(void) const = 0;
    virtual void EnsureAllDataConsumed(RemainingNbOfBits const expectation) const = 0;

    virtual void Close(void) = 0;

    virtual void Skip(size_t nBits) = 0;

    virtual uint8_t     Read_uint8(void)          = 0;
    virtual uint16_t    Read_uint16(void)         = 0;
    virtual uint32_t    Read_uint32(void)         = 0;
    virtual uint64_t    Read_uint64(void)         = 0;
    virtual int8_t      Read_int8(void)           = 0;
    virtual int16_t     Read_int16(void)          = 0;
    virtual int32_t     Read_int32(void)          = 0;
    virtual int64_t     Read_int64(void)          = 0;
    virtual float       Read_float(void)          = 0;
    virtual double      Read_double(void)         = 0;
    virtual bool        Read_bool(void)           = 0;
    virtual bool        Read_bit(void)            = 0;
    virtual uint8_t     Read_bits(uint_fast8_t n) = 0;
    virtual char        Read_char(void)           = 0;
    virtual std::string Read_string(void)         = 0;
    virtual std::string Read_line(void)           = 0;

    virtual void Read_uint8( uint8_t*  pDest, size_t n) = 0;
    virtual void Read_uint16(uint16_t* pDest, size_t n) = 0;
    virtual void Read_uint32(uint32_t* pDest, size_t n) = 0;
    virtual void Read_uint64(uint64_t* pDest, size_t n) = 0;
    virtual void Read_int8(  int8_t*   pDest, size_t n) = 0;
    virtual void Read_int16( int16_t*  pDest, size_t n) = 0;
    virtual void Read_int32( int32_t*  pDest, size_t n) = 0;
    virtual void Read_int64( int64_t*  pDest, size_t n) = 0;
    virtual void Read_float( float*    pDest, size_t n) = 0;
    virtual void Read_double(double*   pDest, size_t n) = 0;
    virtual void Read_bool(  bool*     pDest, size_t n) = 0;
    virtual void Read_bits(  uint8_t*  pDest, size_t n) = 0;
    virtual void Read_char(  char*     pDest, size_t n) = 0;

  protected:
    IStreamReader(void) noexcept = default;
    IStreamReader(const IStreamReader&) noexcept = default;
    IStreamReader(IStreamReader&&) noexcept = default;

    IStreamReader& operator=(const IStreamReader&) noexcept = default;
    IStreamReader& operator=(IStreamReader&&) noexcept = default;
};

/**
 * \fn IStreamReader& IStreamReader::operator>> (uint8_t & value)
 *
 * \brief Reads one element of data from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `value` may contain undefined data
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `value` may contain undefined data
 *
 * ---
 *
 * \param value The read data is written to the referenced variable.
 * \return Reference to this.
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (uint16_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (uint32_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (uint64_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (int8_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (int16_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (int32_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (int64_t & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (float & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (double & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (bool & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (char & value)
 * \copydoc gpcc::stream::IStreamReader::operator>> (uint8_t&)
 */
/**
 * \fn IStreamReader& IStreamReader::operator>> (std::string & value)
 *
 * \brief Reads a null-terminated string from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `value` may contain undefined data
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `value` may contain undefined data
 *
 * ---
 *
 * \param value The read string is written to the referenced variable.
 * \return Reference to this.
 */

/**
 * \fn IStreamReader::States IStreamReader::GetState(void) const
 *
 * \brief Retrieves the actual state of the stream reader.
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
 * \return
 * Current state of the stream reader.
 */

/**
 * \fn IStreamReader::Endian IStreamReader::GetEndian(void) const
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
 * \fn bool IStreamReader::IsRemainingBytesSupported(void) const
 *
 * \brief Queries if @ref RemainingBytes() is supported.
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
 * \retval true   @ref RemainingBytes() is supported.
 * \retval false  @ref RemainingBytes() is not supported.
 */

/**
 * \fn size_t IStreamReader::RemainingBytes(void) const
 *
 * \brief Retrieves the number of bytes that could be read until the stream or the storage behind it becomes empty.
 *
 * This operation is not supported by all implementations of this interface.\n
 * Use @ref IsRemainingBytesSupported() to query if the method is supported.
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open) or
 *        [States::empty](@ref gpcc::stream::IStreamReader::States::empty).
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
 * \throws ClosedError       Stream is already closed ([details](@ref gpcc::stream::ClosedError)).
 * \throws ErrorStateError   Stream is in error state ([details](@ref gpcc::stream::ErrorStateError)).
 * \throws std::logic_error  Operation not supported.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe.
 *
 * ---
 *
 * \return
 * Number of bytes that could be read from the stream until the stream or the storage behind it becomes empty.\n
 * __Note:__\n
 * If zero is returned, then up to 7 bits could still left to be read. Use
 * [GetState()](@ref gpcc::stream::IStreamReader::GetState) to check for
 * [States::empty](@ref gpcc::stream::IStreamReader::States::empty) or use
 * [EnsureAllDataConsumed()](@ref gpcc::stream::IStreamReader::EnsureAllDataConsumed) to check the number of
 * bits left.
 */

/**
 * \fn IStreamReader::EnsureAllDataConsumed
 * \brief Checks if a specific number of bits is remaining to be read and throws if the result is negative.
 *
 * This is intended to be used to check if the expected amounth of data has been read from the stream.
 *
 * \pre   The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open) or
 *        [States::empty](@ref gpcc::stream::IStreamReader::States::empty).
 *
 * \post  The stream's state will explicitly not be modified.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws ClosedError          Stream is already closed ([details](@ref gpcc::stream::ClosedError)).
 *
 * \throws ErrorStateError      Stream is in error state ([details](@ref gpcc::stream::ErrorStateError)).
 *
 * \throws RemainingBitsError   The remaining number of bits in the stream does not match the expectation
 *                              ([details](@ref gpcc::stream::RemainingBitsError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param expectation
 * Expected number of bits left to be read.
 */

/**
 * \fn void IStreamReader::Close(void)
 *
 * \brief Closes the stream if it is not yet closed.
 *
 * Depending on the sub-class, this method may have to close files or EEPROM sections
 * before the stream is closed. These operations may fail, so be aware that this
 * method may throw an exception.
 *
 * The stream must always be closed before it is released. If it is not closed when
 * it is released, then the destructor of the sub-class will close it before release.
 * If an error occurs during close in this situation, then the destructor cannot handle
 * it and the application will be terminated via @ref gpcc::osal::Panic(). This behavior
 * is usually not desired, so it is recommended to close the stream manually before
 * releasing the stream object.
 *
 * If the stream is already in state [States::closed](@ref gpcc::stream::IStreamReader::States::closed), then this
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
 * - If the sub-class works on a file, then the file-descriptor may be left in an undefined state (-> POSIX).
 *   Discussions on the www show that most operating systems close and recycle the file descriptor even
 *   though close(), fclose(), or whatever reported an error. The discussions also show that there is not
 *   really anything more one can do in such a situation.
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::stream::IOError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Cancellation not allowed. Cancellation could corrupt the object or lead to undefined behavior.
 */

/**
 * \fn IStreamReader::Skip
 * \brief Skips a given number of bits in the stream.
 *
 * \pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * The behaviour is the same as if using [Read_bit()](@ref gpcc::stream::IStreamReader::Read_bit) or
 * [Read_bits()](@ref gpcc::stream::IStreamReader::Read_bits) and discarding the read bits.\n
 * However, this method usually provides a better performance and allows to skip one or more bytes.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a skip)
 *
 * You should be aware of the following exceptions:
 * - [IOError](@ref gpcc::stream::IOError)
 * - [EmptyError](@ref gpcc::stream::EmptyError)
 * - [ClosedError](@ref gpcc::stream::ClosedError)
 * - [ErrorStateError](@ref gpcc::stream::ErrorStateError)
 * - any derived from `std::exception`
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a skip)
 *
 * - - -
 *
 * \param nBits
 * Number of bits that shall be skipped.\n
 * Zero is allowed.
 */

/**
 * \fn uint8_t IStreamReader::Read_uint8(void)
 *
 * \brief Reads one element of data from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 *
 * ---
 *
 * \return The read data.
 */
/**
 * \fn uint16_t IStreamReader::Read_uint16(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn uint32_t IStreamReader::Read_uint32(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn uint64_t IStreamReader::Read_uint64(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn int8_t IStreamReader::Read_int8(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn int16_t IStreamReader::Read_int16(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn int32_t IStreamReader::Read_int32(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn int64_t IStreamReader::Read_int64(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn float IStreamReader::Read_float(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn double IStreamReader::Read_double(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn bool IStreamReader::Read_bool(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn bool IStreamReader::Read_bit(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn uint8_t IStreamReader::Read_bits(uint_fast8_t n)
 *
 * \brief Reads up to 8 bits of data from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 *
 * ---
 *
 * \param n
 * Number of bits to be read (0..8).
 * \return
 * A byte containing the read bits. The byte is filled starting with the first read bit at the byte's LSB.
 * Upper unused bits of the byte are zero. If `n` is zero then the return value is zero, too.
 */
/**
 * \fn char IStreamReader::Read_char(void)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(void)
 */
/**
 * \fn std::string IStreamReader::Read_string(void)
 *
 * \brief Reads a null-terminated string from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 *
 * ---
 *
 * \return The read string.
 */
/**
 * \fn std::string IStreamReader::Read_line(void)
 *
 * \brief Reads one line of text from the stream.
 *
 * Reading stops at:
 * - '\\r' (Mac)
 * - '\\n' (Linux/Unix)
 * - '\\r\\n' (Windows)
 * - NUL
 * - End of the stream
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 *
 * ---
 *
 * \return
 * The read string. Any '\\r', '\\n', or '\\r\\n' terminating the line are dropped and not contained in the result.
 */

/**
 * \fn void IStreamReader::Read_uint8(uint8_t* pDest, size_t n)
 *
 * \brief Reads data from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `pDest` may contain undefined data
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `pDest` may contain undefined data
 *
 * ---
 *
 * \param pDest
 * The read data is written to the referenced memory location.
 * \param n
 * Number of elements to be read. Zero is allowed.
 */
/**
 * \fn void IStreamReader::Read_uint16(uint16_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_uint32(uint32_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_uint64(uint64_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_int8(int8_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_int16(int16_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_int32(int32_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_int64(int64_t* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_float(float* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_double(double* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_bool(bool* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */
/**
 * \fn void IStreamReader::Read_bits(uint8_t* pDest, size_t n)
 *
 * \brief Reads multiple bits from the stream.
 *
 * @pre The stream must be in state [States::open](@ref gpcc::stream::IStreamReader::States::open).
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:\n
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `pDest` may contain undefined data
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
 * - the stream enters state [States::error](@ref gpcc::stream::IStreamReader::States::error), if the stream
 *   cannot be recovered (e.g. undo a read)
 * - the memory referenced by parameter `pDest` may contain undefined data
 *
 * ---
 *
 * \param pDest
 * The bits read from the stream are written into the referenced memory location.\n
 * The size of the referenced memory location must be at least n / 8 + 1 bytes.\n
 * The bytes are filled from LSB to MSB. Upper unused bits of the last written byte are zero.
 * \param n
 * Number of bits to be read. Zero is allowed.
 */
/**
 * \fn void IStreamReader::Read_char(char* pDest, size_t n)
 * \copydoc gpcc::stream::IStreamReader::Read_uint8(uint8_t*,size_t)
 */

/**
 * @}
 */

} // namespace stream
} // namespace gpcc

#endif /* SRC_GPCC_STREAM_ISTREAMREADER_HPP_ */
