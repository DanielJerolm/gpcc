/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2018 Daniel Jerolm
*/

#ifndef OBJECT_HPP_201809212218
#define OBJECT_HPP_201809212218

#include "data_types.hpp"
#include "sdo_abort_codes.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include <functional>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace gpcc {

namespace Stream {
  class IStreamWriter;
}

namespace cood {

class ObjectDictionary;

/**
 * \ingroup GPCC_COOD
 * \brief Abstract base class for all kind of objects contained in a CANopen Object Dictionary (class @ref ObjectDictionary).
 *
 * # Object Lifecycle
 * CANopen objects are created by application software that wants to offer access to selected pieces of
 * application data. The objects are created by instantiating a sub-class of class @ref Object, e.g.
 * @ref ObjectVAR, @ref ObjectARRAY, or @ref ObjectRECORD. After object creation, the application data
 * represented by the object can be read and written through the methods offered by base class @ref Object.
 *
 * The application data is accessible via the object, until the @ref Object instance is destroyed.
 * The blue brace in the figure below shows this time-span.\n
 * __It is strictly required, that the life-line of the application data exceeds the life-line of the__
 * __object as shown in the figure below.__
 *
 * When the object is added to an @ref ObjectDictionary instance, ownership will be moved to the
 * object dictionary. Transfer of ownership is enforced by usage of `unique_ptr`. At the same time, the
 * object (and the data represented by it) becomes accessible to clients of the @ref IObjectAccess interface
 * offered by class @ref ObjectDictionary. The object is accessible, until the application removes it from
 * the object dictionary. The red brace in the figure below shows this time-span.
 *
 * Objects are destroyed by the object dictionary when they are removed from the object dictionary. Ownership
 * is not returned to the application.
 *
 * \htmlonly <style>div.image img[src="cood/ObjectLifeCycle.png"]{width:65%;}</style> \endhtmlonly
 * \image html "cood/ObjectLifeCycle.png" "Object Lifecycle"
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.\n
 * Some methods require that the mutex associated with the data represented by the object is locked.\n
 * This can be accomplished via @ref LockData().
 */
class Object
{
    friend class ObjectDictionary;
    friend class ObjectPtr;

  public:
    /// Object codes. They determine the type of object dictionary object.
    enum class ObjectCode
    {
      Null      = 0x0U,  ///<Object code NULL
      Domain    = 0x2U,  ///<Object code DOMAIN
      DefType   = 0x5U,  ///<Object code DEFTYPE
      DefStruct = 0x6U,  ///<Object code DEFSTRUCT
      Variable  = 0x7U,  ///<Object type VARIABLE
      Array     = 0x8U,  ///<Object type ARRAY
      Record    = 0x9U   ///<Object type RECORD
    };

    /// Length of the largest object code's name that could be returned by @ref ObjectCodeToString().
    /** The null-terminator is not included. */
    static size_t const largestObjectCodeNameLength = 9U;


    /** @name Public constants and typedefs for subindex attributes
     * Each subindex of an object dictionary object has a set of attributes which control the access rights
     * and which may tag subindices with special information (e.g. Rx PDO mappable etc.).
     *
     * Attributes can be used both with CANopen and EtherCAT. The attribute values are defined according to
     * the EtherCAT SDO Info "Get Entry Description Response". Usage with CANopen might require a translation
     * by the code accessing the object dictionary and the objects contained in it.
     */
    ///@{
    /// Typedef for the attributes of a subindex.
    typedef uint16_t attr_t;

    static attr_t const attr_ACCESS_RD_PREOP  = 0x0001U; ///<Attribute: Read-access allowed in PREOP (EtherCAT only)
    static attr_t const attr_ACCESS_RD_SAFEOP = 0x0002U; ///<Attribute: Read-access allowed in SAFEOP (EtherCAT only)
    static attr_t const attr_ACCESS_RD_OP     = 0x0004U; ///<Attribute: Read-access allowed in OP (EtherCAT only)
    static attr_t const attr_ACCESS_WR_PREOP  = 0x0008U; ///<Attribute: Write-access allowed in PREOP (EtherCAT only)
    static attr_t const attr_ACCESS_WR_SAFEOP = 0x0010U; ///<Attribute: Write-access allowed in SAFEOP (EtherCAT only)
    static attr_t const attr_ACCESS_WR_OP     = 0x0020U; ///<Attribute: Write-access allowed in OP (EtherCAT only)
    static attr_t const attr_RXMAP            = 0x0040U; ///<Attribute: Value can be mapped to Rx-PDO (EtherCAT only)
    static attr_t const attr_TXMAP            = 0x0080U; ///<Attribute: Value can be mapped to Tx-PDO (EtherCAT only)
    static attr_t const attr_BACKUP           = 0x0100U; ///<Attribute: Backup-tag (EtherCAT only)
    static attr_t const attr_SETTINGS         = 0x0200U; ///<Attribute: Settings-tag (EtherCAT only)
    static attr_t const attr_ACCESS_RDCONST   = 0x0400U; ///<Attribute: Read-access allowed, const value (CANopen only)
    static attr_t const attr_ACCESS_RD        = 0x0407U; ///<Attribute: Read-access allowed (EtherCAT + CANopen)
    static attr_t const attr_ACCESS_WR        = 0x0038U; ///<Attribute: Write-access allowed (EtherCAT + CANopen)
    static attr_t const attr_ACCESS_RW        = 0x043FU; ///<Attribute: Read-write-access allowed (EtherCAT + CANopen)

