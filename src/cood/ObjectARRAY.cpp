/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#include "ObjectARRAY.hpp"
#include "IObjectNotifiable.hpp"
#include "exceptions.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include <cstring>
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \brief Constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param _name
 * Name for the object.
 *
 * \param _attributesSI0
 * Attributes for subindex 0.\n
 * There must be at least one read-permission set.
 *
 * \param _SI0
 * Initial value for SI0.\n
 * SI0 indicates the number of subindices representing array elements and therefore the number of array elements.\n
 * The allowed range is 0-255.\n
 * This must also be within the range given by `_min_SI0` and `_max_SI0`.
 *
 * \param _min_SI0
 * Minimum value for SI0. Allowed range: 0-255.\n
 * This must be equal to or less than `_max_SI0`.
 *
 * \param _max_SI0
 * Maximum value for SI0. Allowed range: 0-255.\n
 * This must be equal to or larger than `_min_SI0`.
 *
 * \param _type
 * CANopen data type of the array elements represented by the object. Since the constructed object is an ARRAY
 * object, the object itself will have the same data type as the data represented by it.\n
 * \n
 * The type of the native data referenced by `_pData` must match the CANopen data type. The documentation of the
 * @ref DataType enumeration contains a list of native types associated with each CANopen data type.\n
 * \n
 * Note that if '_type' is any of the types bit1..bit8 or boolean_native_bit1, then the native data must be an array of
 * data type uint8_t and the bits will be stuffed together inside the native array.\n
 * Example:\n
 * An array of 18 elements of type `bit2` would be comprised of 36 bits. The native array would occupy 5 native
 * elements of type uint8_t and the upper four bits of the last element would be unused.
 *
 * \param _attributes
 * Attributes for the subindices representing array elements.\n
 * The attributes are applied to all subindices except for SI0.\n
 * At least one read- or write-permission must be set.
 *
 * \param _pData
 * Pointer to the memory location containing the data represented by the ARRAY object.\n
 * The memory is provided and owned by the owner of the ARRAY object.\n
 * The memory location must be valid during the life-time of the ARRAY object or until a different
 * memory location is configured via @ref SetData(). \n
 * The memory must contain `_max_SI0` elements of data type `_type`.\n
 * See parameter `_type` for details.
 *
 * \param _pMutex
 * Pointer to a mutex protecting access to SI0 and to the native data referenced by `_pData`.\n
 * The mutex is optional. If the whole object is read-only __and__ if the application does not modify the
 * value of SI0 and if the application does not modify the data referenced by `_pData`, then a mutex is not required
 * and this parameter may be `nullptr`.\n
 * __BUT__ if SI0 or the array data represented by the object is writeable, or if the application may modify SI0, or
 * if the application may modify the native data referenced by `_pData`, then a mutex must be specified.\n
 * \n
 * The application must obey the following rules when accessing the native data referenced by `_pData`:
 * - If the array data is READ-ONLY, then the application must lock the mutex only if it wants to modify the data
 *   referenced by `_pData`. The application does not need to lock the mutex for reading the data referenced by
 *   `_pData` in this case.
 * - If the array data is READ-WRITE, then the application must lock the mutex ALWAYS when it wants to read or write
 *   the data referenced by `_pData`.
 *
 * \param _pNotifiable
 * Pointer to a @ref IObjectNotifiable interface that shall be used to deliver callbacks to the owner of the object.\n
 * nullptr is allowed.
 */
ObjectARRAY::ObjectARRAY(std::string            const & _name,
                         attr_t                 const   _attributesSI0,
                         uint8_t                const   _SI0,
                         uint8_t                const   _min_SI0,
                         uint8_t                const   _max_SI0,
                         DataType               const   _type,
                         attr_t                 const   _attributes,
                         void*                  const   _pData,
                         gpcc::osal::Mutex *    const   _pMutex,
                         IObjectNotifiable *    const   _pNotifiable)
