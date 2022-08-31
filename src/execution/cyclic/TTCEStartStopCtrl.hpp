/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TTCESTARTSTOPCTRL_HPP_201612301751
#define TTCESTARTSTOPCTRL_HPP_201612301751

#include "TriggeredThreadedCyclicExec.hpp"
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Panic.hpp>
#include <limits>

namespace gpcc {
namespace execution {

namespace async
{
  class IWorkQueue;
}

namespace cyclic {

/**
 * \ingroup GPCC_EXECUTION_CYCLIC
 * \brief Base class for implementing a start and stop control logic for class @ref TriggeredThreadedCyclicExec.
 *
 * # Rationale
 * Any instance of a class derived from @ref TriggeredThreadedCyclicExec needs to be started, stopped, and monitored.
 * The required logic is intentionally not included in @ref TriggeredThreadedCyclicExec. Instead it shall be supplied
 * from the outside. There are different ways to implement the required logic depending on the different use-cases,
 * requirements, and circumstances. This class provides a base class that can be useful for many implementations of
 * classes supervising @ref TriggeredThreadedCyclicExec instances.
 *
 * # Features
 * This class provides the following features:
 * - Convenient methods (@ref StartAsync() and @ref StopAsync()) for starting and stopping the managed
 *   @ref TriggeredThreadedCyclicExec instance.
 * - starting can be locked and unlocked (@ref LockStart() and @ref UnlockStart()).
 *   + Multiple objects may lock at the same time.
 *   + All objects must unlock again before start is unlocked.
 * - Method waiting for reaching the stop-state is provided (@ref WaitUntilStopped()).
 * - Automatic restart after any stop due to PLL loss of lock (optional).
 *   + The number of restart attempts is limited.
 *   + The contingent of restart attempts can be refreshed at any time (@ref RefreshRemainingStartAttempts()).
 *   + The initial value for the contingent of restart attempts can be set (@ref SetRestartAttemptsAfterLossOfLock()).
 * - Virtual methods are offered, that can be implemented by derived classes to gather information about state changes.
 *   + Some virtual methods are executed in workqueue context.
 *   + Execution in workqueue context decouples the executed code from the thread of
 *     class @ref TriggeredThreadedCyclicExec.
 *
 * # Internals
 * Internally, class @ref TTCEStartStopCtrl implements the state machine shown in the figure below. Stimuli
 * for the state machine are delivered from two sources:
 * - From any thread via @ref StartAsync() and @ref StopAsync().
 * - From the managed @ref TriggeredThreadedCyclicExec by calls to @ref OnTTCEStateChange. These calls occur in the
 *   thread of the managed @ref TriggeredThreadedCyclicExec instance and trigger execution of `OnRunWQ()` and
 *   `OnStop_WQ()` in work queue context.
 *
 * \htmlonly <style>div.image img[src="execution/cyclic/TTCESSC_StateMachine.png"]{width:75%;}</style> \endhtmlonly
 * \image html "execution/cyclic/TTCESSC_StateMachine.png" "Internal States of class TTCEStartStopCtrl"
 *
 * ## Important details
 * In @ref States::starting, @ref StopAsync() may be invoked while `OnRun_WQ()` is about to be
 * executed by the work queue. This leads to a race-condition, which is solved this way:
 * - @ref StopAsync() will request a stop at the managed @ref TriggeredThreadedCyclicExec, which will always
 *   trigger execution of `OnStop_WQ()` in work queue context.
 * - Work packages are executed in the order in which they have been inserted into the work queue, so `OnRun_WQ()`
 *   is executed before `OnStop_WQ()`.
 * - @ref States::stopPending ignores the call to `OnRun_WQ()`.
 * - @ref States::stopPending does not proceed until `OnStop_WQ()` is executed.
 * - No start/stop requests are forwarded to the managed @ref TriggeredThreadedCyclicExec in @ref States::stopPending.
 *
 * A similar race-condition may occur in @ref States::running, if @ref StopAsync() is invoked while `OnStop_WQ()`
 * is about to be executed by the work queue. This race condition is solved this way:
 * - @ref StopAsync() will request a stop at the managed @ref TriggeredThreadedCyclicExec, which will always
 *   trigger execution of `OnStop_WQ()` in work queue context.
 * - `OnStop_WQ()` will be executed twice:
 *   + First due to the stop done by the managed @ref TriggeredThreadedCyclicExec itself
 *   + A second time due to the stop-request issued by @ref StopAsync() to the managed @ref TriggeredThreadedCyclicExec.
 *   + `OnStop_WQ()` has a function parameter that indicates the reason for stopping. The parameter allows to
 *     distinguish between the two calls.
 * - Work packages are executed in the order in which they have been inserted into the work queue, so the order of
 *   the two calls to `OnStop_WQ()` is preserved.
 * - In @ref States::stopPending the `OnStop_WQ()` initiated by the @ref TriggeredThreadedCyclicExec itself will
 *   lead to @ref States::stoppedStopPending.
 * - In @ref States::stoppedStopPending, the @ref TTCEStartStopCtrl expects the call to `OnStop_WQ()` initiated
 *   by @ref StopAsync().
 * - No start/stop requests are forwarded to the managed @ref TriggeredThreadedCyclicExec in states
 *   @ref States::stopPending and @ref States::stoppedStopPending.
 *
 * Short: Stuff is under control at any time! :-)
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TTCEStartStopCtrl
{
  public:
    /// Enumeration with return codes of @ref StartAsync() and @ref StopAsync().
    enum class Result
    {
      ok,               ///<OK.
      locked,           ///<Cannot start, start is locked.
      alreadyStarted,   ///<Cannot start, already starting.
      alreadyRunning,   ///<Cannot start, already running.
      alreadyStopping,  ///<Cannot start/stop, already stopping.
      alreadyStopped    ///<Cannot stop, already stopped.
    };

    /// Enumeration with the internal states of class @ref TTCEStartStopCtrl.
    enum class States
    {
      stopped,            ///<The managed @ref TriggeredThreadedCyclicExec is stopped.
      starting,           ///<The managed @ref TriggeredThreadedCyclicExec is starting.
      running,            ///<The managed @ref TriggeredThreadedCyclicExec is running.
      stopPending,        ///<The managed @ref TriggeredThreadedCyclicExec is starting/running and a stop request is pending.
                          /**<This state is also reached if a start request is canceled by a stop request before it
                              was recognized by the managed @ref TriggeredThreadedCyclicExec. */
      stoppedStopPending  ///<The managed @ref TriggeredThreadedCyclicExec has stopped by itself, but a stop request is pending.
                          /**<This state is reached if stop is requested via @ref StopAsync(), but the managed
                              @ref TriggeredThreadedCyclicExec has already stopped by itself due to an error. */
    };

