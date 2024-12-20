/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM_TFC

#include <gpcc/osal/Thread.hpp>
#include <gpcc/osal/ConditionVariable.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <gpcc/time/TimePoint.hpp>
#include <gpcc/time/TimeSpan.hpp>
#include "internal/AdvancedUnmanagedMutexLocker.hpp"
#include "internal/TFCCore.hpp"
#include "internal/TimeLimitedThreadBlocker.hpp"
#include "internal/UnmanagedConditionVariable.hpp"
#include "internal/UnmanagedMutex.hpp"
#include "internal/UnmanagedMutexLocker.hpp"
#include <cxxabi.h>
#include <sched.h>
#include <unistd.h>
#include <stdexcept>
#include <system_error>
#include <cerrno>

namespace gpcc {
namespace osal {

Thread::priority_t const Thread::minPriority;
Thread::priority_t const Thread::maxPriority;


// RAII class for pthread_attr_t.
/* Notes:
 * - pthread_attr_t_RAII() initializes an pthread_attr_t via pthread_attr_init().
 * - pthread_attr_t_RAII(pthread_t & thread_id) initializes an pthread_attr_t via pthread_getattr_np().
 * - The pthread_attr_t is destroyed via pthread_attr_destroy() when the RAII object is destroyed.
 */
class pthread_attr_t_RAII final
{
  public:
    pthread_attr_t_RAII(void)
    {
      int const status = pthread_attr_init(&attr);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_attr_t_RAII::pthread_attr_t_RAII: pthread_attr_init() failed");
    }
    explicit pthread_attr_t_RAII(pthread_t const & thread_id)
    {
      int const status = pthread_getattr_np(const_cast<pthread_t&>(thread_id), &attr);
      if (status != 0)
        throw std::system_error(status, std::generic_category(), "pthread_attr_t_RAII::pthread_attr_t_RAII: pthread_getattr_np() failed");
    }
    pthread_attr_t_RAII(pthread_attr_t_RAII const &) = delete;
    pthread_attr_t_RAII(pthread_attr_t_RAII &&) = delete;
    ~pthread_attr_t_RAII(void)
    {
      if (pthread_attr_destroy(&attr) != 0)
        PANIC();
    }

    pthread_attr_t_RAII& operator=(pthread_attr_t_RAII const &) = delete;
    pthread_attr_t_RAII& operator=(pthread_attr_t_RAII &&) = delete;

    operator pthread_attr_t*() { return &attr; }

