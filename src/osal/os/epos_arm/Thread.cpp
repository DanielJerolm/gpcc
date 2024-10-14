/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifdef OS_EPOS_ARM

#include <gpcc/osal/Thread.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <stdexcept>
#include <system_error>
#include <cerrno>


namespace gpcc {
namespace osal {

Thread::priority_t const Thread::minPriority;
Thread::priority_t const Thread::maxPriority;


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
  return EPOS_THREAD_MINIMUMSTACKSIZE;
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
  return EPOS_THREAD_REQUIREDSTACKALIGN;
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
  return 8UL * 1024UL;
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
, pThread(nullptr)
, cancellationRequestedViaThisAPI(false)
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
  {
    MutexLocker joinMutexLocker(joinMutex);
    MutexLocker mutexLocker(mutex);

    if (threadState != ThreadState::noThreadOrJoined)
      Panic("Thread::~Thread: Precons");
  }

  InternalGetThreadRegistry().UnregisterThread(*this);
}

/**
 * \brief Creates an std::string with information about the managed thread.
 *
 * This method is intended to be used to create human-readable information about the threads registered in the
 * application's thread-registry (see @ref GetThreadRegistry()).
 *
 * Example format:\n
 * (Note: The header is not generated by this)\n
 * ~~~{.txt}
 * Name             State Prio ePrio Timeslice Stacksize     Used        [First-----End[\n
 * -------------------------------------------------------------------------------------------
 * ...              run    ppp   ppp      x ms  ssssssss ssssssss (xxx%) 0xXXXXXXXX-0xXXXXXXXX
 * ~~~
 *
 * Note that the output format may be different among the different implementations of class @ref Thread provided by
 * GPCC for different operating systems.
 *
 * This implementation uses the EPOS Thread API to create the output.\n
 * See `epos_thread_CreateInfoTableHeader()`, `epos_thread_CreateInfoString()`, and
 * `epos_thread_CreateInfoTableFooter()`.
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
  using gpcc::string::StringComposer;
  StringComposer infoLine;

  epos_thread_t* pEPOSThread = nullptr;
  {
    gpcc::osal::MutexLocker mutexLocker(mutex);

    if (pThread != nullptr)
    {
      pEPOSThread = pThread;
      epos_thread_IncRefCnt(pEPOSThread);
    }
  }

  if (pEPOSThread != nullptr)
  {
    ON_SCOPE_EXIT(DecRefCnt) { epos_thread_DecRefCnt(pEPOSThread); };

    char* pInfoStr = epos_thread_CreateInfoString(pEPOSThread, nameFieldWidth);
    if (pInfoStr == nullptr)
    {
      if (errno == ENOMEM)
        throw std::bad_alloc();
      else
        throw std::system_error(errno, std::generic_category(), "epos_thread_CreateInfoString() failed");
    }

    ON_SCOPE_EXIT(ReleaseInfoStr) { free(pInfoStr); };

    infoLine << pInfoStr;
  }
  else
  {
    if (name.size() <= nameFieldWidth)
      infoLine << StringComposer::AlignLeft << StringComposer::Width(nameFieldWidth) << name;
    else
      infoLine << name.substr(0, nameFieldWidth - 3U) << "...";

    infoLine << " -----";
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
  MutexLocker mutexLocker(mutex);
  return (epos_thread_Self() == pThread);
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
  // Check parameters
  // ('priority' and 'schedPolicy' are checked in UniversalPrioToEPOSPrio())
  if (!_entryFunction)
    throw std::invalid_argument("Thread::Start: Inv. args.");

  if ((stackSize < GetMinStackSize()) || ((stackSize % GetStackAlign()) != 0U))
    throw std::invalid_argument("Thread::Start: Inv. args.");

  // map universal priority to EPOS
  epos_threadprio_t const mappedPrio = UniversalPrioToEPOSPrio(priority, schedPolicy);
  epos_timeslice_t  const timeslice_ms = UniversalPrioToTimeslice(schedPolicy);

  MutexLocker joinMutexLocker(joinMutex);
  MutexLocker mutexLocker(mutex);

  // check that there is currently no thread
  if (threadState != ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::Start: Precons");

  // prepare thread start
  entryFunction                   = _entryFunction;
  threadState                     = ThreadState::starting;
  cancellationRequestedViaThisAPI = false;

  // create and start thread
  pThread = epos_thread_Create(&Thread::InternalThreadEntry1, this, mappedPrio, timeslice_ms, stackSize, name.c_str());

  if (pThread == nullptr)
  {
    threadState = ThreadState::noThreadOrJoined;

    if (errno == ENOMEM)
      throw std::bad_alloc();
    else
      throw std::system_error(errno, std::generic_category(), "epos_thread_Create() failed");
  }

  // Increment the reference count. This way the thread object will not be used immediately when the thread is joined.
  epos_thread_IncRefCnt(pThread);

  // Wait until the new thread leaves the starting-state. Any unexpected error here will result in panic.
  try
  {
    // Note: Wait() does currently not contain a cancellation point on EPOS.
    // If it ever does, then deferred thread cancellation of the current thread must be disabled temporarily.
    while (threadState == ThreadState::starting)
      threadStateRunningCondVar.Wait(mutex);
  }
  catch (...)
  {
    PANIC();
  }
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
    throw std::logic_error("Thread::Cancel: Precons");

  // not yet terminated?
  if (threadState != ThreadState::terminated)
  {
    // verify, that the current thread is not the one managed by this object
    if (epos_thread_Self() == pThread)
      throw std::logic_error("Thread::Cancel: Precons");

    // verify, that cancellation of the thread has not yet been requested
    if (cancellationRequestedViaThisAPI)
      throw std::logic_error("Thread::Cancel: Precons");

    (void)epos_thread_RequestCancellation(pThread);
    cancellationRequestedViaThisAPI = true;
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

  {
    MutexLocker mutexLocker(mutex);

    // verify that the object manages a thread, which has not yet been joined
    if (threadState == ThreadState::noThreadOrJoined)
      throw std::logic_error("Thread::Join: Precons");

    // verify, that the current thread is not the one managed by this object
    if (epos_thread_Self() == pThread)
      throw std::logic_error("Thread::Join: Precons");
  }

  // Wait for termination and join with thread.
  // Note that the thread's reference count has been incremented in Start(), so the thread's resources will not
  // be immediately released.
  void* const retval = epos_thread_Join(pThread);

  try
  {
    MutexLocker mutexLocker(mutex);

    // check and update threadState, the object does no longer manage a thread
    if (threadState != ThreadState::terminated)
      PANIC();

    threadState = ThreadState::noThreadOrJoined;

    epos_thread_DecRefCnt(pThread);
    pThread = nullptr;

    // anyone interested in if the thread was cancelled?
    if (pCancelled != nullptr)
      *pCancelled = (retval == EPOS_THREAD_CANCELLED);

    if (retval == EPOS_THREAD_CANCELLED)
      return nullptr;
    else
      return retval;
  }
  catch (...)
  {
    PANIC();
  }
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
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if (epos_thread_Self() != pThread)
    throw std::logic_error("Thread::SetCancelabilityEnabled: Precons");

  return epos_thread_EnableDeferredCancellation(enable);
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
bool Thread::IsCancellationPending(void) const
{
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if (epos_thread_Self() != pThread)
    throw std::logic_error("Thread::IsCancellationPending: Precons");

  return epos_thread_IsCancellationPending();
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
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if (epos_thread_Self() != pThread)
    throw std::logic_error("Thread::TestForCancellation: Precons");

  epos_thread_TestCancel();
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
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if (epos_thread_Self() != pThread)
    throw std::logic_error("Thread::TerminateNow: Precons");

  epos_thread_TerminateNow(threadReturnValue);
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
 * This is executed by EPOS upon thread creation. This is a static member function because EPOS (C-code) does
 * not know anything about this-pointers. Parameter 'arg' is the pointer to the Thread-object managing the thread
 * executing this function. This function reconstructs the this-pointer from parameter 'arg' and invokes
 * @ref InternalThreadEntry2(), which is a non-static member function.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant for different values of @p arg.
 *
 * __Exception safety:__\n
 * Basic guarantee:\n
 * This function relies on that EPOS will raise a panic if an uncaught exception propagates out of the entry function
 * for the EPOS thread.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \param arg
 * Argument required by EPOS carrying application specific information.\n
 * Here it points to the @ref Thread object managing the calling thread.
 *
 * \return
 * Return value of the thread entry function passed to @ref Start().
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
 * Basic guarantee:\n
 * This function relies on that EPOS will raise a panic if an uncaught exception propagates out of the entry function
 * for the EPOS thread.
 *
 * __Thread cancellation safety:__\n
 * Strong guarantee.
 *
 * - - -
 *
 * \return
 * Return value of the thread entry function passed to @ref Start().
 */
void* Thread::InternalThreadEntry2(void)
{
  // set threadState to ThreadState::running
  {
    MutexLocker mutexLocker(mutex);
    threadState = ThreadState::running;
    threadStateRunningCondVar.Signal();
  }

  // Set state to "terminated" if the thread leaves this function, either by returning, by TerminateNow() or by
  // thread cancellation.
  ON_SCOPE_EXIT()
  {
    MutexLocker mutexLocker(mutex);
    threadState = ThreadState::terminated;
  };

  // execute thread entry function
  void* retval;
  try
  {
    retval = entryFunction();
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("Thread entry function threw: ", e);
  }

  return retval;
}

/**
 * \brief Checks a given priority level and scheduling policy and maps both to the priority-range of EPOS.
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
 * \param priority
 * Priority level.
 *
 * \param schedpolicy
 * Scheduling policy.
 *
 * \return
 * Parameter `priority` projected on the priority-range of the system based on the selected `schedpolicy`.
 */
epos_threadprio_t Thread::UniversalPrioToEPOSPrio(priority_t const priority, SchedPolicy const schedpolicy) const
{
  static_assert((maxPriority - minPriority + 1U) == 32U,
                "Thread::minPriority...Thread::maxPriority must provide 32 priority levels.");

  static_assert((EPOS_THREAD_PRIO_MIN - EPOS_THREAD_PRIO_MAX + 1U) >= (1U + 32U + 13U),
                "EPOS configuration does not provide enough priority levels.");

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if ((priority < minPriority) || (priority > maxPriority))
    throw std::invalid_argument("Invalid sched. priority/policy");
  #pragma GCC diagnostic pop

  if ((priority != 0U) && (schedpolicy != SchedPolicy::Fifo) && (schedpolicy != SchedPolicy::RR))
    throw std::invalid_argument("Invalid sched. priority/policy");

  epos_threadprio_t prio = 0U;
  switch (schedpolicy)
  {
    case SchedPolicy::Inherit:
      prio = epos_thread_GetPriority(epos_thread_Self());
      break;

    case SchedPolicy::Other:
      prio = EPOS_THREAD_PRIO_MIN - 12U;
      break;

    case SchedPolicy::Idle:
      prio = EPOS_THREAD_PRIO_MIN;
      break;

    case SchedPolicy::Batch:
      prio = EPOS_THREAD_PRIO_MIN - 1U;
      break;

    case SchedPolicy::Fifo:
      // intentional fall-through
    case SchedPolicy::RR:
      prio = (EPOS_THREAD_PRIO_MAX + 32U) - priority;
      break;
  }

  return prio;
}

/**
 * \brief Converts the scheduling policy to an EPOS timeslice quantum.
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
 * \param schedpolicy
 * Scheduling policy.
 *
 * \return
 * Timeslice quantum required by @p schedpolicy.
 */
epos_timeslice_t Thread::UniversalPrioToTimeslice(SchedPolicy const schedpolicy) const
{
  switch (schedpolicy)
  {
    case SchedPolicy::Inherit:
      return epos_thread_GetTimeslice_ms(epos_thread_Self());

    case SchedPolicy::Other:
    case SchedPolicy::Idle:
    case SchedPolicy::Batch:
    case SchedPolicy::RR:
      return EPOS_THREAD_TIMESLICE_DEFAULT_MS;

    case SchedPolicy::Fifo:
      return EPOS_THREAD_TIMESLICE_NONE;
  }

  // never reached, but makes compiler happy
  return EPOS_THREAD_TIMESLICE_DEFAULT_MS;
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_EPOS_ARM
