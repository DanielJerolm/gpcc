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

#include "ObjectEnumRequest.hpp"
#include "gpcc/src/Stream/IStreamReader.hpp"
#include "gpcc/src/Stream/IStreamWriter.hpp"
#include "gpcc/src/string/tools.hpp"
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const ObjectEnumRequest::objectEnumRequestBinarySize;

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
 * \param _startIndex
 * Index where enumeration shall start.\n
 * Objects located at indices less than this will not be enumerated.
 *
 * \param _lastIndex
 * Index where enumeration shall stop.\n
 * Objects located at indices larger than this will not be enumerated.
 *
 * \param _attrFilter
 * Attribute-filter for enumeration.\n
 * Only objects with at least one matching attribute bit will be enumerated.
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
ObjectEnumRequest::ObjectEnumRequest(uint16_t                   const _startIndex,
                                     uint16_t                   const _lastIndex,
                                     gpcc::cood::Object::attr_t const _attrFilter,
                                     size_t                     const _maxResponseSize)
: RequestBase(RequestTypes::objectEnumRequest, _maxResponseSize)
, startIndex(_startIndex)
, lastIndex(_lastIndex)
, attrFilter(_attrFilter)
{
  if ((startIndex > lastIndex) || (attrFilter == 0U))
    throw std::invalid_argument("ObjectEnumRequest::ObjectEnumRequest: Invalid args");
}

/**
 * \brief Constructor. Creates a @ref ObjectEnumRequest object from data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref ObjectEnumRequest object.
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
ObjectEnumRequest::ObjectEnumRequest(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectEnumRequestPassKey)
: RequestBase(RequestTypes::objectEnumRequest, sr, versionOnHand)
, startIndex(sr.Read_uint16())
, lastIndex(sr.Read_uint16())
, attrFilter(static_cast<gpcc::cood::Object::attr_t>(sr.Read_uint16()))
{
  if (versionOnHand != version)
    throw std::runtime_error("ObjectEnumRequest::ObjectEnumRequest: Version not supported");

  if ((startIndex > lastIndex) || (attrFilter == 0U))
    throw std::runtime_error("ObjectEnumRequest::ObjectEnumRequest: Data read from 'sr' is invalid");
}

// <-- RequestBase

/// \copydoc gpcc::cood::RequestBase::GetBinarySize
size_t ObjectEnumRequest::GetBinarySize(void) const
{
  return RequestBase::GetBinarySize() + objectEnumRequestBinarySize;
}

/// \copydoc gpcc::cood::RequestBase::ToBinary
void ObjectEnumRequest::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  RequestBase::ToBinary(sw);

  sw.Write_uint16(startIndex);
  sw.Write_uint16(lastIndex);
  sw.Write_uint16(static_cast<uint16_t>(attrFilter));
}

/// \copydoc gpcc::cood::RequestBase::ToString
std::string ObjectEnumRequest::ToString(void) const
{
  std::ostringstream s;

  s << "Object enum request. Start = "
    << gpcc::string::ToHex(startIndex, 4U) << ", Last = "
    << gpcc::string::ToHex(lastIndex, 4U) << ", AF = "
    << gpcc::string::ToHex(static_cast<uint16_t>(attrFilter), 4U);

  return s.str();
}

// --> RequestBase

} // namespace cood
} // namespace gpcc