    /// Maximum number of start-locks that can be acquired.
    static uint8_t const MaxNbOfLocks = std::numeric_limits<uint8_t>::max();


    TTCEStartStopCtrl(TriggeredThreadedCyclicExec & _ttce,
                      uint8_t const _restartAttemptsAfterLossOfLock,
                      execution::async::IWorkQueue & _wq);
    TTCEStartStopCtrl(TTCEStartStopCtrl const &) = delete;
    TTCEStartStopCtrl(TTCEStartStopCtrl &&) = delete;
    virtual ~TTCEStartStopCtrl(void);


    static char const * Result2String(Result const code);


    TTCEStartStopCtrl& operator=(TTCEStartStopCtrl const &) = delete;
    TTCEStartStopCtrl& operator=(TTCEStartStopCtrl &&) = delete;


    void LockStart(void);
    void UnlockStart(void);

    Result StartAsync(void);
    Result StopAsync(void);

    void WaitUntilStopped(void) const;

    void SetRestartAttemptsAfterLossOfLock(uint8_t const _restartAttemptsAfterLossOfLock);
    void RefreshRemainingStartAttempts(void);

    States GetCurrentState(void) const;


    void OnTTCEStateChange(TriggeredThreadedCyclicExec::States const newState,
                           TriggeredThreadedCyclicExec::StopReasons const stopReason);

  protected:
    /// Reference to a work queue to be used by this class (and maybe derived classes).
    execution::async::IWorkQueue & wq;


