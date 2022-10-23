/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2021 Daniel Jerolm
*/

#if defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)

#include <gpcc/cood/remote_access/infrastructure/IPCClient.hpp>
#include <gpcc/cood/remote_access/roda_itf/IRemoteObjectDictionaryAccessNotifiable.hpp>
#include <gpcc/cood/remote_access/requests_and_responses/RequestBase.hpp>
#include <gpcc/log/logfacilities/ILogFacility.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <functional>
#include <stdexcept>

namespace gpcc {
namespace cood {

using gpcc::log::LogType;

IPCClient::IPCClient(std::string const & name,
                     gpcc::log::ILogFacility & logFacility)
: IRemoteObjectDictionaryAccess()
, logger(name)
, thread(name)
, startStopMutex()
, running(false)
{
  logger.SetLogLevel(gpcc::log::LogLevel::InfoOrAbove);
  logFacility.Register(logger);
}

IPCClient::~IPCClient(void)
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

void IPCClient::Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
                      gpcc::osal::Thread::priority_t const priority,
                      size_t const stackSize)
{
  gpcc::osal::MutexLocker startStopMutexLocker(startStopMutex);

  if (running)
    throw std::logic_error("IPCClient::Start: Already running.");

  logger.Log(LogType::Info, "Starting...");
  thread.Start(std::bind(&IPCClient::ThreadEntry, this), schedPolicy, priority, stackSize);

  running = true;
}

void IPCClient::Stop(void) noexcept
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

void* IPCClient::ThreadEntry(void)
{
  logger.Log(LogType::Info, "Started");

  while (!thread.IsCancellationPending())
  {
    gpcc::osal::Thread::Sleep_ms(1000U);
  }

  return nullptr;
}

// <-- IRemoteObjectDictionaryAccess
void IPCClient::Register(IRemoteObjectDictionaryAccessNotifiable * const pNotifiable)
{
  logger.Log(LogType::Debug, "IPCClient::Register");

  if (pNotifiable == nullptr)
    throw std::invalid_argument("IPCClient::Register: !pNotifiable");

  throw std::logic_error("Not yet implemented");
}

void IPCClient::Unregister(void) noexcept
{
  logger.Log(LogType::Debug, "IPCClient::Unregister");

  gpcc::osal::Panic("Not yet implemented");
}

void IPCClient::Send(std::unique_ptr<RequestBase> & spReq)
{
  if (logger.IsAboveLevel(LogType::Debug))
  {
    try
    {
      std::string message = "IPCClient::Send:\n" + spReq->ToString();
      logger.Log(LogType::Debug, message);
    }
    catch (std::exception const &)
    {
      logger.LogFailed();
    }
  }

  throw std::logic_error("Not yet implemented");
}

void IPCClient::RequestExecutionContext(void)
{
  logger.Log(LogType::Debug, "IPCClient::RequestExecutionContext");

  throw std::logic_error("Not yet implemented");
}

// --> IRemoteObjectDictionaryAccess

} // namespace cood
} // namespace gpcc

#endif // defined(OS_LINUX_ARM) || defined(OS_LINUX_ARM_TFC) || defined(OS_LINUX_X64) || defined(OS_LINUX_X64_TFC)