: Object()
, name(_name)
, attributesSI0(_attributesSI0)
, SI0(_SI0)
, min_SI0(_min_SI0)
, max_SI0(_max_SI0)
, type(_type)
, attributes(_attributes)
, pData(_pData)
, pMutex(_pMutex)
, pNotifiable(_pNotifiable)
{
  // SI0 must have at least read permission set
  if ((attributesSI0 & attr_ACCESS_RD) == 0U)
    throw std::invalid_argument("ObjectARRAY::ObjectARRAY: '_attributesSI0' must have at least one read-permission set");

  // check number of SIs
  if (   (min_SI0 > max_SI0)
      || (SI0 < min_SI0)
      || (SI0 > max_SI0))
  {
    throw std::invalid_argument("ObjectARRAY::ObjectARRAY: \"_SI0 <= _min_SI0 <= _max_SI0\" violated");
  }

  // data type supported?
  if (   (type == DataType::visible_string)
      || (type == DataType::octet_string)
      || (type == DataType::unicode_string)
      || (DataTypeBitLengthTable[static_cast<int>(type)] == 0U)
      || (NativeDataTypeBitLengthTable[static_cast<int>(type)] == 0U))
  {
    throw DataTypeNotSupportedError(type);
  }

  // at least one read or write permission specified for array's data?
  if ((attributes & attr_ACCESS_RW) == 0U)
    throw std::invalid_argument("ObjectARRAY::ObjectARRAY: No read- or write-permissions set in '_attributes'");

  if (pData == nullptr)
    throw std::invalid_argument("ObjectARRAY::ObjectARRAY: '_pData' is nullptr");

  // check: a mutex must be specified if write access is possible
  if ((((attributesSI0 & attr_ACCESS_WR) != 0U) || ((attributes & attr_ACCESS_WR) != 0U)) && (pMutex == nullptr))
    throw std::logic_error("ObjectARRAY::ObjectARRAY: Object with write-permission requires a mutex");
}

/**
 * \brief Updates the value of subindex 0 and the array data represented by the object.
 *
 * \pre   A mutex for protecting the data has been passed to the constructor.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * The mutex associated with the data represented by the object __must not be locked__.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param newSI0
 * New value for subindex 0.\n
 * This must match the bounds for SI0 that have been passed to the constructor (`_min_SI0` and `_max_SI0`).
 *
 * \param pNewData
 * Pointer to the (new) memory location containing the data that shall be represented by the ARRAY object.\n
 * nullptr is not allowed.\n
 * A pointer referencing to the currently configured memory is allowed if the array's data shall not
 * be updated.\n
 * The referenced memory shall be provided and owned by the owner of the ARRAY object.\n
 * The memory location must be valid during the life-time of the ARRAY object or until a different
 * memory location is configured via @ref SetData(). \n
 * The number of data elements in the referenced memory and their type must meet the parameters
 * `_max_SI0` and `_type` that have been passed to the constructor.
 */
void ObjectARRAY::SetData(uint8_t const newSI0, void* const pNewData)
{
  if (pMutex == nullptr)
    throw std::logic_error("ObjectARRAY::SetData: Operation requires that a mutex has been passed to the constructor");

  if ((newSI0 < min_SI0) || (newSI0 > max_SI0))
    throw std::invalid_argument("ObjectARRAY::SetData: 'newSI0' is out of min/max for SI0");

  if (pNewData == nullptr)
    throw std::invalid_argument("ObjectARRAY::SetData: 'pNewData' is nullptr");

  gpcc::osal::MutexLocker mutexLocker(*pMutex);
  SI0   = newSI0;
  pData = pNewData;
}

// <-- base class Object

/// \copydoc Object::GetObjectCode
Object::ObjectCode ObjectARRAY::GetObjectCode(void) const noexcept
{
  return ObjectCode::Array;
}

/// \copydoc Object::GetObjectDataType
DataType ObjectARRAY::GetObjectDataType(void) const noexcept
{
  return MapAlternativeDataTypesToOriginalTypes(type);
}

/// \copydoc Object::GetObjectName
std::string ObjectARRAY::GetObjectName(void) const
{
  return name;
}

/// \copydoc Object::GetMaxNbOfSubindices
uint16_t ObjectARRAY::GetMaxNbOfSubindices(void) const noexcept
{
  return static_cast<uint_fast16_t>(max_SI0) + 1U; // +1 for SI0
}

/// \copydoc Object::IsSubIndexEmpty
bool ObjectARRAY::IsSubIndexEmpty(uint8_t const subIdx) const
{
  if (subIdx > max_SI0)
    throw SubindexNotExistingError();

  return false;
}