    ///@}

    /**
     * \brief Type definition for a before-read-callback.
     *
     * Sub-classes of class @ref Object may allow registration of a before-read-callback function that will be invoked
     * before any data represented by the object dictionary object is read. This allows the object's owner to update
     * the data or to reject the read request.
     *
     * In case of subindices with data types that incorporate flexible length (e.g.
     * [visible_string](@ref gpcc::cood::DataType::visible_string)) __and__ flexible length being supported by the
     * concrete sub-class of class @ref Object, this callback will also be invoked when the current size of a subindex
     * incorporating flexible length is queried via
     * [GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize). This allows the object's owner to update
     * the data, so that [GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize) returns an up-to-date
     * value. This will not be invoked for subindices whose data types do not incorporate flexible length or if the
     * concrete sub-class of class @ref Object does not support flexible length.
     *
     * - - -
     *
     * \param 1st
     * Pointer to the object being read.
     *
     * \param 2nd
     * Subindex being read or in case of a complete access the first subindex being read.\n
     * In case of a complete access, this is either 0 or 1.
     *
     * \param 3rd
     * Access type: Complete access (true) or single access (false).
     *
     * \param 4th
     * Access sub-type: Query subindex size (true) or subindex read (false).\n
     * This is only valid, if 3rd parameter (access type) is false (single access).
     *
     * \return
     * A value from the @ref SDOAbortCode enumeration.\n
     * In case of a read access: If a value other than @ref SDOAbortCode::OK is returned, then the read access will be
     * denied with the returned @ref SDOAbortCode value.\n
     * In case of a subindex size query: If a value other than @ref SDOAbortCode::OK is returned, then
     * [GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize) will throw.
     *
     * - - -
     *
     * __Thread safety requirements/hints:__
     * - The callback function will be executed in the context of the thread that has called
     *   @ref Object::GetSubIdxActualSize(), @ref Object::Read(), or @ref Object::CompleteRead().
     * - The object dictionary is locked for object access when this is executed.
     * - The mutex associated with the data represented by the object is locked when this is executed.
     *
     * __Exception safety requirements/hints:__\n
     * The referenced function/method shall provide at least the strong guarantee.\n
     * The return value shall be used to indicate any error condition that is not a software error.
     *
     * __Thread cancellation safety requirements/hints:__\n
     * The referenced function/method shall provide at least the strong guarantee.
     */
    typedef std::function<SDOAbortCode(Object const *, uint8_t, bool, bool)> tOnBeforeReadCallback;

