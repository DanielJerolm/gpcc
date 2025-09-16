/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2025 WAGO GmbH & Co. KG
*/

#ifdef OS_ZEPHYR

#ifndef SEMAPHORE_HPP_202509161309
#define SEMAPHORE_HPP_202509161309

#include <zephyr/kernel.h>
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
    static size_t const MAX = K_SEM_MAX_LIMIT / 2U;


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
    /// The encapsulated Zephyr semaphore.
    struct k_sem sem_;
};

} // namespace osal
} // namespace gpcc

#endif // #ifndef SEMAPHORE_HPP_202509161309
#endif // #ifdef OS_ZEPHYR