  private:
    pthread_attr_t attr;
};


/**
 * \brief Queries the minimum stack size.
 *
 * The queried value refers to the minimum stack size required to start a thread. It does not include the stack size
 * required by the thread entry function.
 *
 * The queried value may differ among different implementations of this class for different operating systems.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Minimum stack size in byte.
 */
size_t Thread::GetMinStackSize(void)
{
  // query value only once (sometimes PTHREAD_STACK_MIN is a function)
  static const size_t minStackSize = PTHREAD_STACK_MIN;
  return minStackSize;
}

/**
 * \brief Queries the required stack alignment.
 *
 * The queried value may differ among different implementations of this class for different operating systems.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Required stack alignment in byte.
 */
size_t Thread::GetStackAlign(void)
{
  // query value only once
  static const size_t stackAlign = sysconf(_SC_PAGESIZE);
  return stackAlign;
}

/**
 * \brief Queries the recommended default stack size.
 *
 * The queried value may differ among different implementations of this class for different operating systems.
 *
 * __Notes specific to platforms supporting virtual memory (e.g. Linux):__\n
 * The queried value is _the default_ value on these platforms. It is relatively large and sufficient for virtually any
 * application. It consumes virtual memory only and usually there is plenty of virtual memory available (>= 2GB).\n
 * Physical memory is only consumed according to the actual stack growth.\n
 * However if virtual memory is a concern in your application (e.g. if you have many threads), then you should use your
 * own specific stack size values adapted to your application.
 *
 * __Notes specific to platforms not supporting virtual memory (e.g. ChibiOS/RT):__\n
 * The queried value is only a _suggested default_ stack size. There is no support for virtual memory, so physical
 * memory is immediately consumed when creating a thread.\n
 * The queried value is relatively large in order to meet the stack size requirements of carefully and economically
 * designed embedded code. However there is no guarantee that the stack size requirements of your code are met, and the
 * queried value is likely too large for many applications.\n
 * It is strongly recommended that you should determine your stack size requirements and use your own specific stack
 * size values adapted to the specific needs of your application.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Recommended default stack size in byte for this platform.\n
 * Please pay attention to the notes above!
 */
size_t Thread::GetDefaultStackSize(void)
{
  return 8UL * 1024UL * 1024UL;
}

/**
 * \brief Constructor. Creates an empty thread management object.
 *
 * To start a thread, invoke @ref Start().
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _name
 * Name for the thread managed by this object.
 */
Thread::Thread(std::string const & _name)
: pTFCCore(internal::TFCCore::Get())
, name(_name)
, spMutex(std::make_unique<internal::UnmanagedMutex>())
, spJoinMutex(std::make_unique<internal::UnmanagedMutex>())
, entryFunction()
, threadState(ThreadState::noThreadOrJoined)
, spThreadStateRunningCondVar(std::make_unique<internal::UnmanagedConditionVariable>())
, thread_id()
, threadWaitingForJoin(false)
, cancellationPending(false)
, joiningThreadWillNotBlockPerm(false)
{
  InternalGetThreadRegistry().RegisterThread(*this);
}

/**
 * \brief Destructor.
 *
 * \pre   There is either no thread managed by this object, or the thread has terminated and has been joined.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
Thread::~Thread(void)
{
  try
  {
    spJoinMutex->Lock();
    spMutex->Lock();

    if (threadState != ThreadState::noThreadOrJoined)
      Panic("Thread::~Thread: Managed thread not yet joined");

    spMutex->Unlock();
    spJoinMutex->Unlock();

    InternalGetThreadRegistry().UnregisterThread(*this);
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Retrieves the ID of the process.
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
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * ID of the process.
 */
uint32_t Thread::GetPID(void)
{
  static_assert(sizeof(uint32_t) == sizeof(pid_t), "PID does not fit into uint32_t");
  return static_cast<uint32_t>(getpid());
}

/**
 * \brief Suspends execution of the calling thread for a configurable time-span.
 *
 * __TFC specific information:__\n
 * __This will block the calling thread until the emulated system clock has advanced by the given timespan.__
 * __The system clock will be advanced when all threads in the process are permanently blocked__.\n
 * __See__ @ref GPCC_TIME_FLOW_CONTROL __for details.__
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
 * Strong guarantee.\n
 * On some systems, this method contains a cancellation point.
 *
 * - - -
 *
 * \param ms
 * Time-span (in ms) the calling thread shall be suspended. This is the _minimum time_ the thread will be suspended.
 * The thread may be suspended _longer_ than the specified time-span.
 */
void Thread::Sleep_ms(uint32_t const ms)
{
  using namespace gpcc::time;

  internal::TimeLimitedThreadBlocker blocker;
  internal::UnmanagedMutexLocker mutexLocker(internal::TFCCore::Get()->GetBigLock());
  blocker.Block(TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID) + TimeSpan::ms(ms));
}

/**
 * \brief Suspends execution of the calling thread for a configurable time-span.
 *
 * __TFC specific information:__\n
 * __This will block the calling thread until the emulated system clock has advanced by the given timespan.__
 * __The system clock will be advanced when all threads in the process are permanently blocked__.\n
 * __See__ @ref GPCC_TIME_FLOW_CONTROL __for details.__
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
 * Strong guarantee.\n
 * On some systems, this method contains a cancellation point.
 *
 * - - -
 *
 * \param ns
 * Time-span (in ns) the calling thread shall be suspended. This is the _minimum time_ the thread will be suspended.
 * The thread may be suspended _longer_ than the specified time-span.
 */
void Thread::Sleep_ns(uint32_t const ns)
{
  using namespace gpcc::time;

  internal::TimeLimitedThreadBlocker blocker;
  internal::UnmanagedMutexLocker mutexLocker(internal::TFCCore::Get()->GetBigLock());
  blocker.Block(TimePoint::FromSystemClock(gpcc::osal::ConditionVariable::clockID) + TimeSpan::ns(ns));
}

/**
 * \brief Creates an std::string with information about the managed thread.
 *
 * This method is intended to be used to create human-readable information about the threads registered in the
 * application's thread-registry (see @ref GetThreadRegistry()).
 *
 * Output format:
 * ~~~{.txt}
 *          1         2         3         4         5         6         7         8
 * 12345678901234567890123456789012345678901234567890123456789012345678901234567890
 * Name State DS  Scope Policy   prio   Guard   Stack  StackU
 * ...  no    D   SYS   IH other pppp ggggggg sssssss sssssss
 *      start J   PRC   EX idle  ?    ?       ?       ?
 *      run   ?   ?     xx batch
 *      term            xx FIFO
 *                      xx RR
 * ~~~
 *
 * States:\n
 * no    = thread (currently) not registered at OS or thread has been joined\n
 * start = thread is starting\n
 * run   = thread is running (but may be blocked on a @ref Mutex, @ref Semaphore, ...)\n
 * term  = thread has terminated, but not yet been joined
 *
 * DS (Detach state)\n
 * D     = detached\n
 * J     = joinable
 *
 * Scope (Scheduling scope)\n
 * SYS   = system\n
 * PRC   = process
 *
 * Policy (Scheduling policy)\n
 * IH    = Inherited\n
 * EX    = Explicit
 *
 * other = Round-robin time-sharing policy with dynamic priority.\n
 * idle  = Execution of jobs at very low priority.\n
 * batch = Round-robin time-sharing policy with dynamic priority for CPU intensive background tasks.\n
 * FIFO  = Real-Time FIFO policy with static priority.\n
 * RR    = Real-Time round-robin policy with static priority.
 *
 * prio (priority)\n
 * This is only valid for scheduling policies "FIFO" and "RR".\n
 * The value is system specific. Note that the value passed to @ref Start() (0 (low) .. 31 (high))\n
 * has been mapped to the system specific priority values.
 *
 * Guard (stack guard size)
 *
 * Stack (stack size)
 *
 * StackU (stack usage)
 *
 * Note that the output format may be different among the different implementations of class
 * @ref Thread provided by GPCC for different operating systems.
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
 * \param nameFieldWidth
 * Desired width of the "name"-field (see description of output format above).\n
 * The content of the "name"-field is either extended with white spaces or the displayed name is truncated in order
 * to match this parameter.\n
 * This value does not include the separating white-space between the "name"-field and the "state"-field.\n
 * Minimum allowed value: 4
 *
 * \return
 * An std::string containing information about the current thread, formatted as described above.
 */
std::string Thread::GetInfo(size_t const nameFieldWidth) const
{
  if (nameFieldWidth < 4U)
    throw std::invalid_argument("Thread::GetInfo: 'nameFieldWidth' too small");

  // the information is build into "infoLine"
  using gpcc::string::StringComposer;
  StringComposer infoLine;
  infoLine << StringComposer::AlignLeft;

  // start with thread's name
  if (name.size() <= nameFieldWidth)
    infoLine << StringComposer::Width(nameFieldWidth) << name;
  else
    infoLine << name.substr(0, nameFieldWidth - 3U) << "...";

  internal::UnmanagedMutexLocker mutexLocker(*spMutex);

  bool detailsRequired = false;

  infoLine << ' ' << StringComposer::Width(6);
  switch (threadState)
  {
    case ThreadState::noThreadOrJoined:
      infoLine << "no";
      break;

    case ThreadState::starting:
      infoLine << "start";
      break;

    case ThreadState::running:
      infoLine << "run";
      detailsRequired = true;
      break;

    case ThreadState::terminated:
      infoLine << "term";
      break;
  } // switch (threadState)

  if (detailsRequired)
  {
    pthread_attr_t_RAII attr(thread_id);

    int status;
    int i;
    size_t s;
    struct sched_param sp;

    // DS (Detach state)
    infoLine << StringComposer::Width(4);
    status = pthread_attr_getdetachstate(attr, &i);
    if (status == 0)
    {
      if (i == PTHREAD_CREATE_DETACHED)
        infoLine << 'D';
      else if (i == PTHREAD_CREATE_JOINABLE)
        infoLine << 'J';
      else
        infoLine << '?';
    }
    else
      infoLine << "Err";

    // Scope (Scheduling scope)
    infoLine << StringComposer::Width(6);
    status = pthread_attr_getscope(attr, &i);
    if (status == 0)
    {
      if (i == PTHREAD_SCOPE_SYSTEM)
        infoLine << "SYS";
      else if (i == PTHREAD_SCOPE_PROCESS)
        infoLine << "PRC";
      else
        infoLine << '?';
    }
    else
      infoLine << "Err";

    // Policy (Scheduling policy)
    infoLine << StringComposer::Width(3);
    status = pthread_attr_getinheritsched(attr, &i);
    if (status == 0)
    {
      if (i == PTHREAD_INHERIT_SCHED)
        infoLine << "IH";
      else if (i == PTHREAD_EXPLICIT_SCHED)
        infoLine << "EX";
      else
        infoLine << '?';
    }
    else
      infoLine << "Err";

    infoLine << StringComposer::Width(6);
    status = pthread_attr_getschedpolicy(attr, &i);
    if (status == 0)
    {
      if (i == SCHED_OTHER)
        infoLine << "other";
      else if (i == SCHED_IDLE)
        infoLine << "idle";
      else if (i == SCHED_BATCH)
        infoLine << "batch";
      else if (i == SCHED_FIFO)
        infoLine << "FIFO";
      else if (i == SCHED_RR)
        infoLine << "RR";
      else
        infoLine << '?';
    }
    else
      infoLine << "Err";

    // priority
    infoLine << StringComposer::AlignRight << StringComposer::Width(4);
    status = pthread_attr_getschedparam(attr, &sp);
    if (status == 0)
      infoLine << sp.__sched_priority;
    else
      infoLine << "Err";
    infoLine << ' ';

    // stack guard size
    infoLine << StringComposer::Width(7);
    status = pthread_attr_getguardsize(attr, &s);
    if (status == 0)
      infoLine << s;
    else
      infoLine << "Err";
    infoLine << ' ';

    // stack size
    infoLine << StringComposer::Width(7);
    status = pthread_attr_getstacksize(attr, &s);
    if (status == 0)
      infoLine << s;
    else
      infoLine << "Err";
    infoLine << ' ';

    // stack usage
    infoLine << "not imp";
  }
  else
  {
    infoLine << "--- ----- -- ----- ---- ------- ------- -------";
  }

  return infoLine.Get();
}

/**
 * \brief Retrieves if the calling thread is the thread managed by this object.
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
 * \retval true
 *    The thread executing this method __is__ the one managed by this object.
 * \retval false
 *    The thread executing this method __is not__ the one managed by this object.
 */
bool Thread::IsItMe(void) const
{
  internal::UnmanagedMutexLocker mutexLocker(*spMutex);

  if (threadState == ThreadState::running)
    return (pthread_equal(thread_id, pthread_self()) != 0);
  else
    return false;
}

/**
 * \brief Creates a new thread and starts execution of the thread entry function.
 *
 * By default the new thread has deferred thread cancelability enabled.\n
 * The new thread may change cancelability via @ref SetCancelabilityEnabled().
 *
 * __TFC specific information:__\n
 * __Scheduling policy and priority values passed to this method are ignored.__\n
 * __The thread will be scheduled using the Linux scheduling policy "OTHER".__\n
 * __This is not a problem, because TFC pretends that the software is executed on a machine with__
 * __infinite speed and an infinite number of CPU cores.__
 *
 * \pre   There is either no thread managed by this object, or the thread has terminated and has been joined.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param _entryFunction
 * Unmodifiable reference to a functor referencing the thread entry function.\n
 * _A copy is created._\n
 * See @ref tEntryFunction for details.
 *
 * \param schedPolicy
 * Scheduling policy that shall be applied to the new thread:\n
 * - @ref SchedPolicy::Inherit
 * - @ref SchedPolicy::Other
 * - @ref SchedPolicy::Idle
 * - @ref SchedPolicy::Batch
 * - @ref SchedPolicy::Fifo
 * - @ref SchedPolicy::RR
 *
 * \param priority
 * Priority level for the new thread: 0 (low) .. 31 (high)\n
 * This is only relevant for the scheduling policies @ref SchedPolicy::Fifo and @ref SchedPolicy::RR. \n
 * _For the other scheduling policies this parameter is not relevant and must be zero._
 *
 * \param stackSize
 * Size of the stack of the new thread in byte.\n
 * _This must be a multiple of_ @ref Thread::GetStackAlign(). \n
 * _This must be equal to or larger than_ @ref Thread::GetMinStackSize(). \n
 * On some platforms the final stack size might be larger than this, e.g. due to interrupt handling requirements.
 */
void Thread::Start(tEntryFunction const & _entryFunction,
                   SchedPolicy const schedPolicy,
                   priority_t const priority,
                   size_t const stackSize)
{
  // check parameters
  if (!_entryFunction)
    throw std::invalid_argument("Thread::Start: '_entryFunction' refers to nothing");

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if ((priority < minPriority) || (priority > maxPriority))
    throw std::invalid_argument("Thread::Start: 'priority' is out of bounds");
  #pragma GCC diagnostic pop

  if ((priority != 0U) && (schedPolicy != SchedPolicy::Fifo) && (schedPolicy != SchedPolicy::RR))
    throw std::invalid_argument("Thread::Start: Selected scheduling policy requires priority level 0");

  if ((stackSize < GetMinStackSize()) || ((stackSize % GetStackAlign()) != 0U))
    throw std::invalid_argument("Thread::Start: 'stackSize' is invalid");

  internal::UnmanagedMutexLocker joinMutexLocker(*spJoinMutex);
  internal::UnmanagedMutexLocker mutexLocker(*spMutex);

  // check that there is currently no thread
  if (threadState != ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::Start: There is already a thread");

  // create a thread attributes object and apply desired settings
  pthread_attr_t_RAII attr;
  int status;

  status = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);

  if (status == 0)
    status = pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);

