/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef MULTIPLEXERPORT_HPP_202106212102
#define MULTIPLEXERPORT_HPP_202106212102

#include "gpcc/src/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp"
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace cood {

class IRemoteObjectDictionaryAccessNotifiable;
class Multiplexer;

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief One port of a @ref Multiplexer providing one @ref IRemoteObjectDictionaryAccess interface.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class MultiplexerPort final : public IRemoteObjectDictionaryAccess
{
  friend class Multiplexer;

  public:
    MultiplexerPort(void) = delete;
    MultiplexerPort(Multiplexer & _owner, uint8_t const _index) noexcept;
    MultiplexerPort(MultiplexerPort const &) = delete;
    MultiplexerPort(MultiplexerPort &&) = delete;
    ~MultiplexerPort(void) override;

    MultiplexerPort& operator=(MultiplexerPort const &) = delete;
    MultiplexerPort& operator=(MultiplexerPort &&) = delete;

    bool IsClientRegistered(void) const noexcept;

  private:
    /// Enumeration of states of the @ref MultiplexerPort.
    enum class States
    {
      noClientRegistered,  ///<No client is registered at the provided RODA interface.
      notReady,            ///<A client is registered at the provided RODA interface, but the interface is in state "not ready".
      ready                ///<A client is registered at the provided RODA interface, and the interface is in state "ready".
    };


    /// @ref Multiplexer instance this @ref MultiplexerPort belongs to.
    Multiplexer & owner;

    /// Index of this port in `owner.ports`.
    uint8_t const index;


    /// State of this port instance.
    /** RD: `owner.portMutex` OR `owner.muxMutex` is required.\n
        WR: `owner.portMutex` AND `owner.muxMutex` are both required. */
    States state;

    /// [RODAN](@ref IRemoteObjectDictionaryAccessNotifiable) interface registered at this port. nullptr = none.
    /** RD: `owner.portMutex` OR `owner.muxMutex` is required.\n
        WR: `owner.portMutex` AND `owner.muxMutex` are both required. */
    IRemoteObjectDictionaryAccessNotifiable* pRODAN;


    /// Current Session ID.
    /** RD: `owner.portMutex` OR `owner.muxMutex` is required.\n
        WR: `owner.portMutex` AND `owner.muxMutex` are both required. */
    uint8_t sessionID;

    /// Oldest used session ID for which messages might be existing somewhere.
    /** RD: `owner.portMutex` OR `owner.muxMutex` is required.\n
        WR: `owner.portMutex` AND `owner.muxMutex` are both required.\n
        If @ref sessionID shall be incremented, then the new value of @ref sessionID must not equal this. */
    uint8_t oldestUsedSessionID;

    /// Indicates if a message has been forwarded using @ref sessionID.
    /** `owner.portMutex` is required. */
    bool sessionIDUsed;


    /// Flag indicating if the registered client has requested a call to his `LoanExecutionContext()` method.
    /** `owner.portMutex` is required. */
    bool execContextRequested;


    // <-- IRemoteObjectDictionaryAccess
    void Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable) override;
    void Unregister(void) noexcept override;
    void Send(std::unique_ptr<RequestBase> & spReq) override;

    void RequestExecutionContext(void) override;
    // --> IRemoteObjectDictionaryAccess
};

} // namespace cood
} // namespace gpcc

#endif // MULTIPLEXERPORT_HPP_202106212102
