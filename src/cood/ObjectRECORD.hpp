/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2019, 2020-2022 Daniel Jerolm

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

#ifndef OBJECTRECORD_HPP_201810131332
#define OBJECTRECORD_HPP_201810131332

#include "Object.hpp"

namespace gpcc {
namespace cood {

class IObjectNotifiable;

/**
 * \ingroup GPCC_COOD
 * \brief RECORD object dictionary object.
 *
 * # Purpose
 * This class implements a object dictionary object of type RECORD. SI0 is constant and read-only.
 *
 * The data accessible via the RECORD object is located outside the @ref ObjectRECORD instance at the creator of the
 * @ref ObjectRECORD. A mutex (also located outside the @ref ObjectRECORD object at the object's creator) may be
 * specified to protect the data. Last but not least a description of the object is also located at the object's
 * creator.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectRECORD_Storage.png"]{width:65%;}</style> \endhtmlonly
 * \image html "cood/ObjectRECORD_Storage.png" "Data represented by RECORD object"
 *
 * # Structure of the native data
 * ## Description
 * The structure of the native data represented by the RECORD object must be described using a list of
 * @ref ObjectRECORD::SubIdxDescr structures. A pointer to the list must be passed to the constructor of class
 * @ref ObjectRECORD. The order of the fields in the native structure does not need to correspond to the order of
 * the subindices in the RECORD object.
 *
 * __Example:__
 * ~~~{.cpp}
 * // Structure encapsulating the native data.
 * // Note:
 * // - It contains only primitive data types
 * // - It contains at least one implicit padding byte
 * struct Data
 * {
 *   bool data_bool;
 *   int8_t data_i8;
 *   uint8_t data_ui8;
 *   uint32_t data_ui32;
 *   uint8_t data_bits;
 *   char data_visiblestring[8];
 *   uint8_t data_octectstring[4];
 * };
 *
 * // Corresponding description using ObjectRECORD::SubIdxDescr:
 * ObjectRECORD::SubIdxDescr const objDescr[9] =
 * {
 *   // name,       type,                     attributes,               nElements, byteOffset,                         bitOffset
 *   { "Data Bool", DataType::boolean,        Object::attr_ACCESS_RW,   1,         offsetof(Data, data_bool),          0},
 *   { "Data i8",   DataType::integer8,       Object::attr_ACCESS_RW,   1,         offsetof(Data, data_i8),            0},
 *   { "Data ui8",  DataType::unsigned8,      Object::attr_ACCESS_RW,   1,         offsetof(Data, data_ui8),           0},
 *   { "Data ui32", DataType::unsigned32,     Object::attr_ACCESS_RW,   1,         offsetof(Data, data_ui32),          0},
 *   { "Bit 0",     DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(Data, data_bits),          0},
 *   { "Bit 6..5",  DataType::bit2,           Object::attr_ACCESS_RW,   1,         offsetof(Data, data_bits),          6},
 *   { "Bit 1",     DataType::bit1,           Object::attr_ACCESS_RW,   1,         offsetof(Data, data_bits),          1},
 *   { "Text",      DataType::visible_string, Object::attr_ACCESS_RW,   8,         offsetof(Data, data_visiblestring), 0},
 *   { "Octet str", DataType::octet_string,   Object::attr_ACCESS_RW,   4,         offsetof(Data, data_octectstring),  0}
 * };
 *
 * ~~~
 *
 * ## Data Types
 * The native data structure shall be composed of primitive, simple, plain-old-data-types only (i.e. uint8_t,
 * uint16_t). It shall not contain any complex data types that have a non-trivial constructor. The reason for this is
 * because during single subindex write and complete access write binary data will be written to the native data
 * structure's elements using low level functions like `memcpy(...)`.
 *
 * ## Remarks on data types regarding flexible length
 * Class @ref ObjectRECORD supports all data types enumerated in the @ref DataType enumeration. However, there
 * are special remarks for the types listed below:
 *
 * Data type                                       | Data type supported | Flexible length supported
 * ----------------------------------------------- | ------------------- | -------------------------
 * [visible_string](@ref DataType::visible_string) | Yes                 | Yes. Native data type and representation as described [here](@ref DataType::visible_string)
 * [octet_string](@ref DataType::octet_string)     | Yes                 | No. Length is fixed.
 * [unicode_string](@ref DataType::unicode_string) | Yes                 | No. Length is fixed.
 *
 * ## Subindex 0 (SI0)
 * SI0 must not be described by the array of @ref SubIdxDescr structures. Its storage and metadata is build-in into
 * class @ref ObjectRECORD. The value of SI0 is setup during construction of the RECORD object and the subindex is
 * read-only.
 *
 * ## Additional padding fields
 * The native data structure is allowed to contain additional bytes for padding and alignment purposes. These fields
 * are usually automatically inserted by the compiler whenever they are necessary. There is no need to pack the native
 * data in order to avoid padding fields.
 *
 * Class @ref ObjectRECORD does not require that padding fields are described in the array of @ref SubIdxDescr
 * structures. However, padding fields can be described as _gap subindices_ (see chapter "Gap Subindices" below) if
 * they shall appear in the CANopen encoded data for complete access. The array of @ref SubIdxDescr structures is even
 * allowed to describe gaps which are not present in the native data.
 *
 * Note that padding fields in the native data structure _may_ be overwritten with zeros during a complete access write
 * operation, regardless if they are described in the array of @ref SubIdxDescr structures or not. In case of a
 * complete access read operation, the content of padding fields is don't care and reads as zero regardless of their
 * real content and regardless if they are described or not.
 *
 * ## Bit-based data types
 * If the CANopen data type of a subindex is any of the types bit1..bit8 or boolean_native_bit1, then the native data
 * containing the bits represented by the subindex must be an `uint8_t`.
 *
 * The @ref SubIdxDescr structures allow to stuff the data of multiple bit-based subindices together into one
 * native element of type `uint8_t` or into an array of `uint8_t`. The data of random subindices may be stuffed
 * together in random order. The subindices may have different types, as long as the types are any of bit1..bit8 and
 * boolean_native_bit1. In case of an array of `uint8_t`, the stuffed data of multiple bit-based subindices may cross
 * byte-boundaries.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectRECORD_BitBasedData.png"]{width:70%;}</style> \endhtmlonly
 * \image html "cood/ObjectRECORD_BitBasedData.png" "Stuffing of bit-based data"
 *
 * Unused bits inside an `uint8_t` that are not referenced by any @ref SubIdxDescr structure are don't care for
 * class @ref ObjectRECORD if a complete access read takes place. During a complete access write, zeros may be written
 * to unused bits inside an `uint8_t`.
 *
 * ## Not existing / empty subindices
 * The RECORD object may contain empty subindices. From the CANopen/EtherCAT point of view these subindices are not
 * existing. However, they count into SI0, but neither their data nor their meta-data is accessible. During a
 * complete access they occupy zero bits of data.
 *
 * Empty subindices must be described by a @ref SubIdxDescr structure whose `type` attribute is @ref DataType::null
 * and all other attributes must be zero or `nullptr`.
 *
 * ## Gap Subindices
 * RECORD objects may contain subindices describing gaps. Gaps are used to align data in a CANopen encoded binary data
 * stream during complete access.
 *
 * For class @ref ObjectRECORD, gaps have no effect on the structure and alignment of native data. However, the access
 * rights specified in the attributes of gaps should be loose and allow for read and write access if there are other
 * subindices with the same permission. Some EtherCAT masters will only perform complete access, if all subindices have
 * similar access rights. The data read from gaps is zero and any data written to gaps will be discarded.
 *
 * When reading or writing a gap using single subindex access, then neither the before-read nor the before- and after-
 * write callbacks will be invoked.
 *
 * Gap subindices are not mandatory. Bit-based and byte-based data may be mixed in a RECORD object without the need to
 * explicitly specify any gaps, because byte-based data will automatically be aligned to the next byte boundary in
 * CANopen encoded data. However, if alignment to a 16bit boundary is required by the EtherCAT master in CANopen
 * encoded data for complete access, then a gap subindex can be used to achieve the required alignment.
 *
 * Please refer to struct @ref SubIdxDescr to see how to describe a gap subindex.
 *
 * Class @ref ObjectRECORD allows to align bit-based data in CANopen encoded binary data with bit granularity by using
 * gaps. However, if byte based data follows a gap, then the gap must have established at least byte alignment. There
 * must also be no neighbouring gap-subindices in a RECORD object. The constructor of class @ref ObjectRECORD will
 * check these rules and throw in case of any violation.
 *
 * # Complete Access
 * ## SI0
 * In the implementation of @ref ObjectRECORD, SI0 is always read-only. However, if a complete access-write contains
 * data for SI0, then the data must match the current value of SI0. Otherwise the write-access will be rejected with
 * @ref SDOAbortCode::UnsupportedAccessToObject.
 *
 * ## Pure read-only subindices and complete access
 * Subindices may be pure read-only. In case of a complete access write, the data that shall be written to the
 * complete object must contain dummy data for the read-only subindices. The size of the dummy data must match the
 * data type of the subindices as if the subindices were read-write.
 *
 * Note:\n
 * Presence of at least one read-only subindex will result in a more complicated algorithm for writing the data to
 * the native structure during a complete access. If all subindices are writeable, then a simple "memcpy" will be
 * used.
 *
 * ## Pure write-only subindices and complete access
 * Subindices may be pure write-only. In case of a complete access read, pure write-only subindices read as zero.
 * The size of the zero data matches the size of the subindex as if it were read-write.
 *
 * ## Empty subindices and complete access
 * The RECORD object may contain empty subindices (see above). Empty subindices will not occupy any data bits/bytes
 * in a complete access read or write. In contrast to read-only subindices, empty subindices will not prevent a
 * simple "memcpy" in case of complete access write.
 *
 * ## Gap subindices and complete access
 * During complete access, the access rights of gap subindices are treatet the same as the access rights of any other
 * subindex.
 *
 * The data read from gaps is zero and any data written to gaps will be discarded.
 *
 * In @ref ObjectRECORD, a pure read-only gap will not prevent a simple `memcpy` in case of a complete access write.
 * However when designing CANopen objects, keep in mind that a pure read-only gap may reduce the performance in
 * third party object dictionary implementations and in master implementations.
 *
 * ## Data types supporting flexible length and complete access
 * In case of complete access, subindices with data types incorporating flexible length will always be read completely
 * using their _maximum size_. If the _actual size_ of the data represented by the subindex is less than the subindex'
 * _maximum size_, then appropriate filling data will be appended. For instance, data read from subindices of type
 * VISIBLE_STRING will be padded with NUL if required.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * Some methods require that the mutex associated with the data represented by the object is locked.\n
 * This can be accomplished via @ref LockData().
 */
class ObjectRECORD : public Object
{
  public:
    /// Structure describing one subindex of a RECORD object.
    /** Most subindices are "normal" subindices representing data, but there may also be empty
     *  (not existing) subindices and subindices describing gaps in complete access binary data.
     *
     *  Depending on the type of subindex (normal/empty/gap), the attributes of an `SubIdxDescr`
     *  object must be initialized with proper values according to the table below:
     *
     *  Field      | Normal Subindex       | Empty Subindex | Gap in Complete Access binary data
     *  ---------- | --------------------- | -------------- | ------------------------------------
     *  name       | Valid ptr to c-string | nullptr        | Valid ptr to c-string (e.g. "Align")
     *  type       | type                  | DataType::null | DataType::null
     *  attributes | attributes            | 0              | At least one of attr_ACCESS_*
     *  nElements  | 1..n                  | 0              | 1..n
     *  byteOffset | byte offset           | 0              | 0
     *  bitOffset  | bit offset            | 0              | 0
     *
     * __n__ depends on the data type. n times the CANopen bit size of the data type must not
     * exceed 0xFFFE (65534).\n
     * (Background: Bit size 0xFFFF has a special meaning in EtherCAT "Get Entry Description Response")
     *
     * All members are public and can be initialized to any value without validation. However, the
     * constructor of class @ref ObjectRECORD will finally validate the list of `SubIdxDescr` objects
     * passed to it.
     */
    struct SubIdxDescr final
    {
      char const * name;      ///<Name/Description of the subindex.
                              /**<This must point to a null-terminated c-string that will not change during
                                  the life-time of the @ref ObjectRECORD object. */

