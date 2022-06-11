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

#ifdef OS_LINUX_ARM_TFC

#ifndef THREADBLOCKER_HPP_201904071047
#define THREADBLOCKER_HPP_201904071047

#include "ThreadBlockerBase.hpp"
#include "UnmanagedConditionVariable.hpp"

namespace gpcc {
namespace osal {

class Mutex;

namespace internal {

class TFCCore;

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief Blocks a thread until a condition is signaled and unlocks/reaquires a TFC-managed mutex.
 *
 * This class allows to block a thread until a condition is signaled. Before blocking, a _TFC-managed_
 * mutex (class @ref Mutex) is unlocked and after wake-up the _TFC-managed_ mutex is acquired again.
 * Blocking and unlocking the _TFC-managed_ mutex are performed as an atomic operation.
 *
 * This is a helper class for the implementation of the _TFC-managed_ condition variable (class
 * @ref ConditionVariable) and the _TFC-managed_ thread (class @ref Thread). The blocking operation is managed
 * by TFC and this class takes care for all the necessary interaction with class @ref TFCCore. User's of this
 * class just have to invoke @ref Block() and @ref Signal().
 *
 * Once @ref Signal() has been called, @ref Block() will release a potential blocked thread. If @ref Block()
 * is called after @ref Signal(), then the calling thread returns immediately.
 *
 * Signaling is a one-way operation. Once signaled, the @ref ThreadBlocker cannot be reset. The typical
 * life-cycle of class @ref ThreadBlocker therefore is:
 * 1. Instantiation
 * 2. Block
 * 3. Signal
 * 4. Destruction
 *
 * Also valid (checked by unit-tests, but currently not used by TFC) is the following life-cycle:
 * 1. Instantiation
 * 2. Signal
 * 3. Block
 * 4. Destruction
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class ThreadBlocker final: public ThreadBlockerBase
{
  public:
    ThreadBlocker(void);
    ThreadBlocker(ThreadBlocker const &) = delete;
    ThreadBlocker(ThreadBlocker &&) = delete;
    virtual ~ThreadBlocker(void);

    ThreadBlocker& operator=(ThreadBlocker const &) = delete;
    ThreadBlocker& operator=(ThreadBlocker &&) = delete;

    void Signal(void) override;

    void Block(Mutex & mutexToBeUnlocked);

  private:
    /// Pointer to the @ref TFCCore instance.
    /** This is setup by the constructor and not changed afterwards. */
    TFCCore* const pTFCCore;

    /// Flag indicating if wake-up has been signaled or not.
    /** TFCCore's big lock is required. */
    bool signaled;

    /// Flag indicating if a thread is currently blocked or not.
    /** TFCCore's big lock is required. */
    bool blocked;

    /// Condition variable used to signal when @ref signaled has been asserted.
    /** This must be used in conjunction with TFCCore's big lock. */
    UnmanagedConditionVariable signaledCV;
};

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef THREADBLOCKER_HPP_201904071047
#endif // #ifdef OS_LINUX_ARM_TFC
