/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef FAKETERMINAL_HPP_201710172039
#define FAKETERMINAL_HPP_201710172039

#include <gpcc/cli/ITerminal.hpp>
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/ConditionVariable.hpp"
#include <stdexcept>
#include <string>
#include <vector>

namespace gpcc_tests {
namespace cli        {

/**
 * \ingroup GPCC_TESTS_FAKES_CLI
 * \brief Exception that can be intentionally thrown by class @ref FakeTerminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class IntentionallyThrownException : public std::runtime_error
{
  public:
    inline IntentionallyThrownException(void) : std::runtime_error("Intentionally thrown exception") {};
    virtual ~IntentionallyThrownException(void) = default;
};

/**
 * \ingroup GPCC_TESTS_FAKES_CLI
 * \brief Fake terminal intended to be used in unit tests that include an instance of class @ref gpcc::cli::CLI.
 *
 * The fake terminal offers the @ref gpcc::cli::ITerminal interface, which allows to connect it to
 * any [CLI](@ref gpcc::cli::CLI) instance.
 *
 * # Features:
 * - Input from an imaginary user can be entered into the terminal using the public methods
 *   with the "Input_" prefix. A test fixture may invoke these methods to generate input from
 *   an imaginary user:
 *   + @ref Input(char const * input);
 *   + @ref Input_POS1(void);
 *   + @ref Input_END(void);
 *   + @ref Input_ENTER(void);
 *   + @ref Input_DEL(size_t times);
 *   + @ref Input_Backspace(size_t times);
 *   + @ref Input_TAB(size_t times);
 *   + @ref Input_ArrowLeft(size_t times);
 *   + @ref Input_ArrowRight(size_t times);
 *   + @ref Input_ArrowUp(size_t times);
 *   + @ref Input_ArrowDown(size_t times);
 *   + @ref Input_CtrlC(void);
 * - The input of the (imaginary) user can be read via @ref gpcc::cli::ITerminal::Read().
 * - Text can be printed to the imaginary screen of the fake terminal via @ref gpcc::cli::ITerminal::Write().
 * - Control sequences for manipulating the cursor and the content of the imaginary screen can be
 *   written to the fake terminal via @ref gpcc::cli::ITerminal::Write().
 * - The Compare(...)-methods can be used to check the content of the terminal's imaginary screen.
 * - @ref GetScreenContent() can be used to fetch the content of the terminal's imaginary screen in
 *   an `std::string` and perform more sophisticated tests, using e.g. regular expressions etc.
 * - The content of the terminal's imaginary screen can be printed to stdout for debug purposes
 *   using the @ref PrintToStdOut() method.
 * - Lines that have dropped out at the top of the screen due to printing new lines at the bottom of
 *   the screen can be optionally recorded, too. See @ref EnableRecordingOfDroppedOutLines() and
 *   @ref GetDroppedOutLinesPlusCurrentScreenContent().
 * - The Read() and Write() methods offered by the @ref gpcc::cli::ITerminal interface can be
 *   programmed via @ref RequestThrowUponWrite() and @ref RequestThrowUponRead() to intentionally
 *   throw an exception of type @ref IntentionallyThrownException.
 *
 * # Special notes:
 * - Terminal control sequences written via @ref gpcc::cli::ITerminal::Write() cannot be splitted
 *   among multiple calls to gpcc::cli::ITerminal::Write(). For @ref gpcc::cli::CLI this is not an issue.
 * - GPCC should be build with `-DGPCC_CLI_NO_FONT_STYLES` ([details](@ref GPC_CLI_FCS)).
 *
 * # Usage
 *
 * ~~~{.cpp}
 * // In this example, we allocate the terminal and the CLI on the stack
 * gpcc_tests::cli::FakeTerminal terminal(120U, 8U);
 * gpcc::cli::CLI cli(terminal, 120U, 8U, "CLI", nullptr);
 *
 * // start CLI
 * cli.Start(gpcc::osal::Thread::SchedPolicy::Other, 0, gpcc::osal::Thread::GetDefaultStackSize());
 * ON_SCOPE_EXIT(undoCLIStart)
 * {
 *   cli.Stop();
 * };
 *
 * // CLI flushes the terminal, so call WaitForInputProcessed() before using it. Otherwise subdequent input
 * // might be discarded.
 * terminal.WaitForInputProcessed();
 *
 * // login
 * terminal.Input("login");
 * terminal.Input_ENTER();
 * terminal.WaitForInputProcessed();
 *
 * // issue a command
 * terminal.Input("help");
 * terminal.Input_ENTER();
 * terminal.WaitForInputProcessed();
 *
 * // print current screen content to stdout
 * terminal.PrintToStdOut();
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class FakeTerminal final : public gpcc::cli::ITerminal
{
  public:
    /// Width of the terminal's imaginary screen in characters.
    uint8_t const width;

    /// Height of the terminal's imaginary screen in lines.
    uint8_t const height;


    FakeTerminal(void) = delete;
    FakeTerminal(uint8_t const _width, uint8_t const _height);
    FakeTerminal(FakeTerminal const &) = delete;
    FakeTerminal(FakeTerminal &&) = delete;
    ~FakeTerminal(void) = default;

    FakeTerminal& operator=(FakeTerminal const &) = delete;
    FakeTerminal& operator=(FakeTerminal &&) = delete;

    void EnableRecordingOfDroppedOutLines(void);

    void RequestThrowUponWrite(void);
    void RequestThrowUponRead(void);
    void RequestThrowUponFlush(void);

    void Input(char const * input);
    void Input_POS1(void);
    void Input_END(void);
    void Input_ENTER(void);
    void Input_DEL(size_t times);
    void Input_Backspace(size_t times);
    void Input_TAB(size_t times);
    void Input_ArrowLeft(size_t times);
    void Input_ArrowRight(size_t times);
    void Input_ArrowUp(size_t times);
    void Input_ArrowDown(size_t times);
    void Input_CtrlC(void);
    void WaitForInputProcessed(void);

    bool Compare(char const * ref[]) const;
    bool Compare(uint8_t const expected_cursor_x, uint8_t const expected_cursor_y) const;

    std::string GetScreenContent(void) const;
    std::string GetDroppedOutLinesPlusCurrentScreenContent(void) const;

    void PrintToStdOut(void);

  private:
    // ----------------------------------------
    // Output direction: from ITerminal to user
    // ----------------------------------------
    /// Mutex used to protect stuff associated with writing to the terminal's imaginary screen.
    mutable gpcc::osal::Mutex outputMutex;

    /// Imaginary screen of the terminal.
    /** @ref outputMutex is required. */
    std::vector<std::string> lines;

