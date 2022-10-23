/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#include <gpcc/cood/remote_access/infrastructure/IPCServer.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/ResponseBase.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccess.hpp>
#include <gpcc/log/logfacilities/ILogFacility.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <functional>
#include <stdexcept>

namespace gpcc {
namespace cood {

using gpcc::log::LogType;

IPCServer::IPCServer(std::string const & name,
                     gpcc::log::ILogFacility & logFacility,
                     gpcc::cood::IRemoteObjectDictionaryAccess & _roda)
: IRemoteObjectDictionaryAccessNotifiable()
, roda(_roda)
, logger(name)
, thread(name)
, startStopMutex()
, running(false)
{
  logger.SetLogLevel(gpcc::log::LogLevel::InfoOrAbove);
  logFacility.Register(logger);
}

IPCServer::~IPCServer(void)
{
  try
  {
    {
      gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

      if (running)
        PANIC();
    }

    if (logger.GetLogFacility() != nullptr)
      logger.GetLogFacility()->Unregister(logger);
  }
  catch (...)
  {
    PANIC();
  }
}

void IPCServer::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                      gpcc::osal::Thread::priority_t const priority,
                      size_t const stackSize)
{
  gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

  if (running)
    throw std::logic_error("IPCServer::Start: Already running.");

  logger.Log(LogType::Info, "Starting...");
  thread.Start(std::bind(&IPCServer::ThreadEntry, this), schedPolicy, priority, stackSize);

  running = true;
}

void IPCServer::Stop(void) noexcept
{
  try
  {
    gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

    if (!running)
      PANIC();

    logger.Log(LogType::Info, "Stopping...");
    thread.Cancel();
    thread.Join();

    logger.Log(LogType::Info, "Stopped");
    running = false;
  }
  catch (...)
  {
    PANIC();
  }
}

void* IPCServer::ThreadEntry(void)
{
  logger.Log(LogType::Info, "Started");

  ON_SCOPE_EXIT(unregisterRODA)
  {
    logger.Log(LogType::Debug, "Unregistering from RODA interface");
    roda.Unregister();
  };

  logger.Log(LogType::Debug, "Registering at RODA interface");
  roda.Register(this);

  while (!thread.IsCancellationPending())
  {
    gpcc::osal::Thread::Sleep_ms(1000U);
  }

  return nullptr;
}

// <-- IRemoteObjectDictionaryAccessNotifiable
void IPCServer::OnReady(size_t const maxRequestSize, size_t const maxResponseSize) noexcept
{
  if (logger.IsAboveLevel(LogType::Debug))
  {
    try
    {
      std::string message = "IPCServer::OnReady: maxRequestSize=" + std::to_string(maxRequestSize) +
                            ", maxResponseSize=" + std::to_string(maxResponseSize);
      logger.Log(LogType::Debug, message);
    }
    catch (std::exception const &)
    {
      logger.LogFailed();
    }
  }
}

void IPCServer::OnDisconnected(void) noexcept
{
  logger.Log(LogType::Debug, "IPCServer::OnDisconnected");
}

void IPCServer::OnRequestProcessed(std::unique_ptr<ResponseBase> spResponse) noexcept
{
  if (logger.IsAboveLevel(LogType::Debug))
  {
    try
    {
      std::string message = "IPCServer::OnRequestProcessed:\n" + spResponse->ToString();
      logger.Log(LogType::Debug, message);
    }
    catch (std::exception const &)
    {
      logger.LogFailed();
    }
  }

}

void IPCServer::LoanExecutionContext(void) noexcept
{
  logger.Log(LogType::Debug, "IPCServer::LoanExecutionContext");
}

// --> IRemoteObjectDictionaryAccessNotifiable

} // namespace cood
} // namespace gpcc

#endif // defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
