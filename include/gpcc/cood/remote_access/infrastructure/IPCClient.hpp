/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#ifndef IPCCLIENT_HPP_202102101607


#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Thread.hpp>
#include <string>

// forward declarations =======================================================
namespace gpcc {
namespace log  {
  class ILogFacility;
}}


// class definition ===========================================================

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Client providing access to a RODA interface via IPC.
 *
 * This is the counterpart of @ref IPCServer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IPCClient final : public IRemoteObjectDictionaryAccess
{
  public:
    IPCClient(void) = delete;
    IPCClient(std::string const & name,
              gpcc::log::ILogFacility & logFacility);
    IPCClient(IPCClient const &) = delete;
    IPCClient(IPCClient &&) = delete;
    ~IPCClient(void);

    IPCClient& operator=(IPCClient const &) = delete;
    IPCClient& operator=(IPCClient &&) = delete;

    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

  private:
    /// Logger used to log messages.
    gpcc::log::Logger logger;

    /// Thread used as execution context.
    gpcc::osal::Thread thread;

    /// Mutex protecting @ref Start() and @ref Stop().
    gpcc::osal::Mutex startStopMutex;

    /// Flag indicating if the component is running.
    /** @ref startStopMutex required. */
    bool running;


    void* ThreadEntry(void);

    // <-- IRemoteObjectDictionaryAccess
    void Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable) override;
    void Unregister(void) noexcept override;
    void Send(std::unique_ptr<RequestBase> & spReq) override;

    void RequestExecutionContext(void) override;
    // --> IRemoteObjectDictionaryAccess
};

} // namespace cood
} // namespace gpcc

#endif // IPCCLIENT_HPP_202102101607
#endif // defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
