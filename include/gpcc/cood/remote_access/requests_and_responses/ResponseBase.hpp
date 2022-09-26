/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef RESPONSEBASE_HPP_202006282132
#define RESPONSEBASE_HPP_202006282132

#include <gpcc/cood/remote_access/requests_and_responses/ReturnStackItem.hpp>
#include <gpcc/container/IntrusiveDList.hpp>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc   {
namespace stream {
  class IStreamReader;
  class IStreamWriter;
}
}

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_REQRESP
 * \brief Base class for all classes implementing responses for object dictionary remote access requests.
 *
 * # Purpose
 * This class is the counterpart of class @ref RequestBase. It is the base class for a set of classes implementing
 * different types of responses that are generated by a @ref RemoteAccessServer instance when a remote access request
 * (subclass of @ref RequestBase) is processed.
 *
 * # Internals
 * ## Stack of ReturnStackItem objects
 * For routing remote access responses from the @ref RemoteAccessServer back to the client, remote access requests and
 * remote access responses have a stack of @ref ReturnStackItem objects. The stack is moved from the request object to
 * the response object when the request is processed and the response object is created.
 *
 * Classes transporting remote access requests from the client to the server can push @ref ReturnStackItem objects on
 * the stack of the request object. Later when the response shall be routed back to the originator of the request,
 * the @ref ReturnStackItem objects can be popped from the stack of the response object.
 *
 * ## Serialization and deserialization
 * Instances of this class can be serialized into an [IStreamWriter](@ref gpcc::stream::IStreamWriter) via
 * @ref ToBinary(). The size of the serialized data can be determined in advance via @ref GetBinarySize().
 *
 * A remote access response object serialized via @ref ToBinary() can be deserialized via @ref FromBinary().
 * @ref FromBinary() creates an instance of a subclass of class @ref ResponseBase and initializes it with data from
 * the stream. The type and the content of any deserialized object will be equal to the type and content of the
 * original object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class ResponseBase
{
  friend class gpcc::container::IntrusiveDList<ResponseBase>;

  public:
    /// Remote access response types.
    enum class ResponseTypes : uint8_t
    {
      objectEnumResponse   = 0U, ///<Response to object enumeration request.
      objectInfoResponse   = 1U, ///<Response to object info request.
      pingResponse         = 2U, ///<Response to ping request.
      readRequestResponse  = 3U, ///<Response to read request.
      writeRequestResponse = 4U  ///<Response to write request.
    };


    /// Minimum useful value for the _maximum permitted size_ of a serialized response (inclusive any
    /// @ref ReturnStackItem objects) passed to @ref IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed().
    /** This is also the minimum value of parameter `maxResponseSize` of
        @ref IRemoteObjectDictionaryAccessNotifiable::OnReady(). \n
        See @ref GPCC_COOD_REMOTEACCESS_ITF, chapter "Maximum request/response size" for details. */
    static size_t const minimumUsefulResponseSize = 32U;

    /// Maximum permitted size for a serialized response (inclusive any @ref ReturnStackItem objects)
    /// passed to @ref IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed().
    /** This is also the maximum value of parameter `maxResponseSize` of
        @ref IRemoteObjectDictionaryAccessNotifiable::OnReady(). \n
        See @ref GPCC_COOD_REMOTEACCESS_ITF, chapter "Maximum request/response size" for details. */
    static size_t const maxResponseSize = std::numeric_limits<uint32_t>::max();


    ResponseBase(void) = delete;
    virtual ~ResponseBase(void);

    // serialization/deserialization
    static std::unique_ptr<ResponseBase> FromBinary(gpcc::stream::IStreamReader & sr);
    virtual size_t GetBinarySize(void) const;
    virtual void ToBinary(gpcc::stream::IStreamWriter & sw) const;

    // return stack
    void SetReturnStack(std::vector<ReturnStackItem> && rs);
    bool IsReturnStackEmpty(void) const noexcept;
    ReturnStackItem PopReturnStack(void);

    // miscellaneous
    ResponseTypes GetType(void) const noexcept;
    virtual std::string ToString(void) const = 0;

  protected:
    /// Latest version of binary data supported by this class and its sub-classes.
    /** @ref ToBinary() will generate a binary with this version.\n
        @ref FromBinary() will accept this version and potential older versions, too. */
    static uint8_t const version = 1U;

    /// Binary size of a serialized @ref ResponseBase object (excl. `returnStack` and derived class(es)).
    static size_t const baseBinarySize = 3U;


    /// Type of response. Indicates the type of sub-class.
    ResponseTypes const type;


    explicit ResponseBase(ResponseTypes const _type);
    ResponseBase(ResponseTypes const _type, gpcc::stream::IStreamReader & sr, uint8_t const versionOnHand);

    ResponseBase(ResponseBase const & other);
    ResponseBase(ResponseBase && other) noexcept;

    ResponseBase& operator=(ResponseBase const & rhv);
    ResponseBase& operator=(ResponseBase && rhv);

  private:
    /// Prev-pointer used to enqueue instances of this class in a IntrusiveDList<ResponseBase>.
    ResponseBase* pPrevInIntrusiveDList;

    /// Next-pointer used to enqueue instances of this class in a IntrusiveDList<ResponseBase>.
    ResponseBase* pNextInIntrusiveDList;

    /// Stack of information required to route the response back to the originator of the request.
    std::vector<ReturnStackItem> returnStack;


    static ResponseTypes ToResponseType(uint8_t const value);
};

/**
 * \fn std::string ResponseBase::ToString(void) const
 * \brief Creates a human-readable text-representation of the remote access response object.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
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
 * \return
 * Human-readable text-representation of the remote access response object.\n
 * The output may be comprised of multiple lines separated by '\\n'.\n
 * There is no trailing '\\n'.
 */

} // namespace cood
} // namespace gpcc

#endif // RESPONSEBASE_HPP_202006282132
