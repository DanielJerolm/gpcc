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

#ifndef TIMEPOINT_HPP_201701172134
#define TIMEPOINT_HPP_201701172134

#include "clock.hpp"
#include <string>
#include <cstdint>
#include <cstddef>
#include <ctime>

namespace gpcc {
namespace time {

class TimeSpan;

/**
 * \ingroup GPCC_TIME
 * \brief A absolute point in time based on `struct timespec`.
 *
 * _Implicit capabilities: default-construction, copy-construction, copy-assignment, move-construction, move-assignment_
 *
 * This class specifies an absolute point in time relative to the epoch 00:00:00GMT 01.01.1970.\n
 * GMT is also known as Coordinated Universal Time (UTC).\n
 * Internally the class encapsulates a `timespec` struct as defined by your c-library.\n
 * Example:
 * ~~~{.c}
 * struct ::timespec
 * {
 *   __time_t tv_sec;            // Seconds.
 *   __syscall_slong_t tv_nsec;  // Nanoseconds.
 * };
 * ~~~
 *
 * Note:
 * - The represented time is Coordinated Universal Time (UTC), not local time.
 * - Leap seconds are not taken into account.
 * - Class @ref TimePoint allows `tv_sec` to be negative.
 * - Class @ref TimePoint internally enforces `tv_nsec` to be within 0..999,999,999.\n
 *   However setter and assignment-operators accept any value but will normalize them\n
 *   to 0..999,999,999 upon set/assignment.
 *
 * \see Class @ref TimeSpan
 *
 * ---
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class TimePoint final
{
  public:
    /// Length of any string returned by @ref ToString().
    static size_t constexpr stringLength = 25U;


    TimePoint(void) noexcept;
    explicit TimePoint(struct ::timespec const & _ts);
    explicit TimePoint(time_t const sec) noexcept;
    TimePoint(time_t const sec, int32_t const nsec);

    static TimePoint FromSystemClock(Clocks const clock_id);

    TimePoint& operator=(struct ::timespec const & _ts);
    TimePoint& operator=(time_t const sec) noexcept;

    TimePoint  operator + (TimeSpan const & rhv) const;
    TimePoint  operator - (TimeSpan const & rhv) const;
    TimeSpan   operator - (TimePoint const & rhv) const;
    TimePoint& operator+= (TimeSpan const & rhv);
    TimePoint& operator-= (TimeSpan const & rhv);

    bool operator <  (TimePoint const & rhv) const noexcept;
    bool operator <= (TimePoint const & rhv) const noexcept;
    bool operator >  (TimePoint const & rhv) const noexcept;
    bool operator >= (TimePoint const & rhv) const noexcept;
    bool operator == (TimePoint const & rhv) const noexcept;
    bool operator != (TimePoint const & rhv) const noexcept;

    void LatchSystemClock(Clocks const clock_id);
    void Set(time_t const sec, int32_t const nsec);
    time_t Get_sec(void) const noexcept;
    int32_t Get_nsec(void) const noexcept;
    struct ::timespec const & Get_timespec_ref(void) const noexcept;
    struct ::timespec const * Get_timespec_ptr(void) const noexcept;
    std::string ToString(void) const;

  private:
    /// Encapsulated timespec structure.
    struct ::timespec ts;

    static void NormalizeTimespec(struct ::timespec & ts);
};

/**
 * \brief Gets the second-portion of the @ref TimePoint.
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
 * \return Second-portion of the @ref TimePoint.
 */
inline time_t TimePoint::Get_sec(void) const noexcept
{
  return ts.tv_sec;
}

/**
 * \brief Gets the nanosecond-portion of the @ref TimePoint.
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
 * \return
 * Nanosecond-portion of the @ref TimePoint. \n
 * This is always in the range 0..999,999,999.
 */
inline int32_t TimePoint::Get_nsec(void) const noexcept
{
  return ts.tv_nsec;
}

/**
 * \brief Retrieves an unmodifiable reference to the internal `timespec` struct of the @ref TimePoint.
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
 * \return
 * Reference to the internal `timespec` struct of the @ref TimePoint. \n
 * _The referenced struct is valid until the @ref TimePoint object is destroyed._\n
 * _The referenced struct is valid until the @ref TimePoint object is modified._\n
 * The ns-portion is always in the range 0..999,999,999.
 */
inline struct ::timespec const & TimePoint::Get_timespec_ref(void) const noexcept
{
  return ts;
}

/**
 * \brief Retrieves a read-only pointer to the internal `timespec` struct of the @ref TimePoint.
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
 * \return
 * Pointer to the internal `timespec` struct of the @ref TimePoint. \n
 * _The referenced struct is valid until the @ref TimePoint object is destroyed._\n
 * _The referenced struct is valid until the @ref TimePoint object is modified._\n
 * The ns-portion is always in the range 0..999,999,999.
 */
inline struct ::timespec const * TimePoint::Get_timespec_ptr(void) const noexcept
{
  return &ts;
}

} // namespace time
} // namespace gpcc

#endif /* TIMEPOINT_HPP_201701172134 */
