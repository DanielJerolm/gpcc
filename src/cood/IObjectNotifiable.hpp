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

#ifndef IOBJECTNOTIFIABLE_HPP_202103121617
#define IOBJECTNOTIFIABLE_HPP_202103121617

#include "sdo_abort_codes.hpp"
#include <cstdint>
#include <cstddef>

namespace gpcc {
namespace cood {

class Object;

/**
 * \ingroup GPCC_COOD
 * \brief Interface which must be implemented by the owner of a CAN object in order to receive notifications when
 *        the object's data is accessed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The object dictionary is locked for object access when a method offered by this interface is executed.\n
 * The mutex associated with the data represented by the object is locked when a method offered by this interface is
 * executed. The mutex is optional, so this means:
 * - If a data mutex for the CAN object is specified, then __only one__ thread will invoke a method from this interface
 *   at any time.
 * - If __no__ data mutex is specified, then __multiple threads__ may invoke one or more methods offered by this
 *   interface simultaneously.
 */
class IObjectNotifiable
{
  public:
    IObjectNotifiable(void) = default;

    virtual SDOAbortCode OnBeforeRead(gpcc::cood::Object const * pObj,
                                      uint8_t const subindex,
                                      bool const completeAccess,
                                      bool const querySizeWillNotRead) = 0;

    virtual SDOAbortCode OnBeforeWrite(gpcc::cood::Object const * pObj,
                                       uint8_t const subindex,
                                       bool const completeAccess,
                                       uint8_t const valueWrittenToSI0,
                                       void const * pData) = 0;

    virtual void OnAfterWrite(gpcc::cood::Object const * pObj,
                              uint8_t const subindex,
                              bool const completeAccess) = 0;

  protected:
    IObjectNotifiable(IObjectNotifiable const &) = default;
    IObjectNotifiable(IObjectNotifiable &&) = default;
    virtual ~IObjectNotifiable(void) = default;

