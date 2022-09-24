/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#ifndef TIMELIMITEDTHREADBLOCKER_HPP_201904071048
#define TIMELIMITEDTHREADBLOCKER_HPP_201904071048

#include "ThreadBlockerBase.hpp"
#include <gpcc/time/TimePoint.hpp>
#include "UnmanagedConditionVariable.hpp"

namespace gpcc {
namespace osal {

class Mutex;

namespace internal {

class TFCCore;

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \class TimeLimitedThreadBlocker TimeLimitedThreadBlocker.hpp "src/osal/os/linux_arm_tfc/internal/TimeLimitedThreadBlocker.hpp"
 * \brief Blocks a thread until either a condition is signaled or a timeout condition occurs and
 * optionally unlocks/reaquires a TFC-managed mutex.
 *
 * This class allows to block a thread until either a condition is signaled or an timeout condition occurs.
 * As an optional feature, a _TFC-managed_ mutex (class @ref Mutex) can be unlocked before blocking the thread
 * and can be the mutex can be acquired again after wake-up. Blocking and unlocking the _TFC-managed_ mutex are
 * performed as an atomic operation.
 *
 * This is a helper class for the implementation of the _TFC-managed_ condition variable (class
 * @ref ConditionVariable) and the _TFC-managed_ thread (class @ref Thread). The blocking operation is managed
 * by TFC and this class takes care for all the necessary interaction with class @ref TFCCore. User's of this
 * class just have to invoke @ref Signal() and one of the overloaded variants of @ref Block().
 *
 * Once @ref Signal() or @ref SignalTimeout() have been called, any overloaded variant of @ref Block() will
 * release a potential blocked thread. If an overloaded variant of @ref Block() is called after @ref Signal()
 * or @ref SignalTimeout(), then the calling thread returns immediately.
 *
 * Signaling is a one-way operation. Once signaled, the @ref TimeLimitedThreadBlocker cannot be reset. The
 * typical life-cycle of class @ref TimeLimitedThreadBlocker therefore is:
 * 1. Instantiation
 * 2. Block
 * 3. Signal / Signal timeout
 * 4. Destruction
 *
 * Also valid (checked by unit-tests, but currently not used by TFC) is the following life-cycle:
 * 1. Instantiation
 * 2. Signal / Signal timeout
 * 3. Block
 * 4. Destruction
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TimeLimitedThreadBlocker final : public ThreadBlockerBase
{
    friend class TFCCore;

  public:
    TimeLimitedThreadBlocker(void);
    TimeLimitedThreadBlocker(TimeLimitedThreadBlocker const &) = delete;
    TimeLimitedThreadBlocker(TimeLimitedThreadBlocker &&) = delete;
    virtual ~TimeLimitedThreadBlocker(void);

    TimeLimitedThreadBlocker& operator=(TimeLimitedThreadBlocker const &) = delete;
    TimeLimitedThreadBlocker& operator=(TimeLimitedThreadBlocker &&) = delete;

    void Signal(void) override;
    void SignalTimeout(void);

    bool Block(Mutex & mutexToBeUnlocked, gpcc::time::TimePoint const & _absTimeout);
    bool Block(gpcc::time::TimePoint const & _absTimeout);

  private:
    /// Pointer to the @ref TFCCore instance.
    /** This is setup by the constructor and not changed afterwards. */
    TFCCore* const pTFCCore;

    /// Flag indicating if wake-up has been signaled or not.
    /** TFCCore's big lock is required. */
    bool signaled;

    /// Flag indicating if timeout has occurred or not.
    /** TFCCore's big lock is required. */
    bool timeout;

    /// Flag indicating if a thread is currently blocked or not.
    /** TFCCore's big lock is required. */
    bool blocked;

    /// Absolute point in time when the timeout expires.
    /** TFCCore's big lock is required. */
    gpcc::time::TimePoint absTimeout;

    /// Condition variable used to signal when @ref signaled or @ref timeout have been asserted.
    /** This must be used in conjunction with TFCCore's big lock. */
    UnmanagedConditionVariable signaledCV;
};

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef TIMELIMITEDTHREADBLOCKER_HPP_201904071048
#endif // #ifdef OS_LINUX_ARM_TFC
