/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef SIMPLE_CRC_HPP_201612311313
#define SIMPLE_CRC_HPP_201612311313

#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace crc  {

extern uint32_t const crc32ab_table_normal[256];
extern uint32_t const crc32ab_table_reflected[256];
extern uint16_t const crc16_ccitt_table_normal[256];
extern uint8_t const crc8_ccitt_table_normal[256];

void GenerateCRC8Table_normal(uint8_t const forward_polynomial, uint8_t table[256]) noexcept;
void GenerateCRC8Table_reflected(uint8_t const reverse_polynomial, uint8_t table[256]) noexcept;
void GenerateCRC16Table_normal(uint16_t const forward_polynomial, uint16_t table[256]) noexcept;
void GenerateCRC16Table_reflected(uint16_t const reverse_polynomial, uint16_t table[256]) noexcept;
void GenerateCRC32Table_normal(uint32_t const forward_polynomial, uint32_t table[256]) noexcept;
void GenerateCRC32Table_reflected(uint32_t const reverse_polynomial, uint32_t table[256]) noexcept;

void CalcCRC8_noInputReverse(uint8_t & crc, uint8_t const data, uint8_t const table[256]) noexcept;
void CalcCRC8_noInputReverse(uint8_t & crc, void const * const pData, size_t n, uint8_t const table[256]) noexcept;
void CalcCRC8_withInputReverse(uint8_t & crc, uint8_t const data, uint8_t const table[256]) noexcept;
void CalcCRC8_withInputReverse(uint8_t & crc, void const * const pData, size_t n, uint8_t const table[256]) noexcept;

void CalcCRC16_normal_noInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept;
void CalcCRC16_normal_noInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept;
void CalcCRC16_normal_withInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept;
void CalcCRC16_normal_withInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept;
void CalcCRC16_reflected_noInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept;
void CalcCRC16_reflected_noInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept;
void CalcCRC16_reflected_withInputReverse(uint16_t & crc, uint8_t const data, uint16_t const table[256]) noexcept;
void CalcCRC16_reflected_withInputReverse(uint16_t & crc, void const * const pData, size_t n, uint16_t const table[256]) noexcept;

void CalcCRC32_normal_noInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept;
void CalcCRC32_normal_noInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept;
void CalcCRC32_normal_withInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept;
void CalcCRC32_normal_withInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept;
void CalcCRC32_reflected_noInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept;
void CalcCRC32_reflected_noInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept;
void CalcCRC32_reflected_withInputReverse(uint32_t & crc, uint8_t const data, uint32_t const table[256]) noexcept;
void CalcCRC32_reflected_withInputReverse(uint32_t & crc, void const * const pData, size_t n, uint32_t const table[256]) noexcept;

} // namespace crc
} // namespace gpcc

#endif // SIMPLE_CRC_HPP_201612311313