/// \copydoc Object::GetSubIdxDataType
DataType ObjectARRAY::GetSubIdxDataType(uint8_t const subIdx) const
{
  if (subIdx > max_SI0)
    throw SubindexNotExistingError();

  if (subIdx == 0U)
    return DataType::unsigned8;
  else
    return MapAlternativeDataTypesToOriginalTypes(type);
}

/// \copydoc Object::GetSubIdxAttributes
Object::attr_t ObjectARRAY::GetSubIdxAttributes(uint8_t const subIdx) const
{
  if (subIdx > max_SI0)
    throw SubindexNotExistingError();

  if (subIdx == 0U)
    return attributesSI0;
  else
    return attributes;
}

/// \copydoc Object::GetSubIdxMaxSize
size_t ObjectARRAY::GetSubIdxMaxSize(uint8_t const subIdx) const
{
  if (subIdx == 0U)
  {
    return 8U;
  }
  else
  {
    if (subIdx > max_SI0)
      throw SubindexNotExistingError();

    return DataTypeBitLengthTable[static_cast<int>(type)];
  }
}

/// \copydoc Object::GetSubIdxName
std::string ObjectARRAY::GetSubIdxName(uint8_t const subIdx) const
{
  if (subIdx > max_SI0)
    throw SubindexNotExistingError();

  if (subIdx == 0U)
  {
    return "Number of subindices";
  }
  else
  {
    std::string s("Subindex ");
    s += std::to_string(subIdx);
    return s;
  }
}

/// \copydoc Object::LockData
gpcc::osal::MutexLocker ObjectARRAY::LockData(void) const
{
  return gpcc::osal::MutexLocker(pMutex);
}

/// \copydoc Object::GetObjectStreamSize
size_t ObjectARRAY::GetObjectStreamSize(bool const SI016Bits) const noexcept
{
  uint_fast16_t sizeInBit = static_cast<uint_fast16_t>(DataTypeBitLengthTable[static_cast<int>(type)]) * SI0;
  if (SI016Bits)
    sizeInBit += 16U;
  else
    sizeInBit += 8U;
  return sizeInBit;
}

/// \copydoc Object::GetNbOfSubIndices
uint16_t ObjectARRAY::GetNbOfSubIndices(void) const noexcept
{
  return static_cast<uint_fast16_t>(SI0) + 1U; // +1 for SI0
}

/// \copydoc Object::GetSubIdxActualSize
size_t ObjectARRAY::GetSubIdxActualSize(uint8_t const subIdx) const
{
  if (subIdx == 0)
  {
    return 8U;
  }
  else
  {
    if (subIdx > SI0)
      throw SubindexNotExistingError();

    return DataTypeBitLengthTable[static_cast<int>(type)];
  }
}

/// \copydoc Object::Read
SDOAbortCode ObjectARRAY::Read(uint8_t const subIdx,
                               attr_t const permissions,
                               gpcc::Stream::IStreamWriter & isw) const
{
  if (subIdx > SI0)
    return SDOAbortCode::SubindexDoesNotExist;

  if (subIdx == 0U)
  {
    // (subindex 0 shall be read)

    // check permissions
    if ((permissions & attr_ACCESS_RD & attributesSI0) == 0U)
      return SDOAbortCode::AttemptToReadWrOnlyObject;

    // invoke before-read-callback
    if (pNotifiable != nullptr)
    {
      auto const result = pNotifiable->OnBeforeRead(this, 0, false, false);
      if (result != SDOAbortCode::OK)
        return result;
    }

    // read SI0
    isw.Write_uint8(SI0);
  }
  else
  {
    // (subindex 0 shall not be read)

    // check permissions
    if ((permissions & attr_ACCESS_RD & attributes) == 0U)
      return SDOAbortCode::AttemptToReadWrOnlyObject;

    // invoke before-read-callback
    if (pNotifiable != nullptr)
    {
      auto const result = pNotifiable->OnBeforeRead(this, subIdx, false, false);
      if (result != SDOAbortCode::OK)
        return result;
    }

    if (IsNativeDataStuffed(type))
    {
      // (data type is bit-based and native data is stuffed)

      uint8_t const bits = ReadBitsFromMem(subIdx);
      NativeDataToCANopenEncodedData(&bits, type, 1U, false, isw);
    }
    else
    {
      // (data type may be bit-based, but native data is NOT stuffed)

      uint_fast16_t const nativeSizeInByte = NativeDataTypeBitLengthTable[static_cast<int>(type)] / 8U;
      uint8_t const * const ptr = static_cast<uint8_t const *>(pData) + ((subIdx - 1U) * nativeSizeInByte);
      NativeDataToCANopenEncodedData(ptr, type, 1U, false, isw);
    }
  }

  return SDOAbortCode::OK;
}

