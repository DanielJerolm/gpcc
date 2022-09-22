/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectEnumResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ObjectInfoResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/PingResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ReadRequestResponse.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/WriteRequestResponse.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/stream/IStreamReader.hpp>
#include <gpcc/stream/IStreamWriter.hpp>
#include <stdexcept>

namespace gpcc {
namespace cood {

size_t  const ResponseBase::minimumUsefulResponseSize;
size_t  const ResponseBase::maxResponseSize;
uint8_t const ResponseBase::version;
size_t  const ResponseBase::baseBinarySize;


/**
 * \brief Destructor.
 *
 * \pre   The object is not enqueued in any IntrusiveDList<ResponseBase>.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
ResponseBase::~ResponseBase(void)
{
  // object sill in IntrusiveDList?
  if (   (pPrevInIntrusiveDList != nullptr)
      || (pNextInIntrusiveDList != nullptr))
  {
    PANIC();
  }
}

/**
 * \brief Creates a remote access response object (subclass of @ref ResponseBase) from data read from a stream.
 *
 * This is the counterpart of @ref ToBinary().
 *
 * \post   Any data associated with the response object has been consumed from the stream.\n
 *         If the stream contains nothing else but the remote access response object, then the caller should
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
 * Instance of a sub-class of class @ref ResponseBase, created from information consumed from `sr`.
 */
std::unique_ptr<ResponseBase> ResponseBase::FromBinary(gpcc::stream::IStreamReader & sr)
{
  // check version
  auto const _version = sr.Read_uint8();
  if (_version != version)
    throw std::runtime_error("ResponseBase::FromBinary: Version of serialized object is not supported");

  // check type and delegate to appropriate subclass
  auto const _type = ToResponseType(sr.Read_uint8());
  switch (_type)
  {
    case ResponseTypes::objectEnumResponse:
      return std::make_unique<ObjectEnumResponse>(sr, _version, ObjectEnumResponsePassKey());

    case ResponseTypes::objectInfoResponse:
      return std::make_unique<ObjectInfoResponse>(sr, _version, ObjectInfoResponsePassKey());

    case ResponseTypes::pingResponse:
      return std::make_unique<PingResponse>(sr, _version, PingResponsePassKey());

    case ResponseTypes::readRequestResponse:
      return std::make_unique<ReadRequestResponse>(sr, _version, ReadRequestResponsePassKey());

    case ResponseTypes::writeRequestResponse:
      return std::make_unique<WriteRequestResponse>(sr, _version, WriteRequestResponsePassKey());
  }

  throw std::logic_error("ResponseBase::FromBinary: Internal error (no create method)");
}

/**
 * \brief Returns the size of the output of [ToBinary()](@ref gpcc::cood::ResponseBase::ToBinary).
 *
 * This method is intended to be used to determine the exact amount of memory required for invocation of
 * [ToBinary()](@ref gpcc::cood::ResponseBase::ToBinary) in advance.
 *
 * Hint for sub-classing:\n
 * Derived classes shall invoke the GetBinarySize()-method of their base class (e.g. this class) and return the sum of
 * the base class' return value and their own binary size.
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
size_t ResponseBase::GetBinarySize(void) const
{
  return baseBinarySize + (returnStack.size() * ReturnStackItem::binarySize);
}

/**
 * \brief Writes a binary representation of the object into a stream, which can be deserialized into an appropriate
 *        object via [FromBinary()](@ref gpcc::cood::ResponseBase::FromBinary).
 *
 * The counterpart of this is [FromBinary()](@ref gpcc::cood::ResponseBase::FromBinary).
 *
 * [GetBinarySize()](@ref gpcc::cood::ResponseBase::GetBinarySize) may be used to determine the size of the written
 * binary data in advance.
 *
 * Hint for sub-classing:\n
 * Derived classes shall first invoke the ToBinary()-method of their base class (e.g. this class) and then append their
 * own binary to the stream.
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
void ResponseBase::ToBinary(gpcc::stream::IStreamWriter & sw) const
{
  // to be read by FromBinary()
  sw.Write_uint8(version);
  sw.Write_uint8(static_cast<uint8_t>(type));

  // to be read by CTOR
  sw.Write_uint8(returnStack.size());
  for (auto const & e: returnStack)
    e.ToBinary(sw);
}

/**
 * \brief Sets the stack of @ref ReturnStackItem objects.
 *
 * \pre   The current stack of @ref ReturnStackItem objects is empty.
 *
 * \post  The provided items can be popped from the stack of this response object.
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
 *
 * - - -
 *
 * \param rs
 * The content of the referenced vector will be moved into the response object's stack of @ref ReturnStackItem objects.\n
 * The referenced vector will be empty afterwards.
 */
void ResponseBase::SetReturnStack(std::vector<ReturnStackItem> && rs)
{
  if (!returnStack.empty())
    throw std::logic_error("ResponseBase::SetReturnStack: Stack not empty");

  if (rs.size() > std::numeric_limits<uint8_t>::max())
    throw std::invalid_argument("ResponseBase::SetReturnStack: Two many items in 'rs'");

  returnStack = std::move(rs);
  rs.clear();
}

/**
 * \brief Queries if the stack of @ref ReturnStackItem objects is empty.
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
 * \retval true   Stack is empty.
 * \retval false  Stack is not empty.
 */
bool ResponseBase::IsReturnStackEmpty(void) const noexcept
{
  return returnStack.empty();
}

/**
 * \brief Pops one @ref ReturnStackItem from the stack of @ref ReturnStackItem items.
 *
 * \pre   The stack of @ref ReturnStackItem items contains at least one item.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * This will not throw `std::bad_alloc'.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * The @ref ReturnStackItem object popped from the response's stack of @ref ReturnStackItem objects.
 */
ReturnStackItem ResponseBase::PopReturnStack(void)
{
  if (returnStack.empty())
    throw std::runtime_error("ResponseBase::PopReturnStack: Stack empty");

  ReturnStackItem rsi = returnStack.back();
  returnStack.pop_back();
  return rsi;
}

/**
 * \brief Retrieves the type of response.
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
 * Type of response.
 */
ResponseBase::ResponseTypes ResponseBase::GetType(void) const noexcept
{
  return type;
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
 * \param _type
 * Type of response.
 */
ResponseBase::ResponseBase(ResponseTypes const _type)
: type(_type)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, returnStack()
{
}

/**
 * \brief Constructor. Creates a @ref ResponseBase object from data read from an
 *        [IStreamReader](@ref gpcc::stream::IStreamReader) containing a serialized @ref ResponseBase object.
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
 * \param _type
 * Type of request.
 *
 * \param sr
 * Stream from which the data shall be read.
 *
 * \param versionOnHand
 * Version of serialized object read from `sr`.
 */
ResponseBase::ResponseBase(ResponseTypes const _type, gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand)
: type(_type)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, returnStack()
{
  if (versionOnHand != version)
    throw std::runtime_error("ResponseBase::ResponseBase: Version not supported");

  uint_fast8_t n = sr.Read_uint8();
  returnStack.reserve(n);
  while (n != 0U)
  {
    returnStack.emplace_back(sr);
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
 * @ref ResponseBase object that shall be copied.
 */
ResponseBase::ResponseBase(ResponseBase const & other)
: type(other.type)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, returnStack(other.returnStack)
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
 * @ref ResponseBase object that shall be moved into the new constructed object.\n
 * Afterwards, the stack of @ref ReturnStackItem objects of `other` will be empty.
 */
ResponseBase::ResponseBase(ResponseBase && other) noexcept
: type(other.type)
, pPrevInIntrusiveDList(nullptr)
, pNextInIntrusiveDList(nullptr)
, returnStack(std::move(other.returnStack))
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
 * @ref ResponseBase object whose content shall be copy-assigned to this object.\n
 * The types (see @ref ResponseTypes) of the two objects must be equal.
 */
ResponseBase& ResponseBase::operator=(ResponseBase const & rhv)
{
  if (&rhv != this)
  {
    if (type != rhv.type)
      throw std::logic_error("ResponseBase::operator=(&): Different types");

    // not sure if copy-assignment provides the strong guarantee, so we use two steps here...
    auto copyOfReturnStack = rhv.returnStack;
    returnStack = std::move(copyOfReturnStack);
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
 * @ref ResponseBase object whose content shall be move-assigned to this object.\n
 * The types (see @ref ResponseTypes) of the two objects must be equal.\n
 * Afterwards, the stack of @ref ReturnStackItem objects of `rhv` will be empty.
 */
ResponseBase& ResponseBase::operator=(ResponseBase && rhv)
{
  if (&rhv != this)
  {
    if (type != rhv.type)
      throw std::logic_error("ResponseBase::operator=(&&): Different types");

    returnStack = std::move(rhv.returnStack);
    rhv.returnStack.clear();
  }

  return *this;
}

/**
 * \brief Safely converts an uint8_t value into a @ref ResponseTypes enum value.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::runtime_error   `value` is not a valid @ref ResponseTypes enum value.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param value
 * uint8_t value that shall be converted into a @ref ResponseTypes enum value.\n
 * This method handles invalid values gracefully.
 *
 * \return
 * Value from the @ref ResponseTypes enumeration that corresponds to `value`.
 */
ResponseBase::ResponseTypes ResponseBase::ToResponseType(uint8_t const value)
{
  switch (value)
  {
    case static_cast<uint8_t>(ResponseTypes::objectEnumResponse):
      return ResponseTypes::objectEnumResponse;

    case static_cast<uint8_t>(ResponseTypes::objectInfoResponse):
      return ResponseTypes::objectInfoResponse;

    case static_cast<uint8_t>(ResponseTypes::pingResponse):
      return ResponseTypes::pingResponse;

    case static_cast<uint8_t>(ResponseTypes::readRequestResponse):
      return ResponseTypes::readRequestResponse;

    case static_cast<uint8_t>(ResponseTypes::writeRequestResponse):
      return ResponseTypes::writeRequestResponse;

    default:
      throw std::runtime_error("ResponseBase::ToResponseType: 'value' is not a valid ResponseTypes enum value");
  }
}

} // namespace cood
} // namespace gpcc
