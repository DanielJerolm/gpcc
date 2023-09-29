/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_ARM

#include <gpcc/osal/Thread.hpp>
#include <gpcc/osal/AdvancedMutexLocker.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <cxxabi.h>
#include <sched.h>
#include <time.h>
#include <unistd.h>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <cerrno>

#define MS_PER_SEC 1000UL
#define NS_PER_MS  1000000UL
#define NS_PER_SEC 1000000000UL

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
: name(_name)
, mutex()
, joinMutex()
, entryFunction()
, threadState(ThreadState::noThreadOrJoined)
, threadStateRunningCondVar()
, thread_id()
, cancelabilityEnabled(false)
, cancellationPending(false)
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
    joinMutex.Lock();
    mutex.Lock();

    if (threadState != ThreadState::noThreadOrJoined)
      Panic("Thread::~Thread: Managed thread not yet joined");

    mutex.Unlock();
    joinMutex.Unlock();

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
 * This is thread-safe.
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
  timespec req = {0, 0};

  if (ms >= MS_PER_SEC)
  {
    req.tv_sec  = ms / MS_PER_SEC;
    req.tv_nsec = (ms % MS_PER_SEC) * NS_PER_MS;
  }
  else
  {
    req.tv_nsec = ms * NS_PER_MS;
  }

  timespec rem;
  while (true)
  {
    int const status = nanosleep(&req, &rem);
    if (status == 0)
    {
      return;
    }
    else if (status == EINTR)
    {
      req = rem;
      continue;
    }
    else
    {
      throw std::system_error(status, std::generic_category(), "Thread::Sleep_ms(): nanosleep failed");
    }
  }
}

