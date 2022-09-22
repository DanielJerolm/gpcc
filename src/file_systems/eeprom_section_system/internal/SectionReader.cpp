/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "SectionReader.hpp"
#include <gpcc/file_systems/eeprom_section_system/EEPROMSectionSystem.hpp>
#include <gpcc/file_systems/eeprom_section_system/exceptions.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/stream/stream_errors.hpp>
#include "EEPROMSectionSystemInternals.hpp"
#include <exception>
#include <stdexcept>
#include <cstring>

namespace gpcc
{
namespace file_systems
{
namespace eeprom_section_system
{
namespace internal
{

SectionReader::SectionReader(EEPROMSectionSystem & _ESS,
                             std::string const & _sectionName,
                             std::unique_ptr<unsigned char[]> _spMem)
: gpcc::stream::StreamReaderBase(States::open, Endian::Little)
, pESS(&_ESS)
, sectionName(_sectionName)
, spMem(std::move(_spMem))
, rdPtr(spMem.get() + sizeof(DataBlock_t))
, nbOfBitsInBitData(0)
, bitData(0)
, remainingBytesInCurrentBlock(0)
/**
 * \brief Constructor.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.\n
 * The `mutex` of the @ref EEPROMSectionSystem referenced by `_ESS` must be locked.
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
 * \param _ESS
 * Reference to the @ref EEPROMSectionSystem instance containing the section that shall be read.
 * \param _sectionName
 * Name of the section that shall be read.\n
 * The section must be locked at the @ref EEPROMSectionSystem for reading. It will be unlocked by this
 * when the stream is finally closed.
 * \param _spMem
 * A buffer for the new @ref SectionReader instance, allocated by the caller.\n
 * The buffer must have a capacity of at least the block size of the underlying storage.\n
 * The buffer must contain the storage block containing the section head of the section that shall be read.
 */
{
  CommonBlockHead_t const * const pHead = static_cast<CommonBlockHead_t const *>(static_cast<void const*>(spMem.get()));

  if (static_cast<BlockTypes>(pHead->type) != BlockTypes::sectionHead)
    throw std::invalid_argument("SectionReader::SectionReader: Bad block type");

  // load first data block
  if (pESS->LoadNextBlockOfSection(spMem.get()) != NOBLOCK)
    remainingBytesInCurrentBlock = pHead->nBytes - (sizeof(DataBlock_t) + sizeof(uint16_t));

  // special case: empty section?
  if (remainingBytesInCurrentBlock == 0)
  {
    ReleaseBuffer();
    state = States::empty;
  }
}
SectionReader::SectionReader(SectionReader && other) noexcept
: gpcc::stream::StreamReaderBase(std::move(other))
, pESS(other.pESS)
, sectionName(std::move(other.sectionName))
, spMem(std::move(other.spMem))
, rdPtr(other.rdPtr)
, nbOfBitsInBitData(other.nbOfBitsInBitData)
, bitData(other.bitData)
, remainingBytesInCurrentBlock(other.remainingBytesInCurrentBlock)
/**
 * \brief Move constructor.
 *
 * __Thread safety:__\n
 * Do not access object before constructor has finished.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param other
 * The other @ref SectionReader instance that shall be moved into the new created one.\n
 * The other instance is left in state @ref States::closed.
 */
{
  other.ReleaseBuffer();
  other.pESS  = nullptr;
  other.state = States::closed;
}
SectionReader::~SectionReader(void)
/**
 * \brief Destructor. Closes the section (if not yet done) and releases the object.
 *
 * _Any stream should be closed via @ref Close() before it is released._\n
 * If it is not closed yet, then it will be closed now by this destructor.\n
 * If the close-operation fails or if thread cancellation occurs, then the application
 * will terminate via @ref gpcc::osal::Panic().
 *
 * ---
 *
 * __Thread safety:__\n
 * Do not access object after invocation of destructor.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations may only fail due to serious errors that will result in program termination via Panic(...).\n
 * To prevent any error, ensure that the stream is closed __before__ it is released.
 *
 * __Thread cancellation safety:__\n
 * Cancellation not allowed:\n
 * Cancellation will result in @ref gpcc::osal::Panic().
 */
{
  try
  {
    if (state != States::closed)
      Close();
  }
  catch (...)
  {
    PANIC();
  }
}

bool SectionReader::IsRemainingBytesSupported(void) const
/// \copydoc gpcc::stream::IStreamReader::IsRemainingBytesSupported
{
  return false;
}

size_t SectionReader::RemainingBytes(void) const
/// \copydoc gpcc::stream::IStreamReader::RemainingBytes
{
  switch (state)
  {
    case States::open:
      // intentional fall-through
    case States::empty:
      throw std::logic_error("SectionReader::RemainingBytes: Operation not supported");

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  }

  PANIC();
}

/// \copydoc gpcc::stream::IStreamReader::EnsureAllDataConsumed
void SectionReader::EnsureAllDataConsumed(RemainingNbOfBits const expectation) const
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
            if (rdPtr != nullptr)
              throw stream::RemainingBitsError();
            break;
          }

