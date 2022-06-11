/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018, 2020-2022 Daniel Jerolm

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

#include "ObjectVAR.hpp"
#include "IObjectNotifiable.hpp"
#include "exceptions.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/Stream/StreamErrors.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
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
 * \param _type
 * CANopen data type of the data represented by the object.\n
 * Since the constructed object is a VARIABLE object, the object will have the same type as the data
 * represented by it.\n
 * The documentation of the enumeration [DataType](@ref gpcc::cood::DataType) contains a list of native types associated
 * with each CANopen data type.
 *
 * \param _nElements
 * Number of elements of `_type` the data represented by the object is comprised of.\n
 * For most data types this is one.\n
 * For types [DataType::visible_string](@ref gpcc::cood::DataType::visible_string),
 * [DataType::octet_string](@ref gpcc::cood::DataType::octet_string), and
 * [DataType::unicode_string](@ref gpcc::cood::DataType::unicode_string) this may be any number equal to or larger
 * than one.
 *
 * \param _attributes
 * Attributes for the one and only subindex 0.\n
 * At least one read- or write-permission must be specified.
 *
 * \param _pData
 * Pointer to the native data represented by the object. `nullptr` is not allowed.\n
 * The type of the referenced native data must match the CANopen data type specified by parameter `type`
 * and the number of data elements must match parameter `_nElements`.\n
 * The documentation of the enumeration [DataType](@ref gpcc::cood::DataType) contains a list of native types associated
 * with each CANopen data type.\n
 * The memory location must be valid during the life-time of the VARIABLE object or until a different
 * memory location is configured via @ref SetData().
 *
 * \param _pMutex
 * Pointer to a mutex protecting access to the native data referenced by `_pData`.\n
 * The mutex is optional. If the object is read-only __and__ if the application does not modify the
 * data referenced by `_pData`, then a mutex is not required and this parameter may be `nullptr`.\n
 * __BUT__ if the object is writeable, or if the application may modify the data referenced by `_pData`,
 * then a mutex must be specified.\n
 * \n
 * The application must obey the following rules when accessing the native data referenced by `_pData`:
 * - If the object is READ-ONLY, then the application must lock the mutex only if it wants to
 *   modify the data referenced by `_pData`. The application does not need to lock the mutex for reading
 *   the data referenced by `_pData` in this case.
 * - If the object is READ-WRITE, then the application must lock the mutex ALWAYS when it wants to
 *   read or write the data referenced by `_pData`.
 *
 * \param _pNotifiable
 * Pointer to a [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) interface that shall be used to deliver
 * callbacks to the owner of the object.\n
 * nullptr is allowed.
 */
ObjectVAR::ObjectVAR(std::string            const & _name,
                     DataType               const   _type,
                     uint16_t               const   _nElements,
                     attr_t                 const   _attributes,
                     void*                  const   _pData,
                     gpcc::osal::Mutex *    const   _pMutex,
                     IObjectNotifiable *    const   _pNotifiable)
: Object()
, name(_name)
, type(_type)
, nElements(_nElements)
, attributes(_attributes)
, pData(_pData)
, pMutex(_pMutex)
, pNotifiable(_pNotifiable)
{
  // data type supported?
  if (   (DataTypeBitLengthTable[static_cast<int>(type)] == 0U)
      || (NativeDataTypeBitLengthTable[static_cast<int>(type)] == 0U))
  {
    throw DataTypeNotSupportedError(type);
  }

  // check nElements
  if (   (type == DataType::visible_string)
      || (type == DataType::octet_string)
      || (type == DataType::unicode_string))
  {
    if (nElements == 0U)
      throw std::invalid_argument("ObjectVAR::ObjectVAR: '_nElements' is zero");
  }
  else
  {
    if (nElements != 1U)
      throw std::invalid_argument("ObjectVAR::ObjectVAR: '_nElements' must be one for given '_type'");
  }

  // at least one read or write permission specified?
  if ((attributes & attr_ACCESS_RW) == 0U)
    throw std::invalid_argument("ObjectVAR::ObjectVAR: No read- or write-permissions set in '_attributes'");

  // check: a mutex must be specified if write access is possible
  if (((attributes & attr_ACCESS_WR) != 0U) && (pMutex == nullptr))
    throw std::logic_error("ObjectVAR::ObjectVAR: Object with write-permission requires a mutex");

  if (pData == nullptr)
    throw std::invalid_argument("ObjectVAR::ObjectVAR: _pData is nullptr");
}

