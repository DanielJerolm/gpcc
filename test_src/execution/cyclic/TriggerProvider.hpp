/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TRIGGERPROVIDER_HPP_201612302045
#define TRIGGERPROVIDER_HPP_201612302045

#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/stdif/notify/IIRQ2ThreadWakeup.hpp>
#include <gpcc/time/TimeSpan.hpp>
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
    bool SignalFromISR(void) noexcept override;
    bool SignalFromThread(void) override;

    gpcc::StdIf::IIRQ2ThreadWakeup::Result Wait(void) override;
    gpcc::StdIf::IIRQ2ThreadWakeup::Result WaitWithTimeout(gpcc::time::TimeSpan const & timeout) override;
    // <-- gpcc::StdIf::IIRQ2ThreadWakeup
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // TRIGGERPROVIDER_HPP_201612302045