  if (status == 0)
  {
    // Note: "schedPolicy" and "priority" are ignored.
    // In a TFC environment, all threads are scheduled using SCHED_OTHER.
    // Scheduling details are don't care in a TFC environment, because TFC pretends that the
    // software is executed on a machine with infinite speed and an infinite number of CPU cores.
    status = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
    if (status == 0)
      status = pthread_attr_setschedpolicy(attr, SCHED_OTHER);
  }

  if (status == 0)
    status = pthread_attr_setstacksize(attr, stackSize);

  if (status != 0)
    throw std::runtime_error("Thread::Start: Scheduling policy and/or settings not supported");

  // create short name for "pthread_setname_np"
  std::string shortName = name;
  if (shortName.length() > 15U)
    shortName.resize(15);
  char const * const pShortName = shortName.c_str();

  // prepare thread start
  entryFunction                 = _entryFunction;
  threadState                   = ThreadState::starting;
  threadWaitingForJoin          = false;
  cancellationPending           = false;
  joiningThreadWillNotBlockPerm = false;

  // tell TFC that there will be a new thread
  try
  {
    internal::UnmanagedMutexLocker tfcMutexLocker(pTFCCore->GetBigLock());
    pTFCCore->ReportNewThread();
  }
  catch (...)
  {
    PANIC();
  }

