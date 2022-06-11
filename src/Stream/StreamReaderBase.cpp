/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#include "StreamReaderBase.hpp"

namespace gpcc
{
namespace Stream
{

// --> IStreamReader
IStreamReader::States StreamReaderBase::GetState(void) const
/// \copydoc IStreamReader::GetState
{
  return state;
}
IStreamReader::Endian StreamReaderBase::GetEndian(void) const
/// \copydoc IStreamReader::GetEndian
{
  return endian;
}

void StreamReaderBase::Skip(size_t nBits)
/// \copydoc IStreamReader::Skip
{
  while (nBits >= 8U)
  {
    (void)Read_bits(8U);
    nBits -= 8U;
  }

  if (nBits != 0U)
    (void)Read_bits(nBits);
}

uint8_t StreamReaderBase::Read_uint8(void)
/// \copydoc IStreamReader::Read_uint8(void)
{
  return static_cast<uint8_t>(Pop());
}
uint16_t StreamReaderBase::Read_uint16(void)
/// \copydoc IStreamReader::Read_uint16(void)
{
  uint16_t retVal;
  if (endian == Endian::Little)
  {
   retVal  = static_cast<uint16_t>(Pop());
   retVal |= static_cast<uint16_t>(Pop()) << 8U;
  }
  else
  {
    retVal  = static_cast<uint16_t>(Pop()) << 8U;
    retVal |= static_cast<uint16_t>(Pop());
  }
  return retVal;
}
uint32_t StreamReaderBase::Read_uint32(void)
/// \copydoc IStreamReader::Read_uint32(void)
{
  uint32_t retVal;
  if (endian == Endian::Little)
  {
   retVal  = static_cast<uint32_t>(Pop());
   retVal |= static_cast<uint32_t>(Pop()) << 8U;
   retVal |= static_cast<uint32_t>(Pop()) << 16U;
   retVal |= static_cast<uint32_t>(Pop()) << 24U;
  }
  else
  {
    retVal  = static_cast<uint32_t>(Pop()) << 24U;
    retVal |= static_cast<uint32_t>(Pop()) << 16U;
    retVal |= static_cast<uint32_t>(Pop()) << 8U;
    retVal |= static_cast<uint32_t>(Pop());
  }
  return retVal;
}
uint64_t StreamReaderBase::Read_uint64(void)
/// \copydoc IStreamReader::Read_uint64(void)
{
  uint64_t retVal;
  if (endian == Endian::Little)
  {
   retVal  = static_cast<uint64_t>(Pop());
   retVal |= static_cast<uint64_t>(Pop()) << 8U;
   retVal |= static_cast<uint64_t>(Pop()) << 16U;
   retVal |= static_cast<uint64_t>(Pop()) << 24U;
   retVal |= static_cast<uint64_t>(Pop()) << 32U;
   retVal |= static_cast<uint64_t>(Pop()) << 40U;
   retVal |= static_cast<uint64_t>(Pop()) << 48U;
   retVal |= static_cast<uint64_t>(Pop()) << 56U;
  }
  else
  {
    retVal  = static_cast<uint64_t>(Pop()) << 56U;
    retVal |= static_cast<uint64_t>(Pop()) << 48U;
    retVal |= static_cast<uint64_t>(Pop()) << 40U;
    retVal |= static_cast<uint64_t>(Pop()) << 32U;
    retVal |= static_cast<uint64_t>(Pop()) << 24U;
    retVal |= static_cast<uint64_t>(Pop()) << 16U;
    retVal |= static_cast<uint64_t>(Pop()) << 8U;
    retVal |= static_cast<uint64_t>(Pop());
  }
  return retVal;
}
int8_t StreamReaderBase::Read_int8(void)
/// \copydoc IStreamReader::Read_int8(void)
{
  return static_cast<int8_t>(Pop());
}
int16_t StreamReaderBase::Read_int16(void)
/// \copydoc IStreamReader::Read_int16(void)
{
  return static_cast<int16_t>(Read_uint16());
}
int32_t StreamReaderBase::Read_int32(void)
/// \copydoc IStreamReader::Read_int32(void)
{
  return static_cast<int32_t>(Read_uint32());
}
int64_t StreamReaderBase::Read_int64(void)
/// \copydoc IStreamReader::Read_int64(void)
{
  return static_cast<int64_t>(Read_uint64());
}
float StreamReaderBase::Read_float(void)
/// \copydoc IStreamReader::Read_float(void)
{
  uint32_t const tmp = Read_uint32();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  return *reinterpret_cast<float const*>(&tmp);
#pragma GCC diagnostic pop
}
double StreamReaderBase::Read_double(void)
/// \copydoc IStreamReader::Read_double(void)
{
  uint64_t const tmp = Read_uint64();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  return *reinterpret_cast<double const*>(&tmp);
#pragma GCC diagnostic pop
}
bool StreamReaderBase::Read_bool(void)
/// \copydoc IStreamReader::Read_bool(void)
{
  return (PopBits(1) != 0);
}
bool StreamReaderBase::Read_bit(void)
/// \copydoc IStreamReader::Read_bit(void)
{
  return (PopBits(1) != 0);
}
uint8_t StreamReaderBase::Read_bits(uint_fast8_t n)
/// \copydoc IStreamReader::Read_bits(uint_fast8_t)
{
  return PopBits(n);
}
char StreamReaderBase::Read_char(void)
/// \copydoc IStreamReader::Read_char(void)
{
  return static_cast<char>(Pop());
}
std::string StreamReaderBase::Read_string(void)
/// \copydoc gpcc::Stream::IStreamReader::Read_string(void)
{
  std::string s;
  char c = static_cast<char>(Pop());
  while (c != 0x00)
  {
    s += c;
    c = static_cast<char>(Pop());
  }
  return s;
}
void StreamReaderBase::Read_uint8(uint8_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_uint8(uint8_t*,size_t)
{
  Pop(pDest, n);
}
void StreamReaderBase::Read_uint16(uint16_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_uint16(uint16_t*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_uint16();
}
void StreamReaderBase::Read_uint32(uint32_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_uint32(uint32_t*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_uint32();
}
void StreamReaderBase::Read_uint64(uint64_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_uint64(uint64_t*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_uint64();
}
void StreamReaderBase::Read_int8(int8_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_int8(int8_t*,size_t)
{
  Pop(pDest, n);
}
void StreamReaderBase::Read_int16(int16_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_int16(int16_t*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_int16();
}
void StreamReaderBase::Read_int32(int32_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_int32(int32_t*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_int32();
}
void StreamReaderBase::Read_int64(int64_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_int64(int64_t*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_int64();
}
void StreamReaderBase::Read_float(float* pDest, size_t n)
/// \copydoc IStreamReader::Read_float(float*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_float();
}
void StreamReaderBase::Read_double(double* pDest, size_t n)
/// \copydoc IStreamReader::Read_double(double*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_double();
}
void StreamReaderBase::Read_bool(bool* pDest, size_t n)
/// \copydoc IStreamReader::Read_bool(bool*,size_t)
{
  while (n-- != 0)
    *pDest++ = Read_bool();
}
void StreamReaderBase::Read_bits(uint8_t* pDest, size_t n)
/// \copydoc IStreamReader::Read_bits(uint8_t*,size_t)
{
  while (n >= 8)
  {
    *pDest++ = PopBits(8);
    n -= 8;
  }
  if (n != 0)
    *pDest = PopBits(n);
}
void StreamReaderBase::Read_char(char* pDest, size_t n)
/// \copydoc IStreamReader::Read_char(char*,size_t)
{
  Pop(pDest, n);
}
// <-- IStreamReader stuff

StreamReaderBase::StreamReaderBase(States const _state, Endian const _endian) noexcept
: IStreamReader()
, state(_state)
, endian(_endian)
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
 *
 * - - -
 *
 * \param _state Desired initial state for the stream.
 * \param _endian Desired endian for decoding of the data inside the stream.
 */
{
}

} // namespace Stream
} // namespace gpcc
