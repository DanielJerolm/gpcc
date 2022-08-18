/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#ifndef DATA_TYPES_HPP_201809212237
#define DATA_TYPES_HPP_201809212237

#include <string>
#include <cstddef>
#include <cstdint>

namespace gpcc {

namespace Stream {
  class IStreamReader;
  class IStreamWriter;
}

namespace cood {

/**
 * \ingroup GPCC_COOD_DATATYPES
 * \brief Enumeration with CANopen data types.
 *
 * # Native data types
 * Each CANopen data type has an associated native data type which is used for the actual data stored in the
 * application.
 *
 * The native data types referenced in the documentation of the enum values are the defaults which are used by
 * most classes representing CANopen objects to store the application data. There may be classes representing
 * CANopen objects which are specialized to a specific CANopen data type (e.g. VISIBLE_STRING) and which use
 * different native data types for the application data (e.g. `std::string` instead of the default `array of char`).
 * __These classes will contain a prominent hint if they do not use the default native data types specified by this__
 * __enum.__
 *
 * # Alternative native data types
 * In addition to the data types defined by CANopen (_true_ data types), this enum also contains some _additional_ data
 * types invented by GPCC.\n
 * Example:\n
 * For CANopen data type "boolean", there are two enum values:
 * - @ref DataType::boolean
 * - @ref DataType::boolean_native_bit1
 *
 * From the CANopen point of view these data types are equal, since they both represent a CANopen boolean encoded as
 * a single bit in CANopen format. However, they offer alternative native data types which gives us some flexibility
 * in the organization of native data.
 *
 * When creating object dictionary objects (subclasses of class @ref Object), the _additional_ data types can be used
 * just like the _true_ CANopen data types.
 *
 * However, the _additional_ data types invented by GPCC are invisible from the outside of class @ref Object. When
 * accesing an @ref Object via its API (e.g. query the data type of a subindex), all _additional_ data types are always
 * mapped to the _true_ CANopen data type. The mapping is strictly required, because outside GPCC, the _additional_
 * data types are not defined.
 */
enum class DataType
{
  null                        = 0x0000U, ///<Native data type: NONE
                                         /**<This data type is used to describe gap subindices in RECORD objects. */
  boolean                     = 0x0001U, ///<Native data type: bool
                                         /**<A boolean is a single bit in CANopen format, but in native data it is a
                                             C/C++ type bool and will occupy at least one byte. In contrast to types
                                             bit1..bit7, native data will not be stuffed.\n
                                             As an alternative, there is @ref DataType::boolean_native_bit1, which
                                             is also a single bit in CANopen format, but provides a native
                                             representation similar to those of @ref DataType::bit1. */
  integer8                    = 0x0002U, ///<Native data type: int8_t
  integer16                   = 0x0003U, ///<Native data type: int16_t
  integer32                   = 0x0004U, ///<Native data type: int32_t
  unsigned8                   = 0x0005U, ///<Native data type: uint8_t
  unsigned16                  = 0x0006U, ///<Native data type: uint16_t
  unsigned32                  = 0x0007U, ///<Native data type: uint32_t
  real32                      = 0x0008U, ///<Native data type: float
                                         /**<IEC 60559 single precision encoding required. */
  visible_string              = 0x0009U, ///<Native data type: array of char with flexible length
                                         /**<This type shall be used for readable text.\n
                                             The length of the native array is fixed, but if the native array is not
                                             completely filled with characters, then a NUL (0x00) character is used
                                             to indicate the end of the text string.\n
                                             If the native array is completely filled with characters, then there is
                                             no NUL character contained in the native array.\n
                                             When reading a subindex of type visible_string, any bytes behind the NUL
                                             character are don't care for GPCC. When writing to a subindex of type
                                             visible_string, GPCC will fill any bytes behind the NUL character with
                                             0x00.\n
                                             \n
                                             If a trailing NUL character is mandatory in the native data, then the size
                                             of the array can be extended by one extra character which is initialized
                                             with 0x00. Of course the size of the native data configured at the
                                             CAN open object does not comprise the extra character. */
  octet_string                = 0x000AU, ///<Native data type: array of uint8_t with fixed or flexible length
                                         /**<This type is intended to be used for binary data ("byte sausage").\n
                                             The native data may or may not have flexible length depending on the
                                             subclass of class @ref Object being used to represent the native data. */
  unicode_string              = 0x000BU, ///<Native data type: array of uint16_t with fixed or flexible length
                                         /**<This type is intended to be used for binary data ("byte sausage").\n
                                             The native data may or may not have flexible length depending on the
                                             subclass of class @ref Object being used to represent the native data. */
  time_of_day                 = 0x000CU, ///<Native data type: NO SUPPORT YET
  time_difference             = 0x000DU, ///<Native data type: NO SUPPORT YET
  reserved_0x000E             = 0x000EU, ///<Native data type: NONE
  domain                      = 0x000FU, ///<Native data type: NONE
  integer24                   = 0x0010U, ///<Native data type: NO SUPPORT YET
  real64                      = 0x0011U, ///<Native data type: double
                                         /**<IEC 60559 double precision encoding required. */
  integer40                   = 0x0012U, ///<Native data type: NO SUPPORT YET
  integer48                   = 0x0013U, ///<Native data type: NO SUPPORT YET
  integer56                   = 0x0014U, ///<Native data type: NO SUPPORT YET
  integer64                   = 0x0015U, ///<Native data type: int64_t
  unsigned24                  = 0x0016U, ///<Native data type: NO SUPPORT YET
  reserved_0x0017             = 0x0017U, ///<Native data type: NONE
  unsigned40                  = 0x0018U, ///<Native data type: NO SUPPORT YET
  unsigned48                  = 0x0019U, ///<Native data type: NO SUPPORT YET
  unsigned56                  = 0x001AU, ///<Native data type: NO SUPPORT YET
  unsigned64                  = 0x001BU, ///<Native data type: uint64_t
  reserved_0x001C             = 0x001CU, ///<Native data type: NONE
  reserved_0x001D             = 0x001DU, ///<Native data type: NONE
  reserved_0x001E             = 0x001EU, ///<Native data type: NONE
  reserved_0x001F             = 0x001FU, ///<Native data type: NONE
  pdo_communication_parameter = 0x0020U, ///<Native data type: NONE
  pdo_mapping                 = 0x0021U, ///<Native data type: NONE
  sdo_parameter               = 0x0022U, ///<Native data type: NONE
  identity                    = 0x0023U, ///<Native data type: NONE
  reserved_0x0024             = 0x0024U, ///<Native data type: NONE
  commandpar                  = 0x0025U, ///<Native data type: NONE
  reserved_0x0026             = 0x0026U, ///<Native data type: NONE
  reserved_0x0027             = 0x0027U, ///<Native data type: NONE
  reserved_0x0028             = 0x0028U, ///<Native data type: NONE
  syncpar                     = 0x0029U, ///<Native data type: NONE
  reserved_0x002A             = 0x002AU, ///<Native data type: NONE
  reserved_0x002B             = 0x002BU, ///<Native data type: NONE
  reserved_0x002C             = 0x002CU, ///<Native data type: NONE
  reserved_0x002D             = 0x002DU, ///<Native data type: NONE
  reserved_0x002E             = 0x002EU, ///<Native data type: NONE
  reserved_0x002F             = 0x002FU, ///<Native data type: NONE
  bit1                        = 0x0030U, ///<Native data type: uint8_t
                                         /**<This is a single bit in CANopen and also a single bit in native representation,
                                             stored in an uint8_t. In native data, the bits of adjacent subindices of type
                                             bit1..bit8, boolean_native_bit1, and null may be stuffed together, even
                                             across byte boundaries. */
  bit2                        = 0x0031U, ///<Native data type: uint8_t
                                         /**<These are two bits in CANopen and also two bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  bit3                        = 0x0032U, ///<Native data type: uint8_t
                                         /**<These are three bits in CANopen and also three bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  bit4                        = 0x0033U, ///<Native data type: uint8_t
                                         /**<These are four bits in CANopen and also four bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  bit5                        = 0x0034U, ///<Native data type: uint8_t
                                         /**<These are five bits in CANopen and also five bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  bit6                        = 0x0035U, ///<Native data type: uint8_t
                                         /**<These are six bits in CANopen and also six bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  bit7                        = 0x0036U, ///<Native data type: uint8_t
                                         /**<These are seven bits in CANopen and also seven bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  bit8                        = 0x0037U, ///<Native data type: uint8_t
                                         /**<These are eight bits in CANopen and also eight bits in native representation,
                                             stored in one uint8_t or two adjacent uint8_t. In native data, the bits of
                                             adjacent subindices of type bit1..bit8, boolean_native_bit1, and null may
                                             be stuffed together, even across byte boundaries. */
  reserved_0x0038             = 0x0038U, ///<Native data type: NONE
  reserved_0x0039             = 0x0039U, ///<Native data type: NONE
  reserved_0x003A             = 0x003AU, ///<Native data type: NONE
  reserved_0x003B             = 0x003BU, ///<Native data type: NONE
  reserved_0x003C             = 0x003CU, ///<Native data type: NONE
  reserved_0x003D             = 0x003DU, ///<Native data type: NONE
  reserved_0x003E             = 0x003EU, ///<Native data type: NONE
  reserved_0x003F             = 0x003FU, ///<Native data type: NONE

