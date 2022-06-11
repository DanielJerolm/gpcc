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

#ifndef WAITUNTILSTOPPEDHELPER_HPP_201612302047
#define WAITUNTILSTOPPEDHELPER_HPP_201612302047

#include "gpcc/src/osal/ConditionVariable.hpp"
#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/Thread.hpp"

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
