/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_X64

#ifndef THREAD_HPP_201701291628
#define THREAD_HPP_201701291628

#include <gpcc/compiler/definitions.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/ThreadRegistry.hpp>
#include <pthread.h>
#include <atomic>
#include <functional>
#include <string>
#include <climits>
#include <cstddef>
#include <cstdint>

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A class used to create and manage a thread.
 *
 * # Features
 * - Management of a single thread per @ref Thread class instance.
 * - Execution of any entry function or method given by a functor.
 * - The functor allows to pass zero, one, or more parameters of any type to the thread entry function.
 * - Configurable scheduling policy, priority, and stack size.\n
 *   _Please pay attention to the section "Operating System specific notes" at the end of class Thread's documentation._
 * - Well-defined thread life-cycle: Starting, running, terminated, joined.
 * - Creation of a new thread is possible after the previous one has been terminated and joined.
 * - A thread may terminate itself at any time via @ref TerminateNow() or by returning from the thread entry function.
 * - A thread may be cancelled by other threads using deferred cancellation.\n
 *   _Please pay attention to the section "Operating System specific notes" at the end of class Thread's documentation._
 * - Deferred cancellation can be enabled and disabled by the thread.
 * - The @ref Join() method allows to retrieve a `void*` pointer returned by the thread on termination.
 * - Class @ref Thread keeps the application's thread registry (instance of class @ref ThreadRegistry).
 *
 * # Thread creation and life-cycle
 * Any thread's life-cycle is always comprised of the following four states:\n
 * __Starting__ -> __Running__ -> __Terminated__ -> __Joined__
 *
 * New threads are created by invoking @ref Start(). A new thread can only be started if the @ref Thread object does
 * not manage any thread which has not yet been terminated and joined.
 *
 * During invocation of @ref Start() a new thread will be created (the thread is _starting_) and it will enter the
 * _running_ state when it starts executing the referenced thread entry function. As shown in the example below,
 * optional parameters can be passed to the thread entry function via the functor passed to @ref Start().
 *
 * After returning from the thread entry function, or after invocation of @ref TerminateNow(), or after deferred thread
 * cancellation, the thread has _terminated_. _Terminated_ threads must be _joined_ via @ref Join() in order to release
 * the resources occupied by the thread (e.g. the thread's stack). After "joining" a thread, the @ref Thread object may
 * be either destroyed or a new thread may be started by invoking @ref Start().
 *
 * The thread-entry function must always return `void*`. The return value of the thread entry function will be returned
 * by the @ref Join() method when the thread is later "joined". This mechanism can be used to transport results or
 * status codes from the terminated thread to the caller of the @ref Join() method.
 *
 * ~~~{.cpp}
 * void* ThreadEntryFunction(Thread* me, int someParameter)
 * {
 *   // some code to be executed in thread context...
 *
 *   // Return nullptr. We could also return a pointer to something else,
 *   // that could be retrieved via Join() later...
 *   return nullptr;
 * }
 *
 * // ...
 *
 * int main(int argc, char** argv)
 * {
 *   Thread myThread;
 *
 *   // start thread and pass a reference to myThread and the value "12" as parameters
 *   // to the thread entry function
 *   myThread.Start(std::bind(&ThreadEntryFunction, &myThread, 12),
 *                  Thread::SchedPolicy::Other, 0, Thread::GetDefaultStackSize());
 *
 *   // do something else...
 *
 *   // ...and finally wait for the thread to terminate
 *   void* retVal = myThread.Join();
 *
 *   // "retVal" will contain the void-pointer returned by the thread entry function.
 *   // Here it will be nullptr.
 *
 *   return 0;
 * }
 * ~~~
 *
 * # Thread termination
 * There are two ways a thread can terminate itself:
 * - By returning from the thread entry function.
 * - By invoking @ref TerminateNow(void* const) of the @ref Thread object managing the thread.
 *
 * If terminating via @ref TerminateNow(void* const), then the value passed as parameter to @ref TerminateNow() will
 * be returned by @ref Join() when the thread is joined.
 *
 * @ref TerminateNow() terminates the thread using a special exception. All objects instantiated on the thread's stack
 * will be properly released when the thread terminates due to a call to @ref TerminateNow(). The exception can even be
 * caught via `catch (...)`, but it __must__ be thrown again afterwards via `throw`:
 * ~~~{.cpp}
 * try
 * {
 *   // some code that might invoke TerminateNow(...).
 * }
 * catch (...)
 * {
 *   // some clean-up
 *   // ...
 *
 *   // rethrow
 *   throw;
 * }
 * ~~~
 *
 * Threads can also be terminated by other threads. This is called _thread cancellation_ and is described in the next
 * section.
 *
 * # Thread cancellation
 * ## Deferred Cancellation
 * A running thread can be requested to terminate by another thread. The other thread has to invoke @ref Cancel() of
 * the @ref Thread object managing the thread which shall be cancelled. The reaction of the managed thread when
 * @ref Cancel() is invoked depends on if the managed thread has cancellation _enabled_ or _disabled_:
 *
 * If cancellation is _disabled_, then any call to @ref Cancel() has no immediate effect. The managed thread is not
 * cancelled, but the cancellation request is queued until cancellation is enabled again. Cancellation requests are
 * never dropped! However the thread can detect the pending cancellation request via @ref IsCancellationPending().
 *
 * If cancellation is _enabled_ and @ref Cancel() is called (or has been called while cancellation was disabled),
 * then the thread will terminate when the next _cancellation point_ is reached.\n
 * See gpcc/src/osal/os/_your_config_/CancellationPoints.txt for a list of cancellation points applicable to your
 * operating system.
 *
 * Deferred cancellation is implemented using a special exception. All objects instantiated on the thread's stack will
 * be properly released when the thread terminates due to deferred cancellation. The exception can be caught via
 * `catch (...)`, but it __must__ be thrown again afterwards via `throw`:
 * ~~~{.cpp}
 * try
 * {
 *   // some code that might hit a cancellation point while a cancellation request is pending
 * }
 * catch (...)
 * {
 *   // some clean-up
 *   // ...
 *
 *   // rethrow
 *   throw;
 * }
 * ~~~
 *
 * The default configuration for any new thread is that deferred cancellation is _enabled_.\n
 * Threads can retrieve and change their own cancelabilty state via @ref SetCancelabilityEnabled().
 *
 * Threads can invoke @ref IsCancellationPending() to figure out if a cancellation request is pending. The function
 * does not care if cancelabilty is currently enabled or disabled.
 *
 * Threads can invoke @ref TestForCancellation() to terminate if cancellation has been requested and if cancellation
 * is enabled.
 *
 * ## Immediate Cancellation
 * Immediate cancellation is not supported, though your operating system might support it. Immediate cancellation is
 * hard to use correctly and program corruption is almost sure.\n
 * Deferred cancellation or -even better- your own custom mechanism for requesting a thread to terminate is usually
 * what you want to use. See section "user-implemented thread cancellation" below.
 *
 * ## User-implemented thread cancellation
 * The best way to terminate a thread is to write your software in a way that it provides mechanisms to ask a service
 * to stop. After receiving a request to stop, a service or module designed this way should gracefully terminate
 * itself, e.g. by leaving the thread entry function.
 *
 * However, deferred cancellation can be useful in some situations e.g. to get a thread out of a blocking system call.
 *
 * # Threads and C++ exceptions
 * The application will be terminated via @ref gpcc::osal::Panic(), if an uncaught exception leaves the thread entry
 * function. If this is not what you want, then you should catch all exceptions in your thread entry function and
 * handle them properly:
 * ~~~{.cpp}
 * void* ThreadEntryFunction(void)
 * {
 *   try
 *   {
 *     // code executed by thread
 *   }
 *   catch (std::exception const & e)
 *   {
 *     // handle error
 *   }
 *
 *   return nullptr;
 * }
 * ~~~
 *
 * # Thread Registry
 * Any application containing GPCC will contain a thread registry. Class @ref Thread provides access to one global
 * instance of class @ref ThreadRegistry. Class @ref ThreadRegistry is a singleton.
 *
 * All instances of class @ref Thread will register and unregister themselves at the thread registry upon creation
 * and destruction.
 *
 * The thread registry can be used to retrieve information about the number of threads and to retrieve pointers to the
 * @ref Thread objects managing the threads. The pointers in turn can be used to e.g. gather detailed information about
 * each thread using @ref Thread::GetInfo(). However, the thread registry only contains threads created via class
 * @ref Thread. Threads created using the API of your specific operating system will not be contained in the registry.
 *
 * The global thread registry can be accessed via interface @ref IThreadRegistry which can be retrieved from class
 * @ref Thread's static public method @ref GetThreadRegistry().
 *
 * # Operating System specific notes
 * GPCC's OSAL has been designed to be portable and to provide _equivalent functionality_ on different operating
 * systems. Though currently Linux and ChibiOS/RT are the only supported operating systems, GPCC's OSAL can be easily
 * ported to other operating systems. The greater an OS' affinity to POSIX threads, the more simple is the
 * corresponding GPCC OSAL implementation.
 *
 * However the scheduling policies offered by GPCC's OSAL are not supported by all operating systems. Please check out
 * the sections below.
 *
 * ## Linux (linux_x64, linux_arm)
 * Full support.\n
 * Be aware that some scheduling policies might require special user rights/permissions.
 *
 * \htmlonly <style>div.image img[src="osal/os/priority_mapping_linux.png"]{width:50%;}</style> \endhtmlonly
 * \image html "osal/os/priority_mapping_linux.png" "Priority Mapping for Linux"
 *
 * ## ChibiOS/RT (chibios_arm)
 * - Scheduling policies `Other`, `Idle`, and `Batch` are emulated by mapping them to specific fixed priorities (see
 *   figure below).
 * - Scheduling policies `Fifo` and `RR` have the same behavior (both `RR`). Priority is always above the other
 *   policies (see figure below).
 * - System calls (e.g. blocking on a ConditionVariable) do not support deferred cancellation.
 *
 * \htmlonly <style>div.image img[src="osal/os/priority_mapping_chibiosrt.png"]{width:50%;}</style> \endhtmlonly
 * \image html "osal/os/priority_mapping_chibiosrt.png" "Priority Mapping for ChibiOS/RT"
 *
 * ## Time-Flow-Control (linux_x64_tfc, linux_arm_tfc)
 * Full support.\n
 * Be aware that all scheduling policies are mapped to the Linux scheduling policy "OTHER".\n
 * This is not a problem, because TFC pretends that the software is executed on a machine with infinite speed and an
 * infinite number of CPU cores.
 *
 * ## Exception- and Cancellation-safety-notes
 * Each method of this class contains notes about the exception and thread-cancellation safety of the specific method.
 *
 * Note that these specifications often specify a lower guarantee than the actual implementation of this class actually
 * provides. The reason is that this class is operating system specific and portable. The specifications must be so low
 * that they can be fulfilled by any implementation.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Thread final
{
  public:
    /// Type for thread priority levels.
    typedef uint8_t priority_t;

    /// Scheduling policies.
    /** Please refer to chapter "Operating System specific notes" in the documentation of class @ref Thread for more
        information about how the scheduling policies are mapped to specific operating systems. */
    enum class SchedPolicy
    {
      Inherit,      ///<Inherit scheduling policy and priority from the creating thread.
      Other,        ///<Round-robin time-sharing policy with dynamic priority. This is the standard.
      Idle,         ///<Execution of jobs at very low priority.
      Batch,        ///<Round-robin time-sharing policy with dynamic priority for CPU intensive background tasks.
      Fifo,         ///<Real-Time FIFO policy with static priority.
      RR            ///<Real-Time round-robin policy with static priority.
    };

    /**
     * \brief Typedef of functor referencing the thread entry function.
     *
     * - - -
     *
     * Return value:\n
     * A user-defined void-pointer which can be retrieved via @ref Join() after the thread has terminated.
     *
     * - - -
     *
     * __Thread-safety requirements/hints:__\n
     * The referenced function/method will be invoked by the thread managed by this @ref Thread object.\n
     * Class @ref Thread guarantees, that no new thread can be started before the previous one has terminated
     * and joined.
     *
     * __Exception-safety requirements/hints:__\n
     * The referenced function/method shall provide the no-throw guarantee.\n
     * The application will be terminated via @ref gpcc::osal::Panic if an uncaught exception leaves the referenced
     * function/method.
     *
     * __Thread-cancellation-safety requirements/hints:__\n
     * The referenced function/method will be invoked with deferred cancellation enabled by default.\n
     * Deferred cancellation can be disabled by the referenced function/method by invoking
     * @ref SetCancelabilityEnabled().
     */
    typedef std::function<void*(void)> tEntryFunction;


    /// Minimum (lowest) thread priority value.
    static priority_t const minPriority  = 0U;

    /// Maximum (highest) thread priority value.
    static priority_t const maxPriority  = 31U;

    static size_t GetMinStackSize(void);
    static size_t GetStackAlign(void);
    static size_t GetDefaultStackSize(void);


    Thread(void) = delete;
    Thread(std::string const & _name);
    Thread(Thread const &) = delete;
    Thread(Thread &&) = delete;
    ~Thread(void);


    Thread& operator=(Thread const &) = delete;
    Thread& operator=(Thread &&) = delete;

    // general purpose public static methods allowed to be called by ANY thread
    static IThreadRegistry& GetThreadRegistry(void);
    static uint32_t GetPID(void);
    static void Sleep_ms(uint32_t const ms);
    static void Sleep_ns(uint32_t const ns);

    // public methods allowed to be called by ANY thread
    std::string GetName(void) const;
    std::string GetInfo(size_t const nameFieldWidth) const;
    bool IsItMe(void) const;

    // public methods allowed to be called by any thread EXCEPT the one managed by this object
    void Start(tEntryFunction const & _entryFunction,
               SchedPolicy const schedPolicy,
               priority_t const priority,
               size_t const stackSize);
    void Cancel(void);
    void* Join(bool* const pCancelled = nullptr);

    void AdviceTFC_JoiningThreadWillNotBlockPermanently(void);

    // public methods allowed to be called ONLY by the thread managed by this object
    bool SetCancelabilityEnabled(bool const enable);

    bool IsCancellationPending(void) const;
    void TestForCancellation(void);

    NORETURN1 void TerminateNow(void* const threadReturnValue) NORETURN2;

  private:
    /// States of the encapsulated thread.
    enum class ThreadState
    {
      noThreadOrJoined, ///<No thread existing or thread has been joined.
      starting,         ///<Thread is starting.
      running,          ///<Thread is running.
      terminated        ///<Thread has terminated, but not yet joined.
    };


    /// Name of the thread.
    std::string const name;


    /// Mutex protecting access to object's internals.
    /** Locking order: @ref joinMutex -> @ref mutex */
    Mutex mutable mutex;

    /// Mutex used to make @ref Join() thread-safe and to prevent any race between @ref Start() and @ref Join().
    /** Locking order: @ref joinMutex -> @ref mutex */
    Mutex joinMutex;


    /// Functor referencing the thread entry function.
    /** This is used to pass the thread entry function functor from @ref Start() to
        @ref InternalThreadEntry2(). */
    tEntryFunction entryFunction;

    /// Current state of the thread managed by this object.
    /** @ref mutex is required. */
    ThreadState threadState;

    /// Condition variable signaled when @ref threadState is set to @ref ThreadState::running.
    /** This is to be used in conjunction with @ref mutex. */
    ConditionVariable threadStateRunningCondVar;

    /// pthread-handle referencing the thread managed by this object.
    /** @ref mutex is required.\n
        This only contains a valid value if @ref threadState does not equal @ref ThreadState::noThreadOrJoined. */
    pthread_t thread_id;

    /// Thread cancellation pending flag.
    /** true  = thread cancellation is pending\n
        false = thread cancellation is not pending */
    std::atomic<bool> cancellationPending;


    static ThreadRegistry& InternalGetThreadRegistry(void);

    static void* InternalThreadEntry1(void* arg);
    void* InternalThreadEntry2(void);

    int UniversalPrioToSystemPrio(priority_t const priority, SchedPolicy const schedpolicy) const;
};

/**
 * \brief Retrieves a reference to the interface of the application's thread registry.
 *
 * Each application including GPCC and using GPCC's threads will contain one instance of class @ref ThreadRegistry.
 * All @ref Thread instances will register themselves upon creation and unregister themselves upon destruction at
 * the thread registry.
 *
 * The thread registry's public interface (@ref IThreadRegistry) can be retrieved by anybody using this method.
 *
 * \sa @ref IThreadRegistry
 * \sa @ref ThreadRegistry
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This can be invoked by any thread.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Reference to the interface of the application's thread registry.\n
 * The referenced object remains valid until the application terminates.
 */
inline IThreadRegistry& Thread::GetThreadRegistry(void)
{
  return InternalGetThreadRegistry();
}

/**
 * \brief Retrieves the thread's name.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This can be invoked by both the thread managed by this object and by other threads.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * String object containing the thread's name.
 */
inline std::string Thread::GetName(void) const
{
  return name;
}

/**
 * \brief Provides a hint to [TFC](@ref GPCC_TIME_FLOW_CONTROL) that the thread managed by this object, when it is
 *        cancelled, is already blocked in a blocking function that is a canellation point, or that it __will for sure__
 *        hit a cancellation point without being blocked by any activity that requires an increment of the emulated
 *        system time.
 *
 * If the hint is given, then TFC will not increment the emulated system time when a thread joins the thread managed
 * by this object.
 *
 * For details, please refer to @ref GPCC_TIME_FLOW_CONTROL, chapter "Special notes on deferred thread cancellation".
 *
 * \note  This method has no effect in this OSAL variant because TFC is not present.
 *
 * \pre   A thread has been started and the thread has not yet been joined.
 *
 * \pre   The thread has no cancellation request pending.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe, but this must not be called by the thread managed by this object.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 */
inline void Thread::AdviceTFC_JoiningThreadWillNotBlockPermanently(void)
{
  // empty since TFC is not present
}

/**
 * \brief Retrieves if a cancellation request is pending or not.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Only the thread managed by this object is allowed to call this method.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \retval true
 *    A cancellation request is pending.
 * \retval false
 *    No cancellation request pending.
 */
inline bool Thread::IsCancellationPending(void) const
{
  return cancellationPending;
}

} // namespace osal
} // namespace gpcc

#endif // #ifndef THREAD_HPP_201701291628
#endif // #ifdef OS_LINUX_X64