    /**
     * \brief Type definition for a before-write-callback.
     *
     * Sub-classes of class @ref Object may allow registration of a before-write-callback function that will be invoked
     * before any data represented by the object dictionary object is modified. This allows the object's owner to
     * preview the data before the write access takes place and to accept or reject the write request.
     *
     * Note:\n
     * There may be further checks that take place after the before-write-callback has returned with SDOAbortCode::OK.
     * These checks may fail, so there is no guarantee that the write access will really take place even if the
     * before-write-callback has agreed.\n
     * The owner of the CANopen object shall watch for the after-write-callback (@ref tOnAfterWriteCallback), which
     * indicates that the write access has taken place.
     *
     * - - -
     *
     * \param 1st
     * Pointer to the object being written.
     *
     * \param 2nd
     * Subindex being written or in case of a complete access the first subindex being written.\n
     * In case of a complete access, this is either 0 or 1.
     *
     * \param 3rd
     * Access type: Complete access (true) or single access (false).
     *
     * \param 4th
     * Value written to SI0.\n
     * This is valid, if all of the following conditions are true. For all other cases, this will be zero.
     * - The write is a complete access
     * - SI0 is included in the complete access
     * - The written object is not a VARIABLE object
     *
     * \param 5th
     * Pointer to the data that shall be written for preview purposes.\n
     * The data is encoded in the native format of the subindex being written.\n
     * __In case of a complete access__:
     * - The data can be accessed using the same structure which is also used to store the data represented by the
     *   object inside the application.
     * - SI0 is not included in the data (remember: SI0 is in general not part of the structure used to store the
     *   data inside the application).
     * - This will be `nullptr`, if the value written to SI0 is zero, or if SI0 is not written but SI0 is already
     *   zero.
     *
     * \return
     * A value from the @ref SDOAbortCode enumeration.\n
     * If a value other than @ref SDOAbortCode::OK is returned, then the write access will be denied with the
     * returned @ref SDOAbortCode value.
     *
     * - - -
     *
     * __Thread safety requirements/hints:__
     * - The callback function will be executed in the context of the thread that has called @ref Object::Write() or
     *   @ref Object::CompleteWrite().
     * - The object dictionary is locked for object access when this is executed.
     * - The mutex associated with the data represented by the object is locked when this is executed.
     *
     * __Exception safety requirements/hints:__\n
     * The referenced function/method shall provide at least the strong guarantee.\n
     * The return value shall be used to indicate any error condition that is not a software error.
     *
     * __Thread cancellation safety requirements/hints:__\n
     * The referenced function/method shall provide at least the strong guarantee.
     */
    typedef std::function<SDOAbortCode(Object const *, uint8_t, bool, uint8_t, void const *)> tOnBeforeWriteCallback;

    /**
     * \brief Type definition for a after-write callback.
     *
     * Sub-classes of class @ref Object may allow registration of a after-write callback function that will be invoked
     * after data has been written.\n
     * The callback function will only be invoked, if data has really been written. If e.g. the before-write-callback
     * rejects the write access, then the write will not take place and the after-write callback will not be invoked.
     *
     * \pre   Data has been written to the object.
     *
     * - - -
     *
     * \param 1st
     * Pointer to the object being written.
     *
     * \param 2nd
     * Subindex being written or in case of a complete access the first subindex being written.\n
     * In case of a complete access, this is either 0 or 1.
     *
     * \param 3rd
     * Access type: Complete access (true) or single access (false).
     *
     * - - -
     *
     * __Thread safety requirements/hints:__\n
     * - The callback function will be executed in the context of the thread that has called @ref Object::Write() or
     *   @ref Object::CompleteWrite().
     * - The object dictionary is locked for object access when this is executed.
     * - The mutex associated with the data represented by the object is locked when this is executed.
     *
     * __Exception safety requirements/hints:__\n
     * The referenced function/method shall provide the no-throw guarantee.\n
     * Any thrown exception will result in panic.
     *
     * __Thread cancellation safety requirements/hints:__\n
     * The referenced function/method shall not contain any cancellation point.
     */
    typedef std::function<void(Object const *, uint8_t, bool)> tOnAfterWriteCallback;


    virtual ~Object(void);

    Object& operator=(Object const &) = delete;
    Object& operator=(Object &&) = delete;

    static char const * ObjectCodeToString(ObjectCode const objCode);
    static uint8_t ToUint8(ObjectCode const objCode) noexcept;
    static ObjectCode ToObjectCode(uint8_t const value);
    static std::string AttributeToString(attr_t const attributes, bool const etherCATStyle);

    uint16_t GetIndex(void) const;

    // <-- Access to meta data
    //     (Acquisition of the mutex associated with object's data is not required)
    virtual ObjectCode GetObjectCode(void) const noexcept = 0;
    virtual DataType GetObjectDataType(void) const noexcept = 0;
    virtual std::string GetObjectName(void) const = 0;

    virtual uint16_t GetMaxNbOfSubindices(void) const noexcept = 0;
    virtual bool IsSubIndexEmpty(uint8_t const subIdx) const = 0;
    virtual DataType GetSubIdxDataType(uint8_t const subIdx) const = 0;
    virtual attr_t GetSubIdxAttributes(uint8_t const subIdx) const = 0;
    virtual size_t GetSubIdxMaxSize(uint8_t const subIdx) const = 0;
    virtual std::string GetSubIdxName(uint8_t const subIdx) const = 0;

    virtual size_t GetAppSpecificMetaDataSize(uint8_t const subIdx) const;
    virtual std::vector<uint8_t> GetAppSpecificMetaData(uint8_t const subIdx) const;
    // --> Access to meta data


