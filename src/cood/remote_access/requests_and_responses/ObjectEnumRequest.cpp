/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021, 2024, 2025 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <gpcc/string/tools.hpp>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const ObjectEnumRequest::objectEnumRequestBinarySize_;

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
 * \param startIndex
 * Index where enumeration shall start.\n
 * Objects located at indices less than this will not be enumerated.
 *
 * \param lastIndex
 * Index where enumeration shall stop.\n
 * Objects located at indices larger than this will not be enumerated.
 *
 * \param attrFilter
 * Attribute-filter for enumeration.\n
 * Only objects with at least one matching attribute bit will be enumerated.
 *
 * \param maxResponseSize
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
ObjectEnumRequest::ObjectEnumRequest(uint16_t                   const startIndex,
                                     uint16_t                   const lastIndex,
                                     gpcc::cood::Object::attr_t const attrFilter,
                                     size_t                     const maxResponseSize)
: RequestBase(RequestTypes::objectEnumRequest, maxResponseSize)
, startIndex_(startIndex)
, lastIndex_(lastIndex)
, attrFilter_(attrFilter)
{
  if ((startIndex_ > lastIndex_) || (attrFilter_ == 0U))
    throw std::invalid_argument("ObjectEnumRequest::ObjectEnumRequest: Invalid args");
}

/**
 * \brief Constructor. Creates a @ref ObjectEnumRequest object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref ObjectEnumRequest object.
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
ObjectEnumRequest::ObjectEnumRequest(gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectEnumRequestPassKey)
: RequestBase(RequestTypes::objectEnumRequest, sr, versionOnHand)
, startIndex_(sr.Read_uint16())
, lastIndex_(sr.Read_uint16())
, attrFilter_(static_cast<gpcc::cood::Object::attr_t>(sr.Read_uint16()))
{
  if (versionOnHand != version_)
    throw std::runtime_error("ObjectEnumRequest::ObjectEnumRequest: Version not supported");

  if ((startIndex_ > lastIndex_) || (attrFilter_ == 0U))
    throw std::runtime_error("ObjectEnumRequest::ObjectEnumRequest: Data read from 'sr' is invalid");
}

// <-- RequestBase

/// \copydoc gpcc::cood::RequestBase::GetBinarySize
size_t ObjectEnumRequest::GetBinarySize(void) const
{
  return RequestBase::GetBinarySize() + objectEnumRequestBinarySize_;
}

/// \copydoc gpcc::cood::RequestBase::ToBinary
void ObjectEnumRequest::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  RequestBase::ToBinary(sw);

  sw.Write_uint16(startIndex_);
  sw.Write_uint16(lastIndex_);
  sw.Write_uint16(static_cast<uint16_t>(attrFilter_));
}

/// \copydoc gpcc::cood::RequestBase::ToString
std::string ObjectEnumRequest::ToString(void) const
{
  gpcc::string::StringComposer s;

  s << "Object enum request. Start = "
    << gpcc::string::ToHex(startIndex_, 4U) << ", Last = "
    << gpcc::string::ToHex(lastIndex_, 4U) << ", AF = "
    << gpcc::string::ToHex(static_cast<uint16_t>(attrFilter_), 4U);

  return s.Get();
}

// --> RequestBase

} // namespace cood
} // namespace gpcc