/// \copydoc Object::Write
SDOAbortCode ObjectARRAY::Write(uint8_t const subIdx,
                                attr_t const permissions,
                                gpcc::Stream::IStreamReader & isr)
{
  if (subIdx > SI0)
    return SDOAbortCode::SubindexDoesNotExist;

  if (subIdx == 0)
  {
    // (subindex 0 shall be written)

    // check permissions
    if ((permissions & attr_ACCESS_WR & attributesSI0) == 0U)
      return SDOAbortCode::AttemptToWriteRdOnlyObject;

    // read the data that shall be written to SI0 into a temporary variable
    uint8_t data;
    try
    {
      data = isr.Read_uint8();
      isr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::sevenOrLess);
    }
    catch (gpcc::Stream::EmptyError const &)
    {
      return SDOAbortCode::DataTypeMismatchTooSmall;
    }
    catch (gpcc::Stream::RemainingBitsError const &)
    {
      return SDOAbortCode::DataTypeMismatchTooLong;
    }

    // check range
    if (data < min_SI0)
      return SDOAbortCode::ValueTooLow;
    if (data > max_SI0)
      return SDOAbortCode::ValueTooHigh;

    // invoke before-write-callback
    if (pNotifiable != nullptr)
    {
      auto const result = pNotifiable->OnBeforeWrite(this, 0, false, 0, &data);
      if (result != SDOAbortCode::OK)
        return result;
    }

    // finally write to SI0
    SI0 = data;
  }
  else
  {
    // (subindex 0 shall not be written)

    // check permissions
    if ((permissions & attr_ACCESS_WR & attributes) == 0U)
      return SDOAbortCode::AttemptToWriteRdOnlyObject;

    // determine number of bytes required to store the data that shall be written in native format
    // (cannot be zero, ensured by constructor)
    uint_fast16_t const nativeSizeInByte = NativeDataTypeBitLengthTable[static_cast<int>(type)] / 8U;

    // allocate some temporary storage (pTempMem) either on the stack or on the heap
    uint64_t localMem;
    uint8_t* pTempMem;
    if (nativeSizeInByte <= sizeof(localMem))
      pTempMem = reinterpret_cast<uint8_t*>(&localMem);
    else
      pTempMem = new uint8_t[nativeSizeInByte];

    ON_SCOPE_EXIT(releaseTempMem)
    {
      if (pTempMem != reinterpret_cast<uint8_t*>(&localMem))
        delete [] pTempMem;
    };

    // read the data that shall be written to the subindex into pTempMem
    try
    {
      CANopenEncodedDataToNativeData(isr, type, 1U, false, pTempMem);
      isr.EnsureAllDataConsumed(gpcc::Stream::IStreamReader::RemainingNbOfBits::sevenOrLess);
    }
    catch (gpcc::Stream::EmptyError const &)
    {
      return SDOAbortCode::DataTypeMismatchTooSmall;
    }
    catch (gpcc::Stream::RemainingBitsError const &)
    {
      return SDOAbortCode::DataTypeMismatchTooLong;
    }

    // invoke before-write-callback
    if (pNotifiable != nullptr)
    {
      auto const result = pNotifiable->OnBeforeWrite(this, subIdx, false, 0, pTempMem);
      if (result != SDOAbortCode::OK)
        return result;
    }

    // finally write the data to the subindex
    if (IsNativeDataStuffed(type))
    {
      // (data type is bit-based and native data is stuffed)

      WriteBitsToMem(subIdx, *pTempMem);
    }
    else
    {
      // (data type may be bit-based, but native data is NOT stuffed)

      uint8_t* const ptr = static_cast<uint8_t*>(pData) + ((subIdx - 1U) * nativeSizeInByte);
      memcpy(ptr, pTempMem, nativeSizeInByte);
    }
  }

  // invoke after-write-callback
  try
  {
    if (pNotifiable != nullptr)
      pNotifiable->OnAfterWrite(this, subIdx, false);
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("ObjectARRAY::Write: After-write-callback threw: ", e);
  }
  catch (...)
  {
    PANIC();
  }

  return SDOAbortCode::OK;
}

