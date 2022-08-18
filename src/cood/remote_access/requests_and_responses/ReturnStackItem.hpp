/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef RETURNSTACKITEM_HPP_202007140959
#define RETURNSTACKITEM_HPP_202007140959

#include <cstddef>
#include <cstdint>

namespace gpcc   {
namespace Stream {
  class IStreamReader;
  class IStreamWriter;
}
}

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Container for information used to route a remote access response from the server back to the originator of
 *        the remote access request.
 *
 * A stack of @ref ReturnStackItem objects is attached to each remote access request (class @ref RequestBase) and remote
 * access response (class @ref ResponseBase). When a request is executed, then the stack will be moved from the request
 * object to the response object.
 *
 * Whenever a class forwards a remote access request towards the server it will push an instance of this class on the
 * stack of the remote access request object. When the response is returned later, then the class will pop the
 * @ref ReturnStackItem object from the stack of the response and use the included information to route the response
 * back to the originator of the request.
 *
 * A @ref ReturnStackItem is comprised of two pieces of information:
 * - __A 32bit ID identifying the creator of the return stack item.__\n
 *   This shall be used to verify the origin of a @ref ReturnStackItem when popping it from a remote access response
 *   object. The implementation of the creator of the @ref ReturnStackItem is free to select a suitable method to verify
 *   the origin. There is no method prescribed.
 * - __A 32bit info value.__\n
 *   This is specific to the creator of the item. The information contained in this value could be used to:
 *   - Route a response to the proper provided RODA/RODAN interface pair (typical use: multiplexer)
 *   - Assign requests and responses to sessions (typical use: proxies providing a connection across IPC, a network,
 *     or a serial link; multiplexer)
 *   - Identify a response associated with a request (typical use: client)
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ReturnStackItem final
{
  public:
    /// Size of a serialized @ref ReturnStackItem object in byte.
    static size_t const binarySize = 8U;


    ReturnStackItem(void) = delete;
    ReturnStackItem(uint32_t const _id, uint32_t const _info) noexcept;
    explicit ReturnStackItem(gpcc::Stream::IStreamReader & sr);

    ReturnStackItem(ReturnStackItem const &) = default;
    ReturnStackItem(ReturnStackItem &&) = default;

    ~ReturnStackItem(void) = default;

    ReturnStackItem& operator=(ReturnStackItem const &) noexcept = default;
    ReturnStackItem& operator=(ReturnStackItem &&) noexcept = default;

    bool operator==(ReturnStackItem const & rhv) const noexcept;
    bool operator!=(ReturnStackItem const & rhv) const noexcept;

    void ToBinary(gpcc::Stream::IStreamWriter & sw) const;

    uint32_t GetID(void) const noexcept;
    uint32_t GetInfo(void) const noexcept;

  private:
    /// ID of the unit that created this @ref ReturnStackItem instance.
    /** This is intended to verify the origin of a @ref ReturnStackItem when popping them from a remote access
        response object. */
    uint32_t id;

    /// Routing info.
    /** The meaning of the bits depends on the class that created the @ref ReturnStackItem object. */
    uint32_t info;
};

/**
 * \brief Compare for equal operator.
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
 * \param rhv
 * Unmodifiable reference to the @ref ReturnStackItem on the right side of the ==-operator.
 *
 * \retval true   Both objects are equal.
 * \retval false  The objects are not equal.
 */
inline bool ReturnStackItem::operator==(ReturnStackItem const & rhv) const noexcept
{
  return ((id == rhv.id) && (info == rhv.info));
}

/**
 * \brief Compare for not-equal operator.
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
 * \param rhv
 * Unmodifiable reference to the @ref ReturnStackItem on the right side of the !=-operator.
 *
 * \retval true   The objects are not equal.
 * \retval false  Both objects are equal.
 */
inline bool ReturnStackItem::operator!=(ReturnStackItem const & rhv) const noexcept
{
  return ((id != rhv.id) || (info != rhv.info));
}

/**
 * \brief Retrieves the encapsulated ID.
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
 * The encapsulated ID-value.
 */
inline uint32_t ReturnStackItem::GetID(void) const noexcept
{
  return id;
}

/**
 * \brief Retrieves the encapsulated info-value.
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
 * The encapsulated info-value.
 */
inline uint32_t ReturnStackItem::GetInfo(void) const noexcept
{
  return info;
}

} // namespace cood
} // namespace gpcc

#endif // RETURNSTACKITEM_HPP_202007140959
