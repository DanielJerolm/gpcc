/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
