/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2019 Daniel Jerolm
*/

#include <gpcc/cood/ObjectRECORD.hpp>
#include <gpcc/cood/IObjectNotifiable.hpp>
#include <gpcc/cood/exceptions.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/stream/StreamErrors.hpp>
#include <cstring>
#include <limits>
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
 * \param _SI0
 * Value for SI0.\n
 * SI0 indicates the number of subindices representing record elements.\n
 * The allowed range is 0-255.
 *
 * \param _pStruct
 * Pointer to the memory location containing the data represented by the RECORD object.\n
 * nullptr is not allowed.\n
 * The memory shall be provided and owned by the creator of the RECORD object.\n
 * The memory location must be valid during the life-time of the RECORD object or until a different
 * memory location is configured via @ref SetData().
 *
 * \param _structsNativeSizeInByte
 * Size of the structure referenced by `_pStruct` in bytes.\n
 * The value shall contain any padding bytes or gap bytes required by the native rules for data representation
 * and memory layout.
 *
 * \param _pMutex
 * Pointer to a mutex protecting access to the native data referenced by `_pStruct`.\n
 * The mutex is optional. If all subindices are read-only __and__ if the application does not modify the
 * data referenced by `_pStruct`, then a mutex is not required and this parameter may be `nullptr`.\n
 * __BUT__ if at least one subindex is writeable, or if the application may modify the native data
 * referenced by `_pStruct`, then a mutex must be specified.
 *
 * \param _pSIDescriptions
 * Pointer to an array of @ref SubIdxDescr structures describing the subindices of the RECORD object.\n
 * The number of entries contained in the array must match the value of '_SI0'.\n
 * The array must be valid and constant during the life-time of the RECORD object.
 *
 * \param _pNotifiable
 * Pointer to a @ref IObjectNotifiable interface that shall be used to deliver callbacks to the owner of the object.\n
 * nullptr is allowed.
 */
ObjectRECORD::ObjectRECORD(std::string            const & _name,
                           uint8_t                const   _SI0,
                           void*                  const   _pStruct,
                           size_t                 const   _structsNativeSizeInByte,
                           gpcc::osal::Mutex*     const   _pMutex,
                           SubIdxDescr const *    const   _pSIDescriptions,
                           IObjectNotifiable *    const   _pNotifiable)