    inline virtual uint8_t OnBeforeRestartAfterLossOfLock(void) { return 0; }
    inline virtual void OnStateSwitchedTo_Stopped(TriggeredThreadedCyclicExec::StopReasons const stopReason) { (void)stopReason; }
    inline virtual void OnStateSwitchedTo_Starting(void) {}
    inline virtual void OnStateSwitchedTo_Running(void) {}
    inline virtual void OnStateSwitchedTo_StopPending(void) {}
    inline virtual void OnStateSwitchedTo_StoppedStopPending(TriggeredThreadedCyclicExec::StopReasons const stopReason) { (void)stopReason; }
    inline virtual void OnBadAllocWQ(void) { osal::Panic("TTCEStartStopCtrl::OnBadAllocWQ"); }

  private:
    /// Reference to the @ref TriggeredThreadedCyclicExec instance managed by this class.
    TriggeredThreadedCyclicExec & ttce;

    /// Mutex used to make the class' API thread-safe.
    mutable osal::Mutex mutex;

    /// Current state of @ref TTCEStartStopCtrl's internal state machine.
    /** @ref mutex is required. */
    States state;

    /// Condition variable used to signal when @ref state is set to @ref States::stopped.
    /** This is to be used in conjunction with @ref mutex. */
    mutable osal::ConditionVariable condVarStateStopped;

    /// Number of automatic restart attempts after PLL loss of lock. Zero = feature disabled.
    /** @ref mutex is required.\n
        This value is used to refresh @ref remainingRestartAttemptsAfterLossOfLock at the following events:
        - Call to @ref StartAsync() and sampling is started (@ref StartAsync() returns Result::ok).
        - Call to @ref RefreshRemainingStartAttempts(). */
    uint8_t restartAttemptsAfterLossOfLock;

    /// Number of remaining attempts to restart after a PLL loss of lock.
    /** @ref mutex is required. */
    uint8_t remainingRestartAttemptsAfterLossOfLock;

    /// Number of currently active start-locks.
    /** @ref mutex is required.\n
        If this is not zero, then @ref StartAsync() will reject to start the managed @ref TriggeredThreadedCyclicExec. */
    uint8_t nbOfStartLocks;


