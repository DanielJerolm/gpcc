/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
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
 * \class ThreadBlocker ThreadBlocker.hpp "src/osal/os/linux_arm_tfc/internal/ThreadBlocker.hpp"
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