: Object()
, name(_name)
, SI0(_SI0)
, pStruct(_pStruct)
, structsNativeSizeInByte(_structsNativeSizeInByte)
, pMutex(_pMutex)
, pSIDescriptions(_pSIDescriptions)
, pNotifiable(_pNotifiable)
, streamSizeInBit(0)
{
  if (pStruct == nullptr)
    throw std::invalid_argument("ObjectRECORD::ObjectRECORD: '_pStruct' is nullptr");

  if (structsNativeSizeInByte > (static_cast<size_t>(std::numeric_limits<decltype(SubIdxDescr::byteOffset)>::max()) + 1U))
    throw std::invalid_argument("ObjectRECORD::ObjectRECORD: '_structsNativeSizeInByte' exceeds max. byte offset in SubIdxDescr");

  if (pSIDescriptions == nullptr)
    throw std::invalid_argument("ObjectRECORD::ObjectRECORD: '_pSIDescriptions' is nullptr");

  // loop through all subindices
  bool anyWriteable = false;
  bool prevSIwasGap = false;
  for (uint_fast8_t i = 0; i < SI0; i++)
  {
    SubIdxDescr const * const pSIDescr = &pSIDescriptions[i];

    // treat "normal" subindices, empty subindices and gap subindices separately
    if (pSIDescr->type == DataType::null)
    {
      // (empty or gap)

      if (pSIDescr->nElements == 0U)
      {
        // (empty subindex)
        if (   (pSIDescr->name != nullptr)
            || (pSIDescr->attributes != 0U)
            || (pSIDescr->byteOffset != 0U)
            || (pSIDescr->bitOffset  != 0U))
        {
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Invalid description of empty subindex");
        }
      }
      else
      {
        // (gap subindex)
        if (   (pSIDescr->name == nullptr)
            || ((pSIDescr->attributes & attr_ACCESS_RW) == 0U)
            || (pSIDescr->byteOffset != 0U)
            || (pSIDescr->bitOffset  != 0U))
        {
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Invalid description of gap subindex");
        }

        if (prevSIwasGap)
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Adjacent gap subindices");

        prevSIwasGap = true;
        streamSizeInBit += pSIDescr->nElements;
      }
    } // if (pSIDescr->type == DataType::null)
    else
    {
      // ("normal" subindex)

      // name supplied?
      if (pSIDescr->name == nullptr)
        throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex has no name");

      // data type supported?
      auto const bitLength = DataTypeBitLengthTable[static_cast<int>(pSIDescr->type)];
      auto const nativeBitLength = NativeDataTypeBitLengthTable[static_cast<int>(pSIDescr->type)];
      if ((bitLength == 0U) || (nativeBitLength == 0U))
        throw DataTypeNotSupportedError(pSIDescr->type);

      // at least one read or write permission specified?
      if ((pSIDescr->attributes & attr_ACCESS_RW) == 0U)
        throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex has no read- or write-permission set");

      // check nElements depending on data type (array / non-array)
      if (   (pSIDescr->type == DataType::visible_string)
          || (pSIDescr->type == DataType::octet_string)
          || (pSIDescr->type == DataType::unicode_string))
      {
        // (array data type)

        if (   (pSIDescr->nElements == 0U)
            || (pSIDescr->nElements > (0xFFFEU / bitLength)))
        {
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Bit-size of subindex is zero or exceeds 65534 ('nElements' out of range)");
        }
      }
      else
      {
        // (non-array data type)

        if (pSIDescr->nElements != 1U)
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex has invalid 'nElements'");
      }

      // bitOffset must be 0..7 for bit based native data and zero for byte based native data
      if (IsNativeDataStuffed(pSIDescr->type))
      {
        if (pSIDescr->bitOffset > 7U)
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex has invalid 'bitOffset'");

        // any bytes outside the native data structure referenced?
        if ((static_cast<uint_fast32_t>(pSIDescr->byteOffset) + ((pSIDescr->bitOffset + bitLength + 7U) / 8U)) > structsNativeSizeInByte)
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex refers to data outside the native structure");
      }
      else
      {
        if (pSIDescr->bitOffset != 0U)
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex has invalid 'bitOffset'");

        // any bytes outside the native data structure referenced?
        if ((pSIDescr->byteOffset + ((static_cast<uint_fast32_t>(nativeBitLength) * pSIDescr->nElements) / 8U)) > structsNativeSizeInByte)
          throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Subindex refers to data outside the native structure");
      }

      // keep in mind, if any subindex is writeable
      if ((pSIDescr->attributes & attr_ACCESS_WR) != 0U)
        anyWriteable = true;

      // calculate size of whole object in CANopen encoded stream
      if (IsDataTypeBitBased(pSIDescr->type))
      {
        // bit based CANopen data
        streamSizeInBit += bitLength;
      }
      else
      {
        // byte based CANopen data (starts on byte boundary in data stream, padding may be required)

        uint8_t const requiredPadding = (8U - (streamSizeInBit % 8U)) % 8U;

        if (requiredPadding != 0U)
        {
          if (prevSIwasGap)
            throw std::invalid_argument("ObjectRECORD::ObjectRECORD: Gap subindex did not establish at least byte alignment");

          streamSizeInBit += requiredPadding;
        }

        streamSizeInBit += static_cast<uint_fast32_t>(bitLength) * pSIDescr->nElements;
      }

      prevSIwasGap = false;
    } // if (pSIDescr->type == DataType::null)... else...
  } // for (uint_fast8_t i = 0; i < SI0; i++)

  // check: A mutex must be specified if write access is possible
  if ((anyWriteable) && (pMutex == nullptr))
    throw std::invalid_argument("ObjectRECORD::ObjectRECORD: At least one subindex has write-permissions, but no mutex is specified.");
}

