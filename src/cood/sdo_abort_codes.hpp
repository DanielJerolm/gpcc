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

#ifndef SDO_ABORT_CODES_HPP_201809212221
#define SDO_ABORT_CODES_HPP_201809212221

#include <cstdint>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_SDOABORTCODES
 * \brief Enumeration with SDO Abort Codes.
 */
enum class SDOAbortCode
{
  OK                             = 0x00000000UL, ///<OK
  ToggleBitNotAltered            = 0x05030000UL, ///<Toggle bit not altered.
  SDOProtocolTimeOut             = 0x05040000UL, ///<SDO Protocol timeout.
  CMDSpecNotValid                = 0x05040001UL, ///<Client/Server command specifier invalid or unknown.
  InvalidBlockSize               = 0x05040002UL, ///<Invalid block size.
  InvalidSeqNumber               = 0x05040003UL, ///<Invalid sequence number.
  CRCError                       = 0x05040004UL, ///<CRC Error.
  OutOfMemory                    = 0x05040005UL, ///<Out of memory.
  UnsupportedAccessToObject      = 0x06010000UL, ///<Unsupported access to an object.
  AttemptToReadWrOnlyObject      = 0x06010001UL, ///<Attempt to read a write only object.
  AttemptToWriteRdOnlyObject     = 0x06010002UL, ///<Attempt to write a read only object.
  SICannotBeWrittenSI0MustBeZero = 0x06010003UL, ///<Subindex cannot be written, SI0 must be zero for write access.
  CAnotSupportedForVarLengthObjs = 0x06010004UL, ///<SDO Complete Access not supported for variable length objects.
  ObjectLengthExceedsMbxSize     = 0x06010005UL, ///<Object length exceeds mailbox size.
  ObjectMappedToRxPDO            = 0x06010006UL, ///<Object is mapped to RxPDO, SDO download is blocked.
  ObjectDoesNotExist             = 0x06020000UL, ///<Object does not exist.
  ObjectCannotBeMappedIntoPDO    = 0x06040041UL, ///<Object cannot be mapped into the PDO.
  MappingWouldExceedPDOLength    = 0x06040042UL, ///<The number and/or length of the objects to be mapped would exceed the PDO length.
  GeneralParamIncompatibility    = 0x06040043UL, ///<General parameter incompatibility.
  GeneralIntIncompatibility      = 0x06040047UL, ///<General internal incompatibility in the device.
  AccessFailedDueToHWError       = 0x06060000UL, ///<Access failed due to a hardware error.
  DataTypeMismatch               = 0x06070010UL, ///<Data type mismatch (length of service parameter does not match).
  DataTypeMismatchTooLong        = 0x06070012UL, ///<Data type mismatch (length of service parameter too large).
  DataTypeMismatchTooSmall       = 0x06070013UL, ///<Data type mismatch (length of service parameter too small).
  SubindexDoesNotExist           = 0x06090011UL, ///<Sub-index does not exist.
  ValueRangeExceeded             = 0x06090030UL, ///<Value to be written is out of range.
  ValueTooHigh                   = 0x06090031UL, ///<Value to be written exceeds upper bound.
  ValueTooLow                    = 0x06090032UL, ///<Value to be written exceeds lower bound.
  ConfModListDoesNotMatch        = 0x06090033UL, ///<Configured module list does not match detected module list.
  MaxValueLessThanMinValue       = 0x06090036UL, ///<Maximum value is less than minimum value.
  GeneralError                   = 0x08000000UL, ///<General error.
  CantXferDataToApp              = 0x08000020UL, ///<Data cannot be transferred to the application.
  CantXferDataToAppDueLocalCtrl  = 0x08000021UL, ///<Data cannot be transferred to the application due to local control.
  CantXferDataToAppDueDevState   = 0x08000022UL, ///<Data cannot be transferred to the application due to current ESM state.
  OBDDynGenFailedOrODNotPresent  = 0x08000023UL  ///<Object dictionary creation failed or no object dictionary present.
};

char const * SDOAbortCodeToDescrString(SDOAbortCode const value);
SDOAbortCode U32ToSDOAbortCode(uint32_t const value);

} // namespace cood
} // namespace gpcc


#endif // SDO_ABORT_CODES_HPP_201809212221
