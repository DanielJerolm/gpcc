/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TerminalRxParser.hpp"
#include <gpcc/string/tools.hpp>
#include <stdexcept>

namespace gpcc     {
namespace cli      {
namespace internal {

namespace {

// One entry in the table of special code sequences (codeSeqTable).
struct CodeSeqTableEntry
{
  TerminalRxParser::Result const key;
  char const codeSeq[TerminalRxParser::inBufferSize];
};

} // namespace

// Table with special codes send by a terminal if a special key has been pressed.
// Each sequence must be terminated with a null-terminator.
static CodeSeqTableEntry const codeSeqTable[] =
{
  { TerminalRxParser::Result::Backspace,  {0x7F, 0x00}},
  { TerminalRxParser::Result::Tab,        {0x09, 0x00}},
  { TerminalRxParser::Result::LF,         {0x0A, 0x00}},
  { TerminalRxParser::Result::CR,         {0x0D, 0x00}},
  { TerminalRxParser::Result::ArrowLeft,  {0x1B, '[',  'D',  0x00}},
  { TerminalRxParser::Result::ArrowRight, {0x1B, '[',  'C',  0x00}},
  { TerminalRxParser::Result::ArrowUp,    {0x1B, '[',  'A',  0x00}},
  { TerminalRxParser::Result::ArrowDown,  {0x1B, '[',  'B',  0x00}},
  { TerminalRxParser::Result::Pos1,       {0x1B, '[',  'H',  0x00}},
  { TerminalRxParser::Result::Pos1,       {0x1B, '[',  '1',  '~',  0x00}},
  { TerminalRxParser::Result::END,        {0x1B, '[',  'F',  0x00}},
  { TerminalRxParser::Result::END,        {0x1B, '[',  '4',  '~',  0x00}},
  { TerminalRxParser::Result::DEL,        {0x1B, '[',  '3',  '~',  0x00}},
  { TerminalRxParser::Result::Insert,     {0x1B, '[',  '2',  '~',  0x00}},
  { TerminalRxParser::Result::PgUp,       {0x1B, '[',  '5',  '~',  0x00}},
  { TerminalRxParser::Result::PgDn,       {0x1B, '[',  '6',  '~',  0x00}},
  { TerminalRxParser::Result::ETX,        {0x03, 0x00}}
};

// Number of entries in codeSeqTable.
static size_t const codeSeqTableSize = sizeof(codeSeqTable) / sizeof(CodeSeqTableEntry);

#ifndef __DOXYGEN__
uint_fast8_t const TerminalRxParser::inBufferSize;
#endif

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TerminalRxParser::TerminalRxParser(void) noexcept
: nbOfBytesInBuffer(0)
{
  inBuffer[0] = 0x00;
}

/**
 * \brief Clears the parser's internal buffer.
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
void TerminalRxParser::Clear(void) noexcept
{
  nbOfBytesInBuffer = 0;
}

/**
 * \brief Provides one byte of received data to the parser for processing.
 *
 * Note: If a pointer has been retrieved via @ref Output() before, then the pointer
 * becomes invalid and must not be dereferenced any more after calling this.
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
 * \param data
 * Received byte of data that shall be processed by the parser.\n
 * The data is inserted into the parser's internal buffer and examined.
 * \return
 * A value from the @ref Result enumeration indicating if a special key code has been detected
 * in the parser's internal buffer.\n
 * _Special values:_
 * - @ref Result::NoCommand \n
 *   Data contained in the buffer is not a command.
 * - @ref Result::NeedMoreData \n
 *   Could be a command, but the byte sequence is not yet complete.\n
 *   More data must be provided to the parser via this method.
 *
 * If something else but @ref Result::NeedMoreData is returned, then the data must be read from
 * the parser's internal buffer via @ref Output(). Alternatively the parser's buffer could also
 * be cleared via @ref Clear().
 */
TerminalRxParser::Result TerminalRxParser::Input(char const data)
{
  // Check for buffer-full condition.
  // Note that the last byte of the buffer is reserved for the null-terminator
  // setup by Output(...).
  if (nbOfBytesInBuffer >= inBufferSize - 1U)
    throw std::runtime_error("TerminalRxParser::Input: Buffer full");

  // append data to parser's buffer
  inBuffer[nbOfBytesInBuffer++] = data;

  for (size_t i = 0; i < codeSeqTableSize; i++)
  {
    char const * pBuf = inBuffer;
    char const * pTbl = codeSeqTable[i].codeSeq;

    for (uint_fast8_t j = 0; j < nbOfBytesInBuffer; j++)
    {
      char const cBuf = *pBuf++;
      char const cTbl = *pTbl++;

      // mismatch?
      if (cBuf != cTbl)
        break;

      // last character of buffer?
      if (j == nbOfBytesInBuffer - 1U)
      {
        // end of command sequence also reached? => match
        if (*pTbl == 0U)
          return codeSeqTable[i].key;
        //...else this could be a match, but we need more data
        else
          return Result::NeedMoreData;
      } // if (j == nbOfBytesInBuffer - 1U)
    } // for (uint_fast8_t j = 0; j < nbOfBytesInBuffer; j++)
  } // for (size_t i = 0; i < codeSeqTableSize; i++)

  return Result::NoCommand;
}

/**
 * \brief Removes all non-printable characters from the internal buffer.
 *
 * This method is intended to be invoked before @ref Output() is invoked. @ref Output()
 * is intended to be invoked after @ref Input() has returned @ref Result::NoCommand. This is
 * the case when the internal buffer of the parser contains plain data and not a special code
 * sequence. @ref Output() is used to retrieve the plain data in that case.
 *
 * This method will remove all non-printable characters from the internal buffer.
 *
 * _If the buffer contains more than one character, then the last character in the buffer will_
 * _not be removed, even if it is not printable._
 *
 * This is because the last character could be the beginning of a new special code sequence.
 * The user of class @ref TerminalRxParser shall pass this last character to @ref Input()
 * again.
 *
 * Printable characters remain inside the buffer. The remove operation reduces the number of
 * characters in the buffer.
 *
 * Non-printable characters are the opposite of printable ASCII characters.
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
void TerminalRxParser::RemoveNonPrintableCharacters(void) noexcept
{
  auto n = nbOfBytesInBuffer;
  bool const keepLastChar = (n > 1U);

  char* rdPtr = inBuffer;
  char* wrPtr = inBuffer;

  while (n != 0)
  {
    char const c = *rdPtr;

    if ((gpcc::string::IsPrintableASCII(c)) || ((n == 1U) && (keepLastChar)))
    {
      // (keep the character)

      if (rdPtr != wrPtr)
        *wrPtr = c;
      wrPtr++;
    }
    else
    {
      // (drop the character)

      nbOfBytesInBuffer--;
    }

    rdPtr++;
    n--;
  }
}

/**
 * \brief Retrieves a pointer to the parser's internal buffer for reading and resets the parser
 * in order to prepare it for processing new data.
 *
 * This must be called after @ref Input() has returned something else but @ref Result::NeedMoreData
 * in order to remove the data from the parser's internal buffer. Alternatively @ref Clear() could
 * also be called. After calling this, new data can be passed to the parser via @ref Input().
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
 *
 * - - -
 *
 * \param truncateAt
 * If more than `truncateAt` characters are contained in the parser's internal buffer, then the
 * buffer is truncated at the position given by `truncateAt`. Data at and beyond the index given
 * by `truncateAt` is discarded. If this is zero, than all data is discarded.
 * \return
 * Pointer to an internal buffer containing the received data. A null-terminator marks the end of
 * the data.\n
 * The referenced data is valid until the next call to @ref Input().
 */
char const * TerminalRxParser::Output(uint_fast8_t const truncateAt) noexcept
{
  if (nbOfBytesInBuffer > truncateAt)
    nbOfBytesInBuffer = truncateAt;

  inBuffer[nbOfBytesInBuffer] = 0;
  nbOfBytesInBuffer = 0;
  return inBuffer;
}

/**
 * \brief Retrieves the number of bytes inside the parser's internal buffer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * Number of bytes inside the parser's buffer.\n
 * Note that any call to @ref Clear() or @ref Output() will clear the buffer and this method
 * will return zero.
 */
uint_fast8_t TerminalRxParser::GetLevel(void) const noexcept
{
  return nbOfBytesInBuffer;
}

} // namespace internal
} // namespace cli
} // namespace gpcc
