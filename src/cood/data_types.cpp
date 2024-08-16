/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018, 2024 Daniel Jerolm
*/

#include <gpcc/cood/data_types.hpp>
#include <gpcc/cood/exceptions.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/string/tools.hpp>
#include <exception>
#include <limits>
#include <stdexcept>
#include <cstdio>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Look-up-table containing the sizes in bit of one element of data __encoded in CANopen format__ for the
 *        different CANopen types (@ref DataType enum).
 *
 * @ref DataType enum values can be casted to `int` and used as an index to access this field:
 * ~~~{.cpp}
 * uint8_t bw = DataTypeBitLengthTable[static_cast<int>(type)];
 * ~~~
 *
 * A bit-width of zero indicates that the data type is either reserved or not supported by GPCC's object dictionary
 * implementation.
 */
uint8_t const DataTypeBitLengthTable[65] =
{
  /* null                        (0x0000U) */  1U,
  /* boolean                     (0x0001U) */  1U,
  /* integer8                    (0x0002U) */  8U,
  /* integer16                   (0x0003U) */ 16U,
  /* integer32                   (0x0004U) */ 32U,
  /* unsigned8                   (0x0005U) */  8U,
  /* unsigned16                  (0x0006U) */ 16U,
  /* unsigned32                  (0x0007U) */ 32U,
  /* real32                      (0x0008U) */ 32U,
  /* visible_string              (0x0009U) */  8U,
  /* octet_string                (0x000AU) */  8U,
  /* unicode_string              (0x000BU) */ 16U,
  /* time_of_day                 (0x000CU) */ 48U,
  /* time_difference             (0x000DU) */ 48U,
  /* reserved_0x000E             (0x000EU) */  0U,
  /* domain                      (0x000FU) */  0U,
  /* integer24                   (0x0010U) */ 24U,
  /* real64                      (0x0011U) */ 64U,
  /* integer40                   (0x0012U) */ 40U,
  /* integer48                   (0x0013U) */ 48U,
  /* integer56                   (0x0014U) */ 56U,
  /* integer64                   (0x0015U) */ 64U,
  /* unsigned24                  (0x0016U) */ 24U,
  /* reserved_0x0017             (0x0017U) */  0U,
  /* unsigned40                  (0x0018U) */ 40U,
  /* unsigned48                  (0x0019U) */ 48U,
  /* unsigned56                  (0x001AU) */ 56U,
  /* unsigned64                  (0x001BU) */ 64U,
  /* reserved_0x001C             (0x001CU) */  0U,
  /* reserved_0x001D             (0x001DU) */  0U,
  /* reserved_0x001E             (0x001EU) */  0U,
  /* reserved_0x001F             (0x001FU) */  0U,
  /* pdo_communication_parameter (0x0020U) */  0U,
  /* pdo_mapping                 (0x0021U) */  0U,
  /* sdo_parameter               (0x0022U) */  0U,
  /* identity                    (0x0023U) */  0U,
  /* reserved_0x0024             (0x0024U) */  0U,
  /* commandpar                  (0x0025U) */  0U,
  /* reserved_0x0026             (0x0026U) */  0U,
  /* reserved_0x0027             (0x0027U) */  0U,
  /* reserved_0x0028             (0x0028U) */  0U,
  /* syncpar                     (0x0029U) */  0U,
  /* reserved_0x002A             (0x002AU) */  0U,
  /* reserved_0x002B             (0x002BU) */  0U,
  /* reserved_0x002C             (0x002CU) */  0U,
  /* reserved_0x002D             (0x002DU) */  0U,
  /* reserved_0x002E             (0x002EU) */  0U,
  /* reserved_0x002F             (0x002FU) */  0U,
  /* bit1                        (0x0030U) */  1U,
  /* bit2                        (0x0031U) */  2U,
  /* bit3                        (0x0032U) */  3U,
  /* bit4                        (0x0033U) */  4U,
  /* bit5                        (0x0034U) */  5U,
  /* bit6                        (0x0035U) */  6U,
  /* bit7                        (0x0036U) */  7U,
  /* bit8                        (0x0037U) */  8U,
  /* reserved_0x0038             (0x0038U) */  0U,
  /* reserved_0x0039             (0x0039U) */  0U,
  /* reserved_0x003A             (0x003AU) */  0U,
  /* reserved_0x003B             (0x003BU) */  0U,
  /* reserved_0x003C             (0x003CU) */  0U,
  /* reserved_0x003D             (0x003DU) */  0U,
  /* reserved_0x003E             (0x003EU) */  0U,
  /* reserved_0x003F             (0x003FU) */  0U,

  /* boolean_native_bit1         (0x0040U) */  1U
};

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Look-up-table containing the __native sizes__ in bit of one element of data for the different
 *        CANopen types (@ref DataType enum).
 *
 * @ref DataType enum values can be casted to int and used as an index to access this field:
 * ~~~{.cpp}
 * uint8_t bw = NativeDataTypeBitLengthTable[static_cast<int>(type)];
 * ~~~
 *
 * A bit-width of zero indicates that the data type is either reserved or not supported by GPCC's object dictionary
 * implementation.
 */