/**
 * \brief Suspends execution of the calling thread for a configurable time-span.
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
  timespec req = {0, 0};

  if (ns >= NS_PER_SEC)
  {
    req.tv_sec  = ns / NS_PER_SEC;
    req.tv_nsec = ns % NS_PER_SEC;
  }
  else
  {
    req.tv_nsec = ns;
  }

  timespec rem;
  while (true)
  {
    int const status = nanosleep(&req, &rem);
    if (status == 0)
    {
      return;
    }
    else if (status == EINTR)
    {
      req = rem;
      continue;
    }
    else
    {
      throw std::system_error(status, std::generic_category(), "Thread::Sleep_ns(): nanosleep failed");
    }
  }
}

/**
 * \brief Creates an std::string with information about the managed thread.
 *
 * This method is intended to be used to create human-readable information about the threads registered in the
 * application's thread-registry (see @ref GetThreadRegistry()).
 *
 * Output format:\n
 * <tt>          1         2         3         4         5         6         7         8</tt>\n
 * <tt> 12345678901234567890123456789012345678901234567890123456789012345678901234567890</tt>\n
 * <tt> Name State DS  Scope Policy   prio   Guard   Stack  StackU</tt>\n
 * <tt> ...  no    D   SYS   IH other pppp ggggggg sssssss sssssss</tt>\n
 * <tt>      start J   PRC   EX idle  ?    ?       ?       ?</tt>\n
 * <tt>      run   ?   ?     xx batch</tt>\n
 * <tt>      term            xx FIFO</tt>\n
 * <tt>                      xx RR</tt>\n
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
  std::ostringstream infoLine;

  // start with thread's name
  if (name.size() <= nameFieldWidth)
    infoLine << std::left << std::setw(nameFieldWidth) << std::setfill(' ') << name;
  else
    infoLine << name.substr(0, nameFieldWidth - 3U) << "...";

  MutexLocker mutexLocker(mutex);

  bool detailsRequired = false;

  switch (threadState)
  {
    case ThreadState::noThreadOrJoined:
      infoLine << " no    ";
      break;

    case ThreadState::starting:
      infoLine << " start ";
      break;

    case ThreadState::running:
      infoLine << " run   ";
      detailsRequired = true;
      break;

    case ThreadState::terminated:
      infoLine << " term  ";
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
    status = pthread_attr_getdetachstate(attr, &i);
    if (status == 0)
    {
      if (i == PTHREAD_CREATE_DETACHED)
        infoLine << "D   ";
      else if (i == PTHREAD_CREATE_JOINABLE)
        infoLine << "J   ";
      else
        infoLine << "?   ";
    }
    else
      infoLine << "Err ";

    // Scope (Scheduling scope)
    status = pthread_attr_getscope(attr, &i);
    if (status == 0)
    {
      if (i == PTHREAD_SCOPE_SYSTEM)
        infoLine << "SYS   ";
      else if (i == PTHREAD_SCOPE_PROCESS)
        infoLine << "PRC   ";
      else
        infoLine << "?     ";
    }
    else
      infoLine << "Err   ";

    // Policy (Scheduling policy)
    status = pthread_attr_getinheritsched(attr, &i);
    if (status == 0)
    {
      if (i == PTHREAD_INHERIT_SCHED)
        infoLine << "IH ";
      else if (i == PTHREAD_EXPLICIT_SCHED)
        infoLine << "EX ";
      else
        infoLine << "?  ";
    }
    else
      infoLine << "Err";

    status = pthread_attr_getschedpolicy(attr, &i);
    if (status == 0)
    {
      if (i == SCHED_OTHER)
        infoLine << "other ";
      else if (i == SCHED_IDLE)
        infoLine << "idle  ";
      else if (i == SCHED_BATCH)
        infoLine << "batch ";
      else if (i == SCHED_FIFO)
        infoLine << "FIFO  ";
      else if (i == SCHED_RR)
        infoLine << "RR    ";
      else
        infoLine << "?     ";
    }
    else
      infoLine << "Err   ";

    // priority
    status = pthread_attr_getschedparam(attr, &sp);
    if (status == 0)
      infoLine << std::right << std::setw(4) << std::setfill(' ') << sp.__sched_priority << ' ';
    else
      infoLine << "Err  ";

    // stack guard size
    status = pthread_attr_getguardsize(attr, &s);
    if (status == 0)
      infoLine << std::right << std::setw(7) << std::setfill(' ') << s << ' ';
    else
      infoLine << "Err     ";

    // stack size
    status = pthread_attr_getstacksize(attr, &s);
    if (status == 0)
      infoLine << std::right << std::setw(7) << std::setfill(' ') << s << ' ';
    else
      infoLine << "Err     ";

    // stack usage
    infoLine << "not imp";
  }
  else
  {
    infoLine << "--- ----- -- ----- ---- ------- ------- -------";
  }

  return infoLine.str();
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
  MutexLocker mutexLocker(mutex);

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

  MutexLocker joinMutexLocker(joinMutex);
  MutexLocker mutexLocker(mutex);

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
    if (schedPolicy == SchedPolicy::Inherit)
      status = pthread_attr_setinheritsched(attr, PTHREAD_INHERIT_SCHED);
    else
    {
      status = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);

      if (status == 0)
      {
        switch (schedPolicy)
        {
          case SchedPolicy::Other:
            status = pthread_attr_setschedpolicy(attr, SCHED_OTHER);
            break;

          case SchedPolicy::Idle:
            status = pthread_attr_setschedpolicy(attr, SCHED_IDLE);
            break;

          case SchedPolicy::Batch:
            status = pthread_attr_setschedpolicy(attr, SCHED_BATCH);
            break;

          case SchedPolicy::Fifo:
            status = pthread_attr_setschedpolicy(attr, SCHED_FIFO);
            if (status == 0)
            {
              sched_param sp;
              sp.__sched_priority = UniversalPrioToSystemPrio(priority, SchedPolicy::Fifo);
              status = pthread_attr_setschedparam(attr, &sp);
            }
            break;

          case SchedPolicy::RR:
            status = pthread_attr_setschedpolicy(attr, SCHED_RR);
            if (status == 0)
            {
              sched_param sp;
              sp.__sched_priority = UniversalPrioToSystemPrio(priority, SchedPolicy::RR);
              status = pthread_attr_setschedparam(attr, &sp);
            }
            break;

          default:
            throw std::invalid_argument("Thread::Start: 'schedPolicy' is invalid");
        } // switch (schedPolicy)
      } // if (status == 0)
    } // if (schedPolicy == SP_Inherit)... else...
  } // if (status == 0)

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
  entryFunction         = _entryFunction;
  threadState           = ThreadState::starting;
  cancelabilityEnabled  = true;
  cancellationPending   = false;

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
        threadStateRunningCondVar.Wait(mutex);
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
  MutexLocker mutexLocker(mutex);

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
    if (status != 0)
      throw std::system_error(status, std::generic_category(), "Thread::Cancel: pthread_cancel() failed");

    cancellationPending = true;
  }
}

/**
 * \brief Waits for the thread managed by this object to terminate and joins with it.
 *
 * When joining, the resources of the thread managed by this object are released.\n
 * After joining the object no longer manages a thread and a new one may be started via @ref Start() or the
 * Thread-object may be destroyed.
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
  MutexLocker joinMutexLocker(joinMutex);
  AdvancedMutexLocker mutexLocker(mutex);

  // verify that the object manages a thread, which has not yet been joined
  if (threadState == ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::Join: No thread");

  // verify, that the current thread is not the one managed by this object
  if (pthread_equal(thread_id, pthread_self()) != 0)
    throw std::logic_error("Thread::Join: Thread cannot join itself");

  mutexLocker.Unlock();

  // wait for termination and join with thread
  void* retVal;
  int const status = pthread_join(thread_id, &retVal);
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

  // relock mutex
  try
  {
    mutexLocker.Relock();
  }
  catch (...)
  {
    PANIC();
  }

  // check and update threadState, the object does no longer manage a thread
  if (threadState != ThreadState::terminated)
    PANIC();

  threadState = ThreadState::noThreadOrJoined;

  return retVal;
}

/**
 * \brief Enables/disables cancelability.
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
 * false = cancellation shall be disabled\n
 * Note that if cancelability is disabled, any cancellation request will _not be dropped_ but _queued_ until
 * cancellation is enabled again or until the thread terminates.
 */
