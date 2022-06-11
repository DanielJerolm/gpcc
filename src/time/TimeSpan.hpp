/*
    General Purpose Class Collection (GPCC)
    Copyright (C) 2011-2017, 2022 Daniel Jerolm

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

#ifndef TIMESPAN_HPP_201701192108
#define TIMESPAN_HPP_201701192108

#include <string>
#include <cstdint>

namespace gpcc {
namespace time {

/**
 * \ingroup GPCC_TIME
 * \brief Class specifying a time span.
 *
 * _Implicit capabilities: default-construction, copy-construction, copy-assignment, move-construction, move-assignment_
 *
 * Class @ref TimeSpan describes a time span with nanosecond resolution. The time span value can be
 * positive (time span ahead from _past to future_) or negative (time span back from _future to past_).
 *
 * Internally a signed 64bit integer is used allowing to describe time spans from approx. -292.471 years
 * (\f$-2^{63}\,\mbox{ns}\f$) up to approx. +292.471 years (\f$(2^{63}-1)\,\mbox{ns}\f$).
 *
 * Instances of class @ref TimeSpan can be created using one of the class' static factory methods:
 * - `ns(int64_t const ns)`
 * - `us(int64_t const us)`
 * - `ms(int64_t const ms)`
 * - `sec(int64_t const sec)`
 * - `min(int32_t const min)`
 * - `hr(int32_t const hr)`
 * - `days(int32_t const days)`
 * - `NegativeMaximum(void)`
 * - `PositiveMaximum(void)`
 *
 * The value of an instance of class @ref TimeSpan can be retrieved using one of the following getter methods:
 * - `int64_t ns(void) const;`
 * - `int64_t us(void) const;`
 * - `int64_t ms(void) const;`
 * - `int64_t sec(void) const;`
 * - `int32_t min(void) const;`
 * - `int32_t hr(void) const;`
 * - `int32_t days(void) const;`
 *
 * Example:
 * ~~~{.cpp}
 * TimeSpan ts = TimeSpan::hr(3) + TimeSpan::min(30);
 * assert(ts.sec() == 12600);
 * ~~~
 *
 * ---
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class TimeSpan final
{
    friend class TimePoint;

  public:
    /// Precision for converting time-spans to strings.
    enum class Precison
    {
      ns,
      us,
      ms,
      sec
    };

    TimeSpan(void) noexcept;

    static TimeSpan ns(int64_t const ns) noexcept;
    static TimeSpan us(int64_t const us);
    static TimeSpan ms(int64_t const ms);
    static TimeSpan sec(int64_t const sec);
    static TimeSpan min(int32_t const min);
    static TimeSpan hr(int32_t const hr);
    static TimeSpan days(int32_t const days);
    static TimeSpan NegativeMaximum(void) noexcept;
    static TimeSpan PositiveMaximum(void) noexcept;

    TimeSpan  operator+(TimeSpan const & rhv) const;
    TimeSpan  operator-(TimeSpan const & rhv) const;
    TimeSpan& operator+=(TimeSpan const & rhv);
    TimeSpan& operator-=(TimeSpan const & rhv);

    bool operator <  (TimeSpan const & rhv) const noexcept;
    bool operator <= (TimeSpan const & rhv) const noexcept;
    bool operator >  (TimeSpan const & rhv) const noexcept;
    bool operator >= (TimeSpan const & rhv) const noexcept;
    bool operator == (TimeSpan const & rhv) const noexcept;
    bool operator != (TimeSpan const & rhv) const noexcept;

    int64_t ns(void) const noexcept;
    int64_t us(void) const noexcept;
    int64_t ms(void) const noexcept;
    int64_t sec(void) const noexcept;
    int32_t min(void) const noexcept;
    int32_t hr(void) const noexcept;
    int32_t days(void) const noexcept;

    std::string ToString(void) const;
    std::string ToString(Precison const prec) const;

  private:
    /// The encapsulated time value.
    int64_t value;

    explicit TimeSpan(int64_t const _value) noexcept;
};

/**
 * \brief Compares this @ref TimeSpan against another one for less-than.
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
 * true  = This @ref TimeSpan is less than rhv.\n
 * false = This @ref TimeSpan and rhv are equal or this is larger than rhv.
 */
inline bool TimeSpan::operator < (TimeSpan const & rhv) const noexcept
{
  return (value < rhv.value);
}

/**
 * \brief Compares this @ref TimeSpan against another one for equality or less-than.
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
 * true  = This @ref TimeSpan is less than rhv or equal to rhv.\n
 * false = This @ref TimeSpan is larger than rhv.
 */
inline bool TimeSpan::operator <= (TimeSpan const & rhv) const noexcept
{
  return (value <= rhv.value);
}

/**
 * \brief Compares this @ref TimeSpan against another one for greater-than.
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
 * true  = This @ref TimeSpan is larger than rhv.\n
 * false = This @ref TimeSpan and rhv are equal or this is less than rhv.
 */
inline bool TimeSpan::operator > (TimeSpan const & rhv) const noexcept
{
  return (value > rhv.value);
}

/**
 * \brief Compares this @ref TimeSpan against another one for equality or greater-than.
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
 * true  = This @ref TimeSpan is equal to or larger than rhv.\n
 * false = This @ref TimeSpan is less than rhv.
 */
inline bool TimeSpan::operator >= (TimeSpan const & rhv) const noexcept
{
  return (value >= rhv.value);
}

/**
 * \brief Compares this @ref TimeSpan against another one for equality.
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
 * true  = This @ref TimeSpan is equal to rhv.\n
 * false = This @ref TimeSpan is not equal to rhv.
 */
inline bool TimeSpan::operator == (TimeSpan const & rhv) const noexcept
{
  return (value == rhv.value);
}

/**
 * \brief Compares this @ref TimeSpan against another one for inequality.
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
 * true  = This @ref TimeSpan is not equal to rhv.\n
 * false = This @ref TimeSpan is equal to rhv.
 */
inline bool TimeSpan::operator != (TimeSpan const & rhv) const noexcept
{
  return (value != rhv.value);
}

} // namespace time
} // namespace gpcc

#endif // TIMESPAN_HPP_201701192108