    // <-- Access to runtime data
    //     (Acquisition of the mutex associated with object's data is required)
    virtual gpcc::osal::MutexLocker LockData(void) const = 0;

    virtual size_t GetObjectStreamSize(bool const SI016Bits) const noexcept = 0;

    virtual uint16_t GetNbOfSubIndices(void) const noexcept = 0;
    virtual size_t GetSubIdxActualSize(uint8_t const subIdx) const = 0;

    virtual SDOAbortCode Read(uint8_t const subIdx,
                              attr_t const permissions,
                              gpcc::Stream::IStreamWriter & isw) const = 0;
    virtual SDOAbortCode Write(uint8_t const subIdx,
                               attr_t const permissions,
                               gpcc::Stream::IStreamReader & isr) = 0;
    virtual SDOAbortCode CompleteRead(bool const inclSI0,
                                      bool const SI016Bits,
                                      attr_t const permissions,
                                      gpcc::Stream::IStreamWriter & isw) const = 0;
    virtual SDOAbortCode CompleteWrite(bool const inclSI0,
                                       bool const SI016Bits,
                                       attr_t const permissions,
                                       gpcc::Stream::IStreamReader & isr,
                                       gpcc::Stream::IStreamReader::RemainingNbOfBits const ernob) = 0;
    // --> Access to runtime data

   protected:
     Object(void) noexcept;
     Object(Object const &) noexcept = default;
     Object(Object &&) noexcept = default;

     static size_t DetermineSizeOfCANopenEncodedData(void const * const pNativeData,
                                                     DataType const type,
                                                     uint16_t const nDataElements);
     static void NativeDataToCANopenEncodedData(void const * const pNativeData,
                                                DataType const type,
                                                uint16_t const nDataElements,
                                                bool const completeAccess,
                                                gpcc::Stream::IStreamWriter & out);
     static void CANopenEncodedDataToNativeData(gpcc::Stream::IStreamReader & in,
                                                DataType const type,
                                                uint16_t const nDataElements,
                                                bool const completeAccess,
                                                void* const pNativeData);

   private:
     /// Pointer to the @ref ObjectDictionary instance containing this object. `nullptr` = none.
     /** This is setup and cleared by a @ref ObjectDictionary instance upon object registration and removal. */
     ObjectDictionary* pOD;

