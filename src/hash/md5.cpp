/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2020 Daniel Jerolm
*/

/*
 * License note:
 * -------------
 *
 * The functionality provided in this file has been implemented from scratch according to the RFC1321 specification.
 * It is not a derivative work from the reference implementation supplied with RFC1321.
 *
 * See https://tools.ietf.org/html/rfc1321 for details.
 */

#include <gpcc/hash/md5.hpp>
#include <gpcc/compiler/definitions.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/stream/MemStreamWriter.hpp>
#include <cstring>
#include <stdexcept>

// Table T[i] containing 4294967296 * abs(sin(i)) with i in radians.
// Values according to RFC1321.
static uint32_t const T[64] =
{
  0xd76aa478UL, //  1
  0xe8c7b756UL, //  2
  0x242070dbUL, //  3
  0xc1bdceeeUL, //  4
  0xf57c0fafUL, //  5
  0x4787c62aUL, //  6
  0xa8304613UL, //  7
  0xfd469501UL, //  8
  0x698098d8UL, //  9
  0x8b44f7afUL, // 10
  0xffff5bb1UL, // 11
  0x895cd7beUL, // 12
  0x6b901122UL, // 13
  0xfd987193UL, // 14
  0xa679438eUL, // 15
  0x49b40821UL, // 16
  0xf61e2562UL, // 17
  0xc040b340UL, // 18
  0x265e5a51UL, // 19
  0xe9b6c7aaUL, // 20
  0xd62f105dUL, // 21
   0x2441453UL, // 22
  0xd8a1e681UL, // 23
  0xe7d3fbc8UL, // 24
  0x21e1cde6UL, // 25
  0xc33707d6UL, // 26
  0xf4d50d87UL, // 27
  0x455a14edUL, // 28
  0xa9e3e905UL, // 29
  0xfcefa3f8UL, // 30
  0x676f02d9UL, // 31
  0x8d2a4c8aUL, // 32
  0xfffa3942UL, // 33
  0x8771f681UL, // 34
  0x6d9d6122UL, // 35
  0xfde5380cUL, // 36
  0xa4beea44UL, // 37
  0x4bdecfa9UL, // 38
  0xf6bb4b60UL, // 39
  0xbebfbc70UL, // 40
  0x289b7ec6UL, // 41
  0xeaa127faUL, // 42
  0xd4ef3085UL, // 43
   0x4881d05UL, // 44
  0xd9d4d039UL, // 45
  0xe6db99e5UL, // 46
  0x1fa27cf8UL, // 47
  0xc4ac5665UL, // 48
  0xf4292244UL, // 49
  0x432aff97UL, // 50
  0xab9423a7UL, // 51
  0xfc93a039UL, // 52
  0x655b59c3UL, // 53
  0x8f0ccc92UL, // 54
  0xffeff47dUL, // 55
  0x85845dd1UL, // 56
  0x6fa87e4fUL, // 57
  0xfe2ce6e0UL, // 58
  0xa3014314UL, // 59
  0x4e0811a1UL, // 60
  0xf7537e82UL, // 61
  0xbd3af235UL, // 62
  0x2ad7d2bbUL, // 63
  0xeb86d391UL  // 64
};

// Control information for the calculations performed during one of the 16 steps of a "round".
struct RoundCtrl
{
  uint8_t k;
  uint8_t s;
};

// Control information for round 1.
static RoundCtrl const Round1Ctrl[16] =
{
  { 0U,  7U},
  { 1U, 12U},
  { 2U, 17U},
  { 3U, 22U},
  { 4U,  7U},
  { 5U, 12U},
  { 6U, 17U},
  { 7U, 22U},
  { 8U,  7U},
  { 9U, 12U},
  {10U, 17U},
  {11U, 22U},
  {12U,  7U},
  {13U, 12U},
  {14U, 17U},
  {15U, 22U}
};