/// \copydoc Object::CompleteRead
SDOAbortCode ObjectARRAY::CompleteRead(bool const inclSI0,
                                       bool const SI016Bits,
                                       attr_t const permissions,
                                       gpcc::Stream::IStreamWriter & isw) const
{
  // If subindex 0 is included, then check the access permission for subindex 0.
  // Note: Subindex 0 is never pure write-only (ensured by constructor).
  if ((inclSI0) && ((attributesSI0 & attr_ACCESS_RD & permissions) == 0U))
    return SDOAbortCode::AttemptToReadWrOnlyObject;

  // special case: SI0 is not included and SI0 is zero
  if ((!inclSI0) && (SI0 == 0U))
    return SDOAbortCode::OK;

  // determine if the other subindices are pure write-only
  bool const dataPureWriteOnly = ((attributes & attr_ACCESS_RD) == 0U);

  // If there are other subindices, then check the access permissions for the other subindices.
  if ((SI0 != 0U) &&
      (!dataPureWriteOnly) &&
      ((attributes & attr_ACCESS_RD & permissions) == 0U))
    return SDOAbortCode::AttemptToReadWrOnlyObject;

  // call the before-read-callback
  if (pNotifiable != nullptr)
  {
    auto const result = pNotifiable->OnBeforeRead(this, inclSI0 ? 0U : 1U, true, false);
    if (result != SDOAbortCode::OK)
      return result;
  }

  // read subindex 0
  if (inclSI0)
  {
    if (SI016Bits)
      isw.Write_uint16(SI0);
    else
      isw.Write_uint8(SI0);
  }

  // special case: no data that could be read
  if (SI0 == 0U)
    return SDOAbortCode::OK;

  // is the data pure write-only?
  if (dataPureWriteOnly)
  {
    // (data is pure write-only; pure write-only subindices read as zero)

    // calculate number of bits
    uint_fast16_t const nBits = static_cast<uint_fast16_t>(DataTypeBitLengthTable[static_cast<int>(type)]) * SI0;

    if (IsDataTypeBitBased(type))
    {
      isw.FillBits(nBits, false);
    }
    else
    {
      uint_fast16_t const nBytes = nBits / 8U;
      isw.FillBytes(nBytes, 0U);
    }
  }
  else
  {
    // (data is not pure write-only)

    NativeDataToCANopenEncodedData(pData, type, SI0, true, isw);
  }

  return SDOAbortCode::OK;
}