uint8_t const NativeDataTypeBitLengthTable[65] =
{
  /* null                        (0x0000U) */ 0U,
  /* boolean                     (0x0001U) */ sizeof(bool) * 8U,
  /* integer8                    (0x0002U) */ sizeof(int8_t) * 8U,
  /* integer16                   (0x0003U) */ sizeof(int16_t) * 8U,
  /* integer32                   (0x0004U) */ sizeof(int32_t) * 8U,
  /* unsigned8                   (0x0005U) */ sizeof(uint8_t) * 8U,
  /* unsigned16                  (0x0006U) */ sizeof(uint16_t) * 8U,
  /* unsigned32                  (0x0007U) */ sizeof(uint32_t) * 8U,
  /* real32                      (0x0008U) */ sizeof(float) * 8U,
  /* visible_string              (0x0009U) */ sizeof(char) * 8U,
  /* octet_string                (0x000AU) */ sizeof(uint8_t) * 8U,
  /* unicode_string              (0x000BU) */ sizeof(uint16_t) * 8U,
  /* time_of_day                 (0x000CU) */ 0U,
  /* time_difference             (0x000DU) */ 0U,
  /* reserved_0x000E             (0x000EU) */ 0U,
  /* domain                      (0x000FU) */ 0U,
  /* integer24                   (0x0010U) */ 0U,
  /* real64                      (0x0011U) */ sizeof(double) * 8U,
  /* integer40                   (0x0012U) */ 0U,
  /* integer48                   (0x0013U) */ 0U,
  /* integer56                   (0x0014U) */ 0U,
  /* integer64                   (0x0015U) */ sizeof(int64_t) * 8U,
  /* unsigned24                  (0x0016U) */ 0U,
  /* reserved_0x0017             (0x0017U) */ 0U,
  /* unsigned40                  (0x0018U) */ 0U,
  /* unsigned48                  (0x0019U) */ 0U,
  /* unsigned56                  (0x001AU) */ 0U,
  /* unsigned64                  (0x001BU) */ sizeof(uint64_t) * 8U,
  /* reserved_0x001C             (0x001CU) */ 0U,
  /* reserved_0x001D             (0x001DU) */ 0U,
  /* reserved_0x001E             (0x001EU) */ 0U,
  /* reserved_0x001F             (0x001FU) */ 0U,
  /* pdo_communication_parameter (0x0020U) */ 0U,
  /* pdo_mapping                 (0x0021U) */ 0U,
  /* sdo_parameter               (0x0022U) */ 0U,
  /* identitiy                   (0x0023U) */ 0U,
  /* reserved_0x0024             (0x0024U) */ 0U,
  /* commandpar                  (0x0025U) */ 0U,
  /* reserved_0x0026             (0x0026U) */ 0U,
  /* reserved_0x0027             (0x0027U) */ 0U,
  /* reserved_0x0028             (0x0028U) */ 0U,
  /* syncpar                     (0x0029U) */ 0U,
  /* reserved_0x002A             (0x002AU) */ 0U,
  /* reserved_0x002B             (0x002BU) */ 0U,
  /* reserved_0x002C             (0x002CU) */ 0U,
  /* reserved_0x002D             (0x002DU) */ 0U,
  /* reserved_0x002E             (0x002EU) */ 0U,
  /* reserved_0x002F             (0x002FU) */ 0U,
  /* bit1                        (0x0030U) */ sizeof(uint8_t) * 8U,
  /* bit2                        (0x0031U) */ sizeof(uint8_t) * 8U,
  /* bit3                        (0x0032U) */ sizeof(uint8_t) * 8U,
  /* bit4                        (0x0033U) */ sizeof(uint8_t) * 8U,
  /* bit5                        (0x0034U) */ sizeof(uint8_t) * 8U,
  /* bit6                        (0x0035U) */ sizeof(uint8_t) * 8U,
  /* bit7                        (0x0036U) */ sizeof(uint8_t) * 8U,
  /* bit8                        (0x0037U) */ sizeof(uint8_t) * 8U,
  /* reserved_0x0038             (0x0038U) */ 0U,
  /* reserved_0x0039             (0x0039U) */ 0U,
  /* reserved_0x003A             (0x003AU) */ 0U,
  /* reserved_0x003B             (0x003BU) */ 0U,
  /* reserved_0x003C             (0x003CU) */ 0U,
  /* reserved_0x003D             (0x003DU) */ 0U,
  /* reserved_0x003E             (0x003EU) */ 0U,
  /* reserved_0x003F             (0x003FU) */ 0U,

  /* boolean_native_bit1         (0x0040U) */ sizeof(uint8_t) * 8U
};


/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Retrieves a string containing the name of an @ref DataType enum value.
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
 * \param dt
 * Enum value.
 *
 * \return
 * Null-terminated c-string located in code memory.\n
 * The string contains the name of the enum value `dt`.
 */