/**
 * \brief Updates the pointer to the data referenced by the object.
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
 * Pointer to the (new) memory location containing the data that shall be represented by the VARIABLE object.\n
 * nullptr is not allowed.\n
 * A pointer referencing to the currently configured memory is allowed if the objects's data shall not
 * be updated.\n
 * The type of the referenced native data must match the CANopen data type specified by parameter `type`
 * that has been passed to the constructor and the number of data elements must match parameter `_nElements`
 * that has been passed to the constructor.\n
 * The documentation of the enumeration @ref DataType contains a list of native types associated with each
 * CANopen data type.\n
 * The memory location must be valid during the life-time of the VARIABLE object or until a different
 * memory location is configured via @ref SetData().
 */
void ObjectVAR::SetData(void* const pNewData)
{
  if (pMutex == nullptr)
    throw std::logic_error("ObjectVAR::SetData: Operation requires that a mutex has been specified during object creation");

  if (pNewData == nullptr)
    throw std::invalid_argument("ObjectVAR::SetData: pNewData is nullptr");

  gpcc::osal::MutexLocker mutexLocker(*pMutex);
  pData = pNewData;
}

// <-- base class Object

/// \copydoc Object::GetObjectCode
Object::ObjectCode ObjectVAR::GetObjectCode(void) const noexcept
{
  return ObjectCode::Variable;
}

/// \copydoc Object::GetObjectDataType
DataType ObjectVAR::GetObjectDataType(void) const noexcept
{
  return MapAlternativeDataTypesToOriginalTypes(type);
}

/// \copydoc Object::GetObjectName
std::string ObjectVAR::GetObjectName(void) const
{
  return name;
}

/// \copydoc Object::GetMaxNbOfSubindices
uint16_t ObjectVAR::GetMaxNbOfSubindices(void) const noexcept
{
  return 1U;
}

/// \copydoc Object::IsSubIndexEmpty
bool ObjectVAR::IsSubIndexEmpty(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw SubindexNotExistingError();

  return false;
}

/// \copydoc Object::GetSubIdxDataType
DataType ObjectVAR::GetSubIdxDataType(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw SubindexNotExistingError();

  return MapAlternativeDataTypesToOriginalTypes(type);
}

/// \copydoc Object::GetSubIdxAttributes
Object::attr_t ObjectVAR::GetSubIdxAttributes(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw SubindexNotExistingError();

  return attributes;
}

/// \copydoc Object::GetSubIdxMaxSize
size_t ObjectVAR::GetSubIdxMaxSize(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw SubindexNotExistingError();

  return static_cast<uint_fast32_t>(DataTypeBitLengthTable[static_cast<int>(type)]) * nElements;
}

/// \copydoc Object::GetSubIdxName
std::string ObjectVAR::GetSubIdxName(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw SubindexNotExistingError();

  return name;
}

/// \copydoc Object::LockData
gpcc::osal::MutexLocker ObjectVAR::LockData(void) const
{
  return gpcc::osal::MutexLocker(pMutex);
}

/// \copydoc Object::GetObjectStreamSize
size_t ObjectVAR::GetObjectStreamSize(bool const SI016Bits) const noexcept
{
  (void)SI016Bits;
  return static_cast<uint_fast32_t>(DataTypeBitLengthTable[static_cast<int>(type)]) * nElements;
}

/// \copydoc Object::GetNbOfSubIndices
uint16_t ObjectVAR::GetNbOfSubIndices(void) const noexcept
{
  return 1U;
}

/// \copydoc Object::GetSubIdxActualSize
size_t ObjectVAR::GetSubIdxActualSize(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw SubindexNotExistingError();

  // data types with flexible-length require invocation of before-read-callback
  if ((type == DataType::visible_string) && (pNotifiable != nullptr))
  {
    auto const result = pNotifiable->OnBeforeRead(this, 0, false, true);
    if (result == SDOAbortCode::OutOfMemory)
      throw std::bad_alloc();
    else if (result != SDOAbortCode::OK)
      throw std::runtime_error("ObjectVAR::GetSubIdxActualSize: Before-read-callback failed.");
  }

  return DetermineSizeOfCANopenEncodedData(pData, type, nElements);
}