/// \copydoc Object::CompleteWrite
SDOAbortCode ObjectARRAY::CompleteWrite(bool const inclSI0,
                                        bool const SI016Bits,
                                        attr_t const permissions,
                                        gpcc::Stream::IStreamReader & isr,
                                        gpcc::Stream::IStreamReader::RemainingNbOfBits const ernob)
{
  // determine if SI0 is pure read-only
  bool const SI0pureReadOnly = ((attributesSI0 & attr_ACCESS_WR) == 0U);

  // If subindex 0 is included, then check the access permission for subindex 0.
  if ((inclSI0) &&
      (!SI0pureReadOnly) &&
      ((attributesSI0 & attr_ACCESS_WR & permissions) == 0U))
    return SDOAbortCode::AttemptToWriteRdOnlyObject;

  // determine if data is pure read-only
  bool const dataPureReadOnly = ((attributes & attr_ACCESS_WR) == 0U);

  // Prepare a pointer to temporary storage for preview data. The storage will be allocated later.
  uint8_t* pTempMem = nullptr;
  ON_SCOPE_EXIT(releaseTempMem) { delete [] pTempMem; };

  uint16_t newSI0 = SI0;

  try
  {
    // Read the new value for SI0 from isr. If SI0 is not included in the write-operation, or if SI0 is
    // pure read-only, then the "new" value for SI0 will be the "current" value of SI0.
    if (inclSI0)
    {
      // read SI0 from 'isr'
      if (SI016Bits)
        newSI0 = isr.Read_uint16();
      else
        newSI0 = isr.Read_uint8();

      // SI0 pure read-only?
      if (SI0pureReadOnly)
      {
        // the new value for SI0 MUST MATCH the current value of SI0.

        if (newSI0 != SI0)
          return SDOAbortCode::UnsupportedAccessToObject;
      }
      else
      {
        // the new value for SI0 must match bounds for SI0
        if (newSI0 < min_SI0)
          return SDOAbortCode::ValueTooLow;
        else if (newSI0 > max_SI0)
          return SDOAbortCode::ValueTooHigh;
      }
    }

    // if any subindex will be written, then check the permissions for accessing the subindices
    if ((newSI0 != 0U) &&
        (!dataPureReadOnly) &&
        ((attributes & attr_ACCESS_WR & permissions) == 0U))
      return SDOAbortCode::AttemptToWriteRdOnlyObject;

    // determine number of bytes required to store the data that shall be written in native format (excl. SI0)
    uint_fast16_t nBytesNative;
    if (IsNativeDataStuffed(type))
      nBytesNative = ((static_cast<uint_fast16_t>(DataTypeBitLengthTable[static_cast<int>(type)]) * newSI0) + 7U) / 8U;
    else
      nBytesNative = static_cast<uint_fast16_t>(NativeDataTypeBitLengthTable[static_cast<int>(type)] / 8U) * newSI0;

    // allocate temporary storage used to preview the data that shall be written
    if (nBytesNative != 0U)
      pTempMem = new uint8_t[nBytesNative];

    // read the data from 'isr'
    if (newSI0 != 0U)
      CANopenEncodedDataToNativeData(isr, type, newSI0, true, pTempMem);

    isr.EnsureAllDataConsumed(ernob);
  }
  catch (gpcc::Stream::EmptyError const &)
  {
    return SDOAbortCode::DataTypeMismatchTooSmall;
  }
  catch (gpcc::Stream::RemainingBitsError const &)
  {
    return SDOAbortCode::DataTypeMismatchTooLong;
  }

  // invoke before-write-callback
  if (pNotifiable != nullptr)
  {
    auto const result = pNotifiable->OnBeforeWrite(this, inclSI0 ? 0U : 1U, true, inclSI0 ? newSI0 : 0U, pTempMem);
    if (result != SDOAbortCode::OK)
      return result;
  }

  // write to SI0
  SI0 = newSI0;

  // write the other subindices
  if ((SI0 != 0U) && (!dataPureReadOnly))
  {
    // calculate the number of bits to be copied
    uint_fast16_t nBitsToBeCopied;
    if (IsNativeDataStuffed(type))
      nBitsToBeCopied = static_cast<uint_fast16_t>(DataTypeBitLengthTable[static_cast<int>(type)]) * SI0;
    else
      nBitsToBeCopied = static_cast<uint_fast16_t>(NativeDataTypeBitLengthTable[static_cast<int>(type)]) * SI0;

    // calculate the number of whole bytes and remaining bits to be copied
    uint_fast16_t nBytesToBeCopied = nBitsToBeCopied / 8U;
    nBitsToBeCopied %= 8U;

    // copy whole bytes
    if (nBytesToBeCopied != 0U)
      memcpy(pData, pTempMem, nBytesToBeCopied);

    // any bits left to be copied?
    if (nBitsToBeCopied != 0U)
    {
      // copy the remaining bits
      uint8_t* const pLastByte = static_cast<uint8_t*>(pData) + nBytesToBeCopied;
      uint8_t const mask = (1U << nBitsToBeCopied) - 1U;
      *pLastByte = (*pLastByte & (~mask)) | (pTempMem[nBytesToBeCopied] & mask);
    }
  }

  // invoke after-write-callback
  try
  {
    if (pNotifiable != nullptr)
      pNotifiable->OnAfterWrite(this, inclSI0 ? 0U : 1U, true);
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("ObjectARRAY::CompleteWrite: After-write-callback threw: ", e);
  }
  catch (...)
  {
    PANIC();
  }

  return SDOAbortCode::OK;
}
// --> base class Object

