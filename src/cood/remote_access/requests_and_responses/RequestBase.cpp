/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021, 2025 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/RequestBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/PingRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequest.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequest.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t  const RequestBase::minimumUsefulRequestSize;
size_t  const RequestBase::maxRequestSize;
uint8_t const RequestBase::version_;
size_t  const RequestBase::baseBinarySize_;

/**
 * \brief Destructor.
 *
 * \pre   The object is not enqueued in any IntrusiveDList<RequestBase>.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
RequestBase::~RequestBase(void)
{
  // object sill in IntrusiveDList?
  if (   (pPrevInIntrusiveDList != nullptr)
      || (pNextInIntrusiveDList != nullptr))
  {
    PANIC();
  }
}

/**
 * \brief Creates a remote access request object (subclass of @ref RequestBase) from data read from a stream.
 *
 * This is the counterpart of @ref ToBinary().
 *
 * \post   Any data associated with the remote access request object has been consumed from the stream.\n
 *         If the stream contains nothing else but the remote access request object, then the caller should
 *         verify that the stream is empty after calling this.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
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
 * \return
 * Instance of a sub-class of class @ref RequestBase, created from information consumed from `sr`.
 */
std::unique_ptr<RequestBase> RequestBase::FromBinary(gpcc::stream::IStreamReader & sr)
{
  // check version
  auto const version = sr.Read_uint8();
  if (version != version_)
    throw std::runtime_error("RequestBase::FromBinary: Version of serialized object is not supported");

  // check type and delegate to appropriate subclass
  auto const _type = ToRequestType(sr.Read_uint8());
  switch (_type)
  {
    case RequestTypes::objectEnumRequest:
      return std::make_unique<ObjectEnumRequest>(sr, version, ObjectEnumRequestPassKey());

    case RequestTypes::objectInfoRequest:
      return std::make_unique<ObjectInfoRequest>(sr, version, ObjectInfoRequestPassKey());

    case RequestTypes::pingRequest:
      return std::make_unique<PingRequest>(sr, version, PingRequestPassKey());

    case RequestTypes::readRequest:
      return std::make_unique<ReadRequest>(sr, version, ReadRequestPassKey());

    case RequestTypes::writeRequest:
      return std::make_unique<WriteRequest>(sr, version, WriteRequestPassKey());
  }

  throw std::logic_error("RequestBase::FromBinary: Internal error (no create-method)");
}

/**
 * \brief Returns the size of the output of [ToBinary()](@ref gpcc::cood::RequestBase::ToBinary).
 *
 * This method is intended to be used to determine the exact amount of memory required for invocation of
 * [ToBinary()](@ref gpcc::cood::RequestBase::ToBinary) in advance.
 *
 * Hint for sub-classing:\n
 * Derived classes shall invoke the GetBinarySize()-method of their direct base class (e.g. this class) and return the
 * sum of the base class' return value and their own binary size.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Size of the serialized object in byte, excl. any derived classes.
 */
size_t RequestBase::GetBinarySize(void) const
{
  static_assert(RequestBase::maxRequestSize > RequestBase::minimumUsefulRequestSize,
    "Maximum request size is less than the minimum useful size");
  static_assert(RequestBase::baseBinarySize_ < RequestBase::minimumUsefulRequestSize,
    "No payload left for subclass");

  return baseBinarySize_ + (returnStack_.size() * ReturnStackItem::binarySize);
}

/**
 * \brief Writes a binary representation of the object into a stream, which can be deserialized into an appropriate
 *        object via [FromBinary()](@ref gpcc::cood::RequestBase::FromBinary).
 *
 * The counterpart of this is [FromBinary()](@ref gpcc::cood::RequestBase::FromBinary).
 *
 * [GetBinarySize()](@ref gpcc::cood::RequestBase::GetBinarySize) may be used to determine the size of the written
 * binary data in advance.
 *
 * Hint for sub-classing:\n
 * Derived classes shall first invoke the ToBinary()-method of their direct base class (e.g. this class) and then
 * append their own binary to the stream.
 *
 * \pre   This object was not the source of a move-construction or move-assignment operation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - An undefined amount of undefined data may have been written to `sw`. `sw` is not recovered.
 *
 * __Thread cancellation safety:__\n
 * Basic guarantee:
 * - An undefined amount of undefined data may have been written to `sw`. `sw` is not recovered.
 *
 * - - -
 *
 * \param sw
 * The binary data is written into the referenced stream.
 */
