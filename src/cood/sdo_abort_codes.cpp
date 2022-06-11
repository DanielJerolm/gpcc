/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2018 Daniel Jerolm

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

#include "sdo_abort_codes.hpp"
#include <stdexcept>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_SDOABORTCODES
 * \brief Retrieves a string containing a description of an error specified by a @ref SDOAbortCode value.
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
 * \param value
 * Enum value.
 *
 * \return
 * Pointer to a null-terminated c-string located in code-memory, which describes the error condition specified by `value`.\n
 * Format:\n
 * Hex-value (description)\n
 * \n
 * Example:\n
 * 0x05030000 (Toggle bit not altered)
 */
char const * SDOAbortCodeToDescrString(SDOAbortCode const value)
{
  switch (value)
  {
    case SDOAbortCode::OK                             : return "0x00000000 (OK)";
    case SDOAbortCode::ToggleBitNotAltered            : return "0x05030000 (Toggle bit not altered)";
    case SDOAbortCode::SDOProtocolTimeOut             : return "0x05040000 (SDO Protocol timeout)";
    case SDOAbortCode::CMDSpecNotValid                : return "0x05040001 (Client/Server command specifier invalid or unknown)";
    case SDOAbortCode::InvalidBlockSize               : return "0x05040002 (Invalid block size)";
    case SDOAbortCode::InvalidSeqNumber               : return "0x05040003 (Invalid sequence number)";
    case SDOAbortCode::CRCError                       : return "0x05040004 (CRC Error)";
    case SDOAbortCode::OutOfMemory                    : return "0x05040005 (Out of memory)";
    case SDOAbortCode::UnsupportedAccessToObject      : return "0x06010000 (Unsupported access to an object)";
    case SDOAbortCode::AttemptToReadWrOnlyObject      : return "0x06010001 (Attempt to read a write only object)";
    case SDOAbortCode::AttemptToWriteRdOnlyObject     : return "0x06010002 (Attempt to write a read only object)";
    case SDOAbortCode::SICannotBeWrittenSI0MustBeZero : return "0x06010003 (Subindex cannot be written, SI0 must be zero for write access)";
    case SDOAbortCode::CAnotSupportedForVarLengthObjs : return "0x06010004 (SDO Complete Access not supported for variable length objects)";
    case SDOAbortCode::ObjectLengthExceedsMbxSize     : return "0x06010005 (Object length exceeds mailbox size)";
    case SDOAbortCode::ObjectMappedToRxPDO            : return "0x06010006 (Object is mapped to RxPDO, SDO download is blocked)";
    case SDOAbortCode::ObjectDoesNotExist             : return "0x06020000 (Object does not exist)";
    case SDOAbortCode::ObjectCannotBeMappedIntoPDO    : return "0x06040041 (Object cannot be mapped into the PDO)";
    case SDOAbortCode::MappingWouldExceedPDOLength    : return "0x06040042 (The number and/or length of the objects to be mapped would exceed the PDO length)";
    case SDOAbortCode::GeneralParamIncompatibility    : return "0x06040043 (General parameter incompatibility)";
    case SDOAbortCode::GeneralIntIncompatibility      : return "0x06040047 (General internal incompatibility in the device)";
    case SDOAbortCode::AccessFailedDueToHWError       : return "0x06060000 (Access failed due to a hardware error)";
    case SDOAbortCode::DataTypeMismatch               : return "0x06070010 (Data type mismatch (length of service parameter does not match))";
    case SDOAbortCode::DataTypeMismatchTooLong        : return "0x06070012 (Data type mismatch (length of service parameter too large))";
    case SDOAbortCode::DataTypeMismatchTooSmall       : return "0x06070013 (Data type mismatch (length of service parameter too small))";
    case SDOAbortCode::SubindexDoesNotExist           : return "0x06090011 (Sub-index does not exist)";
    case SDOAbortCode::ValueRangeExceeded             : return "0x06090030 (Value to be written is out of range)";
    case SDOAbortCode::ValueTooHigh                   : return "0x06090031 (Value to be written exceeds upper bound)";
    case SDOAbortCode::ValueTooLow                    : return "0x06090032 (Value to be written exceeds lower bound)";
    case SDOAbortCode::ConfModListDoesNotMatch        : return "0x06090033 (Configured module list does not match detected module list)";
    case SDOAbortCode::MaxValueLessThanMinValue       : return "0x06090036 (Maximum value is less than minimum value)";
    case SDOAbortCode::GeneralError                   : return "0x08000000 (General error)";
    case SDOAbortCode::CantXferDataToApp              : return "0x08000020 (Data cannot be transferred to the application)";
    case SDOAbortCode::CantXferDataToAppDueLocalCtrl  : return "0x08000021 (Data cannot be transferred to the application due to local control)";
    case SDOAbortCode::CantXferDataToAppDueDevState   : return "0x08000022 (Data cannot be transferred to the application due to current ESM state)";
    case SDOAbortCode::OBDDynGenFailedOrODNotPresent  : return "0x08000023 (Object dictionary creation failed or no object dictionary present)";
    default:
      throw std::invalid_argument("SDOAbortCodeToDescrString: Unknown/invalid enum value");
  }
}

/**
 * \ingroup GPCC_COOD_SDOABORTCODES
 * \brief Safely converts an uint32_t value into an @ref SDOAbortCode enum value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::invalid_argument   `value` is not a valid @ref SDOAbortCode enum value.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * uint32_t value that shall be converted into a @ref SDOAbortCode enum value.\n
 * This method handles invalid values gracefully.
 *
 * \return
 * Value from the @ref SDOAbortCode enumeration that corresponds to `value`.
 */
