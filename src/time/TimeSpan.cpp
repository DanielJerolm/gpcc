/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2011 Daniel Jerolm
*/

#include "TimeSpan.hpp"
#include <gpcc/compiler/builtins.hpp>
#include "gpcc/src/osal/Panic.hpp"
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#define NS_PER_US  1000
#define NS_PER_MS  1000000
#define NS_PER_SEC 1000000000
#define NS_PER_MIN (60LL * NS_PER_SEC)
#define NS_PER_HR  (60   * NS_PER_MIN)
#define NS_PER_DAY (24   * NS_PER_HR)

namespace {
int64_t const ns_per_unit[7] =
{
  1,
  NS_PER_US,
  NS_PER_MS,
  NS_PER_SEC,
  NS_PER_MIN,
  NS_PER_HR,
  NS_PER_DAY
};

} // anonymous

namespace gpcc {
namespace time {

/**
 * \brief Constructor. Creates a timespan of zero.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
TimeSpan::TimeSpan(void) noexcept
: value(0)
{
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in nanoseconds.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param ns Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::ns(int64_t const ns) noexcept
{
  return TimeSpan(ns);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in microseconds.
 *
 * Full overflow checks when converting the given value to the internal
 * 64bit nanosecond representation are included.
 *
 * ---
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
 * \param us Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::us(int64_t const us)
{
  if ((us > std::numeric_limits<int64_t>::max() / NS_PER_US) ||
      (us < std::numeric_limits<int64_t>::min() / NS_PER_US))
    throw std::overflow_error("TimeSpan::us: Parameter too large");

  return TimeSpan(us * NS_PER_US);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in milliseconds.
 *
 * Full overflow checks when converting the given value to the internal
 * 64bit nanosecond representation are included.
 *
 * ---
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
 * \param ms Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::ms(int64_t const ms)
{
  if ((ms < std::numeric_limits<int64_t>::min() / NS_PER_MS) ||
      (ms > std::numeric_limits<int64_t>::max() / NS_PER_MS))
    throw std::overflow_error("TimeSpan::ms: Parameter too large");

  return TimeSpan(ms * NS_PER_MS);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in seconds.
 *
 * Full overflow checks when converting the given value to the internal
 * 64bit nanosecond representation are included.
 *
 * ---
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
 * \param sec Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::sec(int64_t const sec)
{
  if ((sec < std::numeric_limits<int64_t>::min() / NS_PER_SEC) ||
      (sec > std::numeric_limits<int64_t>::max() / NS_PER_SEC))
    throw std::overflow_error("TimeSpan::sec: Parameter too large");

  return TimeSpan(sec * NS_PER_SEC);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in minutes.
 *
 * Full overflow checks when converting the given value to the internal
 * 64bit nanosecond representation are included.
 *
 * ---
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
 * \param min Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::min(int32_t const min)
{
  if ((min < std::numeric_limits<int64_t>::min() / NS_PER_MIN) ||
      (min > std::numeric_limits<int64_t>::max() / NS_PER_MIN))
    throw std::overflow_error("TimeSpan::min: Parameter too large");

  return TimeSpan(min * NS_PER_MIN);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in hours.
 *
 * Full overflow checks when converting the given value to the internal
 * 64bit nanosecond representation are included.
 *
 * ---
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
 * \param hr Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::hr(int32_t const hr)
{
  if ((hr < std::numeric_limits<int64_t>::min() / NS_PER_HR) ||
      (hr > std::numeric_limits<int64_t>::max() / NS_PER_HR))
    throw std::overflow_error("TimeSpan::hr: Parameter too large");

  return TimeSpan(hr * NS_PER_HR);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with a time value given in days.
 *
 * Full overflow checks when converting the given value to the internal
 * 64bit nanosecond representation are included.
 *
 * ---
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
 * \param days Value to initialize the @ref TimeSpan instance.
 * \return A @ref TimeSpan instance initialized with the given parameter.
 */
TimeSpan TimeSpan::days(int32_t const days)
{
  if ((days < std::numeric_limits<int64_t>::min() / NS_PER_DAY) ||
      (days > std::numeric_limits<int64_t>::max() / NS_PER_DAY))
    throw std::overflow_error("TimeSpan::days: Parameter too large");

  return TimeSpan(days * NS_PER_DAY);
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with the largest possible negative value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return A @ref TimeSpan instance initialized with the largest possible negative value.
 */
TimeSpan TimeSpan::NegativeMaximum(void) noexcept
{
  return TimeSpan(std::numeric_limits<int64_t>::min());
}

/**
 * \brief Creates a @ref TimeSpan instance initialized with the largest possible positive value.
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 * Operations are guaranteed to succeed and satisfy all requirements even in exceptional situations. If an exception occurs, it will be handled internally and not observed by clients.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \return A @ref TimeSpan instance initialized with the largest possible positive value.
 */
TimeSpan TimeSpan::PositiveMaximum(void) noexcept
{
  return TimeSpan(std::numeric_limits<int64_t>::max());
}

/**
 * \brief Adds two @ref TimeSpan instances.
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
 * \param rhv The other operand.
 * \return A new @ref TimeSpan instance being the sum of this TimeSpan instance and rhv.
 */
TimeSpan TimeSpan::operator+(TimeSpan const & rhv) const
{
  int64_t result;
  if (Compiler::OverflowAwareAdd(value, rhv.value, &result))
    throw std::overflow_error("TimeSpan::operator+");
  return TimeSpan(result);
}

/**
 * \brief Subtracts two @ref TimeSpan instances.
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
 * \param rhv The other operand.
 * \return A new @ref TimeSpan instance being the difference between this TimeSpan instance and rhv.
 */
TimeSpan TimeSpan::operator-(TimeSpan const & rhv) const
{
  int64_t result;
  if (Compiler::OverflowAwareSub(value, rhv.value, &result))
    throw std::overflow_error("TimeSpan::operator-");
  return TimeSpan(result);
}

/**
 * \brief Adds another @ref TimeSpan instance to this @ref TimeSpan instance.
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
 * The referenced @ref TimeSpan instance is added to this @ref TimeSpan instance.\n
 * The result is stored in this @ref TimeSpan instance.
 * \return Reference to itself.
 */
TimeSpan& TimeSpan::operator+=(TimeSpan const & rhv)
{
  int64_t result;
  if (Compiler::OverflowAwareAdd(value, rhv.value, &result))
    throw std::overflow_error("TimeSpan::operator+=");
  value = result;
  return *this;
}

/**
 * \brief Subtracts another @ref TimeSpan instance from this @ref TimeSpan instance.
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
 * The referenced @ref TimeSpan instance is subtracted from this @ref TimeSpan instance.\n
 * The result is stored in this @ref TimeSpan instance.
  \return Reference to itself.
 */
TimeSpan& TimeSpan::operator-=(TimeSpan const & rhv)
{
  int64_t result;
  if (Compiler::OverflowAwareSub(value, rhv.value, &result))
    throw std::overflow_error("TimeSpan::operator-=");
  value = result;
  return *this;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in nanoseconds.
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
 * \return Value of the @ref TimeSpan in nanoseconds.
 */
int64_t TimeSpan::ns(void) const noexcept
{
  return value;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in microseconds.
 *
 * The internal time span value is kept in ns. During conversion to us, any remainder is truncated.
 *
 * ---
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
 * \return Value of the @ref TimeSpan in microseconds.
 */
int64_t TimeSpan::us(void) const noexcept
{
  return value / NS_PER_US;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in milliseconds.
 *
 * The internal time span value is kept in ns. During conversion to ms, any remainder is truncated.
 *
 * ---
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
 * \return Value of the @ref TimeSpan in milliseconds.
 */
int64_t TimeSpan::ms(void) const noexcept
{
  return value / NS_PER_MS;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in seconds.
 *
 * The internal time span value is kept in ns. During conversion to sec, any remainder is truncated.
 *
 * ---
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
 * \return Value of the @ref TimeSpan in seconds.
 */
int64_t TimeSpan::sec(void) const noexcept
{
  return value / NS_PER_SEC;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in minutes.
 *
 * The internal time span value is kept in ns. During conversion to min, any remainder is truncated.
 *
 * ---
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
 * \return Value of the @ref TimeSpan in minutes.
 */
int32_t TimeSpan::min(void) const noexcept
{
  return value / NS_PER_MIN;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in hours.
 *
 * The internal time span value is kept in ns. During conversion to hours, any remainder is truncated.
 *
 * ---
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
 * \return Value of the @ref TimeSpan in hours.
 */
int32_t TimeSpan::hr(void) const noexcept
{
  return value / NS_PER_HR;
}

/**
 * \brief Retrieves the value of the @ref TimeSpan in days.
 *
 * The internal time span value is kept in ns. During conversion to days, any remainder is truncated.
 *
 * ---
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
 * \return Value of the @ref TimeSpan in days.
 */
int32_t TimeSpan::days(void) const noexcept
{
  return value / NS_PER_DAY;
}

/**
 * \brief Creates a human-readable string representation of the @ref TimeSpan.
 *
 * There are overloaded versions of this method available:\n
 * - This version creates the smallest, shortest and most compact (but still exact) string
 *   representation of the timespan. It does not use a fixed format and tries to minimize text output.
 * - `ToString(Precison const prec)` creates a string representation using a fixed format:
 *   [d.]hh:mm:ss.[fff[fff[fff]]].
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
 * \return
 * Human-readable string representation of the @ref TimeSpan. \n
 * Examples:\n
 * 0d\n
 * 5d 12hr\n
 * 8d 12:09hr\n
 * 01:12:44hr\n
 * 02:33.768ms\n
 * 12567us\n
 */
std::string TimeSpan::ToString(void) const
{
  enum Units
  {
    ns   = 0,
    us   = 1,
    ms   = 2,
    sec  = 3,
    min  = 4,
    hr   = 5,
    days = 6
  };

  std::ostringstream str;

  int64_t _value = value;
  if (_value < 0)
  {
    str << "-";
    _value = -_value;
  }

  Units base          = Units::days;
  Units currentUnit   = Units::days;
  bool lookingForBase = true;
  bool anyOutputYet   = false;

  while (true)
  {
    int64_t const divider = ns_per_unit[static_cast<int>(currentUnit)];
    int64_t const full = _value / divider;
    int64_t const frac = _value % divider;

    if ((!lookingForBase) || (full != 0) || (currentUnit == Units::ns))
    {
      if (lookingForBase)
      {
        base = currentUnit;
        lookingForBase = false;
      }

      bool const end = (frac == 0);
      switch (currentUnit)
      {
        case Units::days:
          str << full << "d";
          if (!end)
            str << " ";
          break;

        case Units::hr:
          if (anyOutputYet)
            str << std::setw(2) << std::setfill('0');
          str << full;

          if (end)
            str << "hr";
          else
            str << ":";
          break;

        case Units::min:
          if (anyOutputYet)
            str << std::setw(2) << std::setfill('0');
          str << full;

          if (end)
          {
            if (base == Units::min)
              str << "min";
            else
              str << "hr";
          }
          else
            str << ":";
          break;

        case Units::sec:
          if (anyOutputYet)
            str << std::setw(2) << std::setfill('0');
          else if (!end)
              str << "0:" << std::setw(2) << std::setfill('0');
          str << full;

          if (end)
          {
            if (base == Units::sec)
              str << "sec";
            else if (base == Units::min)
              str << "min";
          }
          else
            str << ".";
          break;

        case Units::ms:
          if (anyOutputYet)
            str << std::setw(3) << std::setfill('0');
          str << full;

          if (end)
            str << "ms";
          break;

        case Units::us:
          if (anyOutputYet)
            str << std::setw(3) << std::setfill('0');
          str << full;

          if (end)
            str << "us";
          break;

        case Units::ns:
          if (anyOutputYet)
            str << std::setw(3) << std::setfill('0');
          str << full << "ns";
          break;
      }

      if (end)
        break;

      anyOutputYet = true;
    }

    _value = frac;
    currentUnit = static_cast<Units>(static_cast<int>(currentUnit) - 1);
  }

  return str.str();
}

/**
 * \brief Creates a human readable string representation of the @ref TimeSpan.
 *
 * There are overloaded versions of this method available:\n
 * - This version creates a string representation using a fixed format: [d.]hh:mm:ss.[fff[fff[fff]]].
 * - `ToString(void)` creates the smallest, shortest and most compact (but still exact) string
 *   representation of the timespan. It does not use a fixed format and tries to minimize text output.
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
 * \param prec
 * This determines the format used for fractions of seconds:
 * Value              | format
 * ------------------ | -----------------------
 * @ref Precison::ns  | [d.]hh:mm:ss.fffffffff
 * @ref Precison::us  | [d.]hh:mm:ss.ffffff
 * @ref Precison::ms  | [d.]hh:mm:ss.fff
 * @ref Precison::sec | [d.]hh:mm:ss
 *
 * Note that any fraction below `prec` is truncated. There is no round up/down.
 * \return
 * Human-readable string representation of the @ref TimeSpan. \n
 * Example (prec = Precision::us):\n
 * 00:12:59.039223us\n
 * Example (prec = Precision::ms):\n
 * 00:12:59.039ms\n
 * Example (prec = Precision::sec):\n
 * 00:12:59
 */
std::string TimeSpan::ToString(Precison const prec) const
{
  std::ostringstream str;

  int64_t _value = value;
  if (_value < 0)
    _value = -_value;

  bool zero = true;

  // days
  {
    int32_t const days = _value / NS_PER_DAY;
    _value %= NS_PER_DAY;

    if (days != 0)
    {
      str << days << ".";
      zero = false;
    }
  }

  // hh:mm:ss
  {
    int32_t seconds = _value / NS_PER_SEC;
    if (seconds != 0)
      zero = false;

    int32_t const hours = seconds / 3600;
    seconds %= 3600;

    int32_t const minutes = seconds / 60;
    seconds %= 60;

    str << std::setw(2) << std::setfill('0') << hours << ":";
    str << std::setw(2) << std::setfill('0') << minutes << ":";
    str << std::setw(2) << std::setfill('0') << seconds;
  }

  // ms/us/ns
  if (prec != Precison::sec)
  {
    str << '.';

    uint32_t const ns = _value % NS_PER_SEC;

    switch (prec)
    {
      case Precison::ns:
      {
        if (ns != 0)
          zero = false;
        str << std::setw(9) << std::setfill('0') << ns << "ns";
        break;
      }

      case Precison::us:
      {
        int32_t const us = ns / NS_PER_US;
        if (us != 0)
          zero = false;
        str << std::setw(6) << std::setfill('0') << us << "us";
        break;
      }

      case Precison::ms:
      {
        int32_t const ms = ns / NS_PER_MS;
        if (ms != 0)
          zero = false;
        str << std::setw(3) << std::setfill('0') << ms << "ms";
        break;
      }

      default:
      {
        PANIC();
        break;
      }
    }
  }

  if ((value >= 0) || (zero))
    return str.str();
  else
    return "-" + str.str();
}

/**
 * \brief Private constructor. Creates a @ref TimeSpan initialized with a specific value.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * Safe, no cancellation point included.
 *
 * ---
 *
 * \param _value Attribute `value` is initialized with this.
 */
TimeSpan::TimeSpan(int64_t const _value) noexcept
: value(_value)
{
}

} /* namespace time */
} /* namespace gpcc */