          case RemainingNbOfBits::moreThanSeven:
          {
            if (rdPtr == nullptr)
              throw stream::RemainingBitsError();
            break;
          }

          case RemainingNbOfBits::any:
          {
            break;
          }

          default:
          {
            // (0..7)

            if ((rdPtr != nullptr) || (nbOfBitsInBitData != static_cast<uint8_t>(expectation)))
              throw stream::RemainingBitsError();
            break;
          }
        } // switch (expectation)

        break;
      } // case States::open / States::empty

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)
}

void SectionReader::Close(void)
/// \copydoc gpcc::stream::IStreamReader::Close
{
  if (state != States::closed)
  {
    ReleaseBuffer();

    try
    {
      osal::MutexLocker mutexLocker(pESS->mutex);
      pESS->sectionLockManager.ReleaseReadLock(sectionName);
    }
    catch (...)
    {
      PANIC();
    }

    sectionName.clear();
    pESS = nullptr;
    state = States::closed;
  }
}

void SectionReader::Skip(size_t nBits)
/// \copydoc gpcc::stream::IStreamReader::Skip
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
          if (rdPtr == nullptr)
            state = States::empty;

          // finished?
          if (nBits == 0U)
            return;
        }
      }

      // at this point program logic guarantees, that "nbOfBitsInBitData" is zero and "nBits" is not zero

      // no more bytes left?
      if (rdPtr == nullptr)
      {
        state = States::error;
        throw stream::EmptyError();
      }

      // calculate the number of bytes and bits to be skipped
      size_t             skip_bytes = nBits / 8U;
      uint_fast8_t const skip_bits  = nBits % 8U;

      // skip bytes
      while (skip_bytes != 0U)
      {
        // Bytes are skipped in chunks because a next block must be loaded from the
        // storage device from time to time...
        uint_fast16_t chunkSize;
        if (skip_bytes > remainingBytesInCurrentBlock)
          chunkSize = remainingBytesInCurrentBlock;
        else
          chunkSize = skip_bytes;

        // skip a chunk of bytes
        rdPtr += chunkSize;
        remainingBytesInCurrentBlock -= chunkSize;
        skip_bytes -= chunkSize;

        // time to load the next block?
        if (remainingBytesInCurrentBlock == 0U)
        {
          LoadNextBlock();
          if (rdPtr == nullptr)
          {
            if ((skip_bytes == 0U) && (skip_bits == 0U))
            {
              state = States::empty;
            }
            else
            {
              state = States::error;
              throw stream::EmptyError();
            }
          }
        }
      } // while (skip_bytes != 0)

      // skip bits
      if (skip_bits != 0U)
      {
        // program logic guarantees, that:
        // - rdPtr is not nullptr
        // - nbOfBitsInBitData is zero

        // read one byte and skip bits
        bitData = (*rdPtr++) >> skip_bits;
        nbOfBitsInBitData = 8U - skip_bits;
        remainingBytesInCurrentBlock--;

        // time to load the next block?
        if (remainingBytesInCurrentBlock == 0U)
          LoadNextBlock();
      }

      break;
    }

    case States::empty:
      state = States::error;
      throw stream::EmptyError();

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)
}

