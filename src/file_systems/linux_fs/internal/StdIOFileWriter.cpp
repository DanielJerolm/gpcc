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

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#include "StdIOFileWriter.hpp"
#include "tools.hpp"
#include "gpcc/src/file_systems/exceptions.hpp"
#include "gpcc/src/file_systems/linux_fs/FileStorage.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include <stdexcept>
#include <system_error>
#include <cerrno>

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
 * \throws FileAlreadyAccessedError   File is already in use and cannot be overwritten
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyAccessedError)).
 *
 * \throws FileAlreadyExistingError   File is already existing and `overwriteIfExist` is false, or `fileName`
 *                                    refers to a directory
 *                                    ([details](@ref gpcc::file_systems::FileAlreadyExistingError)).
 *
 * \throws InsufficientSpaceError     Insufficient free space in underlying storage device
 *                                    ([details](@ref gpcc::file_systems::InsufficientSpaceError)).
 *
 * \throws NoSuchDirectoryError       Directory where the file shall be created is not existing
 *                                    ([details](@ref gpcc::file_systems::NoSuchDirectoryError)).
 *
 * \throws std::system_error          Creating the file has failed.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param fileName
 * Path and name of the file that shall be created. If the referenced file is already existing,
 * then parameter `overwriteIfExist` will control if the file will be overwritten or if an exception will be thrown.\n
 * Links included in the path/filename will be dereferenced.
 *
 * \param overwriteIfExist
 * Controls, if an existing file shall be overwritten or not.\n
 * true  = Overwrite\n
 * false = Do not overwrite. @ref FileAlreadyExistingError will be thrown instead.
 *
 * \param _fileStorage
 * Reference to the @ref FileStorage instance which created this @ref StdIOFileWriter.
 *
 * \param _unlockID
 * String required to unlock the file at the @ref FileStorage instance when the file is closed.
 */
