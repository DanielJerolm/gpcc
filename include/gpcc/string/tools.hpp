/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
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

/**
 * \ingroup GPCC_STRING
 * \brief Maximum nesting level for translation of nested exceptions to a string.
 *
 * This is used to limit the number of nested exceptions processed by
 * @ref ExceptionDescriptionToString(std::exception const &) and
 * @ref ExceptionDescriptionToString(std::exception_ptr const &).
 */
constexpr size_t maxDepthForExceptionToStringTranslation = 6U;


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

std::string HexDump(uintptr_t & address,
                    uint8_t const nbOfAddressDigits,
                    void const * & pData,
                    size_t & n,
                    uint8_t const wordSize,
                    uint_fast8_t wordsPerLine);

template<typename T>
std::string ToHex(T const value, uint8_t const digits);
template<typename T>
std::string ToBin(T value, uint8_t digits);
template<typename T>
std::string ToHexNoPrefix(T const value, uint8_t const digits);
template<typename T>
std::string ToDecAndHex(T const value, uint8_t const digits);

// Conversion string to X -----------------------------------------------------
uint8_t DecimalToU8(std::string const & s);
uint8_t AnyNumberToU8(std::string const & s);
uint8_t AnyStringToU8(std::string const & s);
uint8_t TwoDigitHexToU8(std::string const & s);

uint16_t FourDigitHexToU16(std::string const & s);

uint32_t DecimalToU32(std::string const & s);
uint32_t DecimalToU32(std::string const & s, uint32_t const min, uint32_t const max);
uint32_t HexToU32(std::string const & s);
uint32_t HexToU32(std::string const & s, uint32_t const min, uint32_t const max);
uint32_t AnyNumberToU32(std::string const & s);
uint32_t AnyNumberToU32(std::string const & s, uint32_t const min, uint32_t const max);

uint64_t DecimalToU64(std::string const & s);
uint64_t DecimalToU64(std::string const & s, uint64_t const min, uint64_t const max);
uint64_t HexToU64(std::string const & s);
uint64_t HexToU64(std::string const & s, uint64_t const min, uint64_t const max);
uint64_t AnyNumberToU64(std::string const & s);
uint64_t AnyNumberToU64(std::string const & s, uint64_t const min, uint64_t const max);

char AnyStringToChar(std::string const & s);

int32_t DecimalToI32(std::string const & s);
int32_t DecimalToI32(std::string const & s, int32_t const min, int32_t const max);
int32_t AnyNumberToI32(std::string const & s);
int32_t AnyNumberToI32(std::string const & s, int32_t const min, int32_t const max);

int64_t DecimalToI64(std::string const & s);
int64_t DecimalToI64(std::string const & s, int64_t const min, int64_t const max);
int64_t AnyNumberToI64(std::string const & s);
int64_t AnyNumberToI64(std::string const & s, int64_t const min, int64_t const max);

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

#include "tools.tcc"

#endif // TOOLS_HPP_201701151802