    IObjectNotifiable& operator=(IObjectNotifiable const &) = default;
    IObjectNotifiable& operator=(IObjectNotifiable &&) = default;
};

/**
 * \fn gpcc::cood::SDOAbortCode IObjectNotifiable::OnBeforeRead
 * \brief This will be invoked before an object is read.
 *
 * Sub-classes of class [Object](@ref gpcc::cood::Object) may allow registration of an
 * [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) interface. This method will be invoked before any data
 * represented by the object dictionary object is read. This allows the object's owner to update the data or to reject
 * the read request.
 *
 * In case of subindices with data types that incorporate flexible length (e.g.
 * [visible_string](@ref gpcc::cood::DataType::visible_string)) __and__ flexible length being supported by the
 * concrete sub-class of class [Object](@ref gpcc::cood::Object), this method will also be invoked when the current
 * size of a subindex incorporating flexible length is queried via
 * [GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize). This allows the object's owner to update
 * the data, so that [GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize) returns an up-to-date
 * value. This will not be invoked for subindices whose data types do not incorporate flexible length or if the
 * concrete sub-class of class [Object](@ref gpcc::cood::Object) does not support flexible length.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__
 * - This method will be executed in the context of the thread that has called
 *   [Object::GetSubIdxActualSize(...)](@ref gpcc::cood::Object::GetSubIdxActualSize),
 *   [Object::Read(...)](@ref gpcc::cood::Object::Read), or
 *   [Object::CompleteRead(...)](@ref gpcc::cood::Object::CompleteRead).
 * - The object dictionary is locked for object access when this is executed.
 * - The mutex associated with the data represented by the object is locked when this is executed.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the strong guarantee.\n
 * The return value shall be used to indicate any error condition that is not a software error.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method shall provide at least the strong guarantee.
 *
 * - - -
 *
 * \param pObj
 * Pointer to the object being read.
 *
 * \param subindex
 * Subindex being read or in case of a complete access the first subindex being read.\n
 * In case of a complete access, this is either 0 or 1.
 *
 * \param completeAccess
 * Access type: Complete access (true) or single access (false).
 *
 * \param querySizeWillNotRead
 * Access sub-type: Query subindex size (true) or subindex read (false).\n
 * This is only valid, if parameter `completeAccess` is false (single access).
 *
 * \return
 * A value from the [SDOAbortCode](@ref gpcc::cood::SDOAbortCode) enumeration.\n
 * In case of a read access: If a value other than [SDOAbortCode::OK](@ref gpcc::cood::SDOAbortCode::OK) is returned,
 * then the read access will be denied with the returned [SDOAbortCode](@ref gpcc::cood::SDOAbortCode) value.\n
 * In case of a subindex size query: If a value other than [SDOAbortCode::OK](@ref gpcc::cood::SDOAbortCode::OK) is
 * returned, then [GetSubIdxActualSize()](@ref gpcc::cood::Object::GetSubIdxActualSize) will throw.
 */

/**
 * \fn gpcc::cood::SDOAbortCode IObjectNotifiable::OnBeforeWrite
 * \brief This will be invoked before data is written to an object.
 *
 * Sub-classes of class [Object](@ref gpcc::cood::Object) may allow registration of an
 * [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) interface. This method will be invoked before any data
 * represented by the object dictionary object is modified. This allows the object's owner to preview the data before
 * the write access takes place and to accept or reject the write request.
 *
 * Note:\n
 * There may be further checks that take place after this method has returned with SDOAbortCode::OK. These checks may
 * fail, so there is no guarantee that the write access will really take place even if this method has agreed.
 * The owner of the CANopen object shall watch for invocation of
 * [OnAfterWrite(...)](@ref gpcc::cood::IObjectNotifiable::OnAfterWrite), which indicates that the write access has
 * taken place.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__
 * - This method will be executed in the context of the thread that has called
 *   [Object::Write(...)](@ref gpcc::cood::Object) or
 *   [Object::CompleteWrite(...)](@ref gpcc::cood::Object::CompleteWrite).
 * - The object dictionary is locked for object access when this is executed.
 * - The mutex associated with the data represented by the object is locked when this is executed.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide at least the strong guarantee.\n
 * The return value shall be used to indicate any error condition that is not a software error.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method shall provide at least the strong guarantee.
 *
 * - - -
 *
 * \param pObj
 * Pointer to the object being written.
 *
 * \param subindex
 * Subindex being written or in case of a complete access the first subindex being written.\n
 * In case of a complete access, this is either 0 or 1.
 *
 * \param completeAccess
 * Access type: Complete access (true) or single access (false).
 *
 * \param valueWrittenToSI0
 * Value written to SI0.\n
 * This is valid, if all of the following conditions are true. For all other cases, this will be zero.
 * - The write is a complete access
 * - SI0 is included in the complete access
 * - The written object is not a VARIABLE object
 *
 * \param pData
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
 * A value from the [SDOAbortCode](@ref gpcc::cood::SDOAbortCode) enumeration.\n
 * If a value other than [SDOAbortCode::OK](@ref gpcc::cood::SDOAbortCode::OK) is returned, then the write access will
 * be denied with the returned [SDOAbortCode](@ref gpcc::cood::SDOAbortCode) value.
 */

/**
 * \fn void IObjectNotifiable::OnAfterWrite
 * \brief This will be invoked after a write to an object has taken place.
 *
 * Sub-classes of class [Object](@ref gpcc::cood::Object) may allow registration of an
 * [IObjectNotifiable](@ref gpcc::cood::IObjectNotifiable) interface. This method will be invoked after data has been
 * written.\n
 * The method will only be invoked, if data has really been written. If e.g.
 * [OnBeforeWrite(...)](@ref gpcc::cood::IObjectNotifiable::OnBeforeWrite) rejects the write access, then the write
 * will not take place and this method will not be invoked.
 *
 * \pre   Data has been written to the object.
 *
 * - - -
 *
 * __Thread safety requirements/hints:__\n
 * - This method will be executed in the context of the thread that has called
 *   [Object::Write(...)](@ref gpcc::cood::Object::Write) or
 *   [Object::CompleteWrite(...)](@ref gpcc::cood::Object::CompleteWrite).
 * - The object dictionary is locked for object access when this is executed.
 * - The mutex associated with the data represented by the object is locked when this is executed.
 *
 * __Exception safety requirements/hints:__\n
 * This method shall provide the no-throw guarantee.\n
 * Any thrown exception will result in panic.
 *
 * __Thread cancellation safety requirements/hints:__\n
 * This method shall not contain any cancellation point.
 *
 * - - -
 *
 * \param pObj
 * Pointer to the object being written.
 *
 * \param subindex
 * Subindex being written or in case of a complete access the first subindex being written.\n
 * In case of a complete access, this is either 0 or 1.
 *
 * \param completeAccess
 * Access type: Complete access (true) or single access (false).
 */

} // namespace cood
} // namespace gpcc

#endif // IOBJECTNOTIFIABLE_HPP_202103121617