  // Additional data types invented by GPCC to provide alternative native representations of "original" CANopen
  // data types:

  boolean_native_bit1         = 0x0040U  ///<Native data type: uint8_t
                                         /**<This is an alternative to @ref DataType::boolean. \n
                                             In CANopen format, this is a single bit like @ref DataType::boolean, but
                                             in native data _it is not a bool, but a single bit_ stored in an uint8_t
                                             just like @ref DataType::bit1. \n
                                             \n
                                             __This is not a "true" CANopen data type. It has been invented by__
                                             __GPCC and it is undefined in CANopen.__\n
                                             It may be used to describe a CANopen object (subclass of class
                                             @ref Object), but if data types are queried from an @ref Object, then
                                             class @ref Object will always return the true CANopen data type
                                             (@ref DataType::boolean) instead of this. */
};

extern uint8_t const DataTypeBitLengthTable[65];
extern uint8_t const NativeDataTypeBitLengthTable[65];

char const * DataTypeToString(DataType const dt);
uint16_t ToUint16(DataType const dt) noexcept;
DataType ToDataType(uint16_t const value);

std::string CANopenEncodedDataToString(gpcc::Stream::IStreamReader& sr, size_t const sizeInBit, DataType const type);
void StringToCANOpenEncodedData(std::string const & s, size_t const sizeInBit, DataType const type, gpcc::Stream::IStreamWriter& sw);

DataType MapAlternativeDataTypesToOriginalTypes(DataType const dt) noexcept;

bool IsDataTypeBitBased(DataType const type) noexcept;
bool IsNativeDataStuffed(DataType const type) noexcept;

} // namespace cood
} // namespace gpcc

#endif // DATA_TYPES_HPP_201809212237