/**
 * \brief Updates the data represented by the object.
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
 * \param pNewData
 * New pointer to the data represented by the object.\n
 * nullptr is not allowed.\n
 * A pointer referencing to the currently configured memory is allowed if the RECORD's data shall not be updated.\n
 * The memory shall be provided and owned by the creator of the RECORD object.\n
 * The memory location must be valid during the life-time of the RECORD object or until a different memory location
 * is configured via @ref SetData().
 */
void ObjectRECORD::SetData(void* const pNewData)
{
  if (pMutex == nullptr)
    throw std::logic_error("ObjectRECORD::SetData: Operation requires that a mutex has been passed to the constructor");

  if (pNewData == nullptr)
    throw std::invalid_argument("ObjectRECORD::SetData: 'pNewData' is nullptr");

  gpcc::osal::MutexLocker mutexLocker(*pMutex);
  pStruct = pNewData;
}

// <-- base class Object

/// \copydoc Object::GetObjectCode
Object::ObjectCode ObjectRECORD::GetObjectCode(void) const noexcept
{
  return ObjectCode::Record;
}

/// \copydoc Object::GetObjectDataType
DataType ObjectRECORD::GetObjectDataType(void) const noexcept
{
  return DataType::domain;
}

/// \copydoc Object::GetObjectName
std::string ObjectRECORD::GetObjectName(void) const
{
  return name;
}

/// \copydoc Object::GetMaxNbOfSubindices
uint16_t ObjectRECORD::GetMaxNbOfSubindices(void) const noexcept
{
  return static_cast<uint16_t>(SI0) + 1U; // +1 for SI0
}

/// \copydoc Object::IsSubIndexEmpty
bool ObjectRECORD::IsSubIndexEmpty(uint8_t const subIdx) const
{
  if (subIdx == 0U)
    return false;

  if (subIdx > SI0)
    throw SubindexNotExistingError();

  return (pSIDescriptions[subIdx - 1U].nElements == 0U);
}

/// \copydoc Object::GetSubIdxDataType
DataType ObjectRECORD::GetSubIdxDataType(uint8_t const subIdx) const
{
  if (subIdx == 0U)
  {
    return DataType::unsigned8;
  }
  else
  {
    // subindex not existing?
    if (subIdx > SI0)
      throw SubindexNotExistingError();

    // subindex empty?
    if (pSIDescriptions[subIdx - 1U].nElements == 0U)
      throw SubindexNotExistingError();

    return MapAlternativeDataTypesToOriginalTypes(pSIDescriptions[subIdx - 1U].type);
  }
}

/// \copydoc Object::GetSubIdxAttributes
Object::attr_t ObjectRECORD::GetSubIdxAttributes(uint8_t const subIdx) const
{
  if (subIdx == 0U)
  {
    return attr_ACCESS_RD;
  }
  else
  {
    // subindex not existing?
    if (subIdx > SI0)
      throw SubindexNotExistingError();

    // subindex empty?
    if (pSIDescriptions[subIdx - 1U].nElements == 0U)
      throw SubindexNotExistingError();

    return pSIDescriptions[subIdx - 1U].attributes;
  }
}

/// \copydoc Object::GetSubIdxMaxSize
size_t ObjectRECORD::GetSubIdxMaxSize(uint8_t const subIdx) const
{
  if (subIdx == 0U)
  {
    return 8U;
  }
  else
  {
    // subindex not existing?
    if (subIdx > SI0)
      throw SubindexNotExistingError();

    SubIdxDescr const * const pSD = &pSIDescriptions[subIdx - 1U];

    // subindex empty?
    if (pSD->nElements == 0U)
      throw SubindexNotExistingError();

    return static_cast<uint_fast32_t>(DataTypeBitLengthTable[static_cast<int>(pSD->type)]) * pSD->nElements;
  }
}

/// \copydoc Object::GetSubIdxName
std::string ObjectRECORD::GetSubIdxName(uint8_t const subIdx) const
{
  if (subIdx == 0U)
  {
    return "Number of subindices";
  }
  else
  {
    // subindex not existing?
    if (subIdx > SI0)
      throw SubindexNotExistingError();

    // subindex empty?
    if (pSIDescriptions[subIdx - 1U].nElements == 0U)
      throw SubindexNotExistingError();

    return pSIDescriptions[subIdx - 1U].name;
  }
}

