/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011, 2024 Daniel Jerolm
*/

#ifdef OS_LINUX_X64

#ifndef SEMAPHORE_HPP_201701290841
#define SEMAPHORE_HPP_201701290841

#include <semaphore.h>
#include <climits>
#include <cstddef>

namespace gpcc {
namespace osal {

/**
 * \ingroup GPCC_OSAL_THREADING
 * \brief A counting semaphore.
 *
 * # Features
 * - Initial value configurable during instantiation
 * - Post/increment and wait/decrement
 *
 * # Constraints/Restrictions
 * - _All threads using instances of class Semaphore must live in the same process._
 *
 * # Alternatives
 * There are further semaphore implementations available in @ref GPCC_RESOURCEMANAGEMENT ->
 * @ref GPCC_RESOURCEMANAGEMENT_SEMAPHORES.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class Semaphore final
{
  public:
    /// Maximum value of the semaphore.
    static size_t const MAX = SEM_VALUE_MAX;


    Semaphore(void) = delete;
    explicit Semaphore(size_t const initialValue);
    Semaphore(Semaphore const &) = delete;
    Semaphore(Semaphore &&) = delete;
    ~Semaphore(void);

    Semaphore& operator=(Semaphore const &) = delete;
    Semaphore& operator=(Semaphore &&) = delete;

    void Post(void);
    void Wait(void);

  private:
    /// Encapsulated POSIX semaphore structure.
    sem_t semaphore;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef SEMAPHORE_HPP_201701290841
#endif // #ifdef OS_LINUX_X64
