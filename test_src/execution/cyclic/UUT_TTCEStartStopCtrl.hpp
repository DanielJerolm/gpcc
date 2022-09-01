/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef UUT_TTCESTARTSTOPCTRL_HPP_201612302050
#define UUT_TTCESTARTSTOPCTRL_HPP_201612302050

#include <gpcc/execution/cyclic/TriggeredThreadedCyclicExec.hpp>
#include <gpcc/execution/cyclic/TTCEStartStopCtrl.hpp>
#include <cstdint>

namespace gpcc
{
  namespace execution
  {
    namespace async
    {
      class IWorkQueue;
    }
  }
}

namespace gpcc_tests {
namespace execution {
namespace cyclic {

class Trace;

using gpcc::execution::cyclic::TTCEStartStopCtrl;
using gpcc::execution::cyclic::TriggeredThreadedCyclicExec;

// UUT for TTCEStartStopCtrl related tests.
// Class TTCEStartStopCtrl cannot be used directly, because it is an abstract base class.
// The UUT records all invocations of virtual methods.
class UUT_TTCEStartStopCtrl final: public TTCEStartStopCtrl
{
  public:
    UUT_TTCEStartStopCtrl(TriggeredThreadedCyclicExec & _ttce,
                          uint8_t const _restartAttemptsAfterLossOfLock,
                          gpcc::execution::async::IWorkQueue & _wq,
                          Trace & _trace,
                          uint8_t const _onBeforeRestartAfterLossOfLockRetVal);
    UUT_TTCEStartStopCtrl(UUT_TTCEStartStopCtrl const &) = delete;
    UUT_TTCEStartStopCtrl(UUT_TTCEStartStopCtrl&&) = delete;
    ~UUT_TTCEStartStopCtrl(void) = default;

    UUT_TTCEStartStopCtrl& operator=(UUT_TTCEStartStopCtrl const &) = delete;
    UUT_TTCEStartStopCtrl& operator=(UUT_TTCEStartStopCtrl &&) = delete;

  private:
    Trace & trace;
    uint8_t const onBeforeRestartAfterLossOfLockRetVal;

    uint8_t OnBeforeRestartAfterLossOfLock(void) override;
    void OnStateSwitchedTo_Stopped(TriggeredThreadedCyclicExec::StopReasons const stopReason) override;
    void OnStateSwitchedTo_Starting(void) override;
    void OnStateSwitchedTo_Running(void) override;
    void OnStateSwitchedTo_StopPending(void) override;
    void OnStateSwitchedTo_StoppedStopPending(TriggeredThreadedCyclicExec::StopReasons const stopReason) override;
    void OnBadAllocWQ(void) override;
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // UUT_TTCESTARTSTOPCTRL_HPP_201612302050
