/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#ifndef UNITTESTDURATIONLIMITER_HPP_202305301532
#define UNITTESTDURATIONLIMITER_HPP_202305301532

#include <pthread.h>
#include <cstdint>

namespace gpcc_tests {
namespace execution  {

/**
 * \ingroup GPCC_TESTS
 * \brief Watchdog raising a panic after a specified amount of time.
 *
 * # Purpose
 * This class is intended to be used as a guard in unittests to limit the maximum execution time of a unittest case in
 * case of a dead-lock.
 *
 * Usually [TFC](@ref GPCC_TIME_FLOW_CONTROL) is used during unittesting and TFC is already capable of detecting some
 * types of dead-lock. However, [TFC](@ref GPCC_TIME_FLOW_CONTROL) cannot detect all types of dead-locks, or simply TFC
 * is not used during unittesting. In these cases, this class can be used as a guard to limit the maximum execution time
 * of a unittest case.
 *
 * # Usage
 * This class is intended to be instantiated on the stack of the unittest case or as a member of a test fixture.
 * ~~~{.cpp}
 * TEST(TestSuit0815, TestCase_ABC)
 * {
 *   // limit duration to 5 seconds
 *   UnittestDurationLimiter watchdog(5U);
 *
 *   // Conduct unittest
 *   // [...]
 * }
 * ~~~
 *
 * # Compatibility with TFC
 * This class uses pthread directly instead of using @ref gpcc::osal::Thread in order to avoid being supervised by TFC
 * and in order to avoid confusing TFC's dead-lock detection.
 *
 * - - -
 *
 * __Thread safety:__\n
 * Thread-safe.
 */
class UnittestDurationLimiter final
{
  public:
    UnittestDurationLimiter(void) = delete;
    UnittestDurationLimiter(uint8_t const _maxDuration_sec);
    UnittestDurationLimiter(UnittestDurationLimiter const &) = delete;
    UnittestDurationLimiter(UnittestDurationLimiter &&) = delete;
    ~UnittestDurationLimiter(void);

    UnittestDurationLimiter& operator=(UnittestDurationLimiter const &) = delete;
    UnittestDurationLimiter& operator=(UnittestDurationLimiter &&) = delete;

  private:
    /// Maximum tolerated unittest duration in seconds.
    /** The time span is specified using the host's time, __NOT__ the time emulated by GPCC's TFC feature. */
    uint8_t const maxDuration_sec;

    /// Thread used to supervise the unit test.
    pthread_t thread;


    static void* ThreadEntry(void* pArg);
};

} // namespace execution
} // namespace gpcc_tests

#endif // UNITTESTDURATIONLIMITER_HPP_202305301532
