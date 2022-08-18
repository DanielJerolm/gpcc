/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifndef TRIGGEREDTHREADEDCYCLICEXEC_HPP_201612301706
#define TRIGGEREDTHREADEDCYCLICEXEC_HPP_201612301706

#include "gpcc/src/osal/Mutex.hpp"
#include "gpcc/src/osal/Thread.hpp"
#include "gpcc/src/time/TimeSpan.hpp"
#include <functional>
#include <cstdint>
#include <cstddef>

namespace gpcc {

namespace StdIf
{
  class IIRQ2ThreadWakeup;
}

namespace execution {
namespace cyclic {

/**
 * \ingroup GPCC_EXECUTION_CYCLIC
 * \brief Abstract base class for cyclic (triggered) execution of subclass' code in an own thread.
 *
 * The executed subclass' code usually implements some kind of controller or control loop that must be
 * cyclically executed (or "sampled" to say it in the language of control technology). Cyclic execution
 * is triggered by an [IIRQ2ThreadWakeup](@ref gpcc::StdIf::IIRQ2ThreadWakeup) which is used to deliver
 * an (cyclic) trigger to class @ref TriggeredThreadedCyclicExec. Execution of the subclass' code can be
 * enabled and disabled.
 *
 * # Subclassing
 * The derived sub-class must implement the following virtual methods:
 * - @ref Cyclic()
 * - @ref OnStart()
 * - @ref OnStop()
 * - @ref Sample()
 * - @ref OnStateChange()
 *
 * Method @ref OnStart() is invoked upon reception of the trigger exactly one cycle before method
 * @ref Sample() is called for the first time. It can be used by the subclass to initialize or prepare stuff
 * that is used within @ref Sample(). @ref OnStart() must finish before the next trigger is received, or
 * an overrun condition will be detected.
 *
 * Method @ref OnStop() is invoked after cyclic execution of @ref Sample() has stopped. It is either
 * executed upon reception of the next trigger after the last call to @ref Sample() or it is executed
 * directly after @ref Sample() has returned. @ref OnStop() is intended to inform the subclass that cyclic
 * execution of @ref Sample() has been stopped. The subclass may use @ref OnStop() to perform clean-up
 * operations or shut things properly down. In contrast to @ref OnStart(), @ref OnStop() does not necessarily
 * need to finish before reception of the next trigger.\n
 * _Note:_\n
 * _OnStop() is not called or the call is not completed if StopThread() is invoked!_\n
 * See section "Starting and Stopping the internal thread" below.
 *
 * Method @ref Sample() is invoked upon reception of the (cyclic) trigger. The subclass is intended to run the
 * code that shall be cyclically executed from within this method. @ref Sample() must finish before the next
 * trigger is received, or an overrun condition will be detected. Any overrun condition is reported to
 * @ref Sample() via parameter `overrun`.
 *
 * Method @ref Cyclic() is always cyclically invoked, regardless of if the cyclic trigger (e.g. from a PLL)
 * is present or not, or if the PLL (if any) is locked or not, or if cyclic execution of @ref Sample()
 * is currently enabled or not.\n
 * If cyclic execution of @ref Sample() is enabled, then @ref Cyclic() is executed after @ref Sample() has
 * returned. @ref Cyclic() must finish before the next trigger is received, or an overrun condition will be
 * detected. However, @ref Cyclic() should also finish before reception of the next trigger if cyclic execution
 * of @ref Sample() is disabled.\n
 * Subclasses may use @ref Cyclic() to run code cyclically even if cyclic execution of @ref Sample() is
 * disabled or if the cyclic trigger is missing. In the latter case, @ref Cyclic() is executed each time
 * the timeout used to monitor the cyclic trigger expires. Subclasses shall therefore not rely on deterministic
 * behavior regarding calls to @ref Cyclic(). An typical example for usage of @ref Cyclic() includes control of status
 * indicator LEDs or sampling pushbuttons.
 *
 * All virtual methods mentioned in this section are executed in the context of the thread provided by this class.
 * No internal mutex of the @ref TriggeredThreadedCyclicExec is locked upon invocation of any of the virtual methods.
 * Thus it is safe to invoke any of the public methods @ref RequestStartSampling(), @ref RequestStopSampling() and
 * @ref GetCurrentState() from the virtual methods.
 *
 * For further details on the virtual methods please refer to the documentation of these methods.
 *
 * # Trigger
 * On instantiation, a reference to an [IIRQ2ThreadWakeup](@ref gpcc::StdIf::IIRQ2ThreadWakeup) interface is passed
 * to the constructor of class @ref TriggeredThreadedCyclicExec. The interface is used to receive the cyclic trigger.\n
 * The trigger is typically generated by a hardware timer or a PLL implemented using a hardware timer, but it
 * could also be pure software generated.
 *
 * # PLL Lock State Monitoring
 * If a PLL is generating the cyclic trigger via the [IIRQ2ThreadWakeup](@ref gpcc::StdIf::IIRQ2ThreadWakeup) interface,
 * then class @ref TriggeredThreadedCyclicExec is able to monitor the PLL's lock state. If the PLL looses lock, then
 * cyclic execution of the @ref Sample() method is stopped.\n
 * Monitoring is accomplished using a callback passed to the constructor of class @ref TriggeredThreadedCyclicExec.
 * The callback is invoked after reception of each trigger to check the lock state.\n
 * If no PLL is used to generate the trigger, or if lock-state monitoring is not required, then the callback
 * can be omitted during instantiation of class @ref TriggeredThreadedCyclicExec.
 *
 * # Monitoring of cyclic trigger
 * While this class is waiting for the PLL to lock or while the PLL is locked and @ref Sample() is cyclically
 * executed during normal operation, the presence of the cyclic trigger is monitored using a timeout. If the cyclic
 * trigger does not occur within the timeout, then cyclic execution of method @ref Sample() is stopped.\n
 * Note that method @ref Cyclic() will still be cyclically executed each time the timeout expires.
 *
 * # Starting and Stopping sampling
 * Execution of method @ref Sample() can be enabled and disabled using the methods @ref RequestStartSampling()
 * and @ref RequestStopSampling(). \n
 * Sampling is gracefully started and stopped with invocations of @ref OnStart(), @ref OnStop(), and
 * @ref OnStateChange() (see state chart below).
 * Usually class @ref TTCEStartStopCtrl is used in conjunction with this class to conveniently enable and disable
 * cyclic execution of the @ref Sample() method.
 *
 * \htmlonly <style>div.image img[src="execution/cyclic/TTCE_StateMachine.png"]{width:75%;}</style> \endhtmlonly
 * \image html "execution/cyclic/TTCE_StateMachine.png" "Internal States of class TriggeredThreadedCyclicExec"
 *
 * ## Startup sequence
 * After invocation of @ref RequestStartSampling(), the @ref TriggeredThreadedCyclicExec switches to state
 * @ref States::starting upon reception of the next trigger.\n
 * @ref TriggeredThreadedCyclicExec remains in @ref States::starting for the number of trigger events passed to
 * @ref RequestStartSampling() via parameter `startDelay` plus one. If `startDelay` is zero, then the
 * @ref TriggeredThreadedCyclicExec switches to @ref States::waitLock upon reception of the next trigger.
 * Otherwise it remains in @ref States::starting for the number of extra triggers setup via `startDelay`.\n
 * @ref TriggeredThreadedCyclicExec remains in @ref States::waitLock, until the PLL driving the trigger
 * has locked. Once the PLL has locked, or if no callback for lock-check has been specified during instantiation,
 * @ref TriggeredThreadedCyclicExec switches to @ref States::running upon reception of the next trigger.
 *
 * ## Stop sequence
 * After invocation of @ref RequestStopSampling(), class @ref TriggeredThreadedCyclicExec switches to state
 * @ref States::stopped upon the next trigger event.
 *
 * ## Stop due to errors
 * In addition to stops due to calls to @ref RequestStopSampling(), @ref TriggeredThreadedCyclicExec also
 * switches to state @ref States::stopped in case of the following error conditions:
 * - the trigger timeout expires
 * - the PLL driving the trigger leaves the lock-state (optional,  see "PLL Lock state monitoring" above)
 * - @ref Sample() returned false
 *
 * # Starting and Stopping the internal thread
 * @ref TriggeredThreadedCyclicExec uses an own thread to execute the subclass' code. After instantiation,
 * the thread must be started using @ref StartThread() to allow @ref TriggeredThreadedCyclicExec to become
 * functional.
 *
 * Before destruction, the thread must be stopped again using @ref StopThread().
 *
 * _Note: The thread is stopped using deferred cancellation._
 * _If the operating system does not support deferred cancellation, then the thread will be stopped_
 * _upon reception of the next trigger or upon trigger timeout._
 *
 * It is recommended to stop the thread while the @ref TriggeredThreadedCyclicExec instance is in state
 * @ref States::stopped. This prevents any issues with deferred cancellation occurring e.g. during execution
 * of @ref Sample(). \n
 * However, @ref Cyclic() should still be aware of deferred cancellation, because it is also executed in state
 * @ref States::stopped.
 *
 * After terminating the object's thread, it can be restarted by calling @ref StartThread(). The
 * @ref TriggeredThreadedCyclicExec then continues in state @ref States::stopped.
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class TriggeredThreadedCyclicExec
{
  public:
    /// Enumeration with the class' internal states.
    enum class States
    {
      stopped,   ///<Off.
      starting,  ///<Start is requested.
      waitLock,  ///<Waiting for PLL to lock.
      running    ///<Operating.
    };

    /// Enumeration with reasons for entering state @ref States::stopped.
    enum class StopReasons
    {
      none,             ///<State is not @ref States::stopped.
      reqStopSampling,  ///<@ref RequestStopSampling() was called.
      triggerTimeout,   ///<Trigger timeout.
      pllLossOfLock,    ///<PLL loss of lock.
      sampleRetFalse    ///<@ref Sample() returned false.
    };

    /**
     * \brief Type definition of a functor to a method for retrieving the lock-state of the PLL.
     *
     * Note:
     * - Using a PLL to drive the trigger is optional.
     * - Using the PLL lock monitoring feature is optional (see constructor).
     *
     * Return value:\n
     * The return value shall indicate if the PLL is in the lock state or not:\n
     * true  = PLL is locked\n
     * false = PLL is not locked.
     *
     * __Thread-safety requirements/hints:__\n
     * The referenced method will be cyclically executed in the context of class
     * @ref TriggeredThreadedCyclicExec's thread with internal mutexes being locked.
     * The referenced method is not allowed to call any method of class @ref TriggeredThreadedCyclicExec.
     *
     * __Exception-safety requirements/hints:__\n
     * The referenced function/method shall provide the no-throw guarantee.
     *
     * __Thread-cancellation-safety requirements/hints:__\n
     * The referenced function/method will be invoked with deferred cancellation enabled.
     */
    typedef std::function<bool(void)> tIsPllLocked;