  // create and start thread
  status = pthread_create(&thread_id, attr, Thread::InternalThreadEntry1, this);

  // success?
  if (status == 0)
  {
    // disable thread cancellation (of the thread executing this, NOT the new thread) temporarily
    int oldState;
    if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState) != 0)
      PANIC();

    // assign name to thread
    if (!name.empty())
    {
      status = pthread_setname_np(thread_id, pShortName);
      if (status != 0)
        PANIC();
    }

    // wait until the new thread leaves the starting-state
    try
    {
      while (threadState == ThreadState::starting)
        spThreadStateRunningCondVar->Wait(*spMutex);
    }
    catch (...)
    {
      PANIC();
    }

    // recover previous cancelability state
    if (pthread_setcancelstate(oldState, nullptr) != 0)
      PANIC();
  } // if (status == 0)
  else
  {
    // inform TFC that the new thread is gone
    try
    {
      internal::UnmanagedMutexLocker tfcMutexLocker(pTFCCore->GetBigLock());
      pTFCCore->ReportThreadTermination();
    }
    catch (...)
    {
      PANIC();
    }

    threadState = ThreadState::noThreadOrJoined;

    switch (status)
    {
      case EAGAIN:
        throw std::runtime_error("Thread::Start: Out of resources");

      case EINVAL:
        throw std::runtime_error("Thread::Start: Scheduling policy and/or settings not supported (pthread_create)");

      case EPERM:
        throw std::runtime_error("Thread::Start: Insufficient permissions");

      default:
        throw std::runtime_error("Thread::Start: Unspecific error");
    }
  } // if (status == 0)... else...
}

