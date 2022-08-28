/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TimePoint.hpp"
#include "TimeSpan.hpp"
#include <gpcc/compiler/builtins.hpp>
#include <limits>
#include <stdexcept>
#include <cstdio>
#include <ctime>

#define NSEC_PER_SEC  1000000000 ///<Number of ns per sec

namespace gpcc {
namespace time {

size_t constexpr TimePoint::stringLength;

/**
 * \brief Constructor. Creates a timepoint of zero.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TimePoint::TimePoint(void) noexcept
: ts{0, 0}
{
}

/**
 * \brief Constructor. The @ref TimePoint is initialized with the given `timespec` struct.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _ts
 * `timespec` struct used to initialize the @ref TimePoint instance.\n
 * Note: The ns-portion is normalized to 0..999,999,999 by inc/dec of the sec-portion.
 */
TimePoint::TimePoint(struct ::timespec const & _ts)
: ts(_ts)
{
  NormalizeTimespec(ts);
}

/**
 * \brief Constructor. The @ref TimePoint is initialized with the given `time_t` value.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param sec `time_t` used to initialize the @ref TimePoint instance.
 */
TimePoint::TimePoint(time_t const sec) noexcept
: ts{sec, 0}
{
}

/**
 * \brief Constructor. The @ref TimePoint is initialized with the given `time_t` and nanosecond values.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param sec `time_t` struct used to initialize the @ref TimePoint instance.
 * \param nsec
 * Nanosecond value used to initialize the @ref TimePoint instance.\n
 * Note: The ns-portion is normalized to 0..999,999,999 by inc/dec of the sec-portion.
 */
TimePoint::TimePoint(time_t const sec, int32_t const nsec)
: ts{sec, nsec}
{
  NormalizeTimespec(ts);
}

/**
 * \brief Creates a @ref TimePoint instance initialized with the current value of a specific system clock.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param clock_id
 * ID of the system clock whose time shall be used to initialize the @ref TimePoint instance.\n
 * The clocks available depend on the underlying platform/operating system.\n
 * Most commonly used ones: (there may be more)\n
 * - @ref Clocks::realtime (System time)\n
 * - @ref Clocks::monotonic (Monotonic rising time (not any jumps) starting at some arbitrary point in time)
 * \return
 * A @ref TimePoint instance initialized with the current time fetched from the system clock specified by
 * parameter clock_id.
 */
TimePoint TimePoint::FromSystemClock(Clocks const clock_id)
{
  TimePoint tp;
  tp.LatchSystemClock(clock_id);
  return tp;
}

/**
 * \brief Assigns the value of an `timespec` struct to the @ref TimePoint instance.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _ts
 * `timespec` struct whose value shall be assigned to this @ref TimePoint instance.\n
 * Note: The ns-portion is normalized to 0..999,999,999 by inc/dec of the sec-portion.
 * \return Reference to itself.
 */
TimePoint& TimePoint::operator=(struct ::timespec const & _ts)
{
  struct ::timespec copyOf_ts = _ts;
  NormalizeTimespec(copyOf_ts);
  ts = copyOf_ts;
  return *this;
}

/**
 * \brief Assigns the value of a `time_t` to the @ref TimePoint instance.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param sec
 * `time_t` value that shall be assigned to this @ref TimePoint instance.\n
 * The ns-portion of the @ref TimePoint will be set to zero.
 * \return Reference to itself.
 */
TimePoint& TimePoint::operator=(time_t const sec) noexcept
{
  ts.tv_sec  = sec;
  ts.tv_nsec = 0;
  return *this;
}

/**
 * \brief Adds a @ref TimeSpan to this @ref TimePoint and returns a new @ref TimePoint instance.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv Second operand for the addition.
 * \return A new @ref TimePoint instance with the sum of this @ref TimePoint and rhv.
 */
TimePoint TimePoint::operator + (TimeSpan const & rhv) const
{
  // split the rhv time span into a second and nanosecond portion
  int64_t const rhv_sec  = rhv.value / NSEC_PER_SEC;
  int32_t const rhv_nsec = rhv.value % NSEC_PER_SEC;

  // add second and nanosecond portion to this TimePoint's value and store the result in _ts
  struct ::timespec _ts;
  if (sizeof(_ts.tv_sec) == 4U)
  {
    if (Compiler::OverflowAwareAdd(ts.tv_sec, rhv_sec, reinterpret_cast<int32_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator+: Overflow adding seconds");
  }
  else
  {
    if (Compiler::OverflowAwareAdd(ts.tv_sec, rhv_sec, reinterpret_cast<int64_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator+: Overflow adding seconds");
  }
  _ts.tv_nsec = ts.tv_nsec + rhv_nsec;

  // Return a new TimePoint instance made from _ts.
  // The constructor will take care for potential normalization of the ns portion.
  return TimePoint(_ts);
}

/**
 * \brief Subtracts a @ref TimeSpan from this @ref TimePoint and returns a new @ref TimePoint instance.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv Second operand for the addition.
 * \return A new @ref TimePoint instance with the difference between this @ref TimePoint and rhv.
 */
TimePoint TimePoint::operator - (TimeSpan const & rhv) const
{
  // split the rhv time span into a second and nanosecond portion
  int64_t const rhv_sec  = rhv.value / NSEC_PER_SEC;
  int32_t const rhv_nsec = rhv.value % NSEC_PER_SEC;

  // subtract second and nanosecond portion from this TimePoint's value and store the result in _ts
  struct ::timespec _ts;
  if (sizeof(_ts.tv_sec) == 4U)
  {
    if (Compiler::OverflowAwareSub(ts.tv_sec, rhv_sec, reinterpret_cast<int32_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator-(Timespan): Overflow subtracting seconds");
  }
  else
  {
    if (Compiler::OverflowAwareSub(ts.tv_sec, rhv_sec, reinterpret_cast<int64_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator-(Timespan): Overflow subtracting seconds");
  }
  _ts.tv_nsec = ts.tv_nsec - rhv_nsec;

  // Return a new TimePoint instance made from _ts.
  // The constructor will take care for potential normalization of the ns portion.
  return TimePoint(_ts);
}

/**
 * \brief Calculates the difference between two @ref TimePoint instances.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return Difference between the two @ref TimePoint instances.
 */
TimeSpan TimePoint::operator - (TimePoint const & rhv) const
{
  int64_t dsec;
  if (Compiler::OverflowAwareSub(ts.tv_sec, rhv.ts.tv_sec, &dsec))
    throw std::overflow_error("TimePoint::operator-(Timepoint): Overflow subtracting seconds");

  // note: this check is not precise
  if ((dsec > ((std::numeric_limits<int64_t>::max() / NSEC_PER_SEC) - 1)) ||
      (dsec < ((std::numeric_limits<int64_t>::min() / NSEC_PER_SEC) + 1)))
    throw std::overflow_error("TimePoint::operator-(Timepoint): Overflow in final result");

  int32_t const dnsec = static_cast<int32_t>(ts.tv_nsec) - static_cast<int32_t>(rhv.ts.tv_nsec);

  return TimeSpan((dsec * NSEC_PER_SEC) + dnsec);
}

/**
 * \brief Adds a @ref TimeSpan to this @ref TimePoint.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv
 * The referenced @ref TimeSpan instance is added to this @ref TimePoint instance.\n
 * The result is stored in this @ref TimePoint instance.
 * \return Reference to itself.
 */
TimePoint& TimePoint::operator+= (TimeSpan const & rhv)
{
  // split the rhv time span into a second and nanosecond portion
  int64_t const rhv_sec  = rhv.value / NSEC_PER_SEC;
  int32_t const rhv_nsec = rhv.value % NSEC_PER_SEC;

  // add second and nanosecond portion to this TimePoint's value and store the result in _ts
  struct ::timespec _ts;
  if (sizeof(_ts.tv_sec) == 4U)
  {
    if (Compiler::OverflowAwareAdd(ts.tv_sec, rhv_sec, reinterpret_cast<int32_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator+=: Overflow adding seconds");
  }
  else
  {
    if (Compiler::OverflowAwareAdd(ts.tv_sec, rhv_sec, reinterpret_cast<int64_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator+=: Overflow adding seconds");
  }
  _ts.tv_nsec = ts.tv_nsec + rhv_nsec;

  // tv_nsec may be out of bounds and required inc/dec of tv_sec
  NormalizeTimespec(_ts);

  // assign result
  ts = _ts;
  return *this;
}

/**
 * \brief Subtracts a @ref TimeSpan from this @ref TimePoint.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv
 * The referenced @ref TimeSpan instance is subtracted from this @ref TimePoint instance.\n
 * The result is stored in this @ref TimePoint instance.
 * \return Reference to itself.
 */
TimePoint& TimePoint::operator-= (TimeSpan const & rhv)
{
  // split the rhv time span into a second and nanosecond portion
  int64_t const rhv_sec  = rhv.value / NSEC_PER_SEC;
  int32_t const rhv_nsec = rhv.value % NSEC_PER_SEC;

  // subtract second and nanosecond portion from this TimePoint's value and store the result in _ts
  struct ::timespec _ts;
  if (sizeof(_ts.tv_sec) == 4U)
  {
    if (Compiler::OverflowAwareSub(ts.tv_sec, rhv_sec, reinterpret_cast<int32_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator-=: Overflow subtracting seconds");
  }
  else
  {
    if (Compiler::OverflowAwareSub(ts.tv_sec, rhv_sec, reinterpret_cast<int64_t*>(&_ts.tv_sec)))
      throw std::overflow_error("TimePoint::operator-=: Overflow subtracting seconds");
  }
  _ts.tv_nsec = ts.tv_nsec - rhv_nsec;

  // tv_nsec may be out of bounds and required inc/dec of tv_sec
  NormalizeTimespec(_ts);

  // assign result
  ts = _ts;
  return *this;
}

/**
 * \brief Compares this @ref TimePoint against another one for less-than.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return
 * true  = This @ref TimePoint is before rhv.\n
 * false = This @ref TimePoint and rhv are equal or this is later than rhv.
 */
bool TimePoint::operator < (TimePoint const & rhv) const noexcept
{
  return ((ts.tv_sec < rhv.ts.tv_sec) || ((ts.tv_sec == rhv.ts.tv_sec) && (ts.tv_nsec < rhv.ts.tv_nsec)));
}

/**
 * \brief Compares this @ref TimePoint against another one for equality or less-than.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return
 * true  = This @ref TimePoint is before rhv or equal to rhv.\n
 * false = This @ref TimePoint is later than rhv.
 */
bool TimePoint::operator <= (TimePoint const & rhv) const noexcept
{
  return ((ts.tv_sec < rhv.ts.tv_sec) || ((ts.tv_sec == rhv.ts.tv_sec) && (ts.tv_nsec <= rhv.ts.tv_nsec)));
}

/**
 * \brief Compares this @ref TimePoint against another one for greater-than.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return
 * true  = This @ref TimePoint is after rhv.\n
 * false = This @ref TimePoint and rhv are equal or this is before rhv.
 */
bool TimePoint::operator > (TimePoint const & rhv) const noexcept
{
  return ((ts.tv_sec > rhv.ts.tv_sec) || ((ts.tv_sec == rhv.ts.tv_sec) && (ts.tv_nsec > rhv.ts.tv_nsec)));
}

/**
 * \brief Compares this @ref TimePoint against another one for equality or greater-than.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return
 * true  = This @ref TimePoint is equal to or after rhv.\n
 * false = This @ref TimePoint is before rhv.
 */
bool TimePoint::operator >= (TimePoint const & rhv) const noexcept
{
  return ((ts.tv_sec > rhv.ts.tv_sec) || ((ts.tv_sec == rhv.ts.tv_sec) && (ts.tv_nsec >= rhv.ts.tv_nsec)));
}

/**
 * \brief Compares this @ref TimePoint against another one for equality.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return
 * true  = This @ref TimePoint is equal to rhv.\n
 * false = This @ref TimePoint is not equal to rhv.
 */
bool TimePoint::operator == (TimePoint const & rhv) const noexcept
{
  return ((ts.tv_sec == rhv.ts.tv_sec) && (ts.tv_nsec == rhv.ts.tv_nsec));
}

/**
 * \brief Compares this @ref TimePoint against another one for inequality.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param rhv The right-hand operand.
 * \return
 * true  = This @ref TimePoint is not equal to rhv.\n
 * false = This @ref TimePoint is equal to rhv.
 */
bool TimePoint::operator != (TimePoint const & rhv) const noexcept
{
  return ((ts.tv_sec != rhv.ts.tv_sec) || (ts.tv_nsec != rhv.ts.tv_nsec));
}

/**
 * \brief Sets the @ref TimePoint's value to the current value of a specific system clock.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param clock_id
 * ID of the clock whose time shall be retrieved.\n
 * The clocks available depend on the underlying platform/operating system.\n
 * Most commonly used ones: (there may be more)\n
 * - @ref Clocks::realtime (System time)\n
 * - @ref Clocks::monotonic (Monotonic rising time (not any jumps) starting at some arbitrary point in time)
 */
void TimePoint::LatchSystemClock(Clocks const clock_id)
{
  struct ::timespec _ts;
  GetTime(clock_id, _ts);
  NormalizeTimespec(_ts);
  ts = _ts;
}

/**
 * \brief Sets the @ref TimePoint to a specific value.
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param sec
 * Value for the second-portion of the @ref TimePoint.
 * \param nsec
 * Value for the ns-portion of the @ref TimePoint. \n
 * Note: The ns-portion is normalized to 0..999,999,999 by inc/dec of the sec-portion.
 */
void TimePoint::Set(time_t const sec, int32_t const nsec)
{
  struct ::timespec _ts = { sec, nsec };
  NormalizeTimespec(_ts);
  ts = _ts;
}

/**
 * \brief Retrieves a string representation of the @ref TimePoint using the Gregorian Calendar.
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return
 * String representation of the @ref TimePoint using the Gregorian Calendar.\n
 * The length of the returned string is always @ref stringLength chars.
 * Example:\n
 * 2016-10-30 22:53:12.987ms\n
 * Remember that an @ref TimePoint represents Coordinated Universal Time (UTC), not local time.
 */
std::string TimePoint::ToString(void) const
{
  struct tm * pTM = gmtime(&ts.tv_sec);
  char buffer[32];
  if (snprintf(buffer, sizeof(buffer),
              "%04d-%02d-%02d %02d:%02d:%02d.%03dms", 1900 + static_cast<int>(pTM->tm_year),
                                                      1    + static_cast<int>(pTM->tm_mon),
                                                      static_cast<int>(pTM->tm_mday),
                                                      static_cast<int>(pTM->tm_hour),
                                                      static_cast<int>(pTM->tm_min),
                                                      static_cast<int>(pTM->tm_sec),
                                                      static_cast<int>(ts.tv_nsec / 1000000L)) != stringLength)
  {
    throw std::logic_error("TimePoint::ToString: Unexpected string length");
  }
  return std::string(buffer);
}

/**
 * \brief Static helper function:\n
 * Normalizes the ns-portion of a `timespec` struct to [0..1E9-1] by inc/dec of the second portion.
 *
 * Full arithmetic overflow checks are included.
 *
 * ---
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * Basic guarantee:
 * - `ts` will contain random data
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param ts
 * `timespec` structure, which shall be normalized.
 */
void TimePoint::NormalizeTimespec(struct ::timespec & ts)
{
  if (ts.tv_nsec < 0)
  {
    do
    {
      ts.tv_nsec += NSEC_PER_SEC;

      if (ts.tv_sec == std::numeric_limits<time_t>::min())
      {
        ts.tv_nsec = 0; // ensure basic exception safety
        throw std::overflow_error("TimePoint::NormalizeNanoseconds: Overflow decrementing seconds");
      }
      ts.tv_sec--;
    }
    while (ts.tv_nsec < 0);
  } // if (ts.tv_nsec < 0)
  else
  {
    while (ts.tv_nsec >= NSEC_PER_SEC)
    {
      ts.tv_nsec -= NSEC_PER_SEC;

      if (ts.tv_sec == std::numeric_limits<time_t>::max())
      {
        ts.tv_nsec = 0; // ensure basic exception safety
        throw std::overflow_error("TimePoint::NormalizeNanoseconds: Overflow incrementing seconds");
      }
      ts.tv_sec++;
    }
  } // if (ts.tv_nsec < 0)... else...
}

} // namespace time
} // namespace gpcc