      DataType type;          ///<CANopen data type of the data represented by the subindex.

      attr_t attributes;      ///<Attributes of the subindex.

      uint16_t nElements;     ///<Number of data elements.
                              /**<Usually 1, but for data types Null (gap), VISIBLE_STRING, OCTET_STRING, and
                                  UNICODE_STRING this may be larger than one. */

      uint16_t byteOffset;    ///<Byte offset of the native data inside the structure referenced by
                              ///<parameter `_pStruct` passed to the constructor of class @ref ObjectRECORD.

      uint8_t bitOffset;      ///<Bit offset of the native data inside the byte referenced by 'byteOffset'.
                              /**<For byte-based native data, this must always be zero. */
    };


    ObjectRECORD(void) = delete;
    ObjectRECORD(std::string            const & _name,
                 uint8_t                const   _SI0,
                 void*                  const   _pStruct,
                 size_t                 const   _structsNativeSizeInByte,
                 gpcc::osal::Mutex*     const   _pMutex,
                 SubIdxDescr const *    const   _pSIDescriptions,
                 IObjectNotifiable *    const   _pNotifiable);
    ObjectRECORD(ObjectRECORD const &) = delete;
    ObjectRECORD(ObjectRECORD &&) = delete;
    virtual ~ObjectRECORD(void) = default;