/// \copydoc Object::Read
SDOAbortCode ObjectVAR::Read(uint8_t const subIdx,
                             attr_t const permissions,
                             gpcc::Stream::IStreamWriter & isw) const
{
  if (subIdx != 0U)
    return SDOAbortCode::SubindexDoesNotExist;

  if ((permissions & attr_ACCESS_RD & attributes) == 0U)
    return SDOAbortCode::AttemptToReadWrOnlyObject;

  if (pNotifiable != nullptr)
  {
    auto const result = pNotifiable->OnBeforeRead(this, 0, false, false);
    if (result != SDOAbortCode::OK)
      return result;
  }

  NativeDataToCANopenEncodedData(pData, type, nElements, false, isw);

  return SDOAbortCode::OK;
}

/// \copydoc Object::Write
SDOAbortCode ObjectVAR::Write(uint8_t const subIdx,
                              attr_t const permissions,
                              gpcc::Stream::IStreamReader & isr)
{
  if (subIdx != 0U)
    return SDOAbortCode::SubindexDoesNotExist;

  if ((permissions & attr_ACCESS_WR & attributes) == 0U)
    return SDOAbortCode::AttemptToWriteRdOnlyObject;

  // determine number of bytes required to store the data that shall be written in native format
  // (cannot be zero, ensured by constructor)
  uint_fast32_t const nBytesNative = static_cast<uint_fast32_t>(NativeDataTypeBitLengthTable[static_cast<int>(type)] / 8U) * nElements;

  // allocate some temporary storage (pTempMem) either on the stack or on the heap
  uint64_t localMem;
  uint8_t* pTempMem;
  if (nBytesNative <= sizeof(localMem))
    pTempMem = reinterpret_cast<uint8_t*>(&localMem);
  else
    pTempMem = new uint8_t[nBytesNative];

  ON_SCOPE_EXIT(releaseTempMem)
  {
    if (pTempMem != reinterpret_cast<uint8_t*>(&localMem))
      delete [] pTempMem;
  };

  // read data into pTempMem for preview
  try
  {
    CANopenEncodedDataToNativeData(isr, type, nElements, false, pTempMem);
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

  // invoke before-write-callback (preview)
  if (pNotifiable != nullptr)
  {
    auto const result = pNotifiable->OnBeforeWrite(this, 0, false, 0, pTempMem);
    if (result != SDOAbortCode::OK)
      return result;
  }

  // finally write to the object's data
  switch (nBytesNative)
  {
    case 1U:
      *static_cast<uint8_t*>(pData) = *pTempMem;
      break;

    case 2U:
      *static_cast<uint16_t*>(pData) = *reinterpret_cast<uint16_t*>(pTempMem);
      break;

    case 4U:
      *static_cast<uint32_t*>(pData) = *reinterpret_cast<uint32_t*>(pTempMem);
      break;

    case 8U:
      *static_cast<uint64_t*>(pData) = localMem;
      break;

    default:
      memcpy(pData, pTempMem, nBytesNative);
  }

  try
  {
    if (pNotifiable != nullptr)
      pNotifiable->OnAfterWrite(this, 0, false);
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("ObjectVAR::Write: After-write-callback threw: ", e);
  }
  catch (...)
  {
    PANIC();
  }

  return SDOAbortCode::OK;
}

/// \copydoc Object::CompleteRead
SDOAbortCode ObjectVAR::CompleteRead(bool const inclSI0,
                                     bool const SI016Bits,
                                     attr_t const permissions,
                                     gpcc::Stream::IStreamWriter & isw) const
{
  (void)inclSI0;
  (void)SI016Bits;
  (void)permissions;
  (void)isw;

  return SDOAbortCode::UnsupportedAccessToObject;
}

/// \copydoc Object::CompleteWrite
SDOAbortCode ObjectVAR::CompleteWrite(bool const inclSI0,
                                      bool const SI016Bits,
                                      attr_t const permissions,
                                      gpcc::Stream::IStreamReader & isr,
                                      gpcc::Stream::IStreamReader::RemainingNbOfBits const ernob)
{
  (void)inclSI0;
  (void)SI016Bits;
  (void)permissions;
  (void)isr;
  (void)ernob;

  return SDOAbortCode::UnsupportedAccessToObject;
}
// --> base class Object

} // namespace cood
} // namespace gpcc
