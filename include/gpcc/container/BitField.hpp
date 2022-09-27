/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef BITFIELD_HPP_201612310952
#define BITFIELD_HPP_201612310952

#include <limits>
#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc      {
namespace container {

/**
 * \ingroup GPCC_CONTAINER
 * \brief A bit-field for efficient storage of bits/booleans.
 *
 * # Features
 * Before using this class, consider using std::vector<bool> or std::bitset which are provided by STL and
 * should be generally preferred. However, this class provides the following advantages:
 * - creation from arrays of binary data
 * - assignment of bits from arrays of binary data
 * - high efficient search for locating asserted bits and cleared bits
 * - access to internal storage for direct appliance of user-specific operations on the bits
 * - generation of human-readable strings listing asserted/deasserted bits (example output: 1,2,5-8,9)
 *
 * # Internals
 * Internally, the bits are stored in an dynamic array of elements of type @ref storage_t. Each @ref storage_t element
 * contains multiple (for instance 32 or 64) bits. The exact number of bits stored in one @ref storage_t element is
 * @ref storage_t_size_in_bit. Upper unused bits in the last @ref storage_t element are undefined. Bit zero of the
 * @ref BitField corresponds to the LSB of the first @ref storage_t element of the array. Further array elements are
 * filled with bits from LSB to MSB.
 *
 * Access to bits via operator [] is implemented using a proxy (@ref BitProxy).
 *
 * # Access to internal storage
 * One feature of this class is that the internal @ref storage_t elements are accessible by clients. This allows
 * clients to implement bit-level logical operations efficiently.
 *
 * Example 1: Negate bits in bit field
 * ~~~{.cpp}
 * // Some bit field with some data
 * BitField bf(nBitsData, pData);
 *
 * // get access to bf's internal storage
 * size_t nElements;
 * BitField::storage_t* pInt = bf.GetInternalStorage(&nElements);
 *
 * // negate all bits
 * for (size_t i = 0; i < nElements; i++
 *   pInt[i] = ~pInt[i];
 * ~~~
 *
 * Example 2: Combine bits from multiple bit fields logically with each other
 * ~~~{.cpp}
 * // Some bit fields containing some data.
 * // Note that all bit fields contain the same number of bits.
 * BitField const bf1(nData, pDataForBf1);
 * BitField const bf2(nData, pDataForBf2);
 * BitField bf3(nData, pDataForBf3);
 *
 * // Get access to the internal storage of the bit fields.
 * // Note that all bit fields have the same size, so the number of storage_t elements
 * // is the same for all three BitFields. If we are not interested in the number of
 * // elements, we can pass nullptr or nothing to GetInternalStorage().
 * size_t nElements;
 * BitField::storage_t const * pS1 = bf1.GetInternalStorage(&nElements);
 * BitField::storage_t const * pS2 = bf2.GetInternalStorage(nullptr);
 * BitField::storage_t       * pS3 = bf3.GetInternalStorage();
 *
 * // Combine bits
 * for (size_t i = 0; i < nElements; i++)
 *   pS3[i] &= pS1[i] | pS2[i];
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class BitField final
{
  public:
    /// Data type used for internal storage of bits. @ref storage_t_size_in_bit bits are stored in one element of this type.
    typedef unsigned int storage_t;

    /// Proxy for accessing/modifying a single bit inside an @ref BitField instance via operator [].
    class BitProxy
    {
        friend class BitField;

      public:
        BitProxy(BitField::storage_t & _storage, uint_fast8_t const _bit) noexcept;

        BitProxy& operator=(BitProxy const & rhs) noexcept;  // (lvalue use)
        BitProxy& operator=(bool const b) noexcept;          // (lvalue use)

        operator bool() const noexcept;                      // (rvalue use)

      private:
        BitField::storage_t & storage;
        uint_fast8_t const bit;

        BitProxy(BitProxy const &) noexcept = default;
    };

    /// Number of bits stored in one element of type @ref storage_t.
    static size_t const storage_t_size_in_bit = sizeof(storage_t) * 8U;

    /// No bit found. Special return value of functions used to find bits.
    static size_t const NO_BIT = std::numeric_limits<size_t>::max();


    BitField(void) noexcept;
    BitField(size_t const _nBits);
    BitField(size_t const _nBits, uint8_t const * const pData);
    BitField(BitField const & other);
    BitField(BitField&& other) noexcept;
    ~BitField(void) = default;


    BitField& operator=(BitField const & other);
    BitField& operator=(BitField&& other) noexcept;

    bool operator==(BitField const & rhv) const noexcept;

    const BitProxy operator[](size_t const index) const;
    BitProxy operator[](size_t const index);


    size_t GetSize(void) const noexcept;
    void Resize(size_t const newSize);

    void ClearAll(void) noexcept;
    void SetAll(void) noexcept;

    void Assign(size_t const newSize, uint8_t const * const pNewData);

    void ClearBit(size_t const index);
    void SetBit(size_t const index);
    void WriteBit(size_t const index, bool const value);
    bool GetBit(size_t const index) const;

    size_t FindFirstSetBit(size_t const startIndex) const noexcept;
    size_t FindFirstClearedBit(size_t const startIndex) const noexcept;
    size_t FindFirstSetBitReverse(size_t startIndex) const noexcept;
    size_t FindFirstClearedBitReverse(size_t startIndex) const noexcept;

    std::string EnumerateBits(bool const setNotCleared, bool const noWhitespaces = false) const;
    std::string EnumerateBitsCompressed(bool const setNotCleared, bool const noWhitespaces = false) const;

    storage_t const * GetInternalStorage(size_t * const pSize = nullptr) const noexcept;
    storage_t * GetInternalStorage(size_t * const pSize = nullptr) noexcept;

  private:
    /// Number of bits stored in the @ref BitField.
    size_t nBits;

    /// Storage for the bits.
    std::unique_ptr<storage_t[]> spStorage;


    void CheckMax_nBits(size_t const _nBits) const;
    size_t nbOf_storage_t_elements(size_t const _nBits) const noexcept;
};

/**
 * \brief Retrieves the size of the @ref BitField.
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
 * Size of the @ref BitField in bit.
 */
inline size_t BitField::GetSize(void) const noexcept
{
  return nBits;
}

} // namespace container
} // namespace gpcc

#endif // BITFIELD_HPP_201612310952