/**
 * \brief Requests cancellation of the thread managed by this object.
 *
 * Note that the cancelability state of the thread managed by this object determines when exactly the cancellation
 * request will take effect. If cancelability is disabled, then the cancellation request will be deferred until
 * cancelability is enabled. The managed thread can change its cancelability state via
 * @ref SetCancelabilityEnabled().
 *
 * In any case, cancellation of the thread occurs asynchronously with respect to returning from this method.
 *
 * __TFC specific information:__\n
 * __TFC's dead lock detection will be disabled until the thread has terminated!__
 *
 * \pre   A thread has been started and the thread has not yet been joined.
 *
 * \pre   This has not yet been called for the thread managed by this @ref Thread instance.
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
 * Strong guarantee.
 */
void Thread::Cancel(void)
{
  internal::UnmanagedMutexLocker mutexLocker(*spMutex);

  // verify that the object manages a thread, which has not yet been joined
  if (threadState == ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::Cancel: No thread");

  // not yet terminated?
  if (threadState != ThreadState::terminated)
  {
    // verify, that the current thread is not the one managed by this object
    if (pthread_equal(thread_id, pthread_self()) != 0)
      throw std::logic_error("Thread::Cancel: Invoked by the managed thread");

    // verify, that cancellation of the thread has not yet been requested
    if (cancellationPending)
      throw std::logic_error("Thread::Cancel: Cancellation already requested");

    // cancel thread
    int const status = pthread_cancel(thread_id);
    if (status == 0)
    {
      // inform TFC about cancellation request
      try
      {
        internal::UnmanagedMutexLocker tfcMutexLocker(pTFCCore->GetBigLock());
        pTFCCore->ReportThreadCancellationRequested();

        cancellationPending = true;
      }
      catch (...)
      {
        PANIC();
      }
    }
    else
    {
      throw std::system_error(status, std::generic_category(), "Thread::Cancel: pthread_cancel() failed");
    }
  }
}

/**
 * \brief Waits for the thread managed by this object to terminate and joins with it.
 *
 * When joining, the resources of the thread managed by this object are released.\n
 * After joining the object no longer manages a thread and a new one may be started via @ref Start() or the
 * Thread-object may be destroyed.
 *
 * __TFC specific information:__\n
 * Joining a thread may consume emulated system time. See @ref GPCC_TIME_FLOW_CONTROL, chapter "Special notes on
 * deferred thread cancellation" for details.
 *
 * \pre   A thread has been started and it has not yet been joined.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param pCancelled
 * The referenced variable is set to _false_, if the joined thread has terminated by itself.\n
 * The referenced variable is set to _true_, if the joined thread hit a cancellation point and was cancelled due to a
 * pending cancellation request.\n
 * This may be `nullptr`, if this information is not interesting.
 *
 * \return
 * Void-pointer returned by the entry function of the terminated thread or void-pointer passed to
 * @ref TerminateNow(). \n
 * _nullptr, if the joined thread was cancelled._
 */
void* Thread::Join(bool* const pCancelled)
{
  internal::UnmanagedMutexLocker joinMutexLocker(*spJoinMutex);
  internal::AdvancedUnmanagedMutexLocker mutexLocker(*spMutex);

  // verify that the object manages a thread, which has not yet been joined
  if (threadState == ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::Join: No thread");

  // verify, that the current thread is not the one managed by this object
  if (pthread_equal(thread_id, pthread_self()) != 0)
    throw std::logic_error("Thread::Join: Thread cannot join itself");

  // check if thread has already terminated
  bool const alreadyTerminated = (threadState == ThreadState::terminated);

  // if the joining thread will block, then inform TFC
  if ((!alreadyTerminated) && (!joiningThreadWillNotBlockPerm))
  {
    internal::UnmanagedMutexLocker bigLockLocker(pTFCCore->GetBigLock());
    pTFCCore->ReportThreadPermanentlyBlockedBegin();
  }

  // join with thread
  threadWaitingForJoin = true;
  mutexLocker.Unlock();

  ON_SCOPE_EXIT(DeferredCancellationDuring_pthread_join)
  {
    mutexLocker.Relock();

    // we are no longer waiting to join
    threadWaitingForJoin = false;

    // If we told TFC that this thread is blocked before we attempted to join with the managed thread,
    // then we have to inform TFC now, that this thread is no longer blocked.
    if ((!alreadyTerminated) && (!joiningThreadWillNotBlockPerm))
    {
      internal::UnmanagedMutexLocker bigLockLocker(pTFCCore->GetBigLock());

      // If the managed thread has not yet terminated in the meantime, then we have to announce that
      // this thread is about to wake up
      if (threadState != ThreadState::terminated)
        pTFCCore->ReportThreadAboutToWakeUp();

      // we are no longer blocked
      pTFCCore->ReportThreadPermanentlyBlockedEnd();
    }
  };

  // Join with the managed thread.
  // pthread_join() guarantees, that the managed thread will NOT be joined if deferred cancellation occurs
  // while the current thread is blocked in pthread_join().
  void* retVal;
  int const status = pthread_join(thread_id, &retVal);

  ON_SCOPE_EXIT_DISMISS(DeferredCancellationDuring_pthread_join);

  // anything that goes wrong now cannot be recovered...
  ON_SCOPE_EXIT(errorHandling) { PANIC(); };

  mutexLocker.Relock();
  threadWaitingForJoin = false;

  // inform TFC
  if (status == 0)
  {
    // (thread successfully joined)
    internal::UnmanagedMutexLocker bigLockLocker(pTFCCore->GetBigLock());

    // if we really blocked then tell TFC that we have woken up
    if ((!alreadyTerminated) && (!joiningThreadWillNotBlockPerm))
      pTFCCore->ReportThreadPermanentlyBlockedEnd();

    // note: leaving the thread entry function is treated as permanent blocking
    // (see InternalThreadEntry2() for details)
    pTFCCore->ReportThreadAboutToWakeUp();
    pTFCCore->ReportThreadPermanentlyBlockedEnd();
    pTFCCore->ReportThreadTermination();
  }
  else
  {
    // (pthread_join() failed: The thread was NOT joined)

    // If we told TFC that this thread is blocked before we attempted to join with the managed thread,
    // then we have to inform TFC now, that this thread is no longer blocked.
    if ((!alreadyTerminated) && (!joiningThreadWillNotBlockPerm))
    {
      internal::UnmanagedMutexLocker bigLockLocker(pTFCCore->GetBigLock());

      // If the managed thread has not yet terminated in the meantime, then we have to announce that
      // this thread is about to wake up
      if (threadState != ThreadState::terminated)
        pTFCCore->ReportThreadAboutToWakeUp();

      // we are no longer blocked
      pTFCCore->ReportThreadPermanentlyBlockedEnd();
    }
  }

  // the hot stuff is done
  ON_SCOPE_EXIT_DISMISS(errorHandling);

  // error?
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "Thread::Join: pthread_join() failed");

  // thread cancelled?
  if (retVal == PTHREAD_CANCELED)
  {
    if (pCancelled != nullptr)
      *pCancelled = true;
    retVal = nullptr;
  }
  else if (pCancelled != nullptr)
    *pCancelled = false;

  // check and update threadState, the object does no longer manage a thread
  if (threadState != ThreadState::terminated)
    PANIC();

  threadState = ThreadState::noThreadOrJoined;

  return retVal;
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
void Thread::AdviceTFC_JoiningThreadWillNotBlockPermanently(void)
{
  internal::UnmanagedMutexLocker mutexLocker(*spMutex);

  // verify that the object manages a thread, which has not yet been joined
  if (threadState == ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::AdviceTFC_JoiningThreadWillNotBlockPermanently: No thread");

  // verify, that the current thread is not the one managed by this object
  if (pthread_equal(thread_id, pthread_self()) != 0)
    throw std::logic_error("Thread::AdviceTFC_JoiningThreadWillNotBlockPermanently: Thread cannot give advice about itself");

  if (cancellationPending)
    throw std::logic_error("Thread::AdviceTFC_JoiningThreadWillNotBlockPermanently: Cancellation request already pending");

  joiningThreadWillNotBlockPerm = true;
}

/**
 * \brief Enables/disables cancelability and retrieves the previous state.
 *
 * This function has no effect, if the current cancelability state already equals @p enable.
 *
 * Note that if cancelability is disabled, any cancellation request will _not be dropped_ but _queued_ until
 * cancellation is enabled again or until the thread terminates.
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
 * \param enable
 * New cancelability state:\n
 * true = cancellation shall be enabled\n
 * false = cancellation shall be disabled
 *
 * \return
 * Previous cancelability state.\n
 * This could be stored and used to recover the previous state at a later point in time, e.g. if cancelability shall
 * be disabled temporarily only.
 */
bool Thread::SetCancelabilityEnabled(bool const enable)
{
  // verify that the current thread is the one managed by this object
  {
    internal::UnmanagedMutexLocker locker(*spMutex);
    if ((threadState != ThreadState::running) || (pthread_equal(thread_id, pthread_self()) == 0))
      throw std::logic_error("Thread::SetCancelabilityEnabled: Not invoked by the managed thread");
  }

  int oldstate;
  int const status = pthread_setcancelstate(enable ? PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE, &oldstate);

  if (status != 0)
    throw std::system_error(status, std::generic_category(), "Thread::SetCancelabilityEnabled: pthread_setcancelstate() failed");

  return (oldstate == PTHREAD_CANCEL_ENABLE);
}

/**
 * \brief This is an explicit cancellation point for the thread managed by this object.
 *
 * If a cancellation request is pending for the thread managed by this object, and if thread cancelability is enabled,
 * then this method will never return.
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
 * Strong guarantee.\n
 * This method is an explicit cancellation point.
 */
void Thread::TestForCancellation(void)
{
  // verify that the current thread is the one managed by this object
  {
    internal::UnmanagedMutexLocker mutexLocker(*spMutex);

    if ((threadState != ThreadState::running) || (pthread_equal(thread_id, pthread_self()) == 0))
      throw std::logic_error("Thread::TestForCancellation: Not invoked by the managed thread");
  }

  pthread_testcancel();
}

/**
 * \brief This allows the thread managed by this object to terminate itself.
 *
 * This method will never return.
 *
 * __Note:__\n
 * __Stack-unwinding will take place.__\n
 * This means that all objects created on the stack will be released during thread termination. This includes RAII
 * objects like automatic mutex lockers etc.
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
 * Strong guarantee.
 *
 * - - -
 *
 * \param threadReturnValue
 * A void-pointer that will be returned by @ref Join().
 */
void Thread::TerminateNow(void* const threadReturnValue)
{
  // verify that the current thread is the one managed by this object
  {
    internal::UnmanagedMutexLocker mutexLocker(*spMutex);

    if ((threadState != ThreadState::running) || (pthread_equal(thread_id, pthread_self()) == 0))
      throw std::logic_error("Thread::TerminateNow: Not invoked by the managed thread");
  }

  pthread_exit(threadReturnValue);
}

/**
 * \brief Retrieves a reference to the application's thread registry.
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
 * Reference to the application's thread registry.
 */
ThreadRegistry& Thread::InternalGetThreadRegistry(void)
{
  static ThreadRegistry threadRegistry;
  return threadRegistry;
}

/**
 * \brief Internal thread entry function (step 1).
 *
 * This is executed by the pthread-library upon thread creation. This is a static member function because the pthread-
 * library does not know anything about this-pointers. Parameter 'arg' is the pointer to the Thread-object managing
 * the thread executing this function. This function reconstructs the this-pointer from parameter 'arg' and invokes
 * @ref InternalThreadEntry2(), which is a non-static member function.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant for different values of parameter `arg`.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * The only exception which could be thrown is the special exception `abi::__forced_unwind` which is used by
 * pthread-library to implement deferred thread cancellation.
 *
 * __Thread cancellation safety:__\n
 * Welcome and properly handled.
 *
 * - - -
 *
 * \param arg
 * Argument required by POSIX carrying application specific information.\n
 * Here it points to the @ref Thread object managing the calling thread.
 *
 * \return
 * The return value of the thread entry function.
 */
void* Thread::InternalThreadEntry1(void* arg)
{
  if (arg == nullptr)
    PANIC();

  return static_cast<Thread*>(arg)->InternalThreadEntry2();
}

/**
 * \brief Internal thread entry function (step 2).
 *
 * See @ref InternalThreadEntry1() for details.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Program logic ensures that there is only one thread per instance of class @ref Thread.
 *
 * __Exception safety:__\n
 * Strong guarantee.\n
 * The only exception which could be thrown is the special exception `abi::__forced_unwind` which is used by
 * pthread-library to implement deferred thread cancellation.
 *
 * __Thread cancellation safety:__\n
 * Welcome and properly handled.
 *
 * - - -
 *
 * \return
 * The return value of the thread entry function.
 */
void* Thread::InternalThreadEntry2(void)
{
  void* retVal = nullptr;

  try
  {
    internal::AdvancedUnmanagedMutexLocker mutexLocker(*spMutex);

    // set threadState to ThreadState::running
    threadState = ThreadState::running;
    spThreadStateRunningCondVar->Signal();

    // execute user's thread entry function
    mutexLocker.Unlock();
    retVal = entryFunction();
    mutexLocker.Relock();

    // set threadState to ThreadState::terminated
    threadState = ThreadState::terminated;

    // Inform TFC:
    // - If a thread is blocked inside pthread_join(), then tell TFC that the blocked thread is about to wake up
    // - If a cancellation request is pending, then tell TFC that the cancellation has taken place
    // - Finally tell TFC that this thread is going to block. The thread which will join with this thread will
    //   tell TFC that this thread has woken up again and terminated. This small indirection is necessary,
    //   because a thread cannot tell TFC itself that it has terminated.
    internal::UnmanagedMutexLocker tfcMutexLocker(pTFCCore->GetBigLock());
    if ((threadWaitingForJoin) && (!joiningThreadWillNotBlockPerm))
      pTFCCore->ReportThreadAboutToWakeUp();

    if (cancellationPending)
      pTFCCore->ReportThreadCancellationDone();

    pTFCCore->ReportThreadPermanentlyBlockedBegin();

    mutexLocker.Unlock();
  }
  catch (abi::__forced_unwind const &)
  {
    // Thread was cancelled using POSIX functionality.
    try
    {
      spMutex->Lock();

      // update state
      threadState = ThreadState::terminated;

      // Inform TFC:
      // - If a thread is blocked inside pthread_join(), then tell TFC that the blocked thread is about to wake up
      // - If a cancellation request is pending, then tell TFC that the cancellation has taken place
      // - Finally tell TFC that this thread is going to block. The thread which will join with this thread will
      //   tell TFC that this thread has woken up again and terminated. This small indirection is necessary,
      //   because a thread cannot tell TFC itself that it has terminated.
      internal::UnmanagedMutexLocker tfcMutexLocker(pTFCCore->GetBigLock());
      if ((threadWaitingForJoin) && (!joiningThreadWillNotBlockPerm))
        pTFCCore->ReportThreadAboutToWakeUp();

      if (cancellationPending)
        pTFCCore->ReportThreadCancellationDone();

      pTFCCore->ReportThreadPermanentlyBlockedBegin();

      spMutex->Unlock();
    }
    catch (...)
    {
      PANIC(); // Handling of deferred cancellation (abi::__forced_unwind) failed
    }

    throw;
  }
  catch (std::exception const & e)
  {
    Panic("Thread::InternalThreadEntry2: Caught exception: ", e);
  }
  catch (...)
  {
    Panic("Thread::InternalThreadEntry2: Caught unknown exception");
  }

  return retVal;
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM_TFC
