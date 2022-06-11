/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018, 2020, 2022 Daniel Jerolm

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

#include "Object.hpp"
#include "exceptions.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include <cstring>

namespace gpcc {
namespace cood {

#ifndef __DOXYGEN__
size_t const Object::largestObjectCodeNameLength;

Object::attr_t const Object::attr_ACCESS_RD_PREOP;
Object::attr_t const Object::attr_ACCESS_RD_SAFEOP;
Object::attr_t const Object::attr_ACCESS_RD_OP;
Object::attr_t const Object::attr_ACCESS_WR_PREOP;
Object::attr_t const Object::attr_ACCESS_WR_SAFEOP;
Object::attr_t const Object::attr_ACCESS_WR_OP;
Object::attr_t const Object::attr_RXMAP;
Object::attr_t const Object::attr_TXMAP;
Object::attr_t const Object::attr_BACKUP;
Object::attr_t const Object::attr_SETTINGS;
Object::attr_t const Object::attr_ACCESS_RDCONST;
Object::attr_t const Object::attr_ACCESS_RD;
Object::attr_t const Object::attr_ACCESS_WR;
Object::attr_t const Object::attr_ACCESS_RW;
#endif

/**
 * \brief Destructor.
 *
 * \pre   The object must not be registered at an object dictionary.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Object::~Object(void)
{
  if (pOD != nullptr)
    gpcc::osal::Panic("Object::~Object: Still registered at object dictionary?");
}

/**
 * \brief Retrieves a string containing the name of an @ref ObjectCode enum value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param objCode
 * Enum value.
 *
 * \return
 * Null-terminated c-string located in code memory.\n
 * The string contains the name of the enum value `objCode`.
 */
char const * Object::ObjectCodeToString(ObjectCode const objCode)
{
  switch (objCode)
  {
    case ObjectCode::Null:      return "NULL";
    case ObjectCode::Domain:    return "DOMAIN";
    case ObjectCode::DefType:   return "DEFTYPE";
    case ObjectCode::DefStruct: return "DEFSTRUCT";
    case ObjectCode::Variable:  return "VAR";
    case ObjectCode::Array:     return "ARRAY";
    case ObjectCode::Record:    return "RECORD";
  }

  throw std::invalid_argument("Object::ObjectCodeToString: Unknown/invalid enum value");
}

/**
 * \brief Converts a value from the @ref ObjectCode enumeration into an uint8_t.
 *
 * @ref ToObjectCode(uint8_t const value) is the counterpart of this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param objCode
 * Value that shall be converted into an uint8_t.
 *
 * \return
 * Enum value `objCode` expressed as uint8_t.
 */
uint8_t Object::ToUint8(ObjectCode const objCode) noexcept
{
  return static_cast<uint8_t>(objCode);
}

/**
 * \brief Safely converts a uint8_t into a value from the @ref ObjectCode enumeration.
 *
 * This is the counterpart of @ref ToUint8(ObjectCode const objCode).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * Value that shall be converted into a value from the @ref ObjectCode enumeration.\n
 * This function is well aware of invalid values.
 *
 * \return
 * Value from the @ref ObjectCode enumeration that corresponds to parameter `value`.
 */
Object::ObjectCode Object::ToObjectCode(uint8_t const value)
{
  switch (value)
  {
    case static_cast<uint8_t>(ObjectCode::Null):      return ObjectCode::Null;
    case static_cast<uint8_t>(ObjectCode::Domain):    return ObjectCode::Domain;
    case static_cast<uint8_t>(ObjectCode::DefType):   return ObjectCode::DefType;
    case static_cast<uint8_t>(ObjectCode::DefStruct): return ObjectCode::DefStruct;
    case static_cast<uint8_t>(ObjectCode::Variable):  return ObjectCode::Variable;
    case static_cast<uint8_t>(ObjectCode::Array):     return ObjectCode::Array;
    case static_cast<uint8_t>(ObjectCode::Record):    return ObjectCode::Record;
  }

  throw std::invalid_argument("Object::ToObjectCode: 'value' invalid");
}

/**
 * \brief Creates an std::string containing a human-readable representation of an subindex attribute value.
 *
 * There are two styles available: CANopen style and EtherCAT style.
 *
 * __Example for EtherCAT style:__\n
 * R--W--,RxM,TxM,B,S\n
 * \n
 * Meaning of the different characters (comma characters are used to separate fields comprised of more than
 * one character):\n
 * R/-  = Read permission in PREOP yes/no\n
 * R/-  = Read permission in SAFEOP yes/no\n
 * R/-  = Read permission in OP yes/no\n
 * W/-  = Write permission in PREOP yes/no\n
 * W/-  = Write permission in SAFEOP yes/no\n
 * W/-  = Write permission in OP yes/no\n
 * RxM/ = RxMap possible yes/no\n
 * TxM/ = TxMap possible yes/no\n
 * B/   = Backup-tag set yes/no\n
 * S/   = Settings-tag set yes/no\n
 * \n
 * __Example for CANopen style:__\n
 * rw\n
 * wo\n
 * ro\n
 * const
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
 * No cancellation point included.
 *
 * - - -
 *
 * \param attributes
 * Attributes value.
 *
 * \param etherCATStyle
 * Controls the output style:\n
 * true = EtherCAT\n
 * false = CANopen
 *
 * \return
 * The generated string.
 */
std::string Object::AttributeToString(attr_t const attributes, bool const etherCATStyle)
{
  std::string s;
  s.reserve(16);

  if (etherCATStyle)
  {
    if ((attributes & attr_ACCESS_RD_PREOP) != 0U)
      s += "R";
    else
      s += "-";
    if ((attributes & attr_ACCESS_RD_SAFEOP) != 0U)
      s += "R";
    else
      s += "-";
    if ((attributes & attr_ACCESS_RD_OP) != 0U)
      s += "R";
    else
      s += "-";
    if ((attributes & attr_ACCESS_WR_PREOP) != 0U)
      s += "W";
    else
      s += "-";
    if ((attributes & attr_ACCESS_WR_SAFEOP) !=0U)
      s += "W";
    else
      s += "-";
    if ((attributes & attr_ACCESS_WR_OP) != 0U)
      s += "W";
    else
      s += "-";

    if ((attributes & attr_RXMAP) != 0U)
      s += ",RxM";
    if ((attributes & attr_TXMAP) != 0U)
      s += ",TxM";
    if ((attributes & attr_BACKUP) != 0U)
      s += ",B";
    if ((attributes & attr_SETTINGS) != 0U)
      s += ",S";
  }
  else
  {
    if ((attributes & attr_ACCESS_RD) == attr_ACCESS_RDCONST)
      s += "const";
    else if ((attributes & (attr_ACCESS_RD | attr_ACCESS_WR)) == (attr_ACCESS_RD | attr_ACCESS_WR))
      s += "rw";
    else if ((attributes & attr_ACCESS_RD) != 0U)
      s += "ro";
    else if ((attributes & attr_ACCESS_WR) != 0U)
      s += "wo";
  }

  return s;
}

/**
 * \brief Retrieves the index of the object.
 *
 * \pre   The object must be contained in an object dictionary.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Index of the object.
 */
uint16_t Object::GetIndex(void) const
{
  // Q: Why is this method classified thread-safe, though there is no mutex protecting access to "index"?
  // A: The value is set upon registration at an object dictionary. During registration,
  //    usage of std::unique_ptr ensures that there is only one owner and ownership moves
  //    to the object dictionary. Registration and object access is properly locked against each other
  //    by class ObjectDictionary.

  if (pOD == nullptr)
    throw std::logic_error("Object::GetIndex: The object is not registered at an object dictionary");

  return index;
}

size_t Object::GetAppSpecificMetaDataSize(uint8_t const subIdx) const
{
  (void)subIdx;
  return 0U;
}

std::vector<uint8_t> Object::GetAppSpecificMetaData(uint8_t const subIdx) const
{
  (void)subIdx;
  throw std::logic_error("Object::GetAppSpecificMetaData: Object has no application-specific meta data.");
}


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
 */
Object::Object(void) noexcept
: pOD(nullptr)
, index(0U)
{
}

/**
 * \brief Examines the native data of a subindex and determines the number of bits that would be written by
 *        [Object::NativeDataToCANopenEncodedData()](@ref gpcc::cood::Object::NativeDataToCANopenEncodedData).
 *
 * This method can be used to preview the size of the data that will be written by
 * [Object::NativeDataToCANopenEncodedData()](@ref gpcc::cood::Object::NativeDataToCANopenEncodedData).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws DataTypeNotSupportedError   CANopen data type is not supported ([details](@ref DataTypeNotSupportedError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pNativeData
 * Pointer to the native data. nullptr is not allowed.
 *
 * \param type
 * CANopen data type of the data.
 *
 * \param nDataElements
 * Number of data elements. Zero is not allowed.
 *
 * \return
 * Number of bits that would be written by
 * [Object::NativeDataToCANopenEncodedData()](@ref gpcc::cood::Object::NativeDataToCANopenEncodedData).
 */
size_t Object::DetermineSizeOfCANopenEncodedData(void const * const pNativeData, DataType const type, uint16_t const nDataElements)
{
  if ((pNativeData == nullptr) || (nDataElements == 0U))
    throw std::invalid_argument("Object::DetermineSizeOfCANopenEncodedData: Invalid arguments");

  if (type == DataType::visible_string)
  {
    size_t len = strnlen(static_cast<char const *>(pNativeData), nDataElements);

    // Object::NativeDataToCANopenEncodedData() will add a NUL character if the whole space is not occupied
    if (len < nDataElements)
      len++;

    return len * 8U;
  }
  else
  {
    uint_fast32_t const bitLen = DataTypeBitLengthTable[static_cast<int>(type)];
    if (bitLen == 0U)
      throw DataTypeNotSupportedError(type);

    return bitLen * nDataElements;
  }
}

/**
 * \brief Converts native data into CANopen format and writes it into an [IStreamWriter](@ref gpcc::Stream::IStreamWriter).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - incomplete/undefined data may have been written to `out`.
 *
 * \throws DataTypeNotSupportedError   CANopen data type is not supported ([details](@ref DataTypeNotSupportedError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - incomplete/undefined data may have been written to `out`.
 *
 * - - -
 *
 * \param pNativeData
 * Pointer to the native data that shall be converted into CANopen format and written into the
 * [IStreamWriter](@ref gpcc::Stream::IStreamWriter).\n
 * \n
 * Depending on parameter 'type', `pNativeData` will be casted to a pointer to a specific C/C++ data type
 * interpreted in the machine's native format. The documentation of the @ref DataType enumeration contains a list
 * of native C/C++ types associated with each CANopen data type.\n
 * \n
 * The size of the native data type may be larger than the size of the CANopen data type. If the memory of the
 * native data type contains some "spare" bits/bytes, then these bits/bytes will be don't care for the write
 * operation to @p out.
 *
 * \param type
 * CANopen type of the data.
 *
 * \param nDataElements
 * Number of data elements. Zero is not allowed.\n
 * If this is larger than one, then the native data will be treated as an array of native data elements.\n
 * Note:\n
 * - Depending on the data type, native data may be stuffed (-> @ref IsNativeDataStuffed()).
 * - Some data types offer flexible length (e.g. VISIBLE_STRING).\n
 *   During _single subindex access_, the number of bits/bytes written to @p out may be less than this parameter might
 *   suggest. [Object::DetermineSizeOfCANopenEncodedData()](@ref gpcc::cood::Object::DetermineSizeOfCANopenEncodedData)
 *   can be used to preview the size of the data that will be written to @p out.\n
 *   During a _complete access_, the number of bits/bytes written to @p out will match this parameter multiplied with
 *   the size of the data type.
 *
 * \param completeAccess
 * Indicates if this is invoked during a complete access.\n
 * true = complete access (all data elements will be written to @p out)\n
 * false = single subindex access (for data types with flexible length, less than @p nDataElements elements of data
 *         may be written to @p out)
 *
 * \param out
 * The native data will be converted into CANopen format and written into the referenced
 * [IStreamWriter](@ref gpcc::Stream::IStreamWriter).\n
 * [Object::DetermineSizeOfCANopenEncodedData()](@ref gpcc::cood::Object::DetermineSizeOfCANopenEncodedData)
 * can be used to preview the size of the data that will be written.
 */
void Object::NativeDataToCANopenEncodedData(void const * const pNativeData,
                                            DataType const type,
                                            uint16_t const nDataElements,
                                            bool const completeAccess,
                                            gpcc::Stream::IStreamWriter & out)
{
  if (pNativeData == nullptr)
    throw std::invalid_argument("Object::NativeDataToCANopenEncodedData: pNativeData is nullptr");

  if (nDataElements == 0U)
    throw std::invalid_argument("Object::NativeDataToCANopenEncodedData: nDataElements is zero");

  switch (type)
  {
    case DataType::boolean:
    {
      out.Write_bool(static_cast<bool const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer8:
    {
      out.Write_int8(static_cast<int8_t const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer16:
    {
      out.Write_int16(static_cast<int16_t const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer32:
    {
      out.Write_int32(static_cast<int32_t const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned8:
    {
      out.Write_uint8(static_cast<uint8_t const *>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned16:
    {
      out.Write_uint16(static_cast<uint16_t const *>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned32:
    {
      out.Write_uint32(static_cast<uint32_t const *>(pNativeData), nDataElements);
      break;
    }

    case DataType::real32:
    {
      out.Write_float(static_cast<float const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::visible_string:
    {
      size_t const len = strnlen(static_cast<char const *>(pNativeData), nDataElements);

      if (completeAccess)
      {
        out.Write_char(static_cast<char const*>(pNativeData), len);
        out.FillBytes(nDataElements - len, 0x00U);
      }
      else
      {
        out.Write_char(static_cast<char const*>(pNativeData), len);
        if (len < nDataElements)
          out.Write_uint8(0x00U);
      }

      break;
    }

    case DataType::octet_string:
    {
      out.Write_uint8(static_cast<uint8_t const *>(pNativeData), nDataElements);
      break;
    }

    case DataType::unicode_string:
    {
      out.Write_uint16(static_cast<uint16_t const *>(pNativeData), nDataElements);
      break;
    }

    case DataType::real64:
    {
      out.Write_double(static_cast<double const *>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer64:
    {
      out.Write_int64(static_cast<int64_t const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned64:
    {
      out.Write_uint64(static_cast<uint64_t const*>(pNativeData), nDataElements);
      break;
    }

    case DataType::bit1:
    case DataType::bit2:
    case DataType::bit3:
    case DataType::bit4:
    case DataType::bit5:
    case DataType::bit6:
    case DataType::bit7:
    case DataType::bit8:
    case DataType::boolean_native_bit1:
    {
      // get a pointer to the first byte of native data containing the first bit
      uint8_t const * p = static_cast<uint8_t const *>(pNativeData);

      // calculate number of bits to be written
      uint_fast32_t nBits = static_cast<uint_fast32_t>(nDataElements) * DataTypeBitLengthTable[static_cast<int>(type)];

      while (nBits != 0U)
      {
        uint_fast8_t chunkSize;
        if (nBits > 8U)
          chunkSize = 8U;
        else
          chunkSize = nBits;

        out.Write_Bits(*p++, chunkSize);
        nBits -= chunkSize;
      }
      break;
    }

    default:
      throw DataTypeNotSupportedError(type);
  } // switch (type)
}

/**
 * \brief Reads data in CANopen format from an @ref gpcc::Stream::IStreamReader and converts it to native data.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - data may have been read from `in`. The read-pointer will not be recovered in case of an exception.
 * - undefined/incomplete data may have been written to `pNativeData`
 *
 * \throws DataTypeNotSupportedError   CANopen data type is not supported ([details](@ref DataTypeNotSupportedError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - data may have been read from `in`. The read-pointer will not be recovered in case of deferred thread cancellation.
 * - undefined/incomplete data may have been written to `pNativeData`
 *
 * - - -
 *
 * \param in
 * @ref gpcc::Stream::IStreamReader from which the data shall be read. The data shall be encoded in CANopen format.
 *
 * \param type
 * CANopen type of data.
 *
 * \param nDataElements
 * Number of data elements. If this is larger than one, then the native data will be treated as
 * an array of native data elements.\n
 * Note that depending on the data type, native data may be stuffed (-> @ref IsNativeDataStuffed()).\n
 * Zero is not allowed.
 *
 * \param completeAccess
 * Indicates if this is invoked during a complete access.\n
 * true = complete access (@p nDataElements elements of data will be read from @p in)\n
 * false = single subindex access (for data types with flexible length, less than @p nDataElements elements of data
 *         may be contained in @p in. Data will be read until either the stream is empty or @p nDataElements of
 *         data have been read)
 *
 * \param pNativeData
 * Pointer to the memory location where the data read from the @ref gpcc::Stream::IStreamReader shall be written
 * to after conversion to native format.\n
 * \n
 * Depending on parameter 'type', `pNativeData` will be casted to a pointer to a specific C/C++ data type
 * interpreted in the machine's native format. The documentation of the @ref DataType enumeration contains a list
 * of native C/C++ types associated with each CANopen data type.\n
 * \n
 * The size of the native data type may be larger than the size of the CANopen data type.
 * If the memory of the native data type is not completely filled with the read data,
 * then zeros will be written to native "spare" bits and bytes.\n
 * \n
 * Unused bytes in VISIBLE_STRING data will be filled with zeros.
 */
void Object::CANopenEncodedDataToNativeData(gpcc::Stream::IStreamReader & in,
                                            DataType const type,
                                            uint16_t const nDataElements,
                                            bool const completeAccess,
                                            void* const pNativeData)
{
  if (pNativeData == nullptr)
    throw std::invalid_argument("Object::CANopenEncodedDataToNativeData: pNativeData is nullptr");

  if (nDataElements == 0U)
    throw std::invalid_argument("Object::CANopenEncodedDataToNativeData: nDataElements is zero");

  switch (type)
  {
    case DataType::boolean:
    {
      in.Read_bool(static_cast<bool*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer8:
    {
      in.Read_int8(static_cast<int8_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer16:
    {
      in.Read_int16(static_cast<int16_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer32:
    {
      in.Read_int32(static_cast<int32_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned8:
    {
      in.Read_uint8(static_cast<uint8_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned16:
    {
      in.Read_uint16(static_cast<uint16_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned32:
    {
      in.Read_uint32(static_cast<uint32_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::real32:
    {
      in.Read_float(static_cast<float*>(pNativeData), nDataElements);
      break;
    }

    case DataType::visible_string:
    {
      char* pND = static_cast<char*>(pNativeData);

      if (completeAccess)
      {
        // (complete access)

        in.Read_char(pND, nDataElements);

        // ensure that anything behind the NUL-terminator is 0x00 in native data
        size_t const len = strnlen(pND, nDataElements);
        if (len < nDataElements)
          memset(pND + len, 0x00, nDataElements - len);
      }
      else
      {
        // (single subindex access)

        if (in.IsRemainingBytesSupported())
        {
          // (RemainingBytes() is supported)

          // figure out the amount of bytes in the stream
          auto rb = in.RemainingBytes();

          if (rb > nDataElements)
            rb = nDataElements;

          in.Read_char(pND, rb);

          // ensure that there is a NUL-terminator in native data and that anything behind the NUL-terminator is 0x00
          if (rb < nDataElements)
          {
            pND[rb] = 0;

            // Check for first NUL-terminator in read data. Maybe there was one inside the data
            rb = strnlen(pND, nDataElements) + 1U;
            if (rb < nDataElements)
              memset(pND + rb, 0x00, nDataElements - rb);
          }
        }
        else
        {
          // (RemainingBytes() not supported)

          uint_fast16_t n = nDataElements;

          while (n != 0U)
          {
            char c;
            if (in.GetState() != gpcc::Stream::IStreamReader::States::empty)
              c = in.Read_char();
            else
              c = 0;

            if (c == 0)
            {
              // fill the rest with 0x00
              memset(pND, 0x00, n);

              // skip the rest, but be aware of the end of the stream
              while ((in.GetState() != gpcc::Stream::IStreamReader::States::empty) && (--n != 0U))
              {
                in.Skip(8U);
              }

              // finished
              break;
            }

            *pND++ = c;
            n--;
          } // while (n != 0U)
        }
      } // if (completeAccess)... else...

      break;
    }

    case DataType::octet_string:
    {
      in.Read_uint8(static_cast<uint8_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unicode_string:
    {
      in.Read_uint16(static_cast<uint16_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::real64:
    {
      in.Read_double(static_cast<double*>(pNativeData), nDataElements);
      break;
    }

    case DataType::integer64:
    {
      in.Read_int64(static_cast<int64_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::unsigned64:
    {
      in.Read_uint64(static_cast<uint64_t*>(pNativeData), nDataElements);
      break;
    }

    case DataType::bit1:
    case DataType::bit2:
    case DataType::bit3:
    case DataType::bit4:
    case DataType::bit5:
    case DataType::bit6:
    case DataType::bit7:
    case DataType::bit8:
    case DataType::boolean_native_bit1:
    {
      // get a pointer to the first byte of native data where the first bit shall be written to
      uint8_t* p = static_cast<uint8_t*>(pNativeData);

      // calculate number of bits to be read
      uint_fast32_t nBits = static_cast<uint_fast32_t>(nDataElements) * DataTypeBitLengthTable[static_cast<int>(type)];

      while (nBits != 0U)
      {
        uint_fast8_t chunkSize;
        if (nBits > 8U)
          chunkSize = 8U;
        else
          chunkSize = nBits;

        in.Read_bits(p++, chunkSize);
        nBits -= chunkSize;
      }
      break;
    }

    default:
      throw DataTypeNotSupportedError(type);
  } // switch (type)
}

} // namespace cood
} // namespace gpcc