/// \copydoc Object::LockData
gpcc::osal::MutexLocker ObjectRECORD::LockData(void) const
{
  return gpcc::osal::MutexLocker(pMutex);
}

/// \copydoc Object::GetObjectStreamSize
size_t ObjectRECORD::GetObjectStreamSize(bool const SI016Bits) const noexcept
{
  if (SI016Bits)
    return streamSizeInBit + 16U;
  else
    return streamSizeInBit + 8U;
}

/// \copydoc Object::GetNbOfSubIndices
uint16_t ObjectRECORD::GetNbOfSubIndices(void) const noexcept
{
  return static_cast<uint16_t>(SI0) + 1U; // +1 for SI0
}

/// \copydoc Object::GetSubIdxActualSize
size_t ObjectRECORD::GetSubIdxActualSize(uint8_t const subIdx) const
{
  if (subIdx == 0U)
  {
    return 8U;
  }
  else
  {
    // subindex not existing?
    if (subIdx > SI0)
      throw SubindexNotExistingError();

    SubIdxDescr const * const pSD = &pSIDescriptions[subIdx - 1U];

    // subindex empty?
    if (pSD->nElements == 0U)
      throw SubindexNotExistingError();

    // data types with flexible-length require invocation of before-read-callback
    if ((pSD->type == DataType::visible_string) && (pNotifiable != nullptr))
    {
      auto const result = pNotifiable->OnBeforeRead(this, subIdx, false, true);
      if (result == SDOAbortCode::OutOfMemory)
        throw std::bad_alloc();
      else if (result != SDOAbortCode::OK)
        throw std::runtime_error("ObjectRECORD::GetSubIdxActualSize: Before-read-callback failed.");
    }

    uint8_t const * const pNativeData = static_cast<uint8_t const *>(pStruct) + pSD->byteOffset;
    return DetermineSizeOfCANopenEncodedData(pNativeData, pSD->type, pSD->nElements);
  }
}

/// \copydoc Object::Read
SDOAbortCode ObjectRECORD::Read(uint8_t const subIdx,
                                attr_t const permissions,
                                gpcc::Stream::IStreamWriter & isw) const
{
  // subindex not existing?
  if (subIdx > SI0)
    return SDOAbortCode::SubindexDoesNotExist;

  if (subIdx == 0U)
  {
    // (subindex 0 shall be read)

    // check permissions
    if ((permissions & attr_ACCESS_RD) == 0U)
      return SDOAbortCode::AttemptToReadWrOnlyObject;

    // invoke before-read-callback
    if (pNotifiable != nullptr)
    {
      auto const result = pNotifiable->OnBeforeRead(this, 0, false, false);
      if (result != SDOAbortCode::OK)
        return result;
    }

    // read SI0
    NativeDataToCANopenEncodedData(&SI0, DataType::unsigned8, 1U, false, isw);
  } // if (subIdx == 0)
  else
  {
    // (subindex 0 shall not be read)

    // get a pointer to the description of the subindex
    SubIdxDescr const * const pSIDescr = &pSIDescriptions[subIdx - 1U];

    // subindex empty?
    if (pSIDescr->nElements == 0U)
      return SDOAbortCode::SubindexDoesNotExist;

    // check permissions
    if ((pSIDescr->attributes & attr_ACCESS_RD & permissions) == 0U)
      return SDOAbortCode::AttemptToReadWrOnlyObject;

    // gap subindex?
    if (pSIDescr->type == DataType::null)
    {
      isw.FillBits(pSIDescr->nElements, false);
    }
    else
    {
      // invoke before-read-callback
      if (pNotifiable != nullptr)
      {
        auto const result = pNotifiable->OnBeforeRead(this, subIdx, false, false);
        if (result != SDOAbortCode::OK)
          return result;
      }

      if (IsNativeDataStuffed(pSIDescr->type))
      {
        // (data type is bit-based and native data is stuffed)

        uint8_t const bits = ReadBits(pStruct, pSIDescr);
        NativeDataToCANopenEncodedData(&bits, pSIDescr->type, 1U, false, isw);
      }
      else
      {
        // (data type may be bit-based, but native data is NOT stuffed)

        uint8_t const * const pNativeData = static_cast<uint8_t const *>(pStruct) + pSIDescr->byteOffset;
        NativeDataToCANopenEncodedData(pNativeData, pSIDescr->type, pSIDescr->nElements, false, isw);
      }
    }
  } // if (subIdx == 0)... else...

  return SDOAbortCode::OK;
}