/**
 * \brief Reads the bits represented by a subindex from @ref pData (native format).
 *
 * \pre   The size of the array elements must be 1..8 bits, and the native data must be stuffed
 *        (= Data types bit1..bit8 and boolean_native_bit1).
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref pMutex must be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose data shall be read. Allowed range: 1..SI0. There is no bounds check.
 *
 * \return
 * Bits represented by subindex 'subIdx', read from the memory referenced by @ref pData. \n
 * The read bits are aligned to the LSB. Upper unused bits will be zero.
 */
uint8_t ObjectARRAY::ReadBitsFromMem(uint8_t const subIdx) const
{
  uint8_t const arrayElementSizeInBit = DataTypeBitLengthTable[static_cast<int>(type)];

  // check precondition
  if ((arrayElementSizeInBit < 1U) || (arrayElementSizeInBit > 8U))
    throw std::logic_error("ObjectARRAY::ReadBitsFromMem: Incompatible data type");

  // calculate the bit-offset of the first bit to be read inside the array
  uint_fast16_t bitOffset = static_cast<uint_fast16_t>(subIdx - 1U) * arrayElementSizeInBit;

  // get a pointer to the byte containing the first bit to be read
  uint8_t const * const ptr = static_cast<uint8_t const*>(pData) + (bitOffset / 8U);

  // calculate the offset of the first bit to be read inside the byte referenced by ptr
  bitOffset %= 8U;

  // load bits
  uint_fast8_t bits = ptr[0] >> bitOffset;

  // calculate the number of bits loaded into 'bits'
  uint_fast8_t const loadedBits = 8U - bitOffset;

  // if the next byte also contains some bits, then read them too
  // (bits dropped above bit 7 when shifting up are don't care)
  if (loadedBits < arrayElementSizeInBit)
    bits = bits | (ptr[1] << loadedBits);

  // clear all uninteresting bits
  bits &= ((1U << arrayElementSizeInBit) - 1U);

  return bits;
}

/**
 * \brief Writes the bits represented by a subindex into @ref pData (native format).
 *
 * \pre   The size of the array elements must be 1..8 bits, and the native data must be stuffed
 *        (= Data types bit1..bit8 and boolean_native_bit1).
 *
 * - - -
 *
 * __Thread safety:__\n
 * @ref pMutex must be locked.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose data shall be written. Allowed range: 1..SI0. There is no bounds check.
 *
 * \param data
 * The bits that shall be written. The bits must be aligned to the LSB. Upper unused bits will be ignored.
 */
void ObjectARRAY::WriteBitsToMem(uint8_t const subIdx, uint8_t const data)
{
  uint8_t const arrayElementSizeInBit = DataTypeBitLengthTable[static_cast<int>(type)];

  // check precondition
  if ((arrayElementSizeInBit < 1U) || (arrayElementSizeInBit > 8U))
    throw std::logic_error("ObjectARRAY::WriteBitsToMem: Incompatible data type");

  // calculate the bit-offset of the first bit to be written inside the array
  uint_fast16_t bitOffset = static_cast<uint_fast16_t>(subIdx - 1U) * arrayElementSizeInBit;

  // get a pointer to the byte containing the first bit to be written
  uint8_t * const ptr = static_cast<uint8_t*>(pData) + (bitOffset / 8U);

  // calculate the offset of the first bit to be written inside the byte referenced by ptr
  bitOffset %= 8U;

  // load bits
  uint_fast16_t bits = ptr[0];

  // calculate the number of bits loaded into 'bits' starting at bitOffset
  uint_fast8_t const loadedBits = 8U - bitOffset;

  // if the next byte also contains some bits, then read them too
  if (loadedBits < arrayElementSizeInBit)
    bits = bits | (static_cast<uint_fast16_t>(ptr[1]) << 8U);

  uint_fast16_t const mask = ((1U << arrayElementSizeInBit) - 1U);

  // clear bits
  bits &= ~(mask << bitOffset);

  // write bits
  bits |= (data & mask) << bitOffset;

  // write back to ptr
  ptr[0] = static_cast<uint8_t>(bits);

  // if the next byte also contains some bits, then write them too
  if (loadedBits < arrayElementSizeInBit)
    ptr[1] = static_cast<uint8_t>(bits >> 8U);
}

} // namespace cood
} // namespace gpcc
