/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)

#ifndef STDIO_TO_ITERMINAL_HPP_201712292228
#define STDIO_TO_ITERMINAL_HPP_201712292228

#include "ITerminal.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include <termios.h>

namespace gpcc {
namespace cli  {

/**
 * \ingroup GPCC_CLI
 * \brief Adapter providing an @ref ITerminal interface from the standard streams STDIN and STDOUT.
 *
 * This adapter is intended to be used to connect a @ref CLI component to a terminal via STDIN and STDOUT.
 *
 * # Requirements
 * - STDIN must have been opened in blocking mode
 * - STDOUT must have been opened in blocking mode
 * - The terminal connected to STDIN and STDOUT should not be accessed by other processes
 *
 * # Special notes on process termination (e.g. Panic())
 * This class' constructor will apply custom settings to STDIN in order to meet the requirements of
 * class @ref CLI and the @ref ITerminal interface. The original settings are preserved by the constructor
 * and restored by the destructor.
 *
 * Some systems require that the terminal settings are recovered upon process termination to prevent
 * confusion of other processes using the same terminal (e.g. a command shell).
 *
 * To ensure recovery of the original settings, the following actions shall be taken:
 * - The @ref StdIO_to_ITerminal object should always be destroyed when the process terminates _normally_.
 * - During _abnormal_ process termination due to a _panic condition_, the settings should be restored by
 *   installing a custom Panic()-handler during application initialization, which will invoke
 *   @ref RecoverStdInSettings().
 * - During _abnormal_ process termination due to a segmentation fault appropriate signal handlers
 *   could be installed which will invoke @ref RecoverStdInSettings().
 *
 * For details, please refer to:
 * - @ref RecoverStdInSettings()
 * - @ref GPCC_OSAL_PANIC
 *
 * # Usage in a TFC environment
 * This adapter could be used in a test application using an OSAL variant that implements GPCC's TFC feature,
 * though this use case should be very rare.
 *
 * This adapter contains blocking system calls used to access STDIN and STDOUT. The system calls are not seen by
 * TFC and thus they are not managed by TFC. TFC advances the system time only if all threads in the process are
 * blocked on a TFC-managed lock or sleep. When using this adapter, the thread of the @ref CLI instance using this
 * adapter will block on the read() system call, but TFC will not increment the emulated system time. However, physical
 * time passes by while the thread is blocked. To increment the emulated time, the user could execute a CLI command
 * (@ref Command) which invokes @ref gpcc::osal::Thread::Sleep_ms() or @ref gpcc::osal::Thread::Sleep_ns().
 *
 * For details on TFC, please refer to:
 * - @ref GPCC_OSAL
 * - @ref GPCC_TIME_FLOW_CONTROL
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class StdIO_to_ITerminal final : public ITerminal
{
  public:
    /// Maximum value for timeout (in ms) when reading from STDIN.
    static uint16_t const MAX_TIMEOUT_MS = 2550;


    StdIO_to_ITerminal(void);
    StdIO_to_ITerminal(StdIO_to_ITerminal const &) = delete;
    StdIO_to_ITerminal(StdIO_to_ITerminal &&) = delete;
    ~StdIO_to_ITerminal(void);

    StdIO_to_ITerminal& operator=(StdIO_to_ITerminal const &) = delete;
    StdIO_to_ITerminal& operator=(StdIO_to_ITerminal &&) = delete;

    bool RecoverStdInSettings(void) noexcept;

  private:
    /// Mutex used to protect stuff related to reading from STDIN.
    /** Locking order: @ref stdinMutex -> @ref stdinConfigMutex */
    osal::Mutex stdinMutex;

    /// Mutex used to protect stuff related to STDIN settings.
    /** Locking order: @ref stdinMutex -> @ref stdinConfigMutex */
    osal::Mutex stdinConfigMutex;

    /// Mutex used to protect stuff related to writing to STDOUT.
    osal::Mutex stdoutMutex;

    /// termios structure used to conserve the original settings for STDIN.
    /** This is setup in the constructor and not changed after setup.\n
        No mutex is required for read access. */
    struct termios originalSettings;

    /// termios structure containing our own settings.
    /** @ref stdinConfigMutex is required. */
    struct termios workSettings;

    /// Currently configured timeout for reading from STDIN in ms.
    /** @ref stdinConfigMutex is required. */
    uint16_t currentTimeout_ms;

    /// Flag indicating if read access to STDIN has been deactivated and if STDIN settings have been restored.
    /** @ref stdinConfigMutex is required.\n
        true  = Access to STDIN has been deactivated and the @ref originalSettings have been restored\n
        false = Normal operation */
    bool stdInRecoveredAndDeactivated;

    // <-- ITerminal
    size_t Read(char * pBuffer, size_t bufferSize, uint16_t timeout_ms) override;
    void Flush(void) override;

    void Write(char const * pBuffer, size_t s) override;
    // --> ITerminal
};

} // namespace cli
} // namespace gpcc

#endif // STDIO_TO_ITERMINAL_HPP_201712292228
#endif // #if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC) || defined(__DOXYGEN__)