     /// Object's index.
     /** This is setup by a @ref ObjectDictionary instance upon object registration.\n
         This is only valid, if @ref pOD is not nullptr. */
     uint16_t index;
};

/**
 * \fn Object::GetObjectCode
 * \brief Retrieves the object code of the object.
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
 * \return
 * Object code of the object.
 */

/**
 * \fn Object::GetObjectDataType
 * \brief Retrieves the data type of the object.
 *
 * This will always return a _true_ CANopen data type.\n
 * _Additional_ CANopen data types invented by GPCC to offer alternative native representations of _true_ CANopen
 * data types will always be mapped to the original _true_ data type.
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
 * \return
 * Data type of the object.
 */

/**
 * \fn Object::GetObjectName
 * \brief Retrieves the name/description of the object.
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
 * \return
 * Name/description of the object.
 */

/**
 * \fn Object::GetMaxNbOfSubindices
 * \brief Retrieves the maximum number of subindices (incl. subindex 0).
 *
 * Objects of type ARRAY and RECORD may have a non-const subindex 0. This means that the value of
 * subindex 0 and therefore the number of subindices may change dynamically after the object has been
 * created, e.g. if the object allows write-access to subindex 0, or if the application changes the
 * value of subindex 0 during run-time.
 *
 * This method retrieves the maximum number of subindices that may occur during runtime. The maximum value
 * is set during object creation and cannot be changed during runtime.
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
 * \return
 * Maximum number of subindices, inclusive subindex 0.\n
 * For a VARIABLE object, this is always 1.\n
 * For all other types of objects, this is equal to or greater than 1.
 */

/**
 * \fn Object::IsSubIndexEmpty
 * \brief Retrieves if a subindex is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose empty-status shall be retrieved.
 *
 * \return
 * true = subindex is empty\n
 * false = subindex is not empty
 */

/**
 * \fn Object::GetSubIdxDataType
 * \brief Retrieves the CANopen data type of a subindex.
 *
 * This will always return a _true_ CANopen data type.\n
 * _Additional_ CANopen data types invented by GPCC to offer alternative native representations of _true_ CANopen
 * data types will always be mapped to the original _true_ data type.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose data type shall be retrieved.
 *
 * \return
 * CANopen data type of the subindex.
 */

/**
 * \fn Object::GetSubIdxAttributes
 * \brief Retrieves the attributes of a subindex.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose attributes shall be retrieved.
 *
 * \return
 * Attributes of the subindex.
 */

/**
 * \fn Object::GetSubIdxMaxSize
 * \brief Retrieves the maximum size of a subindex in bit.
 *
 * The _maximum size_ of a subindex is the maximum possible value for the _actual size_ of a subindex.
 *
 * The _actual size_ of a subindex is the number of bits occupied by a subindex, if it would be stored in
 * CANopen format into an [IStreamWriter](@ref gpcc::Stream::IStreamWriter) via
 * [Object::Read()](@ref gpcc::cood::Object::Read).
 *
 * For data types with flexible length (e.g. VISIBLE_STRING), the _actual size_ may change during runtime.\n
 * The _maximum size_ of a subindex is always constant.
 *
 * For subindices whose data type does not have flexible length, the _actual size_ is always equal to the
 * _maximum size_.
 *
 * In case of a complete access, a subindex will always occupy the _maximum size_, even if its _actual size_
 * is smaller. Note that SI0 may occupy 16 bits during complete access for ARRAY and RECORD objects.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose maximum size shall be retrieved.
 *
 * \return
 * Maximum size of the subindex in bit.\n
 * The value is based on the size of the CANopen data type of the subindex. The native size may differ.
 */

/**
 * \fn Object::GetSubIdxName
 * \brief Retrieves the name/description of a subindex.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose name/description shall be retrieved.
 *
 * \return
 * Name/description of the subindex.
 */


/**
 * \fn Object::GetAppSpecificMetaDataSize
 * \brief Retrieves the size of the application-specific meta data of a specific subindex.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose application-specific meta data's size shall be queried.
 *
 * \return
 * Size of the application-specific meta data of the subindex in byte.\n
 * Zero if there is no application-specific meta data attached to the given subindex, or if the object does not support
 * application-specific meta data at all.
 */


/**
 * \fn std::vector<uint8_t> Object::GetAppSpecificMetaData
 * \brief Retrieves the application-specific meta data of a specific subindex.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc             Out of memory.
 *
 * \throws std::logic_error           There is no application-specific meta data attached to the given subindex or the
 *                                    object does not support application-specific meta data at all.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose application-specific meta data shall be retrieved.
 *
 * \return
 * Application-specific meta data of the given subindex.
 */


/**
 * \fn Object::LockData
 * \brief Locks the mutex used to protect the application data represented by the CANopen object.
 *
 * If there is no mutex, then the returned [MutexLocker](@ref gpcc::osal::MutexLocker) will behave passive.
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
 * [MutexLocker](@ref gpcc::osal::MutexLocker) used to lock the mutex.
 */

/**
 * \fn Object::GetObjectStreamSize
 * \brief Retrieves the number of bits occupied by the whole object if it would be stored in
 *        CANopen format into a [IStreamWriter](@ref gpcc::Stream::IStreamWriter) using "complete access".
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param SI016Bits
 * true  = SI0 shall be treated as UNSIGNED16 (-> EtherCAT)\n
 * false = SI0 shall be treated as UNSIGNED8\n
 * This parameter is don't care for VARIABLE objects.
 *
 * \return
 * Number of bits occupied by the complete object if it would be stored in CANopen format into a
 * [IStreamWriter](@ref gpcc::Stream::IStreamWriter) using "complete access".\n
 * For ARRAY and RECORD objects, SI0 is always included and the number of subindices is given by
 * the current value of SI0 (and not by the maximum value of SI0).\n
 * Bits for alignment:
 * - The value includes any bits required for alignment of byte-based data _inside_ the stream.
 * - The stream's write pointer is assumed to be initially located on a byte-boundary. This method does
 *   not consider any potential leading alignment bits.
 * - After writing the object's data to the stream, the write pointer may not be located on a byte boundary.
 *   This method does not consider any potential trailing alignment bits required to fill the last byte.
 */

/**
 * \fn Object::GetNbOfSubIndices
 * \brief Retrieves the number of subindices (incl. subindex 0).
 *
 * Note that the number of subindices of ARRAY and RECORD objects may change dynamically, e.g. if the object allows
 * write-access to subindex 0, or if the application changes the value of subindex 0 during run-time.
 *
 * This method returns the actual number of subindices of the object.\n
 * [GetMaxNbOfSubindices()](@ref gpcc::cood::Object::GetMaxNbOfSubindices) can be used to determine
 * the maximum possible/allowed number of subindices.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
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
 * Number of subindices, inclusive subindex 0.\n
 * For a VARIABLE object, this is always 1.\n
 * For all other types of objects, this is the current value of SI0 plus 1.
 */

/**
 * \fn Object::GetSubIdxActualSize
 * \brief Retrieves the actual size of a subindex in bit.
 *
 * The _actual size_ of a subindex is the number of bits occupied by a subindex, if it would be stored in
 * CANopen format into an [IStreamWriter](@ref gpcc::Stream::IStreamWriter) via
 * [Object::Read()](@ref gpcc::cood::Object::Read). The _actual size_ may vary for data types with flexible
 * length (e.g. VISIBLE_STRING). If the subindex' data type offers flexible length and if the object supports it,
 * then the before-read-callback will be invoked to update the subindex' data if necessary and thus get an up-to-date
 * size value.
 *
 * The _actual size_ is always equal to or less than the _maximum size_ of the subindex.\n
 * The _maximum size_ can be retrieved via [Object::GetSubIdxMaxSize()](@ref gpcc::cood::Object::GetSubIdxMaxSize).\n
 * The _maximum size_ of a subindex is always constant.
 *
 * For subindices whose data type does not have flexible length, the _actual size_ is always equal to the
 * _maximum size_.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws SubindexNotExistingError   Subindex is not existing or empty
 *                                    ([details](@ref gpcc::cood::SubindexNotExistingError)).
 *
 * \throws std::runtime_error         The before-read-callback returned an error status. Note that
 *                                    [SDOAbortCode::OutOfMemory](@ref gpcc::cood::SDOAbortCode::OutOfMemory) will
 *                                    result in `std::bad_alloc` being thrown.
 *
 * \throws std::bad_alloc             Out of memory (may only occurr in before-read-callback).
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param subIdx
 * Subindex whose actual size shall be retrieved.
 *
 * \return
 * Actual size of the subindex in bit.\n
 * The value is based on the size of the CANopen data type of the subindex. The native size may differ.
 */

/**
 * \fn Object::Read
 * \brief Reads the native data of one subindex into an [IStreamWriter](@ref gpcc::Stream::IStreamWriter) using
 *        CANopen encoding.
 *
 * The table below enumerates common error conditions and how they are treated by this method:
 * Error condition                         | Reported via   | 'isw' modified
 * --------------------------------------- | -------------- | ---------------
 * Subindex is not existing                | SDO Abort Code | no
 * Subindex is empty (RECORD objects only) | SDO Abort Code | no
 * Insufficient permissions                | SDO Abort Code | no
 * Before-read-callback does not agree     | SDO Abort Code | no
 * Before-read-callback throws             | Exception      | no
 * Error writing to 'isw' (e.g. full)      | Exception      | possible
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the before-read-callback may have been invoked
 * - incomplete/undefined data may have been written to `isw`.
 *
 * \throws FullError   `isw` is full ([details](@ref gpcc::Stream::FullError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the before-read-callback may have been invoked
 * - incomplete/undefined data may have been written to `isw`.
 *
 * - - -
 *
 * \param subIdx
 * Subindex that shall be read.
 *
 * \param permissions
 * Access permissions.
 *
 * \param isw
 * [IStreamWriter](@ref gpcc::Stream::IStreamWriter) into which the data read from the subindex shall be written.\n
 * Byte-based data will be written into the stream using byte-alignment.\n
 * Bit-based data will be stuffed together.\n
 * To achieve CANopen encoding, the stream writer must be configured to use little-endian.\n
 * [Object::GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize) can be used to preview the
 * number of bits that will be written to @p isw.
 *
 * \return
 * Result of the operation.\n
 * If the return value indicates an error condition, then the method guarantees that `isw` has not been modified.
 */

/**
 * \fn Object::Write
 * \brief Writes CANopen encoded data read from an [IStreamReader](@ref gpcc::Stream::IStreamReader) into one subindex using
 *        native encoding.
 *
 * The table below enumerates common error conditions and how they are treated by this method:
 * Error condition                                               | Reported via   | 'isr' modified | Object's data modified
 * ------------------------------------------------------------- | -------------- | -------------- | ----------------------
 * Subindex is not existing                                      | SDO Abort Code | no             | no
 * Subindex is empty (RECORD objects only)                       | SDO Abort Code | no             | no
 * Insufficient permissions                                      | SDO Abort Code | no             | no
 * Error reading from 'isr' (empty-error)                        | SDO Abort Code | possible       | no
 * Error reading from 'isr' (other errors)                       | Exception      | possible       | no
 * 'isr' not empty after reading (more than 7 bits remaining)    | SDO Abort Code | yes            | no
 * Attempt to write invalid data to SI0 (ARRAY and RECORD only)  | SDO Abort Code | yes            | no
 * Before-write-callback does not agree                          | SDO Abort Code | yes            | no
 * Before-write-callback throws                                  | Exception      | yes            | no
 * After-write-callback throws                                   | Panic          | yes            | yes
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - data may have been read from `isr`. The read-pointer of `isr` will not be recovered in case of an exception.
 * - the before-write-callback may have been invoked.
 * - the data represented by the subindex is guaranteed to be not modified in case of any exception.
 *
 * \throws EmptyError           Not enough data in `isr` ([details](@ref gpcc::Stream::EmptyError)).
 *
 * \throws std::bad_alloc       Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - data may have been read from `isr`. The read-pointer of `isr` will not be recovered in case of deferred thread cancellation.
 * - the before-write-callback may have been invoked.
 * - the data represented by the subindex is guaranteed to be not modified in case of deferred thread cancellation.
 *
 * - - -
 *
 * \param subIdx
 * Subindex that shall be written.
 *
 * \param permissions
 * Access permissions.
 *
 * \param isr
 * [IStreamReader](@ref gpcc::Stream::IStreamReader) from which the data shall be read.\n
 * The stream must contain the data for the subindex only. There must be no additional data in the stream.\n
 * Byte-based data will be read from the stream using byte-alignment.\n
 * Bit-based data is assumed to be stuffed together.\n
 * To achieve CANopen encoding, the stream reader must be configured to use little-endian.
 *
 * \return
 * Result of the operation.\n
 * If the return value indicates an error condition, then the method guarantees that the data represented by the
 * subindex has not been modified. However, data may have been read from `isr`. The read-pointer of `isr` will not be recovered
 * in case of an error.
 */

/**
 * \fn Object::CompleteRead
 * \brief Reads the complete object (native format) and stores the read data into an
 *        [IStreamWriter](@ref gpcc::Stream::IStreamWriter) using CANopen encoding.
 *
 * This method is used to implement an EtherCAT SDO Complete Access.
 *
 * Complete access (read) is only supported by objects of type DEFSTRUCT, ARRAY, and RECORD.
 *
 * The table below enumerates common error conditions and how they are treated by this method:
 * Error condition                                       | Reported via   | 'isw' modified
 * ----------------------------------------------------- | -------------- | ---------------
 * Complete Access not supported (e.g. VARIABLE objects) | SDO Abort Code | no
 * Insufficient permissions                              | SDO Abort Code | no
 * Before-read-callback does not agree                   | SDO Abort Code | no
 * Before-read-callback throws                           | Exception      | no
 * Error writing to 'isw' (e.g. full)                    | Exception      | possible
 *
 * Note:\n
 * - Any subindices that are pure write-only read as zero and occupy the size of the subindex inside 'isw' as if they
 *   were RO or RW. For all other subindices that have at least one readable-attribute set, the permissions must fit
 *   or an appropriate error code will be returned.
 * - Empty subindices (RECORD objects only) will not be read and they will occupy no bits in `isw`.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - the before-read-callback may have been invoked
 * - incomplete/undefined data may have been written to `isw`.
 *
 * \throws FullError   `isw` is full ([details](@ref gpcc::Stream::FullError)).
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - the before-read-callback may have been invoked
 * - incomplete/undefined data may have been written to `isw`.
 *
 * - - -
 *
 * \param inclSI0
 * true  = SI0 shall be included\n
 * false = SI0 shall not be included
 *
 * \param SI016Bits
 * This is don't care if 'inclSI0' is false.\n
 * true  = SI0 shall be treated as UNSIGNED16 in CANopen encoded data (-> EtherCAT)\n
 * false = SI0 shall be treated as UNSIGNED8 in CANopen encoded data
 *
 * \param permissions
 * Access permissions.
 *
 * \param isw
 * [IStreamWriter](@ref gpcc::Stream::IStreamWriter) into which the data shall be written.\n
 * Byte-based data will be written into the stream using byte-alignment.\n
 * Bit-based data will be stuffed together.\n
 * To achieve CANopen encoding, the stream writer must be configured to use little-endian.\n
 * [Object::GetObjectStreamSize()](@ref gpcc::cood::Object::GetObjectStreamSize) can be used to preview the
 * number of bits that will be written to @p isw.
 *
 * \return
 * Result of the operation.\n
 * If the return value indicates an error condition, then the method guarantees that `isw` has not been modified.
 */

/**
 * \fn Object::CompleteWrite
 * \brief Writes CANopen encoded data read from an [IStreamReader](@ref gpcc::Stream::IStreamReader) to the complete
 *        object in native format.
 *
 * This method is used to implement an EtherCAT SDO Complete Access.
 *
 * Complete access (write) is only supported by objects of type ARRAY and RECORD.
 *
 * The table below enumerates common error conditions and how they are treated by this method:
 * Error condition                                                  | Reported via   | 'isr' modified | Object's data modified
 * ---------------------------------------------------------------- | -------------- | -------------- | ----------------------
 * Complete Access not supported (e.g. VARIABLE objects)            | SDO Abort Code | no             | no
 * Insufficient permissions                                         | SDO Abort Code | no             | no
 * Error reading from 'isr' (empty-error)                           | SDO Abort Code | possible       | no
 * Error reading from 'isr' (other errors)                          | Exception      | possible       | no
 * Expected number of bits remaining in 'isr' not met (see 'ernob') | SDO Abort Code | yes            | no
 * Attempt to write invalid data to SI0 (ARRAY and RECORD only)     | SDO Abort Code | yes            | no
 * Before-write-callback does not agree                             | SDO Abort Code | yes            | no
 * Before-write-callback throws                                     | Exception      | yes            | no
 * After-write-callback throws                                      | Panic          | yes            | yes
 *
 * Note:\n
 * - Any subindices that are pure read-only will be ignored by the write operation, but 'isr' must provide some dummy
 *   data for them. The size of the dummy data must match the size of the pure read-only subindices as if they were WO
 *   or RW. For all other subindices that have at least one write-attribute set, the permissions must be suitable or an
 *   appropriate error code will be returned.
 * - Empty subindices (RECORD objects only) will not be written and `isr` must not contain any bits for empty subindices.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The mutex associated with the data represented by the object must be locked.\n
 * This can be accomplished via [LockData()](@ref gpcc::cood::Object::LockData).
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - data may have been read from `isr`. The read-pointer of `isr` will not be recovered in case of an exception.
 * - the before-write-callback may have been invoked.
 * - the data represented by the object is guaranteed to be not modified in case of any exception.
 *
 * \throws EmptyError           Not enough data in `isr` ([details](@ref gpcc::Stream::EmptyError)).
 *
 * \throws std::bad_alloc       Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - data may have been read from `isr`. The read-pointer of `isr` will not be recovered in case of deferred thread cancellation.
 * - the before-write-callback may have been invoked.
 * - the data represented by the object is guaranteed to be not modified in case of deferred thread cancellation.
 *
 * - - -
 *
 * \param inclSI0
 * true  = SI0 shall be included\n
 * false = SI0 shall not be included
 *
 * \param SI016Bits
 * This is don't care if 'inclSI0' is false.\n
 * true  = SI0 shall be treated as UNSIGNED16 in CANopen encoded data (-> EtherCAT)\n
 * false = SI0 shall be treated as UNSIGNED8 in CANopen encoded data
 *
 * \param permissions
 * Access permissions.
 *
 * \param isr
 * [IStreamReader](@ref gpcc::Stream::IStreamReader) from which the data shall be read.\n
 * Byte-based data will be read from the stream using byte-alignment.\n
 * Bit-based data is assumed to be stuffed together.\n
 * To achieve CANopen encoding, the stream reader must be configured to use little-endian.
 *
 * \param ernob
 * Expected number of bits remaining in `isr` after reading the data from the stream for preview purposes. The
 * [expectation](@ref gpcc::Stream::IStreamReader::RemainingNbOfBits) will be checked before the before-write-callback
 * is invoked. Common values:
 * - gpcc::Stream::IStreamReader::RemainingNbOfBits::any
 * - gpcc::Stream::IStreamReader::RemainingNbOfBits::sevenOrLess
 * - gpcc::Stream::IStreamReader::RemainingNbOfBits::moreThanSeven
 *
 * \return
 * Result of the operation.\n
 * If the return value indicates an error condition, then the method guarantees that the data represented by the
 * object has not been modified. However, data may have been read from `isr`. The read-pointer of `isr` will not be recovered
 * in case of an error.
 */

} // namespace cood
} // namespace gpcc

#endif // OBJECT_HPP_201809212218
