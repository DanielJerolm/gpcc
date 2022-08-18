/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include "ObjectRECORD_wicb.hpp"

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
 * \param _SI0
 * Value for SI0.\n
 * SI0 indicates the number of subindices representing record elements.\n
 * The allowed range is 0-255.
 *
 * \param _pStruct
 * Pointer to the memory location containing the data represented by the RECORD object.\n
 * nullptr is not allowed.\n
 * The memory shall be provided and owned by the creator of the RECORD object.\n
 * The memory location must be valid during the life-time of the RECORD object or until a different
 * memory location is configured via @ref SetData().
 *
 * \param _structsNativeSizeInByte
 * Size of the structure referenced by `_pStruct` in bytes.\n
 * The value shall contain any padding bytes or gap bytes required by the native rules for data representation
 * and memory layout.
 *
 * \param _pMutex
 * Pointer to a mutex protecting access to the native data referenced by `_pStruct`.\n
 * The mutex is optional. If all subindices are read-only __and__ if the application does not modify the
 * data referenced by `_pStruct`, then a mutex is not required and this parameter may be `nullptr`.\n
 * __BUT__ if at least one subindex is writeable, or if the application may modify the native data
 * referenced by `_pStruct`, then a mutex must be specified.
 *
 * \param _pSIDescriptions
 * Pointer to an array of @ref ObjectRECORD::SubIdxDescr structures describing the subindices of the RECORD object.\n
 * The number of entries contained in the array must match the value of '_SI0'.\n
 * The array must be valid and constant during the life-time of the RECORD object.
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
ObjectRECORD_wicb::ObjectRECORD_wicb(std::string            const & _name,
                                     uint8_t                const   _SI0,
                                     void*                  const   _pStruct,
                                     size_t                 const   _structsNativeSizeInByte,
                                     gpcc::osal::Mutex*     const   _pMutex,
                                     SubIdxDescr const *    const   _pSIDescriptions,
                                     tOnBeforeReadCallback  const & _onBeforeReadCallback,
                                     tOnBeforeWriteCallback const & _onBeforeWriteCallback,
                                     tOnAfterWriteCallback  const & _onAfterWriteCallback)
: IObjectNotifiable()
, ObjectRECORD(_name, _SI0, _pStruct, _structsNativeSizeInByte, _pMutex, _pSIDescriptions, this)
, onBeforeReadCallback(_onBeforeReadCallback)
, onBeforeWriteCallback(_onBeforeWriteCallback)
, onAfterWriteCallback(_onAfterWriteCallback)
{
}

// <-- IObjectNotifiable

/// \copydoc gpcc::cood::IObjectNotifiable::OnBeforeRead
SDOAbortCode ObjectRECORD_wicb::OnBeforeRead(gpcc::cood::Object const * pObj,
                                          uint8_t const subindex,
                                          bool const completeAccess,
                                          bool const querySizeWillNotRead)
{
  if (!onBeforeReadCallback)
    return SDOAbortCode::OK;

  return onBeforeReadCallback(pObj, subindex, completeAccess, querySizeWillNotRead);
}

/// \copydoc gpcc::cood::IObjectNotifiable::OnBeforeWrite
SDOAbortCode ObjectRECORD_wicb::OnBeforeWrite(gpcc::cood::Object const * pObj,
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
void ObjectRECORD_wicb::OnAfterWrite(gpcc::cood::Object const * pObj,
                                  uint8_t const subindex,
                                  bool const completeAccess)
{
  if (onAfterWriteCallback)
    onAfterWriteCallback(pObj, subindex, completeAccess);
}

// --> IObjectNotifiable

} // namespace cood
} // namespace gpcc
