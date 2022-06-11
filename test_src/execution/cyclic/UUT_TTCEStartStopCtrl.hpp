/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017 Daniel Jerolm

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

#ifndef UUT_TTCESTARTSTOPCTRL_HPP_201612302050
#define UUT_TTCESTARTSTOPCTRL_HPP_201612302050

#include "gpcc/src/execution/cyclic/TriggeredThreadedCyclicExec.hpp"
#include "gpcc/src/execution/cyclic/TTCEStartStopCtrl.hpp"
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