void RequestBase::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  // to be read by FromBinary()
  sw.Write_uint8(version_);
  sw.Write_uint8(static_cast<uint8_t>(type_));

  // to be read by CTOR
  sw.Write_uint32(maxResponseSize_);

  sw.Write_uint8(returnStack_.size());
  for (auto const & e: returnStack_)
    e.ToBinary(sw);
}

/**
 * \brief Pushes an @ref ReturnStackItem on the request's stack of @ref ReturnStackItem items.
 *
 * This will also increase the value of the maximum permitted response size embedded in the request object.
 *
 * \pre   The stack contains less than 255 items.
 *
 * \pre   The current permitted response size is equal to or less than the maximum (@ref ResponseBase::maxResponseSize)
 *        minus the binary size of one @ref ReturnStackItem.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rsi
 * A copy of the referenced @ref ReturnStackItem object will be pushed onto the stack of @ref ReturnStackItem objects.
 */
void RequestBase::Push(ReturnStackItem const & rsi)
{
  if (returnStack_.size() == std::numeric_limits<uint8_t>::max())
    throw std::runtime_error("RequestBase::Push: Stack size at maximum");

  if (maxResponseSize_ > (ResponseBase::maxResponseSize - ReturnStackItem::binarySize))
    throw std::logic_error("RequestBase::Push: 'maxResponseSize' would overflow");

  returnStack_.emplace_back(rsi);
  maxResponseSize_ += ReturnStackItem::binarySize;
}

/**
 * \brief Removes the latest pushed @ref ReturnStackItem from the stack of return stack items.
 *
 * This will also revert increase of the value of the maximum permitted response size embedded in the request object
 * which has been done by @ref Push().
 *
 * \pre   The stack is not empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void RequestBase::UndoPush(void)
{
  if (returnStack_.empty())
    throw std::logic_error("RequestBase::UndoPush: Empty");

  returnStack_.pop_back();
  maxResponseSize_ -= ReturnStackItem::binarySize;
}

/**
 * \brief Extracts the stack of @ref ReturnStackItem objects from the request object.
 *
 * \post   The stack of @ref ReturnStackItem objects is empty.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param dest
 * The stack of @ref ReturnStackItem objects will be move-assigned to the referenced vector.
 */
void RequestBase::ExtractReturnStack(std::vector<ReturnStackItem> & dest) noexcept
{
  dest = std::move(returnStack_);
  returnStack_.clear();
}

/**
 * \brief Retrieves the size of the serialized stack of @ref ReturnStackItem objects included in the output
 *        of @ref ToBinary() and @ref GetBinarySize().
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Size of the serialized stack of of @ref ReturnStackItem objects in byte.
 */
size_t RequestBase::GetReturnStackSize(void) const noexcept
{
  return returnStack_.size() * ReturnStackItem::binarySize;
}

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
 * \param type
 * Type of request.
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
RequestBase::RequestBase(RequestTypes const type, size_t const maxResponseSize)
: type_(type)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, maxResponseSize_(maxResponseSize)
, returnStack_()
{
  if (   (maxResponseSize_ < ResponseBase::minimumUsefulResponseSize)
      || (maxResponseSize_ > ResponseBase::maxResponseSize))
  {
    throw std::invalid_argument("RequestBase::RequestBase: '_maxResponseSize' invalid");
  }
}

/**
 * \brief Constructor. Creates a @ref RequestBase object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref RequestBase object.
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
 * \param type
 * Type of request.
 *
 * \param sr
 * Stream from which the data shall be read.
 *
 * \param versionOnHand
 * Version of serialized object read from `sr`.
 */
