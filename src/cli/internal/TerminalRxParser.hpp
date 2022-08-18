/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TERMINALRXPARSER_HPP_201710041404
#define TERMINALRXPARSER_HPP_201710041404

#include <cstdint>

namespace gpcc     {
namespace cli      {
namespace internal {

/**
 * \ingroup GPCC_CLI_INTERNALS
 * \brief Parser for data received from a terminal using VT100-encoding.
 *
 * This class is a helper for class @ref CLI.
 *
 * Class @ref TerminalRxParser allows to separate control-data transmitted by a VT100-compatible
 * terminal from normal character-data. Beside separation, the control-data is also decoded. Class
 * @ref TerminalRxParser also allows to remove non-printable characters from the "normal"
 * character-data.
 *
 * # Usage
 * Received data shall be passed to the parser byte by byte. This is accomplished via
 * method @ref Input().
 *
 * @ref Input() accumulates the received data bytes in a buffer (attribute `inBuffer`) until it is
 * clear if the data in the buffer is just plain data or if the data in the buffer is special
 * control-data.\n
 * The return value of @ref Input() (@ref Result) indicates the current status of the parser's
 * buffer. Summarized, the following scenarios may occur:\n
 * a) the buffer contains a special code sequence\n
 * b) the buffer contains plain data\n
 * c) the buffer may contain a special code sequence, but more data is needed
 *
 * If the status value returned by @ref Input() indicates that no more data is needed, then the
 * buffer either contains a special code sequence or plain data. In both cases, some actions must
 * be taken to drain the buffer before more data is passed to the parser via @ref Input():
 * - @ref Clear()\n
 *   The buffer's content is dropped. This is usually done, if the return value of @ref Input()
 *   indicates that the buffer contains a special code sequence.
 * - @ref Output()\n
 *   The buffer's content can be read. This is usually done, if the return value of @ref Input()
 *   indicates that the buffer contains plain data.
 *
 * If plain data has been detected and there is more than one byte of data in the buffer,
 * then the last byte of data should not be processed by the user of class @ref TerminalRxParser.
 * Instead, the user should pass the last byte to @ref Input() again, because it might be
 * the beginning of a new control-data sequence.
 *
 * The number of bytes currently stored in the parser's buffer can be retrieved via @ref GetLevel().
 *
 * Before calling @ref Output(), @ref RemoveNonPrintableCharacters() may be invoked to remove
 * non-printable characters from the buffer.
 *
 * After invoking @ref Clear() or @ref Output(), new data can be passed to the parser via @ref Input().
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class TerminalRxParser final
{
  public:
    /// Enumeration of return values of method @ref Input().
    enum class Result
    {
      Backspace,
      Tab,
      LF,
      CR,
      ArrowLeft,
      ArrowRight,
      ArrowUp,
      ArrowDown,
      Pos1,
      END,
      DEL,
      Insert,
      PgUp,
      PgDn,
      ETX,          ///<CTRL+C
      // -- codes for further keys shall be added here --
      NoCommand,    ///<No command, its normal data.
      NeedMoreData  ///<Could be a command. Need more data.
    };

    /// Size of the parser's buffer.
    /** This must meet the length of the largest special code sequence defined in table `codeSeqTable`
        (-> TerminalRxParser.cpp) inclusive the null-terminator. */
    static uint_fast8_t const inBufferSize = 5U;


    TerminalRxParser(void) noexcept;
    TerminalRxParser(TerminalRxParser const &) noexcept = default;
    TerminalRxParser(TerminalRxParser &&) noexcept = default;
    ~TerminalRxParser(void) = default;

    TerminalRxParser& operator=(TerminalRxParser const &) noexcept = default;
    TerminalRxParser& operator=(TerminalRxParser &&) noexcept = default;

    void Clear(void) noexcept;
    Result Input(char const data);
    void RemoveNonPrintableCharacters(void) noexcept;
    char const * Output(uint_fast8_t const truncateAt = inBufferSize) noexcept;
    uint_fast8_t GetLevel(void) const noexcept;

  private:
    /// Parser's buffer for incoming data.
    char inBuffer[inBufferSize];

    /// Number of bytes inside @ref inBuffer.
    uint_fast8_t nbOfBytesInBuffer;
};

} // namespace internal
} // namespace cli
} // namespace gpcc

#endif // TERMINALRXPARSER_HPP_201710041404
