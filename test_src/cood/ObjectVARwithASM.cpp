/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "ObjectVARwithASM.hpp"
#include <gpcc/cood/exceptions.hpp>

namespace gpcc_tests {
namespace cood       {

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
 *
 * \param _appSpecMetaData
 * Vector containing application specific meta data.\n
 * The vector's content will be moved into the new object.
 */
ObjectVARwithASM::ObjectVARwithASM(std::string                     const & _name,
                                   gpcc::cood::DataType            const   _type,
                                   uint16_t                        const   _nElements,
                                   gpcc::cood::Object::attr_t      const   _attributes,
                                   void*                           const   _pData,
                                   gpcc::osal::Mutex *             const   _pMutex,
                                   gpcc::cood::IObjectNotifiable * const   _pNotifiable,
                                   std::vector<uint8_t>                 && _appSpecMetaData)
: ObjectVAR(_name, _type, _nElements, _attributes, _pData, _pMutex, _pNotifiable)
, appSpecMetaData(std::move(_appSpecMetaData))
{
}

// <-- gpcc::cood::Object
/// \copydoc gpcc::cood::Object::GetAppSpecificMetaDataSize
size_t ObjectVARwithASM::GetAppSpecificMetaDataSize(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw gpcc::cood::SubindexNotExistingError();

  return appSpecMetaData.size();
}

/// \copydoc gpcc::cood::Object::GetAppSpecificMetaData
std::vector<uint8_t> ObjectVARwithASM::GetAppSpecificMetaData(uint8_t const subIdx) const
{
  if (subIdx != 0U)
    throw gpcc::cood::SubindexNotExistingError();

  if (appSpecMetaData.size() == 0U)
    throw std::logic_error("ObjectVARwithASM::GetAppSpecificMetaData: No ASM");

  return appSpecMetaData;
}
// --> gpcc::cood::Object

} // namespace cood
} // namespace gpcc_tests
