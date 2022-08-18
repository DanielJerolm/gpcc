/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#ifdef OS_LINUX_X64_TFC

#ifndef UNMANAGEDCONDITIONVARIABLE_HPP_201702252116
#define UNMANAGEDCONDITIONVARIABLE_HPP_201702252116

#include <pthread.h>

namespace gpcc     {
namespace osal     {
namespace internal {

class UnmanagedMutex;

/**
 * \ingroup GPCC_TIME_FLOW_CONTROL
 * \brief A native, unmanaged condition variable.
 *
 * This class provides a trivial condition variable with the following properties:
 * - Methods: Signal, Broadcast and Wait
 * - Waiting without timeout only
 * - Which waiting thread is woken up if the condition variable is signaled depends on the underlying
 *   operating system
 * - Spurious wake-ups possible
 *
 * __This condition variable is completely based on the underlying OS and it is not managed by GPCC's TFC feature.__\n
 * __This condition variable implementation is intended to be used by the internals of TFC only.__
 *
 * This condition variable is intended to be used in conjunction with the unmanaged mutex provided
 * by class @ref UnmanagedMutex.
 *
 * ---
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class UnmanagedConditionVariable final
{
  public:
    UnmanagedConditionVariable(void);
    UnmanagedConditionVariable(UnmanagedConditionVariable const &) = delete;
    UnmanagedConditionVariable(UnmanagedConditionVariable &&) = delete;
    ~UnmanagedConditionVariable(void);

    UnmanagedConditionVariable& operator=(UnmanagedConditionVariable const &) = delete;
    UnmanagedConditionVariable& operator=(UnmanagedConditionVariable &&) = delete;

    void Wait(UnmanagedMutex & mutex);

    void Signal(void);
    void Broadcast(void);

  private:
    /// The encapsulated POSIX condition-variable.
    pthread_cond_t condVar;
};

} // namespace internal
} // namespace osal
} // namespace gpcc

#endif // #ifndef UNMANAGEDCONDITIONVARIABLE_HPP_201702252116
#endif // #ifdef OS_LINUX_X64_TFC
