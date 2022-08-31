/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#include <gpcc/cli/StdIO_to_ITerminal.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <errno.h>
#include <unistd.h>
#include <cstdio>

namespace gpcc {
namespace cli  {

#ifndef __DOXYGEN__
uint16_t const StdIO_to_ITerminal::MAX_TIMEOUT_MS;
#endif

/**
 * \brief Constructor.
 *
 * Creates an @ref StdIO_to_ITerminal object and configures STDIN according to the needs of @ref CLI. \n
 * STDIN's current configuration is stored and later restored either when the @ref StdIO_to_ITerminal object
 * is destroyed or when @ref RecoverStdInSettings() is invoked.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
StdIO_to_ITerminal::StdIO_to_ITerminal(void)
: ITerminal()
, stdinMutex()
, stdinConfigMutex()
, stdoutMutex()
, currentTimeout_ms(1000)
, stdInRecoveredAndDeactivated(false)
{
  int status;

  // conserve current STDIN settings
  // -------------------------------
  status = tcgetattr(STDIN_FILENO, &originalSettings);
  if (status != 0)
    throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::StdIO_to_ITerminal: tcgetattr() failed (capture of original settings)");

  // initialize our own settings for STDIN
  // -------------------------------------
  workSettings = originalSettings;

  // configuration:
  // - input characters are not echoed back
  // - non-canonical
  // - no check for special input characters
  // - read() shall return after reception of at least one character or timeout
  workSettings.c_lflag      = workSettings.c_lflag & ~(ECHO|ICANON|ISIG);
  workSettings.c_cc[VMIN]   = 0;
  workSettings.c_cc[VTIME]  = (currentTimeout_ms + 99U) / 100U;

  // apply our own settings to STDIN
  // -------------------------------
  status = tcsetattr(STDIN_FILENO, TCSAFLUSH, &workSettings);
  if (status != 0)
    throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::StdIO_to_ITerminal: tcsetattr() failed");

  // ensure that original settings are restored in case of any error
  ON_SCOPE_EXIT() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings); };

  // check if our own settings have been successfully applied
  // --------------------------------------------------------
  struct termios currentSettings;
  status = tcgetattr(STDIN_FILENO, &currentSettings);
  if (status != 0)
    throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::StdIO_to_ITerminal: tcgetattr() failed (verification of applied settings)");

  if ((currentSettings.c_lflag     != workSettings.c_lflag) ||
      (currentSettings.c_cc[VMIN]  != workSettings.c_cc[VMIN]) ||
      (currentSettings.c_cc[VTIME] != workSettings.c_cc[VTIME]))
  {
    throw std::runtime_error("StdIO_to_ITerminal::StdIO_to_ITerminal: Settings could not be applied");
  }

  // everything is fine
  // ------------------
  ON_SCOPE_EXIT_DISMISS();
}

/**
 * \brief Destructor.
 *
 * If @ref RecoverStdInSettings() has not been invoked before, then this will
 * recover the original settings for STDIN which have been stored by the constructor.
 *
 * Before destroying an @ref StdIO_to_ITerminal object it must be ensured, that a potentially installed
 * custom Panic()-handler will not invoke @ref RecoverStdInSettings() any more. This destructor may invoke
 * gpcc::osal::Panic().
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
StdIO_to_ITerminal::~StdIO_to_ITerminal(void)
{
  gpcc::osal::MutexLocker stdinConfigMutexLocker(stdinConfigMutex);

  // recover stdin settings if necessary
  if (!stdInRecoveredAndDeactivated)
  {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings) != 0)
      gpcc::osal::Panic("StdIO_to_ITerminal::~StdIO_to_ITerminal: tcsetattr() failed");
  }
}

/**
 * \brief Recovers the original STDIN settings stored by the constructor and deactivates any read access to STDIN.
 *
 * The constructor of this class applies special settings to STDIN via `tcsetattr(...)`. The original settings are
 * stored by the constructor for recovery by this class' destructor.
 *
 * Recovery is required on some systems, because terminal settings are not automatically recovered on process
 * termination and other processes using the terminal (e.g. a command shell) might be confused by the settings
 * applied by this class' constructor.
 *
 * In some situations (especially during abnormal process termination, e.g. via @ref gpcc::osal::Panic()), this
 * class' destructor will not run and thus the original settings will not be restored.
 *
 * On some systems, the settings applied by the constructor are not automatically reverted on process termination and
 * recovery _is_ required. This method is intended to be invoked from a custom Panic()-handler in order to recover the
 * original STDIN settings stored by the constructor before the process will be terminated by the Panic()-handler.
 * See @ref GPCC_OSAL_PANIC for details on Panic() handling.
 *
 * Note:
 * - This method contains a mutex lock operation which might contain a call to @ref gpcc::osal::Panic().
 * - If read access to STDIN is already deactivated, then this method does nothing.
 * - Any new attempt to access STDIN via @ref ITerminal::Read() or @ref ITerminal::Flush() will be rejected.\n
 *   If a read-access is in process (a thread is blocked in @ref ITerminal::Read()), then the thread might return
 *   with no data read or incomplete data read, since this method flushes STDIN.
 * - A thread already blocked in @ref ITerminal::Read() will not notice the call to this method.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Result of the operation:\n
 * true  = success\n
 * false = error
 */
bool StdIO_to_ITerminal::RecoverStdInSettings(void) noexcept
{
  try
  {
    gpcc::osal::MutexLocker stdinConfigMutexLocker(stdinConfigMutex);

    if (!stdInRecoveredAndDeactivated)
    {
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings) != 0)
        return false;

      stdInRecoveredAndDeactivated = true;
    }
  }
  catch (std::exception const &)
  {
    return false;
  }