    void OnRun_WQ(void);
    void OnStop_WQ(TriggeredThreadedCyclicExec::StopReasons const stopReason);
};

/**
 * \fn virtual uint8_t TTCEStartStopCtrl::OnBeforeRestartAfterLossOfLock(void)
 * \brief This is invoked directly before the managed [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec)
 *        instance is restarted automatically after a loss of lock of the PLL.
 *
 * Derived classes may overwrite this to get informed if the [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec)
 * is about to be restarted due to a loss of lock of the PLL. This is also used to program the delay after which the
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance shall move from
 * [TriggeredThreadedCyclicExec::States::starting](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec::States::starting)
 * to [TriggeredThreadedCyclicExec::States::waitLock](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec::States::waitLock).
 *
 * Note:\n
 * - If the PLL looses lock, then this is only called if the automatic restart feature is enabled and if the
 *   contingent of restarts is _not_ expired.
 * - If the contingent of restarts _is_ expired, then there is no attempt to restart and @ref TTCEStartStopCtrl
 *   will switch to state @ref States::stopped like it does in case of any other error or stop request.
 * - If no PLL is used to trigger the managed [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec)
 *   instance, or if the automatic restart feature is disabled, or if the default return value (0) is sufficient, then
 *   there is no need for derived classes to overwrite this.
 * - @ref OnStateSwitchedTo_Stopped() will not be invoked in case of an automatic restart.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the work queue passed to the constructor.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 *
 * ---
 *
 * \return
 * Number of extra cycles the managed [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec)
 * instance shall remain in state [TriggeredThreadedCyclicExec::States::starting](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec::States::starting)
 * before moving to state [TriggeredThreadedCyclicExec::States::waitLock](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec::States::waitLock).\n
 * The default implementation returns zero.
 */

/**
 * \fn virtual void TTCEStartStopCtrl::OnStateSwitchedTo_Stopped(TriggeredThreadedCyclicExec::StopReasons const stopReason)
 * \brief This is invoked after @ref TTCEStartStopCtrl has entered state @ref States::stopped.
 *
 * Derived classes may overwrite this to get informed when the managed
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance has stopped.
 * This method is the right place for error logging. Remember that the managed
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) object can either stop by
 * itself due to an error or it can stop due to an stop request issued via @ref StopAsync(). Examine parameter
 * `stopReason` to find out.
 *
 * Note that there may have been a transition to @ref States::stoppedStopPending before, if an stop request and
 * an autonomous stop due to an error occurred at about the same time (see documentation of
 * class @ref TTCEStartStopCtrl and @ref OnStateSwitchedTo_StoppedStopPending() for details).
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the work queue passed to the constructor.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 *
 * ---
 *
 * \param stopReason
 * Reason why the managed [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec)
 * instance has stopped.
 */

/**
 * \fn virtual void TTCEStartStopCtrl::OnStateSwitchedTo_Starting(void)
 * \brief This is called after @ref TTCEStartStopCtrl has entered state @ref States::starting.
 *
 * Derived classes may overwrite this to get informed _after_ start of sampling has been requested
 * at the managed [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed either in the context of the work queue passed to the constructor or in the
 * context of the thread executing @ref StartAsync().
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 */

/**
 * \fn virtual void TTCEStartStopCtrl::OnStateSwitchedTo_Running(void)
 * \brief This is called after @ref TTCEStartStopCtrl has entered state @ref States::running.
 *
 * Derived classes may overwrite this to get informed when the managed
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance has entered the
 * running state.
 *
 * This may be executed before [TriggeredThreadedCyclicExec::OnStart()](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec::OnStart).
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the work queue passed to the constructor.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 */

/**
 * \fn virtual void TTCEStartStopCtrl::OnStateSwitchedTo_StopPending(void)
 * \brief This is called after the @ref TTCEStartStopCtrl has entered state @ref States::stopPending.
 *
 * Derived classes may overwrite this to get informed _after_ stop of sampling has been requested
 * at the managed [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the thread that executes @ref StopAsync().
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 */

/**
 * \fn virtual void TTCEStartStopCtrl::OnStateSwitchedTo_StoppedStopPending(TriggeredThreadedCyclicExec::StopReasons const stopReason)
 * \brief This is called after @ref TTCEStartStopCtrl has entered state @ref States::stoppedStopPending.
 *
 * Derived classes may overwrite this to get informed if the managed
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance has stopped itself
 * due to an error while a stop request is pending. This is the right place for error logging.
 *
 * The difference to @ref OnStateSwitchedTo_Stopped() is that the
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance has stopped by
 * itself due to an error, but asynchronously an additional stop request has been issued (e.g. via @ref StopAsync()) and
 * is still pending. Note that @ref OnStateSwitchedTo_Stopped() will be called after the additional stop request has
 * been recognized by the [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec)
 * instance.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the work queue passed to the constructor.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 *
 * ---
 *
 * \param stopReason
 * Reason why the [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance has
 * stopped.
 */

/**
 * \fn virtual void TTCEStartStopCtrl::OnBadAllocWQ(void)
 * \brief This is invoked if an out-of-memory error occurred within @ref OnTTCEStateChange().
 *
 * The @ref TTCEStartStopCtrl receives state change notifications from the managed
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance via
 * @ref OnTTCEStateChange(). Internally a work package is created and enqueued into @ref wq upon each invocation of
 * @ref OnTTCEStateChange(). If the system runs out of memory, then these operations may fail.
 * _The state change notifications are crucial, so dropping them in case of failure is not an option._ Throwing an
 * exception in case of failure would result in program abort via Panic() because
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) does not expect
 * @ref OnTTCEStateChange() to throw.
 *
 * Derived classes can overwrite this to setup the desired behavior for this (rare) error case. If this method
 * returns, then @ref OnTTCEStateChange() will retry to create and enqueue the work package. If it fails, then
 * this is invoked again. The number of retries is not limited.
 *
 * Derived classes have two options to handle the error within this method:
 * 1. Abort the program via Panic()
 * 2. Sleep for some time and return in order to retry.
 *
 * Release of some unused memory is not an option. This should be better implemented via the new-handler provided
 * by your C++ runtime (see STL documentation for details).
 *
 * The default implementation aborts the program via Panic().
 *
 * ---
 *
 * __Thread safety:__\n
 * This is intended to be invoked in the context of the thread of the managed
 * [TriggeredThreadedCyclicExec](@ref gpcc::execution::cyclic::TriggeredThreadedCyclicExec) instance.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This may be invoked with deferred cancellation enabled.\n
 * Subclasses shall be aware of deferred cancellation.
 */
} // namespace cyclic
} // namespace execution
} // namespace gpcc

#endif // TTCESTARTSTOPCTRL_HPP_201612301751