    TriggeredThreadedCyclicExec(char const * const pThreadName,
                                StdIf::IIRQ2ThreadWakeup & _trigger,
                                time::TimeSpan const & _timeout,
                                tIsPllLocked const & _isPllLockedFunc);
    TriggeredThreadedCyclicExec(TriggeredThreadedCyclicExec const &) = delete;
    TriggeredThreadedCyclicExec(TriggeredThreadedCyclicExec &&) = delete;
    virtual ~TriggeredThreadedCyclicExec(void);

    static char const * State2String(States const state);
    static char const * StopReasons2String(StopReasons const code);
    static char const * StopReasons2Description(StopReasons const code);

    TriggeredThreadedCyclicExec& operator=(TriggeredThreadedCyclicExec const &) = delete;
    TriggeredThreadedCyclicExec& operator=(TriggeredThreadedCyclicExec &&) = delete;


    void StartThread(osal::Thread::SchedPolicy const schedPolicy,
                     osal::Thread::priority_t const priority,
                     size_t const stackSize);
    void StopThread(void) noexcept;


    void RequestStartSampling(uint8_t const startDelay);
    void RequestStopSampling(void);

    States GetCurrentState(void) const;

  protected:
    virtual void Cyclic(void) = 0;
    virtual void OnStart(void) = 0;
    virtual void OnStop(void) = 0;
    virtual bool Sample(bool const overrun) = 0;
    virtual void OnStateChange(States const newState, StopReasons const stopReason) = 0;

