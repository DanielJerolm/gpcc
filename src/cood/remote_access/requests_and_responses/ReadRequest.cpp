/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ReadRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequestResponse.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/string/tools.hpp>
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const ReadRequest::readRequestBinarySize;

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
 * \param _accessType
 * Access type.
 *
 * \param _index
 * Index of the object that shall be read.
 *
 * \param _subindex
 * Subindex that shall be read.\n
 * In case of a complete access, this is the first subindex being read. It must be zero or one.
 *
 * \param _permissions
 * Permissions for the read-operation provided by the originator of the read request.\n
 * This shall be composed by logical-or-combination of one or multiple Object::attr_ACCESS_xxx values.\n
 * The composed value shall have at least one read permission (@ref Object::attr_ACCESS_RD) bit set.\n
 * There shall be no other bits set except for read permission bits.
 *
 * \param _maxResponseSize
 * Maximum size (in byte) of the serialized response object that can be processed by the creator of this request.
 * The value should be the minimum of the capability of the creator and the maximum possible response size announced
 * by @ref IRemoteObjectDictionaryAccessNotifiable::OnReady(), parameter `maxResponseSize`.\n
 * Minimum value: @ref ResponseBase::minimumUsefulResponseSize \n
 * Maximum value: @ref ResponseBase::maxResponseSize \n
 * \n
 * The value usually does not contain any @ref ReturnStackItem objects. However, if the creator of the request is going
 * to push a @ref ReturnStackItem object onto the stack of the request before passing it to
 * @ref IRemoteObjectDictionaryAccess::Send(), then `_maxResponseSize` shall be decreased by the size of
 * a serialized @ref ReturnStackItem object to compensate for @ref Push(), which will add the size of a
 * @ref ReturnStackItem object.\n
 * See figure below:
 * \htmlonly <style>div.image img[src="cood/RODA_ReqCTOR_MaxResponseSize.png"]{width:80%;}</style> \endhtmlonly
 * \image html "cood/RODA_ReqCTOR_MaxResponseSize.png" "Maximum response size with one ReturnStackItem"
 */
ReadRequest::ReadRequest(AccessType     const _accessType,
                         uint16_t       const _index,
                         uint8_t        const _subindex,
                         Object::attr_t const _permissions,
                         size_t         const _maxResponseSize)
: RequestBase(RequestTypes::readRequest, _maxResponseSize)
, accessType(_accessType)
, index(_index)
, subindex(_subindex)
, permissions(_permissions)
{
  if ((accessType != AccessType::singleSubindex) && (subindex > 1U))
    throw std::invalid_argument("ReadRequest::ReadRequest: Complete access requires '_subindex' in range 0..1");

  if (   ((permissions & Object::attr_ACCESS_RD) == 0U)
      || ((permissions & Object::attr_ACCESS_RD) != permissions))
  {
    throw std::invalid_argument("ReadRequest::ReadRequest: '_permissions' invalid.");
  }
}

/**
 * \brief Constructor. Creates a @ref ReadRequest object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref ReadRequest object.
 *
 * This is intended to be invoked by @ref RequestBase::FromBinary() only. In conjunction with
 * @ref RequestBase::FromBinary(), this is the counterpart to @ref RequestBase::ToBinary().
 *
 * \post   Any data associated with the object has been consumed from the stream.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from `sr` and `sr` is not recovered.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of data may have been read from `sr` and `sr` is not recovered.
 *
 * - - -
 *
 * \param sr
 * Stream from which the data shall be read.
 *
 * \param versionOnHand
 * Version of serialized object read from `sr`.
 */
ReadRequest::ReadRequest(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, ReadRequestPassKey)
: RequestBase(RequestTypes::readRequest, sr, versionOnHand)
, accessType(U8_to_AccessType(sr.Read_uint8()))
, index(sr.Read_uint16())
, subindex(sr.Read_uint8())
, permissions(static_cast<Object::attr_t>(sr.Read_uint16()))
{
  if (versionOnHand != version)
    throw std::runtime_error("ReadRequest::ReadRequest: Version not supported");

  if ((accessType != AccessType::singleSubindex) && (subindex > 1U))
    throw std::runtime_error("ReadRequest::ReadRequest: Data read from 'sr' is invalid");

  if (   ((permissions & Object::attr_ACCESS_RD) == 0U)
      || ((permissions & Object::attr_ACCESS_RD) != permissions))
  {
    throw std::runtime_error("ReadRequest::ReadRequest: Data read from 'sr' is invalid");
  }
}