std::string SectionReader::Read_string(void)
/// \copydoc gpcc::stream::IStreamReader::Read_string(void)
{
  std::string s;

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
      bool done = false;
      do
      {
        if (rdPtr == nullptr)
        {
          state = States::error;
          throw stream::EmptyError();
        }

        size_t const n = strnlen(static_cast<char const*>(static_cast<void const*>(rdPtr)), remainingBytesInCurrentBlock);
        if (n == 0)
        {
          // remove NULL-terminator
          rdPtr++;
          remainingBytesInCurrentBlock--;
          done = true;
        }
        else
        {
          try
          {
            s.append(static_cast<char const*>(static_cast<void const*>(rdPtr)), n);
          }
          catch (...)
          {
            state = States::error;
            throw;
          }
          rdPtr += n;
          remainingBytesInCurrentBlock -= n;

          if (remainingBytesInCurrentBlock != 0)
          {
            // remove NULL-terminator
            rdPtr++;
            remainingBytesInCurrentBlock--;
            done = true;
          }
        }

        // time to load the next block?
        if (remainingBytesInCurrentBlock == 0)
        {
          LoadNextBlock();
          if (rdPtr == nullptr)
            state = States::empty;
        }
      }
      while (!done);
    }
    break;

    case States::empty:
      state = States::error;
      throw stream::EmptyError();

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)

  return s;
}
std::string SectionReader::Read_line(void)
/// \copydoc gpcc::stream::IStreamReader::Read_line(void)
{
  std::string s;

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
      if (rdPtr == nullptr)
      {
        state = States::error;
        throw stream::EmptyError();
      }

      // Lambda: Loads next block, if current block has been consumed. Returns false, if there is no more data.
      // Exception safety: Basic, switches 'state' to States::error and releases buffer
      auto LoadNextBlockIfRequired = [&]() -> bool
      {
        if (remainingBytesInCurrentBlock == 0U)
        {
          LoadNextBlock();
          if (rdPtr == nullptr)
          {
            state = States::empty;
            return false;
          }
        }

        return true;
      };

      while (true)
      {
        // Locate the byte (p) beyond the last character of the line or beyond the end of the current block.
        // Note: remainingBytesInCurrentBlock is not zero
        size_t n = remainingBytesInCurrentBlock;
        char const * p = static_cast<char const*>(static_cast<void const*>(rdPtr));
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
        // Note that there may be another block where the search for the line end must continue.
        n = p - static_cast<char const*>(static_cast<void const*>(rdPtr));

        // read n bytes into an std::string instance
        if (n != 0U)
        {
          try
          {
            s.append(static_cast<char const*>(static_cast<void const*>(rdPtr)), n);
          }
          catch (...)
          {
            state = States::error;
            throw;
          }

          rdPtr += n;
          remainingBytesInCurrentBlock -= n;

          if (!LoadNextBlockIfRequired())
            break;
        }

        // consume '\n' or NUL
        if ((c == '\n') || (c == 0x00))
        {
          ++rdPtr;
          --remainingBytesInCurrentBlock;

          LoadNextBlockIfRequired();
          break;
        }
        // consume '\r' or '\r\n'
        else if (c == '\r')
        {
          ++rdPtr;
          --remainingBytesInCurrentBlock;

          if (!LoadNextBlockIfRequired())
            break;

          if ((remainingBytesInCurrentBlock != 0U) && (*rdPtr == '\n'))
          {
            // it was an '\r\n'
            ++rdPtr;
            --remainingBytesInCurrentBlock;
            LoadNextBlockIfRequired();
          }

          break;
        }
      } // while (true)
    }
    break;

    case States::empty:
      state = States::error;
      throw stream::EmptyError();

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)

  return s;
}

