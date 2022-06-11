/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018, 2021, 2022 Daniel Jerolm

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

#ifndef OBJECTVAR_HPP_201809291301
#define OBJECTVAR_HPP_201809291301

#include "Object.hpp"

namespace gpcc {

namespace osal {
  class Mutex;
}

namespace cood {

class IObjectNotifiable;

/**
 * \ingroup GPCC_COOD
 * \brief VARIABLE object dictionary object.
 *
 * # Application Data Representation
 * VARIABLE object dictionary objects are intended to represent single items of application data and
 * arrays of application data with fixed or flexible length. In contrast to ARRAY object dictionary
 * objects, individual array elements cannot be accessed. Instead only the complete array can be
 * accessed.
 *
 * The data represented by an @ref ObjectVAR object is located outside the object at the application.\n
 * A mutex (also located outside this object at the application) may be specified to protect the data.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectVAR_Storage.png"]{width:25%;}</style> \endhtmlonly
 * \image html "cood/ObjectVAR_Storage.png" "Data represented by VARIABLE object"
 *
 * The mutex is optional. If the object is read-only __and__ if the application does not modify the data, then
 * a mutex is not required.\n
 * __BUT__ if the object is writeable, or if the application may modify the data, then a mutex is requried.
 *
 * The application must obey the following rules when accessing the data:
 * - If the object is READ-ONLY, then the application must lock the mutex only if it wants to modify the data.
 *   The application does not need to lock the mutex for reading the data in this case.
 * - If the object is READ-WRITE, then the application must lock the mutex ALWAYS when it wants to
 *   read or write the data.
 *
 * # Remarks on data types
 * Class @ref ObjectVAR supports all data types enumerated in the @ref DataType enumeration. However, there
 * are special remarks for the types listed below:
 *
 * Data type                                       | Data type supported | Flexible length supported
 * ----------------------------------------------- | ------------------- | -------------------------
 * [visible_string](@ref DataType::visible_string) | Yes                 | Yes. Native data type and representation as described [here](@ref DataType::visible_string)
 * [octet_string](@ref DataType::octet_string)     | Yes                 | No. Length is fixed.
 * [unicode_string](@ref DataType::unicode_string) | Yes                 | No. Length is fixed.
 *
 * # Object Lifecycle
 * Please refer to chapter "Object Lifecycle" in documentation of base class @ref Object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * Some methods require that the mutex associated with the data represented by the object is locked.\n
 * This can be accomplished via @ref LockData().
 */
class ObjectVAR : public Object
{
  public:
    ObjectVAR(void) = delete;
    ObjectVAR(std::string            const & _name,
              DataType               const   _type,
              uint16_t               const   _nElements,
              attr_t                 const   _attributes,
              void*                  const   _pData,
              gpcc::osal::Mutex *    const   _pMutex,
              IObjectNotifiable *    const   _pNotifiable);
    ObjectVAR(ObjectVAR const &) = delete;
    ObjectVAR(ObjectVAR &&) = delete;
    virtual ~ObjectVAR(void) = default;

    ObjectVAR& operator=(ObjectVAR const &) = delete;
    ObjectVAR& operator=(ObjectVAR &&) = delete;

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

    /// Data type of the data represented by the object.
    DataType const type;

    /// Number of elements of @ref type the data represented by the object is comprised of.
    /** For most data types this is one.\n
        For types @ref DataType::visible_string, @ref DataType::octet_string, and @ref DataType::unicode_string
        this may be any number equal to or larger than one. */
    uint16_t const nElements;

    /// Attributes of the object.
    attr_t const attributes;

    /// Pointer to the data represented by the object.
    /** @ref pMutex is required. */
    void* pData;

    /// Pointer to the mutex protecting access to the data represented by the object.
    /** `nullptr` if no mutex is required to access the data referenced by @ref pData. */
    gpcc::osal::Mutex* const pMutex;

    /// Notifiable interface used to inform the owner of the object about read/write accesses.
    /** This may be `nullptr`. */
    IObjectNotifiable * const pNotifiable;
};

} // namespace cood
} // namespace gpcc

#endif // OBJECTVAR_HPP_201809291301