  private:
    /// Enumeration with flags for asynchronous requests issued to the class' state machine.
    /** Note:\n
        The enum values are stored in @ref asyncReqFlags (uint8_t) later. Multiple enum values may be
        or-combined in @ref asyncReqFlags to allow multiple flags being set simultaneously.\n
        _Therefore enum values must be a power of 2 (0,1,2,4,8,...)._ */
    enum class AsyncReqFlags
    {
      none  = 0x00, ///<No request pending.
      start = 0x01, ///<@ref RequestStartSampling() has been called.
      stop  = 0x02  ///<@ref RequestStopSampling() has been called.
    };


    /// Reference to a [IIRQ2ThreadWakeup](@ref gpcc::StdIf::IIRQ2ThreadWakeup) subclass instance providing the cyclic
    /// trigger.
    StdIf::IIRQ2ThreadWakeup & trigger;

    /// Timeout for the cyclic trigger.
    time::TimeSpan const timeout;

    /// Functor to a method for retrieving the PLL lock state.
    /** If no function/method is referenced, then the PLL lock state is not checked. */
    tIsPllLocked const isPllLockedFunc;

    /// Thread used for cyclic execution of the subclasses' code.
    osal::Thread thread;

    /// Mutex used to make things thread-safe.
    mutable osal::Mutex mutex;