  return true;
}

// <-- ITerminal
/// \copydoc ITerminal::Read
size_t StdIO_to_ITerminal::Read(char * pBuffer, size_t bufferSize, uint16_t timeout_ms)
{
  // check input parameters
  if (pBuffer == nullptr)
    throw std::invalid_argument("StdIO_to_ITerminal::Read: !pBuffer");

  if (bufferSize == 0U)
    throw std::invalid_argument("StdIO_to_ITerminal::Read: bufferSize == 0");

  if (timeout_ms > MAX_TIMEOUT_MS)
    throw std::invalid_argument("StdIO_to_ITerminal::Read: timeout_ms too large");

  gpcc::osal::AdvancedMutexLocker stdinMutexLocker(stdinMutex);
  gpcc::osal::AdvancedMutexLocker stdinConfigMutexLocker(stdinConfigMutex);

  // deactivated?
  if (stdInRecoveredAndDeactivated)
    throw std::logic_error("StdIO_to_ITerminal::Read: Read-access to STDIN has been deactivated and settings have been recovered");

  // change the read timeout if necessary
  if (timeout_ms != currentTimeout_ms)
  {
    uint8_t const newVTIME = (timeout_ms + 99U) / 100U;

    // change of VTIME necessary?
    if (newVTIME != workSettings.c_cc[VTIME])
    {
      // backup current value of VTIME
      auto const prevVTIME = workSettings.c_cc[VTIME];

      // change VTIME
      workSettings.c_cc[VTIME] = newVTIME;
      int const status = tcsetattr(STDIN_FILENO, TCSANOW, &workSettings);

      // error?
      if (status != 0)
      {
        // recover previous VTIME value
        workSettings.c_cc[VTIME] = prevVTIME;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &workSettings) != 0)
        {
          stdinConfigMutexLocker.Unlock();
          stdinMutexLocker.Unlock();

          gpcc::osal::Panic("StdIO_to_ITerminal::Read: tcsetattr() failed upon error recovery");
        }

        // throw an exception, because change of CTIME failed
        throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::Read: tcsetattr() failed (new VTIME value)");
      }
    }

    currentTimeout_ms = timeout_ms;
  }

  stdinConfigMutexLocker.Unlock();

  // perform the read operation
  while (true)
  {
    ssize_t const n = read(STDIN_FILENO, pBuffer, bufferSize);
    if (n < 0)
    {
      if (errno == EINTR)
        continue;
      else if (errno == EAGAIN)
        throw std::runtime_error("StdIO_to_ITerminal::Read: STDIN has been opened in non-blocking mode");
      else
        throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::Read: read() failed");
    }

    return n;
  }
}

/// \copydoc ITerminal::Flush
void StdIO_to_ITerminal::Flush(void)
{
  gpcc::osal::MutexLocker stdinMutexLocker(stdinMutex);
  gpcc::osal::MutexLocker stdinConfigMutexLocker(stdinConfigMutex);

  // deactivated?
  if (stdInRecoveredAndDeactivated)
    throw std::logic_error("StdIO_to_ITerminal::Flush: Access to STDIN has been deactivated and settings have been recovered");

  // perform the flush
  int const status = tcflush(STDIN_FILENO, TCIFLUSH);
  if (status != 0)
    throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::Flush: tcflush() failed");
}

/// \copydoc ITerminal::Write
void StdIO_to_ITerminal::Write(char const * pBuffer, size_t s)
{
  // check input parameters
  if (pBuffer == nullptr)
    throw std::invalid_argument("StdIO_to_ITerminal::Write: !pBuffer");

  if (s > std::numeric_limits<ssize_t>::max())
    throw std::invalid_argument("StdIO_to_ITerminal::Write: s too large");

  // special case: nothing to print
  if (s == 0U)
    return;

  gpcc::osal::MutexLocker stdoutMutexLocker(stdoutMutex);

  // write the data
  ssize_t ret;
  while ((s != 0) && ((ret = write(STDOUT_FILENO, pBuffer, s)) != 0))
  {
    if (ret == -1)
    {
      if (errno == EINTR)
        continue;
      else if (errno == EAGAIN)
        throw std::runtime_error("StdIO_to_ITerminal::Write: STDOUT has been opened in non-blocking mode");
      else
        throw std::system_error(errno, std::generic_category(), "StdIO_to_ITerminal::Write: write() failed");
    }

    s       -= ret;
    pBuffer += ret;
  }

  // ...and finally flush STDOUT
  fflush(stdout);
}
// --> ITerminal

} // namespace cli
} // namespace gpcc

#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
