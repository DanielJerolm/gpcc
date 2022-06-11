/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2020, 2022 Daniel Jerolm

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

#ifndef SIMPLE_CRC_HPP_201612311313
#define SIMPLE_CRC_HPP_201612311313

#include <cstdint>
#include <cstddef>

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
