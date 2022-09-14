/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoRequest.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <gpcc/string/tools.hpp>
#include <sstream>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t const ObjectInfoRequest::objectInfoRequestBinarySize;

/**
 * \brief Constructor.
 *
 * The response (@ref ObjectInfoResponse) will always contain information about the object plus information about as
 * many subindices as possible from the range of subindices specified by parameters `_firstSubindex` and
 * `_lastSubIndex`. Information about the complete range will not be provided if:
 * a) The specified range exceeds the maximum number of subindices.
 * b) The payload of the response is completely consumed.
 *
 * In case of b) another request could be issued which continues the query at the next subindex that did not fit into
 * the response.
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
 * \param _index
 * Index of the object whose meta data shall be read.
 *
 * \param _firstSubIndex
 * Number of the first subindex whose meta data shall be read.
 *
 * \param _lastSubIndex
 * Number of the last subindex whose meta data shall be read.
 *
 * \param _inclusiveNames
 * Controls if the names of the object and the subindices shall be included in the response (true) or not (false).\n
 * If names are included, then the size of the response may increase significantly.
 *
 * \param _inclusiveAppSpecificMetaData
 * Controls if application specific meta data of the subindices shall be included in the response (true) or not (false).\n
 * If application specific meta data is included, then the size of the response may increase significantly.
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
ObjectInfoRequest::ObjectInfoRequest(uint16_t const _index,
                                     uint8_t  const _firstSubIndex,
                                     uint8_t  const _lastSubIndex,
                                     bool     const _inclusiveNames,
                                     bool     const _inclusiveAppSpecificMetaData,
                                     size_t   const _maxResponseSize)
: RequestBase(RequestTypes::objectInfoRequest, _maxResponseSize)
, index(_index)
, firstSubIndex(_firstSubIndex)
, lastSubIndex(_lastSubIndex)
, inclusiveNames(_inclusiveNames)
, inclusiveAppSpecificMetaData(_inclusiveAppSpecificMetaData)
{
  if (firstSubIndex > lastSubIndex)
    throw std::invalid_argument("ObjectInfoRequest::ObjectInfoRequest: first/last subindex invalid");
}

/**
 * \brief Constructor. Creates a @ref ObjectInfoRequest object from data read from an
 *        [IStreamReader](@ref gpcc::Stream::IStreamReader) containing a serialized @ref ObjectInfoRequest object.
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
ObjectInfoRequest::ObjectInfoRequest(gpcc::Stream::IStreamReader & sr, uint8_t const versionOnHand, ObjectInfoRequestPassKey)
: RequestBase(RequestTypes::objectInfoRequest, sr, versionOnHand)
, index(sr.Read_uint16())
, firstSubIndex(sr.Read_uint8())
, lastSubIndex(sr.Read_uint8())
, inclusiveNames(sr.Read_bool())
, inclusiveAppSpecificMetaData(sr.Read_bool())
{
  sr.Skip(6U);

  if (versionOnHand != version)
    throw std::runtime_error("ObjectInfoRequest::ObjectInfoRequest: Version not supported");

  if (firstSubIndex > lastSubIndex)
    throw std::runtime_error("ObjectInfoRequest::ObjectInfoRequest: Data read from 'sr' is invalid");
}

// <-- RequestBase

/// \copydoc gpcc::cood::RequestBase::GetBinarySize
size_t ObjectInfoRequest::GetBinarySize(void) const
{
  return RequestBase::GetBinarySize() + objectInfoRequestBinarySize;
}

/// \copydoc gpcc::cood::RequestBase::ToBinary
void ObjectInfoRequest::ToBinary(gpcc::Stream::IStreamWriter & sw) const
{
  RequestBase::ToBinary(sw);

  sw.Write_uint16(index);
  sw.Write_uint8(firstSubIndex);
  sw.Write_uint8(lastSubIndex);
  sw.Write_bool(inclusiveNames);
  sw.Write_bool(inclusiveAppSpecificMetaData);
  sw.AlignToByteBoundary(false);
}

/// \copydoc gpcc::cood::RequestBase::ToString
std::string ObjectInfoRequest::ToString(void) const
{
  std::ostringstream s;

  s << "Object info request for "
    << gpcc::string::ToHex(index, 4U) << ", SI range "
    << static_cast<unsigned int>(firstSubIndex) << ".." << static_cast<unsigned int>(lastSubIndex);

  if (inclusiveNames)
    s << ", incl. names";
  else
    s << ", excl. names";

  if (inclusiveAppSpecificMetaData)
    s << ", incl. asm";
  else
    s << ", excl. asm";

  return s.str();
}

// --> RequestBase

} // namespace cood
} // namespace gpcc
