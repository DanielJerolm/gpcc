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

#include "ObjectARRAY_wicb.hpp"

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
 * \param _attributesSI0
 * Attributes for subindex 0.\n
 * There must be at least one read-permission set.
 *
 * \param _SI0
 * Initial value for SI0.\n
 * SI0 indicates the number of subindices representing array elements and therefore the number of array elements.\n
 * The allowed range is 0-255.\n
 * This must also be within the range given by `_min_SI0` and `_max_SI0`.
 *
 * \param _min_SI0
 * Minimum value for SI0. Allowed range: 0-255.\n
 * This must be equal to or less than `_max_SI0`.
 *
 * \param _max_SI0
 * Maximum value for SI0. Allowed range: 0-255.\n
 * This must be equal to or larger than `_min_SI0`.
 *
 * \param _type
 * CANopen data type of the array elements represented by the object. Since the constructed object is an ARRAY
 * object, the object itself will have the same data type as the data represented by it.\n
 * \n
 * The type of the native data referenced by `_pData` must match the CANopen data type. The documentation of the
 * @ref DataType enumeration contains a list of native types associated with each CANopen data type.\n
 * \n
 * Note that if '_type' is any of the types bit1..bit8 or boolean_native_bit1, then the native data must be an array of
 * data type uint8_t and the bits will be stuffed together inside the native array.\n
 * Example:\n
 * An array of 18 elements of type `bit2` would be comprised of 36 bits. The native array would occupy 5 native
 * elements of type uint8_t and the upper four bits of the last element would be unused.
 *
 * \param _attributes
 * Attributes for the subindices representing array elements.\n
 * The attributes are applied to all subindices except for SI0.\n
 * At least one read- or write-permission must be set.
 *
 * \param _pData
 * Pointer to the memory location containing the data represented by the ARRAY object.\n
 * The memory is provided and owned by the owner of the ARRAY object.\n
 * The memory location must be valid during the life-time of the ARRAY object or until a different
 * memory location is configured via @ref SetData(). \n
 * The memory must contain `_max_SI0` elements of data type `_type`.\n
 * See parameter `_type` for details.
 *
 * \param _pMutex
 * Pointer to a mutex protecting access to SI0 and to the native data referenced by `_pData`.\n
 * The mutex is optional. If the whole object is read-only __and__ if the application does not modify the
 * value of SI0 and if the application does not modify the data referenced by `_pData`, then a mutex is not required
 * and this parameter may be `nullptr`.\n
 * __BUT__ if SI0 or the array data represented by the object is writeable, or if the application may modify SI0, or
 * if the application may modify the native data referenced by `_pData`, then a mutex must be specified.\n
 * \n
 * The application must obey the following rules when accessing the native data referenced by `_pData`:
 * - If the array data is READ-ONLY, then the application must lock the mutex only if it wants to modify the data
 *   referenced by `_pData`. The application does not need to lock the mutex for reading the data referenced by
 *   `_pData` in this case.
 * - If the array data is READ-WRITE, then the application must lock the mutex ALWAYS when it wants to read or write
 *   the data referenced by `_pData`.
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
ObjectARRAY_wicb::ObjectARRAY_wicb(std::string            const & _name,
                                   attr_t                 const   _attributesSI0,
                                   uint8_t                const   _SI0,
                                   uint8_t                const   _min_SI0,
                                   uint8_t                const   _max_SI0,
                                   DataType               const   _type,
                                   attr_t                 const   _attributes,
                                   void*                  const   _pData,
                                   gpcc::osal::Mutex *    const   _pMutex,
                                   tOnBeforeReadCallback  const & _onBeforeReadCallback,
                                   tOnBeforeWriteCallback const & _onBeforeWriteCallback,
                                   tOnAfterWriteCallback  const & _onAfterWriteCallback)
: IObjectNotifiable()
, ObjectARRAY(_name, _attributesSI0, _SI0, _min_SI0, _max_SI0, _type, _attributes, _pData, _pMutex, this)
, onBeforeReadCallback(_onBeforeReadCallback)
, onBeforeWriteCallback(_onBeforeWriteCallback)
, onAfterWriteCallback(_onAfterWriteCallback)
{
}

// <-- IObjectNotifiable

/// \copydoc gpcc::cood::IObjectNotifiable::OnBeforeRead
SDOAbortCode ObjectARRAY_wicb::OnBeforeRead(gpcc::cood::Object const * pObj,
                                          uint8_t const subindex,
                                          bool const completeAccess,
                                          bool const querySizeWillNotRead)
{
  if (!onBeforeReadCallback)
    return SDOAbortCode::OK;

  return onBeforeReadCallback(pObj, subindex, completeAccess, querySizeWillNotRead);
}

/// \copydoc gpcc::cood::IObjectNotifiable::OnBeforeWrite
SDOAbortCode ObjectARRAY_wicb::OnBeforeWrite(gpcc::cood::Object const * pObj,
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
void ObjectARRAY_wicb::OnAfterWrite(gpcc::cood::Object const * pObj,
                                  uint8_t const subindex,
                                  bool const completeAccess)
{
  if (onAfterWriteCallback)
    onAfterWriteCallback(pObj, subindex, completeAccess);
}

// --> IObjectNotifiable

} // namespace cood
} // namespace gpcc
