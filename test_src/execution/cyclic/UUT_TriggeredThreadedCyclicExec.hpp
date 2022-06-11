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

#ifndef UUT_TRIGGEREDTHREADEDCYCLICEXEC_HPP_201612302049
#define UUT_TRIGGEREDTHREADEDCYCLICEXEC_HPP_201612302049

#include "gpcc/src/execution/cyclic/TriggeredThreadedCyclicExec.hpp"
#include "gpcc/src/osal/Mutex.hpp"

namespace gpcc
{
  namespace execution
  {
    namespace cyclic
    {
      class TTCEStartStopCtrl;
    }
  }
  namespace StdIf
  {
    class IIRQ2ThreadWakeup;
  }
  namespace time
  {
    class TimeSpan;
  }
}

namespace gpcc_tests {
namespace execution {
namespace cyclic {

class Trace;

using gpcc::execution::cyclic::TriggeredThreadedCyclicExec;
using gpcc::execution::cyclic::TTCEStartStopCtrl;

// UUT for TriggeredThreadedCyclicExec related tests.
// Class TriggeredThreadedCyclicExec cannot be used directly, because it is an abstract base class.
// The UUT records all invocations of Cyclic(), OnStart(), OnStop(), Sample(...), OnStateChange() and IsPllRunning().
// Return values for Sample() and IsPllRunning() can be set via SetSampleRetVal() and SetIsPllRunningRetVal().
// All public methods are thread-safe.
class UUT_TriggeredThreadedCyclicExec final : public TriggeredThreadedCyclicExec
{
  public:
    UUT_TriggeredThreadedCyclicExec(Trace & _trace,
                                    gpcc::StdIf::IIRQ2ThreadWakeup & trigger,
                                    gpcc::time::TimeSpan const & waitForTriggerTimeout);
    UUT_TriggeredThreadedCyclicExec(UUT_TriggeredThreadedCyclicExec const &) = delete;
    UUT_TriggeredThreadedCyclicExec(UUT_TriggeredThreadedCyclicExec &&) = delete;
    ~UUT_TriggeredThreadedCyclicExec(void) = default;

    UUT_TriggeredThreadedCyclicExec& operator=(UUT_TriggeredThreadedCyclicExec const &) = delete;
    UUT_TriggeredThreadedCyclicExec& operator=(UUT_TriggeredThreadedCyclicExec &&) = delete;

    void SetTTCEStartStopCtrl(TTCEStartStopCtrl* const _pTTCEStartStopCtrl);


    void SetSampleRetVal(bool const value);
    void SetIsPllRunningRetVal(bool const value);

  private:
    Trace & trace;
    TTCEStartStopCtrl* pTTCEStartStopCtrl;

    gpcc::osal::Mutex mutex;
    // Return value used when Sample() is called next time. "mutex" is required.
    bool sampleRetVal;
    // Return value used when IsPllRunning() is called next time. "mutex" is required.
    bool isPllRunningRetVal;


    void Cyclic(void) override;
    void OnStart(void) override;
    void OnStop(void) override;
    bool Sample(bool const overrun) override;
    void OnStateChange(States const newState, StopReasons const stopReason) override;

    bool IsPllRunning(void);
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // UUT_TRIGGEREDTHREADEDCYCLICEXEC_HPP_201612302049