void Thread::SetCancelabilityEnabled(bool const enable)
{
  // verify that the current thread is the one managed by this object
  {
    MutexLocker locker(mutex);
    if ((threadState != ThreadState::running) || (pthread_equal(thread_id, pthread_self()) == 0))
      throw std::logic_error("Thread::SetCancelabilityEnabled: Not invoked by the managed thread");
  }

  // requested value different from current one?
  if (cancelabilityEnabled != enable)
  {
    cancelabilityEnabled = enable;

    int status;
    if (enable)
      status = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    else
      status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    if (status != 0)
    {
      cancelabilityEnabled = !cancelabilityEnabled;
      throw std::system_error(status, std::generic_category(), "Thread::SetCancelabilityEnabled: pthread_setcancelstate() failed");
    }
  }
}

/**
 * \brief Retrieves if cancelability is enabled or disabled.
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
 * \return
 * Current cancelability state:\n
 * true  = cancellation enabled\n
 * false = cancellation disabled
 */
bool Thread::GetCancelabilityEnabled(void) const
{
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if ((threadState != ThreadState::running) || (pthread_equal(thread_id, pthread_self()) == 0))
    throw std::logic_error("Thread::GetCancelabilityEnabled: Not invoked by the managed thread");

  return cancelabilityEnabled;
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
    MutexLocker mutexLocker(mutex);

    if ((threadState != ThreadState::running) || (pthread_equal(thread_id, pthread_self()) == 0))
      throw std::logic_error("Thread::TestForCancellation: Not invoked by the managed thread");
  }

  pthread_testcancel();
}

/**
 * \brief This allows the thread managed by this object to terminate itself.
 *
 * The method will never return.
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
    MutexLocker mutexLocker(mutex);

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
    AdvancedMutexLocker mutexLocker(mutex);

    // set threadState to ThreadState::running
    threadState = ThreadState::running;
    threadStateRunningCondVar.Signal();

    // execute user's thread entry function
    mutexLocker.Unlock();
    retVal = entryFunction();
    mutexLocker.Relock();

    // set threadState to ThreadState::terminated
    threadState = ThreadState::terminated;
  }
  catch (abi::__forced_unwind const &)
  {
    // Thread was cancelled using POSIX functionality.
    // Do not forget to set threadState to ThreadState::terminated and rethrow the exception.

    try
    {
      mutex.Lock();
      threadState = ThreadState::terminated;
      mutex.Unlock();
    }
    catch (...)
    {
      PANIC(); // Handling of deferred cancellation (abi::__forced_unwind) failed
    }

    throw;
  }
  catch (...)
  {
    Panic("Thread::InternalThreadEntry2: Local error or uncaught exception from user's thread entry function");
  }

  return retVal;
}

/**
 * \brief Converts the priority levels @ref minPriority ... @ref maxPriority to the priority-range of the system.
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
 * \param priority
 * Priority level.
 *
 * \param schedpolicy
 * Scheduling policy used.\n
 * _This must be SP_Fifo or SP_RR._

 * \return
 * Parameter `priority` projected on the priority-range of the system.\n
 * Example:\n
 * If the system has priority levels from 0..99, then 0 corresponds to 0 and
 * 31 to 99. This means that 15 or 16 would result in approx. 50.
 */
int Thread::UniversalPrioToSystemPrio(priority_t const priority, SchedPolicy const schedpolicy) const
{
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if ((priority < minPriority) || (priority > maxPriority))
    throw std::invalid_argument("Thread::UniversalPrioToSystemPrio: 'priority' is invalid");
  #pragma GCC diagnostic pop

  int min;
  int max;

  if (schedpolicy == SchedPolicy::Fifo)
  {
    min = sched_get_priority_min(SCHED_FIFO);
    max = sched_get_priority_max(SCHED_FIFO);
  }
  else if (schedpolicy == SchedPolicy::RR)
  {
    min = sched_get_priority_min(SCHED_RR);
    max = sched_get_priority_max(SCHED_RR);
  }
  else
  {
    throw std::invalid_argument("Thread::UniversalPrioToSystemPrio: 'schedpolicy' is invalid");
  }

  if (max < min)
    throw std::runtime_error("Thread::UniversalPrioToSystemPrio: maxPrio < minPrio");

  return min + ((static_cast<int>(priority - minPriority) * (max - min)) / (maxPriority - minPriority));
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_LINUX_ARM
