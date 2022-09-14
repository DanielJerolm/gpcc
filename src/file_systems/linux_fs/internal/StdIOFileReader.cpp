/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#include "StdIOFileReader.hpp"
#include <gpcc/file_systems/exceptions.hpp>
#include <gpcc/file_systems/linux_fs/FileStorage.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/stream/stream_errors.hpp>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace gpcc         {
namespace file_systems {
namespace linux_fs     {
namespace internal     {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws IOError                Reading from the file has failed ([details](@ref gpcc::Stream::IOError)).
 *
 * \throws NoSuchFileError        File is not existing ([details](@ref gpcc::file_systems::NoSuchFileError)).
 *
 * \throws NotARegularFileError   File is not a regular file ([details](@ref gpcc::file_systems::NotARegularFileError)).
 *
 * \throws std::system_error      Opening the file has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param fileName
 * Path and name of the file that shall be opened for reading.\n
 * Links included in the path/filename will be dereferenced.\n
 * The referenced file must be a regular file.
 *
 * \param _fileStorage
 * Reference to the @ref FileStorage instance which created this @ref StdIOFileReader.
 *
 * \param _unlockID
 * String required to unlock the file at the @ref FileStorage instance when the file is closed.
 */
StdIOFileReader::StdIOFileReader(std::string const & fileName, FileStorage & _fileStorage, std::string const & _unlockID)
: StreamReaderBase(States::open, Endian::Little)
, fileStorage(_fileStorage)
, unlockID(_unlockID)
, fd(nullptr)
, nextByte(0)
, nbOfBitsInBitData(0)
, bitData(0)
{
  // step 1: check if the file is a REGULAR file
  struct stat s;
  if (stat(fileName.c_str(), &s) != 0)
  {
    if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchFileError(fileName);
    else
      throw std::system_error(errno, std::generic_category(), "StdIOFileReader::StdIOFileReader: \"stat\" failed on \"" + fileName + "\"");
  }

  if (S_ISDIR(s.st_mode))
    throw NoSuchFileError(fileName);

  if (!S_ISREG(s.st_mode))
    throw NotARegularFileError(fileName);

  // step 2: open file
  fd = fopen(fileName.c_str(), "rb");
  if (fd == nullptr)
  {
    if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchFileError(fileName);
    else
      throw std::system_error(errno, std::generic_category(), "StdIOFileReader::StdIOFileReader: \"fopen\" failed on \"" + fileName + "\"");
  }

  ReadAheadNextByte();
}

/**
 * \brief Move constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * The other @ref StdIOFileReader instance that shall be moved into the new created one.\n
 * The other instance is left in state @ref States::closed.
 */
StdIOFileReader::StdIOFileReader(StdIOFileReader && other) noexcept
: StreamReaderBase(std::move(other))
, fileStorage(other.fileStorage)
, unlockID(std::move(other.unlockID))
, fd(other.fd)
, nextByte(other.nextByte)
, nbOfBitsInBitData(other.nbOfBitsInBitData)
, bitData(other.bitData)
{
  other.unlockID.clear();
  other.fd = nullptr;
  other.state = States::closed;
}

/**
 * \brief Destructor. Closes the file (if not yet done) and releases the object.
 *
 * _Any stream should be closed via_ @ref Close() _before it is released._\n
 * If it is not closed yet, then it will be closed now by this destructor.\n
 * If the close-operation fails or if thread cancellation occurs, then the application
 * will terminate via @ref gpcc::osal::Panic(). To avoid this scenario, invoke @ref Close()
 * before destroying the object.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee:\n
 * Operations may only fail due to serious errors that will result in program termination via Panic(...).\n
 * To prevent any error, ensure that the stream is closed __before__ it is released.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.\n
 * Any cancellation will result in @ref gpcc::osal::Panic().
 */
StdIOFileReader::~StdIOFileReader(void)
{
  try
  {
    if (state != States::closed)
      Close();
  }
  catch (std::exception const & e)
  {
    PANIC_E(e);
  }
  catch (...)
  {
    PANIC();
  }
}

/// \copydoc gpcc::Stream::IStreamReader::IsRemainingBytesSupported(void) const
bool StdIOFileReader::IsRemainingBytesSupported(void) const
{
  return false;
}

/// \copydoc gpcc::Stream::IStreamReader::RemainingBytes(void) const
size_t StdIOFileReader::RemainingBytes(void) const
{
  switch (state)
  {
    case States::open:
      // intentional fall-through
    case States::empty:
      throw std::logic_error("StdIOFileReader::RemainingBytes: Operation not supported");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  }

  PANIC();
}

/// \copydoc gpcc::Stream::IStreamReader::EnsureAllDataConsumed
void StdIOFileReader::EnsureAllDataConsumed(RemainingNbOfBits const expectation) const
{
  switch (state)
  {
    case States::open:
    // intentional fall-through
    case States::empty:
      {
        switch (expectation)
        {
          case RemainingNbOfBits::sevenOrLess:
          {
            if (feof(fd) == 0)
              throw Stream::RemainingBitsError();
            break;
          }

          case RemainingNbOfBits::moreThanSeven:
          {
            if (feof(fd) != 0)
              throw Stream::RemainingBitsError();
            break;
          }

          case RemainingNbOfBits::any:
          {
            break;
          }

          default:
          {
            // (0..7)

            if ((feof(fd) == 0) || (nbOfBitsInBitData != static_cast<uint8_t>(expectation)))
              throw Stream::RemainingBitsError();
            break;
          }
        } // switch (expectation)

        break;
      } // case States::open / States::empty

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

/// \copydoc gpcc::Stream::IStreamReader::Close(void)
void StdIOFileReader::Close(void)
{
  if (state != States::closed)
  {
    ON_SCOPE_EXIT()
    {
      fileStorage.ReleaseReadLock(unlockID);
      state = States::closed;
      fd = nullptr;
      unlockID.clear();
    };

    if (fclose(fd) != 0)
      ThrowIOErrorPlusNestedSystemError("StdIOFileReader::Close: \"fclose\" failed", errno);
  }
}

/// \copydoc gpcc::Stream::IStreamReader::Skip
void StdIOFileReader::Skip(size_t nBits)
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
          if (feof(fd) != 0)
            state = States::empty;

          // finished?
          if (nBits == 0U)
            return;
        }
      }

