/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#include "TriggerProvider.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/osal/Panic.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/raii/scope_guard.hpp"
#include "gpcc/src/time/TimePoint.hpp"
#include <stdexcept>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

TriggerProvider::TriggerProvider(gpcc::time::TimeSpan const _expectedWaitWithTimeoutValue,
                                 uint32_t const _permanentTriggerSleep_ms)
: gpcc::StdIf::IIRQ2ThreadWakeup()
, expectedWaitWithTimeoutValue(_expectedWaitWithTimeoutValue)
, permanentTriggerSleep_ms(_permanentTriggerSleep_ms)
, mutex()
, threadInWaitWithTimeout(false)
, threadInWaitWithTimeoutSetConvar()
, continueFlag(false)
, permanentContinue(false)
, continueFlagSetConvar()
, desiredReturnValue(gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK)
{
}

bool TriggerProvider::WaitForThread(uint32_t const timeout_ms)
{
  // Blocks until a thread enters WaitWithTimeout() or a timeout occurs.
  // true  = OK, thread inside WaitWithTimeout()
  // false = timeout

  using namespace gpcc::time;

  gpcc::osal::MutexLocker mutexLocker(mutex);

  TimePoint const absTimeout = TimePoint::FromSystemClock(Clocks::monotonic) + TimeSpan::ms(timeout_ms);
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
void TriggerProvider::Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result const _desiredReturnValue, bool const permanent)
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

// --> gpcc::StdIf::IIRQ2ThreadWakeup
void TriggerProvider::SignalFromISR(void) noexcept
{
  gpcc::osal::Panic("Unexpected call to TriggerProvider::SignalFromISR");
}
void TriggerProvider::SignalFromThread(void)
{
  gpcc::osal::Panic("Unexpected call to TriggerProvider::SignalFromThread");
}

gpcc::StdIf::IIRQ2ThreadWakeup::Result TriggerProvider::Wait(void)
{
  gpcc::osal::Panic("Unexpected call to TriggerProvider::Wait");

  // never reached, but makes compiler happy
  return gpcc::StdIf::IIRQ2ThreadWakeup::Result::OK;
}
gpcc::StdIf::IIRQ2ThreadWakeup::Result TriggerProvider::WaitWithTimeout(gpcc::time::TimeSpan const & timeout)
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
// <-- gpcc::StdIf::IIRQ2ThreadWakeup

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests
