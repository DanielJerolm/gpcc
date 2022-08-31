/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef MULTIPLEXER_HPP_202106212050
#define MULTIPLEXER_HPP_202106212050

#include "MultiplexerPort.hpp"
#include "gpcc/src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp"
#include <gpcc/osal/Mutex.hpp>
#include <memory>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace cood {

class IRemoteObjectDictionaryAccess;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Multiplexer. Connects to one @ref IRemoteObjectDictionaryAccess interface and provides multiple
 *        @ref IRemoteObjectDictionaryAccess interfaces to other clients.
 *
 * \htmlonly <style>div.image img[src="cood/RODA_Multiplexer.png"]{width:75%;}</style> \endhtmlonly
 * \image html "cood/RODA_Multiplexer.png" "Multiplexer"
 *
 * # Features
 * - __Requires__ one [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface and __provides__ up to 256
 *   [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess)/
 *   [RODAN](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable) interface pairs for up to 256 clients.\n
 *   (Without a multiplexer, one RODA interface allows to connect to one client only).
 * - The number of provided RODA interfaces is dynamic:
 *   - New RODA interfaces can be requested at any time.
 *   - RODA interfaces that are no longer required can be disposed at any time.
 * - Clients can be connected to and disconnected from a provided RODA interface at any time.
 * - The multiplexer is _100% transparent_ for clients:
 *   - Usage of sessions: A client connected to a provided RODA interface will not receive any responses adressed to a
 *     client that was formerly connected to that provided RODA interface.
 *   - A client does not take notice of other clients connected the multiplexer.
 *   - The ready/not-ready state of all provided RODA interface gracefully follows the state of the required RODA
 *     interface the multiplexer is connected to.
 *
 * # Usage
 * ## Setup
 * First instantiate the class.
 *
 * After instantiation:
 * - Use @ref Connect() to connect the multiplexer to a [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess)
 *   interface.
 * - Use @ref CreatePort() to create one or more @ref MultiplexerPort instances. Each @ref MultiplexerPort instance
 *   offers one [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface.
 * - Connect/disconnect clients to/from the [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interfaces provided
 *   by @ref MultiplexerPort instances.
 *
 * These steps can be accomplished at any time in any order.
 *
 * ## Teardown
 * Before destruction of a @ref Multiplexer instance, the following preconditions must be met:
 * - Unregister all the clients from all the @ref MultiplexerPort instances.\n
 *   @ref MultiplexerPort::IsClientRegistered() may be used to query if a client is registered at a port.
 * - Discard all shared_ptr instances referencing to @ref MultiplexerPort instances.
 * - Use @ref Disconnect() to disconnect the multiplexer instance from the
 *   [RODA](@ref gpcc::cood::IRemoteObjectDictionaryAccess) interface.
 *
 * These steps can be accomplished at any time in any order.
 *
 * # Internals
 *
 * \htmlonly <style>div.image img[src="cood/RODA_Multiplexer_internals.png"]{width:100%;}</style> \endhtmlonly
 * \image html "cood/RODA_Multiplexer_internals.png" "Multiplexer internals"
 *
 * __Structure__\n
 * Class @ref Multiplexer offers a required (client) RODA/RODAN interface pair for connection to a provided (server)
 * RODA/RODAN interface pair.
 *
 * For each RODA/RODAN interface pair provided by the multiplexer, class @ref Multiplexer comprises one instance of
 * class @ref MultiplexerPort. Each @ref MultiplexerPort instance manages one provided pair of RODA/RODAN interfaces.
 *
 * Class @ref Multiplexer has a `state` which tracks both if the multiplexer is connected to a RODA interface and
 * the state of that interface ('ready' and 'not ready').
 *
 * Each @ref MultiplexerPort instance has a `state` which tracks if a client is connected to the provided RODA/RODAN
 * interface pair and the state of the provided RODA interface.
 *
 * __Mutexes__\n
 * Class @ref Multiplexer comprises three mutexes which are shared among class @ref Multiplexer and the
 * @ref MultiplexerPort instances.
 *
 * Most attributes of class @ref Multiplexer and class @ref MultiplexerPort require both the `muxMutex` and the
 * `portMutex` for write access, while one of the two mutexes is sufficient for read access.
 *
 * During calls to the RODAN interface provided by class @ref Multiplexer (bottom right of multiplexer shown in figure
 * above) `muxMutex` will always be locked. If class @ref Multiplexer needs to fiddle in the guts of a
 * @ref MultiplexerPort instance, then it may additionally lock `portMutex`. Most calls to the multiplexer's RODAN
 * interface will be forwarded to the RODAN interface required by a @ref MultiplexerPort. During calls to a client, the
 * `muxMutex` is always locked.
 *
 * During calls of a client to the Register() and Unregister() methods of the RODA interface provided by a
 * @ref MultiplexerPort (top left of multiplexer shown in figure above), both `muxMutex` and `portMutex` will be locked.
 * This allows the @ref MultiplexerPort to change its state and maybe send a [PingRequest](@ref gpcc::cood::PingRequest)
 * via the RODA interface required by the multiplexer (top right in figure above).
 *
 * During calls of a client to the Send() and RequestExecutionContext() methods of the RODA interface provided by a
 * @ref MultiplexerPort (top left in figure above) only `portMutex` will be locked. This allows the @ref MultiplexerPort
 * to read all its guts plus the guts of the @ref Multiplexer and to delegate the calls to the RODA interface required
 * by the @ref Multiplexer (top right in fiugre above). At the same time, calls made in the context of the RODAN
 * interface (bottom left) to the RODA interface (top left) are dead-lock free.
 *
 * __Session ID__\n
 * Each @ref MultiplexerPort uses a session ID to distinguish "old" responses in case a client is unregistered and
 * the just unregistered client or a different one is registered. The session ID is embedded in a @ref ReturnStackItem
 * object and attached to each forwarded RODA request.
 *
 * The session ID is incremented each time a client registers. Session IDs may wear out. To prevent wear-out, class
 * @ref MultiplexerPort will send a ping to the server when a client is registered. If the ping is later received, the
 * @ref MultiplexerPort is sure that the connection to the RODA server is flushed and it will refresh its session IDs.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Multiplexer final : private IRemoteObjectDictionaryAccessNotifiable
{
  friend class MultiplexerPort;

  public:
    /// Maximum number of exposed ports.
    static size_t constexpr maxNbOfPorts = 256U;


    Multiplexer(void);
    Multiplexer(Multiplexer const &) = delete;
    Multiplexer(Multiplexer &&) = delete;
    ~Multiplexer(void) override;

    Multiplexer& operator=(Multiplexer const &) = delete;
    Multiplexer& operator=(Multiplexer &&) = delete;


    void Connect(IRemoteObjectDictionaryAccess & roda);
    void Disconnect(void) noexcept;

    std::shared_ptr<MultiplexerPort> CreatePort(void);

  private:
    /// Enumeration of states of the multiplexer.
    enum class States
    {
      notConnected,   ///<Multiplexer is not connected to a RODA interface.
      disconnecting,  ///<@ref Disconnect() is in process.
      notReady,       ///<Multiplexer is connected to a RODA interface, but the RODA interface is not ready.
      ready           ///<Multiplexer is connected to a RODA interface and the RODA interface is ready.
    };


    /// Owner ID used to tag requests and to check responses.
    uint32_t ownerID;

    /// Mutex used to protect @ref Connect() and @ref Disconnect() against each other.
    /** Locking order: @ref connectMutex -> @ref muxMutex -> @ref portMutex */
    gpcc::osal::Mutex connectMutex;

    /// Mutex used to make the multiplexer and its ports thread-safe. This is intended to be locked by the multiplexer.
    /** Locking order: @ref connectMutex -> @ref muxMutex -> @ref portMutex */
    gpcc::osal::Mutex muxMutex;

    /// Mutex used to make the multiplexer and its ports thread-safe. This is intended to be locked by ports.
    /** Locking order: @ref connectMutex -> @ref muxMutex -> @ref portMutex */
    gpcc::osal::Mutex portMutex;


    /// Current state of the multiplexer.
    /** RD: @ref muxMutex OR @ref portMutex is required.\n
        WR: @ref muxMutex AND @ref portMutex are both required. */
    States state;

    /// [RODA](@ref IRemoteObjectDictionaryAccess) interface the multiplexer is connected to. nullptr = none.
    /** RD: @ref muxMutex OR @ref portMutex is required.\n
        WR: @ref muxMutex AND @ref portMutex are both required. */
    IRemoteObjectDictionaryAccess* pRODA;

    /// Maximum request size a client connected to a port of the multiplexer is allowed to transmit.
    /** RD: @ref muxMutex OR @ref portMutex is required.\n
        WR: @ref muxMutex AND @ref portMutex are both required.\n
        This is only valid if @ref state is @ref States::ready. \n
        This is set by @ref OnReady(). The size for a @ref ReturnStackItem is already subtracted.\n
        This may be zero in case of request size starvation. */
    size_t maxRequestSize;

    /// Maximum response size a client connected to a port of the @ref Multiplexer can receive.
    /** RD: @ref muxMutex OR @ref portMutex is required.\n
        WR: @ref muxMutex AND @ref portMutex are both required.\n
        This is only valid if @ref state is @ref States::ready. \n
        This is set by @ref OnReady(). The size for a @ref ReturnStackItem is already subtracted.\n
        This may be zero in case of response size starvation. */
    size_t maxResponseSize;

    /// Multiplexer's ports.
    /** RD: @ref muxMutex OR @ref portMutex is required.\n
        WR: @ref muxMutex AND @ref portMutex are both required. */
    std::vector<std::shared_ptr<MultiplexerPort>> ports;


    // <-- IRemoteObjectDictionaryAccessNotifiable
    void OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept override;
    void OnDisconnected(void) noexcept override;
    void OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept override;

    void LoanExecutionContext(void) noexcept override;
    // --> IRemoteObjectDictionaryAccessNotifiable
};

} // namespace cood
} // namespace gpcc

#endif // MULTIPLEXER_HPP_202106212050