/**
 * \brief Calculates the maximum size of data that can be read.
 *
 * This may be used by the client of a RODA interface to determine the maximum size of the data that could be read
 * using a @ref ReadRequest object.
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
 * \param maxResponseSize
 * Maximum permitted response size in byte.\n
 * The value should be the minimum of the capability of the creator of the @ref ReadRequest and the maximum possible
 * response size announced by @ref IRemoteObjectDictionaryAccessNotifiable::OnReady(), parameter `maxResponseSize`.
 *
 * \param withRSI
 * Controls, if the function shall reserve space for one @ref ReturnStackItem pushed onto the stack of the
 * @ref ReadRequest object by the creator of the request object. The @ref ReturnStackItem will also be present on the
 * stack of the response object and thus reduce the size of the data that could be read.
 *
 * \return
 * Maximum size of the data that could be read.\n
 * Zero, if the given `maxResponseSize` is too small to read any data.
 */
size_t ReadRequest::CalcMaxDataPayloadInResponse(size_t const maxResponseSize, bool const withRSI) noexcept
{
  return ReadRequestResponse::CalcMaxDataPayload(maxResponseSize, withRSI ? ReturnStackItem::binarySize : 0U);
}

// <-- RequestBase

/// \copydoc gpcc::cood::RequestBase::GetBinarySize
size_t ReadRequest::GetBinarySize(void) const
{
  return RequestBase::GetBinarySize() + readRequestBinarySize;
}

/// \copydoc gpcc::cood::RequestBase::ToBinary
void ReadRequest::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  RequestBase::ToBinary(sw);

  sw.Write_uint8(static_cast<uint8_t>(accessType));
  sw.Write_uint16(index);
  sw.Write_uint8(subindex);
  sw.Write_uint16(permissions);
}

/// \copydoc gpcc::cood::RequestBase::ToString
std::string ReadRequest::ToString(void) const
{
  std::ostringstream s;

  s << "Read request (";

  switch (accessType)
  {
    case AccessType::singleSubindex:
      s << "single subindex";
      break;

    case AccessType::completeAccess_SI0_8bit:
      s << "CA, SI0 8bit";
      break;

    case AccessType::completeAccess_SI0_16bit:
      s << "CA, SI0 16bit";
      break;
  }

  s << ") for " << gpcc::string::ToHex(index, 4U) << ':' << static_cast<unsigned int>(subindex) << ", "
       "Permission = " << gpcc::string::ToHex(static_cast<uint16_t>(permissions), 4U);

  return s.str();
}

// --> RequestBase

/**
 * \brief Safely converts an uint8_t value into a @ref AccessType enum value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   `u8` is not a valid value from the @ref AccessType enumeration.\n
 *                              This is not treated as a logic error, because `u8` is considered to be untested
 *                              data from outside world.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param u8
 * uint8_t value that shall be converted into a @ref AccessType enum value.\n
 * This method handles invalid values gracefully.
 *
 * \return
 * Corresponding value from the @ref AccessType enumeration.
 */
ReadRequest::AccessType ReadRequest::U8_to_AccessType(uint8_t const u8)
{
  switch (u8)
  {
    case static_cast<uint8_t>(AccessType::singleSubindex):
      return AccessType::singleSubindex;

    case static_cast<uint8_t>(AccessType::completeAccess_SI0_8bit):
      return AccessType::completeAccess_SI0_8bit;

    case static_cast<uint8_t>(AccessType::completeAccess_SI0_16bit):
      return AccessType::completeAccess_SI0_16bit;

    default:
      throw std::runtime_error("ReadRequest::U8_to_AccessType: No valid access type value");
  }
}

} // namespace cood
} // namespace gpcc