      // at this point program logic guarantees, that "nbOfBitsInBitData" is zero and "nBits" is not zero

      ON_SCOPE_EXIT(setErrorState) { state = States::error; };

      // stream empty?
      if (feof(fd) != 0)
        throw Stream::EmptyError();

      // calculate the number of bytes and bits to be skipped
      size_t             skip_bytes = nBits / 8U;
      uint_fast8_t const skip_bits  = nBits % 8U;

      // skip bytes
      if (skip_bytes != 0U)
      {
        // The last byte must be skipped using ReadAheadNextByte(). The stream's state must not
        // be "empty" afterwards. If we would use fseek() only, then we could not detect if there
        // is an attempt to skip/read beyond the end of the stream/file.

        if (skip_bytes > 2U)
        {
          auto const seek_offset = skip_bytes - 2U;

          if (seek_offset > std::numeric_limits<long>::max())
            throw std::runtime_error("StdIOFileReader::Skip: Number of bytes to skip exceeds data type 'long' in internal computation");

          if (fseek(fd, seek_offset, SEEK_CUR) != 0)
            ThrowIOErrorPlusNestedSystemError("StdIOFileReader::Skip: fseek failed", errno);

          skip_bytes -= seek_offset;
        }

        if (skip_bytes == 2U)
        {
          ReadAheadNextByte();

          if (state == States::empty)
            throw Stream::EmptyError();
        }

        ReadAheadNextByte();
      }

