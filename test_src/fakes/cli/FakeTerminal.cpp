/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "FakeTerminal.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/string/tools.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <iomanip>
#include <iostream>
#include <cstring>

namespace gpcc_tests {
namespace cli        {


/**
 * \brief Constructor. Creates a @ref FakeTerminal with configurable screen size.
 *
 * If required, @ref EnableRecordingOfDroppedOutLines() can be invoked after object creation to
 * enable recording of lines that drop out at the top of the terminal's virtual screen when new
 * lines are printed at the bottom of the terminal's screen.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _width
 * Desired width of the terminal's imaginary screen in characters.\n
 * Allowed range: 2..254.
 * \param _height
 * Desired height of the terminal's imanginary screen in lines.\n
 * Allowed range: 2..254.
 */
FakeTerminal::FakeTerminal(uint8_t const _width, uint8_t const _height)
: ITerminal()
, width(_width)
, height(_height)
, outputMutex()
, lines()
, linesDroppedOut()
, recordLinesDroppedOut(false)
, cursor_x(0)
, cursor_y(0)
, throwRequested_out(false)
, inputMutex()
, inputBuffer()
, inputBufferNotEmptyCV()
, readingThreadBlocked(false)
, readingThreadBlockedCV()
, throwRequested_in(false)
, throwRequested_flush(false)
{
  if ((width < 2U) || (width > 254U))
    throw std::invalid_argument("FakeTerminal::FakeTerminal: _width out of range");

  if ((height < 2U) || (height > 254U))
    throw std::invalid_argument("FakeTerminal::FakeTerminal: _height out of range");

  lines.resize(height);
}

/**
 * \brief Enables recording of lines that drop out at the top of the terminal's virtual screen
 *        when new lines are printed at the bottom.
 *
 * This method does nothing if recording is already enabled.
 *
 * Recording can be enabled at any time.
 *
 * \warning   Anything ever printed to the terminal will be recorded. This option may consume large amounts of
 *            memory if lots of text is printed to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::EnableRecordingOfDroppedOutLines(void)
{
  gpcc::osal::MutexLocker ml(outputMutex);
  recordLinesDroppedOut = true;
}

/**
 * \brief Requests the fake terminal to intentionally throw an exception
 * when [ITerminal::Write()](@ref gpcc::cli::ITerminal::Write()) is executed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::RequestThrowUponWrite(void)
{
  gpcc::osal::MutexLocker ml(outputMutex);
  throwRequested_out = true;
}

/**
 * \brief Requests the fake terminal to intentionally throw an exception
 * when [ITerminal::Read()](@ref gpcc::cli::ITerminal::Read()) is executed.
 *
 * If a thread is currently blocked in Read(), then the thread
 * will be woken up and the exception will be thrown.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::RequestThrowUponRead(void)
{
  gpcc::osal::MutexLocker ml(inputMutex);
  inputBufferNotEmptyCV.Signal();
  throwRequested_in = true;
}

/**
 * \brief Requests the fake terminal to intentionally throw an exception
 * when [ITerminal::Flush()](@ref gpcc::cli::ITerminal::Flush()) is executed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::RequestThrowUponFlush(void)
{
  gpcc::osal::MutexLocker ml(inputMutex);
  throwRequested_flush = true;
}

/**
 * \brief Writes input from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param input
 * Null-terminated string containing the input that shall be entered into the terminal.\n
 * Note: All input is entered in zero time and will become readable via @ref ITerminal immediately.
 */
void FakeTerminal::Input(char const * input)
{
  WriteToInputBuffer(input);
}

/**
 * \brief Sends a POS1-keystroke from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::Input_POS1(void)
{
  const char seq[] = {0x1B, '[', '1', '~', 0x0};
  WriteToInputBuffer(seq);
}

/**
 * \brief Sends an END-keystroke from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::Input_END(void)
{
  const char seq[] = {0x1B, '[', '4', '~', 0x0};
  WriteToInputBuffer(seq);
}

/**
 * \brief Sends an ENTER-keystroke from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::Input_ENTER(void)
{
  const char seq[] = {0x0D, 0x0};
  WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more DEL-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_DEL(size_t times)
{
  const char seq[] = {0x1B, '[', '3', '~', 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more BACKSPACE-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_Backspace(size_t times)
{
  const char seq[] = {0x7F, 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more TAB-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_TAB(size_t times)
{
  const char seq[] = {0x09, 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more ARROW-LEFT-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_ArrowLeft(size_t times)
{
  const char seq[] = {0x1B, '[', 'D', 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more ARROW-RIGHT-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_ArrowRight(size_t times)
{
  const char seq[] = {0x1B, '[', 'C', 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more ARROW-UP-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_ArrowUp(size_t times)
{
  const char seq[] = {0x1B, '[', 'A', 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends one or more ARROW-DOWN-keystroke(s) from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined number of keystrokes (0..`times`) may have been written to the input buffer
 *
 * - - -
 *
 * \param times
 * Number of keystrokes to be generated. Zero is allowed.
 */
void FakeTerminal::Input_ArrowDown(size_t times)
{
  const char seq[] = {0x1B, '[', 'B', 0x0};
  while (times-- != 0)
    WriteToInputBuffer(seq);
}

/**
 * \brief Sends a CTRL+C-keystroke from an "imaginary user" to the terminal.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::Input_CtrlC(void)
{
  const char seq[] = {0x03, 0x0};
  WriteToInputBuffer(seq);
}

/**
 * \brief Blocks the calling thread, until all input has been processed and until
 * a thread waiting for input has been blocked in @ref gpcc::cli::ITerminal::Read().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 */
void FakeTerminal::WaitForInputProcessed(void)
{
  gpcc::osal::MutexLocker ml(inputMutex);
  while ((!readingThreadBlocked) || (!inputBuffer.empty()) || (throwRequested_in))
    readingThreadBlockedCV.Wait(inputMutex);
}

/**
 * \brief Compares the content of the terminal's virtual screen against an expectation/reference.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param ref
 * Pointer to an array of pointers to null-terminated strings containing the expected/reference
 * terminal content. The number of strings __must match__ the number of lines of the terminal which
 * has been configured at the constructor when the @ref FakeTerminal was created.\n
 * Example for a fake terminal with 8 lines:
 * ~~~{.cpp}
 * char const * ref[8] =
 * {
 *  "Hello World!",
 *  "",
 *  "",
 *  "",
 *  "",
 *  "",
 *  "",
 *  ""
 * };
 * ~~~
 * Note:\n
 * Even white-spaces are compared, though they are invisible. This is especially true for
 * _trailing_ white-spaces.
 *
 * \return
 * Result of the comparison:\n
 * true  = match\n
 * false = mismatch (at least one character is different)\n
 * @ref PrintToStdOut() may be helpful for debugging, if this returns false.
 */
bool FakeTerminal::Compare(char const * ref[]) const
{
  // Compares the terminal's content against a reference.
  // Result: true = equal, false = not equal

  gpcc::osal::MutexLocker outputMutexLocker(outputMutex);

  uint_fast8_t i = 0;
  for (auto & line : lines)
  {
    if (line.compare(ref[i++]) != 0)
      return false;
  }
  return true;
}

/**
 * \brief Compares the position of the terminal's imaginary cursor to an expectation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param expected_cursor_x
 * Expected value for the cursor's x-component.
 * \param expected_cursor_y
 * Expected value for the cursor's y-component.
 * \return
 * Result of the comparison:\n
 * true  = match\n
 * false = mismatch
 */
bool FakeTerminal::Compare(uint8_t const expected_cursor_x, uint8_t const expected_cursor_y) const
{
  gpcc::osal::MutexLocker outputMutexLocker(outputMutex);
  return ((cursor_x == expected_cursor_x) && (cursor_y == expected_cursor_y));
}

/**
 * \brief Retrieves an std::string containing the content of the fake terminal's virtual screen.
 *
 * This method is intended to be used to perform wildcard based test/comparison or regular expression based
 * test/comparison on the content of the fake terminal's screen.
 *
 * See @ref gpcc::string::TestSimplePatternMatch(std::string const & s, char const * pCharSeq, bool const caseSensitive)
 * for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * String containing the current content of the terminal's screen.\n
 * The single lines of the screen are concatenated together. Each line ends with a '\\n'.\n
 * There are no additional space-characters, except they have been explicitly printed to the terminal.\n
 * Example output for a fake terminal with 4 lines if nothing has ever printed to the terminal:\n
 * "\\n\\n\\n\\n".
 */
std::string FakeTerminal::GetScreenContent(void) const
{
  gpcc::osal::MutexLocker outputMutexLocker(outputMutex);

  std::string screen;

  for (auto const & line : lines)
  {
    screen += line;
    screen += '\n';
  }

  return screen;
}

/**
 * \brief Retrieves an std::string containing the content of the fake terminal's virtual screen plus
 *        any lines that have dropped out at the top of the terminal's screen due to new lines printed
 *        at the bottom of the screen.
 *
 * This method is intended to be used to perform wildcard based test/comparison or regular expression based
 * test/comparison on _everything_ ever printed to the fake terminal's screen.
 *
 * Note:\n
 * Recording of dropped out lines must have been started via @ref EnableRecordingOfDroppedOutLines(). Any lines
 * which have dropped out of the terminal's screen before recording has been enabled are not recorded.
 *
 * See @ref gpcc::string::TestSimplePatternMatch(std::string const & s, char const * pCharSeq, bool const caseSensitive)
 * for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * String containing all lines that have dropped out at the top of the terminal's screen plus the current
 * content of the terminal's screen.\n
 * All lines are concatenated together. Each line ends with a '\\n'.\n
 * There are no additional space-characters, except they have been explicitly printed to the terminal.\n
 * Example output for a fake terminal with 4 lines if nothing has ever printed to the terminal:\n
 * "\\n\\n\\n\\n".
 */
std::string FakeTerminal::GetDroppedOutLinesPlusCurrentScreenContent(void) const
{
  gpcc::osal::MutexLocker outputMutexLocker(outputMutex);

  if (!recordLinesDroppedOut)
  {
    throw std::logic_error("FakeTerminal::GetDroppedOutLinesPlusCurrentScreenContent: Recording of lines dropped out " \
                           "at the top of the terminal's virtual screen is not enabled.");
  }

  std::string output;

  for (auto const & line : linesDroppedOut)
  {
    output += line;
    output += '\n';
  }

  for (auto const & line : lines)
  {
    output += line;
    output += '\n';
  }

  return output;
}

/**
 * \brief Prints the content of the terminal's virtual screen to stdout.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Output to stdout may be incomplete
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Output to stdout may be incomplete
 */
void FakeTerminal::PrintToStdOut(void)
{
  gpcc::osal::MutexLocker outputMutexLocker(outputMutex);

  uint_fast8_t i = 0;
  for (auto & line : lines)
  {
    std::cout << std::right << std::setw(2) << static_cast<unsigned int>(i) << " " << line << std::endl;
    i++;
  }
  std::cout << "Cursor: (" << static_cast<unsigned int>(cursor_x) << "," << static_cast<unsigned int>(cursor_y) << ")" << std::endl;
}

/**
 * \brief Writes a null-terminated byte/character-sequence to the fake terminal's input buffer.
 *
 * The input buffer's content can be read via @ref gpcc::cli::ITerminal::Read().
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread-cancellation-safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param p
 * Pointer to a null-terminated byte- or character-sequence that shall be written into the
 * fake terminal's input buffer. A deep copy is created.
 */
void FakeTerminal::WriteToInputBuffer(char const * p)
{
  gpcc::osal::MutexLocker ml(inputMutex);

  if (inputBuffer.empty())
    inputBufferNotEmptyCV.Signal();

  char c;
  size_t nbOfAddedChars = 0;

  ON_SCOPE_EXIT(restoreInputBuffer)
  {
    inputBuffer.resize(inputBuffer.size() - nbOfAddedChars);
  };

  while ((c = *p++) != 0x00)
  {
    inputBuffer.push_back(c);
    nbOfAddedChars++;
  }

  ON_SCOPE_EXIT_DISMISS(restoreInputBuffer);
}

// <-- gpcc::cli::ITerminal
/// \copydoc gpcc::cli::ITerminal::Read
size_t FakeTerminal::Read(char * pBuffer, size_t bufferSize, uint16_t timeout_ms)
{
  try
  {
    // check input parameters
    if (pBuffer == nullptr)
      throw std::invalid_argument("FakeTerminal::Read: !pBuffer");

    if (bufferSize == 0)
      throw std::invalid_argument("FakeTerminal::Read: bufferSize == 0");

    using namespace gpcc::time;
    gpcc::osal::MutexLocker inputMutexLocker(inputMutex);

    if (timeout_ms != 0)
    {
      readingThreadBlocked = true;
      ON_SCOPE_EXIT() { readingThreadBlocked = false; };

      // signal if input buffer is empty
      if ((inputBuffer.empty()) && (!throwRequested_in))
        readingThreadBlockedCV.Signal();

      // wait for data or timeout
      bool timeout = false;
      TimePoint const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(timeout_ms);
      while ((inputBuffer.empty()) && (!throwRequested_in) && (!timeout))
        timeout = inputBufferNotEmptyCV.TimeLimitedWait(inputMutex, absTimeout);
    }

    if (throwRequested_in)
    {
      throwRequested_in = false;
      throw IntentionallyThrownException();
    }

    // transfer data from inputBuffer into pBuffer
    size_t retVal = std::min(bufferSize, inputBuffer.size());
    if (retVal != 0)
    {
      memcpy(pBuffer, inputBuffer.data(), retVal);
      inputBuffer.erase(inputBuffer.begin(), inputBuffer.begin() + retVal);
    }

    return retVal;
  }
  catch (IntentionallyThrownException const &)
  {
    throw;
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("FakeTerminal::Read: Exception thrown: ", e);
  }
}

/// \copydoc gpcc::cli::ITerminal::Flush
void FakeTerminal::Flush(void)
{
  gpcc::osal::MutexLocker inputMutexLocker(inputMutex);

  if (throwRequested_flush)
  {
    throwRequested_flush = false;
    throw IntentionallyThrownException();
  }

  if ((readingThreadBlocked) && (!throwRequested_in))
    readingThreadBlockedCV.Signal();

  inputBuffer.clear();
}

/// \copydoc gpcc::cli::ITerminal::Write
void FakeTerminal::Write(char const * pBuffer, size_t s)
{
  // Note: terminal control sequences (e.g. cursor move) cannot be splitted among multiple calls to this.

  try
  {
    // empty string?
    if (s == 0)
      return;

    // no buffer?
    if (pBuffer == nullptr)
      throw std::invalid_argument("FakeTerminal::Write: !pBuffer");

    // convert to std::string
    std::string str(pBuffer, s);
    if (str.find(static_cast<char>(0x00)) != std::string::npos)
      throw std::invalid_argument("FakeTerminal::WriteToTerminal: length mismatch pBuffer <-> s");

    gpcc::osal::MutexLocker outputMutexLocker(outputMutex);

    if (throwRequested_out)
    {
      throwRequested_out = false;
      throw IntentionallyThrownException();
    }

    // terminal control or plain text?
    if (str.front() == '\x1B')
    {
      // (terminal control)

      // terminal control needs at least 4 characters
      if (str.length() < 4U)
        throw std::runtime_error("FakeTerminal::WriteToTerminal: Bad command: too short");

      if (str[1U] != '[')
        throw std::runtime_error("FakeTerminal::WriteToTerminal: Bad command: '[' missing");

      // extract a string containing the number and extract the control character
      std::string const nb = str.substr(2U, str.length() - 3U);
      char const c = str.back();

      // convert number string "nb" to int32_t "n"
      uint32_t const u32 = gpcc::string::DecimalToU32(nb);
      if (u32 > 9999U)
        throw std::runtime_error("FakeTerminal::WriteToTerminal: Bad command: Bad number");
      int32_t n = u32;

      // examine control character
      if ((c == 'D') || (c == 'C'))
      {
        // (move cursor)

        // D = move "n" chars to the left, C = move "n" chars to the right
        if (c == 'D')
          n = -n;

        // move cursor and be aware of boundaries
        int32_t newCursorX = cursor_x + n;
        if (newCursorX < 0)
          cursor_x = 0;
        else if (newCursorX >= width)
          throw std::runtime_error("FakeTerminal::WriteToTerminal: UUT attempted to move cursor beyond width of terminal");
        else
          cursor_x = newCursorX;
      }
      else if (c == 'P')
      {
        // (delete n characters at current cursor position)

        std::string & termLine = lines[cursor_y];
        size_t const termLineLength = termLine.length();
        if (cursor_x < termLineLength)
        {
          size_t const maxToDelete = termLineLength - cursor_x;
          if (static_cast<uint32_t>(n) > maxToDelete)
            n = maxToDelete;
          termLine.erase(cursor_x, n);
        }
      }
      else
        throw std::runtime_error("FakeTerminal::WriteToTerminal: Bad command: Bad D/C/P");
    } // if (str.front() == '\x1B')
    else
    {
      // (plain text)

      // Process input character by character. Not the most performant approach, but simple.
      for (auto const c: str)
      {
        if (c == '\n')
        {
          cursor_x = 0;
          cursor_y++;
          if (cursor_y == height)
          {
            if (recordLinesDroppedOut)
              linesDroppedOut.push_back(std::move(lines[0]));

            cursor_y = height - 1U;
            for (uint_fast8_t i = 0; i < height-1U; i++)
              lines[i] = std::move(lines[i + 1U]);
            lines[height - 1U].clear();
          }
        }
        else
        {
          if (cursor_x >= width-1U)
            throw std::runtime_error("FakeTerminal::WriteToTerminal: UUT attempted write to last character of line");
          std::string & termLine = lines[cursor_y];
          if (termLine.length() <= cursor_x)
            termLine.resize(cursor_x + 1U, ' ');

          termLine[cursor_x] = c;
          cursor_x++;
        }
      } // for (auto const c: str)
    } // if (str.front() == '\x1B')... else...
  }
  catch (IntentionallyThrownException const &)
  {
    throw;
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("FakeTerminal::Write: Exception thrown.\n" \
                      "Did you build GPCC with \"-DGPCC_CLI_NO_FONT_STYLES\" ?\n" \
                      "Exception: ", e);
  }
}
// --> gpcc::cli::ITerminal

} // namespace cli
} // namespace gpcc_tests
