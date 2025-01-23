/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#include <gpcc/string/BinaryDumper.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <gpcc/string/tools.hpp>
#include <gpcc/compiler/builtins.hpp>
#include <stdexcept>

namespace gpcc   {
namespace string {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pData
 * Pointer to the data that shall be dumped.\n
 * This must be aligned to @p wordSize. \n
 * `nullptr` is allowed, if @p nBytes is zero.
 *
 * \param nBytes
 * Size of the data referenced by @p pData in bytes.\n
 * Zero is allowed. This must be an integer multiple of @p wordSize.
 *
 * \param address
 * Address associated with the first word in the buffer referenced by @p pData. \n
 * This is used for printing only. No memory access to the given address will occur.
 *
 * \param wordSize
 * Word size (in byte) that shall be used to interpret the data referenced by @p pData. \n
 * Allowed values: 1, 2, 4, or 8.
 */
BinaryDumper::BinaryDumper(void const * const pData,
                           size_t const nBytes,
                           uintptr_t const address,
                           uint8_t const wordSize)
: address_(address)
, pData_(pData)
, nWords_(0U)
, wordSize_(wordSize)
, wordsPerLine_(0U)
, addressFieldWidth_(0U)
, maxLineLength_(0U)
{
  // ---------------------------------------------------
  // Check parameters and initialize class' members
  // ---------------------------------------------------
  if ((wordSize_ != 1U) && (wordSize_ != 2U) && (wordSize_ != 4U) && (wordSize_ != 8U))
    throw std::invalid_argument("wordSize");

  wordsPerLine_ = bytesPerLine_ / wordSize_;

  if ((pData_ == nullptr) && (nBytes != 0))
    throw std::invalid_argument("!pData");

  if ((reinterpret_cast<uintptr_t>(pData_) % wordSize_) != 0U)
    throw std::invalid_argument("pData <-> wordSize");

  if ((nBytes % wordSize_) != 0U)
    throw std::invalid_argument("nBytes <-> wordSize");

  nWords_ = nBytes / wordSize_;

  // ---------------------------------------------------
  // Calculate required field width for the address
  // ---------------------------------------------------
  // determine the address printed in the last line of the dump
  uint_fast8_t const bytesInIncompleteLastLine = (nBytes % bytesPerLine_);
  bool const lastLineFull = (bytesInIncompleteLastLine == 0U);

  size_t addressSummand = nBytes - bytesInIncompleteLastLine;
  if ((addressSummand != 0U) && (lastLineFull))
    addressSummand -= bytesPerLine_;

  uintptr_t lastPrintedAddr;
  if (gpcc::compiler::OverflowAwareAdd(address_, addressSummand, &lastPrintedAddr))
    throw std::invalid_argument("address overflow");

  // determine the number of bits required to represent the address printed in the last line of the dump
  uint_fast8_t nbOfRequiredBits = (sizeof(uintptr_t) * 8U) - gpcc::compiler::CountLeadingZeros(lastPrintedAddr);
  if (nbOfRequiredBits < 8U)
    nbOfRequiredBits = 8U;

  // convert to bytes and round up to next byte boundary
  uint_fast8_t const nbOfRequiredBytes = (nbOfRequiredBits + 7U) / 8U;

  // determine the required number of hex digits
  uint_fast8_t const nbOfRequiredHexDigits = nbOfRequiredBytes * 2U;

  // determine the field width required to print the address in front of each line
  addressFieldWidth_ = 2U + nbOfRequiredHexDigits + 2U;
  //                   |    |                       |
  //                   0x   hex digits              trailing ": "

  // the headline ("Address ") requires a minimum width of 8
  if (addressFieldWidth_ < 8U)
    addressFieldWidth_ = 8U;

  // ---------------------------------------------------
  // Calculate the maximum length of each generated line
  // ---------------------------------------------------
  //  8 bit: Address XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX XX | 0123456789ABCDEF
  // 16 bit: Address XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX | 0123456789ABCDEF
  // 32 bit: Address XXXXXXXX XXXXXXXX XXXXXXXX XXXXXXXX | 0123456789ABCDEF
  // 64 bit: Address XXXXXXXXXXXXXXXX XXXXXXXXXXXXXXXX | 0123456789ABCDEF
  maxLineLength_ = static_cast<uint8_t>(addressFieldWidth_ + (wordsPerLine_ * ((wordSize_ * 2U) + 1U)) + 18U);
}

/**
 * \brief Retrieves a headline for the dump.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * String containing the headline for the dump.
 */
std::string BinaryDumper::GetHeadLine(void) const
{
  StringComposer sc(maxLineLength_);
  sc << StringComposer::AlignLeft << StringComposer::Width(addressFieldWidth_) << "Address ";

  sc << StringComposer::BaseHex << StringComposer::Uppercase;
  for (uint_fast8_t i = 0U; i < wordsPerLine_; ++i)
  {
    sc << '+' << StringComposer::Width(wordSize_ * 2U) << static_cast<unsigned int>(i * wordSize_);
  }

  sc << "| 0123456789ABCDEF";

  return sc.Get();
}

/**
 * \brief Retrieves one line of dumped data.
 *
 * If all data has been dumped (or if there was no data right from the begin), then the line will only contain the
 * address of the next word that would be dumped if there had been any data.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - The object is left in a valid, but undefined state.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * String containing one line of dumped data.
 */
std::string BinaryDumper::GetLine(void)
{
  StringComposer sc(maxLineLength_);
  sc << StringComposer::AlignRightPadZero << StringComposer::BaseHex << StringComposer::Uppercase;

  // dump the address
  sc << "0x" << StringComposer::Width(addressFieldWidth_ - 4U) << address_ << ": ";

  // dump the data, if any
  if (nWords_ != 0U)
  {
    uint_fast8_t const wordsInThisLine = (nWords_ > wordsPerLine_) ? wordsPerLine_ : nWords_;
    uint_fast8_t const bytesInThisLine = wordsInThisLine * wordSize_;

    // Note: AppendHexDump() will pad the line with whitespaces if wordsInThisLine is less than wordsPerLine_.
    AppendHexDump(sc, reinterpret_cast<uint8_t const*>(pData_), wordsInThisLine);
    sc << "| ";
    AppendASCIIDump(sc, reinterpret_cast<char const *>(pData_), bytesInThisLine);

    // update pointer, address, and remaining words
    pData_ = reinterpret_cast<uint8_t const*>(pData_) + bytesInThisLine;
    nWords_ -= wordsInThisLine;
    address_ += bytesPerLine_;
  }

  return sc.Get();
}

/**
 * \brief Queries, if all data has been dumped.
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
 * \retval true   All data has been dumped or there was no data right from the begin.
 * \retval false  There is at least one word left to be dumped.
 */
bool BinaryDumper::IsAllDataDumped(void) const noexcept
{
  return (nWords_ == 0U);
}

/**
 * \brief Dumps 16 bytes of data into an @ref StringComposer using hexadecimal format.
 *
 * The dump always contains a trailing white-space.
 *
 * The dump will be padded with white-spaces if @p nbOfWords is less than @ref wordsPerLine_. The resulting length of
 * the dump is always the same as if @ref wordsPerLine_ words of data had been dumped.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * The data will be dumped into the referenced @ref StringComposer. \n
 * The @ref StringComposer must be configured as follows:\n
 * AlignRightPadZero, BaseHex, UpperCase
 *
 * \param pData
 * Pointer to the data that shall be dumped.\n
 * This a pointer to `uint8_t`, but the data will be read and interpreted according to @ref wordSize_.
 *
 * \param nbOfWords
 * Number of words that shall be read from @p pData and dumped to @p sc.
 */
void BinaryDumper::AppendHexDump(StringComposer & sc, uint8_t const * pData, uint_fast8_t const nbOfWords) const
{
  for (uint_fast8_t i = 0U; i < wordsPerLine_; ++i)
  {
    sc << StringComposer::Width(wordSize_ * 2U);

    if (i < nbOfWords)
    {
      switch (wordSize_)
      {
        case 1U:
        {
          uint16_t const v = *reinterpret_cast<uint8_t const*>(pData);
          sc << v;
          break;
        }

        case 2U:
        {
          uint16_t const v = *reinterpret_cast<uint16_t const*>(pData);
          sc << v;
          break;
        }

        case 4U:
        {
          uint32_t const v = *reinterpret_cast<uint32_t const*>(pData);
          sc << v;
          break;
        }

        case 8U:
        {
          uint64_t const v = *reinterpret_cast<uint64_t const*>(pData);
          sc << v;
          break;
        }
      }

      pData += wordSize_;
    }
    else
    {
      sc << ' '; // note: Will be padded with white spaces until "wordSize_ * 2U"
    }

    sc << ' ';
  }
}

/**
 * \brief Dumps 16 bytes of data into an @ref StringComposer using ASCII characters.
 *
 * Non-printable characters in the data will be substituted with "." characters.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * The data will be dumped into the referenced @ref StringComposer.
 *
 * \param pData
 * Pointer to the data that shall be dumped.
 *
 * \param nbOfBytes
 * Number of bytes that shall be read from @p pData and dumped to @p sc.
 */
void BinaryDumper::AppendASCIIDump(StringComposer & sc, char const * pData, uint_fast8_t nbOfBytes) const
{
  while (nbOfBytes != 0U)
  {
    char c = *pData;
    if (!IsPrintableASCII(c))
      c = '.';
    sc << c;

    ++pData;
    --nbOfBytes;
  }
}

} // namespace string
} // namespace gpcc
