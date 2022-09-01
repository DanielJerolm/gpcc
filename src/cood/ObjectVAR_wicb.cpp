/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/ObjectVAR_wicb.hpp>

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
 * The documentation of the enumeration @ref DataType contains a list of native types associated with each
 * CANopen data type.
 *
 * \param _nElements
 * Number of elements of `_type` the data represented by the object is comprised of.\n
 * For most data types this is one.\n
 * For types @ref DataType::visible_string, @ref DataType::octet_string, and @ref DataType::unicode_string
 * this may be any number equal to or larger than one.
 *
 * \param _attributes
 * Attributes for the one and only subindex 0.\n
 * At least one read- or write-permission must be specified.
 *
 * \param _pData
 * Pointer to the native data represented by the object. `nullptr` is not allowed.\n
 * The type of the referenced native data must match the CANopen data type specified by parameter `type`
 * and the number of data elements must match parameter `_nElements`.\n
 * The documentation of the enumeration @ref DataType contains a list of native types associated with each
 * CANopen data type.\n
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
 * \param _onBeforeReadCallback
 * Functor to a callback that will be invoked before reading from the object.\n
 * For details please refer to @ref tOnBeforeReadCallback. \n
 * This is optional. The functor may refer to nothing.
 *
 * \param _onBeforeWriteCallback
 * Functor to a callback that will be invoked before writing to the object.\n
 * For details please refer to @ref tOnBeforeWriteCallback. \n
 * This is optional. The functor may refer to nothing.
 *
 * \param _onAfterWriteCallback
 * Functor to a callback that will be invoked after writing to the object.\n
 * For details please refer to the documentation of @ref tOnAfterWriteCallback. \n
 * This is optional. The functor may refer to nothing.
 */
ObjectVAR_wicb::ObjectVAR_wicb(std::string            const & _name,
                               DataType               const   _type,
                               uint16_t               const   _nElements,
                               attr_t                 const   _attributes,
                               void*                  const   _pData,
                               gpcc::osal::Mutex *    const   _pMutex,
                               tOnBeforeReadCallback  const & _onBeforeReadCallback,
                               tOnBeforeWriteCallback const & _onBeforeWriteCallback,
                               tOnAfterWriteCallback  const & _onAfterWriteCallback)
: IObjectNotifiable()
, ObjectVAR(_name, _type, _nElements, _attributes, _pData, _pMutex, this)
, onBeforeReadCallback(_onBeforeReadCallback)
, onBeforeWriteCallback(_onBeforeWriteCallback)
, onAfterWriteCallback(_onAfterWriteCallback)
{
}

// <-- IObjectNotifiable

/// \copydoc gpcc::cood::IObjectNotifiable::OnBeforeRead
SDOAbortCode ObjectVAR_wicb::OnBeforeRead(gpcc::cood::Object const * pObj,
                                          uint8_t const subindex,
                                          bool const completeAccess,
                                          bool const querySizeWillNotRead)
{
  if (!onBeforeReadCallback)
    return SDOAbortCode::OK;

  return onBeforeReadCallback(pObj, subindex, completeAccess, querySizeWillNotRead);
}

/// \copydoc gpcc::cood::IObjectNotifiable::OnBeforeWrite
SDOAbortCode ObjectVAR_wicb::OnBeforeWrite(gpcc::cood::Object const * pObj,
                                           uint8_t const subindex,
                                           bool const completeAccess,
                                           uint8_t const valueWrittenToSI0,
                                           void const * pData)
{
  if (!onBeforeWriteCallback)
    return SDOAbortCode::OK;

  return onBeforeWriteCallback(pObj, subindex, completeAccess, valueWrittenToSI0, pData);
}

/// \copydoc gpcc::cood::IObjectNotifiable::OnAfterWrite
void ObjectVAR_wicb::OnAfterWrite(gpcc::cood::Object const * pObj,
                                  uint8_t const subindex,
                                  bool const completeAccess)
{
  if (onAfterWriteCallback)
    onAfterWriteCallback(pObj, subindex, completeAccess);
}

// --> IObjectNotifiable

} // namespace cood
} // namespace gpcc
