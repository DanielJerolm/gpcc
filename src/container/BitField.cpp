/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "BitField.hpp"
#include "gpcc/src/Compiler/builtins.hpp"
#include <sstream>
#include <stdexcept>
#include <cstring>

namespace gpcc      {
namespace container {

#ifndef __DOXYGEN__
size_t const BitField::storage_t_size_in_bit;
size_t const BitField::NO_BIT;
#endif

/**
 * \brief Constructor. Creates a @ref BitProxy instance accessing a specific bit of an @ref BitField::storage_t element.
 *
 * Instances of class @ref BitProxy are intended to be created by operator[] of class @ref BitField.
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
 * \param _storage
 * Reference to the BitField's @ref storage_t element containing the bit that shall be accessed.
 * \param _bit
 * Index of the bit in `_storage` that shall be accessed.\n
 * This must be within 0 and @ref BitField::storage_t_size_in_bit - 1.
 */
BitField::BitProxy::BitProxy(BitField::storage_t & _storage, uint_fast8_t const _bit) noexcept
: storage(_storage)
, bit(_bit)
{
}

/**
 * \brief Assigns a value from another @ref BitProxy to the bit represented by this @ref BitProxy.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref BitField object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhs
 * Another @ref BitProxy instance. The value represented by it is assigned to the bit represented by this proxy
 * instance.
 * \return
 * Reference to this proxy.
 */
BitField::BitProxy& BitField::BitProxy::operator=(BitProxy const & rhs) noexcept
{
  BitField::storage_t const mask = static_cast<BitField::storage_t>(1U) << bit;
  if (rhs)
    storage |= mask;
  else
    storage &= ~mask;

  return *this;
}

/**
 * \brief Assigns a bool to the bit represented by this @ref BitProxy.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref BitField object is modified. Concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param b
 * Value to be assigned to the bit represented by this proxy instance.
 * \return
 * Reference to this proxy.
 */
BitField::BitProxy& BitField::BitProxy::operator=(bool const b) noexcept
{
  BitField::storage_t const mask = static_cast<BitField::storage_t>(1U) << bit;
  if (b)
    storage |= mask;
  else
    storage &= ~mask;

  return *this;
}

/**
 * \brief Operator bool. Retrieves the value of the bit represented by this @ref BitProxy.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref BitField object is not modified. Concurrent accesses are safe.
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
 * Value of the bit represented by this proxy instance.
 */
BitField::BitProxy::operator bool() const noexcept
{
  BitField::storage_t const mask = static_cast<BitField::storage_t>(1U) << bit;
  return ((storage & mask) != 0);
}



/**
 * \brief Constructor. Creates an empty @ref BitField.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
BitField::BitField(void) noexcept
: nBits(0)
, spStorage()
{
}

/**
 * \brief Constructor. Creates a @ref BitField containing _nBits, all initialized with '0'.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _nBits   Desired size for the @ref BitField.
 */
BitField::BitField(size_t const _nBits)
: nBits(_nBits)
, spStorage()
{
  if (nBits != 0)
  {
    CheckMax_nBits(nBits);

    size_t const nbOfElements = nbOf_storage_t_elements(nBits);
    spStorage.reset(new storage_t[nbOfElements]);
    memset(spStorage.get(), 0x00, nbOfElements * sizeof(storage_t));
  }
}

/**
 * \brief Constructor. Creates a @ref BitField initialized with binary data from an array.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _nBits
 * Desired size for the @ref BitField.
 * \param pData
 * Pointer to an array of binary data that shall be used to initialize the @ref BitField. The LSB of the byte
 * referenced by `pData` will be used to initialize bit 0, the LSB of `pData+1` will be used to initialize bit 8,
 * and so on.\n
 * _nullptr is not allowed._
 */
BitField::BitField(size_t const _nBits, uint8_t const * const pData)
: nBits(_nBits)
, spStorage()
{
  if (pData == nullptr)
    throw std::invalid_argument("BitField::BitField: !pData");

  if (nBits != 0)
  {
    CheckMax_nBits(nBits);

    size_t const nbOfElements = nbOf_storage_t_elements(nBits);
    spStorage.reset(new storage_t[nbOfElements]);
    spStorage[nbOfElements - 1U] = 0;
    memcpy(spStorage.get(), pData, (nBits + 7U) / 8U);
  }
}

/**
 * \brief Copy constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other   Unmodifiable reference to another @ref BitField instance that shall be copied.
 */
BitField::BitField(BitField const & other)
: nBits(other.nBits)
, spStorage()
{
  if (nBits != 0)
  {
    size_t const nbOfElements = nbOf_storage_t_elements(nBits);
    spStorage.reset(new storage_t[nbOfElements]);
    memcpy(spStorage.get(), other.spStorage.get(), nbOfElements * sizeof(storage_t));
  }
}

/**
 * \brief Move constructor.
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
 * \param other
 * Reference to another @ref BitField instance whose content shall be moved into the new @ref BitField instance.\n
 * The other @ref BitField instance will be empty afterwards.
 */
BitField::BitField(BitField&& other) noexcept
: nBits(other.nBits)
, spStorage(std::move(other.spStorage))
{
  other.nBits = 0;
}

/**
 * \brief Copy assignment operator. Assigns the content of another @ref BitField to this (deep copy).
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * Unmodifiable reference to a @ref BitField instance whose content shall be copy-assigned to this.\n
 * A deep copy is created.
 * \return
 * Reference to this @ref BitField instance.
 */
BitField& BitField::operator=(BitField const & other)
{
  if (&other != this)
  {
    if (other.nBits == 0)
    {
      nBits = 0;
      spStorage.reset();
    }
    else
    {
      size_t const myElements = nbOf_storage_t_elements(nBits);
      size_t const otherElements = nbOf_storage_t_elements(other.nBits);

      if (myElements != otherElements)
      {
        std::unique_ptr<storage_t[]> spNewStorage(new storage_t[otherElements]);
        spStorage = std::move(spNewStorage);
      }

      nBits = other.nBits;

      memcpy(spStorage.get(), other.spStorage.get(), otherElements * sizeof(storage_t));
    }
  }

  return *this;
}

/**
 * \brief Move assignment operator.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * Reference to another @ref BitField instance whose content shall be move-assigned to this.\n
 * The other @ref BitField instance will be empty afterwards.
 * \return
 * Reference to this @ref BitField instance.
 */
BitField& BitField::operator=(BitField&& other) noexcept
{
  if (&other != this)
  {
    spStorage = std::move(other.spStorage);
    nBits = other.nBits;
    other.nBits = 0;
  }

  return *this;
}

/**
 * \brief Operator==. Compares the bits stored in this @ref BitField instance against the bits stored
 * in another @ref BitField instance for equality.
 *
 * Two @ref BitField instances are equal if they have the same size and if the contained bits
 * (if any) have the same values.
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
 * \param rhv
 * Unmodifiable reference to a @ref BitField instance that shall be compared to this @ref BitField instance.
 * \return
 * Result of the comparison:\n
 * true  = The two @ref BitField instances contain the same number of bits and the bit's values are equal.\n
 * false = Either the size of the two @ref BitField instances is different or at least one bit is different.\n
 * Comparison of two @ref BitField instances with size zero will result in "true".
 */
bool BitField::operator==(BitField const & rhv) const noexcept
{
  // compare against self? => Equal
  if (this == &rhv)
    return true;

  // different number of bits? => Not equal
  if (nBits != rhv.nBits)
    return false;

  // no bits? => Equal
  if (nBits == 0)
    return true;

  size_t const nbOfFullUsedBytes    = nBits / 8U;
  uint_fast8_t const bitsInLastByte = nBits % 8U;

  if (bitsInLastByte == 0)
  {
    // there are no unused (and therefore undefined) bits in the last byte of storage,
    // so a simple memcmp() will do the job
    return (memcmp(spStorage.get(), rhv.spStorage.get(), nbOfFullUsedBytes) == 0);
  }
  else
  {
    // completely used bytes can be checked using memcmp()...
    if ((nbOfFullUsedBytes != 0) && (memcmp(spStorage.get(), rhv.spStorage.get(), nbOfFullUsedBytes) != 0))
      return false;

    // ...and the last byte must be compared manually, because at least one upper bit is unused and undefined
    uint8_t const lastByteThis = *(static_cast<uint8_t const *>(static_cast<void const *>(    spStorage.get())) + nbOfFullUsedBytes);
    uint8_t const lastByteRhv  = *(static_cast<uint8_t const *>(static_cast<void const *>(rhv.spStorage.get())) + nbOfFullUsedBytes);
    uint8_t const mask = ~(0xFFU << bitsInLastByte);

    return ((lastByteThis & mask) == (lastByteRhv & mask));
  }
}

/**
 * \brief Operator []. Provides read-only access to a single bit of the @ref BitField.
 *
 * Note: @ref BitField::BitProxy is used as a proxy to access the bit (proxy pattern).
 *
 * Example:
 * ~~~{.cpp}
 * BitField const bf(16, pSomeData);
 * bool bit_3 = bf[3];
 * if (bf[4])
 * {
 *   // ...
 * }
 * else if (!bf[5])
 * {
 *   // ...
 * }
 * ~~~
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * Note that the proxy accesses the object after this method has returned.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::out_of_range   Parameter `index` is out of bounds.

 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the bit that shall be accessed. A range check will be applied.
 * \return
 * A proxy providing access to the bit. The proxy is transparent for the caller.
 */
const BitField::BitProxy BitField::operator[](size_t const index) const
{
  if (index >= nBits)
    throw std::out_of_range("BitField::operator[]: index too large");

  return BitProxy(spStorage[index / storage_t_size_in_bit], index % storage_t_size_in_bit);
}

/**
 * \brief Operator []. Provides read/write-access to a single bit inside the @ref BitField.
 *
 * Note: @ref BitField::BitProxy is used as a proxy to access the bit (proxy pattern).
 *
 * Example:
 * ~~~{.cpp}
 * BitField bf(16, pSomeData);
 * bool bit_3 = bf[3];
 * bf[4] = !bf[5];
 * bf[5] = false;
 * ~~~
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * Note that the proxy accesses (and maybe modifies) the object after this method has returned.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::out_of_range   Parameter `index` is out of bounds.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the bit that shall be accessed. A range check will be applied.
 * \return
 * A proxy providing access to the bit. The proxy is transparent for the caller.
 */
BitField::BitProxy BitField::operator[](size_t const index)
{
  if (index >= nBits)
    throw std::out_of_range("BitField::operator[]: index too large");

  return BitProxy(spStorage[index / storage_t_size_in_bit], index % storage_t_size_in_bit);
}

/**
 * \brief Changes the size of the @ref BitField.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newSize
 * New size for the @ref BitField in bit.\n
 * If the @ref BitField is enlarged, then new bits are initialized with zero.
 */
void BitField::Resize(size_t const newSize)
{
  if (newSize == nBits)
    return;

  if (newSize == 0)
  {
    nBits = 0;
    spStorage.reset();
  }
  else
  {
    CheckMax_nBits(newSize);

    size_t const currElements = nbOf_storage_t_elements(nBits);
    size_t const newElements = nbOf_storage_t_elements(newSize);

    // reallocation of storage required?
    if (currElements != newElements)
    {
      std::unique_ptr<storage_t[]> spNewStorage(new storage_t[newElements]);

      // enlargement?
      if (newElements > currElements)
      {
        // (enlarge)
        memcpy(spNewStorage.get(), spStorage.get(), currElements * sizeof(storage_t));
        memset(spNewStorage.get() + currElements, 0x00, (newElements - currElements) * sizeof(storage_t));
      }
      else
      {
        // (shrink)
        memcpy(spNewStorage.get(), spStorage.get(), newElements * sizeof(storage_t));
      }

      spStorage = std::move(spNewStorage);
    }

    // In case of enlarge, new bits must be cleared.
    // Note that only the top bits of the old last element (currElements-1) must be cleared.
    // Any __new__ elements have been cleared by memset() already.
    if (newSize > nBits)
    {
      uint_fast8_t const usedBitsInLastElement = nBits % storage_t_size_in_bit;
      if (usedBitsInLastElement != 0)
      {
        storage_t const mask = ~(std::numeric_limits<storage_t>::max() << usedBitsInLastElement);
        spStorage[currElements - 1U] &= mask;
      }
    }

    // adopt new size
    nBits = newSize;
  }
}

/**
 * \brief Clears all bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void BitField::ClearAll(void) noexcept
{
  size_t const nElements = nbOf_storage_t_elements(nBits);
  memset(spStorage.get(), 0x00, nElements * sizeof(storage_t));
}

/**
 * \brief Sets all bits.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void BitField::SetAll(void) noexcept
{
  size_t const nElements = nbOf_storage_t_elements(nBits);
  memset(spStorage.get(), 0xFF, nElements * sizeof(storage_t));
}

/**
 * \brief Assigns the content of a block of data to the @ref BitField.
 *
 * - - -
 *
 * __Thread-safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception-safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory. There is a guarantee provided that there will be no
 *                          heap allocation under certain conditions. See parameter `newSize`
 *                          for details.
 *
 * __Thread-cancellation-safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newSize
 * New size for the @ref BitField in bit.\n
 * This controls the number of bits copied from `pNewData` into the @ref BitField. \n
 * This method guarantees that there will be no reallocation on heap, if any of the following
 * conditions is true:
 * - `newSize` is zero
 * - `newSize` is equal to the current size of the @ref BitField.
 * \param pNewData
 * Pointer to an array of binary data that shall be copied into the @ref BitField. The LSB of the byte
 * referenced by `pNewData` will be used to initialize bit 0, the LSB of `pNewData+1` will be used to
 * initialize bit 8, and so on.\n
 * _nullptr is not allowed._
 */
void BitField::Assign(size_t const newSize, uint8_t const * const pNewData)
{
  if (pNewData == nullptr)
    throw std::invalid_argument("BitField::Assign: !pNewData");

  // handle special case "zero size"
  if (newSize == 0)
  {
    nBits = 0;
    spStorage.reset();
    return;
  }

  // size changed?
  size_t currElements;
  if (newSize != nBits)
  {
    CheckMax_nBits(newSize);

    // reallocate if necessary
    currElements = nbOf_storage_t_elements(nBits);
    size_t const newElements = nbOf_storage_t_elements(newSize);
    if (currElements != newElements)
    {
      spStorage.reset(new storage_t[newElements]);
      currElements = newElements;
    }

    nBits = newSize;
  }
  else
    currElements = nbOf_storage_t_elements(nBits);

  // copy data into bitfield
  spStorage[currElements - 1U] = 0;
  memcpy(spStorage.get(), pNewData, (nBits + 7U) / 8U);
}

/**
 * \brief Clears a specific bit.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::out_of_range   Parameter `index` is out of bounds.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the bit that shall be cleared. A range check will be applied.
 */
void BitField::ClearBit(size_t const index)
{
  if (index >= nBits)
    throw std::out_of_range("BitField::ClearBit: index too large");

  spStorage[index / storage_t_size_in_bit] &= ~(static_cast<storage_t>(1U) << (index % storage_t_size_in_bit));
}

/**
 * \brief Sets a specific bit.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::out_of_range   Parameter `index` is out of bounds.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the bit that shall be set. A range check will be applied.
 */
void BitField::SetBit(size_t const index)
{
  if (index >= nBits)
    throw std::out_of_range("BitField::SetBit: index too large");

  spStorage[index / storage_t_size_in_bit] |= static_cast<storage_t>(1U) << (index % storage_t_size_in_bit);
}

/**
 * \brief Writes to a specific bit.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::out_of_range   Parameter `index` is out of bounds.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the bit that shall be written. A range check will be applied.
 * \param value
 * Value that shall be written to the bit:\n
 * true = '1'\n
 * false = '0'
 */
void BitField::WriteBit(size_t const index, bool const value)
{
  if (value)
    SetBit(index);
  else
    ClearBit(index);
}

/**
 * \brief Retrieves the state of a specific bit.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::out_of_range   Parameter `index` is out of bounds.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param index
 * Index of the bit whose value shall be retrieved. A range check will be applied.
 * \return
 * Value of the bit addressed by `index`:\n
 * true  = '1'\n
 * false = '0'
 */
bool BitField::GetBit(size_t const index) const
{
  if (index >= nBits)
    throw std::out_of_range("BitField::GetBit: index too large");

  return ((spStorage[index / storage_t_size_in_bit] & (static_cast<storage_t>(1U) << (index % storage_t_size_in_bit))) != 0);
}

/**
 * \brief Locates the first asserted bit starting at a given index searching towards larger indices.
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
 * \param startIndex
 * The search starts at the given index.
 * \return
 * Index of the first asserted bit at or above `startIndex`.\n
 * @ref BitField::NO_BIT if no asserted bit is found, or if `startIndex` refers to beyond the end of the @ref BitField.
 */
size_t BitField::FindFirstSetBit(size_t const startIndex) const noexcept
{
  // beyond end?
  if (startIndex >= nBits)
    return NO_BIT;

  size_t storageIdx = startIndex / storage_t_size_in_bit;
  uint_fast8_t firstBit = startIndex % storage_t_size_in_bit;
  storage_t value = spStorage[storageIdx] & ((std::numeric_limits<storage_t>::max()) << firstBit);
  size_t index = storageIdx * storage_t_size_in_bit;
  while (true)
  {
    int const tz = Compiler::CountTrailingZeros(value);

    if (tz == std::numeric_limits<storage_t>::digits)
    {
      // no '1' found in "value"
      index += storage_t_size_in_bit;
      if (index >= nBits)
        return NO_BIT;

      storageIdx++;
      value = spStorage[storageIdx];
    }
    else
    {
      // '1' found on bit "tz" in "value"
      index += static_cast<size_t>(tz);
      if (index < nBits)
        return index;
      else
        return NO_BIT;
    }
  }

  // never reached, but makes compiler happy
  return NO_BIT;
}

/**
 * \brief Locates the first cleared bit starting at a given index searching towards larger indices.
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
 * \param startIndex
 * The search starts at the given index.
 * \return
 * Index of the first cleared bit at or above `startIndex`.\n
 * @ref BitField::NO_BIT if no asserted bit is found, or if `startIndex` refers to beyond the end of the @ref BitField.
 */
size_t BitField::FindFirstClearedBit(size_t const startIndex) const noexcept
{
  // beyond end?
  if (startIndex >= nBits)
    return NO_BIT;

  size_t storageIdx = startIndex / storage_t_size_in_bit;
  uint_fast8_t firstBit = startIndex % storage_t_size_in_bit;
  storage_t value = spStorage[storageIdx] | (~(std::numeric_limits<storage_t>::max() << firstBit));
  size_t index = storageIdx * storage_t_size_in_bit;
  while (true)
  {
    int const tz = Compiler::CountTrailingOnes(value);

    if (tz == std::numeric_limits<storage_t>::digits)
    {
      // no '0' found in "value"
      index += storage_t_size_in_bit;
      if (index >= nBits)
        return NO_BIT;

      storageIdx++;
      value = spStorage[storageIdx];
    }
    else
    {
      // '0' found on bit "tz" in "value"
      index += static_cast<size_t>(tz);
      if (index < nBits)
        return index;
      else
        return NO_BIT;
    }
  }

  // never reached, but makes compiler happy
  return NO_BIT;
}

/**
 * \brief Locates the first asserted bit starting at a given index searching towards smaller indices.
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
 * \param startIndex
 * The search starts at the given index. If this is out of range, then the value will be limited to the size
 * of the @ref BitField.
 * \return
 * Index of the first asserted bit at or below `startIndex`.\n
 * @ref BitField::NO_BIT if no asserted bit is found.
 */
size_t BitField::FindFirstSetBitReverse(size_t startIndex) const noexcept
{
  if (nBits == 0)
    return NO_BIT;

  if (startIndex >= nBits)
    startIndex = nBits - 1U;

  size_t storageIdx = startIndex / storage_t_size_in_bit;
  uint_fast8_t firstBit = startIndex % storage_t_size_in_bit;

  storage_t value = spStorage[storageIdx];
  if (firstBit != storage_t_size_in_bit - 1U)
    value &= (~(std::numeric_limits<storage_t>::max() << (firstBit + 1U)));

  size_t index = storageIdx * storage_t_size_in_bit;
  while (true)
  {
    int const lz = Compiler::CountLeadingZeros(value);

    if (lz == std::numeric_limits<storage_t>::digits)
    {
      // no '1' found in value
      if (storageIdx == 0)
        return NO_BIT;

      index -= storage_t_size_in_bit;
      storageIdx--;
      value = spStorage[storageIdx];
    }
    else
    {
      // '1' found after "lz" leading zeros in "value"
      index += (storage_t_size_in_bit - lz - 1U);
      return index;
    }
  }

  // never reached, but makes compiler happy
  return NO_BIT;
}

/**
 * \brief Locates the first cleared bit starting at a given index searching towards smaller indices.
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
 * \param startIndex
 * The search starts at the given index. If this is out of range, then the value will be limited to the size
 * of the @ref BitField.
 * \return
 * Index of the first cleared bit at or below `startIndex`.\n
 * @ref BitField::NO_BIT if no asserted bit is found.
 */
size_t BitField::FindFirstClearedBitReverse(size_t startIndex) const noexcept
{
  if (nBits == 0)
    return NO_BIT;

  if (startIndex >= nBits)
    startIndex = nBits - 1U;

  size_t storageIdx = startIndex / storage_t_size_in_bit;
  uint_fast8_t firstBit = startIndex % storage_t_size_in_bit;

  storage_t value = spStorage[storageIdx];
  if (firstBit != storage_t_size_in_bit - 1U)
    value |= (std::numeric_limits<storage_t>::max() << (firstBit + 1U));

  size_t index = storageIdx * storage_t_size_in_bit;
  while (true)
  {
    int const lz = Compiler::CountLeadingOnes(value);

    if (lz == std::numeric_limits<storage_t>::digits)
    {
      // no '0' found in value
      if (storageIdx == 0)
        return NO_BIT;

      index -= storage_t_size_in_bit;
      storageIdx--;
      value = spStorage[storageIdx];
    }
    else
    {
      // '0' found after "lz" leading ones in "value"
      index += (storage_t_size_in_bit - lz - 1U);
      return index;
    }
  }

  // never reached, but makes compiler happy
  return NO_BIT;
}

/**
 * \brief Enumerates all asserted or cleared bits in a human-readable form in an std::string.
 *
 * The indices of the bits are enumerated starting at zero. If there is more than one index, then the
 * indices are separated by comma. Example:\n
 * `1, 3, 4, 5, 8`
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param setNotCleared
 * Controls if asserted or cleared bits shall be enumerated:\n
 * true = asserted bits\n
 * false = cleared bits.
 * \param noWhitespaces
 * Flag controlling if whitespaces behind commata shall not be used in order to compress the string:\n
 * true = indices are separated by "," without any whitespaces\n
 * false = indices are separated by ", " (default)
 * \return
 * An std::string containing an enumeration of the indices of all asserted or cleared bits.
 */
std::string BitField::EnumerateBits(bool const setNotCleared, bool const noWhitespaces) const
{
  std::ostringstream str;

  size_t index = 0;
  bool first = true;
  while (true)
  {
    if (setNotCleared)
      index = FindFirstSetBit(index);
    else
      index = FindFirstClearedBit(index);

    if (index == NO_BIT)
      break;

    if (first)
      first = false;
    else
    {
      if (noWhitespaces)
        str << ",";
      else
        str << ", ";
    }

    str << index;

    index++;
  }

  return str.str();
}

/**
 * \brief Enumerates all asserted or cleared bits in a human-readable form in an std::string and uses
 * range notation for adjacent indices.
 *
 * The indices of the bits are enumerated starting at zero. If there is more than one index, then the
 * indices are separated by comma. Adjacent indices are written using range-notation. Example:\n
 * `1, 3-5, 8`
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out-of-memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param setNotCleared
 * Controls if asserted or cleared bits shall be enumerated:\n
 * true = asserted bits\n
 * false = cleared bits.
 * \param noWhitespaces
 * Flag controlling if whitespaces behind commata shall not be used in order to compress the string:\n
 * true = indices are separated by "," without any whitespaces\n
 * false = indices are separated by ", " (default)
 * \return
 * An std::string containing an enumeration of the indices of all asserted or cleared bits.
 */
std::string BitField::EnumerateBitsCompressed(bool const setNotCleared, bool const noWhitespaces) const
{
  std::ostringstream str;

  size_t firstBit = 0;
  size_t lastBit;
  bool first = true;

  while (true)
  {
    // look for interesting bit
    if (setNotCleared)
      firstBit = FindFirstSetBit(firstBit);
    else
      firstBit = FindFirstClearedBit(firstBit);

    if (firstBit == NO_BIT)
      break;

    // look for first not-interesting bit starting at firstBit+1
    if (setNotCleared)
      lastBit = FindFirstClearedBit(firstBit + 1);
    else
      lastBit = FindFirstSetBit(firstBit + 1);
    if (lastBit != NO_BIT)
      lastBit--;
    else
      lastBit = nBits-1;

    if (first)
      first = false;
    else
    {
      if (noWhitespaces)
        str << ",";
      else
        str << ", ";
    }

    if (firstBit == lastBit)
      str << firstBit;
    else
      str << firstBit << '-' << lastBit;

    firstBit = lastBit + 1;
  }

  return str.str();
}

/**
 * \brief Retrieves a read-only pointer to the internal storage of the @ref BitField.
 *
 * Access to internal storage is useful for implementing efficient bit manipulation, but must be
 * used carefully (see documentation of return value).
 *
 * Example: Combine bits from multiple bit fields logically with each other
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
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * Note that the retrieved pointer can be used to read the object later.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pSize
 * The size of the internal storage in number of @ref storage_t elements is written into the referenced variable.\n
 * nullptr is allowed, if the caller is not interested in the size, e.g. if multiple BitFields with the same size
 * are involved (see example above). The internal storage of different @ref BitField instances with the same size (in
 * bit) always have the same size (in @ref storage_t elements).
 * \return
 * Pointer to the internal storage of the @ref BitField. \n
 * The pointer is valid until the @ref BitField is destroyed, resized (@ref Resize()) or until new content is
 * assigned to the @ref BitField.
 */
BitField::storage_t const * BitField::GetInternalStorage(size_t * const pSize) const noexcept
{
  return const_cast<BitField*>(this)->GetInternalStorage(pSize);
}

/**
 * \brief Retrieves a pointer to the internal storage of the @ref BitField.
 *
 * Access to internal storage is useful for implementing efficient bit manipulation, but must be
 * used carefully (see documentation of return value).
 *
 * Example: Combine bits from multiple bit fields logically with each other
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
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.\n
 * Note: Clients may manipulate the object's state through the pointer retrieved from this method.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pSize
 * The size of the internal storage in number of @ref storage_t elements is written into the referenced variable.\n
 * nullptr is allowed, if the caller is not interested in the size, e.g. if multiple BitFields with the same size
 * are involved (see example above). The internal storage of different @ref BitField instances with the same size (in
 * bit) always have the same size (in @ref storage_t elements).
 * \return
 * Pointer to the internal storage of the @ref BitField. \n
 * The pointer is valid until the @ref BitField is destroyed, resized (@ref Resize()) or until new content is
 * assigned to the @ref BitField.
 */
BitField::storage_t * BitField::GetInternalStorage(size_t * const pSize) noexcept
{
  if (pSize != nullptr)
    *pSize = nbOf_storage_t_elements(nBits);
  return spStorage.get();
}

/**
 * \brief Checks a given value indicating the size of a @ref BitField for upper bound violation.
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
 * \param _nBits
 * Value to be checked.\n
 * If the value is invalid, then an exception (std::length_error) will be thrown.
 */
void BitField::CheckMax_nBits(size_t const _nBits) const
{
  if (_nBits > std::numeric_limits<size_t>::max() - (storage_t_size_in_bit - 1U))
    throw std::length_error("BitField::CheckMax_nBits");
}

/**
 * \brief Calculates then number of @ref storage_t elements required to store a given number of bits.
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
 * \param _nBits
 * Number of bits.
 * \return
 * Number of @ref storage_t elements required to store `_nBits` bits.
 */
size_t BitField::nbOf_storage_t_elements(size_t const _nBits) const noexcept
{
  return (_nBits + (storage_t_size_in_bit - 1U)) / storage_t_size_in_bit;
}

} // namespace container
} // namespace gpcc