// Control information for round 2.
static RoundCtrl const Round2Ctrl[16] =
{
  { 1U,  5U},
  { 6U,  9U},
  {11U, 14U},
  { 0U, 20U},
  { 5U,  5U},
  {10U,  9U},
  {15U, 14U},
  { 4U, 20U},
  { 9U,  5U},
  {14U,  9U},
  { 3U, 14U},
  { 8U, 20U},
  {13U,  5U},
  { 2U,  9U},
  { 7U, 14U},
  {12U, 20U}
};

// Control information for round 3.
static RoundCtrl const Round3Ctrl[16] =
{
  { 5U,  4U},
  { 8U, 11U},
  {11U, 16U},
  {14U, 23U},
  { 1U,  4U},
  { 4U, 11U},
  { 7U, 16U},
  {10U, 23U},
  {13U,  4U},
  { 0U, 11U},
  { 3U, 16U},
  { 6U, 23U},
  { 9U,  4U},
  {12U, 11U},
  {15U, 16U},
  { 2U, 23U}
};

// Control information for round 4.
static RoundCtrl const Round4Ctrl[16] =
{
  { 0U,  6U},
  { 7U, 10U},
  {14U, 15U},
  { 5U, 21U},
  {12U,  6U},
  { 3U, 10U},
  {10U, 15U},
  { 1U, 21U},
  { 8U,  6U},
  {15U, 10U},
  { 6U, 15U},
  {13U, 21U},
  { 4U,  6U},
  {11U, 10U},
  { 2U, 15U},
  { 9U, 21U}
};