      // skip bits
      if (skip_bits != 0U)
      {
        // program logic guarantees, that nbOfBitsInBitData is zero

        // stream empty?
        if (feof(fd) != 0)
          throw Stream::EmptyError();

        // read one byte and skip bits
        bitData = nextByte >> skip_bits;
        nbOfBitsInBitData = 8U - skip_bits;

        ReadAheadNextByte();
      }

      ON_SCOPE_EXIT_DISMISS(setErrorState);
      break;
    }

    case States::empty:
      state = States::error;
      throw Stream::EmptyError();

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

/// \copydoc gpcc::Stream::IStreamReader::Read_string(void)
std::string StdIOFileReader::Read_string(void)
{
  std::string s;

  DiscardBits();

  switch (state)
  {
    case States::open:
    {
      ON_SCOPE_EXIT() { state = States::error; };

      UndoReadAhead();

      int c;
      while (((c = fgetc(fd)) != EOF) && (c != 0))
        s += static_cast<char>(c);

      if (ferror(fd) != 0)
        ThrowIOErrorPlusNestedSystemError("StdIOFileReader::Read_string: \"fgetc\" failed", errno);

      if (c != 0)
        throw Stream::EmptyError();

      ReadAheadNextByte();

      ON_SCOPE_EXIT_DISMISS();
      break;
    }

    case States::empty:
      state = States::error;
      throw Stream::EmptyError();

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)

  return s;
}

/// \copydoc gpcc::Stream::IStreamReader::Read_line(void)
std::string StdIOFileReader::Read_line(void)
{
  std::string s;

  DiscardBits();

  switch (state)
  {
    case States::open:
    {
      ON_SCOPE_EXIT() { state = States::error; };

      UndoReadAhead();

      int c;
      while (((c = fgetc(fd)) != EOF) && (c != '\n') && (c != '\r') && (c != 0))
        s += static_cast<char>(c);

      if (ferror(fd) != 0)
        ThrowIOErrorPlusNestedSystemError("StdIOFileReader::Read_line: \"fgetc\" failed", errno);

      // consume NUL, '\r', '\n', or '\r\n'
      if (c != EOF)
      {
        if (c == '\r')
        {
          // its a '\r' or '\r\n'
          ReadAheadNextByte();

          if ((state != States::empty) && (nextByte == '\n'))
          {
            // it was an '\r\n'
            ReadAheadNextByte();
          }
        }
        else
        {
          // its a NUL or '\n'
          ReadAheadNextByte();
        }
      }
      else
      {
        state = States::empty;
      }

      ON_SCOPE_EXIT_DISMISS();
      break;
    }

    case States::empty:
      state = States::error;
      throw Stream::EmptyError();

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)

  return s;
}

/// \copydoc gpcc::Stream::StreamReaderBase::Pop(void)
unsigned char StdIOFileReader::Pop(void)
{
  DiscardBits();

  switch (state)
  {
    case States::open:
    {
      uint8_t const readData = nextByte;
      try
      {
        ReadAheadNextByte();
      }
      catch (...)
      {
        state = States::error;
        throw;
      }

      return static_cast<unsigned char>(readData);
    }

    case States::empty:
      state = States::error;
      throw Stream::EmptyError();

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)

  PANIC();
}

/// \copydoc gpcc::Stream::StreamReaderBase::Pop(void* p, size_t n)
void StdIOFileReader::Pop(void* p, size_t n)
{
  if (n == 0)
    return;

  DiscardBits();

  switch (state)
  {
    case States::open:
    {
      ON_SCOPE_EXIT() { state = States::error; };

      UndoReadAhead();

      size_t const nr = fread(p, n, 1, fd);
      if (nr == 0)
      {
        if (ferror(fd) != 0)
          ThrowIOErrorPlusNestedSystemError("StdIOFileReader::Pop: \"fread\" failed", errno);
        else if (feof(fd) != 0)
          throw Stream::EmptyError();
        else
          throw Stream::IOError("StdIOFileReader::Pop: \"fread\" failed");
      }

      ReadAheadNextByte();

      ON_SCOPE_EXIT_DISMISS();
      break;
    }

    case States::empty:
      state = States::error;
      throw Stream::EmptyError();

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

/// \copydoc gpcc::Stream::StreamReaderBase::PopBits(uint_fast8_t n)
uint8_t StdIOFileReader::PopBits(uint_fast8_t n)
{
  if (n == 0)
    return 0;

  if (n > 8U)
    throw std::invalid_argument("StdIOFileReader::PopBits: Bad n");

  switch (state)
  {
    case States::open:
    {
      uint16_t data;

      // need to read another byte from the stream?
      if (nbOfBitsInBitData < n)
      {
        if (feof(fd) != 0)
        {
          state = States::error;
          throw Stream::EmptyError();
        }

        data = static_cast<uint16_t>(static_cast<uint16_t>(nextByte) << nbOfBitsInBitData) | bitData;
        nbOfBitsInBitData += 8U;

        try
        {
          ReadAheadNextByte();
        }
        catch (...)
        {
          state = States::error;
          throw;
        }
      }
      else
        data = bitData;

      // read bits
      uint8_t const bits = data & (static_cast<uint_fast16_t>(static_cast<uint_fast16_t>(1U) << n) - 1U);

      // remove read bits and update bitData
      nbOfBitsInBitData -= n;
      bitData = static_cast<uint8_t>(data >> n);

      // all bits read and file empty?
      if ((nbOfBitsInBitData == 0) && (feof(fd) != 0))
        state = States::empty;

      return bits;
    }

    case States::empty:
      state = States::error;
      throw Stream::EmptyError();

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)

  PANIC();
}

/**
 * \brief If the object's state is @ref States::open and if there are any bits not yet read,
 *        then the bits are discarded and the state is set to @ref States::empty if @ref fd is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void StdIOFileReader::DiscardBits(void) noexcept
{
  if ((state == States::open) && (nbOfBitsInBitData != 0))
  {
    nbOfBitsInBitData = 0;
    bitData = 0;

    if (feof(fd) != 0)
      state = States::empty;
  }
}

/**
 * \brief Reverts the last call to @ref ReadAheadNextByte().
 *
 * Note: Reverting multiple calls is not possible and will result in undefined behavior.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws IOError   Access to file has failed ([details](@ref gpcc::Stream::IOError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void StdIOFileReader::UndoReadAhead(void)
{
  if (ungetc(nextByte, fd) == EOF)
    ThrowIOErrorPlusNestedSystemError("StdIOFileReader::UndoReadAhead: \"ungetc\" failed", errno);
}

/**
 * \brief Reads the next byte from @ref fd into @ref nextByte.
 *
 * If there is no byte to be read, then @ref state will be set to @ref States::empty if there are no
 * more bits to be read, too.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws IOError   Access to file has failed ([details](@ref gpcc::Stream::IOError)).
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void StdIOFileReader::ReadAheadNextByte(void)
{
  int const c = fgetc(fd);
  if (c == EOF)
  {
    if (ferror(fd) != 0)
    {
      ThrowIOErrorPlusNestedSystemError("StdIOFileReader::ReadAheadNextByte: \"fgetc\" failed", errno);
    }
    else if (feof(fd) != 0)
    {
      if (nbOfBitsInBitData == 0)
        state = States::empty;
    }
    else
    {
      throw Stream::IOError("StdIOFileReader::ReadAheadNextByte: \"fgetc\" failed");
    }
  }
  else
  {
    nextByte = static_cast<uint8_t>(c);
  }
}

/**
 * \brief Throws an @ref gpcc::Stream::IOError plus a nested std::system_error.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * This intentionally throws.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pDescr
 * Pointer to a null-terminated c-string containing the text for the @ref gpcc::Stream::IOError exception.
 * \param copyOfErrno
 * `errno` value for the nested std::system_error exception.
 */
void StdIOFileReader::ThrowIOErrorPlusNestedSystemError(char const * const pDescr, int const copyOfErrno) const
{
  try
  {
    throw std::system_error(copyOfErrno, std::generic_category());
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(gpcc::Stream::IOError(pDescr));
  }
}

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
