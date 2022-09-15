/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include <gpcc/stream/MemStreamWriter.hpp>
#include <gpcc/stream/stream_errors.hpp>
#include <gpcc/osal/Panic.hpp>
#include "cstring"

namespace gpcc
{
namespace stream
{

MemStreamWriter::MemStreamWriter(void* const _pMem, size_t const _size, Endian const _endian)
: StreamWriterBase((_size != 0)?States::open : States::full, _endian)
, pMem(reinterpret_cast<char*>(_pMem))
, remainingBytes(_size)
, nbOfBitsWritten(0)
, bitData(0)
/**
 * \brief Constructor.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
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
 * \param _pMem
 * Pointer to the beginning of the block of memory into which the new created MemStreamWriter shall write.\n
 * nullptr is allowed if parameter `_size` is zero.
 * \param _size
 * Size of the block of memory referenced by parameter `_pMem` in bytes.
 * If this is zero, then the initial state of the stream will be @ref States::full.
 * \param _endian
 * Endian that shall be used when writing data into the memory block referenced by parameter `_pMem`.
 */
{
  if ((remainingBytes != 0) && (pMem == nullptr))
    throw std::invalid_argument("MemStreamWriter::MemStreamWriter: _pMem == nullptr");
}
MemStreamWriter::MemStreamWriter(MemStreamWriter const & other) noexcept
: StreamWriterBase(other)
, pMem(other.pMem)
, remainingBytes(other.remainingBytes)
, nbOfBitsWritten(other.nbOfBitsWritten)
, bitData(other.bitData)
/**
 * \brief Copy-constructor.
 *
 * Creates a copy of an existing @ref MemStreamWriter instance.
 *
 * Note:
 * - The new @ref MemStreamWriter will write to the _same_ memory as the existing one.
 * - Remember: If single bits are written, then class @ref MemStreamWriter caches the bits until a complete
 *   byte can be written or until the stream is closed and the last byte (containing less than 8 bits and
 *   at least one zero-bit for padding) is written upon close.\n
 * - The new @ref MemStreamWriter will contains a copy of the cached bits of the existing
 *   @ref MemStreamWriter (if there are any cached bits).
 * - It is recommended to close the existing @ref MemStreamWriter after creating the new one. This
 *   prevents issues with cached bits being written when the existing @ref MemStreamWriter is closed.
 *   Otherwise such a write upon close could overwrite data being written through the new
 *   @ref MemStreamWriter and the other way round.
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * @ref MemStreamWriter instance that shall be copied.
 */
{
}
MemStreamWriter::MemStreamWriter(MemStreamWriter && other) noexcept
: StreamWriterBase(std::move(other))
, pMem(other.pMem)
, remainingBytes(other.remainingBytes)
, nbOfBitsWritten(other.nbOfBitsWritten)
, bitData(other.bitData)
/**
 * \brief Move-constructor.
 *
 * Move-constructs a new @ref MemStreamWriter instance from an existing @ref MemStreamWriter instance.
 *
 * Note:
 * - The existing @ref MemStreamWriter will be left in state [closed](@ref IStreamWriter::States::closed).
 * - The existing @ref MemStreamWriter will not write cached bits to memory when it is put into
 *   state [closed](@ref IStreamWriter::States::closed).
 * - Instead cached bits will be moved to the new @ref MemStreamWriter. The new @ref MemStreamWriter
 *   continues seamlessly at the point where the existing @ref MemStreamWriter was closed due to the
 *   move-operation.
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * The new @ref MemStreamWriter instance is move-constructed from this.\n
 * The referenced @ref MemStreamWriter will be left in state [closed](@ref IStreamWriter::States::closed).
 */
{
  other.pMem = nullptr;
  other.state = States::closed;
}
MemStreamWriter::~MemStreamWriter(void)
/**
 * \brief Destructor. Closes the stream (if not yet done) and releases the object.
 *
 * __Thread safety:__\n
 * Do not access object after invocation of destructor.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  if (state != States::closed)
    Close();
}

MemStreamWriter& MemStreamWriter::operator=(MemStreamWriter const & rhv) noexcept
/**
 * \brief Copy-assignment operator.
 *
 * Copy-assigns another @ref MemStreamWriter instance to this @ref MemStreamWriter instance.
 *
 * Note:
 * - This @ref MemStreamWriter instance will be closed before assignment, if it is not yet closed.\n
 *   This is the same as if @ref Close() would have been invoked manually.
 * - This @ref MemStreamWriter instance will write cached bits to memory upon close,
 *   if there are any cached bits. This is the same as if @ref Close() would have been invoked manually.
 * - After assignment, this @ref MemStreamWriter instance will point to the _same_ memory as the
 *   other @ref MemStreamWriter instance does.
 * - After assignment, this @ref MemStreamWriter instance will contain a copy of the cached
 *   bits from the other @ref MemStreamWriter (if there are any cached bits).
 * - Remember: If single bits are written, then class @ref MemStreamWriter caches the bits until a complete
 *   byte can be written or until the stream is closed and the last byte (containing less than 8 bits and
 *   at least one zero-bit for padding) is written upon close.\n
 * - It is recommended to close the other @ref MemStreamWriter after copy-assignment. This
 *   prevents issues with cached bits being written when the other @ref MemStreamWriter is closed.
 *   Otherwise such a write could overwrite data being written through this @ref MemStreamWriter
 *   and the other way round.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv
 * Another @ref MemStreamWriter instance whose contents shall be copy-assigned to this
 * @ref MemStreamWriter instance.\n
 * This instance is closed before the copy-assignment takes place.
 */
{
  if (&rhv != this)
  {
    Close();

    StreamWriterBase::operator=(rhv);
    pMem = rhv.pMem;
    remainingBytes = rhv.remainingBytes;
    nbOfBitsWritten = rhv.nbOfBitsWritten;
    bitData = rhv.bitData;
  }

  return *this;
}
MemStreamWriter& MemStreamWriter::operator=(MemStreamWriter&& rhv) noexcept
/**
 * \brief Move-assignment operator.
 *
 * Move-assigns another @ref MemStreamWriter instance to this @ref MemStreamWriter instance.
 *
 * Note:
 * - This @ref MemStreamWriter instance will be closed before assignment, if it is not yet closed.\n
 *   This is the same as if @ref Close() would have been invoked manually.
 * - This @ref MemStreamWriter instance will write cached bits to memory upon close,
 *   if there are any cached bits. This is the same as if @ref Close() would have been invoked manually.
 * - The other @ref MemStreamWriter will be left in state [closed](@ref IStreamWriter::States::closed).
 * - The other @ref MemStreamWriter will not write cached bits to memory when it is put into
 *   state [closed](@ref IStreamWriter::States::closed).
 * - Instead cached bits will be moved into this @ref MemStreamWriter. This @ref MemStreamWriter
 *   will continue seamlessly at the point where the other @ref MemStreamWriter has stopped due to the
 *   move-operation.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv
 * Another @ref MemStreamWriter instance whose contents shall be move-assigned to this instance.\n
 * This instance will be closed before the move-assignment takes place.\n
 * The other instance will be left in state @ref IStreamWriter::States::closed.
 */
{
  if (&rhv != this)
  {
    Close();

    StreamWriterBase::operator=(std::move(rhv));
    pMem = rhv.pMem;
    remainingBytes = rhv.remainingBytes;
    nbOfBitsWritten = rhv.nbOfBitsWritten;
    bitData = rhv.bitData;

    rhv.pMem = nullptr;
    rhv.state = States::closed;
  }

  return *this;
}

// --> IStreamWriter

bool MemStreamWriter::IsRemainingCapacitySupported(void) const
/// \copydoc IStreamWriter::IsRemainingCapacitySupported
{
  return true;
}

size_t MemStreamWriter::RemainingCapacity(void) const
/// \copydoc IStreamWriter::RemainingCapacity
{
  switch (state)
  {
    case States::open:
    case States::full:
      return remainingBytes;

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  }

  PANIC();
}
uint_fast8_t MemStreamWriter::GetNbOfCachedBits(void) const
/// \copydoc IStreamWriter::GetNbOfCachedBits
{
  switch (state)
  {
    case States::open:
    case States::full:
      return nbOfBitsWritten;

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  }

  PANIC();
}
void MemStreamWriter::Close(void) noexcept
/// \copydoc IStreamWriter::Close
{
  if (state == States::open)
  {
    // any bits left to be written?
    if (nbOfBitsWritten != 0)
    {
      // the design guarantees that one byte of capacity is left
      *pMem++ = static_cast<char>(bitData);
      remainingBytes--;
    }
  }

  pMem = nullptr;
  state = States::closed;
}
// <-- IStreamWriter

// --> StreamWriterBase
void MemStreamWriter::Push(char c)
/// \copydoc StreamWriterBase::Push(char c)
{
  // Write bits first if some bits are not yet written so that the byte based data will
  // be aligned to a byte boundary.
  if (nbOfBitsWritten != 0)
  {
    // move the bits to be written into d
    char const d = static_cast<char>(bitData);

    // clear bit buffer now and not after writing the bits, because we are going to call this method recursive now!
    nbOfBitsWritten = 0;
    bitData = 0;

    // write bits
    Push(d);
  }

  switch (state)
  {
    case States::open:
    {
      // write byte
      *pMem++ = c;
      remainingBytes--;

      // full?
      if (remainingBytes == 0)
      {
        pMem = nullptr;
        state = States::full;
      }
      break;
    }

    case States::full:
    {
      state = States::error;
      throw FullError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}
void MemStreamWriter::Push(void const * pData, size_t n)
/// \copydoc StreamWriterBase::Push(void const * pData, size_t n)
{
  if (n == 0)
    return;

  // Write bits first if some bits are not yet written so that the byte based data will
  // be aligned to a byte boundary.
  if (nbOfBitsWritten != 0)
  {
    // move the bits to be written into d
    char const d = static_cast<char>(bitData);

    // clear bit buffer now and not after writing the bits, because we are going to call this method recursive now!
    nbOfBitsWritten = 0;
    bitData = 0;

    // write bits
    Push(d);
  }

  switch (state)
  {
    case States::open:
    {
      // does "n" exceed the remaining capacity?
      if (n > remainingBytes)
      {
        pMem = nullptr;
        state = States::error;
        throw FullError();
      }

      // write bytes
      memcpy(pMem, pData, n);
      pMem += n;
      remainingBytes -= n;

      // full?
      if (remainingBytes == 0)
      {
        pMem = nullptr;
        state = States::full;
      }

      break;
    }

    case States::full:
    {
      state = States::error;
      throw FullError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}
void MemStreamWriter::PushBits(uint8_t bits, uint_fast8_t n)
/// \copydoc StreamWriterBase::PushBits
{
  if (n == 0)
    return;

  if (n > 8)
    throw std::invalid_argument("MemStreamWriter::PushBits: n must be [0..8].");

  switch (state)
  {
    case States::open:
    {
      // clear upper bits that shall be ignored
      bits &= (1U << n) - 1U;

      // combine potential previously written bits with the bits that shall be written
      uint_fast16_t data = static_cast<uint_fast16_t>(bitData) | (static_cast<uint_fast16_t>(bits) << nbOfBitsWritten);
      nbOfBitsWritten += n;

      // one byte filled up with bits?
      if (nbOfBitsWritten >= 8)
      {
        // write byte into the stream
        *pMem++ = static_cast<char>(data);
        remainingBytes--;

        nbOfBitsWritten -= 8;
        data >>= 8U;

        // buffer full?
        if (remainingBytes == 0)
        {
          pMem = nullptr;

          // more bits to be written?
          if (nbOfBitsWritten != 0)
          {
            // wrote beyond end of stream
            state = States::error;
            throw FullError();
          }
          else
          {
            // buffer full
            state = States::full;
          }
        }
      }

      // store temporary stuff back in bitData
      bitData = static_cast<uint8_t>(data);

      break;
    }

    case States::full:
    {
      // (attempt to write to a full buffer)
      state = States::error;
      throw FullError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}
// <-- StreamWriterBase

} // namespace stream
} // namespace gpcc
