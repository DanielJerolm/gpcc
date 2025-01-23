/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 Daniel Jerolm
*/

#ifndef BINARYDUMPER_HPP_202501082012
#define BINARYDUMPER_HPP_202501082012

#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc   {
namespace string {

class StringComposer;


/**
 * \ingroup GPCC_STRING
 * \brief Builds a human-readable dump of binary data line-by-line.
 *
 * # Output examples
 * Output example for 8 bit word size:
 * ~~~{.txt}
 * Address +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F | 0123456789ABCDEF
 * 0x0250: 00 41 42 FE DC 99 AB 21 21 21 21 00 00 00 00 00 | .AB....!!!!.....
 * 0x0260: 42 21 00 00 00 00 00 00                         | B!......
 * ~~~
 *
 * Output example for 32 bit word size:
 * ~~~{.txt}
 * Address +0       +4       +8       +C       | 0123456789ABCDEF
 * 0x0250: 004142FE DC99AB21 21212100 00000000 | .AB....!!!!.....
 * 0x0260: 42210000 00000000                   | B!......
 * ~~~
 *
 * Note that regardless of the word size (8, 16, 32, or 64 bit), each line always contains up to 16 bytes of data.
 *
 * # Usage
 * Any @ref BinaryDumper() instance is intended to dump a specific, continuous block of binary data. The reference to
 * the data and the settings are bound to the @ref BinaryDumper() upon construction.
 *
 * After construction, a headline can be retrieved from the @ref BinaryDumper instance via @ref GetHeadLine().
 *
 * The dump can be retrieved line-by-line via @ref GetLine(). After all data has been dumped, any further call to
 * @ref GetLine() will return a line containing the next address, but no data:
 * ~~~{.txt}
 * 0x0270:
 * ~~~
 *
 * If there is no data right from instantiation, then the first call to @ref GetLine() will also return a line
 * containing the next address, but no data.
 *
 * Before invocation of @ref GetLine(), @ref IsAllDataDumped() can be used to query if there is any data left to be
 * dumped. The @ref BinaryDumper instance cannot be reused. After all data has been dumped, the object is intended to be
 * destroyed.
 *
 * The following example will print a dump of binary data to a command line interface:
 * ~~~{.cpp}
 * // CLI instantiated somewhere
 * gpcc::cli::CLI cli;
 *
 * // data to be dumped
 * std::vector<uint8_t> binaryData;
 * uintptr_t const vaddr = 0x250;
 *
 * BinaryDumper bd(binaryData.data(), binaryData.Size(), vaddr, 1U);
 *
 * cli.WriteLine(bd.GetHeadLine());
 *
 * // Note: If there would be zero bytes of data, then the do-while-loop will ensure that at least one line
 * // containing the address but no data will be printed.
 * do
 * {
 *   cli.WriteLine(bd.GetLine());
 * }
 * while (!bd.IsAllDataDumped());
 * ~~~
 *
 * # Dumping large data in small chunks
 * The following example shows how to dump a large chunk of data (e.g. several kB) without loading all the data into
 * RAM first. Instead small blocks of data (here 64 byte) are fetched and dumped in a loop.
 *
 * Example:
 * ~~~{.cpp}
 * // CLI instantiated somewhere
 * gpcc::cli::CLI cli;
 *
 * // The address may start at any value. Here we choose zero.
 * uintptr_t vaddr = 0x00;
 *
 * bool printHeadline = true;
 * while (true)
 * {
 *   uint8_t data[64];
 *   size_t nBytes;
 *   FetchNext64Byte(data, &nBytes);
 *
 *   BinaryDumper bd(data, nBytes, vaddr, 1U);
 *
 *   if (printHeadline)
 *   {
 *     cli.WriteLine(bd.GetHeadLine());
 *     cli.WriteLine(bd.GetLine());
 *     printHeadline = false;
 *   }
 *
 *   while (!bd.IsAllDataDumped())
 *     cli.WriteLine(bd.GetLine());
 *
 *   if (nBytes == 0U)
 *     break;
 *
 *   vaddr += nBytes;
 * }
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class BinaryDumper final
{
  public:
    BinaryDumper(void) = delete;
    BinaryDumper(void const * const pData,
                 size_t const nBytes,
                 uintptr_t const address,
                 uint8_t const wordSize);
    BinaryDumper(BinaryDumper const &) noexcept = default;
    BinaryDumper(BinaryDumper &&) noexcept = default;
    ~BinaryDumper(void) = default;

    BinaryDumper& operator=(BinaryDumper const &) noexcept = default;
    BinaryDumper& operator=(BinaryDumper &&) noexcept = default;

    std::string GetHeadLine(void) const;
    std::string GetLine(void);

    bool IsAllDataDumped(void) const noexcept;

  private:
    /// Size (in byte) of the data dumped in one full line of output.
    static size_t constexpr bytesPerLine_ = 16U;


    /// Address of the next word that will be printed.
    uintptr_t address_;

    /// Pointer to the next word that will be printed.
    /** May be `nullptr` if @ref nWords_ is zero. */
    void const * pData_;

    /// Number of words left to be printed. If this is zero, then there is no more data to be printed.
    size_t nWords_;

    /// Word-size in byte used to interpret the data referenced by @p pData_.
    uint8_t const wordSize_;

    /// Number of words printed in each line of output.
    /** The value is selected based on @ref wordSize_ so that each line contains 16 byte of binary data. */
    uint8_t wordsPerLine_;

    /// Field width reserved for the address at the beginning of each line of output.
    /** This is incl. "0x" and ": ". */
    uint8_t addressFieldWidth_;

    /// Maximum length (in characters) of any line generated by @ref GetHeadLine() or @ref GetLine().
    /** This does not include the trailing NUL. */
    uint8_t maxLineLength_;


    void AppendHexDump(StringComposer & sc, uint8_t const * pData, uint_fast8_t const nbOfWords) const;
    void AppendASCIIDump(StringComposer & sc, char const * pData, uint_fast8_t nbOfBytes) const;
};

} // namespace string
} // namespace gpcc

#endif // BINARYDUMPER_HPP_202501082012
