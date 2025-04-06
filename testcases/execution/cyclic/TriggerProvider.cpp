/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
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

TriggerProvider::TriggerProvider(gpcc::time::TimeSpan const expectedWaitWithTimeoutValue,
                                 uint32_t const permanentTriggerSleep_ms)
: gpcc::stdif::IIRQ2ThreadWakeup()
, expectedWaitWithTimeoutValue_(expectedWaitWithTimeoutValue)
, permanentTriggerSleep_ms_(permanentTriggerSleep_ms)
, mutex_()
, threadInWaitWithTimeout_(false)
, threadInWaitWithTimeoutSetConvar_()
, continueFlag_(false)
, permanentContinue_(false)
, continueFlagSetConvar_()
, desiredReturnValue_(gpcc::stdif::IIRQ2ThreadWakeup::Result::OK)
{
}

bool TriggerProvider::WaitForThread(uint32_t const timeout_ms)
{
  // Blocks until a thread enters WaitWithTimeout() or a timeout occurs.
  // true  = OK, thread inside WaitWithTimeout()
  // false = timeout

  using namespace gpcc::time;

  gpcc::osal::MutexLocker mutexLocker(mutex_);

  TimePoint const absTimeout = TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID)
                             + TimeSpan::ms(timeout_ms);
  while ((!threadInWaitWithTimeout_) || (continueFlag_))
  {
    if (threadInWaitWithTimeoutSetConvar_.TimeLimitedWait(mutex_, absTimeout))
    {
      // timeout
      break;
    }
  }

  return threadInWaitWithTimeout_;
}

void TriggerProvider::Trigger(gpcc::stdif::IIRQ2ThreadWakeup::Result const desiredReturnValue, bool const permanent)
{
  gpcc::osal::MutexLocker mutexLocker(mutex_);

  if (!threadInWaitWithTimeout_)
    throw std::runtime_error("TriggerProvider::Trigger: No thread inside WaitWithTimeout()");

  if (continueFlag_)
    throw std::runtime_error("TriggerProvider::Trigger: Trigger already pending!");

  continueFlag_ = true;
  permanentContinue_ = permanent;
  desiredReturnValue_ = desiredReturnValue;
  continueFlagSetConvar_.Signal();
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
  if (timeout != expectedWaitWithTimeoutValue_)
    gpcc::osal::Panic("TriggerProvider::WaitWithTimeout: UUT passed unexpected timeout value");

  gpcc::osal::MutexLocker mutexLocker(mutex_);

  if (threadInWaitWithTimeout_)
    gpcc::osal::Panic("TriggerProvider::WaitWithTimeout: threadInWaitWithTimeout_ already set");

  // signal that a thread is within WaitWithTimeout()
  threadInWaitWithTimeout_ = true;
  threadInWaitWithTimeoutSetConvar_.Signal();

  ON_SCOPE_EXIT() { threadInWaitWithTimeout_ = false; };

  // wait for go
  while ((!continueFlag_) && (!permanentContinue_))
    continueFlagSetConvar_.Wait(mutex_);

  continueFlag_ = false;

  if (permanentContinue_)
    gpcc::osal::Thread::Sleep_ms(permanentTriggerSleep_ms_);

  // leave WaitWithTimeout()
  return desiredReturnValue_;
}
// <-- gpcc::stdif::IIRQ2ThreadWakeup

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
