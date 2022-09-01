/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#ifndef RODANOTIFIABLELISTENER_HPP_202008172059
#define RODANOTIFIABLELISTENER_HPP_202008172059

#include <gpcc/container/IntrusiveDList.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace gpcc {
namespace cood {
  class IRemoteObjectDictionaryAccess;
}

namespace log  {
  class Logger;
}
} // namespace gpcc

namespace gpcc_tests {
namespace cood       {

/**
 * \ingroup GPCC_TESTS_COOD_RA
 * \brief Listener for a RODAN interface
 *        ([IRemoteObjectDictionaryAccessNotifiable](@ref gpcc::cood::IRemoteObjectDictionaryAccessNotifiable)-interface).
 *
 * # Features
 * - Records number of calls to RODAN.
 * - Checks order of calls.
 * - Tracks the ready-state of the associated RODA-interface
 *   ([IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)).
 * - Stores responses received via the RODAN inferface.
 * - Blocks (with timeout) until reception of a response.
 * - Blocks (with timeout) until state is "ready".
 *
 * # Life cycle
 * 1. Instantiate
 * 2. @ref Register()
 * 3. Run tests on corresponding RODA-interface ([IRemoteObjectDictionaryAccess](@ref gpcc::cood::IRemoteObjectDictionaryAccess)).
 * 4. @ref Unregister()
 * 5. Optional: Go back to 2.
 * 6. At the end (or at any time), use @ref AnyError() to check if any error has been detected by the listener.
 * 7. Destroy.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class RODAN_Listener final : private gpcc::cood::IRemoteObjectDictionaryAccessNotifiable
{
  public:
    /// Duration of call to LoanExecutionContext() in ms.
    static uint8_t const loanExecContextDuration_ms = 10U;

    /// Listener's states.
    enum class States
    {
      unregistered, ///<Listener is not registered at a IRemoteObjectDictionaryAccess interface.
      notReady,     ///<Listener is registered, but IRemoteObjectDictionaryAccess interface is not ready.
      ready         ///<Listener is registered and IRemoteObjectDictionaryAccess interface is ready.
    };


    explicit RODAN_Listener(gpcc::log::Logger & _logger);
    RODAN_Listener(RODAN_Listener const &) = delete;
    RODAN_Listener(RODAN_Listener &&) = delete;
    ~RODAN_Listener(void);

    RODAN_Listener& operator=(RODAN_Listener const &) = delete;
    RODAN_Listener& operator=(RODAN_Listener &&) = delete;

    void Register(gpcc::cood::IRemoteObjectDictionaryAccess & roda);
    void Unregister(gpcc::cood::IRemoteObjectDictionaryAccess & roda);

    void SetOnLoanExecutionContext(std::function<void(void)> const & func);

    bool AnyError(void) const;
    States GetState(void) const;
    bool IsRegistered(void) const;
    bool WaitForStateReady(uint32_t const timeout_ms);

    uint32_t GetNbOfCallsOnReady(void) const;
    size_t   GetMaxRequestSize(void) const;
    size_t   GetMaxResponseSize(void) const;
    uint32_t GetNbOfCallsOnDisconnected(void) const;
    uint32_t GetNbOfCallsOnRequestProcessed(void) const;
    uint32_t GetNbOfCallsLoanExecutionContext(void) const;

    bool WaitForResponseAvailable(uint32_t const timeout_ms);
    size_t GetNbOfAvailableResponses(void) const;
    std::unique_ptr<gpcc::cood::ResponseBase> PopResponse(void);

  private:
    /// Logger used to log messages.
    gpcc::log::Logger & logger;

    /// Mutex used to make @ref Register() and @ref Unregister() thread-safe.
    /** Locking order: @ref regUnregMutex -> @ref apiMutex */
    gpcc::osal::Mutex regUnregMutex;

    /// Mutex used to make the API thread-safe.
    /** Locking order: @ref regUnregMutex -> @ref apiMutex */
    gpcc::osal::Mutex mutable apiMutex;

    /// Current state of the listener.
    /** @ref apiMutex required. */
    States state;

    /// Flag indicating if the listener detected any error yet.
    /** @ref apiMutex required. */
    bool anyError;


    /// Number of calls to OnReady().
    /** @ref apiMutex required. */
    uint32_t nbOfCallsOnReady;

    /// Latest "maxRequestSize" passed to OnReady()
    /** @ref apiMutex required.\n
        This is only valid, if @ref nbOfCallsOnReady is not zero. */
    size_t latestMaxRequestSize;

    /// Latest "maxResponseSize" passed to OnReady()
    /** @ref apiMutex required.\n
        This is only valid, if @ref nbOfCallsOnReady is not zero. */
    size_t latestMaxResponseSize;

    /// Number of calls to OnDisconnected().
    /** @ref apiMutex required. */
    uint32_t nbOfCallsOnDisconnected;

    /// Number of calls to OnRequestProcessed().
    /** @ref apiMutex required. */
    uint32_t nbOfCallsOnRequestProcessed;

    /// Number of calls to LoanExecutionContext().
    /** @ref apiMutex required. */
    uint32_t nbOfCallsLoanExecutionContext;

    /// CV used to signal when the listener's state changes to "ready".
    /** This shall be used in conjunction with @ref apiMutex */
    gpcc::osal::ConditionVariable stateReadyCV;

    /// CV used to signal when the response queue (@ref responses) is no longer empty.
    /** This shall be used in conjunction with @ref apiMutex */
    gpcc::osal::ConditionVariable respAvailCV;

    /// Queue for received responses.
    /** @ref apiMutex required. */
    gpcc::container::IntrusiveDList<gpcc::cood::ResponseBase> responses;

    /// Pointer to a user-defined function that shall be invoked by `LoanExecutionContext()`.
    /** @ref apiMutex required. */
    std::function<void(void)> onLoanExecutionContext;

    // <-- gpcc::cood::IRemoteObjectDictionaryAccessNotifiable
    void OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept override;
    void OnDisconnected(void) noexcept override;
    void OnRequestProcessed(std::unique_ptr<gpcc::cood::ResponseBase> spResponse) noexcept override;

    void LoanExecutionContext(void) noexcept override;
    // --> gpcc::cood::IRemoteObjectDictionaryAccessNotifiable
};

} // namespace cood
} // namespace gpcc_tests

#endif // RODANOTIFIABLELISTENER_HPP_202008172059