char const * DataTypeToString(DataType const dt)
{
  switch (dt)
  {
    case DataType::null                        : return "NULL";
    case DataType::boolean                     : return "BOOLEAN";
    case DataType::integer8                    : return "INTEGER8";
    case DataType::integer16                   : return "INTEGER16";
    case DataType::integer32                   : return "INTEGER32";
    case DataType::unsigned8                   : return "UNSIGNED8";
    case DataType::unsigned16                  : return "UNSIGNED16";
    case DataType::unsigned32                  : return "UNSIGNED32";
    case DataType::real32                      : return "REAL32";
    case DataType::visible_string              : return "VISIBLE_STRING";
    case DataType::octet_string                : return "OCTET_STRING";
    case DataType::unicode_string              : return "UNICODE_STRING";
    case DataType::time_of_day                 : return "TIME_OF_FDAY";
    case DataType::time_difference             : return "TIME_DIFFERENCE";
    case DataType::reserved_0x000E             : return "RESERVED_0x000E";
    case DataType::domain                      : return "DOMAIN";
    case DataType::integer24                   : return "INTEGER24";
    case DataType::real64                      : return "REAL64";
    case DataType::integer40                   : return "INTEGER40";
    case DataType::integer48                   : return "INTEGER48";
    case DataType::integer56                   : return "INTEGER56";
    case DataType::integer64                   : return "INTEGER64";
    case DataType::unsigned24                  : return "UNSIGNED24";
    case DataType::reserved_0x0017             : return "RESERVED_0x0017";
    case DataType::unsigned40                  : return "UNSIGNED40";
    case DataType::unsigned48                  : return "UNSIGNED48";
    case DataType::unsigned56                  : return "UNSIGNED56";
    case DataType::unsigned64                  : return "UNSIGNED64";
    case DataType::reserved_0x001C             : return "RESERVED_0x001C";
    case DataType::reserved_0x001D             : return "RESERVED_0x001D";
    case DataType::reserved_0x001E             : return "RESERVED_0x001E";
    case DataType::reserved_0x001F             : return "RESERVED_0x001F";
    case DataType::pdo_communication_parameter : return "PDO_COM_PARAM";
    case DataType::pdo_mapping                 : return "PDO_MAPPING";
    case DataType::sdo_parameter               : return "SDO_PARAMETER";
    case DataType::identity                    : return "IDENTITY";
    case DataType::reserved_0x0024             : return "RESERVED_0x0024";
    case DataType::commandpar                  : return "COMMANDPAR";
    case DataType::reserved_0x0026             : return "RESERVED_0x0026";
    case DataType::reserved_0x0027             : return "RESERVED_0x0027";
    case DataType::reserved_0x0028             : return "RESERVED_0x0028";
    case DataType::syncpar                     : return "SYNCPAR";
    case DataType::reserved_0x002A             : return "RESERVED_0x002A";
    case DataType::reserved_0x002B             : return "RESERVED_0x002B";
    case DataType::reserved_0x002C             : return "RESERVED_0x002C";
    case DataType::reserved_0x002D             : return "RESERVED_0x002D";
    case DataType::reserved_0x002E             : return "RESERVED_0x002E";
    case DataType::reserved_0x002F             : return "RESERVED_0x002F";
    case DataType::bit1                        : return "BIT1";
    case DataType::bit2                        : return "BIT2";
    case DataType::bit3                        : return "BIT3";
    case DataType::bit4                        : return "BIT4";
    case DataType::bit5                        : return "BIT5";
    case DataType::bit6                        : return "BIT6";
    case DataType::bit7                        : return "BIT7";
    case DataType::bit8                        : return "BIT8";
    case DataType::reserved_0x0038             : return "RESERVED_0x0038";
    case DataType::reserved_0x0039             : return "RESERVED_0x0039";
    case DataType::reserved_0x003A             : return "RESERVED_0x003A";
    case DataType::reserved_0x003B             : return "RESERVED_0x003B";
    case DataType::reserved_0x003C             : return "RESERVED_0x003C";
    case DataType::reserved_0x003D             : return "RESERVED_0x003D";
    case DataType::reserved_0x003E             : return "RESERVED_0x003E";
    case DataType::reserved_0x003F             : return "RESERVED_0x003F";

    case DataType::boolean_native_bit1         : return "BOOLEAN (native BIT1)";
  }

  throw std::invalid_argument("DataTypeToString: Unknown/invalid enum value");
}

/**
 * \brief Converts a value from the @ref DataType enumeration into an uint16_t.
 *
 * @ref ToDataType(uint16_t const value) is the counterpart of this.
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
 * \param dt
 * Value that shall be converted into an uint16_t.
 *
 * \return
 * Enum value `dt` expressed as uint16_t.
 */
uint16_t ToUint16(DataType const dt) noexcept
{
  return static_cast<uint16_t>(dt);
}

/**
 * \brief Safely converts an uint16_t value into a value from the @ref DataType enumeration.
 *
 * This is the counterpart of @ref ToUint16(DataType const dt).
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
 * Value that shall be converted into a value from the @ref DataType enumeration.\n
 * This function is well aware of invalid values.
 *
 * \return
 * Value from the @ref DataType enumeration that corresponds to parameter `value`.
 */
