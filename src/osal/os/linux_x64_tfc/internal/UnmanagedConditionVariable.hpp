/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2021, 2022 Daniel Jerolm

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