    ObjectRECORD& operator=(ObjectRECORD const &) = delete;
    ObjectRECORD& operator=(ObjectRECORD &&) = delete;

    void SetData(void* const pNewData);

    // <-- base class Object
    ObjectCode GetObjectCode(void) const noexcept override;
    DataType GetObjectDataType(void) const noexcept override;
    std::string GetObjectName(void) const override;

    uint16_t GetMaxNbOfSubindices(void) const noexcept override;
    bool IsSubIndexEmpty(uint8_t const subIdx) const override;
    DataType GetSubIdxDataType(uint8_t const subIdx) const override;
    attr_t GetSubIdxAttributes(uint8_t const subIdx) const override;
    size_t GetSubIdxMaxSize(uint8_t const subIdx) const override;
    std::string GetSubIdxName(uint8_t const subIdx) const override;

    gpcc::osal::MutexLocker LockData(void) const override;

    size_t GetObjectStreamSize(bool const SI016Bits) const noexcept override;

    uint16_t GetNbOfSubIndices(void) const noexcept override;
    size_t GetSubIdxActualSize(uint8_t const subIdx) const override;

    SDOAbortCode Read(uint8_t const subIdx,
                      attr_t const permissions,
                      gpcc::Stream::IStreamWriter & isw) const override;
    SDOAbortCode Write(uint8_t const subIdx,
                       attr_t const permissions,
                       gpcc::Stream::IStreamReader & isr) override;
    SDOAbortCode CompleteRead(bool const inclSI0,
                              bool const SI016Bits,
                              attr_t const permissions,
                              gpcc::Stream::IStreamWriter & isw) const override;
    SDOAbortCode CompleteWrite(bool const inclSI0,
                               bool const SI016Bits,
                               attr_t const permissions,
                               gpcc::Stream::IStreamReader & isr,
                               gpcc::Stream::IStreamReader::RemainingNbOfBits const ernob) override;
    // --> base class Object

