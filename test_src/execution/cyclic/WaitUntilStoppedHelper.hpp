/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef WAITUNTILSTOPPEDHELPER_HPP_201612302047
#define WAITUNTILSTOPPEDHELPER_HPP_201612302047

#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Thread.hpp>

namespace gpcc
{
  namespace execution
  {
    namespace cyclic
    {
      class TTCEStartStopCtrl;
    }
  }
  namespace time
  {
    class TimeSpan;
  }
}

namespace gpcc_tests {
namespace execution {
namespace cyclic {

// Helper for TTCEStartStopCtrl related unit tests. Invokes TTCEStartStopCtrl::WaitUntilStopped()
// using an own thread.
class WaitUntilStoppedHelper final
{
  public:
    WaitUntilStoppedHelper(gpcc::execution::cyclic::TTCEStartStopCtrl & _uut);
    WaitUntilStoppedHelper(WaitUntilStoppedHelper const &) = delete;
    WaitUntilStoppedHelper(WaitUntilStoppedHelper &&) = delete;
    ~WaitUntilStoppedHelper(void) = default;

    WaitUntilStoppedHelper& operator=(WaitUntilStoppedHelper const &) = delete;
    WaitUntilStoppedHelper& operator=(WaitUntilStoppedHelper &&) = delete;

    void Start(void);
    void Stop(void);


    void StartWaiting(void);
    bool IsStopped(void) const;
    bool WaitUntilStopped(gpcc::time::TimeSpan const & timeout) const;

  private:
    enum class States
    {
      idle,             // Helper is idle
      reqStartWaiting,  // Helper is requested to enter uut.WaitUntilStopped()
      waiting,          // Helper is inside uut.WaitUntilStopped()
      stopped           // Helper has returned from uut.WaitUntilStopped()
    };

    gpcc::execution::cyclic::TTCEStartStopCtrl & uut;

    mutable gpcc::osal::Mutex mutex;
    States state;
    gpcc::osal::ConditionVariable state_ReqStartWaiting_Entered_ConVar;
    gpcc::osal::ConditionVariable state_Waiting_Entered_ConVar;
    mutable gpcc::osal::ConditionVariable state_Stopped_Entered_ConVar;
    gpcc::osal::Thread thread;


    void* ThreadEntry(void);
};

} // namespace cyclic
} // namespace execution
} // namespace gpcc_tests

#endif // WAITUNTILSTOPPEDHELPER_HPP_201612302047