SDOAbortCode U32ToSDOAbortCode(uint32_t const value)
{
  switch (value)
  {
    case static_cast<uint32_t>(SDOAbortCode::OK)                             : return SDOAbortCode::OK;
    case static_cast<uint32_t>(SDOAbortCode::ToggleBitNotAltered)            : return SDOAbortCode::ToggleBitNotAltered;
    case static_cast<uint32_t>(SDOAbortCode::SDOProtocolTimeOut)             : return SDOAbortCode::SDOProtocolTimeOut;
    case static_cast<uint32_t>(SDOAbortCode::CMDSpecNotValid)                : return SDOAbortCode::CMDSpecNotValid;
    case static_cast<uint32_t>(SDOAbortCode::InvalidBlockSize)               : return SDOAbortCode::InvalidBlockSize;
    case static_cast<uint32_t>(SDOAbortCode::InvalidSeqNumber)               : return SDOAbortCode::InvalidSeqNumber;
    case static_cast<uint32_t>(SDOAbortCode::CRCError)                       : return SDOAbortCode::CRCError;
    case static_cast<uint32_t>(SDOAbortCode::OutOfMemory)                    : return SDOAbortCode::OutOfMemory;
    case static_cast<uint32_t>(SDOAbortCode::UnsupportedAccessToObject)      : return SDOAbortCode::UnsupportedAccessToObject;
    case static_cast<uint32_t>(SDOAbortCode::AttemptToReadWrOnlyObject)      : return SDOAbortCode::AttemptToReadWrOnlyObject;
    case static_cast<uint32_t>(SDOAbortCode::AttemptToWriteRdOnlyObject)     : return SDOAbortCode::AttemptToWriteRdOnlyObject;
    case static_cast<uint32_t>(SDOAbortCode::SICannotBeWrittenSI0MustBeZero) : return SDOAbortCode::SICannotBeWrittenSI0MustBeZero;
    case static_cast<uint32_t>(SDOAbortCode::CAnotSupportedForVarLengthObjs) : return SDOAbortCode::CAnotSupportedForVarLengthObjs;
    case static_cast<uint32_t>(SDOAbortCode::ObjectLengthExceedsMbxSize)     : return SDOAbortCode::ObjectLengthExceedsMbxSize;
    case static_cast<uint32_t>(SDOAbortCode::ObjectMappedToRxPDO)            : return SDOAbortCode::ObjectMappedToRxPDO;
    case static_cast<uint32_t>(SDOAbortCode::ObjectDoesNotExist)             : return SDOAbortCode::ObjectDoesNotExist;
    case static_cast<uint32_t>(SDOAbortCode::ObjectCannotBeMappedIntoPDO)    : return SDOAbortCode::ObjectCannotBeMappedIntoPDO;
    case static_cast<uint32_t>(SDOAbortCode::MappingWouldExceedPDOLength)    : return SDOAbortCode::MappingWouldExceedPDOLength;
    case static_cast<uint32_t>(SDOAbortCode::GeneralParamIncompatibility)    : return SDOAbortCode::GeneralParamIncompatibility;
    case static_cast<uint32_t>(SDOAbortCode::GeneralIntIncompatibility)      : return SDOAbortCode::GeneralIntIncompatibility;
    case static_cast<uint32_t>(SDOAbortCode::AccessFailedDueToHWError)       : return SDOAbortCode::AccessFailedDueToHWError;
    case static_cast<uint32_t>(SDOAbortCode::DataTypeMismatch)               : return SDOAbortCode::DataTypeMismatch;
    case static_cast<uint32_t>(SDOAbortCode::DataTypeMismatchTooLong)        : return SDOAbortCode::DataTypeMismatchTooLong;
    case static_cast<uint32_t>(SDOAbortCode::DataTypeMismatchTooSmall)       : return SDOAbortCode::DataTypeMismatchTooSmall;
    case static_cast<uint32_t>(SDOAbortCode::SubindexDoesNotExist)           : return SDOAbortCode::SubindexDoesNotExist;
    case static_cast<uint32_t>(SDOAbortCode::ValueRangeExceeded)             : return SDOAbortCode::ValueRangeExceeded;
    case static_cast<uint32_t>(SDOAbortCode::ValueTooHigh)                   : return SDOAbortCode::ValueTooHigh;
    case static_cast<uint32_t>(SDOAbortCode::ValueTooLow)                    : return SDOAbortCode::ValueTooLow;
    case static_cast<uint32_t>(SDOAbortCode::ConfModListDoesNotMatch)        : return SDOAbortCode::ConfModListDoesNotMatch;
    case static_cast<uint32_t>(SDOAbortCode::MaxValueLessThanMinValue)       : return SDOAbortCode::MaxValueLessThanMinValue;
    case static_cast<uint32_t>(SDOAbortCode::GeneralError)                   : return SDOAbortCode::GeneralError;
    case static_cast<uint32_t>(SDOAbortCode::CantXferDataToApp)              : return SDOAbortCode::CantXferDataToApp;
    case static_cast<uint32_t>(SDOAbortCode::CantXferDataToAppDueLocalCtrl)  : return SDOAbortCode::CantXferDataToAppDueLocalCtrl;
    case static_cast<uint32_t>(SDOAbortCode::CantXferDataToAppDueDevState)   : return SDOAbortCode::CantXferDataToAppDueDevState;
    case static_cast<uint32_t>(SDOAbortCode::OBDDynGenFailedOrODNotPresent)  : return SDOAbortCode::OBDDynGenFailedOrODNotPresent;
    default:
      throw std::invalid_argument("U32ToSDOAbortCode: 'value' is not a valid SDOAbortCode enum value");
  }
}

} // namespace cood
} // namespace gpcc