  private:
    /// Name of the object.
    std::string const name;

    /// Value of subindex 0.
    /** This value indicates the number of subindices excl. SI0.\n
        This value indicates the number of record elements (incl. empty subindices). */
    uint8_t const SI0;

    /// Pointer to the structure containing the native data represented by the RECORD object.
    /** @ref pMutex is required.\n
        The memory is provided and owned by the creator of the RECORD object. */
    void* pStruct;

    /// Size of the native structure referenced by @ref pStruct in byte.
    size_t const structsNativeSizeInByte;

    /// Pointer to the mutex protecting access to the data represented by the object (@ref pStruct).
    /** This is `nullptr` if no mutex is required to access the data referenced by @ref pStruct. */
    gpcc::osal::Mutex* const pMutex;

    /// Pointer to an array of @ref SubIdxDescr structures describing the subindices of the RECORD object.
    SubIdxDescr const * const pSIDescriptions;


    /// Notifiable interface used to inform the owner of the object about read/write accesses.
    /** This may be `nullptr`. */
    IObjectNotifiable * const pNotifiable;


    /// Size of the complete object (complete access, encoded in CANopen format, excl. SI0) in bit.
    size_t streamSizeInBit;


    void WriteBits(void* const pDestStruct, SubIdxDescr const * const pSubIdxDescr, uint8_t const newBits);
    uint8_t ReadBits(void const * const pSrcStruct, SubIdxDescr const * const pSubIdxDescr) const;
};

} // namespace cood
} // namespace gpcc

#endif // OBJECTRECORD_HPP_201810131332