namespace gpcc {
namespace hash {

/**
 * \ingroup GPCC_HASH
 * \brief Calculates a MD5 hash for a block of data provided via a pointer and size value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param pData
 * Pointer to the data.\n
 * The address of the referenced memory location must be aligned to a 4-byte boundary.\n
 * _nullptr_ is not allowed, except 's' is zero.
 *
 * \param s
 * Size of the data in byte. Zero is allowed.
 *
 * \return
 * MD5 hash calculated over `s` bytes of data referenced by `pData`.\n
 * The size is always 16 bytes (128 bit).
 */
std::vector<uint8_t> MD5Sum(void const * const pData, size_t const s)
{
  if ((pData == nullptr) && (s != 0U))
    throw std::invalid_argument("MD5Sum: !pData");

  // verify alignment and create a pointer to allow for 32bit access to the data
  if ((reinterpret_cast<uintptr_t>(pData) % 4U) != 0U)
    throw std::invalid_argument("MD5Sum: pData not aligned to 4-byte boundary");

  uint32_t const * pDataU32 = reinterpret_cast<uint32_t const *>(pData);

  // Container for final result. It is allocated here to avoid a late std::bad_alloc.
  std::vector<uint8_t> result(16U);

  // F(x,y,z), G(x,y,z), H(x,y,z), I(x,y,z) according to RFC1321 plus some optimizations
  auto F = [](uint32_t const x, uint32_t const y, uint32_t const z) -> uint32_t
  {
    return z ^ (x & (y ^ z));
  };

  auto G = [](uint32_t const x, uint32_t const y, uint32_t const z) -> uint32_t
  {
    return y ^ (z & (x ^ y));
  };

  auto H = [](uint32_t const x, uint32_t const y, uint32_t const z) -> uint32_t
  {
    return x ^ y ^ z;
  };

  auto I = [](uint32_t const x, uint32_t const y, uint32_t const z) -> uint32_t
  {
    return y ^ (x | (~z));
  };

  // working registers A...D, initialized according to RFC1321
  uint32_t A = 0x67452301UL;
  uint32_t B = 0xEFCDAB89UL;
  uint32_t C = 0x98BADCFEUL;
  uint32_t D = 0x10325476UL;

  // Data is processed in blocks of 64 bytes (512bit). We will finally zero this in any case, because it may
  // contain sensitive information.
  uint32_t block[16];
  ON_SCOPE_EXIT(shredBlock) { memset(block, 0U, sizeof(block)); };

  // We use a small state machine to control filling the blocks with input data. After all input data has been
  // processed the state machine controls appending a single '1'-bit (0x80), padding zeros, and appending the size
  // value. The states are:
  enum States
  {
    fillData,
    pad0x80,
    padLength,
    done
  };

  States state = fillData;

  size_t remainingBytes = s;
  do
  {
    // determine the number of bytes to pick up from the input data during this loop cycle
    uint_fast8_t bytesToPick;
    if (remainingBytes >= sizeof(block))
      bytesToPick = sizeof(block);
    else
      bytesToPick = remainingBytes;

    uint32_t* pBlock = block;
    uint_fast8_t bytesInBlock = bytesToPick;

    // pick as many data as possible in chunks of 4 byte
    if (bytesToPick >= 4U)
    {
      uint_fast8_t wordsToPick = bytesToPick / 4U;

      bytesToPick    -= wordsToPick * 4U;
      remainingBytes -= wordsToPick * 4U;
      while (wordsToPick != 0U)
      {
        #if GPCC_SYSTEMS_ENDIAN == GPCC_BIG
          uint32_t const v = *pDataU32++;

          *pBlock++ =   ((v & 0x000000FFUL) << 24U)
                      | ((v & 0x0000FF00UL) <<  8U)
                      | ((v & 0x00FF0000UL) >>  8U)
                      | ((v & 0xFF000000UL) >> 24U);
        #elif GPCC_SYSTEMS_ENDIAN == GPCC_LITTLE
          *pBlock++ = *pDataU32++;
        #else
          #error "Endian not supported!"
        #endif
        wordsToPick--;
      }
    }

    // pick any remaining bytes (0..3)
    uint8_t* pBlockBytePtr = reinterpret_cast<uint8_t*>(pBlock);
    if (bytesToPick != 0U)
    {
      // create a pointer to access the input data byte by byte
      uint8_t const * pDataU32BytePtr = reinterpret_cast<uint8_t const *>(pDataU32);

      remainingBytes -= bytesToPick;

      while (bytesToPick != 0U)
      {
        *pBlockBytePtr++ = *pDataU32BytePtr++;
        bytesToPick--;
      }

      if (remainingBytes != 0U)
        throw std::logic_error("MD5Sum: Internal error");
    }

    if (state == States::fillData)
    {
      // all input data processed?
      if (remainingBytes == 0U)
        state = States::pad0x80;
    }

    if (state == States::pad0x80)
    {
      // at least one byte left in "block"?
      if (bytesInBlock != sizeof(block))
      {
        *pBlockBytePtr++ = 0x80U;
        bytesInBlock++;
        state = States::padLength;
      }
    }

    if (state == States::padLength)
    {
      // pad until at least 8 bytes are left in "block"
      while (bytesInBlock < (sizeof(block) - 8U))
      {
        *pBlockBytePtr++ = 0U;
        bytesInBlock++;
      }

      // exactly 8 bytes left in "block"?
      if (bytesInBlock == (sizeof(block) - 8U))
      {
        // calculate length in bit and append it to "block"
        size_t const s_in_bit = (s & 0x1FFFFFFFFFFFFFFFULL) * 8U;

        *pBlockBytePtr++ = (s_in_bit & 0x00000000000000FFULL);
        *pBlockBytePtr++ = (s_in_bit & 0x000000000000FF00ULL) >> 8U;
        *pBlockBytePtr++ = (s_in_bit & 0x0000000000FF0000ULL) >> 16U;
        *pBlockBytePtr++ = (s_in_bit & 0x00000000FF000000ULL) >> 24U;
        *pBlockBytePtr++ = (s_in_bit & 0x000000FF00000000ULL) >> 32U;
        *pBlockBytePtr++ = (s_in_bit & 0x0000FF0000000000ULL) >> 40U;
        *pBlockBytePtr++ = (s_in_bit & 0x00FF000000000000ULL) >> 48U;
        *pBlockBytePtr++ = (s_in_bit & 0xFF00000000000000ULL) >> 56U;
        bytesInBlock += 8U;

        if (bytesInBlock != sizeof(block))
          throw std::logic_error("MD5Sum: Internal error");

        state = States::done;
      }
      else
      {
        // There were less than 8 bytes left in the block. Fill the block with zeros and we will
        // take another loop cycle.
        while (bytesInBlock < sizeof(block))
        {
          *pBlockBytePtr++ = 0U;
          bytesInBlock++;
        }
      }
    }

    // at this point, "block" must always be completely filled
    if (bytesInBlock != sizeof(block))
      throw std::logic_error("MD5Sum: Internal error");

    uint32_t const AA = A;
    uint32_t const BB = B;
    uint32_t const CC = C;
    uint32_t const DD = D;

    auto RotateRegisters = [&A, &B, &C, &D]()
    {
      uint32_t const v = D;
      D = C;
      C = B;
      B = A;
      A = v;
    };

    auto RotateLeft = [](uint32_t const v, uint_fast8_t const n) -> uint32_t
    {
      // most compilers will recognize this pattern and use a suitable instruction if available
      return (v << n) | (v >> (32U - n));
    };

    // Round 1
    // From spec: [abcd k s i] denotes a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s)
    for (uint_fast8_t i = 0U; i < 16U; i++)
    {
      uint32_t const v = A + F(B, C, D) + block[Round1Ctrl[i].k] + T[i];
      A = B + RotateLeft(v, Round1Ctrl[i].s);
      RotateRegisters();
    }

    // Round 2
    // From spec: [abcd k s i] denotes a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s)
    for (uint_fast8_t i = 0U; i < 16U; i++)
    {
      uint32_t const v = A + G(B, C, D) + block[Round2Ctrl[i].k] + T[i + 16U];
      A = B + RotateLeft(v, Round2Ctrl[i].s);
      RotateRegisters();
    }

    // Round 3
    // From spec: [abcd k s t] denotes a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s).
    for (uint_fast8_t i = 0U; i < 16U; i++)
    {
      uint32_t const v = A + H(B, C, D) + block[Round3Ctrl[i].k] + T[i + 32U];
      A = B + RotateLeft(v, Round3Ctrl[i].s);
      RotateRegisters();
    }

    // Round 4
    // From spec: [abcd k s t] denote a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s).
    for (uint_fast8_t i = 0; i < 16U; i++)
    {
      uint32_t const v = A + I(B, C, D) + block[Round4Ctrl[i].k] + T[i + 48U];
      A = B + RotateLeft(v, Round4Ctrl[i].s);
      RotateRegisters();
    }

    A = A + AA;
    B = B + BB;
    C = C + CC;
    D = D + DD;
  }
  while (state != States::done);

  // build result
  gpcc::Stream::MemStreamWriter msw(result.data(), result.size(), gpcc::Stream::MemStreamWriter::Endian::Little);
  msw.Write_uint32(A);
  msw.Write_uint32(B);
  msw.Write_uint32(C);
  msw.Write_uint32(D);
  msw.Close();

  return result;
}

/**
 * \ingroup GPCC_HASH
 * \brief Calculates a MD5 hash for a block of data given as std::vector of uint8_t.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param data
 * Reference to an std::vector<uint8_t> containing the data.
 *
 * \return
 * MD5 hash calculated for the data provided by `data`.\n
 * The size is always 16 bytes (128 bit).
 */
std::vector<uint8_t> MD5Sum(std::vector<uint8_t> const & data)
{
  return MD5Sum(data.data(), data.size());
}

/**
 * \ingroup GPCC_HASH
 * \brief Calculates a MD5 hash for a block of data given as std::vector of char.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param data
 * Reference to an std::vector<char> containing the data.
 *
 * \return
 * MD5 hash calculated for the data provided by `data`.\n
 * The size is always 16 bytes (128 bit).
 */
std::vector<uint8_t> MD5Sum(std::vector<char> const & data)
{
  return MD5Sum(data.data(), data.size());
}

}
}