StdIOFileWriter::StdIOFileWriter(std::string const & fileName, bool const overwriteIfExist, FileStorage & _fileStorage, std::string const & _unlockID)
: StreamWriterBase(States::open, Endian::Little)
, fileStorage(_fileStorage)
, unlockID(_unlockID)
, fd(nullptr)
, nbOfBitsWritten(0)
, bitData(0)
{
  if ((!overwriteIfExist) && (CheckFileOrDirExists(fileName)))
    throw FileAlreadyExistingError(fileName);

  fd = fopen(fileName.c_str(), "wb");
  if (fd == nullptr)
  {
    if (errno == EISDIR)
      throw FileAlreadyExistingError(fileName);
    else if ((errno == ENOENT) || (errno == ENOTDIR))
      throw NoSuchDirectoryError(fileName);
    else if ((errno == ENOSPC) || (errno == EDQUOT))
      throw InsufficientSpaceError();
    else if (errno == ETXTBSY)
       throw FileAlreadyAccessedError(fileName);
    else
      throw std::system_error(errno, std::generic_category(), "StdIOFileWriter::StdIOFileWriter: \"fopen\" failed on \"" + fileName + "\"");
  }
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
 * The other @ref StdIOFileWriter instance that shall be moved into the new constructed one.\n
 * The referenced instance will be switched to @ref States::closed.
 */
StdIOFileWriter::StdIOFileWriter(StdIOFileWriter && other) noexcept
: StreamWriterBase(std::move(other))
, fileStorage(other.fileStorage)
, unlockID(std::move(other.unlockID))
, fd(other.fd)
, nbOfBitsWritten(other.nbOfBitsWritten)
, bitData(other.bitData)
{
  other.fd = nullptr;
  other.state = States::closed;
  other.unlockID.clear();
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
StdIOFileWriter::~StdIOFileWriter(void)
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

/// \copydoc gpcc::Stream::IStreamWriter::IsRemainingCapacitySupported(void) const
bool StdIOFileWriter::IsRemainingCapacitySupported(void) const
{
  return false;
}

/// \copydoc gpcc::Stream::IStreamWriter::RemainingCapacity(void) const
size_t StdIOFileWriter::RemainingCapacity(void) const
{
  switch (state)
  {
    case States::open:
      throw std::logic_error("StdIOFileWriter::RemainingCapacity: Operation not supported");

    case States::full:
      // (this state is not used by class StdIOFileWriter)
      throw std::logic_error("StdIOFileWriter::RemainingCapacity: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  }

  PANIC();
}

/// \copydoc gpcc::Stream::IStreamWriter::GetNbOfCachedBits
uint_fast8_t StdIOFileWriter::GetNbOfCachedBits(void) const
{
  switch (state)
  {
    case States::open:
      return nbOfBitsWritten;

    case States::full:
      // (this state is not used by class StdIOFileWriter)
      throw std::logic_error("StdIOFileWriter::GetNbOfCachedBits: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  }

  PANIC();
}

/// \copydoc gpcc::Stream::IStreamWriter::Close
void StdIOFileWriter::Close(void)
{
  switch (state)
  {
    case States::open:
      FlushBitsAndClose();
      break;

    case States::full:
      // (this state is not used by class StdIOFileWriter)
      state = States::error;
      CloseFile();
      throw std::logic_error("StdIOFileWriter::Close: Unused state (States::full) encountered");

    case States::closed:
      break;

    case States::error:
      CloseFile();
      break;
  } // switch (state)
}

/// \copydoc gpcc::Stream::StreamWriterBase::Push(char c)
void StdIOFileWriter::Push(char c)
{
  if (nbOfBitsWritten != 0)
    PushBitsPlusGap();

  switch (state)
  {
    case States::open:
    {
      if (fputc(c, fd) == EOF)
      {
        state = States::error;

        if (errno == ENOSPC)
          throw Stream::FullError();
        else
          ThrowIOError("StdIOFileWriter::Push: \"fputc\" failed", errno);
      }
      break;
    }

    case States::full:
      // (this state is not used by class StdIOFileWriter)
      state = States::error;
      throw std::logic_error("StdIOFileWriter::Push: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

/// \copydoc gpcc::Stream::StreamWriterBase::Push(void const * pData, size_t n)
void StdIOFileWriter::Push(void const * pData, size_t n)
{
  if (n == 0)
    return;

  if (nbOfBitsWritten != 0)
    PushBitsPlusGap();

  switch (state)
  {
    case States::open:
    {
      if (fwrite(pData, n, 1U, fd) == 0)
      {
        state = States::error;

        if (errno == ENOSPC)
          throw Stream::FullError();
        else
          ThrowIOError("StdIOFileWriter::Push: \"fwrite\" failed", errno);
      }
      break;
    }

    case States::full:
      // (this state is not used by class StdIOFileWriter)
      state = States::error;
      throw std::logic_error("StdIOFileWriter::Push: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

/// \copydoc gpcc::Stream::StreamWriterBase::PushBits(uint8_t bits, uint_fast8_t n)
void StdIOFileWriter::PushBits(uint8_t bits, uint_fast8_t n)
{
  if (n == 0)
    return;

  if (n > 8)
    throw std::invalid_argument("StdIOFileWriter::PushBits: n must be [0..8].");

  switch (state)
  {
    case States::open:
    {
      // clear upper bits that shall be ignored
      bits &= static_cast<uint_fast16_t>(static_cast<uint_fast16_t>(1U) << n) - 1U;

      // combine potential previously written bits with the bits that shall be written
      uint_fast16_t data = static_cast<uint_fast16_t>(bitData) | (static_cast<uint_fast16_t>(bits) << nbOfBitsWritten);
      nbOfBitsWritten += n;

      // one byte filled up with bits?
      if (nbOfBitsWritten >= 8U)
      {
        // write byte into the stream
        if (fputc(static_cast<char>(data), fd) == EOF)
        {
          state = States::error;

          if (errno == ENOSPC)
            throw Stream::FullError();
          else
            ThrowIOError("StdIOFileWriter::PushBits: \"fputc\" failed", errno);
        }

        nbOfBitsWritten -= 8U;
        data >>= 8U;
      }

      // store temporary stuff back in bitData
      bitData = static_cast<uint8_t>(data);

      break;
    }

    case States::full:
      // (this state is not used by class StdIOFileWriter)
      state = States::error;
      throw std::logic_error("StdIOFileWriter::PushBits: Unused state (States::full) encountered");

    case States::closed:
      throw Stream::ClosedError();

    case States::error:
      throw Stream::ErrorStateError();
  } // switch (state)
}

/**
 * \brief Fills empty bits up with zeros to get one byte and pushes the byte onto the stream.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the stream enters state @ref States::error, if the stream cannot be recovered (e.g. undo a write)
 *
 * \throws IOError           [Details](@ref gpcc::Stream::IOError)
 * \throws FullError         [Details](@ref gpcc::Stream::FullError)
 * \throws ClosedError       [Details](@ref gpcc::Stream::ClosedError)
 * \throws ErrorStateError   [Details](@ref gpcc::Stream::ErrorStateError)
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the stream enters state @ref States::error, if the stream cannot be recovered (e.g. undo a write)
 */
void StdIOFileWriter::PushBitsPlusGap(void)
{
  // move the bits to be written into d
  char const d = static_cast<char>(bitData);

  // clear bit-buffer now and not after writing the bits in order to prevent a recursive call to this
  nbOfBitsWritten = 0;
  bitData = 0;

  // write bits
  Push(d);
}

/**
 * \brief Throws a @ref gpcc::Stream::IOError plus a nested std::system_error.
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
void StdIOFileWriter::ThrowIOError(char const * const pDescr, int const copyOfErrno) const
{
  try
  {
    throw std::system_error(copyOfErrno, std::generic_category());
  }
  catch (std::exception const &)
  {
    std::throw_with_nested(Stream::IOError(pDescr));
  }
}

/**
 * \brief Closes the underlying file. Any bits not yet written are dropped.
 *
 * \post   The stream is in state @ref States::closed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:\n
 * - In any case, the stream will __always__ be closed, even in case of an error.
 * - Most operating systems close the underlying file, even if an error occurs during
 *   the close-operation. However, the state of the file depends on the underlying
 *   operating system in this case.
 *
 * \throws IOError     [Details](@ref gpcc::Stream::IOError)
 * \throws FullError   [Details](@ref gpcc::Stream::FullError)
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.\n
 * Cancellation could corrupt the object or lead to undefined behavior.
 */
void StdIOFileWriter::CloseFile(void)
{
  ON_SCOPE_EXIT()
  {
    fileStorage.ReleaseWriteLock(unlockID);
    state = States::closed;
    fd = nullptr;
    unlockID.clear();
  };

  if (fclose(fd) != 0)
  {
    if (errno == ENOSPC)
      throw Stream::FullError();
    else
      ThrowIOError("StdIOFileWriter::CloseFile: \"fclose\" failed", errno);
  }
}

/**
 * \brief Closes the underlying file and ignores any errors during close. Any bits not yet written are dropped.
 *
 * \post   The stream is in state @ref States::closed.
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
 * Deferred cancellation is not allowed.\n
 * Cancellation could corrupt the object or lead to undefined behavior.
 */
void StdIOFileWriter::CloseFileNoThrow(void) noexcept
{
  ON_SCOPE_EXIT()
  {
    fileStorage.ReleaseWriteLock(unlockID);
    state = States::closed;
    fd = nullptr;
    unlockID.clear();
  };

  (void)fclose(fd);
}

/**
 * \brief Writes any pending bits to the underlying file and closes it.
 *
 * \post   The stream is in state @ref States::closed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - In any case, the stream will __always__ be closed, even in case of an error.
 * - Most operating systems close the underlying file, even if an error occurs during
 *   the close-operation. However, the state of the file depends on the underlying
 *   operating system in this case.
 *
 * \throws IOError           [Details](@ref gpcc::Stream::IOError)
 * \throws FullError         [Details](@ref gpcc::Stream::FullError)
 * \throws ClosedError       [Details](@ref gpcc::Stream::ClosedError)
 * \throws ErrorStateError   [Details](@ref gpcc::Stream::ErrorStateError)
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.\n
 * Cancellation could corrupt the object or lead to undefined behavior.
 */
void StdIOFileWriter::FlushBitsAndClose(void)
{
  ON_SCOPE_EXIT() { CloseFileNoThrow(); };

  if (nbOfBitsWritten != 0)
    PushBitsPlusGap();

  ON_SCOPE_EXIT_DISMISS();

  CloseFile();
}

} // namespace internal
} // namespace linux_fs
} // namespace file_systems
} // namespace gpcc

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
