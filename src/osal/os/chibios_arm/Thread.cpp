/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2020, 2022 Daniel Jerolm

    This file is part of the General Purpose Class Collection (GPCC).

    The General Purpose Class Collection (GPCC) is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The General Purpose Class Collection (GPCC) is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes the General Purpose Class Collection (GPCC), without being obliged
    to provide the source code for any proprietary components. See the file
    license_exception.txt for full details of how and when the exception can be applied.
*/

#ifdef OS_CHIBIOS_ARM

#include "Thread.hpp"
#include "Panic.hpp"
#include "gpcc/src/osal/AdvancedMutexLocker.hpp"
#include "gpcc/src/osal/MutexLocker.hpp"
#include "gpcc/src/string/tools.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <cxxabi.h>

#define NS_PER_SEC      1000000000UL
#define NS_PER_SYSTICK  (NS_PER_SEC / CH_CFG_ST_FREQUENCY)

static_assert(NS_PER_SEC % NS_PER_SYSTICK == 0, "Some calculations in this class will not work with configured system tick period");

namespace gpcc {
namespace osal {

Thread::priority_t const Thread::minPriority;
Thread::priority_t const Thread::maxPriority;

msg_t const Thread::TEC_Normal;
msg_t const Thread::TEC_TerminateNow;
msg_t const Thread::TEC_Cancelled;


// Exception used to emulate deferred cancellation.
class ThreadCancellationException final
{
};

// Exception used to implement TerminateNow().
class ThreadTerminateNowException final
{
  public:
    void* const pThreadReturnValue;