RequestBase::RequestBase(RequestTypes const type, gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand)
: type_(type)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, returnStack_()
{
  if (versionOnHand != version_)
    throw std::runtime_error("RequestBase::RequestBase: Version not supported");

  maxResponseSize_ = sr.Read_uint32();
  if (   (maxResponseSize_ < ResponseBase::minimumUsefulResponseSize)
      || (maxResponseSize_ > ResponseBase::maxResponseSize))
  {
    throw std::runtime_error("RequestBase::RequestBase: 'maxResponseSize' invalid");
  }

  uint_fast8_t n = sr.Read_uint8();
  returnStack_.reserve(n);
  while (n != 0U)
  {
    returnStack_.emplace_back(sr);
    --n;
  }
}

/**
 * \brief Copy-constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * @ref RequestBase object that shall be copied.
 */
RequestBase::RequestBase(RequestBase const & other)
: type_(other.type_)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, maxResponseSize_(other.maxResponseSize_)
, returnStack_(other.returnStack_)
{
}

/**
 * \brief Move-constructor.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * @ref RequestBase object that shall be moved into the new constructed object.\n
 * Afterwards, the stack of @ref ReturnStackItem objects of `other` will be empty.
 */
RequestBase::RequestBase(RequestBase && other) noexcept
: type_(other.type_)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, maxResponseSize_(other.maxResponseSize_)
, returnStack_(std::move(other.returnStack_))
{
}

/**
 * \brief Copy-assignment operator.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throw std::bad_alloc   Out of memory.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * @ref RequestBase object whose content shall be copy-assigned to this object.\n
 * The types (see @ref RequestTypes) of the two objects must be equal.
 */
RequestBase& RequestBase::operator=(RequestBase const & rhv)
{
  if (&rhv != this)
  {
    if (type_ != rhv.type_)
      throw std::logic_error("RequestBase::operator=(&): Different types");

    // not sure if copy-assignment provides the strong guarantee, so we use two steps here...
    auto copyOfReturnStack = rhv.returnStack_;
    returnStack_ = std::move(copyOfReturnStack);

    maxResponseSize_ = rhv.maxResponseSize_;
  }

  return *this;
}

/**
 * \brief Move-assignment operator.
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
 * \param rhv
 * @ref RequestBase object whose content shall be move-assigned to this object.\n
 * The types (see @ref RequestTypes) of the two objects must be equal.\n
 * Afterwards, the stack of @ref ReturnStackItem objects of `rhv` will be empty.
 */
RequestBase& RequestBase::operator=(RequestBase && rhv)
{
  if (&rhv != this)
  {
    if (type_ != rhv.type_)
      throw std::logic_error("RequestBase::operator=(&&): Different types");

    returnStack_ = std::move(rhv.returnStack_);
    rhv.returnStack_.clear();

    maxResponseSize_ = rhv.maxResponseSize_;
  }

  return *this;
}

/**
 * \brief Safely converts an uint8_t value into a @ref RequestTypes enum value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   `value` is not a valid @ref RequestTypes enum value.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * uint8_t value that shall be converted into a @ref RequestTypes enum value.\n
 * This method handles invalid values gracefully.
 *
 * \return
 * Value from the @ref RequestTypes enumeration that corresponds to `value`.
 */
RequestBase::RequestTypes RequestBase::ToRequestType(uint8_t const value)
{
  switch (value)
  {
    case static_cast<uint8_t>(RequestTypes::objectEnumRequest):
      return RequestTypes::objectEnumRequest;

    case static_cast<uint8_t>(RequestTypes::objectInfoRequest):
      return RequestTypes::objectInfoRequest;

    case static_cast<uint8_t>(RequestTypes::pingRequest):
      return RequestTypes::pingRequest;

    case static_cast<uint8_t>(RequestTypes::readRequest):
      return RequestTypes::readRequest;

    case static_cast<uint8_t>(RequestTypes::writeRequest):
      return RequestTypes::writeRequest;

    default:
      throw std::runtime_error("RequestBase::ToRequestType: 'value' is not a valid RequestTypes enum value");
  }
}

} // namespace cood
} // namespace gpcc
