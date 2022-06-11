/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2021, 2022 Daniel Jerolm

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

#ifndef TOOLS_HPP_201701151802
#define TOOLS_HPP_201701151802

#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cstdarg>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace string {

// String manipulation --------------------------------------------------------
std::string Trim(std::string const & s);
std::string Trim(std::string const & s, char const c);
std::vector<std::string> Split(std::string const & s, char const separator, bool const skipEmptyParts);
std::vector<std::string> Split(std::string const & s, char const separator, bool const skipEmptyParts, char const quotationMark);
void ConditionalConcat(std::vector<std::string> & v, char const glueChar);
void InsertIndention(std::string & s, size_t const n);

// Tests ----------------------------------------------------------------------
bool StartsWith(std::string const & s, char const * pCharSeq) noexcept;
bool EndsWith(std::string const & s, char const * pCharSeq) noexcept;
size_t CountChar(std::string const & s, char const c) noexcept;
bool TestSimplePatternMatch(std::string const & s, char const * pCharSeq, bool const caseSensitive);
bool TestSimplePatternMatch(char const * pStr, char const * pCharSeq, bool const caseSensitive);
bool IsPrintableASCII(char const c) noexcept;
bool IsPrintableASCIIOnly(std::string const & s) noexcept;
bool IsDecimalDigitsOnly(std::string const & s) noexcept;

// Conversion X to string -----------------------------------------------------
std::string ExceptionDescriptionToString(std::exception const & e);
std::string ExceptionDescriptionToString(std::exception_ptr const & ePtr);
std::string HexDump(uint32_t const address, void const * const pData, size_t const n, uint8_t const wordSize, uint8_t valuesPerLine);
std::string ToHex(uint32_t const value, uint8_t const width);
std::string ToBin(uint32_t value, uint8_t width);
std::string ToHexNoPrefix(uint32_t const value, uint8_t const width);
std::string ToDecAndHex(uint32_t const value, uint8_t const width);

// Conversion string to X -----------------------------------------------------
uint8_t DecimalToU8(std::string const & s);
uint32_t DecimalToU32(std::string const & s);
int32_t DecimalToI32(std::string const & s);
uint8_t AnyStringToU8(std::string const & s);
uint32_t AnyStringToU32(std::string const & s);
char AnyStringToChar(std::string const & s);
uint8_t TwoDigitHexToU8(std::string const & s);
uint16_t FourDigitHexToU16(std::string const & s);
double ToDouble(std::string const & s);

// Extraction and breakdown ---------------------------------------------------
std::vector<std::pair<std::string,std::string>> ExtractFieldAndValue(std::string const & input,
                                                                     char const separatorChar,
                                                                     char const assignmentChar,
                                                                     char const quotationMarkChar);

// Composition ----------------------------------------------------------------
std::unique_ptr<char[]> VASPrintf(char const * const pFmt, va_list args);
std::unique_ptr<char[]> ASPrintf(char const * const pFmt, ...);

} // namesapce string
} // namesapce gpcc


#endif // TOOLS_HPP_201701151802
