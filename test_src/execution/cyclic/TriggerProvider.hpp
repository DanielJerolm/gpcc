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

#ifndef TRIGGERPROVIDER_HPP_201612302045
#define TRIGGERPROVIDER_HPP_201612302045

#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/StdIf/IIRQ2ThreadWakeup.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <cstdint>

namespace gpcc_tests {
namespace execution {
namespace cyclic {

// This class provides a trigger via gpcc::StdIf::IIRQ2ThreadWakeup. Trigger generation is under
// full manual control. The class is intended to be used in unit tests of classes
// TriggeredThreadedCyclicExec and TTCEStartStopCtrl.
class TriggerProvider final : public gpcc::StdIf::IIRQ2ThreadWakeup
{
  public:
    TriggerProvider(gpcc::time::TimeSpan const _expectedWaitWithTimeoutValue,
                    uint32_t const _permanentTriggerSleep_ms);
    TriggerProvider(TriggerProvider const &) = delete;
    TriggerProvider(TriggerProvider &&) = delete;
    ~TriggerProvider(void) = default;

    TriggerProvider& operator=(TriggerProvider const &) = delete;
    TriggerProvider& operator=(TriggerProvider &&) = delete;

    bool WaitForThread(uint32_t const timeout_ms);
    void Trigger(gpcc::StdIf::IIRQ2ThreadWakeup::Result const _desiredReturnValue, bool const permanent);

  private:
    // Expected timeout when IIRQ2ThreadWakeup::WaitWithTimeout is invoked.
    gpcc::time::TimeSpan const expectedWaitWithTimeoutValue;

    // Time span slept in continous trigger mode before WaitWithTimeout() returns.
    uint32_t const permanentTriggerSleep_ms;

    // Mutex to make stuff thread-safe.
    gpcc::osal::Mutex mutex;

    // Flag indicating that a thread is inside "WaitWithTimeout" and associated ConVar.
    bool threadInWaitWithTimeout;
    gpcc::osal::ConditionVariable threadInWaitWithTimeoutSetConvar;

    // Flag signaling that the thread in "WaitWithTimeout" shall continue and associated ConVar.
    bool continueFlag;
    bool permanentContinue;
    gpcc::osal::ConditionVariable continueFlagSetConvar;

    // Desired return value for "WaitWithTimeout".
    gpcc::StdIf::IIRQ2ThreadWakeup::Result desiredReturnValue;


    // --> gpcc::StdIf::IIRQ2ThreadWakeup
    void SignalFromISR(void) noexcept override;
    void SignalFromThread(void) override;

    gpcc::StdIf::IIRQ2ThreadWakeup::Result Wait(void) override;
    gpcc::StdIf::IIRQ2ThreadWakeup::Result WaitWithTimeout(gpcc::time::TimeSpan const & timeout) override;
    // <-- gpcc::StdIf::IIRQ2ThreadWakeup
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // TRIGGERPROVIDER_HPP_201612302045
