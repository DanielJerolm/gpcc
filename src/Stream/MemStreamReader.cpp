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

#include "MemStreamReader.hpp"
#include "StreamErrors.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include <cstring>


namespace gpcc
{
namespace Stream
{

MemStreamReader::MemStreamReader(void const * const _pMem, size_t const _size, Endian const _endian)
: StreamReaderBase(States::open, _endian)
, pMem(static_cast<char const*>(_pMem))
, remainingBytes(_size)
, nbOfBitsInBitData(0)
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
 * Pointer to the beginning of the block of memory from which the new created MemStreamReader shall read.\n
 * nullptr is allowed if parameter `_size` is zero.
 * \param _size
 * Size of the block of memory referenced by parameter `_pMem` in bytes.
 * If this is zero, then the initial state of the stream will be @ref States::empty.
 * \param _endian
 * Endian that shall be used when reading data from the memory block referenced by parameter `_pMem`.
 */
{
  if (remainingBytes == 0U)
  {
    pMem = nullptr;
    state = States::empty;
  }
  else if (pMem == nullptr)
    throw std::invalid_argument("MemStreamReader::MemStreamReader: _pMem == nullptr");
}
MemStreamReader::MemStreamReader(MemStreamReader const & other) noexcept
: StreamReaderBase(other)
, pMem(other.pMem)
, remainingBytes(other.remainingBytes)
, nbOfBitsInBitData(other.nbOfBitsInBitData)
, bitData(other.bitData)
/**
 * \brief Copy-constructor.
 *
 * Creates a copy of an existing @ref MemStreamReader instance.
 *
 * Note:
 * - The new @ref MemStreamReader will read from the _same_ memory as the existing one.
 * - The new @ref MemStreamReader will receive a copy of the cached bits inside the
 *   existing @ref MemStreamReader, if there are any cached bits.
 * - The new @ref MemStreamReader will start reading at the same byte- and bit-position
 *   that will also be be read next by the existing @ref MemStreamReader.
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
 * @ref MemStreamReader instance that shall be copied.
 */
{
}
MemStreamReader::MemStreamReader(MemStreamReader&& other) noexcept
: StreamReaderBase(std::move(other))
, pMem(other.pMem)
, remainingBytes(other.remainingBytes)
, nbOfBitsInBitData(other.nbOfBitsInBitData)
, bitData(other.bitData)
/**
 * \brief Move-constructor.
 *
 * Move-constructs a new @ref MemStreamReader instance from an existing @ref MemStreamReader instance.
 *
 * Note:
 * - The existing @ref MemStreamReader will be left in state [closed](@ref IStreamReader::States::closed).
 * - The new @ref MemStreamReader will read from the _same_ memory as the existing one.
 * - The new @ref MemStreamReader will receive cached bits from the existing reader,
 *   if there are any cached bits.
 * - The new @ref MemStreamReader will start reading at the same byte- and bit-position
 *   that would also have been read next by the existing @ref MemStreamReader.
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
 * The new @ref MemStreamReader instance is move-constructed from this.\n
 * The referenced @ref MemStreamReader will be left in state @ref IStreamReader::States::closed.
 */
{
  other.pMem = nullptr;
  other.state = States::closed;
}

MemStreamReader& MemStreamReader::operator=(MemStreamReader const & rhv) noexcept
/**
 * \brief Copy-assignment operator.
 *
 * Copy-assigns another @ref MemStreamReader instance to this @ref MemStreamReader instance.
 *
 * Note:
 * - This @ref MemStreamReader instance will be closed before assignment, if it is not yet closed.\n
 *   This is the same as if @ref Close() would have been invoked manually.
 * - After assignment, this @ref MemStreamReader instance will point to the _same_ memory as the
 *   other @ref MemStreamReader instance does.
 * - After assignment, this @ref MemStreamReader instance will contain a copy of the cached
 *   bits of the other @ref MemStreamReader (if there are any cached bits).
 * - After assignment, this @ref MemStreamReader will start reading from the same byte- and bit-
 *   position that will also be read next by the other @ref MemStreamReader instance.
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
 * @ref MemStreamReader instance whose contents shall be copy-assigned to this @ref MemStreamReader
 * instance.\n
 * This instance is closed before the copy-assignment takes place.
 */
{
  if (&rhv != this)
  {
    Close();

    StreamReaderBase::operator=(rhv);

    pMem = rhv.pMem;
    remainingBytes = rhv.remainingBytes;
    nbOfBitsInBitData = rhv.nbOfBitsInBitData;
    bitData = rhv.bitData;
  }

  return *this;
}
MemStreamReader& MemStreamReader::operator=(MemStreamReader&& rhv) noexcept
/**
 * \brief Move-assignment operator.
 *
 * Move-assigns another @ref MemStreamReader instance to this @ref MemStreamReader instance.
 *
 * Note:
 * - This @ref MemStreamReader instance will be closed before assignment, if it is not yet closed.\n
 *   This is the same as if @ref Close() would have been invoked manually.
 * - The other @ref MemStreamReader will be left in state [closed](@ref IStreamReader::States::closed).
 * - After assignment, this @ref MemStreamReader instance will point to the _same_ memory as the
 *   other @ref MemStreamReader instance did before the assignment.
 * - During assignment, bits cached in the other @ref MemStreamReader instance will be moved into this
 *   @ref MemStreamReader instance (if there are any cached bits).
 * - After assignment, this @ref MemStreamReader will continue reading seamlessly at the same byte- and
 *   bit-position that would have been read next by the other @ref MemStreamReader instance.
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
 * @ref MemStreamReader instance whose contents shall be move-assigned to this @ref MemStreamReader
 * instance.\n
 * This instance is closed before the move-assignment takes place.\n
 * The @ref MemStreamReader instance referenced by `rhv` will be left in state @ref IStreamReader::States::closed.
 */
{
  if (&rhv != this)
  {
    Close();

    StreamReaderBase::operator=(std::move(rhv));

    pMem = rhv.pMem;
    remainingBytes = rhv.remainingBytes;
    nbOfBitsInBitData = rhv.nbOfBitsInBitData;
    bitData = rhv.bitData;

    rhv.pMem = nullptr;
    rhv.state = States::closed;
  }

  return *this;
}

/**
 * \brief Skips one or more bytes and returns a @ref MemStreamReader object for reading the skipped bytes.
 *
 * If there are any bits of the current read byte that have not yet been read, then the bits will be discarded
 * and the read pointer will be moved to the next byte boundary before the new @ref MemStreamReader object is
 * created.
 *
 * \htmlonly <style>div.image img[src="stream/MemStreamReader_SubStream.png"]{width:60%;}</style> \endhtmlonly
 * \image html "stream/MemStreamReader_SubStream.png" "SubStream(): Before, after, and new stream"
 *
 * \pre   The stream must be in [States::open](@ref gpcc::Stream::IStreamReader::States::open) or
 *        [States::empty](@ref gpcc::Stream::IStreamReader::States::empty).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param n
 * Number of bytes to be skipped.\n
 * Zero is allowed.\n
 * Note that any bits that have not yet been read will be discarded, even if this is zero.
 *
 * \return
 * A @ref MemStreamReader object for reading the skipped bytes.
 */
MemStreamReader MemStreamReader::SubStream(size_t const n)
{
  switch (state)
  {
    case States::open:
    {
      if (n > remainingBytes)
        throw EmptyError();

      MemStreamReader subBlock(pMem, n, endian);

      // discard any bits from the last read byte that have not yet been read
      if (nbOfBitsInBitData != 0U)
      {
        nbOfBitsInBitData = 0;
        bitData = 0;
      }

      remainingBytes -= n;

      if (remainingBytes == 0U)
      {
        // (empty now)
        pMem = nullptr;
        state = States::empty;
      }
      else
      {
        // (move read pointer forward)
        pMem += n;
      }

      return subBlock;
    }

    case States::empty:
    {
      if (n != 0U)
        throw EmptyError();

      return MemStreamReader(nullptr, 0, endian);
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)

  PANIC();
}

/**
 * \brief Reduces the remaining number of bytes left to be read.
 *
 * \htmlonly <style>div.image img[src="stream/MemStreamReader_Shrink.png"]{width:60%;}</style> \endhtmlonly
 * \image html "stream/MemStreamReader_Shrink.png" "Shrink(): Before and after"
 *
 * \pre   The stream must be in [States::open](@ref gpcc::Stream::IStreamReader::States::open) or
 *        [States::empty](@ref gpcc::Stream::IStreamReader::States::empty).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newRemainingBytes
 * New value for the amount of bytes remaining to be read.\n
 * This must be equal to or less than the return value of @ref RemainingBytes(). \n
 * This method has no effect on any bits of the currently read byte that have not yet been read.
 */
void MemStreamReader::Shrink(size_t const newRemainingBytes)
{
  switch (state)
  {
    case States::open:
    {
      if (newRemainingBytes > remainingBytes)
        throw std::invalid_argument("MemStreamReader::Shrink: Attempt to enlarge remaining number of bytes");

      remainingBytes = newRemainingBytes;
      if (remainingBytes == 0U)
      {
        pMem = nullptr;

        if (nbOfBitsInBitData == 0U)
          state = States::empty;
      }

      break;
    }

    case States::empty:
    {
      if (newRemainingBytes != 0U)
        throw std::invalid_argument("MemStreamReader::Shrink: 'newRemainingBytes' must be zero in state 'empty'");

      break;
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}

/**
 * \brief Retrieves the read-pointer of the @ref MemStreamReader.
 *
 * This is intended to be invoked only by the owner of the memory read by the @ref MemStreamReader.
 *
 * The concept of the @ref IStreamReader interface and class @ref MemStreamReader does not offer access to the
 * MemStreamReader's read-pointer by intention. If the read-pointer of a @ref MemStreamReader would be accessible in
 * principle, then class @ref MemStreamReader could not encapsulate the underlying memory and it could not guard,
 * control and restrict access to the underlying memory.
 *
 * However, there are some rare use cases where access to the read-pointer may be advantageous. In these cases, __the__
 * __owner of the memory__ read by the @ref MemStreamReader (__who is also the creator__ of the @ref MemStreamReader
 * instance) may use this method to retrieve the read-pointer.
 *
 * This method is intended to be used only by the owner of the memory read by the @ref MemStreamReader. It shall not be
 * used by others who may have received a @ref MemStreamReader instance from the owner of the memory. Usage is
 * restricted by checking parameters `_pMem` and `_size` which should be known by the owner of the underlying memory
 * only.
 *
 * If any user of a @ref MemStreamReader instance, _who is not the owner of the underlying memory_ needs the
 * read-pointer, then he/she _has to ask the owner of the memory_ to provide the read-pointer.
 *
 * The decision if the owner of the memory provides a method to retrieve a read-pointer or if such a method is not
 * provided is solely left to the designer of the owner of the memory.
 *
 * If the owner of the memory offers a Getter()-method for the read-pointer, then the Getter()-method shall receive a
 * unmodifiable reference to the @ref MemStreamReader as function parameter. The getter shall then use this method to
 * determine the read-pointer of the referenced @ref MemStreamReader.
 *
 * __Limitations__:\n
 * This method works in conjunction with @ref MemStreamReader instances created by the owner of the memory or copies of
 * such @ref MemStreamReader instances. Of course any number of bytes may have been read from the @ref MemStreamReader
 * as long as at least one byte of data is left to be read.
 *
 * This method will reject retrieval of the read-pointer in conjunction with any @ref MemStreamReader instances created
 * via @ref SubStream() and in conjunction with any @ref MemStreamReader instances which have been shrinked via
 * @ref Shrink(). The only exception is: The substream or the shrinked stream is identical to the original stream
 * (e.g. shrink by zero).
 *
 * \warning
 * If the read-pointer shall be accessible or not requires a careful decision. Offering a read-pointer inherently
 * weakens the encapsulation of the memory and the guardening and control offered by class @ref MemStreamReader.
 * __The best practice is to avoid offering the read-pointer at all.__
 *
 * \pre   The @ref MemStreamReader is in state @ref States::open
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::logic_error   `_pMem` and `_size` are not plausible. `_pMem` and `_size` should be known by the owner
 *                            of the underlying memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _pMem
 * Pointer to the beginning of the memory read by this @ref MemStreamReader instance.\n
 * This will throw if the provided pointer is not plausible.
 *
 * \param _size
 * Size of the memory read by this @ref MemStreamReader instance.\n
 * This will throw if the provided value is not plausible.
 *
 * \return
 * Current read-pointer of the @ref MemStreamReader. \n
 * The read-pointer points to the next byte that will be read from the underlying memory. Keep in mind that if single
 * bits are read, then one byte will be read from the underlying memory (the read-pointer is advanced!) and any bits
 * not yet read are stored in a special intermediate storage location. Further bits will be read from the special
 * intermediate storage location until all bits are read and a new byte is read.\n
 * For details, please refer to chapter "Reading bit-based data" in the details of @ref IStreamReader.
 */
void const * MemStreamReader::GetReadPtr(void const * const _pMem, size_t const _size) const
{
  if (state != States::open)
    throw std::logic_error("MemStreamReader::GetReadPtr: State is not 'open'");

  // No more bytes left?
  // Note: there may be up to 7 bits left to be read before the stream is empty.
  if (remainingBytes == 0U)
    throw std::logic_error("MemStreamReader::GetReadPtr: No more bytes to be read.");

  // Check authorization by comparing the end of this MemStreamReader and the end of the memory described by
  // _pMem and _size. Only the owner of the memory should know proper values for _pMem and _size.
  uintptr_t const thisEnd = reinterpret_cast<uintptr_t>(pMem) + remainingBytes;
  uintptr_t const memEnd  = reinterpret_cast<uintptr_t>(_pMem) + _size;

  if (thisEnd != memEnd)
    throw std::logic_error("MemStreamReader::GetReadPtr: _pMem and _size are not plausible.");

  return pMem;
}

// --> IStreamReader
bool MemStreamReader::IsRemainingBytesSupported(void) const
/// \copydoc IStreamReader::IsRemainingBytesSupported
{
  return true;
}

size_t MemStreamReader::RemainingBytes(void) const
/// \copydoc IStreamReader::RemainingBytes
{
  switch (state)
  {
    case States::open:
    case States::empty:
      return remainingBytes;

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  }

  PANIC();
}

/// \copydoc IStreamReader::EnsureAllDataConsumed
void MemStreamReader::EnsureAllDataConsumed(RemainingNbOfBits const expectation) const
{
  switch (state)
  {
    case States::open:
    case States::empty:
      {
        switch (expectation)
        {
          case RemainingNbOfBits::sevenOrLess:
          {
            if (remainingBytes != 0U)
              throw RemainingBitsError();
            break;
          }

          case RemainingNbOfBits::moreThanSeven:
          {
            if (remainingBytes == 0U)
              throw RemainingBitsError();
            break;
          }

          case RemainingNbOfBits::any:
          {
            break;
          }

          default:
          {
            // (0..7)

            if ((remainingBytes != 0U) || (nbOfBitsInBitData != static_cast<uint8_t>(expectation)))
              throw RemainingBitsError();
            break;
          }
        } // switch (expectation)

        break;
      } // case States::open / States::empty

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}

void MemStreamReader::Close(void) noexcept
/// \copydoc IStreamReader::Close(void)
{
  pMem = nullptr;
  state = States::closed;
}
void MemStreamReader::Skip(size_t nBits)
/// \copydoc IStreamReader::Skip
{
  if (nBits == 0U)
    return;

  switch (state)
  {
    case States::open:
    {
      // are there any bits that have not been read yet? -> skip them first
      if (nbOfBitsInBitData != 0U)
      {
        // will all bits be skipped or will at least one bit be left?
        if (nBits < nbOfBitsInBitData)
        {
          // (at least one bit will be left to be read after skip)

          bitData >>= nBits;
          nbOfBitsInBitData -= nBits;

          // Finished. The desired number of bits has been skipped.
          return;
        }
        else
        {
          // (all bits are skipped)

          nBits -= nbOfBitsInBitData;
          bitData = 0;
          nbOfBitsInBitData = 0;

          // stream empty now?
          if (pMem == nullptr)
            state = States::empty;

          // finished?
          if (nBits == 0U)
            return;
        }
      }

      // at this point program logic guarantees, that "nbOfBitsInBitData" is zero and "nBits" is not zero

      // no more bytes left?
      if (pMem == nullptr)
      {
        state = States::error;
        throw EmptyError();
      }

      // calculate the number of bytes and bits to be skipped
      size_t       const skip_bytes = nBits / 8U;
      uint_fast8_t const skip_bits  = nBits % 8U;

      // skip bytes
      if (skip_bytes > remainingBytes)
      {
        pMem = nullptr;
        state = States::error;
        throw EmptyError();
      }
      else if (skip_bytes == remainingBytes)
      {
        pMem = nullptr;
        remainingBytes = 0;
        state = States::empty;
      }
      else
      {
        pMem += skip_bytes;
        remainingBytes -= skip_bytes;
      }

      // skip bits
      if (skip_bits != 0U)
      {
        // no more bytes left?
        if (pMem == nullptr)
        {
          state = States::error;
          throw EmptyError();
        }

        bitData = static_cast<uint8_t>(*pMem) >> skip_bits;
        nbOfBitsInBitData = 8U - skip_bits;

        remainingBytes--;
        if (remainingBytes == 0U)
          pMem = nullptr;
        else
          pMem++;
      }
      break;
    } // case States::open

    case States::empty:
    {
      state = States::error;
      throw EmptyError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}
std::string MemStreamReader::Read_string(void)
/// \copydoc IStreamReader::Read_string(void)
{
  // discard any bits from the last read byte that have not yet been read
  if (nbOfBitsInBitData != 0)
  {
    nbOfBitsInBitData = 0;
    bitData = 0;
  }

  switch (state)
  {
    case States::open:
    {
      ON_SCOPE_EXIT()
      {
        pMem = nullptr;
        state = States::error;
      };

      // no more bytes left?
      if (pMem == nullptr)
        throw EmptyError();

      // look for null-terminator
      size_t n = remainingBytes;
      char const * p = pMem;
      do
      {
        if (n == 0)
          throw std::runtime_error("MemStreamReader::Read_string: No null-terminator located");
        n--;
      }
      while (*p++ != 0x00);
      n = p - pMem;

      // Read n bytes into an std::string instance. null-terminator is dropped.
      std::string str(pMem, n - 1U);
      pMem += n;
      remainingBytes -= n;

      // empty now?
      if (remainingBytes == 0)
      {
        pMem = nullptr;
        state = States::empty;
      }

      ON_SCOPE_EXIT_DISMISS();
      return str;
    }

    case States::empty:
    {
      state = States::error;
      throw EmptyError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)

  PANIC();
}
std::string MemStreamReader::Read_line(void)
/// \copydoc IStreamReader::Read_line(void)
{
  // discard any bits from the last read byte that have not yet been read
  if (nbOfBitsInBitData != 0)
  {
    nbOfBitsInBitData = 0;
    bitData = 0;
  }

  switch (state)
  {
    case States::open:
    {
      ON_SCOPE_EXIT()
      {
        pMem = nullptr;
        state = States::error;
      };

      // no more bytes left?
      if (pMem == nullptr)
        throw EmptyError();

      // Locate the byte (p) beyond the last character of the line.
      // Note: remainingBytes is not zero
      size_t n = remainingBytes;
      char const * p = pMem;
      char c = *p;
      while ((c != '\n') && (c != '\r') && (c != 0x00))
      {
        --n;
        ++p;

        if (n == 0U)
          break;

        c = *p;
      }

      // Calculate number of characters the line is comprised of. This is excl. NUL, \n, or \r.
      n = p - pMem;

      // read n bytes into an std::string instance
      std::string str(pMem, n);
      pMem += n;
      remainingBytes -= n;

      // consume NUL, '\r', '\n', or '\r\n'
      if (remainingBytes != 0U)
      {
        if (c == '\r')
        {
          // its a '\r' or '\r\n'
          ++pMem;
          --remainingBytes;

          if ((remainingBytes != 0U) && (*pMem == '\n'))
          {
            // it was an '\r\n'
            ++pMem;
            --remainingBytes;
          }
        }
        else
        {
          // its a NUL or '\n'
          ++pMem;
          --remainingBytes;
        }
      }

      // empty now?
      if (remainingBytes == 0)
      {
        pMem = nullptr;
        state = States::empty;
      }

      ON_SCOPE_EXIT_DISMISS();
      return str;
    }

    case States::empty:
    {
      state = States::error;
      throw EmptyError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)

  PANIC();
}
// <-- IStreamReader

// --> StreamReaderBase
unsigned char MemStreamReader::Pop(void)
/// \copydoc StreamReaderBase::Pop(void)
{
  // discard any bits from the last read byte that have not yet been read
  if (nbOfBitsInBitData != 0)
  {
    nbOfBitsInBitData = 0;
    bitData = 0;
  }

  switch (state)
  {
    case States::open:
    {
      // no more bytes left?
      if (pMem == nullptr)
      {
        state = States::error;
        throw EmptyError();
      }

      // read one byte
      unsigned char const c = static_cast<unsigned char>(*pMem++);
      remainingBytes--;

      // empty now?
      if (remainingBytes == 0)
      {
        pMem = nullptr;
        state = States::empty;
      }

      return c;
    }

    case States::empty:
    {
      state = States::error;
      throw EmptyError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)

  PANIC();
}
void MemStreamReader::Pop(void* p, size_t n)
/// \copydoc StreamReaderBase::Pop(void* p, size_t n)
{
  if (n == 0)
    return;

  // discard any bits from the last read byte that have not yet been read
  if (nbOfBitsInBitData != 0)
  {
    nbOfBitsInBitData = 0;
    bitData = 0;
  }

  switch (state)
  {
    case States::open:
    {
      // does "n" exceed the remaining number of bytes?
      if (n > remainingBytes)
      {
        pMem = nullptr;
        state = States::error;
        throw EmptyError();
      }

      // read data
      memcpy(p, pMem, n);
      pMem += n;
      remainingBytes -= n;

      // empty now?
      if (remainingBytes == 0)
      {
        pMem = nullptr;
        state = States::empty;
      }

      break;
    }

    case States::empty:
    {
      state = States::error;
      throw EmptyError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)
}
uint8_t MemStreamReader::PopBits(uint_fast8_t n)
/// \copydoc StreamReaderBase::PopBits(uint_fast8_t n)
{
  if (n == 0)
    return 0;

  if (n > 8)
    throw std::invalid_argument("MemStreamReader::PopBits: n must be [0..8].");

  switch (state)
  {
    case States::open:
    {
      // fetch next 8 bits required?
      if (n > nbOfBitsInBitData)
      {
        // no more bytes left?
        if (pMem == nullptr)
        {
          state = States::error;
          throw EmptyError();
        }

        // read one byte
        unsigned char const c = static_cast<unsigned char>(*pMem++);
        remainingBytes--;

        // no more bytes left?
        if (remainingBytes == 0)
          pMem = nullptr;

        // append read byte to bitData
        bitData |= static_cast<uint16_t>(c) << nbOfBitsInBitData;
        nbOfBitsInBitData += 8;
      }

      // read bits
      uint8_t const bits = bitData & ((1U << n) - 1U);
      bitData >>= n;
      nbOfBitsInBitData -= n;

      // all bits read and stream empty?
      if ((nbOfBitsInBitData == 0) && (pMem == nullptr))
        state = States::empty;

      return bits;
    }

    case States::empty:
    {
      state = States::error;
      throw EmptyError();
    }

    case States::closed:
      throw ClosedError();

    case States::error:
      throw ErrorStateError();
  } // switch (state)

  PANIC();
}
// <-- StreamReaderBase

} // namespace Stream
} // namespace gpcc
