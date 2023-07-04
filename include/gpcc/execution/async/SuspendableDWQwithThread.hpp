/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#ifndef SUSPENDABLEDWQWITHTTHREAD_HPP_202305112025
#define SUSPENDABLEDWQWITHTTHREAD_HPP_202305112025

#include <gpcc/execution/async/DeferredWorkQueue.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Mutex.hpp>
#include <gpcc/osal/Thread.hpp>
#include <string>

namespace gpcc      {
namespace execution {
namespace async     {

/**
 * \ingroup GPCC_EXECUTION_ASYNC
 * \brief Provides a @ref DeferredWorkQueue and one [Thread](@ref gpcc::osal::Thread) driving the work queue. Work
 *        package execution can be suspended and resumed.
 *
 * This is a convenient class for clients. Each work queue usually also requires a thread. Using this class, clients do
 * not need to setup a thread themselves.
 *
 * This class does not expect work packages to throw. If a work package throws, then this class will
 * [panic](@ref GPCC_OSAL_PANIC).
 *
 * The work packages are executed with deferred thread cancellation disabled. The @ref Stop() method will stop the
 * thread after a work package currently in progress has completed. Until then the @ref Stop() method blocks.
 *
 * In addition to starting and stopping the thread, work package execution can also be suspended and resumed via
 * @ref Suspend() and @ref Resume(). After starting the thread, work package execution is initially suspended.
 *
 * Usage example:
 * ~~~{.cpp}
 * auto spDWQ = std::make_unique<SuspendableDWQwithThread>("MyThread");
 *
 * // start the work queue
 * spDWQ->Start(Thread::SchedPolicy::Other, 0U, Thread::GetDefaultStackSize()));
 * spDWQ->Resume());
 *
 * // put work packages into the work queue and watch them being executed
 * // [...]
 *
 * spDWQ->Suspend();
 *
 * // Work packages are not executed any more.
 * // More work packages could be added to the work queue, but they won't be executed.
 * // [...]
 *
 * spDWQ->Resume();
 *
 * // work packages are executed again
 * // [...]
 *
 * // Stop work queue's thread and destroy the object. A call to Suspend() is optional.
 * spDWQ->Stop();
 * spDWQ.reset();
 * ~~~
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class SuspendableDWQwithThread final
{
  public:
    SuspendableDWQwithThread(void) = delete;
    SuspendableDWQwithThread(std::string const & threadName);
    SuspendableDWQwithThread(SuspendableDWQwithThread const &) = delete;
    SuspendableDWQwithThread(SuspendableDWQwithThread&&) = delete;
    ~SuspendableDWQwithThread(void);

    SuspendableDWQwithThread& operator=(SuspendableDWQwithThread const &) = delete;
    SuspendableDWQwithThread& operator=(SuspendableDWQwithThread&&) = delete;

    void Start(gpcc::osal::Thread::SchedPolicy const schedPolicy,
               gpcc::osal::Thread::priority_t const priority,
               size_t const stackSize);
    void Stop(void) noexcept;

    void Suspend(void);
    void Resume(void);

    DeferredWorkQueue& GetDWQ(void) noexcept;

  private:
    /// Enum with combined control- and status-values.
    enum class CtrlStat
    {
      noThread,     ///<No thread existing.
      suspendReq,   ///<Thread shall be parked outside the work queue.
      suspended,    ///<Thread is suspended (= parked outside the workqueue)
      startReq,     ///<Thread shall enter work queue's Work()-function.
      running,      ///<Thread is inside the work queue's Work()-function.
      terminateReq  ///<Thread shall terminate.
    };


    /// Deferred work queue instance.
    DeferredWorkQueue dwq;

    /// Thread used to drive @ref dwq.
    gpcc::osal::Thread thread;

    /// Mutex used to protect the API.
    /** Locking order: @ref apiMutex -> @ref mutex */
    gpcc::osal::Mutex apiMutex;

    /// Mutex used to protect the internal state.
    /** Locking order: @ref apiMutex -> @ref mutex */
    gpcc::osal::Mutex mutex;

    /// Signals a change of @ref ctrlStat.
    gpcc::osal::ConditionVariable cvCtrlStatEvent;

    /// Combined control/status.
    /** @ref mutex required. */
    CtrlStat ctrlStat;


    void* ThreadEntry(void) noexcept;
};

/**
 * \brief Retrieves a reference to the encapsulated @ref DeferredWorkQueue.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * Reference to the encapsulated  @ref DeferredWorkQueue instance.\n
 * The object's life-span is coupled to the life-span of the @ref SuspendableDWQwithThread instance.
 */
inline DeferredWorkQueue& SuspendableDWQwithThread::GetDWQ(void) noexcept
{
  return dwq;
}

} // namespace async
} // namespace execution
} // namespace gpcc

#endif // SUSPENDABLEDWQWITHTTHREAD_HPP_202305112025