DataType ToDataType(uint16_t const value)
{
  switch (value)
  {
    case static_cast<uint16_t>(DataType::null)                       : return DataType::null;
    case static_cast<uint16_t>(DataType::boolean)                    : return DataType::boolean;
    case static_cast<uint16_t>(DataType::integer8)                   : return DataType::integer8;
    case static_cast<uint16_t>(DataType::integer16)                  : return DataType::integer16;
    case static_cast<uint16_t>(DataType::integer32)                  : return DataType::integer32;
    case static_cast<uint16_t>(DataType::unsigned8)                  : return DataType::unsigned8;
    case static_cast<uint16_t>(DataType::unsigned16)                 : return DataType::unsigned16;
    case static_cast<uint16_t>(DataType::unsigned32)                 : return DataType::unsigned32;
    case static_cast<uint16_t>(DataType::real32)                     : return DataType::real32;
    case static_cast<uint16_t>(DataType::visible_string)             : return DataType::visible_string;
    case static_cast<uint16_t>(DataType::octet_string)               : return DataType::octet_string;
    case static_cast<uint16_t>(DataType::unicode_string)             : return DataType::unicode_string;
    case static_cast<uint16_t>(DataType::time_of_day)                : return DataType::time_of_day;
    case static_cast<uint16_t>(DataType::time_difference)            : return DataType::time_difference;
    case static_cast<uint16_t>(DataType::reserved_0x000E)            : return DataType::reserved_0x000E;
    case static_cast<uint16_t>(DataType::domain)                     : return DataType::domain;
    case static_cast<uint16_t>(DataType::integer24)                  : return DataType::integer24;
    case static_cast<uint16_t>(DataType::real64)                     : return DataType::real64;
    case static_cast<uint16_t>(DataType::integer40)                  : return DataType::integer40;
    case static_cast<uint16_t>(DataType::integer48)                  : return DataType::integer48;
    case static_cast<uint16_t>(DataType::integer56)                  : return DataType::integer56;
    case static_cast<uint16_t>(DataType::integer64)                  : return DataType::integer64;
    case static_cast<uint16_t>(DataType::unsigned24)                 : return DataType::unsigned24;
    case static_cast<uint16_t>(DataType::reserved_0x0017)            : return DataType::reserved_0x0017;
    case static_cast<uint16_t>(DataType::unsigned40)                 : return DataType::unsigned40;
    case static_cast<uint16_t>(DataType::unsigned48)                 : return DataType::unsigned48;
    case static_cast<uint16_t>(DataType::unsigned56)                 : return DataType::unsigned56;
    case static_cast<uint16_t>(DataType::unsigned64)                 : return DataType::unsigned64;
    case static_cast<uint16_t>(DataType::reserved_0x001C)            : return DataType::reserved_0x001C;
    case static_cast<uint16_t>(DataType::reserved_0x001D)            : return DataType::reserved_0x001D;
    case static_cast<uint16_t>(DataType::reserved_0x001E)            : return DataType::reserved_0x001E;
    case static_cast<uint16_t>(DataType::reserved_0x001F)            : return DataType::reserved_0x001F;
    case static_cast<uint16_t>(DataType::pdo_communication_parameter): return DataType::pdo_communication_parameter;
    case static_cast<uint16_t>(DataType::pdo_mapping)                : return DataType::pdo_mapping;
    case static_cast<uint16_t>(DataType::sdo_parameter)              : return DataType::sdo_parameter;
    case static_cast<uint16_t>(DataType::identity)                   : return DataType::identity;
    case static_cast<uint16_t>(DataType::reserved_0x0024)            : return DataType::reserved_0x0024;
    case static_cast<uint16_t>(DataType::commandpar)                 : return DataType::commandpar;
    case static_cast<uint16_t>(DataType::reserved_0x0026)            : return DataType::reserved_0x0026;
    case static_cast<uint16_t>(DataType::reserved_0x0027)            : return DataType::reserved_0x0027;
    case static_cast<uint16_t>(DataType::reserved_0x0028)            : return DataType::reserved_0x0028;
    case static_cast<uint16_t>(DataType::syncpar)                    : return DataType::syncpar;
    case static_cast<uint16_t>(DataType::reserved_0x002A)            : return DataType::reserved_0x002A;
    case static_cast<uint16_t>(DataType::reserved_0x002B)            : return DataType::reserved_0x002B;
    case static_cast<uint16_t>(DataType::reserved_0x002C)            : return DataType::reserved_0x002C;
    case static_cast<uint16_t>(DataType::reserved_0x002D)            : return DataType::reserved_0x002D;
    case static_cast<uint16_t>(DataType::reserved_0x002E)            : return DataType::reserved_0x002E;
    case static_cast<uint16_t>(DataType::reserved_0x002F)            : return DataType::reserved_0x002F;;
    case static_cast<uint16_t>(DataType::bit1)                       : return DataType::bit1;
    case static_cast<uint16_t>(DataType::bit2)                       : return DataType::bit2;
    case static_cast<uint16_t>(DataType::bit3)                       : return DataType::bit3;
    case static_cast<uint16_t>(DataType::bit4)                       : return DataType::bit4;
    case static_cast<uint16_t>(DataType::bit5)                       : return DataType::bit5;
    case static_cast<uint16_t>(DataType::bit6)                       : return DataType::bit6;
    case static_cast<uint16_t>(DataType::bit7)                       : return DataType::bit7;
    case static_cast<uint16_t>(DataType::bit8)                       : return DataType::bit8;
    case static_cast<uint16_t>(DataType::reserved_0x0038)            : return DataType::reserved_0x0038;
    case static_cast<uint16_t>(DataType::reserved_0x0039)            : return DataType::reserved_0x0039;
    case static_cast<uint16_t>(DataType::reserved_0x003A)            : return DataType::reserved_0x003A;
    case static_cast<uint16_t>(DataType::reserved_0x003B)            : return DataType::reserved_0x003B;
    case static_cast<uint16_t>(DataType::reserved_0x003C)            : return DataType::reserved_0x003C;
    case static_cast<uint16_t>(DataType::reserved_0x003D)            : return DataType::reserved_0x003D;
    case static_cast<uint16_t>(DataType::reserved_0x003E)            : return DataType::reserved_0x003E;
    case static_cast<uint16_t>(DataType::reserved_0x003F)            : return DataType::reserved_0x003F;

    case static_cast<uint16_t>(DataType::boolean_native_bit1)        : return DataType::boolean_native_bit1;
  }

  throw std::invalid_argument("ToDataType: 'value' invalid");
}

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Reads the data of one subindex (encoded in CANopen format) from an @ref gpcc::stream::IStreamReader and
 *        generates a human-readable string representation of the data value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Data may be read from the IStreamReader `sr`. The read-pointer will not be recovered in case of an error.
 *
 * \throws DataTypeNotSupportedError   CANopen data type is not supported by this function
 *                                     ([details](@ref DataTypeNotSupportedError).
 *
 * \throws std::bad_alloc              Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Data may be read from the IStreamReader `sr`. The read-pointer will not be recovered in case of an error.
 *
 * - - -
 *
 * \param sr
 * Reference to an @ref gpcc::stream::IStreamReader from which the CANopen encoded data shall be read.
 *
 * \param sizeInBit
 * Size (in bit) of the CANopen encoded data to be read from `sr`.\n
 * Note:\n
 * - For @ref DataType::null, @ref DataType::visible_string, @ref DataType::octet_string and
 *   @ref DataType::unicode_string this may be any integer multiple of the size of the CANopen data type.
 * - For @ref DataType::null and @ref DataType::visible_string this may be zero.
 * - For @ref DataType::visible_string, this function is aware of a potential NUL-terminator anywhere
 *   inside the data read from `sr`.
 * - For all other data types, this must match the size of the CANopen data type.
 * - In almost any use case, `sr` will contain the data of a subindex that has been read from an object
 *   via @ref Object::Read(). \n
 *   In these cases, `sizeInBit` should be retrieved from @ref Object::GetSubIdxActualSize().
 *
 * \param type
 * Data type of the data in `sr`.\n
 * The _additional_ @ref DataType enum values invented by GPCC providing an alternative native representation of a
 * _true_ CANopen data type are not supported. This is by intention, because these data types will never be returned by
 * any query on class @ref Object.
 *
 * \return
 * The generated human-readable string.
 */
std::string CANopenEncodedDataToString(gpcc::stream::IStreamReader& sr, size_t const sizeInBit, DataType const type)
{
  // check sizeInBit
  switch (type)
  {
    case DataType::null:
      break;

    case DataType::visible_string:
      if (sizeInBit == 0U)
        return "\"\"";

      // intentional fall-through

    case DataType::octet_string:
    case DataType::unicode_string:
    {
      uint8_t const bitSize = DataTypeBitLengthTable[static_cast<int>(type)];
      if (   (sizeInBit < bitSize)
          || (sizeInBit % bitSize != 0U))
      {
        throw std::invalid_argument("CANopenEncodedDataToString: 'sizeInBit' is invalid");
      }
      break;
    }

    default:
    {
      if (DataTypeBitLengthTable[static_cast<int>(type)] != sizeInBit)
        throw std::invalid_argument("CANopenEncodedDataToString: 'sizeInBit' is invalid");
    }
  } // switch (type)

  // convert to string
  switch (type)
  {
    case DataType::null:
    {
      sr.Skip(sizeInBit);
      return "";
    }

    case DataType::boolean:
    {
      bool const b = sr.Read_bool();

      if (b)
        return "TRUE";
      else
        return "FALSE";
    }

    case DataType::integer8:
    {
      int8_t const i8 = sr.Read_int8();
      return std::to_string(i8);
    }

    case DataType::integer16:
    {
      int16_t const i16 = sr.Read_int16();
      return std::to_string(i16);
    }

    case DataType::integer32:
    {
      int32_t const i32 = sr.Read_int32();
      return std::to_string(i32);
    }

    case DataType::unsigned8:
    {
      uint8_t const ui8 = sr.Read_uint8();
      return gpcc::string::ToDecAndHex(ui8, 2U);
    }

    case DataType::unsigned16:
    {
      uint16_t const ui16 = sr.Read_uint16();
      return gpcc::string::ToDecAndHex(ui16, 4U);
    }

    case DataType::unsigned32:
    {
      uint32_t const ui32 = sr.Read_uint32();
      return gpcc::string::ToDecAndHex(ui32, 8U);
    }

    case DataType::real32:
    {
      float const r32 = sr.Read_float();
      char buf[32];
      size_t const n = snprintf(buf, sizeof(buf), "%G", r32);
      if (n >= sizeof(buf))
        throw std::runtime_error("CANopenEncodedDataToString: snprintf requires unexpected buffer size");

      return buf;
    }

    case DataType::visible_string:
    {
      size_t n = sizeInBit / 8U;

      std::string s;
      s.reserve(n+2);

      s += '"';
      while (n-- != 0U)
      {
        char const c = sr.Read_char();
        if (c == 0)
        {
          sr.Skip(n * 8U);
          break;
        }
        s += c;
      }
      s += '"';

      return s;
    }

    case DataType::octet_string:
    {
      size_t n = sizeInBit / 8U;

      std::string s("(hex) ");
      s.reserve(6U + n * 3U - 1U);

      while (n-- != 0U)
      {
        uint8_t const value = sr.Read_uint8();

        s += gpcc::string::ToHexNoPrefix(value, 2U);
        if (n != 0U)
          s += ' ';
      }

      return s;
    }

    case DataType::unicode_string:
    {
      size_t n = sizeInBit / 16U;

      std::string s("(hex) ");
      s.reserve(6U + n * 5U - 1U);

      while (n-- != 0U)
      {
        uint16_t const value = sr.Read_uint16();

        s += gpcc::string::ToHexNoPrefix(value, 4U);
        if (n != 0U)
          s += ' ';
      }

      return s;
    }

    case DataType::real64:
    {
      double const r64 = sr.Read_double();

      char buf[32];
      size_t const n = snprintf(buf, sizeof(buf), "%G", r64);
      if (n >= sizeof(buf))
        throw std::runtime_error("CANopenEncodedDataToString: snprintf requires unexpected buffer size");

      return buf;
    }

    case DataType::integer64:
    {
      int64_t const i64 = sr.Read_int64();
      return std::to_string(i64);
    }

    case DataType::unsigned64:
    {
      uint64_t const u64 = sr.Read_uint64();
      std::string s = std::to_string(u64);
      s += " (0x";
      s += gpcc::string::ToHexNoPrefix(static_cast<uint32_t>(u64 >> 32U), 8U);
      s += '.';
      s += gpcc::string::ToHexNoPrefix(static_cast<uint32_t>(u64 & 0xFFFFFFFFUL), 8U);
      s += ")";
      return s;
    }

    case DataType::bit1:
    case DataType::bit2:
    case DataType::bit3:
    case DataType::bit4:
    case DataType::bit5:
    case DataType::bit6:
    case DataType::bit7:
    case DataType::bit8:
    {
      // read bits
      uint8_t const bits = sr.Read_bits(sizeInBit);
      return gpcc::string::ToBin(bits, sizeInBit);
    }

    case DataType::boolean_native_bit1:
      throw std::invalid_argument("CANopenEncodedDataToString: 'type' is not a data type defined by CANopen");

    default:
      throw DataTypeNotSupportedError(type);
  } // switch (type)
}

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Converts a human-readable string representation of the data of one subindex into CANopen encoded data and
 *        writes the CANopen encoded data into an @ref gpcc::stream::IStreamWriter.
 *
 * This function is intended to process user input. It is aware of all sort of erroneous input strings.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - Undefined or incomplete data may have been written to the IStreamWriter `sw`.
 *
 * \throws DataTypeNotSupportedError   CANopen data type is not supported by this function
 *                                     ([details](@ref DataTypeNotSupportedError)).
 *
 * \throws std::invalid_argument       `sizeInBit` does not match `type`, or `s` cannot be converted to `type`.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - Undefined or incomplete data may have been written to the IStreamWriter `sw`.
 *
 * - - -
 *
 * \param s
 * Human-readable string representation of the data.\n
 * __Required format:__\n
 * _BOOLEAN_:\n
 * TRUE, FALSE, true, false\n
 * \n
 * _INTEGER8/16/32/64_\n
 * Examples: 5, -8\n
 * \n
 * _UNSIGNED8/16/32_\n
 * Examples: 5, 0xA7, 0b01101001\n
 * \n
 * _REAL32/64_\n
 * [+|-]digits[.][digits][(e|E)[+|-]digits]\n
 * \n
 * _VISIBLE_STRING_\n
 * Example: Text\n
 * Example: Text "Text-in-double-quotes" text\n
 * Double-quotes can be used within the string as shown above. No escape characters are necessary.\n
 * If less characters than `sizeInBit`/8 are provided in `s`, then the remaining space will be filled with
 * NUL (0x00) characters.\n
 * If more characters than `sizeInBit`/8 are provided in `s`, then an exception will be thrown.\n
 * \n
 * _OCTET_STRING_\n
 * Example: 5B\n
 * One byte must be provided in hex-format (two digits, without prefix '0x') in `s`. If the octet-string is comprised
 * of multiple bytes, then this must be called multiple times, each time with exactly one byte (= two digits) of data
 * provided in `s`. @ref gpcc::string::Split() may be used to process such a string.\n
 * \n
 * _UNICODE_STRING_\n
 * Example: 5B3E\n
 * One 16bit-word must be provided in hex-format (four digits, without prefix '0x') in `s`. If the unicode-string is
 * comprised of multiple 16bit-words, then this must be called multiple times, each time with exactly one 16bit-word
 * (= two digits) of data provided in `s`. @ref gpcc::string::Split() may be used to process such a string.\n
 * \n
 * _bit1..bit8_\n
 * Examples: 0, 1, 3, 0x3, 0b11\n
 * Unused upper bits must always be zero.
 *
 * \param sizeInBit
 * Size (in bit) of the CANopen encoded data that shall be written into `sw`.\n
 * Note:
 * - Zero is not allowed.
 * - For @ref DataType::visible_string this may be an integer multiple of the size of the CANopen data type.
 *   Unused bytes will be filled with zeros (NUL-terminator).
 * - For @ref DataType::octet_string (and @ref DataType::unicode_string), this parameter may be larger than 8 (16),
 *   but only one byte (16-bit word) of data must be contained in 's' and this method will only write one byte
 *   (16bit-word) of data into `sw`.\n
 *   If the octet string (unicode string) is comprised of multiple bytes (16bit-words), then this method must be
 *   called multiple times. This parameter must not necessarily be decreased among multiple calls to this method
 *   when processing the data elements of an octet string (unicode string).\n
 *   The caller may use @ref gpcc::string::Split() to process a string containing multiple bytes (16bit-words).
 * - For all other types, this must match the size of the CANopen data type.
 *
 * \param type
 * Desired CANopen data type for the data stored in `sw`.\n
 * The _additional_ @ref DataType enum values invented by GPCC providing an alternative native representation of a
 * _true_ CANopen data type are not supported. This is by intention, because these data types will never be returned by
 * any query on class @ref Object.
 *
 * \param sw
 * The CANopen encoded data will be written into the referenced @ref gpcc::stream::IStreamWriter.
 */
void StringToCANOpenEncodedData(std::string const & s, size_t const sizeInBit, DataType const type, gpcc::stream::IStreamWriter& sw)
{
  // check sizeInBit
  switch (type)
  {
    case DataType::null:
    {
      // Data type "null" is not supported. This will throw a DataTypeNotSupportedError in the second switch-case.
      break;
    }

    case DataType::visible_string:
    case DataType::octet_string:
    case DataType::unicode_string:
    {
      uint8_t const bitSize = DataTypeBitLengthTable[static_cast<int>(type)];
      if (   (sizeInBit < bitSize)
          || (sizeInBit % bitSize != 0U))
      {
        throw std::invalid_argument("StringToCANOpenEncodedData: 'sizeInBit' is invalid");
      }
      break;
    }

    default:
    {
      if (DataTypeBitLengthTable[static_cast<int>(type)] != sizeInBit)
        throw std::invalid_argument("StringToCANOpenEncodedData: 'sizeInBit' is invalid");
    }
  } // switch (type)

  try
  {
    // convert string to data
    switch (type)
    {
      case DataType::boolean:
      {
        bool b;
        if ((s == "TRUE") || (s == "true"))
          b = true;
        else if ((s == "FALSE") || (s == "false"))
          b = false;
        else
          throw std::invalid_argument("Expected: TRUE, FALSE, true, or false");

        sw.Write_bool(b);
        break;
      }

      case DataType::integer8:
      {
        int32_t const i32 = gpcc::string::DecimalToI32(s,
                                                       std::numeric_limits<int8_t>::min(),
                                                       std::numeric_limits<int8_t>::max());

        sw.Write_int8(static_cast<int8_t>(i32));
        break;
      }

      case DataType::integer16:
      {
        int32_t const i32 = gpcc::string::DecimalToI32(s,
                                                       std::numeric_limits<int16_t>::min(),
                                                       std::numeric_limits<int16_t>::max());

        sw.Write_int16(static_cast<int16_t>(i32));
        break;
      }

      case DataType::integer32:
      {
        int32_t const i32 = gpcc::string::DecimalToI32(s);
        sw.Write_int32(i32);
        break;
      }

      case DataType::unsigned8:
      {
        uint8_t const u8 = gpcc::string::AnyStringToU8(s);

        sw.Write_uint8(static_cast<uint8_t>(u8));
        break;
      }

      case DataType::unsigned16:
      {
        uint32_t const u32 = gpcc::string::AnyNumberToU32(s,
                                                          std::numeric_limits<uint16_t>::min(),
                                                          std::numeric_limits<uint16_t>::max());

        sw.Write_uint16(static_cast<uint16_t>(u32));
        break;
      }

      case DataType::unsigned32:
      {
        uint32_t const u32 = gpcc::string::AnyNumberToU32(s);
        sw.Write_uint32(u32);
        break;
      }

      case DataType::real32:
      {
        size_t n;
        float const f = std::stof(s, &n);
        if (n != s.length())
          throw std::invalid_argument("Expected: d[.d][+-E+-d]");

        sw.Write_float(f);
        break;
      }

      case DataType::visible_string:
      {
        size_t const nBytes = sizeInBit / 8U;

        if (s.length() > nBytes)
          throw std::invalid_argument("String is too large");

        size_t nZeros = nBytes - s.length();

        // write string (null-terminator not included)
        sw.Write_char(s.c_str(), s.length());

        // fill the rest with NULLs
        sw.FillBytes(nZeros, 0x00U);

        break;
      }

      case DataType::octet_string:
      {
        uint8_t const u8 = gpcc::string::TwoDigitHexToU8(s);
        sw.Write_uint8(u8);
        break;
      }

      case DataType::unicode_string:
      {
        uint16_t const u16 = gpcc::string::FourDigitHexToU16(s);
        sw.Write_uint16(u16);
        break;
      }

      case DataType::real64:
      {
        size_t n;
        double const d = std::stod(s, &n);
        if (n != s.length())
          throw std::invalid_argument("Expected: d[.d][+-E+-d]");

        sw.Write_double(d);
        break;
      }

      case DataType::integer64:
      {
        size_t n;
        int64_t const i64 = std::stoll(s, &n);
        if (n != s.length())
          throw std::invalid_argument("Expected: -9223372036854775808..9223372036854775807");

        sw.Write_int64(i64);
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
      {
        uint8_t const bits = gpcc::string::AnyNumberToU32(s, 0U, (1UL << sizeInBit) - 1U);

        sw.Write_Bits(bits, sizeInBit);
        break;
      }

    case DataType::boolean_native_bit1:
      throw std::invalid_argument("StringToCANOpenEncodedData: 'type' is not a data type defined by CANopen");

      default:
        throw DataTypeNotSupportedError(type);
    } // switch (type)
  }
  catch (std::invalid_argument const &)
  {
    std::throw_with_nested(std::invalid_argument("StringToCANOpenEncodedData: Cannot convert 's' (" + s + ") to type " + DataTypeToString(type)));
  }
  catch (std::out_of_range const &)
  {
    std::throw_with_nested(std::invalid_argument("StringToCANOpenEncodedData: Cannot convert 's' (" + s + ") to type " + DataTypeToString(type)));
  }
}

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Translates the _additional_ @ref DataType enum values invented by GPCC that provide an alternative native
 *        representation of a _true_ CANopen data type to the original _true_ CANopen @ref DataType enum value.
 *
 * Example:
 * - @ref DataType::boolean translates to @ref DataType::boolean
 * - @ref DataType::boolean_native_bit1 translates to @ref DataType::boolean
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
 * \param dt
 * Date type.
 *
 * \return
 * Original (_true_) CANopen data type, if `dt` is a data type invented by GPCC that provides an alternative native
 * representation of the original data type.\n
 * If `dt` is a _true_ CANopen data type, then `dt` is returned without any modification.
 */
DataType MapAlternativeDataTypesToOriginalTypes(DataType const dt) noexcept
{
  switch (dt)
  {
    case DataType::boolean_native_bit1:
      return DataType::boolean;

    default:
      return dt;
  }
}

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Retrieves if CANopen data of a given @ref DataType is bit-based or not.
 *
 * Bit-based data of adjacent subindices encoded in a binary stream used for complete access to an object is always
 * bit-stuffed.
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
 * \param type
 * Data type.
 *
 * \retval true   Data type is bit-based.
 * \retval false  Data type is byte-based.
 */
bool IsDataTypeBitBased(DataType const type) noexcept
{
  return (   (type == DataType::null)
          || (type == DataType::boolean)
          || ((type >= DataType::bit1) && (type <= DataType::bit8))
          || (type == DataType::boolean_native_bit1));
}

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Retrieves if native data of a given CANopen @ref DataType is bit-stuffed or not.
 *
 * This function refers to the native data types specified by the @ref DataType enum.
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
 * \param type
 * Data type.
 *
 * \retval true  Native data __is__ bit-stuffed among adjacent data of given type.
 * \retval false Native data __is not__ bit-stuffed among adjacent data of given type.
 */
bool IsNativeDataStuffed(DataType const type) noexcept
{
  return (   (type == DataType::null)
          || ((type >= DataType::bit1) && (type <= DataType::bit8))
          || (type == DataType::boolean_native_bit1));
}

} // namespace cood
} // namespace gpcc
