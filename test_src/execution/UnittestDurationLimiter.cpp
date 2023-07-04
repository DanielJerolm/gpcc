/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2023 Daniel Jerolm
*/

#include <gpcc_test/execution/UnittestDurationLimiter.hpp>
#include <gpcc/osal/Panic.hpp>
#include <gpcc/raii/scope_guard.hpp>
#include <unistd.h>
#include <stdexcept>
#include <system_error>

#ifndef NDEBUG
  #warning "UnittestDurationLimiter with no function due to absence of NDEBUG switch!"
#endif

namespace gpcc_tests {
namespace execution  {

/**
 * \brief Constructor. Supervision of the guarded unittest case starts with the construction of this object.
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
 * \param _maxDuration_sec
 * Maximum duration of the guarded unittest case in seconds. Zero is not allowed.\n
 * After this time has expired, a panic will be raised.
 */
UnittestDurationLimiter::UnittestDurationLimiter(uint8_t const _maxDuration_sec)
: maxDuration_sec(_maxDuration_sec)
, thread()
{
  if (maxDuration_sec == 0U)
    throw std::invalid_argument("_maxDuration_sec is zero");

  // create thread attribute object
  pthread_attr_t attr;
  int status = pthread_attr_init(&attr);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "UnittestDurationLimiter::UnittestDurationLimiter: Could not create pthread_attr_t object");

  ON_SCOPE_EXIT(destroy_attr)
  {
    pthread_attr_destroy(&attr);
  };

  // configure thread attribute object
  status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  if (status == 0)
    status = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

  if (status != 0)
    throw std::system_error(status, std::generic_category(), "UnittestDurationLimiter::UnittestDurationLimiter: Could not configure pthread_attr_t object");

#ifdef NDEBUG
  // start thread
  status = pthread_create(&thread, &attr, UnittestDurationLimiter::ThreadEntry, this);
  if (status != 0)
    throw std::system_error(status, std::generic_category(), "UnittestDurationLimiter::UnittestDurationLimiter: Could not start thread");

  (void)pthread_setname_np(thread, "UDL");
#endif
}

/**
 * \brief Destructor. Supervision of the unittest's execution time ends here.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Deferred cancellation is not allowed.
 */
UnittestDurationLimiter::~UnittestDurationLimiter(void)
{
#ifdef NDEBUG
  int status = pthread_cancel(thread);
  if (status != 0)
    PANIC();

  status = pthread_join(thread, nullptr);
  if (status != 0)
    PANIC();
#endif
}

/**
 * \brief Entry function for the class' thread.
 *
 * This will sleep for the guard time and then raise a panic if the thread is not cancelled before.
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
 * \param pArg
 * Pointer to the @ref UnittestDurationLimiter instance.
 *
 * \return
 * This will never return.
 */
void* UnittestDurationLimiter::ThreadEntry(void* pArg)
{
  UnittestDurationLimiter* const p = reinterpret_cast<UnittestDurationLimiter*>(pArg);

  sleep(p->maxDuration_sec);
  gpcc::osal::Panic("UnittestDurationLimiter: Maximum execution time exceeded!");

  return nullptr;
}

} // namespace execution
} // namespace gpcc_tests
