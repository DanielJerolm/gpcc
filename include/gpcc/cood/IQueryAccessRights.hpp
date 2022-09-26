/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef IQUERYACCESSRIGHTS_HPP_202110090919
#define IQUERYACCESSRIGHTS_HPP_202110090919

#include <gpcc/cood/Object.hpp>
#include <functional>

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD
 * \brief Interface for querying access rights that shall be applied when accessing an [Object](@ref gpcc::cood::Object)
 *        from an [ObjectDictionary](@ref gpcc::cood::ObjectDictionary).
 *
 * Further this interface offers locking and synchronization to allow access rights to change when there is no ongoing
 * access to an [Object](@ref gpcc::cood::Object). The other way round, the provided locking and synchronization ensures
 * that access rights cannot change while there is at least one ongoing access to an [Object](@ref gpcc::cood::Object).
 *
 * Typical clients of this interface are classes that access [objects](@ref Object) contained in an
 * [ObjectDictionary](@ref gpcc::cood::ObjectDictionary) directly. Some examples from GPCC:
 * - [ThreadBasedRemoteAccessServer](@ref gpcc::cood::ThreadBasedRemoteAccessServer)
 * - [WorkQueueBasedRemoteAccessServer](@ref gpcc::cood::WorkQueueBasedRemoteAccessServer)
 * - [CLIAdapterBase](@ref gpcc::cood::CLIAdapterBase)
 *
 * An instance of this interface may be used by multiple clients.
 *
 * Each client is intended to acquire no more than one lock. Multiple clients may have acquired the access rights at the
 * same time.
 *
 * Typical providers of this interface are classes that track or manage the internal state of a device. This allows
 * to apply different access rights that depend on the current state of the device. Examples for devices with an
 * internal state that determines the access rights for object dictionary access are:
 * - EtherCAT slave devices
 * - CANopen slave devices
 *
 * # Usage (client view)
 * A client who wants to access an [Object](@ref gpcc::cood::Object) shall accomplish the following steps:
 *
 * 1. Query the [Object](@ref gpcc::cood::Object) from the [ObjectDictionary](@ref gpcc::cood::ObjectDictionary).
 * 2. Acquire and query the access rights via @ref AcquireAccessRights(). \n
 *    _In case of `false` being returned:_\n
 *    Access rights could not be acquired, most likely because access rights are currently changing. Wait for the
 *    callback passed to @ref AcquireAccessRights() and retry this step.\n
 *    _In case of `true`:_\n
 *    Access rights have been acquired and queried. Continue at step 3.
 * 3. Read/Write the object using the access rights queried in step 2.
 * 4. Release the access rights via @ref ReleaseAccessRights().
 *
 * If the client got `false` in step 2, then he/she has to wait for the callback and retry step 2. If the client is no
 * longer interested in acquisition of the access rights, then he/she shall invoke @ref Abort() to ensure that the
 * callback will not be invoked.
 *
 * The client may access multiple subindices or even multiple objects while he/she has the access rights acquired
 * (locked). However the client shall release the access rights quickly. While the client has access rights acquired,
 * the internal state of the device that determines the access rights cannot change. Usually there are requirements on
 * the maximum allowed latency until when the internal device state must change due to an event or state change request.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IQueryAccessRights
{
  public:
    /**
     * \brief Type definition for a callback used to indicate that it is worth to retry calling
     *        [AcquireAccessRights()](@ref gpcc::cood::IQueryAccessRights::AcquireAccessRights).
     *
     * - - -
     *
     * __Thread safety requirements/hints:__\n
     * This will be invoked in an unspecified thread context.\n
     * This callback is dead-lock-free in conjunction with:
     * - [IQueryAccessRights::AcquireAccessRights()](@ref gpcc::cood::IQueryAccessRights::AcquireAccessRights())
     * - [IQueryAccessRights::ReleaseAccessRights()](@ref gpcc::cood::IQueryAccessRights::ReleaseAccessRights())
     *
     * This callback is explicitly __not__ dead-lock-free in conjunction with:
     * - [IQueryAccessRights::Abort()](@ref gpcc::cood::IQueryAccessRights::Abort())
     *
     * Other combinations are not dead-lock-free or have not yet been evaluated.
     *
     * __Exception safety requirements/hints:__\n
     * The referenced function/method shall provide the no-throw guarantee.\n
     * Any thrown exception will result in Panic() at the caller.
     *
     * __Thread cancellation safety requirements/hints:__\n
     * The referenced function/method shall not contain any cancellation point.
     */
    typedef std::function<void(void)> tOnUnlockedCallback;


    virtual bool AcquireAccessRights(Object::attr_t & rights, tOnUnlockedCallback const & cb) = 0;
    virtual void ReleaseAccessRights(void) = 0;
    virtual void Abort(tOnUnlockedCallback const & cb) = 0;

  protected:
    IQueryAccessRights(void) = default;
    IQueryAccessRights(IQueryAccessRights const &) = default;
    IQueryAccessRights(IQueryAccessRights &&) = default;
    virtual ~IQueryAccessRights(void) = default;

    IQueryAccessRights& operator=(IQueryAccessRights const &) = default;
    IQueryAccessRights& operator=(IQueryAccessRights &&) = default;
};

/**
 * \fn bool IQueryAccessRights::AcquireAccessRights(gpcc::cood::Object::attr_t & rights, tOnUnlockedCallback const & cb)
 * \brief Acquires and queries access rights for object dictionary access.
 *
 * The acquisition and query are performed as one atomic operation.
 *
 * \pre   The caller does not yet have access rights acquired.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * \throws std::bad_alloc   Out of memory. This is only possible, if access rights could not be acquired.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param rights
 * If access rights are acquired, then the queried access rights are written into the referenced variable.
 *
 * \param cb
 * If access rights could not be acquired, then the referenced callback function will be invoked when the
 * access rights become unlocked. Upon reception of the callback, it is worth to retry calling this method.\n
 * A copy will be created.\n
 * This is also used as a handle to identify the caller to handle a potential call to
 * [Abort()](@ref gpcc::cood::IQueryAccessRights::Abort).
 *
 * \retval true    Access rights were acquired and queried. There will be no callback.
 * \retval false   Access rights could not be acquired. The callback will be invoked when it is worth to retry
 *                 acquisition.
 */

/**
 * \fn void IQueryAccessRights::ReleaseAccessRights(void)
 * \brief Releases access rights that have previously been acquried via
 *        [AcquireAccessRights()](@ref gpcc::cood::IQueryAccessRights::AcquireAccessRights).
 *
 * \pre   The client has access rights acquired.
 *
 * \post  The client has no access rights acquired.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */

/**
 * \fn void IQueryAccessRights::Abort(tOnUnlockedCallback const & cb)
 * \brief Aborts delivery of a callback enqueued by
 *        [AcquireAccessRights()](@ref gpcc::cood::IQueryAccessRights::AcquireAccessRights).
 *
 * If the callback is enqueued, then it is discarded and will not be invoked when the access rights become unlocked.\n
 * If delivery of the callback is in process, then this blocks until the callback has been delivered.\n
 * If the callback is neither enqueued, nor delivery is in process, then this method has no effect.
 *
 * \post  The callback will not be invoked by guarantee after this method has returned.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param cb
 * Unmodifiable reference to the callback passed to
 * [AcquireAccessRights()](@ref gpcc::cood::IQueryAccessRights::AcquireAccessRights).
 */

} // namespace cood
} // namespace gpcc

#endif // IQUERYACCESSRIGHTS_HPP_202110090919