    /// Flags for signaling asynchronous requests to the @ref TriggeredThreadedCyclicExec's state machine.
    /** @ref mutex is required. */
    uint8_t asyncReqFlags;

    /// Current state of the @ref TriggeredThreadedCyclicExec's state machine.
    /** @ref mutex is required. */
    States state;

    /// Counter used to implement the start delay.
    /** @ref mutex is required. */
    uint8_t startDelayCnt;


    void* InternalThreadEntry(void);
};

/**
 * \fn virtual void TriggeredThreadedCyclicExec::Cyclic(void) = 0
 * \brief This is called cyclically regardless of the @ref TriggeredThreadedCyclicExec's state.
 *
 * This is called independently of the @ref TriggeredThreadedCyclicExec's state each time when either a trigger
 * is received or a timeout occurs while waiting for the trigger.
 *
 * If the @ref TriggeredThreadedCyclicExec is in state @ref States::running, then this is called after @ref Sample().
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the TriggeredThreadedCyclicExec's thread.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This is invoked with deferred cancellation enabled.\n
 * The subclass must be aware of deferred cancellation if @ref StopThread() is invoked.
 */

/**
 * \fn virtual void TriggeredThreadedCyclicExec::OnStart(void) = 0
 * \brief This is called after the @ref TriggeredThreadedCyclicExec has switched to @ref States::running.
 *
 * The order of calling when switching to state @ref States::running is as follows:
 * 1. State is set to @ref States::running
 * 2. @ref OnStateChange()
 * 3. @ref OnStart()
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the @ref TriggeredThreadedCyclicExec's thread.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This is invoked with deferred cancellation enabled.\n
 * The subclass must be aware of deferred cancellation if @ref StopThread() is invoked.
 */

/**
 * \fn virtual void TriggeredThreadedCyclicExec::OnStop(void) = 0
 * \brief This is called after the @ref TriggeredThreadedCyclicExec has left state @ref States::running.
 *
 * The order of calling when leaving state @ref States::running is as follows:
 * 1. State is set to @ref States::stopped
 * 2. @ref OnStop()
 * 3. @ref OnStateChange()
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the @ref TriggeredThreadedCyclicExec's thread.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This is invoked with deferred cancellation enabled.\n
 * The subclass must be aware of deferred cancellation if @ref StopThread() is invoked.
 */

/**
 * \fn virtual bool TriggeredThreadedCyclicExec::Sample(bool const overrun) = 0
 * \brief In state @ref States::running, this is called cyclically each time the trigger is received.
 *
 * __Thread safety:__\n
 * This is executed in the context of the @ref TriggeredThreadedCyclicExec's thread.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This is invoked with deferred cancellation enabled.\n
 * The subclass must be aware of deferred cancellation if @ref StopThread() is invoked.
 *
 * ---
 *
 * \param overrun
 * true  = at least the last trigger event was missed\n
 * false = previous trigger event was normally received
 * \return
 * true  = OK\n
 * false = Cyclic execution of @ref Sample() shall be stopped.\n
 * The @ref TriggeredThreadedCyclicExec's state will be changed to @ref States::stopped. The reason for stopping
 * reported via @ref OnStateChange() will be @ref StopReasons::sampleRetFalse. @ref OnStop() will be called.
 */

/**
 * \fn virtual void TriggeredThreadedCyclicExec::OnStateChange(TriggeredThreadedCyclicExec::State const newState, TriggeredThreadedCyclicExec::StopReasons const stopReason) = 0
 * \brief This is called after the state of @ref TriggeredThreadedCyclicExec has changed or after a stop request is
 * received when the state is already @ref States::stopped.
 *
 * Usually a subclass implements this method to inform an [TTCEStartStopCtrl](@ref gpcc::execution::cyclic::TTCEStartStopCtrl)
 * instance via [TTCEStartStopCtrl::OnTTCEStateChange()](@ref gpcc::execution::cyclic::TTCEStartStopCtrl::OnTTCEStateChange)
 * about state changes.
 *
 * If the previous state was @ref States::running, then the order of calling is as follows:
 * 1. State is set to the new state
 * 2. @ref OnStop()
 * 3. @ref OnStateChange()
 *
 * If the new state is @ref States::running, then the order of calling is as follows:
 * 1. State is set to the new state
 * 2. @ref OnStateChange()
 * 3. @ref OnStart()
 *
 * If neither the previous, nor the new state is @ref States::running, then the order of calling is as follows:
 * 1. State is set to the new state
 * 2. @ref OnStateChange()
 *
 * ---
 *
 * __Thread safety:__\n
 * This is executed in the context of the @ref TriggeredThreadedCyclicExec's thread.\n
 * All public methods of the @ref TriggeredThreadedCyclicExec instance are allowed to be called from this method.
 *
 * __Exception safety:__\n
 * Any thrown exception will result in program termination via @ref gpcc::osal::Panic().
 *
 * __Thread cancellation safety:__\n
 * This is invoked with deferred cancellation enabled.\n
 * The subclass must be aware of deferred cancellation if @ref StopThread() is invoked.
 *
 * ---
 *
 * \param newState
 * New state of the @ref TriggeredThreadedCyclicExec.
 * \param stopReason
 * If the new state is @ref States::stopped, then this provides the reason for the transition to @ref States::stopped. \n
 * If the new state is not @ref States::stopped, then this is always @ref StopReasons::none.
 */

} // namespace cyclic
} // namespace execution
} // namespace gpcc

#endif // TRIGGEREDTHREADEDCYCLICEXEC_HPP_201612301706
