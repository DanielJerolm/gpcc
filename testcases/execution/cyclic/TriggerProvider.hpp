/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2025 Daniel Jerolm
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

// This class provides a trigger via gpcc::stdif::IIRQ2ThreadWakeup. Trigger generation is under
// full manual control. The class is intended to be used in unit tests of classes
// TriggeredThreadedCyclicExec and TTCEStartStopCtrl.
class TriggerProvider final : public gpcc::stdif::IIRQ2ThreadWakeup
{
  public:
    TriggerProvider(gpcc::time::TimeSpan const expectedWaitWithTimeoutValue,
                    uint32_t const permanentTriggerSleep_ms);
    TriggerProvider(TriggerProvider const &) = delete;
    TriggerProvider(TriggerProvider &&) = delete;
    ~TriggerProvider(void) = default;

    TriggerProvider& operator=(TriggerProvider const &) = delete;
    TriggerProvider& operator=(TriggerProvider &&) = delete;

    bool WaitForThread(uint32_t const timeout_ms);
    void Trigger(gpcc::stdif::IIRQ2ThreadWakeup::Result const desiredReturnValue, bool const permanent);

  private:
    // Expected timeout when IIRQ2ThreadWakeup::WaitWithTimeout() is invoked.
    gpcc::time::TimeSpan const expectedWaitWithTimeoutValue_;

    // Time span slept in continous trigger mode before WaitWithTimeout() returns.
    uint32_t const permanentTriggerSleep_ms_;

    // Mutex to make stuff thread-safe.
    gpcc::osal::Mutex mutex_;

    // Flag indicating that a thread is inside "WaitWithTimeout()" and associated ConVar.
    bool threadInWaitWithTimeout_;
    gpcc::osal::ConditionVariable threadInWaitWithTimeoutSetConvar_;

    // Flag signaling that the thread in "WaitWithTimeout()" shall continue and associated ConVar.
    bool continueFlag_;
    bool permanentContinue_;
    gpcc::osal::ConditionVariable continueFlagSetConvar_;

    // Desired return value for "WaitWithTimeout()".
    gpcc::stdif::IIRQ2ThreadWakeup::Result desiredReturnValue_;


    // --> gpcc::stdif::IIRQ2ThreadWakeup
    bool SignalFromISR(void) noexcept override;
    bool SignalFromThread(void) override;

    gpcc::stdif::IIRQ2ThreadWakeup::Result Wait(void) override;
    gpcc::stdif::IIRQ2ThreadWakeup::Result WaitWithTimeout(gpcc::time::TimeSpan const & timeout) override;
    // <-- gpcc::stdif::IIRQ2ThreadWakeup
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // TRIGGERPROVIDER_HPP_201612302045