    /// Record of lines that dropped out at the top of the terminal's virtual screen.
    /** @ref outputMutex is required.\n
        Lines are only recorded, if @ref recordLinesDroppedOut is `true`. */
    std::vector<std::string> linesDroppedOut;

    /// Flag indicating if lines that dropped out at the top of the terminal's virtual screen
    /// shall be recorded in @ref linesDroppedOut or not.
    /** @ref outputMutex is required. */
    bool recordLinesDroppedOut;

    /// Current cursor position (x-component).
    /** @ref outputMutex is required. */
    uint8_t cursor_x;

    /// Current cursor position (y-component).
    /** @ref outputMutex is required. */
    uint8_t cursor_y;

    /// Flag indicating if throwing an exception is requested.
    /** @ref outputMutex is required. */
    bool throwRequested_out;


    // ---------------------------------------
    // Input direction: from user to ITerminal
    // ---------------------------------------
    /// Mutex used to protect stuff associated with reading from the terminal's keyboard.
    gpcc::osal::Mutex inputMutex;

    /// Input buffer.
    /** @ref inputMutex is required. */
    std::vector<char> inputBuffer;

    /// Condition Variable indicating that @ref inputBuffer is no longer empty.
    /** This is to be used in conjunction with @ref inputMutex. */
    gpcc::osal::ConditionVariable inputBufferNotEmptyCV;

    /// Flag indicating if a thread is blocked in ITerminal::Read(), waiting for input.
    /** @ref inputMutex is required. */
    bool readingThreadBlocked;

    /// Condition variable indicating that a thread is blocked in ITerminal::Read().
    /** This is to be used in conjunction with @ref inputMutex. \n
        Threads are blocked in ITerminal::Read(), if there is nothing to do:
        - @ref inputBuffer empty
        - @ref throwRequested_in is false */
    gpcc::osal::ConditionVariable readingThreadBlockedCV;

    /// Flag indicating if throwing an exception is requested.
    /** @ref inputMutex is required. */
    bool throwRequested_in;

    /// Flag indicating if throwing an exception is requested for ITerminal::Flush().
    /** @ref inputMutex is required. */
    bool throwRequested_flush;


    void WriteToInputBuffer(char const * p);

    // <-- gpcc::cli::ITerminal
    size_t Read(char * pBuffer, size_t bufferSize, uint16_t timeout_ms) override;
    void Flush(void) override;
    void Write(char const * pBuffer, size_t s) override;
    // --> gpcc::cli::ITerminal
};

} // namespace cli
} // namespace gpcc_tests

#endif // FAKETERMINAL_HPP_201710172039
