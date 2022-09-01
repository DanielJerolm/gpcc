/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef IREMOTEOBJECTDICTIONARYACCESSNOTIFIABLE_HPP_202007071233
#define IREMOTEOBJECTDICTIONARYACCESSNOTIFIABLE_HPP_202007071233

#include <cstddef>
#include <memory>

namespace gpcc {
namespace cood {

class ResponseBase;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_ITF
 * \brief Notifiable interface that shall be implemented by users of the @ref IRemoteObjectDictionaryAccess interface
 *        in order to receive notifications and responses.
 *
 * This is used in conjunction with the @ref IRemoteObjectDictionaryAccess interface.\n
 * The pair of the two interfaces is abbreviated using the term RODA/RODAN.\n
 * For details please refer to @ref GPCC_COOD_REMOTEACCESS_ITF.
 *
 * - - -
 *
 * __Thread safety:__\n
 * All methods required by this notifiable interface will be invoked by one thread only. No more than one method is
 * invoked at any time.
 */
class IRemoteObjectDictionaryAccessNotifiable
{
  public:
    virtual void OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept = 0;
    virtual void OnDisconnected(void) noexcept = 0;
    virtual void OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept = 0;

    virtual void LoanExecutionContext(void) noexcept = 0;

  protected:
    IRemoteObjectDictionaryAccessNotifiable(void) = default;
    IRemoteObjectDictionaryAccessNotifiable(IRemoteObjectDictionaryAccessNotifiable const &) = default;
    IRemoteObjectDictionaryAccessNotifiable(IRemoteObjectDictionaryAccessNotifiable &&) noexcept = default;

    virtual ~IRemoteObjectDictionaryAccessNotifiable(void) = default;

    IRemoteObjectDictionaryAccessNotifiable& operator=(IRemoteObjectDictionaryAccessNotifiable const &) = default;
    IRemoteObjectDictionaryAccessNotifiable& operator=(IRemoteObjectDictionaryAccessNotifiable &&) noexcept = default;
};

/**
 * \fn IRemoteObjectDictionaryAccessNotifiable::OnReady
 * \brief Indicates, that the [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)
 *        interface is ready to accept remote access requests and informs about maximum permitted message sizes.
 *
 * \post  [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send) will
 *        accept remote access requests.
 *
 * \post  [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *        will accept calls.
 *
 * \note  The post-conditions are valid upon entry into this method.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be implemented thread-safe.\n
 * All methods required by this notifiable interface will be invoked by one thread only. No more than one method is
 * invoked at any time.
 *
 * Dead-lock free invocation of the following interfaces/methods is possible from within this method:
 * - [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * - [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *
 * __Exception safety:__\n
 * No-throw guarantee required.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred cancellation disabled.
 *
 * - - -
 *
 * \param maxRequestSize
 * Maximum size (in byte) of a request that can be processed by this RODA/RODAN interface pair.\n
 * The value refers to a serialized request object inclusive any [ReturnStackItem](@ref gpcc::cood::ReturnStackItem)
 * objects contained in the request.\n
 * Minimum value: [RequestBase::minimumUsefulRequestSize](@ref gpcc::cood::RequestBase::minimumUsefulRequestSize)\n
 * Maximum value: [RequestBase::maxRequestSize](@ref gpcc::cood::RequestBase::maxRequestSize)\n
 * __Special values:__\n
 * _Zero_: The structure of the link to the remote access server does not allow to transmit at least a request with
 * the minimum useful size ([RequestBase::minimumUsefulRequestSize](@ref gpcc::cood::RequestBase::minimumUsefulRequestSize)).\n
 *
 * \param maxResponseSize
 * Maximum size (in byte) of a response object, the client could receive from this RODA/RODAN interface pair.\n
 * The value refers to a serialized response object inclusive any [ReturnStackItem](@ref gpcc::cood::ReturnStackItem)
 * objects potentially contained in the response.\n
 * Minimum value: [ResponseBase::minimumUsefulResponseSize](@ref gpcc::cood::ResponseBase::minimumUsefulResponseSize)\n
 * Maximum value: [ResponseBase::maxResponseSize](@ref gpcc::cood::ResponseBase::maxResponseSize)\n
 * __Special values:__\n
 * _Zero_: The structure of the link to the remote access server does not allow to transmit at least a response with
 * the minimum useful size ([ResponseBase::minimumUsefulResponseSize]
 * (@ref gpcc::cood::ResponseBase::minimumUsefulResponseSize)) from the server to the client.\n
 */

/**
 * \fn IRemoteObjectDictionaryAccessNotifiable::OnDisconnected
 * \brief Indicates, that the [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)
 *        interface has been disconnected from the remote access server.
 *
 * The disconnected-state is not final. The interface may become operational again. See
 * [OnReady()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnReady) for details.
 *
 * \post  [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send) will
 *        not accept remote access requests any more.
 *
 * \post  Any pending responses that have not yet been delivered to
 *        [OnRequestProcessed()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed) will
 *        be dropped and will not be delivered, even if the
 *        [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface becomes operational
 *        again.
 *
 * \post  Open requests send to the remote access server may or may not be executed by the remote access server. If a
 *        request is executed, then its response will be discarded in any case, even if the
 *        [IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface becomes operational
 *        again.
 *
 * \post  [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *        will not accept any calls any more.
 *
 * \post  Any pending request for a call to [LoanExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext)
 *        will be dropped.
 *
 * \note  In most cases this notification is delivered _after_ the interface is not operational any more. Therefore
 *        [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send) and
 *        [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *        may not accept calls any more _even before_ this is invoked.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be implemented thread-safe.\n
 * All methods required by this notifiable interface will be invoked by one thread only. No more than one method is
 * invoked at any time.
 *
 * Dead-lock free invocation of the following interfaces/methods is possible from within this method:
 * - [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * - [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *
 * __Exception safety:__\n
 * No-throw guarantee required.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred cancellation disabled.
 */

/**
 * \fn IRemoteObjectDictionaryAccessNotifiable::OnRequestProcessed
 * \brief Passes a response to the client.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be implemented thread-safe.\n
 * All methods required by this notifiable interface will be invoked by one thread only. No more than one method is
 * invoked at any time.
 *
 * Dead-lock free invocation of the following interfaces/methods is possible from within this method:
 * - [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * - [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *
 * __Exception safety:__\n
 * No-throw guarantee required.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred cancellation disabled.
 *
 * - - -
 *
 * \param spResponse
 * Remote access response object.\n
 * Ownership moves to the called method.
 */

/**
 * \fn IRemoteObjectDictionaryAccessNotifiable::LoanExecutionContext
 * \brief This is invoked upon request via
 *        [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext).
 *
 * - - -
 *
 * __Thread safety:__\n
 * This shall be implemented thread-safe.\n
 * All methods required by this notifiable interface will be invoked by one thread only. No more than one method is
 * invoked at any time.
 *
 * Dead-lock free invocation of the following interfaces/methods is possible from within this method:
 * - [IRemoteObjectDictionaryAccess::Send()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::Send)
 * - [IRemoteObjectDictionaryAccess::RequestExecutionContext()](@ref gpcc::cood::IRemoteObjectDictionaryAccess::RequestExecutionContext)
 *
 * __Exception safety:__\n
 * No-throw guarantee required.
 *
 * __Thread cancellation safety:__\n
 * This will be invoked with deferred cancellation disabled.
 */

} // namespace cood
} // namespace gpcc

#endif // IREMOTEOBJECTDICTIONARYACCESSNOTIFIABLE_HPP_202007071233
