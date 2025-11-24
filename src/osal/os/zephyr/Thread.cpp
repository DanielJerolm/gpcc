/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR

#include <gpcc/osal/Thread.hpp>
#include <gpcc/osal/MutexLocker.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <gpcc/raii/unique_c_ptr.hpp>
#include <gpcc/string/StringComposer.hpp>
#include <limits>
#include <stdexcept>
#include <system_error>
#include <cerrno>

#if !defined(CONFIG_THREAD_STACK_INFO)
  #error "GPCC requires Zephyr CONFIG_THREAD_STACK_INFO"
#endif

namespace
{
  // Exception used to emulate deferred cancellation.
  class ThreadCancellationException final
  {
  };

  // Exception used to implement TerminateNow().
  class ThreadTerminateNowException final
  {
    public:
      void* const threadReturnValue_;

      ThreadTerminateNowException(void* const threadReturnValue) : threadReturnValue_(threadReturnValue) {};
  };
}

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
  return 1U * 1024U;
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
  return 8U;
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
  return 6U * 1024U;
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
 * \param name
 * Name for the thread managed by this object.
 */
Thread::Thread(std::string const & name)
: name_(name)
, mutex_()
, joinMutex_()
, entryFunction_()
, threadState_(ThreadState::noThreadOrJoined)
, threadStateRunningCondVar_()
, pStack_(nullptr)
, stackSize_(0U)
, thread_()
, thread_id_()
, threadRetVal_(nullptr)
, cancelabilityEnabled_(false)
, cancellationPending_(false)
, cancelled_(false)
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
    MutexLocker joinMutexLocker(joinMutex_);
    MutexLocker mutexLocker(mutex_);

    if (threadState_ != ThreadState::noThreadOrJoined)
      Panic("Thread::~Thread: Precons");
  }

  InternalGetThreadRegistry().UnregisterThread(*this);
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
  uint32_t rest = ms;
  while (rest != 0U)
  {
    uint32_t const portion = (rest > std::numeric_limits<int32_t>::max()) ? std::numeric_limits<int32_t>::max() : rest;
    if (k_msleep(portion) != 0)
    {
      // k_wakeup() is not used, so returning anything but zero is not anticipated
      PANIC();
    }
    rest -= portion;
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
  // overflow-free round up to us
  uint32_t const us = (ns / 1000UL) + (((ns % 1000UL) != 0U) ? 1U : 0U);
  if (k_usleep(us) != 0)
  {
    // k_wakeup() is not used, so returning anything but zero is not anticipated
    PANIC();
  }
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
 * Name             State                   Prio Stacksize      Used\n
 * -----------------------------------------------------------------
 * ...              run                      ppp  ssssssss  ssssssss
 * ~~~
 *
 * Note that the output format may be different among the different implementations of class @ref Thread provided by
 * GPCC for different operating systems.
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

  if (nameFieldWidth < 4U)
    throw std::invalid_argument("Inv. args.");

  StringComposer infoLine;
  infoLine << StringComposer::AlignLeft;

  gpcc::osal::MutexLocker mutexLocker(mutex_);

  if (name_.size() <= nameFieldWidth)
    infoLine << StringComposer::Width(nameFieldWidth) << name_;
  else
    infoLine << name_.substr(0, nameFieldWidth - 3U) << "...";

  infoLine << ' ';

  if (threadState_ != ThreadState::noThreadOrJoined)
  {
    char buf[24];
    char const * state = k_thread_state_str(thread_id_, buf, sizeof(buf));
    infoLine << StringComposer::Width(sizeof(buf)) << state;

    infoLine << StringComposer::AlignRight;

    int priority = k_thread_priority_get(thread_id_);
    infoLine << StringComposer::Width(4) << priority;

    infoLine << StringComposer::Width(10) << stackSize_;

    size_t freeSpace = 0U;
    int status = k_thread_stack_space_get(thread_id_, &freeSpace);
    if (status != 0)
      throw std::system_error(status, std::generic_category(), "k_thread_stack_space_get() failed");

    size_t usedSpace = 0U;
    if (freeSpace < stackSize_)
      usedSpace = stackSize_ - freeSpace;

    infoLine << StringComposer::Width(10) << usedSpace;
  }
  else
  {
    infoLine << "----------------------- ---- --------- ---------";
  }

  return infoLine.Get();
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
 * \param entryFunction
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
void Thread::Start(tEntryFunction const & entryFunction,
                   SchedPolicy const schedPolicy,
                   priority_t const priority,
                   size_t const stackSize)
{
  // Check parameters
  // ('priority' and 'schedPolicy' are checked in UniversalPrioToZephyrPrio())
  if (!entryFunction)
    throw std::invalid_argument("Inv. args.");

  if ((stackSize < GetMinStackSize()) || ((stackSize % GetStackAlign()) != 0U))
    throw std::invalid_argument("Inv. args.");

  // map universal priority to Zephyr
  int const mappedPrio = UniversalPrioToZephyrPrio(priority, schedPolicy);

  MutexLocker joinMutexLocker(joinMutex_);
  MutexLocker mutexLocker(mutex_);

  // check that there is currently no thread
  if (threadState_ != ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread alread existing");

  // allocate memory for stack
  pStack_ = k_thread_stack_alloc(stackSize, 0U);
  if (pStack_ == nullptr)
    throw std::bad_alloc();

  stackSize_ = stackSize;

  ON_SCOPE_EXIT(releaseStack)
  {
    k_thread_stack_free(pStack_);
    pStack_ = nullptr;
  };

  // prepare thread start
  entryFunction_        = entryFunction;
  threadState_          = ThreadState::starting;
  threadRetVal_         = nullptr;
  cancelabilityEnabled_ = true;
  cancellationPending_  = false;
  cancelled_            = false;

  // start thread
  thread_id_ = k_thread_create(&thread_,
                               pStack_, stackSize,
                               &Thread::InternalThreadEntry1,
                               this, nullptr, nullptr,
                               mappedPrio,
                               0U,
                               K_NO_WAIT);

  // Wait until the new thread leaves the starting-state. Any unexpected error here will result in panic.
  try
  {
    while (threadState_ == ThreadState::starting)
      threadStateRunningCondVar_.Wait(mutex_);
  }
  catch (...)
  {
    PANIC();
  }

  ON_SCOPE_EXIT_DISMISS(releaseStack);
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
  MutexLocker mutexLocker(mutex_);

  // verify that the object manages a thread, which has not yet been joined
  if (threadState_ == ThreadState::noThreadOrJoined)
    throw std::logic_error("No thread");

  // not yet terminated?
  if (threadState_ != ThreadState::terminated)
  {
    // verify, that the current thread is not the one managed by this object
    if (IsItMe())
      throw std::logic_error("Wrong caller");

    // verify, that cancellation of the thread has not yet been requested
    if (cancellationPending_)
      throw std::logic_error("Already pending");

    cancellationPending_.store(true, std::memory_order_relaxed);
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
  MutexLocker joinMutexLocker(joinMutex_);

  {
    MutexLocker mutexLocker(mutex_);

    // verify that the object manages a thread, which has not yet been joined
    if (threadState_ == ThreadState::noThreadOrJoined)
      throw std::logic_error("No thread");

    // verify, that the current thread is not the one managed by this object
    if (IsItMe())
      throw std::logic_error("Wrong caller");
  }

  // wait for termination and join with thread
  int status = k_thread_join(&thread_, K_FOREVER);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "k_thread_join() failed");

  try
  {
    MutexLocker mutexLocker(mutex_);

    // check and update threadState_, the object does no longer manage a thread
    if (threadState_ != ThreadState::terminated)
      PANIC();

    threadState_ = ThreadState::noThreadOrJoined;

    // release stack
    status = k_thread_stack_free(pStack_);
    if (status != 0)
      PANIC();

    pStack_ = nullptr;

    // anyone interested in if the thread was cancelled?
    if (pCancelled != nullptr)
      *pCancelled = cancelled_;

    return threadRetVal_;
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
  // verify that the current thread is the one managed by this object
  if (!IsItMe())
    throw std::logic_error("Wrong caller");

  bool const retVal = cancelabilityEnabled_;
  cancelabilityEnabled_ = enable;
  return retVal;
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
  // verify that the current thread is the one managed by this object
  if (!IsItMe())
    throw std::logic_error("Wrong caller");

  return cancellationPending_.load(std::memory_order_relaxed);
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
  if (!IsItMe())
    throw std::logic_error("Wrong caller");

  if (cancellationPending_.load(std::memory_order_relaxed))
    throw ThreadCancellationException();
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
  if (!IsItMe())
    throw std::logic_error("Wrong caller");

  throw ThreadTerminateNowException(threadReturnValue);
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
 * This is executed by Zephyr upon thread creation. This is a static member function because Zephyr (C-code) does
 * not know anything about this-pointers. Parameter @p p1 is the pointer to the Thread-object managing the thread
 * executing this function. This function reconstructs the this-pointer from parameter @p p1 and invokes
 * @ref InternalThreadEntry2(), which is a non-static member function.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant for different values of @p p1.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Not applicable.
 *
 * - - -
 *
 * \param p1
 * Argument required by Zephyr carrying application specific information.\n
 * Here it points to the @ref Thread object managing the calling thread.
 *
 * \param p2
 * Argument required by Zephyr carrying application specific information.\n
 * Unused here.
 *
 * \param p3
 * Argument required by Zephyr carrying application specific information.\n
 * Unused here.
 */
void Thread::InternalThreadEntry1(void* p1, void* p2, void* p3) noexcept
{
  (void)p2;
  (void)p3;
  if (p1 == nullptr)
    PANIC();

  static_cast<Thread*>(p1)->InternalThreadEntry2();
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
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Not applicable.
 */
void Thread::InternalThreadEntry2(void) noexcept
{
  // set threadState to ThreadState::running
  {
    MutexLocker mutexLocker(mutex_);
    threadState_ = ThreadState::running;
    threadStateRunningCondVar_.Signal();
  }

  // Set state to "terminated" if the thread leaves this function, either by returning, by TerminateNow() or by
  // thread cancellation.
  ON_SCOPE_EXIT()
  {
    MutexLocker mutexLocker(mutex_);
    threadState_ = ThreadState::terminated;
  };

  // execute thread entry function
  try
  {
    threadRetVal_ = entryFunction_();
  }
  catch (ThreadCancellationException const &)
  {
    cancelled_ = true;
    threadRetVal_ = nullptr;
  }
  catch (ThreadTerminateNowException const & e)
  {
    threadRetVal_ = e.threadReturnValue_;
  }
  catch (std::exception const & e)
  {
    gpcc::osal::Panic("Thread entry function threw: ", e);
  }
  catch (...)
  {
    gpcc::osal::Panic("Thread entry function threw");
  }
}

/**
 * \brief Checks a given priority level and scheduling policy and maps both to the priority-range of Zephyr.
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
int Thread::UniversalPrioToZephyrPrio(priority_t const priority, SchedPolicy const schedpolicy) const
{
  static_assert((maxPriority - minPriority + 1U) == 32U,
                "Thread::minPriority...Thread::maxPriority must provide 32 priority levels.");

  static_assert(CONFIG_NUM_PREEMPT_PRIORITIES >= 35U,
                "Zephyr configuration does not provide enough preemptible priority levels.");

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if ((priority < minPriority) || (priority > maxPriority))
    throw std::invalid_argument("Invalid prio/policy");
  #pragma GCC diagnostic pop

  if ((priority != 0U) && (schedpolicy != SchedPolicy::Fifo) && (schedpolicy != SchedPolicy::RR))
    throw std::invalid_argument("Invalid prio/policy");

  int prio = 0U;
  switch (schedpolicy)
  {
    case SchedPolicy::Inherit:
      prio = k_thread_priority_get(k_current_get());
      break;

    case SchedPolicy::Other:
      prio = CONFIG_NUM_PREEMPT_PRIORITIES - 3U;
      break;

    case SchedPolicy::Idle:
      prio = CONFIG_NUM_PREEMPT_PRIORITIES - 1U;
      break;

    case SchedPolicy::Batch:
      prio = CONFIG_NUM_PREEMPT_PRIORITIES - 2U;
      break;

    case SchedPolicy::Fifo:
      throw std::logic_error("Policy not supported");

    case SchedPolicy::RR:
      prio = maxPriority - (priority - minPriority);
      break;
  }

  return prio;
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_ZEPHYR
