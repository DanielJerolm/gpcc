/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#ifndef IPCSERVER_HPP_202102101606

#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
#include <gpcc/log/Logger.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Thread.hpp>
#include <string>

// forward declarations =======================================================
namespace gpcc {

namespace cood {
  class IRemoteObjectDictionaryAccess;
}

namespace log  {
  class ILogFacility;
}
}


// class definition ===========================================================

namespace gpcc {
namespace cood {

/**
 * \ingroup GPCC_COOD_REMOTEACCESS_INFRASTRUCTURE
 * \brief Server providing access to a RODA interface via IPC.
 *
 * This is the counterpart of @ref IPCClient.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class IPCServer final : private IRemoteObjectDictionaryAccessNotifiable
{
  public:
    IPCServer(void) = delete;
    IPCServer(std::string const & name,
              gpcc::log::ILogFacility & logFacility,
              gpcc::cood::IRemoteObjectDictionaryAccess & _roda);
    IPCServer(IPCServer const &) = delete;
    IPCServer(IPCServer &&) = delete;
    ~IPCServer(void);

    IPCServer& operator=(IPCServer const &) = delete;
    IPCServer& operator=(IPCServer &&) = delete;

    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

  private:
    /// RODA interface that shall be accessible by @ref IPCClient via IPC.
    gpcc::cood::IRemoteObjectDictionaryAccess & roda;


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

    // <-- IRemoteObjectDictionaryAccessNotifiable
    void OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept override;
    void OnDisconnected(void) noexcept override;
    void OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept override;

    void LoanExecutionContext(void) noexcept override;
    // --> IRemoteObjectDictionaryAccessNotifiable
};

} // namespace cood
} // namespace gpcc

#endif // IPCSERVER_HPP_202102101606
#endif // defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
