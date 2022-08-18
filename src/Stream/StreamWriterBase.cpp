/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "StreamWriterBase.hpp"

namespace gpcc
{
namespace Stream
{

// --> IStreamWriter
IStreamWriter::States StreamWriterBase::GetState(void) const
/// \copydoc IStreamWriter::GetState
{
  return state;
}
IStreamWriter::Endian StreamWriterBase::GetEndian(void) const
/// \copydoc IStreamWriter::GetEndian
{
  return endian;
}

uint_fast8_t StreamWriterBase::AlignToByteBoundary(bool const fillWithOnesNotZeros)
{
  uint_fast8_t nbOfBits = (8U - GetNbOfCachedBits()) % 8U;
  FillBits(nbOfBits, fillWithOnesNotZeros);
  return nbOfBits;
}

void StreamWriterBase::FillBits(size_t n, bool const oneNotZero)
/// \copydoc IStreamWriter::FillBits
{
  uint8_t const val = oneNotZero ? 0xFFU : 0x00U;
  while (n != 0U)
  {
    if (n >= 8U)
    {
      PushBits(val, 8U);
      n -= 8U;
    }
    else
    {
      PushBits(val, n);
      break;
    }
  }
}

void StreamWriterBase::FillBytes(size_t n, uint8_t const value)
/// \copydoc IStreamWriter::FillBytes
{
  while (n-- != 0U)
    Push(value);
}

void StreamWriterBase::Write_uint8(uint8_t data)
/// \copydoc IStreamWriter::Write_uint8(uint8_t)
{
  Push(static_cast<char>(data));
}
void StreamWriterBase::Write_uint8(uint8_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_uint8(uint8_t const *,size_t)
{
  Push(pData, n);
}
void StreamWriterBase::Write_uint16(uint16_t data)
/// \copydoc IStreamWriter::Write_uint16(uint16_t)
{
  if (endian == Endian::Little)
  {
    Push(static_cast<char>(data));
    Push(static_cast<char>(data >> 8));
  }
  else
  {
    Push(static_cast<char>(data >> 8));
    Push(static_cast<char>(data));
  }
}
void StreamWriterBase::Write_uint16(uint16_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_uint16(uint16_t const *,size_t)
{
  while (n-- != 0)
    Write_uint16(*pData++);
}
void StreamWriterBase::Write_uint32(uint32_t data)
/// \copydoc IStreamWriter::Write_uint32(uint32_t)
{
  if (endian == Endian::Little)
  {
    Push(static_cast<char>(data));
    Push(static_cast<char>(data >> 8));
    Push(static_cast<char>(data >> 16));
    Push(static_cast<char>(data >> 24));
  }
  else
  {
    Push(static_cast<char>(data >> 24));
    Push(static_cast<char>(data >> 16));
    Push(static_cast<char>(data >> 8));
    Push(static_cast<char>(data));
  }
}
void StreamWriterBase::Write_uint32(uint32_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_uint32(uint32_t const *,size_t)
{
  while (n-- != 0)
    Write_uint32(*pData++);
}
void StreamWriterBase::Write_uint64(uint64_t data)
/// \copydoc IStreamWriter::Write_uint64(uint64_t)
{
  if (endian == Endian::Little)
  {
    Push(static_cast<char>(data));
    Push(static_cast<char>(data >> 8));
    Push(static_cast<char>(data >> 16));
    Push(static_cast<char>(data >> 24));
    Push(static_cast<char>(data >> 32));
    Push(static_cast<char>(data >> 40));
    Push(static_cast<char>(data >> 48));
    Push(static_cast<char>(data >> 56));
  }
  else
  {
    Push(static_cast<char>(data >> 56));
    Push(static_cast<char>(data >> 48));
    Push(static_cast<char>(data >> 40));
    Push(static_cast<char>(data >> 32));
    Push(static_cast<char>(data >> 24));
    Push(static_cast<char>(data >> 16));
    Push(static_cast<char>(data >> 8));
    Push(static_cast<char>(data));
  }
}
void StreamWriterBase::Write_uint64(uint64_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_uint64(uint64_t const *,size_t)
{
  while (n-- != 0)
    Write_uint64(*pData++);
}

void StreamWriterBase::Write_int8(int8_t data)
/// \copydoc IStreamWriter::Write_int8(int8_t)
{
  Push(static_cast<char>(data));
}
void StreamWriterBase::Write_int8(int8_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_int8(int8_t const *,size_t)
{
  Push(pData, n);
}
void StreamWriterBase::Write_int16(int16_t data)
/// \copydoc IStreamWriter::Write_int16(int16_t)
{
  Write_uint16(static_cast<uint16_t>(data));
}
void StreamWriterBase::Write_int16(int16_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_int16(int16_t const *,size_t)
{
  Write_uint16(reinterpret_cast<uint16_t const *>(pData), n);
}
void StreamWriterBase::Write_int32(int32_t data)
/// \copydoc IStreamWriter::Write_int32(int32_t)
{
  Write_uint32(static_cast<uint32_t>(data));
}
void StreamWriterBase::Write_int32(int32_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_int32(int32_t const *,size_t)
{
  Write_uint32(reinterpret_cast<uint32_t const *>(pData), n);
}
void StreamWriterBase::Write_int64(int64_t data)
/// \copydoc IStreamWriter::Write_int64(int64_t)
{
  Write_uint64(static_cast<uint64_t>(data));
}
void StreamWriterBase::Write_int64(int64_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_int64(int64_t const *,size_t)
{
  Write_uint64(reinterpret_cast<uint64_t const *>(pData), n);
}
void StreamWriterBase::Write_float(float data)
/// \copydoc IStreamWriter::Write_float(float)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  Write_uint32(*reinterpret_cast<uint32_t const *>(static_cast<void const*>(&data)));
#pragma GCC diagnostic pop
}
void StreamWriterBase::Write_float(float const * pData, size_t n)
/// \copydoc IStreamWriter::Write_float(float const *,size_t)
{
  Write_uint32(reinterpret_cast<uint32_t const *>(pData), n);
}
void StreamWriterBase::Write_double(double data)
/// \copydoc IStreamWriter::Write_double(double)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  Write_uint64(*reinterpret_cast<uint64_t const*>(static_cast<void const*>(&data)));
#pragma GCC diagnostic pop
}
void StreamWriterBase::Write_double(double const * pData, size_t n)
/// \copydoc IStreamWriter::Write_double(double const *,size_t)
{
  Write_uint64(reinterpret_cast<uint64_t const *>(pData), n);
}
void StreamWriterBase::Write_bool(bool data)
/// \copydoc IStreamWriter::Write_bool(bool)
{
  PushBits(data?1:0, 1);
}
void StreamWriterBase::Write_bool(bool const * pData, size_t n)
/// \copydoc IStreamWriter::Write_bool(bool const *,size_t)
{
  while (n-- != 0)
    PushBits((*pData++)?1:0, 1);
}
void StreamWriterBase::Write_Bit(bool data)
/// \copydoc IStreamWriter::Write_Bit(bool)
{
  PushBits(data?1:0, 1);
}
void StreamWriterBase::Write_Bits(uint8_t bits, uint_fast8_t n)
/// \copydoc IStreamWriter::Write_Bits(uint8_t,uint_fast8_t)
{
  PushBits(bits, n);
}
void StreamWriterBase::Write_Bits(uint8_t const * pData, size_t n)
/// \copydoc IStreamWriter::Write_Bits(uint8_t const *,size_t)
{
  while (n > 7)
  {
    PushBits(*pData++, 8);
    n -= 8;
  }

  if (n != 0)
    PushBits(*pData, n);
}
void StreamWriterBase::Write_char(char data)
/// \copydoc IStreamWriter::Write_char(char)
{
  Push(data);
}
void StreamWriterBase::Write_char(char const * pData, size_t n)
/// \copydoc IStreamWriter::Write_char(char const *,size_t)
{
  Push(pData, n);
}
void StreamWriterBase::Write_string(std::string const & str)
/// \copydoc IStreamWriter::Write_string(std::string const&)
{
  char const * const p = str.c_str();
  size_t const n = str.length() + 1;
  Push(p, n);
}
void StreamWriterBase::Write_line(std::string const & str)
/// \copydoc IStreamWriter::Write_line(std::string const&)
{
  char const * const p = str.c_str();
  Push(p, str.length());
  Push('\n');
}
// <-- IStreamWriter

StreamWriterBase::StreamWriterBase(States const _state, Endian const _endian) noexcept
: IStreamWriter()
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
 * \param _endian Desired endian for the data inside the stream.
 */
{
}

} // namespace Stream
} // namespace gpcc
