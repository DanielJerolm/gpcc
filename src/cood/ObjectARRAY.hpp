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

#ifndef OBJECTARRAY_HPP_201810092146
#define OBJECTARRAY_HPP_201810092146

#include "Object.hpp"

namespace gpcc {
namespace cood {

class IObjectNotifiable;

/**
 * \ingroup GPCC_COOD
 * \brief ARRAY object dictionary object.
 *
 * # Application Data Representation
 * ARRAY object dictionary objects are intended to represent array-type application data. ARRAY objects
 * offer access to the size of the array via subindex 0 (SI0) and to the array's data via subindex 1..255.
 * Each of the subindices 1..255 corresponds to one array element of application data.
 *
 * The array data represented by an @ref ObjectARRAY object is located outside the object at the application.\n
 * The array size (SI0) is located inside the @ref ObjectARRAY object.\n
 * A mutex (located outside the object at the application) may be specified to protect both the data and the
 * value of SI0.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectARRAY_Storage.png"]{width:80%;}</style> \endhtmlonly
 * \image html "cood/ObjectARRAY_Storage.png" "Data represented by ARRAY object"
 *
 * Class @ref ObjectARRAY supports write-access to SI0 which will change the size of the array. Upper and lower
 * bounds for SI0 can be specified during object creation and will be enforced during any write-access to SI0.
 * The size of the storage reserved by the application for the array's data must meet the upper bound for SI0
 * as shown in the figure above.
 *
 * Attributes (including access rights) can be specified separately for SI0 and for the other subindices
 * representing the array data.
 *
 * The mutex is optional. If the array data and the array size are read-only __and__ if the application
 * does not modify neither the data nor the array size, then a mutex is not required.\n
 * __BUT__ if the array data or the array size are writeable, or if the application may modify the data
 * or the array size, then a mutex is requried.
 *
 * The application must obey the following rules when accessing the array data:
 * - If the array data and SI0 are READ-ONLY, then the application must lock the mutex only if it wants
 *   to modify the data. The application does not need to lock the mutex for reading the data in this case.
 * - If the object's data and/or SI0 are READ-WRITE, then the application must lock the mutex ALWAYS when
 *   it wants to read or write the data.
 *
 * The array size (SI0) is stored inside the @ref ObjectARRAY object. The application can only modify the array
 * size via @ref SetData(), which will always lock the mutex. The application can query the current number of
 * subindices via @ref GetNbOfSubIndices() and thus calculate the current value of SI0. The application can also use
 * the before-write-callback to receive an information in case of a write-access to SI0.
 *
 * # Remarks on data types
 * Class @ref ObjectARRAY supports all data types enumerated in the @ref DataType enumeration, except for the
 * types listed below:
 *
 * Data type                                       | Data type supported | Flexible length supported
 * ----------------------------------------------- | ------------------- | -------------------------
 * [visible_string](@ref DataType::visible_string) | No                  | N.a.
 * [octet_string](@ref DataType::octet_string)     | No                  | N.a.
 * [unicode_string](@ref DataType::unicode_string) | No                  | N.a.
 *
 * # Object Lifecycle
 * Please refer to chapter "Object Lifecycle" in documentation of base class @ref Object.
 *
 * # Native data layout
 * ## Byte based data types
 * This applies to all data types __except__ for bit1..bit8 and boolean_native_bit1.
 *
 * The native data must be organized as an array of elements, whose native data type matches the CANopen data
 * type. The documentation of the @ref DataType enum specifies the compatible native data type for each CANopen
 * data type.
 *
 * The layout of the array must match the native requirements for padding and alignment. These requirements are
 * automatically fulfilled if the array is instantiated using standard C/C++ mechanisms:
 * ~~~{.cpp}
 * // array data allocated on stack or global variable
 * uint32_t array1[12];
 *
 * // array data allocated on the heap
 * uint32_t* pArray2 = new uint32_t[12];
 * ~~~
 *
 * ## Bit based data types
 * Data types bit1..bit8 and boolean_native_bit1 are stored inside an array of `uint8_t`. See @ref DataType::bit1 for
 * details. The elements are stuffed together and may even cross byte boundaries.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectARRAY_StuffingNativeBitBasedData_BIT2.png"]{width:60%;}</style> \endhtmlonly
 * \image html "cood/ObjectARRAY_StuffingNativeBitBasedData_BIT2.png" "Data type bit2"
 *
 * \htmlonly <style>div.image img[src="cood/ObjectARRAY_StuffingNativeBitBasedData_BIT6.png"]{width:60%;}</style> \endhtmlonly
 * \image html "cood/ObjectARRAY_StuffingNativeBitBasedData_BIT6.png" "Data type bit6"
 *
 * # Special features
 * This class supports modification of SI0 and writing array data during one complete access.\n
 * This exceeds the EtherCAT specification, which requires that SI0 shall not be modified by a
 * master during complete access to an object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * Some methods require that the mutex associated with the data represented by the object is locked.\n
 * This can be accomplished via @ref LockData().
 */
class ObjectARRAY : public Object
{
  public:
    ObjectARRAY(void) = delete;
    ObjectARRAY(std::string            const & _name,
                attr_t                 const   _attributesSI0,
                uint8_t                const   _SI0,
                uint8_t                const   _min_SI0,
                uint8_t                const   _max_SI0,
                DataType               const   _type,
                attr_t                 const   _attributes,
                void*                  const   _pData,
                gpcc::osal::Mutex *    const   _pMutex,
                IObjectNotifiable *    const   _pNotifiable);
    ObjectARRAY(ObjectARRAY const &) = delete;
    ObjectARRAY(ObjectARRAY &&) = delete;
    virtual ~ObjectARRAY(void) = default;

    ObjectARRAY& operator=(ObjectARRAY const &) = delete;
    ObjectARRAY& operator=(ObjectARRAY &&) = delete;

    void SetData(uint8_t const newSI0, void* const pNewData);

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


    /// Attributes of subindex 0.
    attr_t const attributesSI0;

    /// Value of subindex 0.
    /** @ref pMutex is required.\n
        This value indicates the number of subindices excl. SI0.\n
        This value indicates the number of array elements. */
    uint8_t SI0;

    /// Minimum value for @ref SI0.
    uint8_t const min_SI0;

    /// Maximum value for @ref SI0.
    uint8_t const max_SI0;


    /// Data type of the array elements.
    DataType const type;

    /// Attributes of the subindices representing array elements.
    attr_t const attributes;

    /// Pointer to the memory location containing the data represented by the ARRAY object.
    /** @ref pMutex is required.\n
        The memory is provided and owned by the owner of the ARRAY object. */
    void* pData;

    /// Pointer to the mutex protecting access to the data represented by the object.
    /** This is `nullptr` if no mutex is required to access SI0 and to the data referenced by @ref pData. */
    gpcc::osal::Mutex* const pMutex;


    /// Notifiable interface used to inform the owner of the object about read/write accesses.
    /** This may be `nullptr`. */
    IObjectNotifiable * const pNotifiable;


    uint8_t ReadBitsFromMem(uint8_t const subIdx) const;
    void WriteBitsToMem(uint8_t const subIdx, uint8_t const data);
};

} // namespace cood
} // namespace gpcc

#endif // OBJECTARRAY_HPP_201810092146
