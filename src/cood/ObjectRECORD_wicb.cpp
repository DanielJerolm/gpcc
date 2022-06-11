/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2021, 2022 Daniel Jerolm

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