    ThreadTerminateNowException(void* const _pThreadReturnValue) : pThreadReturnValue(_pThreadReturnValue) {};
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
  return 256U;
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
  static_assert(((PORT_WORKING_AREA_ALIGN == 4U) || (PORT_WORKING_AREA_ALIGN == 8U)), "Check Thread::GetStackAlign().");
  return PORT_WORKING_AREA_ALIGN;
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
, pWA(nullptr)
, pThread(nullptr)
, totalStackSize(0U)
, pThreadReturnValue(nullptr)
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
 * ID of the process.\n
 * Always zero for the ChibiOS/RT port.
 */
uint32_t Thread::GetPID(void)
{
  return 0U;
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
  Internal_Sleep_ns(ms * 1000000ULL);
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
  Internal_Sleep_ns(ns);
}

/**
 * \brief Creates an std::string with information about the managed thread.
 *
 * This method is intended to be used to create human-readable information about the threads registered in the
 * application's thread-registry (see @ref GetThreadRegistry()).
 *
 * Output format:\n
 * <tt>Name             State Prio  StackSize  UsedStack        [Bottom----Top[</tt>\n
 * <tt>...              no     ppp ssssssssss ssssssssss (xxx%) 0xXXXXXXXX 0xXXXXXXXX</tt>\n
 * <tt>                 start</tt>\n
 * <tt>                 run</tt>\n
 * <tt>                 term</tt>\n
 * \n
 * States:\n
 * no    = thread (currently) not registered at OS or thread has been joined\n
 * start = thread is starting\n
 * run   = thread is running (but may be blocked on a @ref Mutex, @ref Semaphore, ...)\n
 * term  = thread has terminated, but not yet been joined
 *
 * Stack bottom:\n
 * This address marks the maximum extend of the stack. If the stack pointer (R13) extends beyond this, then there
 * is a stack overflow condition.
 *
 * Stack top:\n
 * Address of the first byte above the stack, which is not part of the stack.
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
      infoLine << " no     ";
      break;

    case ThreadState::starting:
      infoLine << " start  ";
      break;

    case ThreadState::running:
      infoLine << " run    ";
      detailsRequired = true;
      break;

    case ThreadState::terminated:
      infoLine << " term   ";
      break;
  } // switch (threadState)

  if (detailsRequired)
  {
    // ChibiOS priority
    infoLine << std::right << std::setw(3) << std::setfill(' ') << pThread->prio << ' ';

    // stack size
    infoLine << std::right << std::setw(10) << std::setfill(' ') << totalStackSize << ' ';

    // stack usage
    size_t const used = InternalMeasureStack();
    if (used < 100000000UL)
    {
      // perentage: round up
      uint_fast8_t const percentage = ((used * 100U) + (totalStackSize - 1U)) / totalStackSize;
      infoLine << std::right << std::setw(10) << std::setfill(' ') << used
               << " (" << std::right << std::setw(3) << std::setfill(' ') << static_cast<unsigned int>(percentage) << "%) ";
    }
    else
      infoLine << "       Err (Err%) ";

    // stack bottom
    infoLine << gpcc::string::ToHex(reinterpret_cast<uint32_t>(pThread->wabase), 8U) << ' ';

    // stack top
    infoLine << gpcc::string::ToHex(reinterpret_cast<uint32_t>(pThread), 8U);
  }
  else
  {
    infoLine << "--- ---------- ---------- ------ ---------- ----------";
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
  return ((threadState != ThreadState::noThreadOrJoined) && (chThdGetSelfX() == pThread));
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

  // map universal priority to ChibiOS
  tprio_t const mappedPrio = UniversalPrioToChibiOSPrio(priority, schedPolicy);

  MutexLocker joinMutexLocker(joinMutex);
  MutexLocker mutexLocker(mutex);

  // check that there is currently no thread
  if (threadState != ThreadState::noThreadOrJoined)
    throw std::logic_error("Thread::Start: There is already a thread");

  // determine required ChibiOS thread working area size (multiple of PORT_STACK_ALIGN)
  size_t const WAsize = THD_WORKING_AREA_SIZE(stackSize);

  // allocate thread working area (aligned to PORT_WORKING_AREA_ALIGN)
  pWA = new uint8_t[WAsize + (PORT_WORKING_AREA_ALIGN - 1U)];
  uint8_t* const pWA_aligned = reinterpret_cast<uint8_t*>(MEM_ALIGN_NEXT(pWA, PORT_WORKING_AREA_ALIGN));

  // prepare thread start
  entryFunction         = _entryFunction;
  threadState           = ThreadState::starting;
  pThreadReturnValue    = nullptr;
  cancelabilityEnabled  = true;
  cancellationPending   = false;

  // create and start thread
  pThread = chThdCreateStatic(pWA_aligned, WAsize, mappedPrio, Thread::InternalThreadEntry1, this);

  // Calculate total stack size. May be larger than "stackSize", because THD_WORKING_AREA_SIZE may add some extra
  // bytes for interrupt handling, context switch, etc.
  totalStackSize = reinterpret_cast<uintptr_t>(pThread) - reinterpret_cast<uintptr_t>(pThread->wabase);

  // check: actual stack size must be at least "stackSize"
  if (totalStackSize < stackSize)
    PANIC();

  // Wait until the new thread leaves the starting-state. Any unexpected error here will result in panic.
  try
  {
    // note: Wait() does not contain a cancellation point on ChibiOS
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
 * cancelability is reenabled. The managed thread can change its cancelability state via
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
    if (chThdGetSelfX() == pThread)
      throw std::logic_error("Thread::Cancel: Invoked by the managed thread");

    // verify, that cancellation of the thread has not yet been requested
    if (cancellationPending)
      throw std::logic_error("Thread::Cancel: Cancellation already requested");

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
  if (chThdGetSelfX() == pThread)
    throw std::logic_error("Thread::Join: Thread cannot join itself");

  mutexLocker.Unlock();

  // wait for termination and join with thread
  msg_t const exitCode = chThdWait(pThread);

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

  // clean-up
  delete [] pWA;

  // examine the way the thread has terminated
  switch (exitCode)
  {
    case TEC_Normal:
    case TEC_TerminateNow:
      break;

    case TEC_Cancelled:
      if (pThreadReturnValue != nullptr)
        PANIC(); // unexpected pThreadReturnValue
      break;

    default:
      PANIC(); // Invalid exitCode
      break;
  }

  // anyone interested in if the thread was cancelled?
  if (pCancelled != nullptr)
    *pCancelled = (exitCode == TEC_Cancelled);

  return pThreadReturnValue;
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
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if ((threadState != ThreadState::running) || (chThdGetSelfX() != pThread))
    throw std::logic_error("Thread::SetCancelabilityEnabled: Not invoked by the managed thread");

  cancelabilityEnabled = enable;
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
  if ((threadState != ThreadState::running) || (chThdGetSelfX() != pThread))
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
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if ((threadState != ThreadState::running) || (chThdGetSelfX() != pThread))
    throw std::logic_error("Thread::TestForCancellation: Not invoked by the managed thread");

  if ((cancelabilityEnabled) && (cancellationPending))
    throw ThreadCancellationException();
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
  MutexLocker mutexLocker(mutex);

  // verify that the current thread is the one managed by this object
  if ((threadState != ThreadState::running) || (chThdGetSelfX() != pThread))
    throw std::logic_error("Thread::TerminateNow: Not invoked by the managed thread");

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
 * \brief Suspends execution of the calling thread for a configurable time-span.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.\n
 * This can be invoked by any thread.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param ns
 * Time-span (in ns) the calling thread shall be suspended.\n
 * This is the _minimum time_ the thread will be suspended.
 */
void Thread::Internal_Sleep_ns(uint64_t const ns) noexcept
{
  // Convert time-span (in ns) to system timer ticks.
  // We round up to next tick and add one tick extra for uncertainty due to granularity of system tick interrupt)
  uint64_t ticks = ((ns + (NS_PER_SYSTICK - 1U)) / NS_PER_SYSTICK) + 1U;

  // sleep in chunks of max_sleep_time
  while (ticks > TIME_MAX_INTERVAL)
  {
    chThdSleep(TIME_MAX_INTERVAL);
    ticks -= TIME_MAX_INTERVAL;
  }

  // sleep the rest
  if (ticks != 0U)
    chThdSleep(static_cast<sysinterval_t>(ticks));
}

/**
 * \brief Internal thread entry function (step 1).
 *
 * This is executed by ChibiOS upon thread creation. This is a static member function because ChibiOS (C-code) does
 * not know anything about this-pointers. Parameter 'arg' is the pointer to the Thread-object managing the thread
 * executing this function. This function reconstructs the this-pointer from parameter 'arg' and invokes
 * @ref InternalThreadEntry2(), which is a non-static member function.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is reentrant for different values of parameter `arg`.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation will result in regular return from this method.
 *
 * - - -
 *
 * \param arg
 * Argument required by ChibiOS carrying application specific information.\n
 * Here it points to the @ref Thread object managing the calling thread.
 */
void Thread::InternalThreadEntry1(void* arg) noexcept
{
  if (arg == nullptr)
    PANIC();

  msg_t const status = static_cast<Thread*>(arg)->InternalThreadEntry2();
  chThdExit(status);
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
 * Deferred cancellation will result in regular return from this method.
 *
 * - - -
 *
 * \return
 * A msg_t value that will be passed by the calling function (@ref InternalThreadEntry1()) to `chThdExit()`.\n
 * Using a return value instead of invoking `chThdExit()` directly in this method allows to cleanly leave the
 * try-catch block and to clean the stack properly. Any C++ stuff related to the thread will be properly shut down.
 */
msg_t Thread::InternalThreadEntry2(void) noexcept
{
  try
  {
    chRegSetThreadName(name.c_str());

    // set threadState to ThreadState::running
    AdvancedMutexLocker mutexLocker(mutex);
    threadState = ThreadState::running;
    threadStateRunningCondVar.Signal();
    mutexLocker.Unlock();

    try
    {
      // execute thread entry function
      pThreadReturnValue = entryFunction();
    }
    catch (ThreadCancellationException const &)
    {
      // the thread was cancelled by TestForCancellation()
      mutexLocker.Relock();
      threadState = ThreadState::terminated;

      return TEC_Cancelled;
    }
    catch (ThreadTerminateNowException const & e)
    {
      // the thread has terminated itself by invoking TerminateNow()
      mutexLocker.Relock();
      pThreadReturnValue = e.pThreadReturnValue;
      threadState = ThreadState::terminated;

      return TEC_TerminateNow;
    }
    catch (abi::__forced_unwind const &)
    {
      // catching abi::__forced_unwind should be impossible on this platform
      PANIC();
    }
    catch (...)
    {
      // the thread has terminated due to an uncaught exception
      Panic("Thread::InternalThreadEntry2: Uncaught exception propagated into thread entry function");
    }

    // the thread has terminated by leaving the thread entry function
    mutexLocker.Relock();
    threadState = ThreadState::terminated;

    return TEC_Normal;
  }
  catch (...)
  {
    PANIC();
  }
}

/**
 * \brief Determines the maximum number of bytes used on the thread's stack up to now.
 *
 * Measurement is based on the stack watermark applied by ChibiOS.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This may be executed by any thread if the following requirements are met:
 * - @ref mutex must be locked.
 * - @ref threadState must not be @ref ThreadState::noThreadOrJoined.
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
 * Maximum number of bytes used on the thread's stack up to now.
 */
size_t Thread::InternalMeasureStack(void) const noexcept
{
#if CH_DBG_FILL_THREADS != TRUE
#error "Thread::InternalMeasureStack() requires that CH_DBG_FILL_THREADS is TRUE"
#endif

  // prepare stack fill pattern
  uint32_t const pattern = (static_cast<uint32_t>(CH_DBG_STACK_FILL_VALUE) << 24) |
                           (static_cast<uint32_t>(CH_DBG_STACK_FILL_VALUE) << 16) |
                           (static_cast<uint32_t>(CH_DBG_STACK_FILL_VALUE) <<  8) |
                            static_cast<uint32_t>(CH_DBG_STACK_FILL_VALUE);

  // number of used uint32_t-quantities
  size_t used = totalStackSize / sizeof(uint32_t);

  // pointer to the last uint32_t-quantity of the stack
  uint32_t const * ptr = reinterpret_cast<uint32_t const *>(MEM_ALIGN_NEXT(reinterpret_cast<uintptr_t>(pWA), PORT_WORKING_AREA_ALIGN));

  // loop until end of stack or until the default fill pattern disappears
  while ((*ptr == pattern) && (used != 0U))
  {
    ptr++;
    used--;
  }

  // return number of used bytes
  return used * sizeof(uint32_t);
}

/**
 * \brief Converts the priority level and the scheduling policy to the priority-range of ChibiOS/RT.
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
 * \param priority
 * Priority level.
 *
 * \param schedpolicy
 * Scheduling policy.
 *
 * \return
 * Parameter `priority` projected on the priority-range of the system based on the selected `schedpolicy`.
 */
tprio_t Thread::UniversalPrioToChibiOSPrio(priority_t const priority, SchedPolicy const schedpolicy) const
{
  static_assert(NORMALPRIO + 1U + maxPriority <= HIGHPRIO, "Maximum priority value exceeds HIGHPRIO.");

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtype-limits"
  if ((priority < minPriority) || (priority > maxPriority))
    throw std::invalid_argument("Thread::UniversalPrioToChibiOSPrio: 'priority' is invalid");
  #pragma GCC diagnostic pop

  tprio_t prio = 0U;
  switch (schedpolicy)
  {
    case SchedPolicy::Inherit:
      prio = chThdGetPriorityX();
      break;

    case SchedPolicy::Other:
      prio = NORMALPRIO;
      break;

    case SchedPolicy::Idle:
      prio = LOWPRIO;
      break;

    case SchedPolicy::Batch:
      prio = LOWPRIO + 1U;
      break;

    case SchedPolicy::Fifo:
      prio = NORMALPRIO + 1U + priority;
      break;

    case SchedPolicy::RR:
      prio = NORMALPRIO + 1U + priority;
      break;
  }

  if ((prio < LOWPRIO) || (prio > HIGHPRIO))
    throw std::runtime_error("Thread::UniversalPrioToChibiOSPrio: Bad result");

  return prio;
}

} // namespace osal
} // namespace gpcc

#endif // #ifdef OS_CHIBIOS_ARM