/// \copydoc Object::Write
SDOAbortCode ObjectRECORD::Write(uint8_t const subIdx,
                                 attr_t const permissions,
                                 gpcc::Stream::IStreamReader & isr)
{
  // subindex not existing?
  if (subIdx > SI0)
    return SDOAbortCode::SubindexDoesNotExist;

  // SI0 is always read-only
  if (subIdx == 0U)
    return SDOAbortCode::AttemptToWriteRdOnlyObject;

  // get a pointer to the description of the subindex
  SubIdxDescr const * const pSIDescr = &pSIDescriptions[subIdx - 1U];

  // subindex empty?
  if (pSIDescr->nElements == 0U)
    return SDOAbortCode::SubindexDoesNotExist;

  // check permissions
  if ((pSIDescr->attributes & attr_ACCESS_WR & permissions) == 0U)
    return SDOAbortCode::AttemptToWriteRdOnlyObject;

  // gap subindex?
  if (pSIDescr->type == DataType::null)
  {
    // (gap subindex)

    try
    {
      isr.Skip(pSIDescr->nElements);
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
  }
  else
  {
    // ("normal" subindex)

    // determine number of bytes required to store the data that shall be written in native format
    // (cannot be zero, ensured by constructor)
    uint_fast32_t const nativeSizeInByte = (static_cast<uint_fast32_t>(NativeDataTypeBitLengthTable[static_cast<int>(pSIDescr->type)]) / 8U) * pSIDescr->nElements;

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
      CANopenEncodedDataToNativeData(isr, pSIDescr->type, pSIDescr->nElements, false, pTempMem);
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
    if (IsNativeDataStuffed(pSIDescr->type))
    {
      // (data type is bit-based and native data is stuffed)

      WriteBits(pStruct, pSIDescr, *pTempMem);
    }
    else
    {
      // (data type may be bit-based, but native data is NOT stuffed)

      uint8_t* const ptr = static_cast<uint8_t*>(pStruct) + pSIDescr->byteOffset;
      memcpy(ptr, pTempMem, nativeSizeInByte);
    }

    // invoke after-write-callback
    try
    {
      if (pNotifiable != nullptr)
        pNotifiable->OnAfterWrite(this, subIdx, false);
    }
    catch (std::exception const & e)
    {
      gpcc::osal::Panic("ObjectRECORD::Write: After-write-callback threw: ", e);
    }
    catch (...)
    {
      PANIC();
    }
  }

  return SDOAbortCode::OK;
}

/// \copydoc Object::CompleteRead
SDOAbortCode ObjectRECORD::CompleteRead(bool const inclSI0,
                                        bool const SI016Bits,
                                        attr_t const permissions,
                                        gpcc::Stream::IStreamWriter & isw) const
{
  // check permissions for SI0, if SI0 is included
  if ((inclSI0) && ((permissions & attr_ACCESS_RD) == 0U))
    return SDOAbortCode::AttemptToReadWrOnlyObject;

  // check permissions for all other SIs
  for (uint_fast8_t i = 0; i < SI0; i++)
  {
    // only check subindices which are not empty
    if (pSIDescriptions[i].nElements != 0U)
    {
      // subindex not pure write-only AND permissions not sufficient?
      if (((pSIDescriptions[i].attributes & attr_ACCESS_RD) != 0U) &&
          ((pSIDescriptions[i].attributes & attr_ACCESS_RD & permissions) == 0U))
      {
        return SDOAbortCode::AttemptToReadWrOnlyObject;
      }
    }
  }

  // invoke before-read-callback
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

  // read the other subindices
  for (uint_fast8_t i = 0; i < SI0; i++)
  {
    // get a pointer to the subindex description
    SubIdxDescr const * const pSIDescr = &pSIDescriptions[i];

    // skip empty subindices
    if (pSIDescr->nElements == 0U)
      continue;

    // is the subindex pure write-only or a gap?
    if (   ((pSIDescr->attributes & attr_ACCESS_RD) == 0U)
        || (pSIDescr->type == DataType::null))
    {
      // (data is pure write-only or the subindex is a gap; pure write-only subindices and gap subindices read as zero)

      // calculate number of bits
      // (cannot be zero, ensured by constructor)
      uint32_t const nBits = static_cast<uint32_t>(DataTypeBitLengthTable[static_cast<int>(pSIDescr->type)]) * pSIDescr->nElements;

      if (IsDataTypeBitBased(pSIDescr->type))
      {
        isw.FillBits(nBits, false);
      }
      else
      {
        uint32_t const nBytes = nBits / 8U;
        isw.FillBytes(nBytes, 0U);
      }
    }
    else
    {
      // (data is neither pure write-only, nor a gap)

      if (IsNativeDataStuffed(pSIDescr->type))
      {
        // (data type is bit-based and native data is stuffed)

        uint8_t const bits = ReadBits(pStruct, pSIDescr);
        NativeDataToCANopenEncodedData(&bits, pSIDescr->type, 1U, true, isw);
      }
      else
      {
        // (data type may be bit-based, but native data is NOT stuffed)

        uint8_t const * const ptr = static_cast<uint8_t const *>(pStruct) + pSIDescr->byteOffset;
        NativeDataToCANopenEncodedData(ptr, pSIDescr->type, pSIDescr->nElements, true, isw);
      }
    }
  } // for (uint_fast8_t i = 0; i < SI0; i++)

  return SDOAbortCode::OK;
}

/// \copydoc Object::CompleteWrite
SDOAbortCode ObjectRECORD::CompleteWrite(bool const inclSI0,
                                         bool const SI016Bits,
                                         attr_t const permissions,
                                         gpcc::Stream::IStreamReader & isr,
                                         gpcc::Stream::IStreamReader::RemainingNbOfBits const ernob)
{
  // permission for SI0 must not be checked, because SI0 is always pure read-only in this RECORD object implementation

  // check permissions of the other subindices and check if any SI (except gap SIs) is pure read-only
  bool anySubIdxPureRO = false;
  for (uint_fast8_t i = 0; i < SI0; i++)
  {
    // only check subindices which are not empty
    if (pSIDescriptions[i].nElements != 0U)
    {
      // pure read-only?
      if ((pSIDescriptions[i].attributes & attr_ACCESS_WR) == 0U)
      {
        // pure read-only -> access rights are don't care

        // anySubIdxPureRO: only recognize the SI if it is neither a gap nor an empty subindex
        if (pSIDescriptions[i].type != DataType::null)
          anySubIdxPureRO = true;
      }
      else
      {
        if ((pSIDescriptions[i].attributes & attr_ACCESS_WR & permissions) == 0U)
          return SDOAbortCode::AttemptToWriteRdOnlyObject;
      }
    }
  }

  // prepare pointer to temporary storage for preview data, which will be allocated later
  uint8_t* pTempMem = nullptr;
  ON_SCOPE_EXIT(releaseTempMem) { delete [] pTempMem; };

  try
  {
    // Read and discard potential SI0 data from 'isr'.
    // However, the provided data must match the current value of SI0, because SI0 is always pure read-only
    // in this RECORD object implementation.
    if (inclSI0)
    {
      uint16_t const providedSI0Value = SI016Bits ? isr.Read_uint16() : isr.Read_uint8();

      if (providedSI0Value != SI0)
        return SDOAbortCode::UnsupportedAccessToObject;
    }

    // allocate temporary storage and fill it with zeros
    pTempMem = new uint8_t[structsNativeSizeInByte];
    memset(pTempMem, 0, structsNativeSizeInByte);

    // read data into pTempStorage
    for (uint_fast8_t i = 0; i < SI0; i++)
    {
      // get a pointer to the subindex description
      SubIdxDescr const * const pSIDescr = &pSIDescriptions[i];

      // skip empty subindices
      if (pSIDescr->nElements == 0U)
        continue;

      // gap subindex?
      if (pSIDescr->type == DataType::null)
      {
        // Skip bits in isr. Bits in pTempMem are zero.
        isr.Skip(pSIDescr->nElements);
      }
      // pure RO subindex?
      else if ((pSIDescr->attributes & attr_ACCESS_WR) == 0U)
      {
        // calculate number of bits
        // (cannot be zero, ensured by constructor)
        uint32_t const nBits = static_cast<uint32_t>(DataTypeBitLengthTable[static_cast<int>(pSIDescr->type)]) * pSIDescr->nElements;

        if (IsDataTypeBitBased(pSIDescr->type))
        {
          isr.Skip(nBits);
        }
        else
        {
          // Skip bits in isr. We use Read_uint8() to achieve byte-alignment and then we use Skip().
          uint32_t const nBytes = (nBits / 8U) - 1U;
          (void)isr.Read_uint8();
          isr.Skip(nBytes * 8U);
        }
      }
      // else its a "normal" WR or WO subindex...
      else
      {
        if (IsNativeDataStuffed(pSIDescr->type))
        {
          uint8_t bits;
          CANopenEncodedDataToNativeData(isr, pSIDescr->type, 1U, true, &bits);
          WriteBits(pTempMem, pSIDescr, bits);
        }
        else
        {
          uint8_t* const pDest = pTempMem + pSIDescr->byteOffset;
          CANopenEncodedDataToNativeData(isr, pSIDescr->type, pSIDescr->nElements, true, pDest);
        }
      }
    }

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
    auto const result = pNotifiable->OnBeforeWrite(this, inclSI0 ? 0U : 1U, true, inclSI0 ? SI0 : 0U, pTempMem);
    if (result != SDOAbortCode::OK)
      return result;
  }

  // write the data
  if (!anySubIdxPureRO)
  {
    // (all non-gap-subindices are RW or WO, so we can use a simple memcpy)
    memcpy(pStruct, pTempMem, structsNativeSizeInByte);
  }
  else
  {
    // (at least one subindex (which is neither a gap nor an empty SI) is pure RO,
    // so we have to write the subindices one by one)

    for (uint_fast8_t i = 0; i < SI0; i++)
    {
      // get a pointer to the subindex description
      SubIdxDescr const * const pSIDescr = &pSIDescriptions[i];

      // skip empty subindices, gap subindices, and pure RO subindices
      if (   (pSIDescr->type == DataType::null)
          || ((pSIDescr->attributes & attr_ACCESS_WR) == 0U))
      {
        continue;
      }

      if (IsNativeDataStuffed(pSIDescr->type))
      {
        // (data type is bit-based and native data is stuffed)

        WriteBits(pStruct, pSIDescr, ReadBits(pTempMem, pSIDescr));
      }
      else
      {
        // (data type may be bit-based, but native data is NOT stuffed)

        void* const pDest = static_cast<uint8_t*>(pStruct) + pSIDescr->byteOffset;
        void const * const pSrc = static_cast<uint8_t const *>(pTempMem) + pSIDescr->byteOffset;
        uint_fast32_t const n = NativeDataTypeBitLengthTable[static_cast<int>(pSIDescr->type)] / 8U;

        if (pSIDescr->nElements == 1U)
        {
          switch (n)
          {
            case 1U:
              *static_cast<uint8_t*>(pDest) = *static_cast<uint8_t const *>(pSrc);
              break;

            case 2U:
              *static_cast<uint16_t*>(pDest) = *static_cast<uint16_t const *>(pSrc);
              break;

            case 4U:
              *static_cast<uint32_t*>(pDest) = *static_cast<uint32_t const *>(pSrc);
              break;

            case 8U:
              *static_cast<uint64_t*>(pDest) = *static_cast<uint64_t const *>(pSrc);
              break;

            default:
              memcpy(pDest, pSrc, n);
          }
        }
        else
        {
          memcpy(pDest, pSrc, n * pSIDescr->nElements);
        }
      }
    } // for (uint_fast8_t i = 0; i < SI0; i++)
  } // if (anySubIdxPureRO)

  // invoke after-write-callback
  try
  {
    if (pNotifiable != nullptr)
      pNotifiable->OnAfterWrite(this, inclSI0 ? 0U : 1U, true);
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("ObjectRECORD::CompleteWrite: After-write-callback threw: ", e);
  }
  catch (...)
  {
    PANIC();
  }

  return SDOAbortCode::OK;
}
// --> base class Object

/**
 * \brief Writes bit-based data to the native (stuffed) data represented by a subindex.
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
 * \param pDestStruct
 * Pointer to the structure containing the native data referenced by `pSubIdxDescr`.
 *
 * \param pSubIdxDescr
 * Pointer to the description of the subindex.\n
 * The native data represented by the subindex must be bit-based and stuffed.
 *
 * \param newBits
 * Data to be written. The bits must be aligned to the LSB.\n
 * Potential unused upper bits are don't care and will be ignored.
 */
void ObjectRECORD::WriteBits(void* const pDestStruct, SubIdxDescr const * const pSubIdxDescr, uint8_t const newBits)
{
  // get a pointer to the byte containing at least the first bit that shall be written
  uint8_t* const pNativeData = static_cast<uint8_t*>(pDestStruct) + pSubIdxDescr->byteOffset;

  // get number of bits to be written
  uint_fast8_t const nBits = DataTypeBitLengthTable[static_cast<int>(pSubIdxDescr->type)];

  if ((nBits < 1U) || (nBits > 8U))
    throw std::logic_error("ObjectRECORD::WriteBits: Incompatible data type");

  // create a mask for the bits being written. The mask is aligned to the LSB.
  uint_fast16_t const mask = (1U << nBits) - 1U;

  // read current data
  uint_fast16_t currData = pNativeData[0];
  if ((pSubIdxDescr->bitOffset + nBits) > 8U)
    currData |= static_cast<uint_fast16_t>(pNativeData[1]) << 8U;

  // write newBits into currData
  currData &= ~(mask << pSubIdxDescr->bitOffset);
  currData |= static_cast<uint_fast16_t>(newBits & mask) << pSubIdxDescr->bitOffset;

  // write data back
  pNativeData[0] = static_cast<uint8_t>(currData);
  if ((pSubIdxDescr->bitOffset + nBits) > 8U)
    pNativeData[1] = static_cast<uint8_t>(currData >> 8U);
}

/**
 * \brief Reads bit-based data from the native (stuffed) data represented by a subindex.
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
 * \param pSrcStruct
 * Pointer to the structure containing the native data referenced by `pSubIdxDescr`.
 *
 * \param pSubIdxDescr
 * Pointer to the description of the subindex.\n
 * The native data represented by the subindex must be bit-based and stuffed.
 *
 * \return
 * Data that has been read. The bits are aligned to the LSB. Potential unused upper bits are undefined.
 */
uint8_t ObjectRECORD::ReadBits(void const * const pSrcStruct, SubIdxDescr const * const pSubIdxDescr) const
{
  // get a pointer to the byte containing at least the first bit that shall be read
  uint8_t const * const pNativeData = static_cast<uint8_t const *>(pSrcStruct) + pSubIdxDescr->byteOffset;

  // get number of bits to be read
  uint_fast8_t const nBits = DataTypeBitLengthTable[static_cast<int>(pSubIdxDescr->type)];

  if ((nBits < 1U) || (nBits > 8U))
    throw std::logic_error("ObjectRECORD::ReadBits: Incompatible data type");

  // load bits
  uint_fast16_t bits = pNativeData[0];
  if ((pSubIdxDescr->bitOffset + nBits) > 8U)
    bits |= static_cast<uint_fast16_t>(pNativeData[1]) << 8U;

  // align to LSB
  bits >>= pSubIdxDescr->bitOffset;

  return static_cast<uint8_t>(bits);
}

} // namespace cood
} // namespace gpcc