unsigned char SectionReader::Pop(void)
/// \copydoc gpcc::stream::StreamReaderBase::Pop(void)
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
      // less than 8 bits left?
      if (rdPtr == nullptr)
      {
        // Attempt to read from empty stream.
        // (However, there was at least one bit left to be read, but we need a whole byte.
        // ReleaseBuffer() had already been invoked before.)

        state = States::error;
        throw stream::EmptyError();
      }

      // read one byte
      unsigned char const readByte = *rdPtr++;
      remainingBytesInCurrentBlock--;

      // time to load the next block?
      if (remainingBytesInCurrentBlock == 0)
      {
        LoadNextBlock();
        if (rdPtr == nullptr)
          state = States::empty;
      }

      return readByte;
    }

    case States::empty:
      state = States::error;
      throw stream::EmptyError();

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)

  PANIC();
}
void SectionReader::Pop(void* p, size_t n)
/// \copydoc gpcc::stream::StreamReaderBase::Pop(void* p, size_t n)
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
      // less than 8 bits left?
      if (rdPtr == nullptr)
      {
        // Attempt to read from empty stream.
        // (However, there was at least one bit left to be read, but we need at least one whole byte.
        // ReleaseBuffer() had already been invoked before.)

        state = States::error;
        throw stream::EmptyError();
      }

      uint8_t * wrPtr = static_cast<uint8_t*>(p);
      while (n != 0)
      {
        size_t chunkSize;
        if (n > remainingBytesInCurrentBlock)
          chunkSize = remainingBytesInCurrentBlock;
        else
          chunkSize = n;

        memcpy(wrPtr, rdPtr, chunkSize);
        rdPtr += chunkSize;
        remainingBytesInCurrentBlock -= chunkSize;

        wrPtr += chunkSize;
        n     -= chunkSize;

        // time to load the next block?
        if (remainingBytesInCurrentBlock == 0)
        {
          LoadNextBlock();
          if (rdPtr == nullptr)
          {
            if (n == 0)
              state = States::empty;
            else
            {
              state = States::error;
              throw stream::EmptyError();
            }
          }
        }
      } // while (n != 0)
      break;
    }

    case States::empty:
      state = States::error;
      throw stream::EmptyError();

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)
}
uint8_t SectionReader::PopBits(uint_fast8_t n)
/// \copydoc gpcc::stream::StreamReaderBase::PopBits(uint_fast8_t n)
{
  if (n == 0)
    return 0;

  if (n > 8)
    throw std::invalid_argument("SectionReader::PopBits: bad n");

  switch (state)
  {
    case States::open:
    {
      uint16_t data;

      // need to read another byte from the stream?
      if (nbOfBitsInBitData < n)
      {
        // less than 8 bits left?
        if (rdPtr == nullptr)
        {
          // Attempt to read from empty stream.
          // (However, there was at least one bit left to be read, but we need a whole byte.
          // ReleaseBuffer() had already been invoked before.)

          state = States::error;
          throw stream::EmptyError();
        }

        // read one byte
        uint8_t const readByte = *rdPtr++;
        remainingBytesInCurrentBlock--;

        // time to load the next block?
        if (remainingBytesInCurrentBlock == 0)
          LoadNextBlock();

        // append the read byte to the bits
        data = static_cast<uint16_t>(static_cast<uint16_t>(readByte) << nbOfBitsInBitData) | bitData;
        nbOfBitsInBitData += 8U;
      }
      else
        data = bitData;

      // read bits
      uint8_t const bits = data & (static_cast<uint16_t>(static_cast<uint16_t>(1U) << n) - 1U);

      // remove read bits and update bitData
      nbOfBitsInBitData -= n;
      bitData = static_cast<uint8_t>(data >> n);

      // all bits read and stream empty?
      if ((nbOfBitsInBitData == 0) && (rdPtr == nullptr))
        state = States::empty;

      return bits;
    }

    case States::empty:
      state = States::error;
      throw stream::EmptyError();

    case States::closed:
      throw stream::ClosedError();

    case States::error:
      throw stream::ErrorStateError();
  } // switch (state)

  PANIC();
}

void SectionReader::ReleaseBuffer(void) noexcept
/**
 * \brief Releases the buffer for the currently read storage block and resets associated variables.
 *
 * Calling this if the buffer is already released is not harmful.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 */
{
  spMem.reset();
  rdPtr = nullptr;
  remainingBytesInCurrentBlock = 0;
}
void SectionReader::LoadNextBlock(void)
/**
 * \brief Loads the next block of the section.
 *
 * After calling this, @ref rdPtr must be checked for `nullptr` and @ref state may need to be set to
 * @ref States::empty.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic exception safety:
 * - @ref ReleaseBuffer() is invoked.
 * - the stream's state is set to @ref States::error.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is safe, but:
 * - @ref ReleaseBuffer() is invoked.
 * - the stream's state is set to @ref States::error.
 */
{
  ON_SCOPE_EXIT()
  {
    ReleaseBuffer();
    state = States::error;
  };

  try
  {
    osal::MutexLocker mutexLocker(pESS->mutex);

    if ((pESS->state != EEPROMSectionSystem::States::ro_mount) &&
        (pESS->state != EEPROMSectionSystem::States::mounted))
      throw InsufficientStateError("SectionReader::LoadNextBlock", pESS->state);

    if (pESS->LoadNextBlockOfSection(spMem.get()) != NOBLOCK)
    {
      // prepare to continue reading from the just loaded storage block
      rdPtr = spMem.get() + sizeof(DataBlock_t);
      DataBlock_t const * const pData = static_cast<DataBlock_t const *>(static_cast<void const*>(spMem.get()));
      remainingBytesInCurrentBlock = pData->head.nBytes - (sizeof(DataBlock_t) + sizeof(uint16_t));
    }
    else
    {
      // just clean-up, the calling function will check if state must be switched to States::empty
      ReleaseBuffer();
    }
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(stream::IOError("SectionReader::LoadNextBlock: failed"));
  }

  ON_SCOPE_EXIT_DISMISS();
}

} // namespace internal
} // namespace eeprom_section_system
} // namespace file_systems
} // namespace gpcc
