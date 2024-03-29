/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TriggerProvider.hpp"
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/osal/Thread.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <stdexcept>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

TriggerProvider::TriggerProvider(gpcc::time::TimeSpan const _expectedWaitWithTimeoutValue,
                                 uint32_t const _permanentTriggerSleep_ms)
: gpcc::stdif::IIRQ2ThreadWakeup()
, expectedWaitWithTimeoutValue(_expectedWaitWithTimeoutValue)
, permanentTriggerSleep_ms(_permanentTriggerSleep_ms)
, mutex()
, threadInWaitWithTimeout(false)
, threadInWaitWithTimeoutSetConvar()
, continueFlag(false)
, permanentContinue(false)
, continueFlagSetConvar()
, desiredReturnValue(gpcc::stdif::IIRQ2ThreadWakeup::Result::OK)
{
}

bool TriggerProvider::WaitForThread(uint32_t const timeout_ms)
{
  // Blocks until a thread enters WaitWithTimeout() or a timeout occurs.
  // true  = OK, thread inside WaitWithTimeout()
  // false = timeout

  using namespace gpcc::time;

  gpcc::osal::MutexLocker mutexLocker(mutex);

  TimePoint const absTimeout = TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID)
                             + TimeSpan::ms(timeout_ms);
  while ((!threadInWaitWithTimeout) || (continueFlag))
  {
    if (threadInWaitWithTimeoutSetConvar.TimeLimitedWait(mutex, absTimeout))
    {
      // timeout
      break;
    }
  }

  return threadInWaitWithTimeout;
}
void TriggerProvider::Trigger(gpcc::stdif::IIRQ2ThreadWakeup::Result const _desiredReturnValue, bool const permanent)
{
  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (!threadInWaitWithTimeout)
    throw std::runtime_error("TriggerProvider::Trigger: No thread inside WaitWithTimeout()");

  if (continueFlag)
    throw std::runtime_error("TriggerProvider::Trigger: Trigger already pending!");

  continueFlag = true;
  permanentContinue = permanent;
  desiredReturnValue = _desiredReturnValue;
  continueFlagSetConvar.Signal();
}

// --> gpcc::stdif::IIRQ2ThreadWakeup
bool TriggerProvider::SignalFromISR(void) noexcept
{
  gpcc::osal::Panic("Unexpected call to TriggerProvider::SignalFromISR");
  return false;
}
bool TriggerProvider::SignalFromThread(void)
{
  gpcc::osal::Panic("Unexpected call to TriggerProvider::SignalFromThread");
  return false;
}

gpcc::stdif::IIRQ2ThreadWakeup::Result TriggerProvider::Wait(void)
{
  gpcc::osal::Panic("Unexpected call to TriggerProvider::Wait");

  // never reached, but makes compiler happy
  return gpcc::stdif::IIRQ2ThreadWakeup::Result::OK;
}
gpcc::stdif::IIRQ2ThreadWakeup::Result TriggerProvider::WaitWithTimeout(gpcc::time::TimeSpan const & timeout)
{
  if (timeout != expectedWaitWithTimeoutValue)
    gpcc::osal::Panic("TriggerProvider::WaitWithTimeout: UUT passed unexpected timeout value");

  gpcc::osal::MutexLocker mutexLocker(mutex);

  if (threadInWaitWithTimeout)
    gpcc::osal::Panic("TriggerProvider::WaitWithTimeout: threadInWaitWithTimeout already set");

  // signal that a thread is within WaitWithTimeout()
  threadInWaitWithTimeout = true;
  threadInWaitWithTimeoutSetConvar.Signal();

  ON_SCOPE_EXIT() { threadInWaitWithTimeout = false; };

  // wait for go
  while ((!continueFlag) && (!permanentContinue))
    continueFlagSetConvar.Wait(mutex);

  continueFlag = false;

  if (permanentContinue)
    gpcc::osal::Thread::Sleep_ms(permanentTriggerSleep_ms);

  // leave WaitWithTimeout()
  return desiredReturnValue;
}
// <-- gpcc::stdif::IIRQ2ThreadWakeup

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
